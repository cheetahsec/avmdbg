#pragma once

#include "jdwpSocket.h"

#include <string>
#include <map>
using namespace std;

#include "jdwp.h"
#include "AdbHelper.h"
#include "JdwpValue.h"
#include "JdwpRepObject.h"

#define kMagicHandshake     "JDWP-Handshake"
#define kMagicHandshakeLen  (sizeof(kMagicHandshake)-1)

struct JdwpReplyNotify
{
	HANDLE        mHandle;
	JdwpPacket*   mPacket;
};

struct JdwpBreakPointEvent
{
	u1            mEventKind;
	u1            mSuspendPolicy;
	u4            mModifiers;
	u1            mModKind;
	JdwpLocation  mLocation;
};

class JdwpProtocol {
public:
	bool attachProcess(const string& processName) {
		AdbHelper adbHelper;
		map<string, string> debugableApps;
		adbHelper.getDebugableApps(&debugableApps);
		map<string, string>::iterator it = debugableApps.find(processName);
		if (it != debugableApps.end()) {
			adbHelper.cmd_forword_jdwp("8819", it->second);
			if (tryHandshake("127.0.0.1", 8819)) {
				runWorker();
				return true;
			}
		}
		return false;
	}

	bool tryHandshake(const char* addr, int port) {
		if (mJdwpSocket.Connect(addr, port)) {
			mJdwpSocket.Send(kMagicHandshake, kMagicHandshakeLen);
			char packet[64] = { 0 };
			mJdwpSocket.Recv(packet, sizeof(packet));
			if (::memcmp(packet, kMagicHandshake, kMagicHandshakeLen) == 0) {
				mJdwpSocket.runRecvWorker();
				return true;
			}
		}
		return false;
	}

	JdwpReplyNotify* registerReplyNotify(u4 uPacketId) {
		JdwpReplyNotify* repNotify = new JdwpReplyNotify;
		if (repNotify) {
			repNotify->mHandle = ::CreateEvent(NULL, FALSE, FALSE, NULL);;
			repNotify->mPacket = NULL;
			mReplyMutex.lock();
			mReplyNotifyMap.insert(make_pair(uPacketId, repNotify));
			mReplyMutex.unlock();
		}
		return repNotify;
	}

	bool unRegisterReplyNotify(u4 uPacketId) {
		bool bRet = false;
		mReplyMutex.lock();
		std::map<u4, JdwpReplyNotify*>::iterator it = mReplyNotifyMap.find(uPacketId);
		if (it != mReplyNotifyMap.end()) {
			JdwpReplyNotify*& reqEvent = it->second;
			if (reqEvent) {
				::CloseHandle(reqEvent->mHandle);
				if (reqEvent->mPacket)
					delete reqEvent->mPacket;
				delete reqEvent;
			}
			mReplyNotifyMap.erase(it);
			bRet = true;
		}
		mReplyMutex.unlock();
		return bRet;
	}

	bool waitRepNotify(JdwpReplyNotify* repEvent, ULONG timeOut = 2000) {
		if (repEvent) {
			ULONG uState = WaitForSingleObject(repEvent->mHandle, timeOut);
			if (uState == WAIT_OBJECT_0) {
				return true;
			}
			else {
				std::cout << "[error] waitRepEvent -> uState:" << uState << endl;
			}
		}
		return false;
	}

	bool waitJdwpResponse(JdwpPacket* jdwpPacket, JdwpRepObject* repObject, ULONG timeOut = 2000) {
		bool bRet = false;
		if (!repObject) {
			mJdwpSocket.Send((char*)jdwpPacket->getBuffer(), jdwpPacket->getBufferSize());
			return true;
		}
		JdwpReplyNotify* repEvent = registerReplyNotify(jdwpPacket->getId());
		if (repEvent) {
			mJdwpSocket.Send((char*)jdwpPacket->getBuffer(), jdwpPacket->getBufferSize());
			waitRepNotify(repEvent);
			JdwpPacket* repPacket = repEvent->mPacket;
			if (repPacket) {
				JdwpPacketHeader* jdwpHeader = repPacket->getHeader();
				if (jdwpHeader->errorCode == 0) {
					if (repObject)
						repObject->unserialize(repPacket->getBody(), repPacket->getBodySize());
					bRet = true;
				}
			}
			unRegisterReplyNotify(jdwpPacket->getId());
		}
		return bRet;
	}

