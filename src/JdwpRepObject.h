#pragma once

#include "jdwp.h"
#include "JdwpValue.h"
#include "JdwpSerializer.h"

struct JdwpRepObject {
	virtual void unserialize(u1* buffer, u4 size) = 0;
	virtual ostream& display(ostream& _os, int _level = 0) = 0;
};

/*
	Returns the JDWP version implemented by the target VM.

	VirtualMachine Command Set (1) / Version (1)
*/
struct JdwpRep_Vm_Version : public JdwpRepObject {
public:
	void unserialize(u1* buffer, u4 size) {
		JdwpSerializer _is(buffer, size);
		_is.read(mDescription);
		_is.read(mJdwpMajor);
		_is.read(mJdwpMinor);
		_is.read(mVmVersion);
		_is.read(mVmName);
	}
	ostream& display(ostream& _os, int _level = 0) {
		return _os;
	}
public:
	std::string   mDescription;
	u4            mJdwpMajor;
	u4            mJdwpMinor;
	std::string   mVmVersion;
	std::string   mVmName;
};

/*
	Returns reference types for all the classes loaded by the target VM which match the given signature. 
	Multiple reference types will be returned if two or more class loaders have loaded a class of the same name. 
	The search is confined to loaded classes only; no attempt is made to load a class of the given signature.

	VirtualMachine Command Set (1) / IDSizes (2)
*/
struct JdwpRep_Vm_ClassesBySignature : public JdwpRepObject {
	struct ClasseInfo {
		u1	       mRefTypeTag;	    // Kind of following reference type.
		RefTypeId  mTypeId;         // Loaded reference type
		u4	       mStatus;	        // The current class status.
	};
public:
	void unserialize(u1* buffer, u4 size) {
		JdwpSerializer _is(buffer, size);
		_is.read(mClasses);
		for (u4 i = 0; i < mClasses; i++) {
			ClasseInfo classInfo;
			_is.read(classInfo.mRefTypeTag);
			_is.read(classInfo.mTypeId);
			_is.read(classInfo.mStatus);
			mInfoVec.push_back(classInfo);
		}
	}
	ostream& display(ostream& _os, int _level = 0) {
		return _os;
	}
public:
	u4	                     mClasses;
	std::vector<ClasseInfo>  mInfoVec;
};

/*
	Returns the sizes of variably - sized data types in the target VM.

	VirtualMachine Command Set (1) / IDSizes (7)
*/
struct JdwpRep_Vm_IdSize : public JdwpRepObject {
public:
	void unserialize(u1* buffer, u4 size) {
		JdwpSerializer _is(buffer, size);
		_is.read(mFieldIDSize);
		_is.read(mMethodIDSize);
		_is.read(mObjectIDSize);
		_is.read(mReferenceTypeIDSize);
		_is.read(mFrameIDSize);
	}
	ostream& display(ostream& _os, int _level = 0) {
		return _os;
	}
public:
	u4	mFieldIDSize;
	u4	mMethodIDSize;
	u4	mObjectIDSize;
	u4	mReferenceTypeIDSize;
	u4	mFrameIDSize;
};


/*
	Returns reference types for all classes currently loaded by the target VM.

	VirtualMachine Command Set (1) / AllClassesWithGeneric(20)
*/
struct JdwpRep_Vm_AllClassesWithGeneric : public JdwpRepObject {
	struct ClasseInfo {
		u1	         mRefTypeTag;	    // Kind of following reference type.
		RefTypeId	 mTypeId;           // Loaded reference type
		std::string	 mSignature;	    // The JNI signature of the loaded reference type.
		std::string	 mGenericSignature; // The generic signature of the loaded reference type.
		u4	         mStatus;	        // The current class status.
	};
public:
	void unserialize(u1* buffer, u4 size) {
		JdwpSerializer _is(buffer, size);
		_is.read(mClasses);
		for (u4 i = 0; i < mClasses; i++) {
			ClasseInfo classInfo;
			_is.read(classInfo.mRefTypeTag);
			_is.read(classInfo.mTypeId);
			_is.read(classInfo.mSignature);
			_is.read(classInfo.mGenericSignature);
			_is.read(classInfo.mStatus);
			mInfoVec.push_back(classInfo);
		}
	}
	ostream& display(ostream& _os, int _level = 0) {
		return _os;
	}
public:
	u4	                     mClasses;
	std::vector<ClasseInfo>  mInfoVec;
};


