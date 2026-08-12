"""JDWP packet encoding and decoding.

A JDWP packet is an 11-byte header followed by a variable-length body. All
multi-byte fields are big-endian. The header layout is::

    u4 length      - total packet size, header included
    u4 id          - correlates a command with its reply
    u1 flags       - 0x00 command, 0x80 reply
    u2 cmdset/cmd  - command packets only
    u2 error code  - reply packets only
"""

import itertools
import struct
import threading
from dataclasses import dataclass, field

from .constants import HEADER_LEN, REPLY_FLAG

_id_counter = itertools.count(1)
_id_lock = threading.Lock()


def next_packet_id() -> int:
    with _id_lock:
        return next(_id_counter) & 0x7FFFFFFF


@dataclass
class Packet:
    """One JDWP packet (command or reply)."""

    id: int
    flags: int
    data: bytes = b""
    cmd_set: int = 0
    cmd: int = 0
    error: int = 0

    @property
    def is_reply(self) -> bool:
        return self.flags == REPLY_FLAG

    @classmethod
    def command(cls, cmd_set: int, cmd: int, data: bytes = b"") -> "Packet":
        return cls(id=next_packet_id(), flags=0, cmd_set=cmd_set, cmd=cmd, data=data)

    def encode(self) -> bytes:
        length = HEADER_LEN + len(self.data)
        if self.is_reply:
            header = struct.pack(">IIBH", length, self.id, self.flags, self.error)
        else:
            header = struct.pack(">IIBBB", length, self.id, self.flags, self.cmd_set, self.cmd)
        return header + self.data

    @classmethod
    def decode(cls, header: bytes, body: bytes) -> "Packet":
        if len(header) != HEADER_LEN:
            raise ValueError(f"JDWP header must be {HEADER_LEN} bytes, got {len(header)}")
        length, pkt_id, flags = struct.unpack(">IIB", header[:9])
        if length != HEADER_LEN + len(body):
            raise ValueError("JDWP packet length mismatch")
        if flags == REPLY_FLAG:
            (error,) = struct.unpack(">H", header[9:11])
            return cls(id=pkt_id, flags=flags, error=error, data=body)
        return cls(id=pkt_id, flags=flags, cmd_set=header[9], cmd=header[10], data=body)


class Writer:
    """Big-endian payload builder for command bodies."""

    def __init__(self) -> None:
        self._buf = bytearray()

    def u8(self, value: int) -> "Writer":
        self._buf += struct.pack(">B", value)
        return self

    def u16(self, value: int) -> "Writer":
        self._buf += struct.pack(">H", value)
        return self

    def u32(self, value: int) -> "Writer":
        self._buf += struct.pack(">I", value)
        return self

    def u64(self, value: int) -> "Writer":
        self._buf += struct.pack(">Q", value)
        return self

    def ident(self, value: int, size: int) -> "Writer":
        self._buf += value.to_bytes(size, "big")
        return self

    def string(self, value: str) -> "Writer":
        encoded = value.encode("utf-8")
        self._buf += struct.pack(">I", len(encoded))
        self._buf += encoded
        return self

    def build(self) -> bytes:
        return bytes(self._buf)


class Reader:
    """Big-endian cursor over a reply body, aware of the VM's ID sizes."""

    def __init__(self, data: bytes, id_sizes=None) -> None:
        if id_sizes is None:
            from .ids import IdSizes

            id_sizes = IdSizes()
        self._data = data
        self._pos = 0
        self.id_sizes = id_sizes

    def remaining(self) -> int:
        return len(self._data) - self._pos

    def take(self, size: int) -> bytes:
        """Consume ``size`` raw bytes."""
        if self._pos + size > len(self._data):
            raise ValueError("JDWP reply truncated")
        chunk = self._data[self._pos : self._pos + size]
        self._pos += size
        return chunk

    def u8(self) -> int:
        return struct.unpack(">B", self.take(1))[0]

    def u16(self) -> int:
        return struct.unpack(">H", self.take(2))[0]

    def u32(self) -> int:
        return struct.unpack(">I", self.take(4))[0]

    def u64(self) -> int:
        return struct.unpack(">Q", self.take(8))[0]

    def boolean(self) -> bool:
        return self.u8() != 0

    def string(self) -> str:
        length = self.u32()
        return self.take(length).decode("utf-8", errors="replace")

    def object_id(self) -> int:
        return int.from_bytes(self.take(self.id_sizes.object), "big")

    def reftype_id(self) -> int:
        return int.from_bytes(self.take(self.id_sizes.reftype), "big")

    def method_id(self) -> int:
        return int.from_bytes(self.take(self.id_sizes.method), "big")

    def field_id(self) -> int:
        return int.from_bytes(self.take(self.id_sizes.field), "big")

    def frame_id(self) -> int:
        return int.from_bytes(self.take(self.id_sizes.frame), "big")
