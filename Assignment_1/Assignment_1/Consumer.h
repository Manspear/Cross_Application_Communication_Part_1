#pragma once
#include <Windows.h>
#include <stdlib.h>
#include <cstdio>
#include <iostream>
#include "CircularBuffer.h"
class Consumer
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
			char message[30];
		};
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
	Consumer();
	Consumer(int & delay, int & numMessages, size_t & maxMsgSize, size_t & buffSize, int & chunkSize);
	~Consumer();
	void runConsumer(circularBuffer& buffInst);
};