# avmdbg

[![CI](https://github.com/cheetahsec/avmdbg/actions/workflows/ci.yml/badge.svg)](https://github.com/cheetahsec/avmdbg/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Python](https://img.shields.io/badge/python-3.9%2B-blue.svg)](https://www.python.org/)

A lightweight, **agent-friendly** debugger for the Android runtime
(Dalvik/ART). avmdbg speaks [JDWP](https://docs.oracle.com/javase/8/docs/technotes/guides/jpda/jdwp-spec.html)
over `adb forward` — pure Python, zero dependencies, cross-platform.

It sets breakpoints at **any dex code offset** (not just method entries),
decodes method parameters from Dalvik registers, prints stack traces, and
inspects strings, objects, fields and arrays — all through a one-shot CLI
that prints JSON, or as a Python library.

## Features

- Breakpoints at arbitrary dex offsets in any loaded method
- Parameter decoding from Dalvik registers (p/v naming, wide types)
- Stack traces with class/method/signature resolution
- Register reads (`v0:I` style), string/field/array inspection
- Start apps suspended (`am set-debug-app -w`) to debug startup code
- JSON output + stable exit codes: built for scripts and AI agents
- Ships an [agent skill](skills/avmdbg/SKILL.md) so AI coding agents can
  drive it autonomously

## Requirements

- Python ≥ 3.9
- `adb` (Android SDK platform-tools) on `PATH`
- A connected device or emulator running a **debuggable** build of the target app

## Install

```console
$ pip install git+https://github.com/cheetahsec/avmdbg.git
$ avmdbg --version
```

Or from a source checkout:

```console
$ git clone https://github.com/cheetahsec/avmdbg.git
$ cd avmdbg && pip install .
```

## Quick start

List devices and debuggable processes:

```console
$ avmdbg devices
{"devices": [{"serial": "emulator-5554", "state": "device"}]}

$ avmdbg processes
{"processes": [{"pid": 5103, "name": "com.example.x0r.demo"}]}
```

Launch an app suspended, so breakpoints land before any app code runs:

```console
$ avmdbg start com.example.x0r.demo
{"package": "com.example.x0r.demo", "pid": 5103, "waiting_for_debugger": true}
```

Set a breakpoint and capture the first hit (trigger the method in the app
while it waits):

```console
$ avmdbg break com.example.x0r.demo \
    --class 'Lcom/example/x0r/demo/LoginActivity;' \
    --method test0 --sign '(B[IJLjava/lang/String;)V' \
    --index 0x0 --registers 9 --pretty
{
  "process": {"pid": 5103, "name": "com.example.x0r.demo"},
  "breakpoint": {"request_id": 1, "class": "Lcom/example/x0r/demo/LoginActivity;", ...},
  "hits": [
    {
      "thread": {"id": 5123, "name": "main"},
      "params": {
        "p0": {"type": "object", "slot": 3, "id": 3123456789},
        "p1": {"type": "byte", "slot": 4, "value": 42},
        "p3": {"type": "long", "slot": 6, "value": 1099511627776},
        "p5": {"type": "string", "slot": 8, "value": "hello"}
      },
      "stack": [
        {"class": "Lcom/example/x0r/demo/LoginActivity;", "method": "test0", "index": 0},
        ...
      ]
    }
  ]
}
```

Dig deeper with repeatable options:

```console
$ avmdbg break com.example.x0r.demo --class ... --method ... --sign ... \
    --registers 9 --reg v0:I --reg v1:J --fields p0 --count 3
```

The four breakpoint coordinates come from smali (via `apktool d` or
`baksmali`):

| Option | Where to find it |
|---|---|
| `--class` | `.class` directive, JNI signature form |
| `--method` / `--sign` | `.method` directive |
| `--index` | hex `@` offsets in `baksmali -l` output |
| `--registers` | `.registers` in the method header |

Every command prints one JSON document to stdout; errors are JSON on stderr.
Exit codes: `0` ok · `1` error · `2` adb problem · `3` attach failed ·
`4` breakpoint timeout.

## Python library

```python
from avmdbg import Debugger

with Debugger.attach("com.example.x0r.demo") as dbg:
    dbg.set_breakpoint(
        class_signature="Lcom/example/x0r/demo/LoginActivity;",
        method="test0",
        signature="(B[IJLjava/lang/String;)V",
        index=0,
        registers=9,
    )
    hit = dbg.wait_event(timeout=60)
    print(hit.params)          # decoded parameters
    print(dbg.object_fields(hit.params["p0"]["id"]))
    dbg.resume()
```

## For AI agents

The repository ships a ready-to-use skill at
[`skills/avmdbg/SKILL.md`](skills/avmdbg/SKILL.md). Copy `skills/avmdbg/` into
your agent's skills directory and it will know the full debugging workflow.

## Documentation

- [Usage guide](docs/usage.md) — CLI reference, library API, fixture-app walkthrough
- [How it works](docs/how-it-works.md) — JDWP internals, breakpoint events,
  Dalvik register/slot analysis

## Development

```console
$ pip install -e .[dev]
$ pytest tests -q        # 50 offline unit tests
$ ruff check src tests
```

Device-backed end-to-end tests run only when requested:
`AVMDDBG_INTEGRATION=1 pytest tests/test_integration.py` (installs
`tests/fixtures/app-debug.apk` on the connected device).

## History

avmdbg started in 2016 as a Windows-only C++/pybind11 extension with a
Python 2 API. Version 1.0 is a full rewrite: a pure-Python, cross-platform
JDWP client with a JSON CLI and an agent skill. The legacy implementation
lives on in the git history.

## License

[MIT](LICENSE)
