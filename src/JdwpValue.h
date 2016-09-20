#pragma once

#include "jdwp.h"

#include <string>
#include <vector>
#include <iostream>
using namespace std;

/*
	ARRAY			91		'[' - an array object(objectID size).
	BYTE			66		'B' - a byte value(1 byte).
	CHAR			67		'C' - a character value(2 bytes).
	OBJECT			76		'L' - an object(objectID size).
	FLOAT			70		'F' - a float value(4 bytes).
	DOUBLE			68		'D' - a double value(8 bytes).
	INT	    		73		'I' - an int value(4 bytes).
	LONG			74		'J' - a long value(8 bytes).
	SHORT			83		'S' - a short value(2 bytes).
	VOID			86		'V' - a void value(no bytes).
	BOOLEAN			90		'Z' - a boolean value(1 byte).
	STRING			115		's' - a String object(objectID size).
	THREAD			116		't' - a Thread object(objectID size).
	THREAD_GROUP	103		'g' - a ThreadGroup object(objectID size).
	CLASS_LOADER	108		'l' - a ClassLoader object(objectID size).
	CLASS_OBJECT	99		'c' - a class object object(objectID size)
*/

union JdwpValue
{
	u1   mType;
	struct JValueArry {
		eJdwpType _t;
		ObjectId val;
	} _arry;

	struct JValueByte {
		eJdwpType  _t;
		u1  val;
	} _byte;

	struct JValueChar {
		eJdwpType   _t;
		u2  val;
	} _char;

	struct JValueObject {
		eJdwpType   _t;
		ObjectId  val;
	} _object;

	struct JValueFloat{
		eJdwpType   _t;
		float  val;
	} _float;

	struct JValueDouble{
		eJdwpType   _t;
		double  val;
	} _double;

	struct JValueInt{
		eJdwpType   _t;
		u4  val;
	} _int;

	struct JValueLong{
		eJdwpType   _t;
		u8  val;
	} _long;

	struct JValueShort{
		eJdwpType   _t;
		u2  val;
	} _short;

	struct JValueVoid{
		eJdwpType   _t;
	} _void;

	struct JValueBoolean{
		eJdwpType   _t;
		bool  val;
	} _boolean;

	struct JValueString{
		eJdwpType   _t;
		ObjectId  val;
	} _string;

	struct JValueThread{
		eJdwpType   _t;
		ObjectId  val;
	} _thread;

	struct JValueThreadGroup{
		eJdwpType   _t;
		ObjectId  val;
	} _threadgroup;

	struct JValueClassLoader{
		eJdwpType   _t;
		ObjectId  val;
	} _classloader;

	struct JValueClassObject {
		eJdwpType   _t;
		ObjectId  val;
	} _classobject;

public:
	JdwpValue() {}
	JdwpValue(u1 type) { 
		mType = type;
	}
	u4 getValueSize() {
		switch (mType) {
		case JT_VOID:
			return 0;
		case JT_BYTE:
		case JT_BOOLEAN:
			return 1;
		case JT_CHAR:
		case JT_SHORT:
			return 2;
		case JT_FLOAT:
		case JT_INT:
			return 4;
		case JT_DOUBLE:
		case JT_LONG:
			return 8;
		case JT_OBJECT:
		case JT_ARRAY:
		case JT_STRING:
		case JT_THREAD:
		case JT_THREAD_GROUP:
		case JT_CLASS_LOADER:
		case JT_CLASS_OBJECT:
			return sizeof(ObjectId);
		default:
			return -1;
		}
	}
	const char* getTypeName() {
		switch (mType) {
		case JT_VOID:
			return "void";
		case JT_BYTE:
			return "byte";
		case JT_BOOLEAN:
			return "boolean";
		case JT_CHAR:
			return "char";
		case JT_SHORT:
			return "short";
		case JT_FLOAT:
			return "float";
		case JT_INT:
			return "int";
		case JT_DOUBLE:
			return "double";
		case JT_LONG:
			return "long";
		case JT_OBJECT:
			return "object";
		case JT_ARRAY:
			return "array";
		case JT_STRING:
			return "string";
		case JT_THREAD:
			return "thread";
		case JT_THREAD_GROUP:
			return "threadGroup";
		case JT_CLASS_LOADER:
			return "classLoader";
		case JT_CLASS_OBJECT:
			return "classObject";
		default:
			return "unknown";
		}
	}
};

struct JdwpVariable
{
	string    mName;
	size_t    mSlot;
	string    mSign;
	JdwpValue mValue;
};

string getParamSign(const string& funcSign);
eJdwpType getParamType(const string& param);
size_t getRegisterWide(u1 type);
size_t findTypeEnd(const string& param, size_t index);
bool splitsParam(const string& paramStr, vector<string>& paramVec);
bool analysisParam(const string& paramStr, size_t registerCount, bool bStatic, vector<JdwpVariable>& result);