/*
	Returns the JNI signature of a reference type along with the generic signature if there is one. 
	Generic signatures are described in the signature attribute section in the Java Virtual Machine 
	Specification, 3rd Edition. Since JDWP version 1.5.

	ReferenceType Command Set (2) / SignatureWithGeneric Command (13)
*/
struct JdwpRep_RefType_SignatureWithGeneric : public JdwpRepObject {
public:
	void unserialize(u1* buffer, u4 size) {
		JdwpSerializer _is(buffer, size);
		_is.read(mSignature);
		_is.read(mGenericSignature);
	}
	ostream& display(ostream& _os, int _level = 0) {
		return _os;
	}
public:
	std::string	 mSignature;
	std::string	 mGenericSignature;
};


/*
	Returns information, including the generic signature if any, for each field in a reference type. 
	Inherited fields are not included. The field list will include any synthetic fields created by the compiler. 
	Fields are returned in the order they occur in the class file. 
	Generic signatures are described in the signature attribute section in the Java Virtual Machine Specification, 3rd Edition. Since JDWP version 1.5.

	ReferenceType Command Set (2) / FieldsWithGeneric Command (14)
*/
struct JdwpRep_RefType_FieldsWithGeneric : public JdwpRepObject {

public:
	void unserialize(u1* buffer, u4 size) {
		JdwpSerializer _is(buffer, size);
		_is.read(mDeclared);
		for (u4 i = 0; i < mDeclared; i++) {
			JdwpField methodInfo;
			_is.read(methodInfo.mFieldId);
			_is.read(methodInfo.mName);
			_is.read(methodInfo.mSignature);
			_is.read(methodInfo.mGenericSignature);
			_is.read(methodInfo.mModBits);
			mInfoVec.push_back(methodInfo);
		}
	}
	ostream& display(ostream& _os, int _level = 0) {
		return _os;
	}
public:
	u4	                   mDeclared;
	std::vector<JdwpField> mInfoVec;
};

/*
	Returns the value of one or more static fields of the reference type. Each field must be member of the reference type
	or one of its superclasses, superinterfaces, or implemented interfaces. Access control is not enforced; for example,
	the values of private fields can be obtained.

	ReferenceType Command Set (2) / GetValues Command (6)
*/
struct JdwpRep_RefType_GetValues : public JdwpRepObject {
public:
	void unserialize(u1* buffer, u4 size) {
		JdwpSerializer _is(buffer, size);
		_is.read(mValues);
		for (u4 i = 0; i < mValues; i++) {
			JdwpValue jdwpValue;
			_is.read(jdwpValue.mType);
			u4 uSize = jdwpValue.getValueSize();
			switch (uSize) {
			case 1:
				_is.read(jdwpValue._byte.val);
				break;
			case 2:
				_is.read(jdwpValue._short.val);
				break;
			case 4:
				_is.read(jdwpValue._int.val);
				break;
			case 8:
				_is.read(jdwpValue._long.val);
				break;
			default:
				break;
			}
			mInfoVec.push_back(jdwpValue);
		}
	}
	ostream& display(ostream& _os, int _level = 0) {
		return _os;
	}
public:
	u4                      mValues;
	std::vector<JdwpValue>  mInfoVec;
};

/*
	Returns information, including the generic signature if any, for each method in a reference type.

	ReferenceType Command Set (2) / MethodsWithGeneric (15)
*/
struct JdwpRep_RefType_MethodsWithGeneric : public JdwpRepObject {
	struct MethodInfo {
		MethodId	 mMethodId;
		std::string	 mName;
		std::string	 mSignature;
		std::string	 mGenericSignature;
		u4	         mModBits;
	};
public:
	void unserialize(u1* buffer, u4 size) {
		JdwpSerializer _is(buffer, size);
		_is.read(mDeclared);
		for (u4 i = 0; i < mDeclared; i++) {
			MethodInfo methodInfo;
			_is.read(methodInfo.mMethodId);
			_is.read(methodInfo.mName);
			_is.read(methodInfo.mSignature);
			_is.read(methodInfo.mGenericSignature);
			_is.read(methodInfo.mModBits);
			mInfoVec.push_back(methodInfo);
		}
	}
	ostream& display(ostream& _os, int _level = 0) {
		return _os;
	}
public:
	u4	                    mDeclared;
	std::vector<MethodInfo> mInfoVec;
};


