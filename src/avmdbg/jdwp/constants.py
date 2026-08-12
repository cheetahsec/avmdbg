"""JDWP protocol constants.

Values follow the JDWP specification and the Dalvik/ART implementation.
Reference: https://docs.oracle.com/javase/8/docs/technotes/guides/jpda/jdwp-spec.html
"""

from enum import IntEnum

HANDSHAKE = b"JDWP-Handshake"
HEADER_LEN = 11
REPLY_FLAG = 0x80
NO_ERROR = 0


class CommandSet(IntEnum):
    VIRTUAL_MACHINE = 1
    REFERENCE_TYPE = 2
    CLASS_TYPE = 3
    ARRAY_TYPE = 4
    INTERFACE_TYPE = 5
    METHOD = 6
    FIELD = 8
    OBJECT_REFERENCE = 9
    STRING_REFERENCE = 10
    THREAD_REFERENCE = 11
    THREAD_GROUP_REFERENCE = 12
    ARRAY_REFERENCE = 13
    CLASS_LOADER_REFERENCE = 14
    EVENT_REQUEST = 15
    STACK_FRAME = 16
    CLASS_OBJECT_REFERENCE = 17
    EVENT = 64


class VirtualMachineCmd(IntEnum):
    VERSION = 1
    CLASSES_BY_SIGNATURE = 2
    ALL_CLASSES = 3
    ALL_THREADS = 4
    TOP_LEVEL_THREAD_GROUPS = 5
    DISPOSE = 6
    ID_SIZES = 7
    SUSPEND = 8
    RESUME = 9
    EXIT = 10
    CREATE_STRING = 11
    CAPABILITIES = 12
    CLASS_PATHS = 13
    DISPOSE_OBJECTS = 14
    HOLD_EVENTS = 15
    RELEASE_EVENTS = 16
    CAPABILITIES_NEW = 17
    REDEFINE_CLASSES = 18
    SET_DEFAULT_STRATUM = 19
    ALL_CLASSES_WITH_GENERIC = 20
    INSTANCE_COUNTS = 21


class ReferenceTypeCmd(IntEnum):
    SIGNATURE = 1
    CLASS_LOADER = 2
    MODIFIERS = 3
    FIELDS = 4
    METHODS = 5
    GET_VALUES = 6
    SOURCE_FILE = 7
    NESTED_TYPES = 8
    STATUS = 9
    INTERFACES = 10
    CLASS_OBJECT = 11
    SOURCE_DEBUG_EXTENSION = 12
    SIGNATURE_WITH_GENERIC = 13
    FIELDS_WITH_GENERIC = 14
    METHODS_WITH_GENERIC = 15
    INSTANCES = 16
    CLASS_FILE_VERSION = 17
    CONSTANT_POOL = 18


class MethodCmd(IntEnum):
    LINE_TABLE = 1
    VARIABLE_TABLE = 2
    BYTECODES = 3
    OBSOLETE = 4
    VARIABLE_TABLE_WITH_GENERIC = 5


class ObjectReferenceCmd(IntEnum):
    REFERENCE_TYPE = 1
    GET_VALUES = 2
    SET_VALUES = 3
    MONITOR_INFO = 5
    INVOKE_METHOD = 6
    DISABLE_COLLECTION = 7
    ENABLE_COLLECTION = 8
    IS_COLLECTED = 9
    REFERRING_OBJECTS = 10


class StringReferenceCmd(IntEnum):
    VALUE = 1


class ThreadReferenceCmd(IntEnum):
    NAME = 1
    SUSPEND = 2
    RESUME = 3
    STATUS = 4
    THREAD_GROUP = 5
    FRAMES = 6
    FRAME_COUNT = 7
    OWNED_MONITORS = 8
    CURRENT_CONTENDED_MONITOR = 9
    STOP = 10
    INTERRUPT = 11
    SUSPEND_COUNT = 12
    OWNED_MONITORS_STACK_DEPTH = 13
    FORCE_EARLY_RETURN = 14


class ArrayReferenceCmd(IntEnum):
    LENGTH = 1
    GET_VALUES = 2
    SET_VALUES = 3


class EventRequestCmd(IntEnum):
    SET = 1
    CLEAR = 2
    CLEAR_ALL_BREAKPOINTS = 3


class StackFrameCmd(IntEnum):
    GET_VALUES = 1
    SET_VALUES = 2
    THIS_OBJECT = 3
    POP_FRAME = 4


class EventCmd(IntEnum):
    COMPOSITE = 100


class EventKind(IntEnum):
    SINGLE_STEP = 1
    BREAKPOINT = 2
    FRAME_POP = 3
    EXCEPTION = 4
    USER_DEFINED = 5
    THREAD_START = 6
    THREAD_END = 7
    CLASS_PREPARE = 8
    CLASS_UNLOAD = 9
    CLASS_LOAD = 10
    FIELD_ACCESS = 20
    FIELD_MODIFICATION = 21
    EXCEPTION_CATCH = 30
    METHOD_ENTRY = 40
    METHOD_EXIT = 41
    VM_INIT = 90
    VM_DEATH = 99
    VM_DISCONNECTED = 100


class ModKind(IntEnum):
    COUNT = 1
    CONDITIONAL = 2
    THREAD_ONLY = 3
    CLASS_ONLY = 4
    CLASS_MATCH = 5
    CLASS_EXCLUDE = 6
    LOCATION_ONLY = 7
    EXCEPTION_ONLY = 8
    FIELD_ONLY = 9
    STEP = 10
    INSTANCE_ONLY = 11
    SOURCE_NAME_MATCH = 12


class SuspendPolicy(IntEnum):
    NONE = 0
    EVENT_THREAD = 1
    ALL = 2


