// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"

#include "JdwpProtocol.h"

//#include "log.h"
//INITIALIZE_EASYLOGGINGPP

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}


#include "pybind11/pybind11.h"
#include "AVMDebugger.h"

PYBIND11_PLUGIN(avmdbg) {
	pybind11::module m("avmdbg", "a lightweight debugger for android virtual machine.");

	pybind11::class_<AvmDebugger>(m, "AvmDebugger")
		.def(pybind11::init<>())
		.def("attach", &AvmDebugger::attach)
		.def("waitLoop", &AvmDebugger::waitLoop)
		.def("setBreakPoint", &AvmDebugger::setBreakPoint)
		.def("getStackFrames", &AvmDebugger::getStackFrames)
		.def("getRegisterValue", &AvmDebugger::getRegisterValue)
		.def("getStringValue", &AvmDebugger::getStringValue)
		.def("getObjectFieldValues", &AvmDebugger::getObjectFieldValues)
		.def("getArrayObjectValue", &AvmDebugger::getArrayObjectValue);

	return m.ptr();
}