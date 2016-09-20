#include "AVMDebugger.h"

AvmDebugger::AvmDebugger() {

}


AvmDebugger::~AvmDebugger() {

}

bool AvmDebugger::attach(string & processName) {
	return attachProcess(processName);
}

void AvmDebugger::waitLoop() {
	runEventLoop();
}

void AvmDebugger::registerEventNotify(u4 uRequestId, EventContext eventContext) {
	std::lock_guard<std::mutex> lk(mEventMutex);
	mEventNotifyMap.insert(make_pair(uRequestId, eventContext));
}

bool AvmDebugger::getEventContext(u4 uRequestId, EventContext& eventContext) {
	std::lock_guard<std::mutex> lk(mEventMutex);
	auto item = mEventNotifyMap.find(uRequestId);
	if (item != mEventNotifyMap.end()) {
		eventContext = item->second;
		return true;
	}
	return false;
}

void AvmDebugger::handleEventNotify(JdwpRep_Event_Composite& eventNotify)
{
	for (auto reEvent : eventNotify.mEventList) {
		py::dict paramContext;
		EventContext Context;
		if (!getEventContext(reEvent->mRequestId, Context))
			continue;

		switch (reEvent->mModKind) {
		case EK_BREAKPOINT: {
			py::int_ registerCount = Context.mReqExt["registers"];
			py::str funcSign = Context.mReqExt["sign"];
			py::int_ modBits = Context.mReqExt["modbits"];

			JdwpRep_Event_Composite::EventBreakpoint* breakPoint = (JdwpRep_Event_Composite::EventBreakpoint*)(reEvent);

			// 线程名称
			JdwpRep_ThreadReference_Name threadName;
			if (!getThreadReferenceName(threadName, breakPoint->mThreadId)) {
				continue;
			}

			// 堆栈信息
			JdwpRep_ThreadReference_Frames threadFrames;
			if (!getThreadReferenceFrames(threadFrames, breakPoint->mThreadId)) {
				continue;
			}

			// 解析参数
			vector<JdwpVariable> paramVec;
			string paramSign = getParamSign(funcSign);
			bool bStactic = ((u4)modBits & ACC_STATIC) != 0;
			if (!analysisParam(paramSign, registerCount, bStactic, paramVec)) {
				continue;
			}

			// 获取参数值
			JdwpRep_StackFrame_GetValues paramValues;
			JdwpRep_ThreadReference_Frames::FramesInfo& topFrame = threadFrames.mInfoVec[0];
			if (!getStackFrameValues(paramValues, breakPoint->mThreadId, topFrame.mFrameID, paramVec)) {
				continue;
			}

			// 转换参数值
			py::dict paramDict;
			for (u4 i = 0; i < paramValues.mValues; i++) {
				py::dict param;
				param["type"] = py::str(paramValues.mInfoVec[i].getTypeName());;
				param["slot"] = py::int_(paramVec[i].mSlot);
				formatJdwpValue(paramValues.mInfoVec[i], param);
				paramDict[py::str(paramVec[i].mName)] = param;
			}

			paramContext["param"] = paramDict;
			paramContext["threadId"] = py::int_(breakPoint->mThreadId);
			paramContext["threadName"] = py::str(threadName.mThreadName);
			paramContext["frameId"] = py::int_(topFrame.mFrameID);
			paramContext["reqExt"] = Context.mReqExt;
		}
		default:
			break;
		}

		if (Context.mCallBack)
			Context.mCallBack(paramContext);
	}
}

