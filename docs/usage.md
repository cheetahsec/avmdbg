# avmdbg Usage Guide

avmdbg is a lightweight debugger for the Android runtime (Dalvik/ART). It
speaks JDWP over `adb forward` and can be used two ways:

- as a **CLI** that prints JSON (designed for scripts and AI agents)
- as a **Python library**

Prerequisites: Python ≥ 3.9, `adb` on `PATH` (Android SDK platform-tools), a
debuggable app on a connected device or emulator.

## 1. CLI

Install and check the device:

```console
$ pip install avmdbg
$ avmdbg devices
{"devices": [{"serial": "emulator-5554", "state": "device"}]}

$ avmdbg processes
{"processes": [{"pid": 5103, "name": "com.example.x0r.demo"}]}
```

Start an app suspended, before any of its code runs:

```console
$ avmdbg start com.example.x0r.demo
{"package": "com.example.x0r.demo", "pid": 5103, "waiting_for_debugger": true}
```

Attach to inspect VM info:

```console
$ avmdbg attach com.example.x0r.demo
{"process": {"pid": 5103, "name": "com.example.x0r.demo"}, "vm": {"description": "...", ...}}
```

List loaded classes and methods:

```console
$ avmdbg classes com.example.x0r.demo --match LoginActivity
$ avmdbg methods com.example.x0r.demo --class Lcom/example/x0r/demo/LoginActivity;
```

Set a breakpoint and wait for the first hit:

```console
$ avmdbg break com.example.x0r.demo \
    --class Lcom/example/x0r/demo/LoginActivity; \
    --method test0 --sign "(B[IJLjava/lang/String;)V" \
    --index 0x0 --registers 9 --pretty
```

Breakpoint arguments:

- `--class` — JNI class signature, e.g. `Lcom/example/x0r/demo/LoginActivity;`
- `--method` / `--sign` — method name and signature, e.g. `(B[IJ)V`;
  see [smali's type notation](https://github.com/JesusFreke/smali/wiki/TypesMethodsAndFields)
- `--index` — dex code offset, from `baksmali -l` output (hex or decimal)
- `--registers` — the `.registers` value in the method's smali header
- `--count N` — capture N hits (auto-resumes between hits)
- `--reg v0:I` — additionally read a register on each hit (repeatable);
  the tag letter is the value type (`I` int, `J` long, `L` object, `[` array)
- `--fields p0` — additionally dump the fields of an object parameter
  (repeatable)
- `--no-stack` / `--no-params` — trim the hit context

Every command prints exactly one JSON document to stdout. Errors are JSON on
stderr with stable exit codes: `0` ok, `1` error, `2` adb problem, `3` attach
failed, `4` breakpoint timeout.

## 2. Python library

```python
from avmdbg import Debugger

with Debugger.attach("com.example.x0r.demo") as dbg:
    bp = dbg.set_breakpoint(
        class_signature="Lcom/example/x0r/demo/LoginActivity;",
        method="test0",
        signature="(B[IJLjava/lang/String;)V",
        index=0,
        registers=9,
    )

    hit = dbg.wait_event(timeout=60)   # VM is suspended now
    if hit is None:
        raise SystemExit("breakpoint not hit")

    # Decoded parameters (p0 is `this` for non-static methods)
    for name, param in hit.params.items():
        print(name, param)

    # Call stack, top frame first
    for frame in hit.stack:
        print(frame["class"], "->", frame["method"], hex(frame["index"]))

    # Fields of the `this` object
    print(dbg.object_fields(hit.params["p0"]["id"]))

    # Extra register reads (v/p naming + type tag)
    print(dbg.get_register(hit.thread_id, hit.frame_id, "v0", "I"))

    # Strings and arrays
    # dbg.get_string(object_id); dbg.array_values(object_id)

    dbg.resume()                       # let the app continue
```

### API summary

| Method | Purpose |
|---|---|
| `Debugger.attach(name)` | attach by package name (or `attach_pid(pid)`) |
| `set_breakpoint(...)` | breakpoint at a dex offset; returns a `Breakpoint` |
| `clear_breakpoint(bp)` / `clear_all_breakpoints()` | remove breakpoints |
| `wait_event(timeout)` | wait for a hit; returns an `EventContext` or `None` |
| `resume()` | resume all threads after a hit |
| `stack_frames(thread_id)` | call stack of a suspended thread |
| `get_register(thread_id, frame_id, name, tag)` | read one register |
| `get_string(object_id)` | value of a `String` object |
| `object_fields(object_id)` | static + instance field values |
| `array_values(object_id)` | array elements (bounded preview) |
| `list_classes(match)` | loaded classes, optional substring filter |
| `find_class_methods(sig)` | declared methods of a class |
| `close()` | detach; the app keeps running |

### Notes on registers

- Dalvik registers are 32-bit; `long`/`double` occupy two adjacent registers.
- With `M` registers and `N` parameter slots, parameters live in the last `N`
  registers; locals start at `v0`.
- `p` naming covers parameters only; `p0` is `this` in non-static methods.
- A literal slot 0 is remapped by Dalvik to the first parameter slot, so
  avmdbg reads `v0` through the sentinel slot 1000. See
  [how-it-works.md](how-it-works.md) for the details.

## 3. Try it with the fixture app

`tests/fixtures/app-debug.apk` is a demo app whose
[`LoginActivity.smali`](../tests/fixtures/LoginActivity.smali) contains
`test0(B[IJLjava/lang/String;)V` — the method used in the examples above.

```console
$ adb install -r tests/fixtures/app-debug.apk
$ avmdbg start com.example.x0r.demo
# in another terminal, or after the command returns:
$ avmdbg break com.example.x0r.demo \
    --class Lcom/example/x0r/demo/LoginActivity; \
    --method test0 --sign "(B[IJLjava/lang/String;)V" \
    --index 0 --registers 9 --pretty
# then tap the button in the app to trigger the method
```
