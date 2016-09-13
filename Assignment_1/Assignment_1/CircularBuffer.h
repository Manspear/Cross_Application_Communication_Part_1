#pragma once
#include <Windows.h>
#include <stdlib.h>
#include <cstdio>
#include <iostream>
#include <vector>
#include "FileMapStructs.h"
#include "Mutex.h"
class circularBuffer
{
private:
	struct sLTail
	{
		size_t lDiff, lOldDiff, lPos;
	};
	LPCWSTR msgBuffName;
	const size_t * buffSize;
	int role;
	const size_t * chunkSize;

	HANDLE msgFileMap;
	char* msgBuff;
	HANDLE varFileMap;
	sSharedVars* varBuff;
	size_t clientCount;
	size_t msgCounter;

	sLTail lTail;

	Mutex mutex1;

	bool procMsg(char * msg, size_t * length);
	bool pushMsg(bool reset, bool start, const void * msg, size_t & length);

public:
	size_t sharedMemSize;
	size_t sleepTime;

	/*
	Creates the shared memory space for messages
	role here is (for the moment) unneccesary
	*/
	void initCircBuffer(
		LPCWSTR msgBuffName, 
		const size_t& buffSize, 
		const int& role,
		const size_t& chunkSize, // round up messages to multiple of this.
		LPCWSTR varBuffName, 
		size_t numberOfClients
	);

	void stopCircBuffer();

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