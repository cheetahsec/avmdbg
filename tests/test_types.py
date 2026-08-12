import unittest

from avmdbg.jdwp.constants import Tag
from avmdbg.jdwp.types import (
    compute_param_slots,
    method_params_signature,
    parse_register_name,
    register_width,
    split_type_signatures,
    tag_for_signature,
    tag_from_letter,
)


class SignatureParsingTest(unittest.TestCase):
    def test_primitives(self):
        self.assertEqual(split_type_signatures("ZBSIJFDC"), list("ZBSIJFDC"))

    def test_object(self):
        self.assertEqual(
            split_type_signatures("Ljava/lang/String;I"), ["Ljava/lang/String;", "I"]
        )

    def test_arrays(self):
        self.assertEqual(
            split_type_signatures("[I[[Ljava/lang/String;B"),
            ["[I", "[[Ljava/lang/String;", "B"],
        )

    def test_empty(self):
        self.assertEqual(split_type_signatures(""), [])

    def test_invalid(self):
        with self.assertRaises(ValueError):
            split_type_signatures("X")

    def test_unterminated_object(self):
        with self.assertRaises(ValueError):
            split_type_signatures("Ljava/lang/String")

    def test_method_params_signature(self):
        self.assertEqual(method_params_signature("(B[IJ)V"), "B[IJ")
        self.assertEqual(method_params_signature("()V"), "")

    def test_method_params_invalid(self):
        with self.assertRaises(ValueError):
            method_params_signature("no-parens")

    def test_tag_for_signature(self):
        self.assertEqual(tag_for_signature("I"), Tag.INT)
        self.assertEqual(tag_for_signature("[I"), Tag.ARRAY)
        self.assertEqual(tag_for_signature("Ljava/lang/String;"), Tag.OBJECT)

    def test_tag_from_letter(self):
        self.assertEqual(tag_from_letter("I"), Tag.INT)
        self.assertEqual(tag_from_letter("["), Tag.ARRAY)
        self.assertEqual(tag_from_letter("s"), Tag.STRING)
        with self.assertRaises(ValueError):
            tag_from_letter("II")

    def test_register_width(self):
        self.assertEqual(register_width(Tag.INT), 1)
        self.assertEqual(register_width(Tag.LONG), 2)
        self.assertEqual(register_width(Tag.DOUBLE), 2)


class ParamSlotTest(unittest.TestCase):
    def test_non_static_has_this(self):
        # .registers 9, params B [I J String -> slots 4,5,6,8; this -> 3
        slots = compute_param_slots("(B[IJLjava/lang/String;)V", 9, is_static=False)
        self.assertEqual([s.name for s in slots], ["p0", "p1", "p2", "p3", "p5"])
        self.assertEqual([s.slot for s in slots], [3, 4, 5, 6, 8])
        self.assertEqual(slots[0].signature, "this")
        self.assertTrue(slots[3].wide)  # the J parameter

    def test_static_has_no_this(self):
        slots = compute_param_slots("(B[IC[Ljava/lang/String;)V", 7, is_static=True)
        self.assertEqual([s.name for s in slots], ["p0", "p1", "p2", "p3"])
        self.assertEqual([s.slot for s in slots], [3, 4, 5, 6])

    def test_no_params_static(self):
        self.assertEqual(compute_param_slots("()V", 1, is_static=True), [])

    def test_no_params_instance(self):
        slots = compute_param_slots("()V", 1, is_static=False)
        self.assertEqual(len(slots), 1)
        self.assertEqual(slots[0].slot, 0)

    def test_overflow_rejected(self):
        with self.assertRaises(ValueError):
            compute_param_slots("(JJJ)V", 2, is_static=True)


class RegisterNameTest(unittest.TestCase):
    def test_p_name(self):
        self.assertEqual(parse_register_name("p2"), 2)

    def test_v_name(self):
        self.assertEqual(parse_register_name("v7"), 7)

    def test_v0_untweak_quirk(self):
        # Dalvik remaps the sentinel slot 1000 back to slot 0; a literal
        # slot 0 would be remapped to the first parameter slot instead.
        self.assertEqual(parse_register_name("v0"), 1000)

    def test_case_insensitive(self):
        self.assertEqual(parse_register_name("V3"), 3)

    def test_invalid(self):
        with self.assertRaises(ValueError):
            parse_register_name("x1")


if __name__ == "__main__":
    unittest.main()
