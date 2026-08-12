"""Command line interface for avmdbg.

Designed to be agent-friendly: every command is one-shot, prints a single
JSON document to stdout, reports errors as JSON on stderr and uses stable
exit codes:

- 0  success
- 1  unexpected error
- 2  adb error (missing adb, command failure)
- 3  attach error (process not debuggable / not running)
- 4  timeout waiting for a debug event
"""

import argparse
import json
import sys
import time

from . import __version__, adb
from .debugger import DEFAULT_PORT, Debugger
from .jdwp.errors import AdbError, AttachError, AvmdbgError, JdwpError

EXIT_OK = 0
EXIT_ERROR = 1
EXIT_ADB = 2
EXIT_ATTACH = 3
EXIT_TIMEOUT = 4


def _emit(payload, pretty: bool) -> None:
    json.dump(payload, sys.stdout, indent=2 if pretty else None, ensure_ascii=False)
    sys.stdout.write("\n")


def _emit_error(kind: str, message: str) -> None:
    json.dump({"error": {"type": kind, "message": message}}, sys.stderr, ensure_ascii=False)
    sys.stderr.write("\n")


def _parse_reg_spec(spec: str) -> tuple:
    """Parse a ``NAME:TAG`` register read request, e.g. ``v0:I``."""
    name, sep, tag = spec.partition(":")
    if not sep or not name or len(tag) != 1:
        raise argparse.ArgumentTypeError(
            f"invalid register spec {spec!r}, expected NAME:TAG (e.g. v0:I, p2:L)"
        )
    return name, tag


def _attach(args) -> Debugger:
    return Debugger.attach(
        args.process, serial=args.serial, port=args.port, timeout=args.timeout
    )


def cmd_devices(args) -> dict:
    devices = adb.list_devices()
    return {"devices": [d.__dict__ for d in devices]}


def cmd_processes(args) -> dict:
    processes = adb.debuggable_processes(args.serial)
    return {"processes": [{"pid": p.pid, "name": p.name} for p in processes]}


def cmd_start(args) -> dict:
    """Start an app in wait-for-debugger mode and report its PID."""
    adb.set_debug_app(args.process, serial=args.serial, wait=True)
    try:
        adb.launch_app(args.process, serial=args.serial)
        deadline = args.timeout

        start = time.monotonic()
        pid = None
        while time.monotonic() - start < deadline:
            for proc in adb.debuggable_processes(args.serial):
                if proc.name == args.process:
                    pid = proc.pid
                    break
            if pid is not None:
                break
            time.sleep(0.5)
        if pid is None:
            raise AvmdbgError(
                f"{args.process} did not show a JDWP endpoint within {deadline}s"
            )
        return {"package": args.process, "pid": pid, "waiting_for_debugger": True}
    finally:
        if not args.keep_debug_flag:
            adb.clear_debug_app(serial=args.serial)


def cmd_attach(args) -> dict:
    with _attach(args) as dbg:
        return {
            "process": {"pid": dbg.process.pid, "name": dbg.process.name},
            "vm": dbg.version(),
        }


def cmd_classes(args) -> dict:
    with _attach(args) as dbg:
        classes = dbg.list_classes(match=args.match)
        return {"count": len(classes), "classes": classes}


def cmd_methods(args) -> dict:
    with _attach(args) as dbg:
        methods = dbg.find_class_methods(args.class_signature)
        return {"class": args.class_signature, "count": len(methods), "methods": methods}


def cmd_break(args) -> dict:
    with _attach(args) as dbg:
        bp = dbg.set_breakpoint(
            class_signature=args.class_signature,
            method=args.method,
            signature=args.signature,
            index=args.index,
            registers=args.registers,
        )
        hits = []
        for _ in range(args.count):
            ctx = dbg.wait_event(
                timeout=args.event_timeout,
                include_stack=not args.no_stack,
                include_params=not args.no_params,
            )
            if ctx is None:
                if not hits:
                    raise _EventTimeout(
                        f"no breakpoint hit within {args.event_timeout}s"
                    )
                break
            hit = ctx.to_dict()
            for name, tag in args.reg or []:
                hit.setdefault("registers", {})[name] = dbg.get_register(
                    ctx.thread_id, ctx.frame_id, name, tag
                )
            for name in args.fields or []:
                param = ctx.params.get(name)
                if param and "id" in param:
                    hit.setdefault("fields", {})[name] = dbg.object_fields(param["id"])
            hits.append(hit)
            dbg.resume()
        return {"process": {"pid": dbg.process.pid, "name": dbg.process.name},
                "breakpoint": bp.to_dict(), "hits": hits}


