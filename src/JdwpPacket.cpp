#include "stdafx.h"
#include "JdwpPacket.h"

#include <sstream>

u4 JdwpPacket::stPacketId = 1;

JdwpPacket::JdwpPacket(JdwpPacketHeader* Header) {
	u4 uSize = Tranverse32(Header->length);
	mBuffer.resize(uSize);
	memset(mBuffer.data(), 0, uSize);
	memcpy(mBuffer.data(), Header, sizeof(JdwpPacketHeader));
}

JdwpPacket::JdwpPacket() {
	mBuffer.resize(sizeof(JdwpPacketHeader));
	memset(mBuffer.data(), 0, sizeof(JdwpPacketHeader));
}

JdwpPacket::~JdwpPacket() {

}

void JdwpPacket::setCommand(eJdwpCommandSet cmdSet, eJdwpCommandId cmdId) {
	JdwpPacketHeader* header = (JdwpPacketHeader*)mBuffer.data();
	u4 Length = sizeof(JdwpPacketHeader);
	u4 packetId = JdwpPacket::makePacketId();
	header->length = Tranverse32(Length);
	header->id = Tranverse32(packetId);
	header->flags = 0x00;
	header->cmdSet = cmdSet;
	header->cmdId = cmdId;
}

void JdwpPacket::appendBody(const u1* Data, u4 Size) {
	vector<u1> Temp(Data, Data + Size);
	if (Size == 2 || Size == 4 || Size == 8)
		std::reverse(Temp.begin(), Temp.end());
	mBuffer.insert(mBuffer.end(), Temp.begin(), Temp.end());
	JdwpPacketHeader* Header = (JdwpPacketHeader*)mBuffer.data();
	Header->length = Tranverse32(mBuffer.size());
}

JdwpPacketHeader* JdwpPacket::getHeader() {
	if (mBuffer.size() >= sizeof(JdwpPacketHeader)) {
		return (JdwpPacketHeader*)mBuffer.data();
	}
	return 0;
}

u1* JdwpPacket::getBody() {
	if (mBuffer.size() > sizeof(JdwpPacketHeader))
		return (unsigned char*)mBuffer.data() + sizeof(JdwpPacketHeader);
	return 0;
}

size_t JdwpPacket::getBodySize() {
	if (mBuffer.size() > sizeof(JdwpPacketHeader))
		return mBuffer.size() - sizeof(JdwpPacketHeader);
	return 0;
}

u1* JdwpPacket::getBuffer() {
	return mBuffer.data();
}

size_t JdwpPacket::getBufferSize() {
	return mBuffer.size();
}

u4 JdwpPacket::getId() {
	if (mBuffer.size() >= sizeof(JdwpPacketHeader)) {
		JdwpPacketHeader* Header = (JdwpPacketHeader*)mBuffer.data();
		return Tranverse32(Header->id);
	}
	return 0;
}

eJdwpPacketType JdwpPacket::getType() {
	if (mBuffer.size() >= sizeof(JdwpPacketHeader)) {
		JdwpPacketHeader* header = (JdwpPacketHeader*)mBuffer.data();
		if (header->flags == 0x80)
			return PACKET_TYPE_REPLY;
		if (header->flags == 0x00)
			return PACKET_TYPE_COMMAND;
	}
	return PACKET_TYPE_UNKNOWN;
}

u4 JdwpPacket::makePacketId() {
	return JdwpPacket::stPacketId++;
}

string JdwpPacket::toString() {
	std::stringstream stream;
	if (mBuffer.size() >= sizeof(JdwpPacketHeader)) {
		JdwpPacketHeader* Header = (JdwpPacketHeader*)mBuffer.data();
		u4 Length = Tranverse32(Header->length);
		u4 Id = Tranverse32(Header->id);
		u1 cmdset = Header->cmdSet;
		u1 cmdid = Header->cmdId;
		u2 errCode = Tranverse16(Header->errorCode);
		if (Header->flags == 0x80)
			stream << "[JdwpPacket] ==> type=REPLY | len=" << Length << " | id=" << Id
				<< " | err=" << errCode << endl;
		if (Header->flags == 0x00)
			stream << "[JdwpPacket] ==> type=COMMAND | len=" << Length << " | id=" << Id
				<< " | cmdset=" << cmdset << " |cmdid=" << cmdid << endl;
		return stream.str();
	}
	return string("");
}
