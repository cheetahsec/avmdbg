"""High-level debugging session built on the JDWP client.

Typical usage::

    from avmdbg import Debugger

    dbg = Debugger.attach("com.example.app")
    bp = dbg.set_breakpoint(
        class_signature="Lcom/example/app/LoginActivity;",
        method="checkLogin",
        signature="(Ljava/lang/String;)Z",
        index=0x12,
        registers=6,
    )
    hit = dbg.wait_event(timeout=60)
    print(hit)
    dbg.resume()
    dbg.close()
"""

import time
from dataclasses import dataclass, field

from . import adb
from .jdwp.connection import JdwpConnection
from .jdwp.constants import ACC_STATIC, EventKind, SuspendPolicy, Tag
from .jdwp.errors import AttachError, AvmdbgError, JdwpError
from .jdwp.protocol import JdwpClient
from .jdwp.types import (
    compute_param_slots,
    is_object_tag,
    parse_register_name,
    tag_from_letter,
    tag_name,
)

DEFAULT_PORT = 8819
DEFAULT_TIMEOUT = 10.0
MAX_ARRAY_PREVIEW = 64


@dataclass
class Breakpoint:
    """A location breakpoint registered on the target VM."""

    request_id: int
    class_signature: str
    method: str
    signature: str
    index: int
    registers: int
    mod_bits: int
    class_id: int
    method_id: int

    @property
    def is_static(self) -> bool:
        return bool(self.mod_bits & ACC_STATIC)

    def to_dict(self) -> dict:
        return {
            "request_id": self.request_id,
            "class": self.class_signature,
            "method": self.method,
            "signature": self.signature,
            "index": self.index,
            "registers": self.registers,
            "static": self.is_static,
        }


@dataclass
class EventContext:
    """A breakpoint hit: thread, location, decoded parameters and stack."""

    breakpoint: Breakpoint
    thread_id: int
    thread_name: str
    frame_id: int
    location_index: int
    params: dict = field(default_factory=dict)
    stack: list = field(default_factory=list)

    def to_dict(self) -> dict:
        return {
            "breakpoint": self.breakpoint.to_dict(),
            "thread": {"id": self.thread_id, "name": self.thread_name},
            "frame_id": self.frame_id,
            "index": self.location_index,
            "params": self.params,
            "stack": self.stack,
        }


