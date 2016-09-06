#pragma once
#include <Windows.h>
#include <stdlib.h>
#include <cstdio>
#include <iostream>
#define SHAREDMEMSIZE 1024
#define SLEEPTIME 1
class circularBuffer
{
private:
	struct messageHeader
	{
		size_t id;
		size_t length;
		size_t padding;
		size_t consumerPile;
	};
	struct messageStruct
	{
		messageHeader header;
		char charoo[30];
	};
	int delay;
	int numMessages;
	
	int role;
	const size_t * chunkSize;
	LPCWSTR buffName;
	const size_t * buffSize;
public:
	enum {
		PRODUCER = 0,
		CONSUMER = 1,
		RANDOM = 0,
		MSGSIZE = 1
	};

	size_t sharedMemSize;
	size_t sleepTime;
	/*
	Creates message. Testing function.
	*/
	void* makeMessage();
	/*
	Align message
	*/
	//size_t alignMessage(messageStruct);
	/*
	Creates the shared memory space
	*/
	void initCircBuffer(
		LPCWSTR buffName,          // unique name
		const size_t& buffSize,    // size of the whole filemap
		const int& role,    // is this buffer going to be used as producer == 0 or consumer == 1
		const size_t& chunkSize, // round up messages to multiple of this.
		int& delay, 
		int& numMessages, 
		int& msgSizeMode);
	/*
	Starts and runs the inifnite circBuffer-loop
	*/
	void runCircBuffer();
	/*
	push function used by the producer. The producer attempts to write data to the 
	FileMap. Specifically it tries to write a message and append it to the Shared Memory (FileMap)

	*/
	bool push(const void* msg, size_t length);
	/*
	pop function used by the consumer. The consumer attempts to read data from the FileMap.
	Upon successful reading it uses that data to do something.
	*/
	bool pop(char* msg, size_t& length);

	circularBuffer();
	~circularBuffer();
};