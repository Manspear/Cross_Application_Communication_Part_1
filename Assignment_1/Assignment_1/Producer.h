#pragma once
#include <Windows.h>
#include <stdlib.h>
#include <cstdio>
#include <iostream>
#include "CircularBuffer.h"
class Producer
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

	int delay;
	int requestedMessages;
	int msgSizeMode;
	int maxMsgSize;
	int messageCount;

	int localStep;
	int localDiff;
	int localOldDiff;
	int memorySteps;

	int numMessages;


public:
	Producer();
	Producer(int & delay, int & numMessages, size_t & maxMsgSize, size_t & buffSize, int & chunkSize);
	~Producer();
	void * makeMessage();
	void runProducer(circularBuffer& buffInst);
};