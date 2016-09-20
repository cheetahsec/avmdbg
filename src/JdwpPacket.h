#pragma once

#include <vector>
using namespace std;

#include "Jdwp.h"

/*
* JDWP message header
*/
#pragma pack (1)
typedef struct JdwpPacketHeader {
	u4  length;
	u4  id;
	u1  flags;
	union {
		u2 errorCode;
		struct {
			u1 cmdSet;
			u1 cmdId;
		};
	};
} JdwpPacketHeader;
#pragma pack ()

class JdwpPacket
{
	static u4 stPacketId;
public:
	JdwpPacket(JdwpPacketHeader* Header);
	JdwpPacket();
	~JdwpPacket();
public:

	void setCommand(eJdwpCommandSet cmdSet, eJdwpCommandId cmdId);

	void appendBody(const u1* Data, u4 Size);

	JdwpPacketHeader* getHeader();

	unsigned char* getBody();

	u4 getBodySize();

	u1* getBuffer();

	u4 getBufferSize();

	u4 getId();

	eJdwpPacketType getType();

	static u4 makePacketId();

	string toString();

private:
	std::vector<u1>  mBuffer;
};

