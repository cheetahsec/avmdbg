"""Typed JDWP command wrappers and reply parsers.

Every method maps to one JDWP command of the specification and returns plain
dataclasses. Outbound IDs are encoded with the sizes reported by the target
VM (``VirtualMachine.IDSizes``).
"""

import struct
from dataclasses import dataclass

from .connection import JdwpConnection
from .constants import (
    ArrayReferenceCmd,
    CommandSet,
    EventKind,
    EventRequestCmd,
    MethodCmd,
    ModKind,
    ObjectReferenceCmd,
    ReferenceTypeCmd,
    StackFrameCmd,
    StringReferenceCmd,
    SuspendPolicy,
    Tag,
    ThreadReferenceCmd,
    TypeTag,
    VirtualMachineCmd,
)
from .errors import JdwpConnectionError
from .ids import IdSizes
from .packet import Reader, Writer
from .types import is_object_tag


@dataclass(frozen=True)
class Location:
    type_tag: int
    class_id: int
    method_id: int
    index: int


@dataclass(frozen=True)
class ClassInfo:
    type_tag: int
    type_id: int
    status: int
    signature: str = ""
    generic_signature: str = ""


@dataclass(frozen=True)
class FieldInfo:
    field_id: int
    name: str
    signature: str
    generic_signature: str
    mod_bits: int


@dataclass(frozen=True)
class MethodInfo:
    method_id: int
    name: str
    signature: str
    generic_signature: str
    mod_bits: int


@dataclass(frozen=True)
class FrameInfo:
    frame_id: int
    location: Location


@dataclass(frozen=True)
class Value:
    """A tagged JDWP value. ``value`` holds the decoded Python value."""

    tag: Tag
    value: object


@dataclass(frozen=True)
class JdwpEvent:
    """One event from an Event.Composite notification."""

    kind: EventKind
    request_id: int
    thread_id: int = 0
    location: Location = None


@dataclass(frozen=True)
class CompositeEvent:
    suspend_policy: int
    events: tuple


