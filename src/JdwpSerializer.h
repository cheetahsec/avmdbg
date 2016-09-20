#pragma once

#include <string>
using namespace std;


class JdwpSerializer
{
public:
	JdwpSerializer() {};
	JdwpSerializer(u1* buffer, u4 size) {
		mBuffer = buffer;
		mSize = size;
		mPos = mBuffer;
	}
	~JdwpSerializer() { }

	bool read(u1& v) {
		if (mPos + sizeof(u1) > mBuffer + mSize)
			return false;
		v = *(u1*)mPos;;
		mPos += sizeof(u1);
		return true;
	}

	bool read(u2& v) {
		if (mPos + sizeof(u2) > mBuffer + mSize)
			return false;
		u2* _cur = (u2*)mPos;
		v = Tranverse16(*_cur);
		mPos += sizeof(u2);
		return true;
	}

	bool read(u4& v) {
		if (mPos + sizeof(u4) > mBuffer + mSize)
			return false;
		u4* _cur = (u4*)mPos;
		v = Tranverse32(*_cur);
		mPos += sizeof(u4);
		return true;
	}

	bool read(u8& v) {
		if (mPos + sizeof(u8) > mBuffer + mSize)
			return false;
		v = Tranverse64(*(u8*)mPos);
		mPos += sizeof(u8);
		return true;
	}

	bool read(std::string &v) {
		u4* _cur = (u4*)mPos;
		u4 len = Tranverse32(_cur[0]);
		if (mPos + sizeof(u4) + len > mBuffer + mSize)
			return false;
		mPos += sizeof(u4);
		v.append((char*)mPos, len);
		mPos += len;
		return true;
	}
public:
	u1*    mBuffer;
	u4    mSize;
	u1*    mPos;
};