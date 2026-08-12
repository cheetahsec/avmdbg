---
name: avmdbg
description: Debug Android apps at the smali level with the avmdbg CLI — attach to debuggable processes over JDWP, set breakpoints at arbitrary dex offsets, and dump parameters, registers and stack traces as JSON. Use when reverse-engineering or dynamically analyzing an Android APK, inspecting runtime values (decrypted strings, crypto keys, parameters), tracing call paths, or verifying runtime behavior of a debuggable app. Requires adb and a connected device/emulator.
---

# avmdbg — agent workflow for Android runtime debugging

avmdbg speaks JDWP over `adb forward`. Every command is one-shot, prints one
JSON document to stdout, reports errors as JSON on stderr, and uses stable
exit codes: `0` ok, `1` error, `2` adb problem, `3` attach failed, `4`
breakpoint timeout. Always parse stdout as JSON.

## Install / run

```bash
pip install avmdbg            # or: pip install git+https://github.com/cheetahsec/avmdbg.git
avmdbg --version
```

If pip is unavailable, run from a source checkout with `PYTHONPATH=src python -m avmdbg ...`.

## Standard workflow

1. **Check the device** — `avmdbg devices` (empty list = connect one / start an emulator), then `avmdbg processes` to see debuggable apps. A release app never shows up here; it must be debuggable (`android:debuggable="true"` or a debug build). If `adb` is missing, install Android SDK platform-tools.

2. **Pick the target method from smali** — disassemble with `apktool d app.apk` or `baksmali`. From the smali you need exactly four things:
   - class signature: `Lcom/example/app/LoginActivity;`
   - method name + signature: `test0` / `(B[IJLjava/lang/String;)V`
   - dex code offset: the hex `@` offsets from `baksmali -l` output (`--index`; 0 = method entry)
   - `.registers` count from the method header (`--registers`)
   Offsets from JEB/jadx are not reliable; use baksmali/apktool output.

3. **(Optional) start the app suspended** — `avmdbg start <package>` launches the app in wait-for-debugger mode and returns its pid. Use this to break during app startup. Attach quickly: the VM waits for a debugger.

4. **Break and dump** —

```bash
avmdbg break com.example.x0r.demo \
  --class 'Lcom/example/x0r/demo/LoginActivity;' \
  --method test0 --sign '(B[IJLjava/lang/String;)V' \
  --index 0x0 --registers 9 --pretty
```

   The command attaches, sets the breakpoint, waits up to `--event-timeout` seconds (default 60), prints the hit context and detaches. The hit JSON contains:
   - `params`: decoded method parameters keyed by register name (`p0` is `this` for non-static methods); strings are dereferenced, arrays include a bounded preview, objects carry an `id`
   - `stack`: call stack, top frame first (`class`, `method`, `signature`, `index`)
   - `thread`, `frame_id`, `breakpoint`

5. **Dig deeper on the next run** — re-run with:
   - `--reg v0:I` to also read arbitrary registers (repeatable; tag letters: `Z B S C I F J D` primitives, `L` object, `[` array)
   - `--fields p0` to dump field values of an object parameter (repeatable)
   - `--count N` to capture several hits

6. **Trigger the app** — after starting `avmdbg break`, drive the app (manually, or `adb shell input tap ...`, or `adb shell am start ...`) so the target method executes.

## Interpretation tips

- `p`-registers hold parameters in declaration order; long/double take two slots (`p3` wide → next is `p5`). Locals are `v0..`.
- Integral values are decoded as signed. Object values show `"id"` — use `--fields` to inspect them.
- Exit code 4 means the breakpoint was not hit in time: wrong offset, wrong method signature, or the code path was not executed — re-check the smali and trigger the app again.
- `classes --match <substr>` and `methods --class <sig>` verify that the class/method is actually loaded before setting a breakpoint.
- Multiple devices: pass `--serial <serial>` to any command.

## Library use

For custom tooling, the same functionality is available as a Python library
(`from avmdbg import Debugger`) — see `docs/usage.md` in the repo for the API.
