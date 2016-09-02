#include "CircularBuffer.h"

void * circularBuffer::makeMessage()
{
	struct messageStruct
	{
		messageHeader headero;
		const char* charoo[20];
	};
	void* pointoro = new messageStruct;
	messageStruct* lol = (messageStruct*)pointoro;
	lol->charoo[0] = "O";
	lol->charoo[1] = "R";
	lol->charoo[2] = "C";
	lol->headero.id = 1;
	lol->headero.length = sizeof(lol->charoo);
	lol->headero.padding = 256 - lol->headero.length - sizeof(lol->headero);
	pointoro = (void*)lol;
	return pointoro;
}

void circularBuffer::initCircBuffer(LPCWSTR buffName, const size_t & buffSize, const bool & isProducer, const size_t & chunkSize)
{
}

void circularBuffer::runCircBuffer(bool isProducer)
{
	if (isProducer)
	{
		//Producer do this
		while (true)
		{
			push(NULL, 90);
		}
	}
	else {
		//Consumer do this
		while (true)
		{
			//pop(NULL, (size_t)90);
		}
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
