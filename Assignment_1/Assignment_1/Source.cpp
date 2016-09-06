#include <cstdlib>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include "CircularBuffer.h"
#include <time.h> //use time as a kernel for the rand() function. 
using namespace std;

int main(int argc, char* argv[]) {
	argv[1]; //producer || consumer
	argv[2]; //delay in milliseconds
	argv[3]; //Size of filemap in megabytes, NOT bytes
	argv[4]; //number of messages to produce and consume. Use this to loop through the buffer
	argv[5]; //random || msgSize random indicates that message size can vary. msgSize indicates that all messages be of same size
	
	int delay = atoi(argv[2]);
	int fileMapSize = atoi(argv[3]);
	int numMessages = atoi(argv[4]);
	int role;
	int msgSizeMode;
	//check if this executable is a producer or a consumer
	if (strcmp("producer", argv[1]) == 0)
	{
		role = circularBuffer::PRODUCER;
	}
	else if (strcmp("consumer", argv[1]) == 0)
	{
		role = circularBuffer::CONSUMER;
	}
	else {
		LPCWSTR error = TEXT("argv[1] doesn't supply a valid string");
		OutputDebugString(error);
		return 0;
	}
	if (strcmp("random", argv[5]) == 0)
	{
		msgSizeMode = circularBuffer::RANDOM;
	}
	else if (strcmp("msgSize", argv[5]) == 0)
	{
		msgSizeMode = circularBuffer::MSGSIZE;
	}
	else {
		LPCWSTR error = TEXT("argv[5] doesn't supply a valid string");
		OutputDebugString(error);
		return 0;
	}


	circularBuffer asdf;
	LPCWSTR name = TEXT("Buffero" );

	asdf.initCircBuffer(name, fileMapSize, role, fileMapSize/4, delay, numMessages, msgSizeMode);
	asdf.runCircBuffer();
	//CreateFile(TEXT("Shared"), GENERIC_READ | GENERIC_WRITE, )
	//The first time a specific FileMap is created, it is created. 
	//If you attempt to create the file map again you will only 
	//get a handle to the already created FileMap
	//CreateFileMapping()
	return 0;
}