# How avmdbg Works: Debugging the Android Runtime over JDWP

This document explains the principles behind avmdbg: the JDWP protocol, how
breakpoints at arbitrary dex offsets work, and how call stacks, parameters and
register values are retrieved.

## 1. Dynamic analysis options for Android apps

Common approaches to dynamically analyzing an APK, and their trade-offs:

- **Smali instrumentation**: decompile the APK, insert logging smali, repack
  and reinstall. Tedious — every new log point means another repack cycle, and
  packed/protected apps may fail to rebuild at all.
- **IDA Pro**: supports dex debugging since 6.6, but watchpoints require
  manually specifying the value type and stepping without fixing the type can
  crash the VM (the reason is explained in section 3.3). Commercial.
- **JEB**: powerful smali + native debugging since 2.0. Commercial.
- **AndBug**: unmaintained open-source JDWP client; Linux-only and missing key
  features such as breakpoints at arbitrary code offsets.
- **Android Studio + smalidea**: a solid free solution using an IDE debugger
  plugin, with occasional line-skipping glitches.
- **Hook frameworks** (Xposed, Cydia Substrate, Frida, ddi): great for hooking
  method entries/exits, but they cannot break at arbitrary instructions inside
  a method body.

avmdbg fills the gap: a small scriptable debugger that sets breakpoints at any
dex code offset, dumps parameters/registers and prints stack traces — over a
plain JDWP connection.

## 2. JDWP in a nutshell

JDWP (Java Debug Wire Protocol) is the communication protocol between a
debugger and a target VM. The debugger queries program state and controls
execution (breakpoints, thread state, variable values); the VM notifies the
debugger when events fire (breakpoint hits, thread creation, ...).

