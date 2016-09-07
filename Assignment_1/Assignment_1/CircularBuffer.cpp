#include "CircularBuffer.h"

void circularBuffer::initCircBuffer(LPCWSTR buffName, const size_t & buffSize, const int& role, const size_t & chunkSize)
{
	this->buffName = buffName;
	this->buffSize = &buffSize;
	this->role = role;
	this->chunkSize = &chunkSize;

	//Create another file map here for shared variables like "totalmemorysteps"
	
	int headCurrstep = 0;
	int diffSteps = 0; //if negative head is behind tail, if positive head is in front of tail
	//This variable determines behaviour if diffSteps == 0
	//B4 a new diffSteps is assigned, do oldDiffSteps = diffSteps
	int oldDiffSteps = 0;
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