py::dict AvmDebugger::getRegisterValue(py::dict& Context, string& regName, u1 varType)
{
	py::int_ threadId = Context["threadId"];
	py::int_ frameId = Context["frameId"];
	py::dict paramDict = Context["param"];

	u4 uVarSlot = -1;
	std::transform(regName.begin(), regName.end(), regName.begin(), ::tolower);
	switch (regName.at(0)) {
	case 'p': {
		py::dict param = paramDict[py::str(regName)];
		py::int_ slot = param["slot"];
		uVarSlot = slot;
		break;
	}
	case 'v': {
		sscanf_s(regName.c_str(), "v%d", &uVarSlot);
		if (uVarSlot == -1)
			return py::dict();
		if (uVarSlot == 0)
			uVarSlot = 1000;
		break;
	}
	default:
		return py::dict();
	}

	vector<JdwpVariable> variableVec;
	JdwpVariable jdwpVar;
	jdwpVar.mValue.mType = varType;
	jdwpVar.mSlot = uVarSlot;
	variableVec.push_back(jdwpVar);

	JdwpRep_StackFrame_GetValues frameValues;
	if (getStackFrameValues(frameValues, threadId, frameId, variableVec)) {
		py::dict var;
		var["type"] = py::str(frameValues.mInfoVec[0].getTypeName());
		formatJdwpValue(frameValues.mInfoVec[0], var);
		return var;
	}

	return py::dict();
}

py::str AvmDebugger::getStringValue(ObjectId objectId)
{
	JdwpRep_StringReference_Value strRef;
	if (getStringReferenceValue(strRef, objectId)) {
		return py::str(strRef.mStringValue);
	}
	return py::str("");
}

py::dict AvmDebugger::getArrayObjectValue(ObjectId objectId)
{
	py::dict aryObj;
	JdwpRep_ArrayReference_Length arryLen;
	if (getArrayReferenceLength(arryLen, objectId)) {

		JdwpRep_ArrayReference_Values arrayValues;
		if (getArrayReferenceValues(arrayValues, objectId, 0, arryLen.mArrayLength)) {

			JdwpValue jdwpValue(arrayValues.mType);
			aryObj["type"] = py::str(jdwpValue.getTypeName());
			u4 valSize = jdwpValue.getValueSize();
			py::list dataList;

			for (auto item : arrayValues.mInfoVec) {
				switch (arrayValues.mType) {
				case JT_BYTE:
				case JT_BOOLEAN:
					dataList.append(py::int_(item._byte.val));
					break;
				case JT_CHAR:
				case JT_SHORT:
					dataList.append(py::int_(item._short.val));
					break;
				case JT_FLOAT:
					dataList.append(py::float_(item._float.val));
					break;
				case JT_INT:
					dataList.append(py::int_(item._int.val));
					break;
				case JT_DOUBLE:
					dataList.append(py::float_(item._double.val));
					break;
				case JT_STRING:
				case JT_LONG:
				case JT_OBJECT:
				case JT_ARRAY:
				case JT_THREAD:
				case JT_THREAD_GROUP:
				case JT_CLASS_LOADER:
				case JT_CLASS_OBJECT:
					dataList.append(py::int_(item._long.val));
				}
			}
			aryObj["data"] = dataList;

			return aryObj;
		}
	}
	return py::dict();
}

