#pragma once

#include "JdwpProtocol.h"

#include "pybind11/pybind11.h"
namespace py = pybind11;

typedef std::function<void(py::dict)> EventCallBack;

struct EventContext
{
	EventCallBack mCallBack;
	py::dict      mReqExt;
};

class AvmDebugger : public JdwpProtocol
{
public:
	AvmDebugger();
	~AvmDebugger();

public:
	bool attach(string& processName);

	void waitLoop();

	bool setBreakPoint(py::dict& bp);

	py::list getStackFrames(ObjectId threadId);

	py::str getStringValue(ObjectId objectId);

	py::list getObjectFieldValues(ObjectId objectId);

	py::dict getArrayObjectValue(ObjectId objectId);

	py::dict getRegisterValue(py::dict& Context, string& regName, u1 varType);

private:

	py::list getAllClasses();

	py::list getClassesBySignature(string& classSign);

	virtual void handleEventNotify(JdwpRep_Event_Composite& eventNotify);

	void registerEventNotify(u4 uRequestId, EventContext eventContext);

	bool getEventContext(u4 uRequestId, EventContext& eventContext);

	void formatJdwpValue(JdwpValue& val, py::dict& item);

private:
	std::map<u4, EventContext>  mEventNotifyMap;
	std::mutex                  mEventMutex;
};
