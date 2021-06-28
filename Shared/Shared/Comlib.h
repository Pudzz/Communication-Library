#pragma once
#include <iostream>
#include <Windows.h>
#include <string>
#include <time.h>
#include <shared_mutex>


enum TYPE { PRODUCER, CONSUMER };

struct MsgHeader {
	size_t msgSize;
};

struct ControlHeader {
	size_t offsetPro = 0;
	size_t offsetCon = 0;

	size_t producerSent = 0;
	size_t consumerRead = 0;


	size_t processID = 0;
	// 0 = normal, both is running normal
	// 1 = producer restart, and consumer checks backwards
	// 2 = producer produces a new message, consumer waits for a 0 ID
	// producer can have 0 and 2 to run the program || consumer can have 0 och 1 to run the program		

	// New
	size_t firstMessage = 0;
};

class Comlib {
public:
	Comlib(const std::string& sharedName, const std::string& mutexName, const size_t& bufferSize, TYPE type);
	~Comlib();

	bool SendMsg(MsgHeader* header, ControlHeader* ctrl, const void* msg, size_t& offset);
	bool RecieveMsg(MsgHeader* readMsg, ControlHeader* ctrl, char* msg, size_t& offset);

private:
	TYPE process;
	HANDLE hFilemap;
	HANDLE hCtrlmap;
	HANDLE hMutex;

	std::string smName;
	std::string smCtrlName;
	std::string smMutexName;

	void* msgBuffer;
	void* ctrlBuffer;

	size_t smBufferSize;
	size_t ctrlBufferSize;

	/* For protocol */	
	bool restarting = false;

	/* For closing handle, fileview */
	bool exists = false;

	bool reachedEnd = false;
};
