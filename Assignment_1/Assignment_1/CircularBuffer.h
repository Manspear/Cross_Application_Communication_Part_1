#pragma once
#include <Windows.h>
class circularBuffer
{
private:
	struct messageHeader
	{
		size_t id;
		size_t length;
		size_t padding;
	};
public:
	struct head {
		//writes stuff
	};
	struct tail {
		//reads stuff
	};
	/*
	Creates message. Testing function.
	*/
	void* makeMessage();
	/*
	Creates the shared memory space
	*/
	void initCircBuffer(
		LPCWSTR buffName,          // unique name
		const size_t& buffSize,    // size of the whole filemap
		const bool& isProducer,    // is this buffer going to be used as producer
		const size_t& chunkSize);  // round up messages to multiple of this.
	/*
	Starts and runs the inifnite circBuffer-loop
	*/
	void runCircBuffer(bool isProducer);
	/*
	push function used by the producer. The producer attempts to write data to the 
	FileMap. Specifically it tries to write a message and append it to the Shared Memory (FileMap)

	*/
	bool push(const void* msg, size_t length);
	/*
	pop function used by the consumer. The consumer attempts to read data from the FileMap.
	Upon successful reading it uses that data to do something.
	*/
	bool pop(char* msg, size_t& length);
	circularBuffer();
	~circularBuffer();
};