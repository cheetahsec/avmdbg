#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

#include <stdexcept>
#include <thread>
#include <mutex>
#include <list>
#include <iostream>
using namespace std;

#include "JdwpPacket.h"

class jdwpSocket {
public:
	void Init() {
		WSADATA wsa;
		if (::WSAStartup(514, &wsa) != 0) {
			throw std::runtime_error("WSAStartup failed.");
		}
	}

	jdwpSocket() {
		Init();
		if ((mSocket = ::socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
			throw std::runtime_error("Failed to create socket.");
		}
	}

	bool Connect(const char* address, int port) {
		sockaddr_in addr;
		addr.sin_addr.s_addr = inet_addr(address);
		addr.sin_family = AF_INET;
		addr.sin_port = ::htons(port);
		if (::connect(mSocket, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
			throw std::runtime_error("Failed to connect.");
		}
		return true;
	}

	bool Send(const char* buffer, int size) {
		return ::send(mSocket, buffer, size, 0) > 0;
	}

	bool Recv(char* buffer, int size) {
		return ::recv(mSocket, buffer, size, 0) > 0;
	}

	void runRecvWorker() {
		std::thread recvThread(&jdwpSocket::recvPacket, this);
		recvThread.detach();
	}

	JdwpPacket* getNextPacket() {
		std::lock_guard<std::mutex> lk(mQueueMutex);
		if (mPacketQueue.size())
		{
			JdwpPacket* packet = mPacketQueue.front();
			//std::cout << "mPacketQueue Pop PacketID:" << packet->getPacketId() << endl;
			mPacketQueue.pop_front();
			return packet;
		}
		return 0;
	}

	bool RecvN(char* buffer, int size) {
		int idx = 0;
		while (true)
		{
			int ret = recv(mSocket, buffer + idx, size - idx, 0);
			if (ret <= 0) {
				std::cout << "recv packet fail. LastError:" << WSAGetLastError() << endl;
				return false;
			}
			idx += ret;
			if (idx == size)
				return true;
		}
	}

	void recvPacket()
	{
		while (true)
		{
			JdwpPacketHeader packetHeader;
			memset(&packetHeader, 0, sizeof(packetHeader));

			if (!RecvN((char*)&packetHeader, sizeof(JdwpPacketHeader)))
				break;

			if (Tranverse32(packetHeader.length) == sizeof(JdwpPacketHeader))
				continue;

			JdwpPacket* jdwpPacket = new JdwpPacket(&packetHeader);
			size_t bodySize = jdwpPacket->getBodySize();
			char* bodyBuffer = (char*)jdwpPacket->getBody();
			if (bodyBuffer && bodySize)
				RecvN(bodyBuffer, bodySize);

			std::lock_guard<std::mutex> lk(mQueueMutex);
			mPacketQueue.push_back(jdwpPacket);
		}
	}

	~jdwpSocket() {
		::closesocket(mSocket);
		mSocket = INVALID_SOCKET;
		mPacketQueue.clear();
	}
private:
	SOCKET                  mSocket;
	std::mutex              mQueueMutex;
	std::list<JdwpPacket*>  mPacketQueue;
};