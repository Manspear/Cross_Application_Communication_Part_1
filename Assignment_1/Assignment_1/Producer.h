#pragma once
#include <Windows.h>
#include <stdlib.h>
#include <cstdio>
#include <iostream>
#include "CircularBuffer.h"
#include "FileMapStructs.h"
class Producer
{
private:
	int localStep;
	int localDiff;
	int localOldDiff;
	int memorySteps;

	int delay;
	int requestedMessages;
	int msgSizeMode;
	int maxMsgSize;
	int messageCount;

	int numMessages;

public:
	Producer();
	Producer(int & delay, 
			 int & numMessages, 
			 size_t & maxMsgSize, 
			 size_t & buffSize, 
			 int & chunkSize, 
			 LPCWSTR varBuffName);
	~Producer();
	void * makeMessage();
	void runProducer(circularBuffer& buffInst);
};