	bool getVmVersion(JdwpRep_Vm_Version& vmVersion) {
		JdwpPacket jdwpPacket;
		jdwpPacket.setCommand(SET_VIRTUAL_MACHINE, ID_VM_VERSION);

		return waitJdwpResponse(&jdwpPacket, &vmVersion);
	}

	bool getVmIdSize(JdwpRep_Vm_IdSize& vmIdSize) {
		JdwpPacket jdwpPacket;
		jdwpPacket.setCommand(SET_VIRTUAL_MACHINE, ID_VM_ID_SIZES);

		return waitJdwpResponse(&jdwpPacket, &vmIdSize);
	}

	bool setVmResume() {
		JdwpPacket jdwpPacket;
		jdwpPacket.setCommand(SET_VIRTUAL_MACHINE, ID_VM_RESUME);

		return waitJdwpResponse(&jdwpPacket, NULL);
	}

	bool getVmClassesBySignature(JdwpRep_Vm_ClassesBySignature& vmClasses, string& jniSignature) {
		JdwpPacket jdwpPacket;
		jdwpPacket.setCommand(SET_VIRTUAL_MACHINE, ID_VM_CLASSES_BY_SIGNATURE);
		size_t signLen = jniSignature.length();
		jdwpPacket.appendBody((u1*)&signLen, sizeof(signLen));
		jdwpPacket.appendBody((u1*)jniSignature.c_str(), signLen);

		return waitJdwpResponse(&jdwpPacket, &vmClasses);
	}

	bool getVmAllClassesWithGeneric(JdwpRep_Vm_AllClassesWithGeneric& vmAllClasses) {
		JdwpPacket jdwpPacket;
		jdwpPacket.setCommand(SET_VIRTUAL_MACHINE, ID_VM_ALL_CLASSES_WITH_GENERIC);

		return waitJdwpResponse(&jdwpPacket, &vmAllClasses);
	}

	bool getRefTypeSignatureWithGeneric(JdwpRep_RefType_SignatureWithGeneric& signWithGeneric, RefTypeId refTypeId) {
		JdwpPacket jdwpPacket;
		jdwpPacket.setCommand(SET_REFERENCE_TYPE, ID_RT_SIGNATURE_WITH_GENERIC);
		jdwpPacket.appendBody((u1*)&refTypeId, sizeof(refTypeId));

		return waitJdwpResponse(&jdwpPacket, &signWithGeneric);
	}

	bool getRefTypeFieldsWithGeneric(JdwpRep_RefType_FieldsWithGeneric& fieldsWithGeneric, RefTypeId refTypeId) {
		JdwpPacket jdwpPacket;
		jdwpPacket.setCommand(SET_REFERENCE_TYPE, ID_RT_FIELDS_WITH_GENERIC);
		jdwpPacket.appendBody((u1*)&refTypeId, sizeof(refTypeId));

		return waitJdwpResponse(&jdwpPacket, &fieldsWithGeneric);
	}

	bool getRefTypeMethodsWithGeneric(JdwpRep_RefType_MethodsWithGeneric& methodsWithGeneric, RefTypeId refTypeId) {
		JdwpPacket jdwpPacket;
		jdwpPacket.setCommand(SET_REFERENCE_TYPE, ID_RT_METHODS_WITH_GENERIC);
		jdwpPacket.appendBody((u1*)&refTypeId, sizeof(refTypeId));

		return waitJdwpResponse(&jdwpPacket, &methodsWithGeneric);
	}

