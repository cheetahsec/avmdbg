"""avmdbg - a lightweight, agent-friendly debugger for the Android runtime.

Pure Python, zero dependencies: it speaks JDWP over ``adb forward`` and
exposes breakpoints, stack inspection, register/parameter reads and object
introspection both as a library and as a CLI.
"""

from .debugger import Breakpoint, Debugger, EventContext
from .jdwp.errors import (
    AdbError,
    AttachError,
    AvmdbgError,
    JdwpConnectionError,
    JdwpError,
    JdwpTimeoutError,
)

__version__ = "1.0.0"

__all__ = [
    "AdbError",
    "AttachError",
    "AvmdbgError",
    "Breakpoint",
    "Debugger",
    "EventContext",
    "JdwpConnectionError",
    "JdwpError",
    "JdwpTimeoutError",
    "__version__",
]
