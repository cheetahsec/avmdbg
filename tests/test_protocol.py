import struct
import unittest

from avmdbg.jdwp.constants import EventKind, SuspendPolicy, Tag
from avmdbg.jdwp.ids import IdSizes
from avmdbg.jdwp.packet import Writer
from avmdbg.jdwp.protocol import JdwpClient


class FakeConnection:
    """Replies to every command with a canned payload."""

    def __init__(self, reply: bytes):
        self.reply = reply
        self.sent = []

    def send_command(self, cmd_set, cmd, data=b"", timeout=None):
        self.sent.append((cmd_set, cmd, data))
        return self.reply

    def recv_event(self, timeout=None):
        return None

    @property
    def closed(self):
        return False

    def close(self):
        pass


def make_client(reply: bytes, id_sizes: IdSizes = None) -> JdwpClient:
    client = object.__new__(JdwpClient)
    client._conn = FakeConnection(reply)
    client.id_sizes = id_sizes or IdSizes()
    return client


class VersionTest(unittest.TestCase):
    def test_parse(self):
        payload = (
            Writer()
            .string("ART 2.1.0")
            .u32(1)
            .u32(8)
            .string("13.0")
            .string("Dalvik")
            .build()
        )
        info = make_client(payload).version()
        self.assertEqual(info["description"], "ART 2.1.0")
        self.assertEqual(info["jdwp_major"], 1)
        self.assertEqual(info["vm_name"], "Dalvik")


class ClassesBySignatureTest(unittest.TestCase):
    def test_parse(self):
        payload = (
            Writer()
            .u32(2)
            .u8(1)
            .ident(1001, 8)
            .u32(7)
            .u8(1)
            .ident(1002, 8)
            .u32(7)
            .build()
        )
        classes = make_client(payload).classes_by_signature("Lcom/example/A;")
        self.assertEqual(len(classes), 2)
        self.assertEqual(classes[0].type_id, 1001)
        self.assertEqual(classes[1].type_id, 1002)

    def test_request_body(self):
        client = make_client(Writer().u32(0).build())
        client.classes_by_signature("Lcom/example/A;")
        _, _, data = client._conn.sent[0]
        self.assertEqual(data, Writer().string("Lcom/example/A;").build())


class MethodsWithGenericTest(unittest.TestCase):
    def test_parse(self):
        payload = (
            Writer()
            .u32(1)
            .ident(501, 4)
            .string("checkLogin")
            .string("(Ljava/lang/String;)Z")
            .string("")
            .u32(0x0001)
            .build()
        )
        methods = make_client(payload).reftype_methods_with_generic(1001)
        self.assertEqual(methods[0].method_id, 501)
        self.assertEqual(methods[0].name, "checkLogin")
        self.assertEqual(methods[0].mod_bits, 0x0001)


class FrameGetValuesTest(unittest.TestCase):
    def test_mixed_tags(self):
        payload = (
            Writer()
            .u32(3)
            .u8(Tag.INT)
            .build()
            + struct.pack(">i", -5)
            + Writer().u8(Tag.STRING).ident(7777, 8).u8(Tag.LONG).build()
            + struct.pack(">q", 2**40)
        )
        client = make_client(payload)
        values = client.frame_get_values(100, 200, [(3, Tag.OBJECT), (4, Tag.STRING), (5, Tag.LONG)])
        self.assertEqual(values[0].tag, Tag.INT)
        self.assertEqual(values[0].value, -5)
        self.assertEqual(values[1].tag, Tag.STRING)
        self.assertEqual(values[1].value, 7777)
        self.assertEqual(values[2].value, 2**40)

    def test_signed_decoding(self):
        payload = Writer().u32(1).u8(Tag.BYTE).build() + struct.pack(">b", -1)
        values = make_client(payload).frame_get_values(1, 2, [(0, Tag.BYTE)])
        self.assertEqual(values[0].value, -1)

    def test_request_body_layout(self):
        client = make_client(Writer().u32(0).build())
        client.frame_get_values(100, 200, [(3, Tag.INT)])
        _, _, data = client._conn.sent[0]
        expected = Writer().ident(100, 8).ident(200, 8).u32(1).u32(3).u8(Tag.INT).build()
        self.assertEqual(data, expected)


class ArrayValuesTest(unittest.TestCase):
    def test_primitive_array(self):
        payload = Writer().u8(Tag.INT).u32(3).build() + struct.pack(">iii", 1, 2, 3)
        tag, values = make_client(payload).array_values(55, 0, 3)
        self.assertEqual(tag, Tag.INT)
        self.assertEqual([v.value for v in values], [1, 2, 3])

    def test_object_array_elements_are_tagged(self):
        payload = (
            Writer()
            .u8(Tag.OBJECT)
            .u32(2)
            .u8(Tag.STRING)
            .ident(11, 8)
            .u8(Tag.OBJECT)
            .ident(12, 8)
            .build()
        )
        _tag, values = make_client(payload).array_values(55, 0, 2)
        self.assertEqual(values[0].tag, Tag.STRING)
        self.assertEqual(values[0].value, 11)
        self.assertEqual(values[1].tag, Tag.OBJECT)
        self.assertEqual(values[1].value, 12)


class SetBreakpointTest(unittest.TestCase):
    def test_request_layout(self):
        payload = Writer().u32(4242).build()
        client = make_client(payload)
        request_id = client.set_breakpoint_request(1001, 501, 0x10, SuspendPolicy.ALL)
        self.assertEqual(request_id, 4242)
        _, _, data = client._conn.sent[0]
        expected = (
            Writer()
            .u8(EventKind.BREAKPOINT)
            .u8(SuspendPolicy.ALL)
            .u32(1)
            .u8(7)  # ModKind.LOCATION_ONLY
            .u8(1)  # TypeTag.CLASS
            .ident(1001, 8)
            .ident(501, 4)
            .u64(0x10)
            .build()
        )
        self.assertEqual(data, expected)


class CompositeEventTest(unittest.TestCase):
    def test_breakpoint_event(self):
        payload = (
            Writer()
            .u8(SuspendPolicy.ALL)
            .u32(1)
            .u8(EventKind.BREAKPOINT)
            .u32(4242)
            .ident(999, 8)
            .u8(1)
            .ident(1001, 8)
            .ident(501, 4)
            .u64(0x10)
            .build()
        )
        composite = make_client(b"").parse_composite_event(payload)
        self.assertEqual(composite.suspend_policy, SuspendPolicy.ALL)
        self.assertEqual(len(composite.events), 1)
        event = composite.events[0]
        self.assertEqual(event.kind, EventKind.BREAKPOINT)
        self.assertEqual(event.request_id, 4242)
        self.assertEqual(event.thread_id, 999)
        self.assertEqual(event.location.class_id, 1001)
        self.assertEqual(event.location.method_id, 501)
        self.assertEqual(event.location.index, 0x10)

    def test_vm_death(self):
        payload = (
            Writer()
            .u8(SuspendPolicy.NONE)
            .u32(1)
            .u8(EventKind.VM_DEATH)
            .u32(1)
            .build()
        )
        composite = make_client(b"").parse_composite_event(payload)
        self.assertEqual(composite.events[0].kind, EventKind.VM_DEATH)


if __name__ == "__main__":
    unittest.main()
