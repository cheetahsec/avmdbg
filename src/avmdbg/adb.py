"""Minimal cross-platform adb wrapper used to discover and reach debuggable
processes.

Requires ``adb`` on ``PATH`` (Android SDK platform-tools).
"""

import shutil
import subprocess
from dataclasses import dataclass

from .jdwp.errors import AdbError

DEFAULT_TIMEOUT = 15


@dataclass(frozen=True)
class Device:
    serial: str
    state: str


@dataclass(frozen=True)
class Process:
    pid: int
    name: str


def _adb_base(serial=None) -> list:
    adb = shutil.which("adb")
    if adb is None:
        raise AdbError("adb not found on PATH (install Android SDK platform-tools)")
    cmd = [adb]
    if serial:
        cmd += ["-s", serial]
    return cmd


def run_adb(args, serial=None, timeout: int = DEFAULT_TIMEOUT) -> str:
    """Run one adb command and return stdout; raises AdbError on failure."""
    cmd = _adb_base(serial) + list(args)
    try:
        proc = subprocess.run(
            cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            timeout=timeout,
            creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0),
        )
    except subprocess.TimeoutExpired as exc:
        raise AdbError(f"adb command timed out: {' '.join(cmd)}") from exc
    if proc.returncode != 0:
        detail = proc.stderr.decode("utf-8", errors="replace").strip()
        raise AdbError(f"adb command failed ({' '.join(cmd)}): {detail}")
    return proc.stdout.decode("utf-8", errors="replace")


def list_devices() -> list:
    """Return connected devices in the ``device`` (online) state."""
    out = run_adb(["devices"])
    devices = []
    for line in out.splitlines()[1:]:
        parts = line.split()
        if len(parts) >= 2:
            devices.append(Device(serial=parts[0], state=parts[1]))
    return devices


def jdwp_pids(serial=None) -> list:
    """PIDs of processes with an open JDWP endpoint (debuggable, not yet attached)."""
    out = run_adb(["jdwp"], serial=serial)
    pids = []
    for line in out.splitlines():
        line = line.strip()
        if line.isdigit():
            pids.append(int(line))
    return pids


def process_map(serial=None) -> dict:
    """Map of pid -> process name, parsed from ``ps`` output."""
    try:
        out = run_adb(["shell", "ps", "-A", "-o", "PID,NAME"], serial=serial)
        result = {}
        for line in out.splitlines():
            parts = line.split()
            if len(parts) == 2 and parts[0].isdigit():
                result[int(parts[0])] = parts[1]
        if result:
            return result
    except AdbError:
        pass
    # Fallback for very old toybox/toolbox versions without ``-o`` support.
    out = run_adb(["shell", "ps"], serial=serial)
    result = {}
    for line in out.splitlines()[1:]:
        parts = line.split()
        if len(parts) >= 9 and parts[1].isdigit():
            result[int(parts[1])] = parts[-1]
    return result


def debuggable_processes(serial=None) -> list:
    """Debuggable (JDWP-advertising) processes with their package names."""
    names = process_map(serial)
    return [Process(pid=pid, name=names[pid]) for pid in jdwp_pids(serial) if pid in names]


def forward_jdwp(port: int, pid: int, serial=None) -> None:
    """Forward a local TCP port to the JDWP endpoint of ``pid``."""
    run_adb(["forward", f"tcp:{port}", f"jdwp:{pid}"], serial=serial)


def forward_remove(port: int, serial=None) -> None:
    try:
        run_adb(["forward", "--remove", f"tcp:{port}"], serial=serial)
    except AdbError:
        pass


def set_debug_app(package: str, serial=None, wait: bool = True, persistent: bool = False) -> None:
    """Mark ``package`` as the debug app so it waits for a debugger on start."""
    args = ["shell", "am", "set-debug-app"]
    if wait:
        args.append("-w")
    if persistent:
        args.append("--persistent")
    args.append(package)
    run_adb(args, serial=serial)


def clear_debug_app(serial=None) -> None:
    run_adb(["shell", "am", "clear-debug-app"], serial=serial)


def launch_app(package: str, serial=None) -> None:
    """Launch the package's launcher activity via monkey."""
    run_adb(
        [
            "shell",
            "monkey",
            "-p",
            package,
            "-c",
            "android.intent.category.LAUNCHER",
            "1",
        ],
        serial=serial,
    )
