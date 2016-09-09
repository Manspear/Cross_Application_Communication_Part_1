#include "Consumer.h"

void Consumer::runConsumer(circularBuffer& buffInst)
{
	while (messageCount < requestedMessages)
	{
		char* msg = nullptr;
		size_t length;
		while (!buffInst.pop(msg, length))
		{
			Sleep(delay);
		}
		messageCount++;
	}
}

Consumer::Consumer()
{
}

Consumer::Consumer(int & delay, int & numMessages, size_t & maxMsgSize, size_t & buffSize, int & chunkSize, LPCWSTR varBuffName)
{
	this->delay = delay;
	this->requestedMessages = numMessages;
	this->msgSizeMode = msgSizeMode;
	this->maxMsgSize = maxMsgSize;
	messageCount = 0;
	//Create file map here for messages
	//WOLOLO

	localStep = 0;
	localDiff = 0;
	localOldDiff = 0;
	memorySteps = buffSize / chunkSize;
}

Consumer::~Consumer()
{
}