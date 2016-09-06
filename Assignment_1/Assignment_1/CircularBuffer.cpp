#include "CircularBuffer.h"

void * circularBuffer::makeMessage()
{
	messageStruct* lol = new messageStruct;
	char popo[] = "kkkkkkkkk1";
	int lengthi = strlen(popo)+1;
	memcpy(lol->charoo, popo, lengthi);
	lol->header.id = 1;
	lol->header.length = strlen(lol->charoo)+1;
	lol->header.padding = 256 - lol->header.length - sizeof(lol->header);

	return (void*)lol;
}

void circularBuffer::initCircBuffer(LPCWSTR buffName, const size_t & buffSize, const int& role, const size_t & chunkSize, int& delay,
	int& numMessages, int& msgSizeMode)
{
	this->buffName = buffName;
	this->buffSize = &buffSize;
	this->role = role;
	this->chunkSize = &chunkSize;
	this->delay = delay;
	this->numMessages = numMessages;
}

void circularBuffer::runCircBuffer()
{
	while (role == PRODUCER)
	{
		/*
		Where to test alignment of message?
		the producer's job?

		Maximum message size is 1/4 of buffersize.
		But as long as the message is aligned to at least 256 bits
		it's considered "aligned". So make it a multiple of 256.

		How do you do that?
		messagesize % 256
		256 % 256 = 0
		512 % 256 = 0
		511 % 256 = 255 --> alignment is then current size of data + difference between 256 and 255, which is 1
		1025 % 256 = 1 --> difference between 256 and 1 is 255. "Add" 255 as padding
		*/
		void* msg = makeMessage();
		//Try to push msg
	}
	while (role == CONSUMER)
	{
		void* messageGot = makeMessage();
		messageStruct message;
		memcpy(&message, messageGot, sizeof(messageHeader));
		int readSize = sizeof(messageHeader) + message.header.length;
		printf("%d \n", readSize);
		memcpy(&message, messageGot, readSize);
		printf("%s \n", message.charoo);
		

		char* msg;
		size_t consumerPile;
		size_t length;

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

bool circularBuffer::push(const void * msg, size_t length)
{
	return false;
}

bool circularBuffer::pop(char * msg, size_t & length)
{
	return false;
}

circularBuffer::circularBuffer()
{
}

circularBuffer::~circularBuffer()
{
}