	bool getRefTypeGetValues(JdwpRep_RefType_GetValues& fieldValues, RefTypeId refTypeId, vector<JdwpField>& fieldsVec) {
		JdwpPacket jdwpPacket;
		jdwpPacket.setCommand(SET_REFERENCE_TYPE, ID_RT_GET_VALUES);
		jdwpPacket.appendBody((u1*)&refTypeId, sizeof(refTypeId));

		u4 uSize = fieldsVec.size();
		jdwpPacket.appendBody((u1*)&uSize, sizeof(uSize));
		for (size_t i = 0; i < fieldsVec.size(); i++) {
			jdwpPacket.appendBody((u1*)&fieldsVec[i].mFieldId, sizeof(FieldId));
		}
		return waitJdwpResponse(&jdwpPacket, &fieldValues);
	}

	bool getMethodLineTable(JdwpRep_Method_LineTable& methodLineTable, RefTypeId classId, MethodId methodId) {
		JdwpPacket jdwpPacket;
		jdwpPacket.setCommand(SET_METHOD, ID_M_LINE_TABLE);
		jdwpPacket.appendBody((u1*)&classId, sizeof(classId));
		jdwpPacket.appendBody((u1*)&methodId, sizeof(methodId));

		return waitJdwpResponse(&jdwpPacket, &methodLineTable);
	}

	bool getMethodVariableTableWithGeneric(JdwpRep_Method_VariableTableWithGeneric& variableTable, 
		RefTypeId classId, MethodId methodId) {
		JdwpPacket jdwpPacket;
		jdwpPacket.setCommand(SET_METHOD, ID_M_VARIABLE_TABLE_WITH_GENERIC);
		jdwpPacket.appendBody((u1*)&classId, sizeof(classId));
		jdwpPacket.appendBody((u1*)&methodId, sizeof(methodId));

		return waitJdwpResponse(&jdwpPacket, &variableTable);
	}

	bool setBreakPointEvent(JdwpRep_EventRequest_Set& repEventSet, RefTypeId classId, MethodId methodId, u8 codeIdx) {
		JdwpBreakPointEvent bp;
		bp.mEventKind = EK_BREAKPOINT;
		bp.mSuspendPolicy = SP_ALL; 
		bp.mModifiers = 0x01;
		bp.mModKind = MK_LOCATION_ONLY;
		bp.mLocation.mTypeTag = TT_CLASS;
		bp.mLocation.mClassId = classId;
		bp.mLocation.mMethodId = methodId;
		bp.mLocation.mDexPc = codeIdx;

		JdwpPacket jdwpPacket;
		jdwpPacket.setCommand(SET_EVENT_REQUEST, ID_ER_SET);
		jdwpPacket.appendBody((u1*)&bp.mEventKind, sizeof(bp.mEventKind));
		jdwpPacket.appendBody((u1*)&bp.mSuspendPolicy, sizeof(bp.mSuspendPolicy));
		jdwpPacket.appendBody((u1*)&bp.mModifiers, sizeof(bp.mModifiers));
		jdwpPacket.appendBody((u1*)&bp.mModKind, sizeof(bp.mModKind));
		jdwpPacket.appendBody((u1*)&bp.mLocation.mTypeTag, sizeof(bp.mLocation.mTypeTag));
		jdwpPacket.appendBody((u1*)&bp.mLocation.mClassId, sizeof(bp.mLocation.mClassId));
		jdwpPacket.appendBody((u1*)&bp.mLocation.mMethodId, sizeof(bp.mLocation.mMethodId));
		jdwpPacket.appendBody((u1*)&bp.mLocation.mDexPc, sizeof(bp.mLocation.mDexPc));

		return waitJdwpResponse(&jdwpPacket, &repEventSet);
	}

	bool getObjectReferenceRefType(JdwpRep_ObjectRef_RefType& refType, ObjectId objectId) {
		JdwpPacket jdwpPacket;
		jdwpPacket.setCommand(SET_OBJECT_REFERENCE, ID_OR_REFERENCE_TYPE);
		jdwpPacket.appendBody((u1*)&objectId, sizeof(objectId));
		return waitJdwpResponse(&jdwpPacket, &refType);
	}

