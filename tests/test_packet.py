import struct
import unittest

from avmdbg.jdwp.ids import IdSizes
from avmdbg.jdwp.packet import Packet, Reader, Writer


class PacketCodecTest(unittest.TestCase):
    def test_command_round_trip(self):
        packet = Packet.command(1, 7, b"\xde\xad")
        raw = packet.encode()
        self.assertEqual(struct.unpack(">I", raw[:4])[0], 11 + 2)
        self.assertEqual(raw[8], 0x00)  # command flag
        decoded = Packet.decode(raw[:11], raw[11:])
        self.assertFalse(decoded.is_reply)
        self.assertEqual(decoded.id, packet.id)
        self.assertEqual(decoded.cmd_set, 1)
        self.assertEqual(decoded.cmd, 7)
        self.assertEqual(decoded.data, b"\xde\xad")

    def test_reply_round_trip(self):
        header = struct.pack(">IIBH", 11, 42, 0x80, 0)
        decoded = Packet.decode(header, b"")
        self.assertTrue(decoded.is_reply)
        self.assertEqual(decoded.id, 42)
        self.assertEqual(decoded.error, 0)

    def test_reply_with_error_code(self):
        header = struct.pack(">IIBH", 11, 7, 0x80, 35)
        decoded = Packet.decode(header, b"")
        self.assertEqual(decoded.error, 35)

    def test_header_only_command(self):
        packet = Packet.command(15, 3)
        self.assertEqual(len(packet.encode()), 11)

    def test_ids_increase(self):
        ids = {Packet.command(1, 1).id for _ in range(100)}
        self.assertEqual(len(ids), 100)

    def test_length_mismatch_rejected(self):
        header = struct.pack(">IIBH", 11, 1, 0x80, 0)
        with self.assertRaises(ValueError):
            Packet.decode(header, b"extra")


class WriterReaderTest(unittest.TestCase):
    def test_endianness(self):
        writer = Writer().u8(0x01).u16(0x0203).u32(0x04050607)
        writer.u64(0x08090A0B0C0D0E0F)
        reader = Reader(writer.build())
        self.assertEqual(reader.u8(), 0x01)
        self.assertEqual(reader.u16(), 0x0203)
        self.assertEqual(reader.u32(), 0x04050607)
        self.assertEqual(reader.u64(), 0x08090A0B0C0D0E0F)

    def test_string_round_trip(self):
        reader = Reader(Writer().string("hello").build())
        self.assertEqual(reader.string(), "hello")

    def test_string_utf8(self):
        reader = Reader(Writer().string("héllo").build())
        self.assertEqual(reader.string(), "héllo")

    def test_ident_sizes(self):
        writer = Writer().ident(0x1122, 4).ident(0x334455667788, 8)
        ids = IdSizes(field=4, method=4, object=8, reftype=4, frame=8)
        reader = Reader(writer.build(), ids)
        self.assertEqual(reader.reftype_id(), 0x1122)
        self.assertEqual(reader.object_id(), 0x334455667788)

    def test_truncated_reader_raises(self):
        reader = Reader(b"\x00")
        with self.assertRaises(ValueError):
            reader.u32()

    def test_boolean(self):
        self.assertTrue(Reader(b"\x01").boolean())
        self.assertFalse(Reader(b"\x00").boolean())


if __name__ == "__main__":
    unittest.main()