/*
	Returns line number information for the method, if present.
	The line table maps source line numbers to the initial code index of the line.

	Method Command Set (6) / LineTable (1)
*/
struct JdwpRep_Method_LineTable : public JdwpRepObject {
	struct LineInfo {
		u8   mLineCodeIndex;
		u4	 mLineNumber;
	};
public:
	void unserialize(u1* buffer, u4 size) {
		JdwpSerializer _is(buffer, size);
		_is.read(mStart);
		_is.read(mEnd);
		_is.read(mLines);
		for (u4 i = 0; i < mLines; i++) {
			LineInfo lineInfo;
			_is.read(lineInfo.mLineCodeIndex);
			_is.read(lineInfo.mLineNumber);
			mInfoVec.push_back(lineInfo);
		}
	}
	ostream& display(ostream& _os, int _level = 0) {
		return _os;
	}
public:
	u8                     mStart;
	u8                     mEnd;
	u4                     mLines;
	std::vector<LineInfo>  mInfoVec;
};

/*
	Returns variable information for the method, including generic signatures for the variables.

	Method Command Set (6) / VariableTableWithGeneric (5)
*/
struct JdwpRep_Method_VariableTableWithGeneric : public JdwpRepObject {
	struct VariableInfo {
		u4       mSlot;              // The local variable's index in its frame
		u8       mCodeIndex;         // First code index at which the variable is visible (unsigned).
		string	 mName;              // The variable's name.
		string	 mSignature;         // The variable type's JNI signature.
		string	 mGenericSignature;  // The variable type's generic signature or an empty string if there is none. 
		u4	     mLength;            // Unsigned value used in conjunction with codeIndex.
		//u4       mSlot;              // The local variable's index in its frame
	};
public:
	void unserialize(u1* buffer, u4 size) {
		JdwpSerializer _is(buffer, size);
		_is.read(mArgCnt);
		_is.read(mSlots);
		for (u4 i = 0; i < mSlots; i++) {
			VariableInfo variableInfo;
			_is.read(variableInfo.mCodeIndex);
			_is.read(variableInfo.mName);
			_is.read(variableInfo.mSignature);
			_is.read(variableInfo.mGenericSignature);
			_is.read(variableInfo.mLength);
			_is.read(variableInfo.mSlot);
			mInfoVec.push_back(variableInfo);
		}
	}
	ostream& display(ostream& _os, int _level = 0) {
		return _os;
	}
public:
	u4	                       mArgCnt;   // The number of words in the frame used by arguments.
	u4	                       mSlots;    // The number of variables. 
	std::vector<VariableInfo>  mInfoVec;
};

/*
	Returns the runtime type of the object. The runtime type will be a class or an array.

	ObjectReference Command Set (9) / ReferenceType Command (1)
*/
struct JdwpRep_ObjectRef_RefType : public JdwpRepObject {
public:
	void unserialize(u1* buffer, u4 size) {
		JdwpSerializer _is(buffer, size);
		_is.read(mRefTypeTag);
		_is.read(mTypeID);
	}
	ostream& display(ostream& _os, int _level = 0) {
		return _os;
	}
public:
	u1         mRefTypeTag;   // CLASS / INTERFACE / ARRAY
	RefTypeId  mTypeID;       // The runtime reference type. 
};

