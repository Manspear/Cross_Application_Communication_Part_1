#include "Consumer.h"

void Consumer::runConsumer(circularBuffer& buffInst)
{
	while (true)
	{
		/*void* messageGot = makeMessage();
		messageStruct message;
		memcpy(&message, messageGot, sizeof(messageHeader));
		int readSize = sizeof(messageHeader) + message.header.length;
		printf("%d \n", readSize);
		memcpy(&message, messageGot, readSize);
		printf("%s \n", message.message);


		char* msg;
		size_t consumerPile;
		size_t length;*/

		/*while (true)
		{
		if(consumerPile == 0)
		{
		if (pop(msg, length))
		{
		break;
		}
		else
		{
		Sleep(SLEEPTIME);
		}
		}
		}*/

	}
}

Consumer::Consumer()
{
}

Consumer::Consumer(int & delay, int & numMessages, size_t & maxMsgSize, size_t & buffSize, int & chunkSize)
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