	bool getObjectReferenceGetValues(JdwpRep_ObjectRef_GetValues& refValues, ObjectId objectId, vector<JdwpField>& fieldsVec) {
		JdwpPacket jdwpPacket;
		jdwpPacket.setCommand(SET_OBJECT_REFERENCE, ID_OR_GET_VALUES);
		jdwpPacket.appendBody((u1*)&objectId, sizeof(objectId));

		u4 uSize = fieldsVec.size();
		jdwpPacket.appendBody((u1*)&uSize, sizeof(uSize));
		for (size_t i = 0; i < fieldsVec.size(); i++) {
			jdwpPacket.appendBody((u1*)&fieldsVec[i].mFieldId, sizeof(FieldId));
		}
		return waitJdwpResponse(&jdwpPacket, &refValues);
	}

	bool getStringReferenceValue(JdwpRep_StringReference_Value& threadName, ObjectId ObjectId) {
		JdwpPacket jdwpPacket;
		jdwpPacket.setCommand(SET_STRING_REFERENCE, ID_SR_VALUE);
		jdwpPacket.appendBody((u1*)&ObjectId, sizeof(ObjectId));
		return waitJdwpResponse(&jdwpPacket, &threadName);
	}

	bool getThreadReferenceFrames(JdwpRep_ThreadReference_Frames& threadFrames, ObjectId threadId,
		u4 startFrame = 0, u4 length = -1) {
		JdwpPacket jdwpPacket;
		jdwpPacket.setCommand(SET_THREAD_REFERENCE, ID_TR_FRAMES);
		jdwpPacket.appendBody((u1*)&threadId, sizeof(threadId));
		jdwpPacket.appendBody((u1*)&startFrame, sizeof(startFrame));
		jdwpPacket.appendBody((u1*)&length, sizeof(length));

		return waitJdwpResponse(&jdwpPacket, &threadFrames);
	}

	bool getThreadReferenceName(JdwpRep_ThreadReference_Name& threadName, ObjectId threadId) {
		JdwpPacket jdwpPacket;
		jdwpPacket.setCommand(SET_THREAD_REFERENCE, ID_TR_NAME);
		jdwpPacket.appendBody((u1*)&threadId, sizeof(threadId));
		return waitJdwpResponse(&jdwpPacket, &threadName);
	}

	bool setThreadReferenceResume(ObjectId threadId) {
		JdwpPacket jdwpPacket;
		jdwpPacket.setCommand(SET_THREAD_REFERENCE, ID_TR_RESUME);
		jdwpPacket.appendBody((u1*)&threadId, sizeof(threadId));
		return waitJdwpResponse(&jdwpPacket, NULL);
	}

	bool getThreadReferenceStatus(JdwpRep_ThreadReference_Status& threadStatus, ObjectId threadId) {
		JdwpPacket jdwpPacket;
		jdwpPacket.setCommand(SET_THREAD_REFERENCE, ID_TR_STATUS);
		jdwpPacket.appendBody((u1*)&threadId, sizeof(threadId));
		return waitJdwpResponse(&jdwpPacket, &threadStatus);
	}

	bool getArrayReferenceLength(JdwpRep_ArrayReference_Length& arrayLength, ObjectId objectID) {
		JdwpPacket jdwpPacket;
		jdwpPacket.setCommand(SET_ARRAY_REFERENCE, ID_AR_LENGTH);
		jdwpPacket.appendBody((u1*)&objectID, sizeof(objectID));
		return waitJdwpResponse(&jdwpPacket, &arrayLength);
	}

	bool getArrayReferenceValues(JdwpRep_ArrayReference_Values& arrayValues, ObjectId objectID, u4 firstIdx, u4 arrayLength) {
		JdwpPacket jdwpPacket;
		jdwpPacket.setCommand(SET_ARRAY_REFERENCE, ID_AR_GET_VALUES);
		jdwpPacket.appendBody((u1*)&objectID, sizeof(objectID));
		jdwpPacket.appendBody((u1*)&firstIdx, sizeof(firstIdx));
		jdwpPacket.appendBody((u1*)&arrayLength, sizeof(arrayLength));
		return waitJdwpResponse(&jdwpPacket, &arrayValues);
	}