/*
	Returns the value of one or more instance fields. Each field must be member of the object's type or one of its superclasses, 
	superinterfaces, or implemented interfaces. Access control is not enforced; for example, the values of private fields can be obtained.

	ObjectReference Command Set (9) / GetValues Command (2)
*/
struct JdwpRep_ObjectRef_GetValues : public JdwpRepObject {
public:
	void unserialize(u1* buffer, u4 size) {
		JdwpSerializer _is(buffer, size);
		_is.read(mValues);
		for (u4 i = 0; i < mValues; i++) {
			JdwpValue jdwpValue;
			_is.read(jdwpValue.mType);
			u4 uSize = jdwpValue.getValueSize();
			switch (uSize) {
			case 1:
				_is.read(jdwpValue._byte.val);
				break;
			case 2:
				_is.read(jdwpValue._short.val);
				break;
			case 4:
				_is.read(jdwpValue._int.val);
				break;
			case 8:
				_is.read(jdwpValue._long.val);
				break;
			default:
				break;
			}
			mInfoVec.push_back(jdwpValue);
		}
	}
	ostream& display(ostream& _os, int _level = 0) {
		return _os;
	}
public:
	u4                      mValues;
	std::vector<JdwpValue>  mInfoVec;
};


/*
	Returns the characters contained in the string.
	StringReference Command Set (10) / Value Command (1)
*/
struct JdwpRep_StringReference_Value : public JdwpRepObject {
public:
	void unserialize(u1* buffer, u4 size) {
		JdwpSerializer _is(buffer, size);
		_is.read(mStringValue);
	}
	ostream& display(ostream& _os, int _level = 0) {
		return _os;
	}
public:
	std::string	 mStringValue;  // UTF-8 representation of the string value. 
};

/*
	Returns the thread name.

	ThreadReference Command Set(11) / Frames Command (1)
*/
struct JdwpRep_ThreadReference_Name : public JdwpRepObject {
public:
	void unserialize(u1* buffer, u4 size) {
		JdwpSerializer _is(buffer, size);
		_is.read(mThreadName);
	}
	ostream& display(ostream& _os, int _level = 0) {
		return _os;
	}
public:
	std::string	 mThreadName;
};

/*
	Returns the current status of a thread. The thread status reply indicates the thread status the 
	last time it was running. the suspend status provides information on the thread's suspension, if any.

	ThreadReference Command Set(11) / Frames Command (4)
*/
struct JdwpRep_ThreadReference_Status : public JdwpRepObject {
public:
	void unserialize(u1* buffer, u4 size) {
		JdwpSerializer _is(buffer, size);
		_is.read(mThreadStatus);
		_is.read(mSuspendStatus);
	}
	ostream& display(ostream& _os, int _level = 0) {
		return _os;
	}
public:
	u4	 mThreadStatus;   // One of the thread status codes See JDWP.ThreadStatus 
	u4	 mSuspendStatus;  // One of the suspend status codes See JDWP.SuspendStatus
};


/*
	Returns the current call stack of a suspended thread.
	The sequence of frames starts with the currently executing frame, followed by its caller, and so on.
	The thread must be suspended, and the returned frameID is valid only while the thread is suspended.

	ThreadReference Command Set(11) / Frames Command (6)
*/
struct JdwpRep_ThreadReference_Frames : public JdwpRepObject {
	struct FramesInfo {
		FrameId       mFrameID;
		JdwpLocation  mLocation;
	};
public:
	void unserialize(u1* buffer, u4 size) {
		JdwpSerializer _is(buffer, size);
		_is.read(mFrames);
		for (u4 i = 0; i < mFrames; i++) {
			FramesInfo framesInfo;
			_is.read(framesInfo.mFrameID);
			_is.read(framesInfo.mLocation.mTypeTag);
			_is.read(framesInfo.mLocation.mClassId);
			_is.read(framesInfo.mLocation.mMethodId);
			_is.read(framesInfo.mLocation.mDexPc);
			mInfoVec.push_back(framesInfo);
		}
	}
	ostream& display(ostream& _os, int _level = 0) {
		return _os;
	}
public:
	u4	                     mFrames;
	std::vector<FramesInfo>  mInfoVec;
};

/*
	Returns the number of components in a given array.

	ArrayReference Command Set (13) / Length Command (1)
*/
struct JdwpRep_ArrayReference_Length : public JdwpRepObject {
public:
	void unserialize(u1* buffer, u4 size) {
		JdwpSerializer _is(buffer, size);
		_is.read(mArrayLength);
	}
	ostream& display(ostream& _os, int _level = 0) {
		return _os;
	}
public:
	u4	 mArrayLength;
};

