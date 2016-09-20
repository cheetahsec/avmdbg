#pragma once

#include <string>
#include <list>
using namespace std;

#define Tranverse16(s)   (((s & 0x00FF) << 8) | ((s & 0xFF00) >> 8))
#define Tranverse32(i)   (((i & 0x000000FF) << 24) | ((i & 0x0000FF00) << 8) | \
                          ((i & 0x00FF0000) >> 8) | ((i & 0xFF000000) >> 24))
#define Tranverse64(l)   ((((uint64_t)(l) & 0xff00000000000000) >> 56) | \
						  (((uint64_t)(l) & 0x00ff000000000000) >> 40) | \
						  (((uint64_t)(l) & 0x0000ff0000000000) >> 24) | \
						  (((uint64_t)(l) & 0x000000ff00000000) >> 8)  | \
						  (((uint64_t)(l) & 0x00000000ff000000) << 8)  | \
						  (((uint64_t)(l) & 0x0000000000ff0000) <<24)  | \
						  (((uint64_t)(l) & 0x000000000000ff00) <<40)  | \
						  (((uint64_t)(l) & 0x00000000000000ff) <<56))

/*
* These match the definitions in the VM specification.
*/
typedef    uint8_t     u1;
typedef    uint16_t    u2;
typedef    uint32_t    u4;
typedef    uint64_t    u8;
typedef    int8_t      s1;
typedef    int16_t     s2;
typedef    int32_t     s4;
typedef    int64_t     s8;

/*
* Fundamental types.
*
* ObjectId and RefTypeId must be the same size.
*/
typedef u4 FieldId;     /* static or instance field */
typedef u4 MethodId;    /* any kind of method, including constructors */
typedef u8 ObjectId;    /* any object (threadID, stringID, arrayID, etc) */
typedef u8 RefTypeId;   /* like ObjectID, but unique for Class objects */
typedef u8 FrameId;     /* short-lived stack frame ID */

/*
* Error constants.
*/
enum eJdwpError {
	ERR_NONE = 0,
	ERR_INVALID_THREAD = 10,
	ERR_INVALID_THREAD_GROUP = 11,
	ERR_INVALID_PRIORITY = 12,
	ERR_THREAD_NOT_SUSPENDED = 13,
	ERR_THREAD_SUSPENDED = 14,
	ERR_INVALID_OBJECT = 20,
	ERR_INVALID_CLASS = 21,
	ERR_CLASS_NOT_PREPARED = 22,
	ERR_INVALID_METHODID = 23,
	ERR_INVALID_LOCATION = 24,
	ERR_INVALID_FIELDID = 25,
	ERR_INVALID_FRAMEID = 30,
	ERR_NO_MORE_FRAMES = 31,
	ERR_OPAQUE_FRAME = 32,
	ERR_NOT_CURRENT_FRAME = 33,
	ERR_TYPE_MISMATCH = 34,
	ERR_INVALID_SLOT = 35,
	ERR_DUPLICATE = 40,
	ERR_NOT_FOUND = 41,
	ERR_INVALID_MONITOR = 50,
	ERR_NOT_MONITOR_OWNER = 51,
	ERR_INTERRUPT = 52,
	ERR_INVALID_CLASS_FORMAT = 60,
	ERR_CIRCULAR_CLASS_DEFINITION = 61,
	ERR_FAILS_VERIFICATION = 62,
	ERR_ADD_METHOD_NOT_IMPLEMENTED = 63,
	ERR_SCHEMA_CHANGE_NOT_IMPLEMENTED = 64,
	ERR_INVALID_TYPESTATE = 65,
	ERR_HIERARCHY_CHANGE_NOT_IMPLEMENTED = 66,
	ERR_DELETE_METHOD_NOT_IMPLEMENTED = 67,
	ERR_UNSUPPORTED_VERSION = 68,
	ERR_NAMES_DONT_MATCH = 69,
	ERR_CLASS_MODIFIERS_CHANGE_NOT_IMPLEMENTED = 70,
	ERR_METHOD_MODIFIERS_CHANGE_NOT_IMPLEMENTED = 71,
	ERR_NOT_IMPLEMENTED = 99,
	ERR_NULL_POINTER = 100,
	ERR_ABSENT_INFORMATION = 101,
	ERR_INVALID_EVENT_TYPE = 102,
	ERR_ILLEGAL_ARGUMENT = 103,
	ERR_OUT_OF_MEMORY = 110,
	ERR_ACCESS_DENIED = 111,
	ERR_VM_DEAD = 112,
	ERR_INTERNAL = 113,
	ERR_UNATTACHED_THREAD = 115,
	ERR_INVALID_TAG = 500,
	ERR_ALREADY_INVOKING = 502,
	ERR_INVALID_INDEX = 503,
	ERR_INVALID_LENGTH = 504,
	ERR_INVALID_STRING = 506,
	ERR_INVALID_CLASS_LOADER = 507,
	ERR_INVALID_ARRAY = 508,
	ERR_TRANSPORT_LOAD = 509,
	ERR_TRANSPORT_INIT = 510,
	ERR_NATIVE_METHOD = 511,
	ERR_INVALID_COUNT = 512,
};

