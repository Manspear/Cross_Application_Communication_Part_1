#include "Producer.h"

Producer::Producer()
{
}

Producer::Producer(int& delay, int& numMessages, size_t& maxMsgSize, size_t& buffSize, int& chunkSize)
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

Producer::~Producer()
{
}

void * Producer::makeMessage()
{
	messageStruct* lol = new messageStruct;
	char popo[] = "kkkkkkkkk1";
	int lengthi = strlen(popo) + 1;
	memcpy(lol->message, popo, lengthi);
	lol->header.id = 1;
	lol->header.length = strlen(lol->message) + 1;
	lol->header.padding = 256 - lol->header.length - sizeof(lol->header);

	return (void*)lol;
}

void Producer::runProducer(circularBuffer& buffInst)
{
	while (messageCount < requestedMessages)
	{
		void* msg = makeMessage();
		messageHeader thing;
		memcpy(&thing, msg, sizeof(messageHeader));
		int msgSize = thing.length + thing.padding + sizeof(messageHeader);
		//Try to push msg
		while (!buffInst.push(msg, msgSize))
		{
			Sleep(delay);
		}
		messageCount++;
	}
}