py::list AvmDebugger::getObjectFieldValues(ObjectId objectId)
{
	py::list objectFields;

	JdwpRep_ObjectRef_RefType refType;
	if (!getObjectReferenceRefType(refType, objectId)) 
		return py::list();

	JdwpRep_RefType_FieldsWithGeneric refTypeFields;
	if (!getRefTypeFieldsWithGeneric(refTypeFields, refType.mTypeID))
		return py::list();

	vector<JdwpField> staticFields;
	vector<JdwpField> instanceFields;
	for (auto fieldItem : refTypeFields.mInfoVec) {
		if (fieldItem.mModBits & ACC_STATIC)
			staticFields.push_back(fieldItem);
		else
			instanceFields.push_back(fieldItem);
	}

	JdwpRep_RefType_GetValues staticFieldValues;
	if (staticFields.size() && getRefTypeGetValues(staticFieldValues, refType.mTypeID, staticFields)) {
		for (size_t i = 0; i < staticFieldValues.mInfoVec.size(); i++) {
			py::dict valDict;
			JdwpValue& valItem = staticFieldValues.mInfoVec[i];
			valDict["name"] = py::str(staticFields[i].mName);
			valDict["type"] = py::str(valItem.getTypeName());
			valDict["sign"] = py::str(staticFields[i].mSignature);
			valDict["property"] = py::str("static");
			formatJdwpValue(valItem, valDict);
			objectFields.append(valDict);
		}
	}

	JdwpRep_ObjectRef_GetValues instanceFieldValues;
	if (instanceFields.size() && getObjectReferenceGetValues(instanceFieldValues, objectId, instanceFields)) {
		for (size_t i = 0; i < instanceFieldValues.mInfoVec.size(); i++) {
			py::dict valDict;
			JdwpValue& valItem = instanceFieldValues.mInfoVec[i];
			valDict["name"] = py::str(instanceFields[i].mName);
			valDict["type"] = py::str(valItem.getTypeName());
			valDict["sign"] = py::str(instanceFields[i].mSignature);
			valDict["property"] = py::str("instance");
			formatJdwpValue(valItem, valDict);
			objectFields.append(valDict);
		}
	}

	return objectFields;
}

py::list AvmDebugger::getAllClasses() {
	py::list classes;
	JdwpRep_Vm_AllClassesWithGeneric vmAllClasses;
	if (getVmAllClassesWithGeneric(vmAllClasses)) {
		for (size_t i = 0; i < vmAllClasses.mInfoVec.size(); i++) {
			JdwpRep_Vm_AllClassesWithGeneric::ClasseInfo& classInfo = vmAllClasses.mInfoVec[i];
			py::dict dict;
			dict["mRefTypeTag"] = py::int_(classInfo.mRefTypeTag);
			dict["mTypeId"] = py::int_(classInfo.mTypeId);
			dict["mSignature"] = py::str(classInfo.mSignature);
			dict["mGenericSignature"] = py::str(classInfo.mGenericSignature);
			dict["mStatus"] = py::int_(classInfo.mStatus);
			classes.append(dict);
		}
	}
	return classes;
}

py::list AvmDebugger::getClassesBySignature(string& classSign) {
	py::list classes;
	JdwpRep_Vm_ClassesBySignature vmClasses;
	if (getVmClassesBySignature(vmClasses, classSign)) {
		for (size_t i = 0; i < vmClasses.mInfoVec.size(); i++) {
			JdwpRep_Vm_ClassesBySignature::ClasseInfo& classInfo = vmClasses.mInfoVec[i];
			py::dict dict;
			dict["mRefTypeTag"] = py::int_(classInfo.mRefTypeTag);
			dict["mTypeId"] = py::int_(classInfo.mTypeId);
			dict["mStatus"] = py::int_(classInfo.mStatus);
			classes.append(dict);
		}
	}
	return classes;
}

py::list AvmDebugger::getStackFrames(ObjectId threadId)
{
	py::list stackFrames;

	JdwpRep_ThreadReference_Frames threadFrames;
	if (!getThreadReferenceFrames(threadFrames, threadId))
		return stackFrames;

	for (size_t i = 0; i < threadFrames.mInfoVec.size(); i++) {
		JdwpRep_ThreadReference_Frames::FramesInfo& framesInfo = threadFrames.mInfoVec[i];

		JdwpRep_RefType_SignatureWithGeneric classSign;
		getRefTypeSignatureWithGeneric(classSign, framesInfo.mLocation.mClassId);

		JdwpRep_RefType_MethodsWithGeneric Methods;
		getRefTypeMethodsWithGeneric(Methods, framesInfo.mLocation.mClassId);
		for (size_t k = 0; k < Methods.mInfoVec.size(); k++) {
			JdwpRep_RefType_MethodsWithGeneric::MethodInfo& methodInfo = Methods.mInfoVec[k];
			if (methodInfo.mMethodId == framesInfo.mLocation.mMethodId) {
				py::dict Frame;
				Frame["class"] = py::str(classSign.mSignature);
				Frame["method"] = py::str(methodInfo.mName);
				Frame["sign"] = py::str(methodInfo.mSignature);
				Frame["index"] = py::int_(framesInfo.mLocation.mDexPc);
				stackFrames.append(Frame);
				break;
			}
		}
	}

	return 	stackFrames;
}