class JdwpClient:
    """High-level JDWP command set over a :class:`JdwpConnection`."""

    def __init__(self, connection: JdwpConnection):
        self._conn = connection
        self.id_sizes = self._query_id_sizes()

    def _query_id_sizes(self) -> IdSizes:
        data = self._send(CommandSet.VIRTUAL_MACHINE, VirtualMachineCmd.ID_SIZES)
        reader = Reader(data)
        return IdSizes(
            field=reader.u32(),
            method=reader.u32(),
            object=reader.u32(),
            reftype=reader.u32(),
            frame=reader.u32(),
        )

    def _reader(self, data: bytes) -> Reader:
        return Reader(data, self.id_sizes)

    def _send(self, cmd_set: int, cmd: int, data: bytes = b"", timeout=None) -> bytes:
        return self._conn.send_command(int(cmd_set), int(cmd), data, timeout)

    # -- VirtualMachine command set --------------------------------------

    def version(self) -> dict:
        reader = self._reader(self._send(CommandSet.VIRTUAL_MACHINE, VirtualMachineCmd.VERSION))
        return {
            "description": reader.string(),
            "jdwp_major": reader.u32(),
            "jdwp_minor": reader.u32(),
            "vm_version": reader.string(),
            "vm_name": reader.string(),
        }

    def classes_by_signature(self, signature: str) -> list:
        payload = Writer().string(signature).build()
        reader = self._reader(
            self._send(CommandSet.VIRTUAL_MACHINE, VirtualMachineCmd.CLASSES_BY_SIGNATURE, payload)
        )
        classes = []
        for _ in range(reader.u32()):
            classes.append(
                ClassInfo(
                    type_tag=reader.u8(),
                    type_id=reader.reftype_id(),
                    status=reader.u32(),
                )
            )
        return classes

    def all_classes_with_generic(self) -> list:
        reader = self._reader(
            self._send(CommandSet.VIRTUAL_MACHINE, VirtualMachineCmd.ALL_CLASSES_WITH_GENERIC)
        )
        classes = []
        for _ in range(reader.u32()):
            classes.append(
                ClassInfo(
                    type_tag=reader.u8(),
                    type_id=reader.reftype_id(),
                    signature=reader.string(),
                    generic_signature=reader.string(),
                    status=reader.u32(),
                )
            )
        return classes

    def all_threads(self) -> list:
        reader = self._reader(self._send(CommandSet.VIRTUAL_MACHINE, VirtualMachineCmd.ALL_THREADS))
        return [reader.object_id() for _ in range(reader.u32())]

    def suspend_vm(self) -> None:
        self._send(CommandSet.VIRTUAL_MACHINE, VirtualMachineCmd.SUSPEND)

    def resume_vm(self) -> None:
        self._send(CommandSet.VIRTUAL_MACHINE, VirtualMachineCmd.RESUME)

    def dispose(self) -> None:
        """Invalidate the JDWP connection without killing the target VM."""
        try:
            self._send(CommandSet.VIRTUAL_MACHINE, VirtualMachineCmd.DISPOSE)
        except JdwpConnectionError:
            pass

    # -- ReferenceType command set ----------------------------------------

    def reftype_signature_with_generic(self, type_id: int) -> tuple:
        payload = Writer().ident(type_id, self.id_sizes.reftype).build()
        reader = self._reader(
            self._send(CommandSet.REFERENCE_TYPE, ReferenceTypeCmd.SIGNATURE_WITH_GENERIC, payload)
        )
        return reader.string(), reader.string()

    def reftype_fields_with_generic(self, type_id: int) -> list:
        payload = Writer().ident(type_id, self.id_sizes.reftype).build()
        reader = self._reader(
            self._send(CommandSet.REFERENCE_TYPE, ReferenceTypeCmd.FIELDS_WITH_GENERIC, payload)
        )
        fields = []
        for _ in range(reader.u32()):
            fields.append(
                FieldInfo(
                    field_id=reader.field_id(),
                    name=reader.string(),
                    signature=reader.string(),
                    generic_signature=reader.string(),
                    mod_bits=reader.u32(),
                )
            )
        return fields

    def reftype_methods_with_generic(self, type_id: int) -> list:
        payload = Writer().ident(type_id, self.id_sizes.reftype).build()
        reader = self._reader(
            self._send(CommandSet.REFERENCE_TYPE, ReferenceTypeCmd.METHODS_WITH_GENERIC, payload)
        )
        methods = []
        for _ in range(reader.u32()):
            methods.append(
                MethodInfo(
                    method_id=reader.method_id(),
                    name=reader.string(),
                    signature=reader.string(),
                    generic_signature=reader.string(),
                    mod_bits=reader.u32(),
                )
            )
        return methods

    def reftype_get_values(self, type_id: int, field_ids: list) -> list:
        writer = Writer().ident(type_id, self.id_sizes.reftype).u32(len(field_ids))
        for field_id in field_ids:
            writer.ident(field_id, self.id_sizes.field)
        reader = self._reader(
            self._send(CommandSet.REFERENCE_TYPE, ReferenceTypeCmd.GET_VALUES, writer.build())
        )
        return self._read_values(reader)

    # -- Method command set -----------------------------------------------

    def method_line_table(self, type_id: int, method_id: int) -> dict:
        payload = (
            Writer()
            .ident(type_id, self.id_sizes.reftype)
            .ident(method_id, self.id_sizes.method)
            .build()
        )
        reader = self._reader(
            self._send(CommandSet.METHOD, MethodCmd.LINE_TABLE, payload)
        )
        start, end = reader.u64(), reader.u64()
        lines = []
        for _ in range(reader.u32()):
            lines.append({"code_index": reader.u64(), "line": reader.u32()})
        return {"start": start, "end": end, "lines": lines}

    def method_variable_table_with_generic(self, type_id: int, method_id: int) -> list:
        payload = (
            Writer()
            .ident(type_id, self.id_sizes.reftype)
            .ident(method_id, self.id_sizes.method)
            .build()
        )
        reader = self._reader(
            self._send(CommandSet.METHOD, MethodCmd.VARIABLE_TABLE_WITH_GENERIC, payload)
        )
        reader.u32()  # argCnt
        variables = []
        for _ in range(reader.u32()):
            variables.append(
                {
                    "code_index": reader.u64(),
                    "name": reader.string(),
                    "signature": reader.string(),
                    "generic_signature": reader.string(),
                    "length": reader.u32(),
                    "slot": reader.u32(),
                }
            )
        return variables

    # -- EventRequest command set -----------------------------------------

    def set_breakpoint_request(
        self,
        class_id: int,
        method_id: int,
        index: int,
        suspend_policy: int = SuspendPolicy.ALL,
    ) -> int:
        """Set a location breakpoint; returns the JDWP request id."""
        writer = (
            Writer()
            .u8(EventKind.BREAKPOINT)
            .u8(int(suspend_policy))
            .u32(1)  # modifier count
            .u8(ModKind.LOCATION_ONLY)
            .u8(TypeTag.CLASS)
            .ident(class_id, self.id_sizes.reftype)
            .ident(method_id, self.id_sizes.method)
            .u64(index)
        )
        reader = self._reader(
            self._send(CommandSet.EVENT_REQUEST, EventRequestCmd.SET, writer.build())
        )
        return reader.u32()

    def clear_event_request(self, event_kind: int, request_id: int) -> None:
        payload = Writer().u8(int(event_kind)).u32(request_id).build()
        self._send(CommandSet.EVENT_REQUEST, EventRequestCmd.CLEAR, payload)

    def clear_all_breakpoints(self) -> None:
        self._send(CommandSet.EVENT_REQUEST, EventRequestCmd.CLEAR_ALL_BREAKPOINTS)

    # -- ObjectReference / StringReference command sets -------------------

    def object_reftype(self, object_id: int) -> tuple:
        payload = Writer().ident(object_id, self.id_sizes.object).build()
        reader = self._reader(
            self._send(CommandSet.OBJECT_REFERENCE, ObjectReferenceCmd.REFERENCE_TYPE, payload)
        )
        return reader.u8(), reader.reftype_id()

    def object_get_values(self, object_id: int, field_ids: list) -> list:
        writer = Writer().ident(object_id, self.id_sizes.object).u32(len(field_ids))
        for field_id in field_ids:
            writer.ident(field_id, self.id_sizes.field)
        reader = self._reader(
            self._send(CommandSet.OBJECT_REFERENCE, ObjectReferenceCmd.GET_VALUES, writer.build())
        )
        return self._read_values(reader)

    def string_value(self, object_id: int) -> str:
        payload = Writer().ident(object_id, self.id_sizes.object).build()
        reader = self._reader(
            self._send(CommandSet.STRING_REFERENCE, StringReferenceCmd.VALUE, payload)
        )
        return reader.string()

    # -- ThreadReference command set ---------------------------------------

    def thread_name(self, thread_id: int) -> str:
        payload = Writer().ident(thread_id, self.id_sizes.object).build()
        reader = self._reader(
            self._send(CommandSet.THREAD_REFERENCE, ThreadReferenceCmd.NAME, payload)
        )
        return reader.string()

    def thread_status(self, thread_id: int) -> dict:
        payload = Writer().ident(thread_id, self.id_sizes.object).build()
        reader = self._reader(
            self._send(CommandSet.THREAD_REFERENCE, ThreadReferenceCmd.STATUS, payload)
        )
        return {"status": reader.u32(), "suspend_status": reader.u32()}

    def thread_frames(self, thread_id: int, start: int = 0, length: int = -1) -> list:
        payload = (
            Writer()
            .ident(thread_id, self.id_sizes.object)
            .u32(start & 0xFFFFFFFF)
            .u32(length & 0xFFFFFFFF)
            .build()
        )
        reader = self._reader(
            self._send(CommandSet.THREAD_REFERENCE, ThreadReferenceCmd.FRAMES, payload)
        )
        frames = []
        for _ in range(reader.u32()):
            frames.append(
                FrameInfo(frame_id=reader.frame_id(), location=self._read_location(reader))
            )
        return frames

    def resume_thread(self, thread_id: int) -> None:
        payload = Writer().ident(thread_id, self.id_sizes.object).build()
        self._send(CommandSet.THREAD_REFERENCE, ThreadReferenceCmd.RESUME, payload)

    # -- ArrayReference command set ----------------------------------------

    def array_length(self, object_id: int) -> int:
        payload = Writer().ident(object_id, self.id_sizes.object).build()
        reader = self._reader(
            self._send(CommandSet.ARRAY_REFERENCE, ArrayReferenceCmd.LENGTH, payload)
        )
        return reader.u32()

    def array_values(self, object_id: int, first: int = 0, length: int = 0) -> tuple:
        payload = (
            Writer()
            .ident(object_id, self.id_sizes.object)
            .u32(first)
            .u32(length)
            .build()
        )
        reader = self._reader(
            self._send(CommandSet.ARRAY_REFERENCE, ArrayReferenceCmd.GET_VALUES, payload)
        )
        element_tag = Tag(reader.u8())
        count = reader.u32()
        values = []
        for _ in range(count):
            if element_tag in (Tag.OBJECT, Tag.ARRAY, Tag.STRING):
                # Object arrays tag every element individually.
                tag = Tag(reader.u8())
                values.append(Value(tag=tag, value=reader.object_id()))
            else:
                values.append(Value(tag=element_tag, value=self._read_plain(reader, element_tag)))
        return element_tag, values

    # -- StackFrame command set ---------------------------------------------

    def frame_get_values(self, thread_id: int, frame_id: int, slots: list) -> list:
        """Read local variables. ``slots`` is a list of ``(slot, Tag)`` pairs."""
        writer = (
            Writer()
            .ident(thread_id, self.id_sizes.object)
            .ident(frame_id, self.id_sizes.frame)
            .u32(len(slots))
        )
        for slot, tag in slots:
            writer.u32(slot).u8(int(tag))
        reader = self._reader(
            self._send(CommandSet.STACK_FRAME, StackFrameCmd.GET_VALUES, writer.build())
        )
        return self._read_values(reader)

    def frame_this_object(self, thread_id: int, frame_id: int) -> tuple:
        payload = (
            Writer()
            .ident(thread_id, self.id_sizes.object)
            .ident(frame_id, self.id_sizes.frame)
            .build()
        )
        reader = self._reader(
            self._send(CommandSet.STACK_FRAME, StackFrameCmd.THIS_OBJECT, payload)
        )
        return reader.u8(), reader.object_id()

    # -- Value / location decoding helpers ---------------------------------

    def _read_plain(self, reader: Reader, tag: Tag):
        """Decode one primitive (non-object) value body.

        Integral values are decoded as signed (two's complement), matching
        how the target VM interprets them.
        """
        if tag == Tag.BOOLEAN:
            return reader.boolean()
        if tag == Tag.BYTE:
            return struct.unpack(">b", reader.take(1))[0]
        if tag == Tag.CHAR:
            return reader.u16()
        if tag == Tag.SHORT:
            return struct.unpack(">h", reader.take(2))[0]
        if tag == Tag.FLOAT:
            return struct.unpack(">f", reader.take(4))[0]
        if tag == Tag.INT:
            return struct.unpack(">i", reader.take(4))[0]
        if tag == Tag.DOUBLE:
            return struct.unpack(">d", reader.take(8))[0]
        if tag == Tag.LONG:
            return struct.unpack(">q", reader.take(8))[0]
        if tag == Tag.VOID:
            return None
        raise ValueError(f"not a plain value tag: {tag!r}")

    def _read_values(self, reader: Reader) -> list:
        values = []
        for _ in range(reader.u32()):
            tag = Tag(reader.u8())
            if is_object_tag(tag):
                values.append(Value(tag=tag, value=reader.object_id()))
            else:
                values.append(Value(tag=tag, value=self._read_plain(reader, tag)))
        return values

    def _read_location(self, reader: Reader) -> Location:
        return Location(
            type_tag=reader.u8(),
            class_id=reader.reftype_id(),
            method_id=reader.method_id(),
            index=reader.u64(),
        )

    # -- Event command set ---------------------------------------------------

    def parse_composite_event(self, data: bytes) -> CompositeEvent:
        """Parse the body of an Event.Composite (64/100) command packet."""
        reader = self._reader(data)
        suspend_policy = reader.u8()
        events = []
        for _ in range(reader.u32()):
            kind = EventKind(reader.u8())
            request_id = reader.u32()
            if kind in (
                EventKind.SINGLE_STEP,
                EventKind.BREAKPOINT,
                EventKind.METHOD_ENTRY,
                EventKind.METHOD_EXIT,
                EventKind.FRAME_POP,
            ):
                events.append(
                    JdwpEvent(
                        kind=kind,
                        request_id=request_id,
                        thread_id=reader.object_id(),
                        location=self._read_location(reader),
                    )
                )
            elif kind in (EventKind.VM_INIT, EventKind.THREAD_START, EventKind.THREAD_END):
                events.append(
                    JdwpEvent(kind=kind, request_id=request_id, thread_id=reader.object_id())
                )
            elif kind == EventKind.CLASS_PREPARE:
                events.append(
                    JdwpEvent(
                        kind=kind,
                        request_id=request_id,
                        thread_id=reader.object_id(),
                        location=Location(
                            type_tag=reader.u8(),
                            class_id=reader.reftype_id(),
                            method_id=0,
                            index=0,
                        ),
                    )
                )
                reader.string()  # signature
                reader.u32()  # status
            elif kind == EventKind.CLASS_UNLOAD:
                reader.string()  # signature
                events.append(JdwpEvent(kind=kind, request_id=request_id))
            elif kind == EventKind.VM_DEATH:
                events.append(JdwpEvent(kind=kind, request_id=request_id))
            else:
                raise ValueError(f"unsupported composite event kind: {kind!r}")
        return CompositeEvent(suspend_policy=suspend_policy, events=tuple(events))

    # -- Transport pass-through ----------------------------------------------

    def recv_event_packet(self, timeout=None):
        return self._conn.recv_event(timeout)

    @property
    def closed(self) -> bool:
        return self._conn.closed

    def close(self) -> None:
        self._conn.close()
