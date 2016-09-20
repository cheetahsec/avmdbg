#include "JdwpValue.h"


eJdwpType getParamType(const string& param) {
	switch (param.at(0)) {
	case '[':
		return JT_ARRAY;
	case 'B':
		return JT_BYTE;
	case 'C':
		return JT_CHAR;
	case 'L':
		return JT_OBJECT;
	case 'F':
		return JT_FLOAT;
	case 'D':
		return JT_DOUBLE;
	case 'I':
		return JT_INT;
	case 'J':
		return JT_LONG;
	case 'S':
		return JT_SHORT;
	case 'Z':
		return JT_BOOLEAN;
	default:
		return JT_UNKNOWN;
	}
}

/*
	In dalvik's bytecode, registers are always 32 bits, and can hold any type of value.
	2 registers are used to hold 64 bit types (Long and Double).
	https://github.com/JesusFreke/smali/wiki/Registers
*/
size_t getRegisterWide(u1 type) {
	switch (type) {
	case JT_BYTE:
	case JT_BOOLEAN:
	case JT_CHAR:
	case JT_SHORT:
	case JT_FLOAT:
	case JT_INT:
	case JT_OBJECT:
	case JT_ARRAY:
		return 1;
	case JT_DOUBLE:
	case JT_LONG:
		return 2;
	default:
		return 0;
	}
}

size_t findTypeEnd(const string& param, size_t index) {
	if (index >= param.length())
		return 0;
	switch (param.at(index)) {
	case 'Z':
	case 'B':
	case 'S':
	case 'C':
	case 'I':
	case 'J':
	case 'F':
	case 'D':
		return index + 1;
	case 'L':
		while (param.at(index++) != ';') {
			if (index >= param.length())
				return 0;
		}
		return index;
	case '[':
		while (param.at(index++) != '[') {
			if (index >= param.length())
				return 0;
		}
		return findTypeEnd(param, index);
	default:
		cout << "invalid param string : " << param << endl;
	}
	return 0;
}

string getParamSign(const string& funcSign) {
	int begin = funcSign.find("(");
	int end = funcSign.find(")");
	if (begin != string::npos && end != string::npos && begin < end) {
		return funcSign.substr(begin + 1, end - begin - 1);
	}
	return "";
}

bool splitsParam(const string& paramStr, vector<string>& paramVec) {
	size_t index = 0;
	paramVec.clear();
	if (paramStr.length() == 0)
		return true;
	while (index < paramStr.length()) {
		size_t position = findTypeEnd(paramStr, index);
		if (!position)
			return false;
		paramVec.push_back(paramStr.substr(index, position - index));
		index = position;
	}
	return true;
}

bool analysisParam(const string& paramStr, size_t registerCount, bool bStatic, vector<JdwpVariable>& result) {
	vector<string> paramVec;
	result.clear();

	if (splitsParam(paramStr, paramVec)) {
		// 计算参数占用的总长度
		u4 rCount = 0;
		u4 rIdx = 0;
		for (size_t i = 0; i < paramVec.size(); i++) {
			eJdwpType type = getParamType(paramVec[i]);
			if (type == JT_UNKNOWN) {
				return false;
			}
			rCount += getRegisterWide(type);
		}

		if (rCount > registerCount)
			return false;

		size_t curSlot = registerCount - rCount;
		if (!bStatic) {
			JdwpVariable param;
			param.mValue.mType = JT_OBJECT;
			param.mValue._object.val = 0;
			param.mSign = "this";
			param.mName = "p0";
			param.mSlot = curSlot - 1;
			result.push_back(param);
			rIdx += 1;
		}

		// 分配寄存器
		for (size_t i = 0; i < paramVec.size(); i++) {
			eJdwpType type = getParamType(paramVec[i]);
			if (type == JT_UNKNOWN) {
				result.clear();
				return false;
			}

			JdwpVariable param;
			param.mValue.mType = type;
			param.mSign = paramVec[i];
			param.mSlot = curSlot;
			char rName[32] = { 0 };
			sprintf_s(rName, 32, "p%d", rIdx);
			param.mName = string(rName);
			result.push_back(param);

			size_t wide = getRegisterWide(type);
			curSlot += wide;
			rIdx += wide;
		}
		return true;
	}
	return false;
}