class Debugger:
    """A debugging session attached to one Android process."""

    def __init__(self, client: JdwpClient, process: adb.Process, port: int, serial=None):
        self.client = client
        self.process = process
        self.port = port
        self.serial = serial
        self._breakpoints = {}

    # -- attach / detach ---------------------------------------------------

    @classmethod
    def attach(
        cls,
        process_name: str,
        serial=None,
        port: int = DEFAULT_PORT,
        timeout: float = DEFAULT_TIMEOUT,
    ) -> "Debugger":
        """Attach to a running debuggable process by its package name.

        Sets up ``adb forward tcp:<port> jdwp:<pid>``, connects and performs
        the JDWP handshake.
        """
        candidates = [p for p in adb.debuggable_processes(serial) if p.name == process_name]
        if not candidates:
            raise AttachError(
                f"no debuggable process named {process_name!r}; "
                "is the app running and built as debuggable?"
            )
        process = candidates[0]
        adb.forward_jdwp(port, process.pid, serial)
        try:
            connection = JdwpConnection("127.0.0.1", port, timeout=timeout)
            client = JdwpClient(connection)
        except Exception:
            adb.forward_remove(port, serial)
            raise
        return cls(client=client, process=process, port=port, serial=serial)

    @classmethod
    def attach_pid(
        cls, pid: int, serial=None, port: int = DEFAULT_PORT, timeout: float = DEFAULT_TIMEOUT
    ) -> "Debugger":
        """Attach to a process by PID (must currently advertise a JDWP endpoint)."""
        names = adb.process_map(serial)
        if pid not in adb.jdwp_pids(serial):
            raise AttachError(f"pid {pid} has no JDWP endpoint (not debuggable or not running)")
        process = adb.Process(pid=pid, name=names.get(pid, ""))
        adb.forward_jdwp(port, pid, serial)
        try:
            connection = JdwpConnection("127.0.0.1", port, timeout=timeout)
            client = JdwpClient(connection)
        except Exception:
            adb.forward_remove(port, serial)
            raise
        return cls(client=client, process=process, port=port, serial=serial)

    def close(self) -> None:
        """Detach from the VM (the target process keeps running)."""
        try:
            self.client.dispose()
        finally:
            self.client.close()
            adb.forward_remove(self.port, self.serial)

    def __enter__(self) -> "Debugger":
        return self

    def __exit__(self, *exc_info) -> None:
        self.close()

    # -- VM information ------------------------------------------------------

    def version(self) -> dict:
        return self.client.version()

    def list_classes(self, match=None) -> list:
        """All loaded classes, optionally filtered by a substring."""
        classes = self.client.all_classes_with_generic()
        result = [
            {"signature": c.signature, "status": c.status, "type_tag": c.type_tag}
            for c in classes
        ]
        if match:
            needle = match.lower()
            result = [c for c in result if needle in c["signature"].lower()]
        return result

    def find_class_methods(self, class_signature: str) -> list:
        """Declared methods of a loaded class."""
        classes = self.client.classes_by_signature(class_signature)
        if not classes:
            raise AvmdbgError(f"class not loaded in target VM: {class_signature}")
        methods = self.client.reftype_methods_with_generic(classes[0].type_id)
        return [
            {
                "name": m.name,
                "signature": m.signature,
                "mod_bits": m.mod_bits,
                "static": bool(m.mod_bits & ACC_STATIC),
            }
            for m in methods
        ]

    # -- Breakpoints -----------------------------------------------------------

    def set_breakpoint(
        self,
        class_signature: str,
        method: str,
        signature: str,
        index: int = 0,
        registers: int = 0,
        suspend_policy: SuspendPolicy = SuspendPolicy.ALL,
    ) -> Breakpoint:
        """Set a breakpoint at a dex code offset inside one method.

        ``index`` is the code offset shown by baksmali (``baksmali -l``).
        ``registers`` is the method's ``.registers`` value from its smali
        header and is required to decode parameter values when the breakpoint
        is hit.
        """
        classes = self.client.classes_by_signature(class_signature)
        if not classes:
            raise AvmdbgError(f"class not loaded in target VM: {class_signature}")
        class_id = classes[0].type_id
        methods = self.client.reftype_methods_with_generic(class_id)
        match = next(
            (m for m in methods if m.name == method and m.signature == signature), None
        )
        if match is None:
            raise AvmdbgError(
                f"method {method}{signature} not found in {class_signature}"
            )
        request_id = self.client.set_breakpoint_request(
            class_id, match.method_id, index, suspend_policy
        )
        bp = Breakpoint(
            request_id=request_id,
            class_signature=class_signature,
            method=method,
            signature=signature,
            index=index,
            registers=registers,
            mod_bits=match.mod_bits,
            class_id=class_id,
            method_id=match.method_id,
        )
        self._breakpoints[request_id] = bp
        return bp

    def clear_breakpoint(self, bp: Breakpoint) -> None:
        self.client.clear_event_request(EventKind.BREAKPOINT, bp.request_id)
        self._breakpoints.pop(bp.request_id, None)

    def clear_all_breakpoints(self) -> None:
        self.client.clear_all_breakpoints()
        self._breakpoints.clear()

    @property
    def breakpoints(self) -> list:
        return list(self._breakpoints.values())

    # -- Events -----------------------------------------------------------------

    def wait_event(
        self,
        timeout=None,
        include_stack: bool = True,
        include_params: bool = True,
    ) -> EventContext:
        """Wait for the next breakpoint hit and return its context.

        The VM stays suspended afterwards; call :meth:`resume` to continue
        execution. Returns ``None`` on timeout.
        """
        deadline = None if timeout is None else time.monotonic() + timeout
        while True:
            wait = None if deadline is None else max(0.0, deadline - time.monotonic())
            packet = self.client.recv_event_packet(timeout=wait)
            if packet is None:
                return None
            composite = self.client.parse_composite_event(packet.data)
            for event in composite.events:
                if event.kind != EventKind.BREAKPOINT:
                    continue
                bp = self._breakpoints.get(event.request_id)
                if bp is None:
                    continue
                return self._build_context(
                    bp, event.thread_id, include_stack, include_params
                )

    def _build_context(
        self, bp: Breakpoint, thread_id: int, include_stack: bool, include_params: bool
    ) -> EventContext:
        thread_name = ""
        try:
            thread_name = self.client.thread_name(thread_id)
        except JdwpError:
            pass

        frames = self.client.thread_frames(thread_id)
        if not frames:
            raise AvmdbgError("breakpoint thread has no frames")
        top = frames[0]

        params = {}
        if include_params and bp.registers:
            slots = compute_param_slots(bp.signature, bp.registers, bp.is_static)
            slot_pairs = [(p.slot, p.tag) for p in slots]
            values = self.client.frame_get_values(thread_id, top.frame_id, slot_pairs)
            for param, value in zip(slots, values):
                params[param.name] = self._format_value(
                    value.tag, value.value, slot=param.slot, signature=param.signature
                )

        stack = []
        if include_stack:
            stack = self.stack_frames(thread_id)

        return EventContext(
            breakpoint=bp,
            thread_id=thread_id,
            thread_name=thread_name,
            frame_id=top.frame_id,
            location_index=top.location.index,
            params=params,
            stack=stack,
        )

    def resume(self) -> None:
        """Resume all threads after a suspending event."""
        self.client.resume_vm()

    # -- Inspection ---------------------------------------------------------

    def stack_frames(self, thread_id: int) -> list:
        """Call stack of a suspended thread, top frame first."""
        frames = self.client.thread_frames(thread_id)
        result = []
        for frame in frames:
            signature, _ = self.client.reftype_signature_with_generic(
                frame.location.class_id
            )
            method_name, method_sign = "", ""
            for m in self.client.reftype_methods_with_generic(frame.location.class_id):
                if m.method_id == frame.location.method_id:
                    method_name, method_sign = m.name, m.signature
                    break
            result.append(
                {
                    "frame_id": frame.frame_id,
                    "class": signature,
                    "method": method_name,
                    "signature": method_sign,
                    "index": frame.location.index,
                }
            )
        return result

    def get_register(self, thread_id: int, frame_id: int, name: str, tag_letter: str):
        """Read one register (``v``/``p`` naming) of a suspended frame.

        ``tag_letter`` is the single-character type of the register content,
        e.g. ``I`` (int), ``L`` (object), ``[`` (array), ``J`` (long).
        """
        slot = parse_register_name(name)
        tag = tag_from_letter(tag_letter)
        values = self.client.frame_get_values(thread_id, frame_id, [(slot, tag)])
        if not values:
            return None
        value = values[0]
        return self._format_value(value.tag, value.value, slot=slot)

    def get_string(self, object_id: int) -> str:
        return self.client.string_value(object_id)

    def object_fields(self, object_id: int) -> list:
        """Static and instance field values of an object."""
        _, type_id = self.client.object_reftype(object_id)
        fields = self.client.reftype_fields_with_generic(type_id)
        static_fields = [f for f in fields if f.mod_bits & ACC_STATIC]
        instance_fields = [f for f in fields if not f.mod_bits & ACC_STATIC]

        result = []
        if static_fields:
            values = self.client.reftype_get_values(
                type_id, [f.field_id for f in static_fields]
            )
            for f, v in zip(static_fields, values):
                result.append(self._format_field(f, v, "static"))
        if instance_fields:
            values = self.client.object_get_values(
                object_id, [f.field_id for f in instance_fields]
            )
            for f, v in zip(instance_fields, values):
                result.append(self._format_field(f, v, "instance"))
        return result

    def array_values(self, object_id: int, offset: int = 0, length=None) -> dict:
        """Array element values (bounded reads)."""
        total = self.client.array_length(object_id)
        if length is None:
            length = min(total - offset, MAX_ARRAY_PREVIEW)
        tag, values = self.client.array_values(object_id, offset, length)
        return {
            "id": object_id,
            "type": tag_name(tag),
            "length": total,
            "offset": offset,
            "data": [self._format_value(v.tag, v.value) for v in values],
        }

    # -- Value formatting ------------------------------------------------------

    def _format_field(self, f, value, prop) -> dict:
        item = {
            "name": f.name,
            "signature": f.signature,
            "property": prop,
        }
        item.update(self._format_value(value.tag, value.value))
        return item

    def _format_value(self, tag: Tag, raw, slot=None, signature=None) -> dict:
        item = {"type": tag_name(tag)}
        if slot is not None:
            item["slot"] = slot
        if signature is not None:
            item["signature"] = signature
        if tag == Tag.STRING:
            item["id"] = raw
            try:
                item["value"] = self.client.string_value(raw)
            except JdwpError:
                item["value"] = None
        elif tag == Tag.ARRAY:
            item["id"] = raw
            try:
                length = self.client.array_length(raw)
                preview = min(length, MAX_ARRAY_PREVIEW)
                _, values = self.client.array_values(raw, 0, preview)
                item["length"] = length
                item["data"] = [
                    self._format_value(v.tag, v.value) for v in values
                ]
            except JdwpError:
                item["length"] = None
                item["data"] = []
        elif is_object_tag(tag):
            item["id"] = raw
        else:
            item["value"] = raw
        return item