/*
	Returns a range of array components. The specified range must be within the bounds of the array.

	ArrayReference Command Set (13) / GetValues Command (2)
*/
struct JdwpRep_ArrayReference_Values : public JdwpRepObject {
public:
	void unserialize(u1* buffer, u4 size) {
		JdwpSerializer _is(buffer, size);
		_is.read(mType);
		_is.read(mLen);
		for (u4 i = 0; i < mLen; i++) {
			JdwpValue jdwpValue(mType);
			switch (mType) {
			case JT_BYTE:
			case JT_BOOLEAN:
				_is.read(jdwpValue._byte.val);
				break;
			case JT_CHAR:
			case JT_SHORT:
				_is.read(jdwpValue._short.val);
				break;
			case JT_FLOAT:
			case JT_INT:
				_is.read(jdwpValue._int.val);
				break;
			case JT_DOUBLE:
			case JT_LONG:
				_is.read(jdwpValue._long.val);
				break;
			case JT_OBJECT:
			case JT_ARRAY:
			case JT_STRING:
			case JT_THREAD:
			case JT_THREAD_GROUP:
			case JT_CLASS_LOADER:
			case JT_CLASS_OBJECT:
				_is.read(jdwpValue.mType);
				_is.read(jdwpValue._object.val);
				break;
			default:
				break;
			}
			mInfoVec.push_back(jdwpValue);
		}
	}
	ostream& display(ostream& _os, int _level = 0) {
		return _os;
	}
public:
	u1                      mType;
	u4                      mLen;
	std::vector<JdwpValue>  mInfoVec;
};


/*
	Set an event request. When the event described by this request occurs, an event is sent from the target VM.

	EventRequest Command Set (15) / Set Command (1)

	byte	eventKind;	    // Event kind to request. See JDWP.EventKind for a complete list of events that can be requested.
	byte	suspendPolicy;	// What threads are suspended when this event occurs?
	int	    modifiers;      // Constraints used to control the number of generated events.

	byte	modKind;
	[1] : Count
	--> int  count;

	[2] : Conditional
	--> int	 exprID;

	[3] : ThreadOnly
	--> threadID thread;

	[4] : ClassOnly
	-->referenceTypeID	clazz;

	[5] : ClassMatch
	-->string	classPattern;

	[6] : ClassExclude
	-->string	classPattern;

	[7] : location
	-->location	loc;

	[8] : ExceptionOnly
	-->referenceTypeID	exceptionOrNull;
	-->boolean	        caught;
	-->boolean	        uncaught;

	[9] : FieldOnly
	-->referenceTypeID	declaring;
	-->fieldID	        fieldID;

	[10] : Step
	-->threadID	thread;
	-->int	    size;
	-->int	    depth;

	[11] : InstanceOnly
	-->objectID	instance;

	[12] : SourceNameMatch
	-->string	sourceNamePattern;

	[Reply Data]
	{
	int	requestID;
	}
*/
struct JdwpRep_EventRequest_Set : public JdwpRepObject {
public:
	void unserialize(u1* buffer, u4 size) {
		JdwpSerializer _is(buffer, size);
		_is.read(mRequestID);
	}
	ostream& display(ostream& _os, int _level = 0) {
		return _os;
	}
public:
	u4	mRequestID;  // ID of created request 
};


/*
	Returns the value of one or more local variables in a given frame. Each variable must be visible at the frame's code index.
	Even if local variable information is not available, values can be retrieved if the front-end is able to determine the correct local variable index.
	(Typically, this index can be determined for method arguments from the method signature without access to the local variable table information.)

	StackFrame Command Set(16) / GetValues Command(1)
*/
struct JdwpRep_StackFrame_GetValues : public JdwpRepObject {
public:
	void unserialize(u1* buffer, u4 size) {
		JdwpSerializer _is(buffer, size);
		_is.read(mValues);
		for (u4 i = 0; i < mValues; i++) {
			JdwpValue value;
			_is.read(value.mType);
			switch (value.mType) {
			case JT_BYTE:
			case JT_BOOLEAN:
				_is.read(value._byte.val);
				break;
			case JT_CHAR:
			case JT_SHORT:
				_is.read(value._short.val);
				break;
			case JT_FLOAT:
			case JT_INT:
				_is.read(value._int.val);
				break;
			case JT_DOUBLE:
			case JT_LONG:
				_is.read(value._long.val);
				break;
			case JT_OBJECT:
			case JT_ARRAY:
			case JT_STRING:
			case JT_THREAD:
			case JT_THREAD_GROUP:
			case JT_CLASS_LOADER:
			case JT_CLASS_OBJECT:
				_is.read(value._object.val);
				break;
			default:
				return;
			}
			mInfoVec.push_back(value);
		}
	}
	ostream& display(ostream& _os, int _level = 0) {
		return _os;
	}
public:
	u4	               mValues;
	vector<JdwpValue>  mInfoVec;
};


