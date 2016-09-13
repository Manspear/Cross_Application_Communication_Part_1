#include "Producer.h"

Producer::Producer()
{
}

Producer::Producer(int& delay, int& numMessages, size_t& maxMsgSize, size_t& buffSize, int& chunkSize, LPCWSTR varBuffName)
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

void Producer::makeMessage(char* msg, size_t msgLen)
{
	
	//messageStruct* lol = new messageStruct;
	//char popo[] = "kkkkkkkkk1";
	//int lengthi = strlen(popo) + 1;
	//memcpy(lol->message, popo, lengthi);
	//lol->header.id = 1;
	//lol->header.length = sizeof(messageHeader) + strlen(lol->message) + 1;
	//lol->header.padding = 256 - lol->header.length;
	for (int i = 0; i < msgLen; i++)
	{
		msg[i] = 'a';
		if (i == msgLen - 1)
			msg[i] = '\0';
	}
}

void Producer::runProducer(circularBuffer& buffInst)
{
	int messageLength = 5;
	char* msg = new char[messageLength];
	//Make this change depending on msgSizeMode
	
	while (messageCount < requestedMessages)
	{
		makeMessage(msg, messageLength);
		//Try to push msg
		while (!buffInst.push(msg, messageLength))
		{
			Sleep(delay);
		}
		messageCount++;
	}
}