/*
* ClassStatus constants.  These are bit flags that can be ORed together.
*/
enum eJdwpClassStatus {
	CS_VERIFIED = 0x01,
	CS_PREPARED = 0x02,
	CS_INITIALIZED = 0x04,
	CS_ERROR = 0x08,
};

enum eJdwpPacketType {
	PACKET_TYPE_UNKNOWN = 0,
	PACKET_TYPE_COMMAND = 1,
	PACKET_TYPE_REPLY = 2
};

/* Command Sets */
enum eJdwpCommandSet {
	SET_VIRTUAL_MACHINE = 1,
	SET_REFERENCE_TYPE = 2,
	SET_CLASS_TYPE = 3,
	SET_ARRAY_TYPE = 4,
	SET_INTERFACE_TYPE = 5,
	SET_METHOD = 6,
	SET_FIELD = 8,
	SET_OBJECT_REFERENCE = 9,
	SET_STRING_REFERENCE = 10,
	SET_THREAD_REFERENCE = 11,
	SET_THREAD_GROUP_REFERENCE = 12,
	SET_ARRAY_REFERENCE = 13,
	SET_CLASS_LOADER_REFERENCE = 14,
	SET_EVENT_REQUEST = 15,
	SET_STACK_FRAME = 16,
	SET_CLASS_OBJECT_REFERENCE = 17,
	SET_EVENT = 64
};

enum eJdwpCommandId {
	/* Commands VirtualMachine */
	ID_VM_VERSION = 1,
	ID_VM_CLASSES_BY_SIGNATURE = 2,
	ID_VM_ALL_CLASSES = 3,
	ID_VM_ALL_THREADS = 4,
	ID_VM_TOP_LEVEL_THREAD_GROUPS = 5,
	ID_VM_DISPOSE = 6,
	ID_VM_ID_SIZES = 7,
	ID_VM_SUSPEND = 8,
	ID_VM_RESUME = 9,
	ID_VM_EXIT = 10,
	ID_VM_CREATE_STRING = 11,
	ID_VM_CAPABILITIES = 12,
	ID_VM_CLASS_PATHS = 13,
	ID_VM_DISPOSE_OBJECTS = 14,
	ID_VM_HOLD_EVENTS = 15,
	ID_VM_RELEASE_EVENTS = 16,
	ID_VM_CAPABILITIES_NEW = 17,
	ID_VM_REDEFINE_CLASSES = 18,
	ID_VM_SET_DEFAULT_STRATUM = 19,
	ID_VM_ALL_CLASSES_WITH_GENERIC = 20,
	ID_VM_INSTANCE_COUNTS = 21,

	/* Commands ReferenceType */
	ID_RT_SIGNATURE = 1,
	ID_RT_CLASS_LOADER = 2,
	ID_RT_MODIFIERS = 3,
	ID_RT_FIELDS = 4,
	ID_RT_METHODS = 5,
	ID_RT_GET_VALUES = 6,
	ID_RT_SOURCE_FILE = 7,
	ID_RT_NESTED_TYPES = 8,
	ID_RT_STATUS = 9,
	ID_RT_INTERFACES = 10,
	ID_RT_CLASS_OBJECT = 11,
	ID_RT_SOURCE_DEBUG_EXTENSION = 12,
	ID_RT_SIGNATURE_WITH_GENERIC = 13,
	ID_RT_FIELDS_WITH_GENERIC = 14,
	ID_RT_METHODS_WITH_GENERIC = 15,
	ID_RT_INSTANCES = 16,
	ID_RT_CLASS_FILE_VERSION = 17,
	ID_RT_CONSTANT_POOL = 18,

	/* Commands ClassType */
	ID_CT_SUPERCLASS = 1,
	ID_CT_SET_VALUES = 2,
	ID_CT_INVOKE_METHOD = 3,
	ID_CT_NEW_INSTANCE = 4,

	/* Commands ArrayType */
	ID_AT_NEW_INSTANCE = 1,

	/* Commands Method */
	ID_M_LINE_TABLE = 1,
	ID_M_VARIABLE_TABLE = 2,
	ID_M_BYTECODES = 3,
	ID_M_OBSOLETE = 4,
	ID_M_VARIABLE_TABLE_WITH_GENERIC = 5,