/*
	Returns the value of one or more local variables in a given frame. Each variable must be visible at the frame's code index.
	Even if local variable information is not available, values can be retrieved if the front-end is able to determine the correct local variable index.
	(Typically, this index can be determined for method arguments from the method signature without access to the local variable table information.)

	StackFrame Command Set(16) / GetValues Command(1)
*/
struct JdwpRep_StackFrame_ThisObject : public JdwpRepObject {
public:
	void unserialize(u1* buffer, u4 size) {
		JdwpSerializer _is(buffer, size);
		_is.read(mObjectId);
	}
	ostream& display(ostream& _os, int _level = 0) {
		return _os;
	}
public:
	ObjectId	mObjectId;
};

/*
	Several events may occur at a given time in the target VM. For example, there may be more than
	one breakpoint request for a given location or you might single step to the same location as a
	breakpoint request. These events are delivered together as a composite event. For uniformity,
	a composite event is always used to deliver events, even if there is only one event to report.

	Event Command Set (64) / Composite Command (100)
*/
struct JdwpRep_Event_Composite : public JdwpRepObject {
	struct EventObject {
		u1   mModKind;
		u4   mRequestId;
	};
	struct EventVmStart : public EventObject {
		ObjectId  mThreadId;
	};
	struct EventSingleStep : public EventObject {
		ObjectId     mThreadId;
		JdwpLocation mLocation;
	};
	struct EventBreakpoint : public EventObject {
		ObjectId     mThreadId;
		JdwpLocation mLocation;
	};
	struct EventMethodEntry : public EventObject {
		ObjectId     mThreadId;
		JdwpLocation mLocation;
	};
	struct EventMethodExit : public EventObject {
		ObjectId     mThreadId;
		JdwpLocation mLocation;
	};
	struct EventMethodExitWithValue : public EventObject {
		ObjectId     mThreadId;
		JdwpLocation mLocation;
		u8           mRetValue;   // size ??
	};
	struct EventMonitorContendedEnter : public EventObject {
		ObjectId     mThreadId;
		ObjectId     mObjectId;
		JdwpLocation mLocation;
	};
	struct EventMonitorContendedEntered : public EventObject {
		ObjectId     mThreadId;
		ObjectId     mObjectId;
		JdwpLocation mLocation;
	};
	struct EventMonitorWait : public EventObject {
		ObjectId     mThreadId;
		ObjectId     mObjectId;
		JdwpLocation mLocation;
		u8           mTimeout;
	};
	struct EventMonitorWaited : public EventObject {
		ObjectId     mThreadId;
		ObjectId     mObjectId;
		JdwpLocation mLocation;
		u4           mIsTimeout; // boolean size??
	};
	struct EventException : public EventObject {
		ObjectId     mThreadId;
		JdwpLocation mLocation;
		ObjectId     mException;
		JdwpLocation mCatchLocation;
	};
	struct EventThreadStart : public EventObject {
	};
	struct EventThreadDeath : public EventObject {
		ObjectId  mThreadId;
	};
	struct EventClassPrePare : public EventObject {
		ObjectId  mThreadId;
		u1        mRefTypeTag;
		RefTypeId mTypeId;
		string    mSignature;
		u4        mStatus;
	};
	struct EventClassUnload : public EventObject {
		string    mSignature;
	};
	struct EventFieldAccess : public EventObject {
		ObjectId  mThreadId;
		u1        mRefTypeTag;
		RefTypeId mTypeId;
		FieldId   mFieldId;
		ObjectId  mObjectId;
	};
	struct EventFieldModification : public EventObject {
		ObjectId  mThreadId;
		u1        mRefTypeTag;
		RefTypeId mTypeId;
		FieldId   mFieldId;
		ObjectId  mObjectId;
		u8        mValueToBe;   // size ??
	};
	struct EventVmDeath : public EventObject {
	};
public:
	void unserialize(u1* buffer, u4 size) {
		JdwpSerializer _is(buffer, size);
		_is.read(mSuspendPolicy);
		_is.read(mEvents);
		for (u4 i = 0; i < mEvents; i++) {
			u1 modKind = 0;
			_is.read(modKind);
			switch (modKind) {
			case EK_SINGLE_STEP:
				break;
			case EK_BREAKPOINT:
			{
				EventBreakpoint* event = new EventBreakpoint;
				event->mModKind = modKind;
				_is.read(event->mRequestId);
				_is.read(event->mThreadId);
				_is.read(event->mLocation.mTypeTag);
				_is.read(event->mLocation.mClassId);
				_is.read(event->mLocation.mMethodId);
				_is.read(event->mLocation.mDexPc);
				mEventList.push_back(event);
				break;
			}
			case EK_FRAME_POP:
				break;
			case EK_EXCEPTION:
				break;
			case EK_USER_DEFINED:
				break;
			case EK_THREAD_START:
				break;
			case EK_THREAD_END:
				break;
			case EK_CLASS_PREPARE:
				break;
			case EK_CLASS_UNLOAD:
				break;
			case EK_CLASS_LOAD:
				break;
			case EK_FIELD_ACCESS:
				break;
			case EK_FIELD_MODIFICATION:
				break;
			case EK_EXCEPTION_CATCH:
				break;
			case EK_METHOD_ENTRY:
			{
				EventMethodEntry* event = new EventMethodEntry;
				event->mModKind = modKind;
				_is.read(event->mRequestId);
				_is.read(event->mThreadId);
				_is.read(event->mLocation.mTypeTag);
				_is.read(event->mLocation.mClassId);
				_is.read(event->mLocation.mMethodId);
				_is.read(event->mLocation.mDexPc);
				mEventList.push_back(event);
				break;
			}
			case EK_METHOD_EXIT:
				break;
			case EK_VM_START:
				break;
			case EK_VM_DEATH:
				break;
			case EK_VM_DISCONNECTED:
				break;
			default:
				break;
			}
		}
	}
	ostream& display(ostream& _os, int _level = 0) {
		return _os;
	}
public:
	u1	                       mSuspendPolicy;
	u4	                       mEvents;
	std::vector<EventObject*>  mEventList;
};