Although ART/Dalvik differ from a regular JVM, their debug interface is the
same JDWP. The Dalvik server implementation lives in
[`dalvik/vm/jdwp`](https://android.googlesource.com/platform/dalvik/+/eclair-release/vm/jdwp/)
and the ART one in
[`art/runtime/jdwp`](https://android.googlesource.com/platform/art/+/refs/heads/main/runtime/jdwp/).
Android Studio, Eclipse and DDMS all build on JDWP.

### Handshake

Every session starts with an identity handshake: both sides exchange the
literal string `JDWP-Handshake` before any packet is parsed.

### Transport

The Android runtime exposes JDWP either over adb directly or over a socket.
avmdbg uses adb port forwarding:

```
adb forward tcp:8819 jdwp:<pid>
```

and then speaks plain TCP to `127.0.0.1:8819`.

### Packets

Two packet kinds exist:

- **Command packets**: debugger → VM to query or control; VM → debugger to
  report events.
- **Reply packets**: VM → debugger with the result of a command.

A packet is an 11-byte header plus a variable-length body:

```
u4 length    - total packet length including this header
u4 id        - correlates a reply with its command
u1 flags     - 0x00 = command, 0x80 = reply
u2 cmd set + cmd id   (command packets, as two bytes)
u2 error code         (reply packets; non-zero means the command failed)
```

Commands are grouped into command sets (VirtualMachine, ReferenceType,
ThreadReference, StackFrame, EventRequest, ...). The full list is in the
[JDWP specification](https://docs.oracle.com/javase/8/docs/technotes/guides/jpda/jdwp-spec.html).
Android implements a subset, but everything a debugger needs is there.

Two encoding rules to keep in mind:

1. All multi-byte fields are **big-endian**.
2. Composite types have length-prefixed layouts, e.g. a `string` is
   `u4 length` + UTF-8 bytes.

Some IDs (objectID, referenceTypeID, methodID, fieldID, frameID) have
VM-defined widths; query them once with `VirtualMachine.IDSizes` right after
the handshake. avmdbg honors the reported sizes everywhere instead of
hard-coding the historical Dalvik widths (4-byte field/method IDs, 8-byte
object IDs).

## 3. The core feature: breakpoints at any code offset

Static analysis is enough for simple apps, but you want a real breakpoint
when:

- you need to observe key values at runtime, e.g. a decrypted string;
- you want to hook a low-level shared routine to find suspicious callers;
- the call graph is too tangled and you need a stack trace at a key point.

### 3.1 Setting the breakpoint and handling the event

Setting a breakpoint uses `EventRequest.Set`. It supports many event kinds
(breakpoint, single-step, class load/prepare, method entry/exit, field access,
thread, exception, ...). On success the VM returns a `requestID`; when the
event fires, the VM sends an `Event.Composite` command to the debugger.

A breakpoint request carries one `LocationOnly` modifier:

```
u1 eventKind      = 2 (BREAKPOINT)
u1 suspendPolicy  = 0/1/2 (none / event thread / all threads)
u4 modifiers      = 1
u1 modKind        = 7 (LOCATION_ONLY)
location:
  u1  typeTag     = 1 (class)
  referenceTypeID classId
  methodID        methodId
  u8  index       - dex code offset ("dex pc")
```

- `classId` comes from `VirtualMachine.ClassesBySignature`
- `methodId` from `ReferenceType.MethodsWithGeneric`
- `index` is the code offset shown by `baksmali -l` (offsets printed by other
  disassemblers may be wrong)

When execution reaches the location, the VM sends `Event.Composite` containing
the event kind, the `requestID`, the thread id and the location. The debugger
matches the `requestID` to the pending breakpoint, inspects state, and finally
resumes the VM with `VirtualMachine.Resume` (when the suspend policy was
`SP_ALL`).

### 3.2 Getting the call stack

`ThreadReference.Frames(threadId, startFrame, length)` returns the frames of a
suspended thread; frame 0 is the current method. Each frame carries a
`frameID` (needed later to read variables) and a location (class, method, dex
pc). avmdbg resolves class and method names through
`ReferenceType.SignatureWithGeneric` and `ReferenceType.MethodsWithGeneric`.

### 3.3 Reading parameters and register variables

Dalvik/ART is register-based: smali code operates on registers, not a JVM
operand stack. Reading a local uses `StackFrame.GetValues`:

```
request:  threadID, frameID, u4 slotCount, per slot: u4 slot + u1 tag
reply:    u4 valueCount, per value: u1 tag + value
```

The critical field is **slot** — the variable's index in the frame. Debug
builds may expose `Method.VariableTable`, but release builds (the norm in
reverse engineering) carry no such metadata, so slots must be computed the
way the VM does. From the Dalvik sources (`dvmDbgGetLocalValue` in
`Debugger.c`):

1. `frameID` is the current stack pointer; registers are memory-mapped
   relative to it and `slot` is the index into that area. Parameters occupy
   the **last N registers**; locals start at `v0`.
2. Dalvik untweaks the slot: the sentinel slot **1000** is remapped to slot 0,
   while a literal slot 0 is remapped to the first parameter slot (an Eclipse
   compatibility quirk). avmdbg maps `v0` to slot 1000 for this reason.
3. Value sizes by type: boolean/byte/short/char/int/float read 4 bytes (1
   register); array/object read an object pointer (1 register); double/long
   read 8 bytes (2 registers).
4. This is also why manually typed watch windows (e.g. IDA's) can crash the
   VM: if the register's actual type changes while stepping and the watch
   still reads it as the old type, the bad access can kill the process.

Slot computation (see `avmdbg.jdwp.types.compute_param_slots`):

- A method with `M` registers and parameters occupying `N` slots keeps its
  parameters in the last `N` registers; `v0..v(M-N-1)` are locals.
- `p` naming covers parameters only, starting at `p0`; for non-static methods
  `p0` is `this`.
- long/double parameters occupy two adjacent registers.

Example: `.registers 9`, signature `(B[IJLjava/lang/String;)V`, non-static:

| register | slot | content        |
|----------|------|----------------|
| v0-v2    | 0-2  | locals         |
| p0       | 3    | this           |
| p1       | 4    | B              |
| p2       | 5    | [I             |
| p3       | 6-7  | J (wide)       |
| p5       | 8    | Ljava/lang/String; |

## 4. What avmdbg implements

- attach to a debuggable process (`adb jdwp` + `ps` + `adb forward` + handshake)
- breakpoints at arbitrary dex offsets (`EventRequest.Set/Clear`,
  `Event.Composite` dispatch)
- stack traces (`ThreadReference.Frames` + name resolution)
- parameter/register reads (`StackFrame.GetValues` + slot analysis)
- strings (`StringReference.Value`), object fields
  (`ReferenceType.FieldsWithGeneric` + `GetValues`), arrays
  (`ArrayReference.Length/GetValues`)
- everything exposed both as a Python library and as a one-shot JSON CLI

## 5. References

- [JDWP specification](https://docs.oracle.com/javase/8/docs/technotes/guides/jpda/jdwp-spec.html)
- [smali: Registers](https://github.com/JesusFreke/smali/wiki/Registers),
  [TypesMethodsAndFields](https://github.com/JesusFreke/smali/wiki/TypesMethodsAndFields)
- [Dalvik JDWP sources](https://android.googlesource.com/platform/dalvik/+/eclair-release/vm/jdwp/)
- [ART JDWP sources](https://android.googlesource.com/platform/art/+/refs/heads/main/runtime/jdwp/)