	/* Commands ObjectReference */
	ID_OR_REFERENCE_TYPE = 1,
	ID_OR_GET_VALUES = 2,
	ID_OR_SET_VALUES = 3,
	ID_OR_MONITOR_INFO = 5,
	ID_OR_INVOKE_METHOD = 6,
	ID_OR_DISABLE_COLLECTION = 7,
	ID_OR_ENABLE_COLLECTION = 8,
	ID_OR_IS_COLLECTED = 9,
	ID_OR_REFERRING_OBJECTS = 10,

	/* Commands StringReference */
	ID_SR_VALUE = 1,

	/* Commands ThreadReference */
	ID_TR_NAME = 1,
	ID_TR_SUSPEND = 2,
	ID_TR_RESUME = 3,
	ID_TR_STATUS = 4,
	ID_TR_THREAD_GROUP = 5,
	ID_TR_FRAMES = 6,
	ID_TR_FRAME_COUNT = 7,
	ID_TR_OWNED_MONITORS = 8,
	ID_TR_CURRENT_CONTENDED_MONITOR = 9,
	ID_TR_STOP = 10,
	ID_TR_INTERRUPT = 11,
	ID_TR_SUSPEND_COUNT = 12,
	ID_TR_OWNED_MONITORS_STACK_DEPTH = 13,
	ID_TR_FORCE_EARLY_RETURN = 14,

	/* Commands ThreadGroupReference */
	ID_TGR_NAME = 1,
	ID_TGR_PARENT = 2,
	ID_TGR_CHILDREN = 3,

	/* Commands ArrayReference */
	ID_AR_LENGTH = 1,
	ID_AR_GET_VALUES = 2,
	ID_AR_SET_VALUES = 3,

	/* Commands ClassLoaderReference */
	ID_CLR_VISIBLE_CLASSES = 1,

	/* Commands EventRequest */
	ID_ER_SET = 1,
	ID_ER_CLEAR = 2,
	ID_ER_CLEAR_ALL_BREAKPOINTS = 3,

	/* Commands StackFrame */
	ID_SF_GET_VALUES = 1,
	ID_SF_SET_VALUES = 2,
	ID_SF_THIS_OBJECT = 3,
	ID_SF_POP_FRAME = 4,

	/* Commands ClassObjectReference */
	ID_COR_REFLECTED_TYPE = 1,

	/* Commands Event */
	ID_E_COMPOSITE = 100
};

/*
* EventKind constants.
*/
enum eJdwpEventKind {
	EK_SINGLE_STEP = 1,
	EK_BREAKPOINT = 2,
	EK_FRAME_POP = 3,
	EK_EXCEPTION = 4,
	EK_USER_DEFINED = 5,
	EK_THREAD_START = 6,
	EK_THREAD_END = 7,
	EK_CLASS_PREPARE = 8,
	EK_CLASS_UNLOAD = 9,
	EK_CLASS_LOAD = 10,
	EK_FIELD_ACCESS = 20,
	EK_FIELD_MODIFICATION = 21,
	EK_EXCEPTION_CATCH = 30,
	EK_METHOD_ENTRY = 40,
	EK_METHOD_EXIT = 41,
	EK_VM_INIT = 90,
	EK_VM_DEATH = 99,
	EK_VM_DISCONNECTED = 100,  /* "Never sent across JDWP */
	EK_VM_START = EK_VM_INIT,
	EK_THREAD_DEATH = EK_THREAD_END,
};

/*
* Values for "modKind" in EventRequest.Set.
*/
enum eJdwpModKind {
	MK_COUNT = 1,
	MK_CONDITIONAL = 2,
	MK_THREAD_ONLY = 3,
	MK_CLASS_ONLY = 4,
	MK_CLASS_MATCH = 5,
	MK_CLASS_EXCLUDE = 6,
	MK_LOCATION_ONLY = 7,
	MK_EXCEPTION_ONLY = 8,
	MK_FIELD_ONLY = 9,
	MK_STEP = 10,
	MK_INSTANCE_ONLY = 11,
};

/*
* InvokeOptions constants (bit flags).
*/
enum eJdwpInvokeOptions {
	INVOKE_SINGLE_THREADED = 0x01,
	INVOKE_NONVIRTUAL = 0x02,
};

/*
* StepDepth constants.
*/
enum eJdwpStepDepth {
	SD_INTO = 0,    /* step into method calls */
	SD_OVER = 1,    /* step over method calls */
	SD_OUT = 2,     /* step out of current method */
};

/*
* StepSize constants.
*/
enum JdwpStepSize {
	SS_MIN = 0,    /* step by minimum (e.g. 1 bytecode inst) */
	SS_LINE = 1,    /* if possible, step to next line */
};

