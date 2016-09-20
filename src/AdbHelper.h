#pragma once

#include <Windows.h>
#include <iostream>
#include <string>
#include <map>
#include <vector>
using namespace std;


class AdbHelper
{
public:
	AdbHelper();
	~AdbHelper();
public:
	bool AdbHelper::getDebugableApps(map<string, string>* result) {
		vector<string> jdwpPids;
		cmd_jdwp(&jdwpPids);

		vector<string> psLines;
		cmd_shell_ps(&psLines);

		vector<string>::iterator it = psLines.begin();
		for (; it != psLines.end(); it++) {
			vector<string> info;
			split(*it, " ", &info);
			vector<string>::iterator it = find(jdwpPids.begin(), jdwpPids.end(), info[1]);
			if (it != jdwpPids.end())
				result->insert(make_pair(info[8], info[1]));
		}

		return result->size() > 0;
	}

	bool AdbHelper::cmd_jdwp(vector<string>* jdwpPids) {
		std::string outStr;
		if (execute(string("adb jdwp"), &outStr)) {
			split(outStr, "\r\n", jdwpPids);
			return jdwpPids->size() > 0;
		}
		return false;
	}

	bool AdbHelper::cmd_shell_ps(vector<string>* retLines) {
		std::string outStr;
		if (execute(string("adb shell ps"), &outStr)) {
			split(outStr, "\r\r\n", retLines);
			if (retLines->size() > 1)
				retLines->erase(retLines->begin());
			return retLines->size() > 0;
		}
		return false;
	}

	bool AdbHelper::cmd_forword_jdwp(string port, string pid) {
		return execute(string("adb forward tcp:") + port + string(" jdwp:") + pid, NULL);
	}

	bool AdbHelper::execute(std::string cmdStr, std::string* pOutStr) {
		bool bRet = false;
		HANDLE hPipeRead = NULL;
		HANDLE hPipeWrite = NULL;
		DWORD dwRead = 0;
		DWORD dwFileSize = 0;
		CHAR* pszBuffer = NULL;
		PROCESS_INFORMATION pi = { 0 };
		STARTUPINFO si = { 0 };
		SECURITY_ATTRIBUTES sa = { 0 };

		sa.bInheritHandle = TRUE;
		sa.nLength = sizeof(SECURITY_ATTRIBUTES);

		if (!::CreatePipe(&hPipeRead, &hPipeWrite, &sa, 1024*64))
			return false;
		::SetHandleInformation(hPipeRead, HANDLE_FLAG_INHERIT, 0);

		si.cb = sizeof(STARTUPINFO);
		si.hStdOutput = hPipeWrite;
		si.hStdError = hPipeWrite;
		si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
		si.wShowWindow = SW_HIDE;

		wchar_t cmdLine[MAX_PATH] = { 0 };
		std::wstring wCmdStr(cmdStr.length(), L' ');
		std::copy(cmdStr.begin(), cmdStr.end(), wCmdStr.begin());
		wcscpy_s(cmdLine, MAX_PATH, wCmdStr.c_str());

		if (::CreateProcess(NULL, cmdLine, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
			::WaitForSingleObject(pi.hProcess, 3000);
			::CloseHandle(pi.hProcess);
			::CloseHandle(pi.hThread);

			if (pOutStr != NULL) {
				dwFileSize = ::GetFileSize(hPipeRead, NULL);
				pszBuffer = new CHAR[dwFileSize + sizeof(CHAR)];
				if (NULL != pszBuffer) {
					memset(pszBuffer, 0, dwFileSize + sizeof(CHAR));
					::ReadFile(hPipeRead, pszBuffer, dwFileSize, &dwRead, NULL);
					pOutStr->append(pszBuffer);
					delete[] pszBuffer;
					bRet = true;
				}
			}
		}

		::CloseHandle(hPipeRead);
		::CloseHandle(hPipeWrite);
		return bRet;
	}

private:
	static void split(const string str, const string delim, vector<string>* ret) {
		size_t last = 0;
		size_t index = str.find_first_of(delim, last);
		while (index != -1) {
			string tmp = str.substr(last, index - last);
			if (tmp.length() > 0)
				ret->push_back(tmp);
			last = index + delim.length();
			index = str.find_first_of(delim, last);
		}
		if (str.length() > last)
			ret->push_back(str.substr(last));
	}

private:
	std::string  m_adbPath;
	HANDLE       m_pipe;
	HANDLE       m_hRead;
	HANDLE       m_hWrite;
};