/*
	ClassPrepareEvent	装载某个指定的类所引发的事件
	ClassUnloadEvent	卸载某个指定的类所引发的事件
	BreakingpointEvent	设置断点所引发的事件
	ExceptionEvent	    目标虚拟机运行中抛出指定异常所引发的事件
	MethodEntryEvent	进入某个指定方法体时引发的事件
	MethodExitEvent	    某个指定方法执行完成后引发的事件
	MonitorContendedEnteredEvent	线程已经进入某个指定 Monitor 资源所引发的事件
	MonitorContendedEnterEvent	    线程将要进入某个指定 Monitor 资源所引发的事件
	MonitorWaitedEvent	线程完成对某个指定 Monitor 资源等待所引发的事件
	MonitorWaitEvent	线程开始等待对某个指定 Monitor 资源所引发的事件
	StepEvent	        目标应用程序执行下一条指令或者代码行所引发的事件
	AccessWatchpointEvent	    访问类的某个指定 Field 所引发的事件
	ModificationWatchpointEvent	修改类的某个指定 Field 值所引发的事件
	ThreadDeathEvent	    某个指定线程运行完成所引发的事件
	ThreadStartEvent	    某个指定线程开始运行所引发的事件
	VMDeathEvent	        目标虚拟机停止运行所以的事件
	VMDisconnectEvent	    目标虚拟机与调试器断开链接所引发的事件
	VMStartEvent	        目标虚拟机初始化时所引发的事件
*/