class TypeTag(IntEnum):
    CLASS = 1
    INTERFACE = 2
    ARRAY = 3


class Tag(IntEnum):
    """Value tags used by JDWP and by Dalvik/ART type signatures."""

    ARRAY = ord("[")
    BYTE = ord("B")
    CHAR = ord("C")
    OBJECT = ord("L")
    FLOAT = ord("F")
    DOUBLE = ord("D")
    INT = ord("I")
    LONG = ord("J")
    SHORT = ord("S")
    VOID = ord("V")
    BOOLEAN = ord("Z")
    STRING = ord("s")
    THREAD = ord("t")
    THREAD_GROUP = ord("g")
    CLASS_LOADER = ord("l")
    CLASS_OBJECT = ord("c")


TAG_NAMES = {
    Tag.ARRAY: "array",
    Tag.BYTE: "byte",
    Tag.CHAR: "char",
    Tag.OBJECT: "object",
    Tag.FLOAT: "float",
    Tag.DOUBLE: "double",
    Tag.INT: "int",
    Tag.LONG: "long",
    Tag.SHORT: "short",
    Tag.VOID: "void",
    Tag.BOOLEAN: "boolean",
    Tag.STRING: "string",
    Tag.THREAD: "thread",
    Tag.THREAD_GROUP: "threadGroup",
    Tag.CLASS_LOADER: "classLoader",
    Tag.CLASS_OBJECT: "classObject",
}

OBJECT_TAGS = frozenset(
    {
        Tag.ARRAY,
        Tag.OBJECT,
        Tag.STRING,
        Tag.THREAD,
        Tag.THREAD_GROUP,
        Tag.CLASS_LOADER,
        Tag.CLASS_OBJECT,
    }
)


class ClassStatus(IntEnum):
    VERIFIED = 0x01
    PREPARED = 0x02
    INITIALIZED = 0x04
    ERROR = 0x08


class ThreadStatus(IntEnum):
    ZOMBIE = 0
    RUNNING = 1
    SLEEPING = 2
    MONITOR = 3
    WAIT = 4


class SuspendStatus(IntEnum):
    NOT_SUSPENDED = 0
    SUSPENDED = 1


class StepDepth(IntEnum):
    INTO = 0
    OVER = 1
    OUT = 2


class StepSize(IntEnum):
    MIN = 0
    LINE = 1


# Access flags (Dalvik/ART modifiers).
ACC_PUBLIC = 0x0001
ACC_PRIVATE = 0x0002
ACC_PROTECTED = 0x0004
ACC_STATIC = 0x0008
ACC_FINAL = 0x0010
ACC_SYNCHRONIZED = 0x0020
ACC_VOLATILE = 0x0040
ACC_TRANSIENT = 0x0080
ACC_NATIVE = 0x0100
ACC_INTERFACE = 0x0200
ACC_ABSTRACT = 0x0400
ACC_STRICT = 0x0800
ACC_SYNTHETIC = 0x1000
ACC_ANNOTATION = 0x2000
ACC_ENUM = 0x4000
ACC_CONSTRUCTOR = 0x10000
ACC_DECLARED_SYNCHRONIZED = 0x20000

ERROR_NAMES = {
    0: "NONE",
    10: "INVALID_THREAD",
    11: "INVALID_THREAD_GROUP",
    12: "INVALID_PRIORITY",
    13: "THREAD_NOT_SUSPENDED",
    14: "THREAD_SUSPENDED",
    20: "INVALID_OBJECT",
    21: "INVALID_CLASS",
    22: "CLASS_NOT_PREPARED",
    23: "INVALID_METHODID",
    24: "INVALID_LOCATION",
    25: "INVALID_FIELDID",
    30: "INVALID_FRAMEID",
    31: "NO_MORE_FRAMES",
    32: "OPAQUE_FRAME",
    33: "NOT_CURRENT_FRAME",
    34: "TYPE_MISMATCH",
    35: "INVALID_SLOT",
    40: "DUPLICATE",
    41: "NOT_FOUND",
    50: "INVALID_MONITOR",
    51: "NOT_MONITOR_OWNER",
    52: "INTERRUPT",
    60: "INVALID_CLASS_FORMAT",
    61: "CIRCULAR_CLASS_DEFINITION",
    62: "FAILS_VERIFICATION",
    63: "ADD_METHOD_NOT_IMPLEMENTED",
    64: "SCHEMA_CHANGE_NOT_IMPLEMENTED",
    65: "INVALID_TYPESTATE",
    66: "HIERARCHY_CHANGE_NOT_IMPLEMENTED",
    67: "DELETE_METHOD_NOT_IMPLEMENTED",
    68: "UNSUPPORTED_VERSION",
    69: "NAMES_DONT_MATCH",
    70: "CLASS_MODIFIERS_CHANGE_NOT_IMPLEMENTED",
    71: "METHOD_MODIFIERS_CHANGE_NOT_IMPLEMENTED",
    99: "NOT_IMPLEMENTED",
    100: "NULL_POINTER",
    101: "ABSENT_INFORMATION",
    102: "INVALID_EVENT_TYPE",
    103: "ILLEGAL_ARGUMENT",
    110: "OUT_OF_MEMORY",
    111: "ACCESS_DENIED",
    112: "VM_DEAD",
    113: "INTERNAL",
    115: "UNATTACHED_THREAD",
    500: "INVALID_TAG",
    502: "ALREADY_INVOKING",
    503: "INVALID_INDEX",
    504: "INVALID_LENGTH",
    506: "INVALID_STRING",
    507: "INVALID_CLASS_LOADER",
    508: "INVALID_ARRAY",
    509: "TRANSPORT_LOAD",
    510: "TRANSPORT_INIT",
    511: "NATIVE_METHOD",
    512: "INVALID_COUNT",
}