	bool getStackFrameValues(JdwpRep_StackFrame_GetValues& values, ObjectId threadId, ObjectId frameID, 
		vector<JdwpVariable>& paramVec) {
		JdwpPacket jdwpPacket;
		jdwpPacket.setCommand(SET_STACK_FRAME, ID_SF_GET_VALUES);
		jdwpPacket.appendBody((u1*)&threadId, sizeof(threadId));
		jdwpPacket.appendBody((u1*)&frameID, sizeof(frameID));
		
		u4 slots = paramVec.size();
		jdwpPacket.appendBody((u1*)&slots, sizeof(slots));
		for (size_t i = 0; i < paramVec.size(); i++) {
			jdwpPacket.appendBody((u1*)&paramVec[i].mSlot, sizeof(u4));
			jdwpPacket.appendBody((u1*)&paramVec[i].mValue.mType, sizeof(u1));
		}

		return waitJdwpResponse(&jdwpPacket, &values);
	}

	bool getStackFrameThisObject(JdwpRep_StackFrame_ThisObject& thisObject, ObjectId threadId, ObjectId frameID) {
		JdwpPacket jdwpPacket;
		jdwpPacket.setCommand(SET_STACK_FRAME, ID_SF_THIS_OBJECT);
		jdwpPacket.appendBody((u1*)&threadId, sizeof(threadId));
		jdwpPacket.appendBody((u1*)&frameID, sizeof(frameID));

		return waitJdwpResponse(&jdwpPacket, &thisObject);
	}

	void handleReplyPacket(JdwpPacket* jdwpPacket) {
		u4 packetId = jdwpPacket->getId();
		std::lock_guard<std::mutex> lk(mReplyMutex);
		std::map<u4, JdwpReplyNotify*>::iterator it = mReplyNotifyMap.find(packetId);
		if (it != mReplyNotifyMap.end()) {
			JdwpReplyNotify*& reqReply = it->second;
			reqReply->mPacket = jdwpPacket;
			::SetEvent(reqReply->mHandle);
		}
	}

	void handleCommandPacket(JdwpPacket* jdwpPacket) {
		std::lock_guard<std::mutex> lk(mCommandMutex);
		mCommandPacketQueue.push_back(jdwpPacket);
	}

	void runWorker() {
		std::thread recvThread(&JdwpProtocol::workRoutine, this);
		recvThread.detach();
	}

	void workRoutine() {
		while (true) {
			JdwpPacket* jdwpPacket = mJdwpSocket.getNextPacket();
			if (jdwpPacket) {
				switch (jdwpPacket->getType()) {
				case PACKET_TYPE_REPLY: {
					handleReplyPacket(jdwpPacket);
					break;
				}
				case PACKET_TYPE_COMMAND: {
					handleCommandPacket(jdwpPacket);
					break;
				}
				default:
					std::cout << "[err]: unknown packet -> packetId:" << jdwpPacket->getId() << endl;
					break;
				}
			}
			else {
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}
		}
	}

	virtual void handleEventNotify(JdwpRep_Event_Composite& Event) {};

	JdwpPacket* getNextCommandPacket() {
		std::lock_guard<std::mutex> lk(mCommandMutex);
		if (mCommandPacketQueue.size())
		{
			JdwpPacket* packet = mCommandPacketQueue.front();
			mCommandPacketQueue.pop_front();
			return packet;
		}
		return nullptr;
	}

	void runEventLoop() {

		while (true) {
			JdwpPacket* jdwpPacket = getNextCommandPacket();
			if (jdwpPacket) {
				// ? Event Command Set(64) / Composite Command(100)
				JdwpRep_Event_Composite repEvents;
				repEvents.unserialize(jdwpPacket->getBody(), jdwpPacket->getBodySize());

				handleEventNotify(repEvents);

				if (repEvents.mSuspendPolicy == SP_ALL)
					setVmResume();

				delete jdwpPacket;
			} else {
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}
		}
	}


private:
	jdwpSocket                       mJdwpSocket;
	std::map<u4, JdwpReplyNotify*>   mReplyNotifyMap;
	std::mutex                       mReplyMutex;
	std::list<JdwpPacket*>           mCommandPacketQueue;
	std::mutex                       mCommandMutex;
};