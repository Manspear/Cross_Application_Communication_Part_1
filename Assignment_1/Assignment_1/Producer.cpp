#include "Producer.h"

Producer::Producer()
{
}

Producer::Producer(int& delay, int& numMessages, size_t& maxMsgSize, int& msgSizeMode, size_t& buffSize, int& chunkSize, LPCWSTR varBuffName)
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
	srand(time(NULL));
	testID = 0;
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

	static const char alphanum[] =
		"0123456789"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz";

	for (auto i = 0; i < msgLen; ++i) {
		msg[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
	}
	msg[msgLen-1] = '\0';

	testID++;
}

void Producer::runProducer(circularBuffer& buffInst)
{
	char* msg;
	int messageLength;
	//Make this change depending on msgSizeMode
	while (messageCount < requestedMessages)
	{
		if (msgSizeMode == RANDOM)
		{
			messageLength = rand() % (maxMsgSize - sizeof(sMsgHeader));
			msg = new char[messageLength];
		}
		else if(msgSizeMode == MSGSIZE)
		{
			messageLength = maxMsgSize - sizeof(sMsgHeader);
			msg = new char[maxMsgSize];
		}
		makeMessage(msg, messageLength);
		//Try to push msg
		while (!buffInst.push(msg, messageLength))
		{
			//printf("messageCount %d\n", messageCount);
			Sleep(delay);
		}
		//sent msg
		printf("%d %s\n", messageCount, msg);
		delete[]msg;
		messageCount++;
	}
}
