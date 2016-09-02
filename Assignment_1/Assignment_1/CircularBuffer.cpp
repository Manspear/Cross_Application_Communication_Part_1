#include "CircularBuffer.h"

void * circularBuffer::makeMessage()
{
	struct messageStruct
	{
		messageHeader headero;
		char* charoo[20];
	};
	void* pointoro = new messageStruct;
	messageStruct* lol = (messageStruct*)pointoro;
	lol->charoo = (char*)"porolol";
	return nullptr;
}

void circularBuffer::initCircBuffer(LPCWSTR buffName, const size_t & buffSize, const bool & isProducer, const size_t & chunkSize)
{
}

void circularBuffer::runCircBuffer()
{
	//Producer do this
	while (true)
	{
		
	}
	//Consumer do this
	while (true)
	{

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
