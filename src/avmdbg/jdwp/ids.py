"""Sizes of the variably-sized JDWP identifier types.

The width of object/reference-type/method/field/frame IDs is defined by the
target VM and must be queried with ``VirtualMachine.IDSizes`` right after the
handshake. Historically Dalvik used 4-byte field/method IDs and 8-byte object
IDs; modern ART versions differ between 32-bit and 64-bit processes, so the
values are never hard-coded.
"""

from dataclasses import dataclass


@dataclass(frozen=True)
class IdSizes:
    field: int = 4
    method: int = 4
    object: int = 8
    reftype: int = 8
    frame: int = 8
