# Changelog

All notable changes to this project are documented here. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and this project
adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2026-08-12

Full rewrite of the project.

### Added

- Pure-Python, zero-dependency JDWP client (`avmdbg.jdwp`): packet codec,
  socket transport with handshake, typed command wrappers and composite-event
  parsing; ID widths honor `VirtualMachine.IDSizes`, so modern ART works.
- High-level `avmdbg.Debugger` session API: attach by package or PID,
  breakpoints by class/method/signature/dex-offset, event waits with decoded
  parameters and stack traces, register/string/object/array inspection.
- Cross-platform adb wrapper (`avmdbg.adb`): devices, JDWP pids, `ps`
  parsing, port forwarding, `set-debug-app`, app launch.
- Agent-friendly CLI (`avmdbg` / `python -m avmdbg`): one-shot subcommands
  (`devices`, `processes`, `start`, `attach`, `classes`, `methods`, `break`)
  that print JSON and use stable exit codes.
- Agent skill at `skills/avmdbg/SKILL.md`.
- English documentation: `docs/usage.md` and `docs/how-it-works.md`.
- pytest suite (50 offline unit tests) plus an opt-in device-backed
  integration test; fixtures moved to `tests/fixtures/`.
- GitHub Actions CI (lint, test matrix, build) and a tag-triggered release
  workflow.
- MIT LICENSE, .gitignore.

### Changed

- All code and documentation are now in English.

### Removed

- The legacy Windows-only C++/pybind11 extension, Visual Studio project
  files, vendored pybind11 and easylogging header, the prebuilt `bin/`
  artifacts, the Word documents and the Python 2 demo. The 0.x line remains
  available in the git history.

## [0.x] - 2016

Legacy versions: C++ extension (`AvmDebugger`) with a Python 2 scripting API.
