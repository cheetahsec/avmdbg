"""Exception types raised by the JDWP client."""

from .constants import ERROR_NAMES


class AvmdbgError(Exception):
    """Base class for all avmdbg errors."""


class AdbError(AvmdbgError):
    """An adb command failed or adb is not available."""


class AttachError(AvmdbgError):
    """Attaching to the target process failed."""


class JdwpConnectionError(AvmdbgError):
    """The JDWP transport failed (socket error, broken handshake, ...)."""


class JdwpTimeoutError(AvmdbgError):
    """Timed out waiting for a JDWP reply or event."""


class JdwpError(AvmdbgError):
    """The target VM replied with a non-zero JDWP error code."""

    def __init__(self, code, detail=""):
        self.code = code
        self.name = ERROR_NAMES.get(code, "UNKNOWN")
        message = f"JDWP error {code} ({self.name})"
        if detail:
            message += f": {detail}"
        super().__init__(message)