class _EventTimeout(AvmdbgError):
    pass


COMMANDS = {
    "devices": cmd_devices,
    "processes": cmd_processes,
    "start": cmd_start,
    "attach": cmd_attach,
    "classes": cmd_classes,
    "methods": cmd_methods,
    "break": cmd_break,
}


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog="avmdbg",
        description="A lightweight, agent-friendly debugger for the Android runtime. "
        "All commands print JSON to stdout.",
    )
    parser.add_argument("--version", action="version", version=f"%(prog)s {__version__}")
    parser.add_argument("--serial", help="adb device serial (default: first device)")
    parser.add_argument(
        "--port", type=int, default=DEFAULT_PORT, help="local TCP port for adb forward"
    )
    parser.add_argument(
        "--timeout",
        type=float,
        default=10.0,
        help="JDWP/adb round-trip timeout in seconds",
    )
    parser.add_argument("--pretty", action="store_true", help="pretty-print JSON output")

    sub = parser.add_subparsers(dest="command", required=True, metavar="command")

    sub.add_parser("devices", help="list connected adb devices")
    sub.add_parser("processes", help="list debuggable processes on the device")

    p_start = sub.add_parser(
        "start", help="launch an app suspended, waiting for the debugger"
    )
    p_start.add_argument("process", help="package name, e.g. com.example.app")
    p_start.add_argument(
        "--keep-debug-flag",
        action="store_true",
        help="keep the app marked as debug app after it starts",
    )

    p_attach = sub.add_parser("attach", help="attach to a process and print VM info")
    p_attach.add_argument("process", help="package name, e.g. com.example.app")

    p_classes = sub.add_parser("classes", help="list classes loaded in the target VM")
    p_classes.add_argument("process")
    p_classes.add_argument("--match", help="substring filter on the class signature")

    p_methods = sub.add_parser("methods", help="list methods declared by a class")
    p_methods.add_argument("process")
    p_methods.add_argument(
        "--class",
        dest="class_signature",
        required=True,
        help="JNI class signature, e.g. Lcom/example/app/LoginActivity;",
    )

    p_break = sub.add_parser(
        "break", help="set a breakpoint, wait for hits and dump their context"
    )
    p_break.add_argument("process")
    p_break.add_argument("--class", dest="class_signature", required=True)
    p_break.add_argument("--method", required=True, help="method name")
    p_break.add_argument(
        "--sign", dest="signature", required=True, help="method signature, e.g. (BIJ)V"
    )
    p_break.add_argument(
        "--index",
        type=lambda s: int(s, 0),
        default=0,
        help="dex code offset from baksmali -l (hex like 0x10 or decimal), default 0",
    )
    p_break.add_argument(
        "--registers",
        type=int,
        required=True,
        help="the method's .registers value from its smali header",
    )
    p_break.add_argument(
        "--count", type=int, default=1, help="number of hits to capture, default 1"
    )
    p_break.add_argument(
        "--event-timeout",
        type=float,
        default=60.0,
        help="seconds to wait for each hit, default 60",
    )
    p_break.add_argument(
        "--reg",
        action="append",
        type=_parse_reg_spec,
        metavar="NAME:TAG",
        help="also read a register on each hit, e.g. v0:I or p2:L (repeatable)",
    )
    p_break.add_argument(
        "--fields",
        action="append",
        metavar="PARAM",
        help="dump field values of an object parameter, e.g. p0 (repeatable)",
    )
    p_break.add_argument("--no-stack", action="store_true", help="omit the call stack")
    p_break.add_argument("--no-params", action="store_true", help="omit parameters")
    return parser


def main(argv=None) -> int:
    args = build_parser().parse_args(argv)
    handler = COMMANDS[args.command]
    try:
        _emit(handler(args), args.pretty)
        return EXIT_OK
    except _EventTimeout as exc:
        _emit_error("timeout", str(exc))
        return EXIT_TIMEOUT
    except AttachError as exc:
        _emit_error("attach", str(exc))
        return EXIT_ATTACH
    except AdbError as exc:
        _emit_error("adb", str(exc))
        return EXIT_ADB
    except (AvmdbgError, JdwpError) as exc:
        _emit_error(type(exc).__name__, str(exc))
        return EXIT_ERROR
    except KeyboardInterrupt:
        _emit_error("interrupted", "interrupted by user")
        return EXIT_ERROR


if __name__ == "__main__":
    sys.exit(main())
