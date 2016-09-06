#include "Dummy.h"

CircBufferFixed::CircBufferFixed(LPCWSTR buffName, const size_t & buffSize, const bool & isProducer, const size_t & chunkSize)
{
}

CircBufferFixed::~CircBufferFixed()
{
}

size_t CircBufferFixed::canRead()
{
	return size_t();
}

size_t CircBufferFixed::canWrite()
{
	return size_t();
}

bool CircBufferFixed::push(const void * msg, size_t length)
{
	return false;
}

bool CircBufferFixed::pop(char * msg, size_t & length)
{
	return false;
}