/*
* SuspendPolicy constants.
*/
enum eJdwpSuspendPolicy {
	SP_NONE = 0,          /* suspend no threads */
	SP_EVENT_THREAD = 1,  /* suspend event thread */
	SP_ALL = 2,           /* suspend all threads */
};

/*
* TypeTag constants.
*/
enum eJdwpTypeTag {
	TT_CLASS = 1,
	TT_INTERFACE = 2,
	TT_ARRAY = 3,
};

/*
* Tag constants.
*/
enum eJdwpType {
	JT_UNKNOWN = 0,
	JT_ARRAY = '[',
	JT_BYTE = 'B',
	JT_CHAR = 'C',
	JT_OBJECT = 'L',
	JT_FLOAT = 'F',
	JT_DOUBLE = 'D',
	JT_INT = 'I',
	JT_LONG = 'J',
	JT_SHORT = 'S',
	JT_VOID = 'V',
	JT_BOOLEAN = 'Z',
	JT_STRING = 's',
	JT_THREAD = 't',
	JT_THREAD_GROUP = 'g',
	JT_CLASS_LOADER = 'l',
	JT_CLASS_OBJECT = 'c',
};

/*
* SuspendStatus constants.
*/
enum eJdwpSuspendStatus {
	SUSPEND_STATUS_NOT_SUSPENDED = 0,
	SUSPEND_STATUS_SUSPENDED = 1,
};

/*
* ThreadStatus constants.
*/
enum eJdwpThreadStatus {
	TS_ZOMBIE = 0,
	TS_RUNNING = 1,      // RUNNING
	TS_SLEEPING = 2,     // (in Thread.sleep())
	TS_MONITOR = 3,      // WAITING (monitor wait)
	TS_WAIT = 4,         // (in Object.wait())
};

enum {
	ACC_PUBLIC = 0x00000001,          // class, field, method, ic
	ACC_PRIVATE = 0x00000002,         // field, method, ic
	ACC_PROTECTED = 0x00000004,       // field, method, ic
	ACC_STATIC = 0x00000008,          // field, method, ic
	ACC_FINAL = 0x00000010,           // class, field, method, ic
	ACC_SYNCHRONIZED = 0x00000020,    // method (only allowed on natives)
	ACC_SUPER = 0x00000020,           // class (not used in Dalvik)
	ACC_VOLATILE = 0x00000040,        // field
	ACC_BRIDGE = 0x00000040,          // method (1.5)
	ACC_TRANSIENT = 0x00000080,       // field
	ACC_VARARGS = 0x00000080,         // method (1.5)
	ACC_NATIVE = 0x00000100,          // method
	ACC_INTERFACE = 0x00000200,       // class, ic
	ACC_ABSTRACT = 0x00000400,        // class, method, ic
	ACC_STRICT = 0x00000800,          // method
	ACC_SYNTHETIC = 0x00001000,       // field, method, ic
	ACC_ANNOTATION = 0x00002000,      // class, ic (1.5)
	ACC_ENUM = 0x00004000,            // class, field, ic (1.5)
	ACC_CONSTRUCTOR = 0x00010000,     // method (Dalvik only)
	ACC_DECLARED_SYNCHRONIZED = 0x00020000,       // method (Dalvik only)
	ACC_CLASS_MASK =
	(ACC_PUBLIC | ACC_FINAL | ACC_INTERFACE | ACC_ABSTRACT
		| ACC_SYNTHETIC | ACC_ANNOTATION | ACC_ENUM),
	ACC_INNER_CLASS_MASK =
	(ACC_CLASS_MASK | ACC_PRIVATE | ACC_PROTECTED | ACC_STATIC),
	ACC_FIELD_MASK =
	(ACC_PUBLIC | ACC_PRIVATE | ACC_PROTECTED | ACC_STATIC | ACC_FINAL
		| ACC_VOLATILE | ACC_TRANSIENT | ACC_SYNTHETIC | ACC_ENUM),
	ACC_METHOD_MASK =
	(ACC_PUBLIC | ACC_PRIVATE | ACC_PROTECTED | ACC_STATIC | ACC_FINAL
		| ACC_SYNCHRONIZED | ACC_BRIDGE | ACC_VARARGS | ACC_NATIVE
		| ACC_ABSTRACT | ACC_STRICT | ACC_SYNTHETIC | ACC_CONSTRUCTOR
		| ACC_DECLARED_SYNCHRONIZED),
};

/*
	Holds a JDWP "location".
*/
struct JdwpLocation {
	u1         mTypeTag;
	RefTypeId  mClassId;
	MethodId   mMethodId;
	u8         mDexPc;
};

struct JdwpField {
	FieldId	     mFieldId;
	std::string	 mName;
	std::string	 mSignature;
	std::string	 mGenericSignature;
	u4	         mModBits;
};
