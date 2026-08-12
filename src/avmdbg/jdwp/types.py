"""Dalvik type-signature parsing and register/slot analysis.

Dalvik is register-based: every register is 32 bits wide, 64-bit types
(long/double) occupy two adjacent registers, and a method's parameters are
placed in its last N registers. Locals use the v0..v(M-N-1) range.

Naming conventions used throughout the debugger:

- ``v`` naming covers every register, locals first.
- ``p`` naming covers only parameters; ``p0`` is ``this`` for non-static
  methods, then parameters follow in declaration order.

References:
- https://github.com/JesusFreke/smali/wiki/Registers
- https://github.com/JesusFreke/smali/wiki/TypesMethodsAndFields
"""

from dataclasses import dataclass

from .constants import OBJECT_TAGS, TAG_NAMES, Tag

_PRIMITIVE_TAGS = {
    "B": Tag.BYTE,
    "C": Tag.CHAR,
    "F": Tag.FLOAT,
    "D": Tag.DOUBLE,
    "I": Tag.INT,
    "J": Tag.LONG,
    "S": Tag.SHORT,
    "Z": Tag.BOOLEAN,
    "V": Tag.VOID,
}


@dataclass(frozen=True)
class ParamSlot:
    """A method parameter mapped to its Dalvik register slot."""

    name: str
    slot: int
    signature: str
    tag: Tag
    wide: bool


def tag_for_signature(signature: str) -> Tag:
    """Map a type signature (``I``, ``Ljava/lang/String;``, ``[B`` ...) to a tag."""
    if not signature:
        raise ValueError("empty type signature")
    head = signature[0]
    if head == "[":
        return Tag.ARRAY
    if head == "L":
        return Tag.OBJECT
    try:
        return _PRIMITIVE_TAGS[head]
    except KeyError:
        raise ValueError(f"invalid type signature: {signature!r}") from None


def register_width(tag: Tag) -> int:
    """Number of 32-bit Dalvik registers a value of this type occupies."""
    if tag in (Tag.LONG, Tag.DOUBLE):
        return 2
    return 1


def _type_end(signature: str, index: int) -> int:
    """Return the index one past the type starting at ``index`` (0 on error)."""
    if index >= len(signature):
        return 0
    head = signature[index]
    if head in _PRIMITIVE_TAGS:
        return index + 1
    if head == "L":
        end = signature.find(";", index)
        return end + 1 if end != -1 else 0
    if head == "[":
        i = index
        while i < len(signature) and signature[i] == "[":
            i += 1
        return _type_end(signature, i)
    return 0


def split_type_signatures(params: str) -> list:
    """Split a concatenated parameter signature like ``B[IJLjava/lang/String;``."""
    result = []
    index = 0
    while index < len(params):
        end = _type_end(params, index)
        if not end:
            raise ValueError(f"invalid parameter signature: {params!r}")
        result.append(params[index:end])
        index = end
    return result


def method_params_signature(method_signature: str) -> str:
    """Extract the parameter part of a method signature ``(...)ret``."""
    begin = method_signature.find("(")
    end = method_signature.find(")")
    if begin == -1 or end == -1 or begin > end:
        raise ValueError(f"invalid method signature: {method_signature!r}")
    return method_signature[begin + 1 : end]


def compute_param_slots(
    method_signature: str, register_count: int, is_static: bool
) -> list:
    """Map a method's parameters to Dalvik register slots.

    ``register_count`` is the ``.registers`` value from the smali method
    header. Raises ``ValueError`` when the parameters cannot fit.
    """
    params = split_type_signatures(method_params_signature(method_signature))
    total = sum(register_width(tag_for_signature(sig)) for sig in params)
    this_wide = 0 if is_static else 1
    if total + this_wide > register_count:
        raise ValueError(
            f"parameters need {total + this_wide} registers but method declares {register_count}"
        )

    slots = []
    cursor = register_count - total
    param_index = 0
    if not is_static:
        slots.append(
            ParamSlot(
                name="p0",
                slot=cursor - 1,
                signature="this",
                tag=Tag.OBJECT,
                wide=False,
            )
        )
        param_index += 1
    for sig in params:
        tag = tag_for_signature(sig)
        wide = register_width(tag) == 2
        slots.append(
            ParamSlot(
                name=f"p{param_index}",
                slot=cursor,
                signature=sig,
                tag=tag,
                wide=wide,
            )
        )
        cursor += register_width(tag)
        param_index += register_width(tag)
    return slots


def parse_register_name(name: str) -> int:
    """Resolve a register name to a raw slot number.

    ``v0`` is remapped to slot 1000: the Dalvik JDWP back end untweaks that
    sentinel back to slot 0, while a literal slot 0 is remapped to the first
    parameter slot (a workaround kept for Eclipse compatibility).
    """
    name = name.strip().lower()
    if not name or name[0] not in "vp":
        raise ValueError(f"invalid register name: {name!r}")
    slot = int(name[1:])
    if name[0] == "v" and slot == 0:
        return 1000
    return slot


def tag_from_letter(letter: str) -> Tag:
    """Map a single type letter (``I``, ``L``, ``[`` ...) to a Tag."""
    if len(letter) != 1:
        raise ValueError(f"invalid type letter: {letter!r}")
    if letter == "[":
        return Tag.ARRAY
    if letter == "s":
        return Tag.STRING
    return tag_for_signature(letter)


def tag_name(tag: Tag) -> str:
    return TAG_NAMES.get(tag, "unknown")


def is_object_tag(tag: Tag) -> bool:
    return tag in OBJECT_TAGS
