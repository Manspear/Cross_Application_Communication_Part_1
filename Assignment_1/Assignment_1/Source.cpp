#include <cstdlib>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include "CircularBuffer.h"
#include "Consumer.h"
#include "Producer.h"
#include "Mutex.h"
#include <time.h> //use time as a kernel for the rand() function. 
#define CHUNKSIZE 256
#define NUMCLIENTS 3
using namespace std;
enum {
	PRODUCER = 0,
	CONSUMER = 1,
	RANDOM = 0,
	MSGSIZE = 1
};
int main(int argc, char* argv[]) {
	//argv[1]; producer || consumer
	//argv[2]; delay in milliseconds
	//argv[3]; Size of filemap in megabytes, NOT bytes
	//argv[4]; number of messages to produce and consume. Use this to loop through the buffer
	//argv[5]; random || msgSize random indicates that message size can vary. msgSize indicates that all messages be of same size
	//convert FileMap-size to bytes
	//Count = "number of 256 byte steps" inside of that memory space.
	//Count is shared between processes. Head keeps track of the current "Count-step" it's on
	//Tail only makes sure that it doesn't read from the same Count-step as the head
	int delay = atoi(argv[2]);
	size_t fileMapSize = atoi(argv[3]); 
	//fileMapSize = fileMapSize << 20; //converts to bytes
	int numMessages = atoi(argv[4]);
	int role;
	int msgSizeMode;
	size_t maxMsgSize = fileMapSize / 4;
	//check if this executable is a producer or a consumer
	if (strcmp("producer", argv[1]) == 0)
	{
		role = PRODUCER;
	}
	else if (strcmp("consumer", argv[1]) == 0)
	{
		role = CONSUMER;
	}
	else {
		LPCWSTR error = TEXT("argv[1] doesn't supply a valid string");
		OutputDebugString(error);
		return 0;
	}
	if (strcmp("random", argv[5]) == 0)
	{
		msgSizeMode = RANDOM;
	}
	else if (atoi(argv[5]) <= (size_t)(fileMapSize / 4))
	{
		msgSizeMode = MSGSIZE;
		maxMsgSize = atoi(argv[5]);
	}
	else {
		LPCWSTR error = TEXT("argv[5] doesn't supply a valid string");
		OutputDebugString(error);
		return 0;
	}

	circularBuffer cirB;
	LPCWSTR msgBuffName = TEXT("MessageBuffer" );
	LPCWSTR varBuffName = TEXT("VarBuffer");
	cirB.initCircBuffer(msgBuffName, fileMapSize, role, CHUNKSIZE, varBuffName, NUMCLIENTS);
	Mutex mut = Mutex(TEXT("Herbert"));

	//mut.lock();
	if (role == PRODUCER)
	{
		int chunkSize = 256;
		Producer producer = Producer(delay, numMessages, maxMsgSize, msgSizeMode, fileMapSize, chunkSize, varBuffName);
		producer.runProducer(cirB);

		//Consumer consumer = Consumer(delay, numMessages, maxMsgSize, fileMapSize, chunkSize, varBuffName);
		//consumer.runConsumer(cirB);
	}
	//mut.unlock();
	mut.lock();
	if (role == CONSUMER)
	{
		printf("Consumer Init\n");
		//Sleep(500);
		int chunkSize = 256;
		Consumer consumer = Consumer(delay, numMessages, maxMsgSize, fileMapSize, chunkSize, varBuffName);
		consumer.runConsumer(cirB);
	}
	mut.unlock();
	//CreateFile(TEXT("Shared"), GENERIC_READ | GENERIC_WRITE, )
	//The first time a specific FileMap is created, it is created. 
	//If you attempt to create the file map again you will only 
	//get a handle to the already created FileMap
	//CreateFileMapping()
	cin.get();
	return 0;
}