bool AvmDebugger::setBreakPoint(py::dict& breakPoint)
{
	py::str classSign = breakPoint["class"];
	py::str methodName = breakPoint["method"];
	py::str methodSign = breakPoint["sign"];
	py::int_ codeIdx = breakPoint["index"];
	py::function callBack = breakPoint["callback"];

	JdwpRep_Vm_ClassesBySignature vmClasses;
	if (getVmClassesBySignature(vmClasses, (string)classSign)) {
		JdwpRep_Vm_ClassesBySignature::ClasseInfo& classInfo = vmClasses.mInfoVec[0];
		JdwpRep_RefType_MethodsWithGeneric refTypeMethods;
		if (getRefTypeMethodsWithGeneric(refTypeMethods, classInfo.mTypeId)) {
			auto it = refTypeMethods.mInfoVec.begin();
			for (; it != refTypeMethods.mInfoVec.end(); it++) {
				JdwpRep_RefType_MethodsWithGeneric::MethodInfo& methodInfo = (*it);
				if (methodName == methodInfo.mName && methodSign == methodInfo.mSignature) {
					JdwpRep_EventRequest_Set repEventSet;
					if (setBreakPointEvent(repEventSet, classInfo.mTypeId, methodInfo.mMethodId, codeIdx)) {
						EventContext eventContext;
						eventContext.mCallBack = callBack;
						breakPoint["modbits"] = py::int_(methodInfo.mModBits);
						eventContext.mReqExt = breakPoint;
						registerEventNotify(repEventSet.mRequestID, eventContext);
						return true;
					}
				}
			}
		}
	}
	return false;
}

void AvmDebugger::formatJdwpValue(JdwpValue& val, py::dict& item) {
	switch (val.mType) {
		case JT_OBJECT: {
			item["id"] = py::int_(val._object.val);
			// item["value"] = getObjectFieldValues(val._object.val);
			break;
		}
		case JT_STRING: {
			item["id"] = py::int_(val._string.val);
			item["value"] = getStringValue(val._string.val);
			break;
		}
		case JT_ARRAY: {
			item["id"] = py::int_(val._arry.val);
			item["value"] = getArrayObjectValue(val._arry.val);
			break;
		}
		case JT_BYTE: {
			item["value"] = py::int_(val._byte.val);
			break;
		}
		case JT_BOOLEAN: {
			item["value"] = py::int_(val._boolean.val);
			break;
		}
		case JT_CHAR: {
			item["value"] = py::int_(val._byte.val);
			break;
		}
		case JT_SHORT: {
			item["value"] = py::int_(val._short.val);
			break;
		}
		case JT_FLOAT: {
			item["value"] = py::float_(val._float.val);
			break;
		}
		case JT_INT: {
			item["value"] = py::int_(val._int.val);
			break;
		}
		case JT_DOUBLE: {
			item["value"] = py::float_(val._double.val);
			break;
		}
		case JT_LONG: {
			item["value"] = py::int_(val._long.val);
			break;
		}
		case JT_THREAD: {
			item["id"] = py::int_(val._thread.val);
			break;
		}
		case JT_THREAD_GROUP: {
			item["id"] = py::int_(val._threadgroup.val);
			break;
		}
		case JT_CLASS_LOADER: {
			item["id"] = py::int_(val._classloader.val);
			break;
		}
		case JT_CLASS_OBJECT: {
			item["id"] = py::int_(val._classobject.val);
			break;
		}
	}
}