#include "CircularBuffer.h"

void circularBuffer::initCircBuffer(LPCWSTR msgBuffName, const size_t & buffSize, const int& role, const size_t & chunkSize, LPCWSTR varBuffBuffName, size_t numberOfClients)
{
	this->buffSize = &buffSize;
	this->role = role;
	this->chunkSize = &chunkSize;

	msgFileMap = CreateFileMapping(
		INVALID_HANDLE_VALUE,
		nullptr,
		PAGE_READWRITE,
		0,
		buffSize,
		msgBuffName
	);
	if (msgFileMap == nullptr)
	{
		LPCWSTR error = TEXT("ERROR: Failed to create FileMap for messages\n");
		OutputDebugString(error);
		abort();
	}
	varFileMap = CreateFileMapping(
		INVALID_HANDLE_VALUE,
		nullptr,
		PAGE_READWRITE,
		0,
		sizeof(sSharedVars),
		varBuffBuffName
	);
	if (varFileMap == nullptr)
	{
		LPCWSTR error = TEXT("ERROR: Failed to create FileMap for varBuffiables\n");
		OutputDebugString(error);
		abort();
	}
	msgBuff = (char*)MapViewOfFile(
		msgFileMap,
		FILE_MAP_ALL_ACCESS,
		0,
		0,
		buffSize
	);
	varBuff = (sSharedVars*)MapViewOfFile(
		varFileMap,
		FILE_MAP_ALL_ACCESS,
		0,
		0,
		sizeof(sSharedVars)
	);
	varBuff->headPos = 0;
	varBuff->tailPos = 0;
	varBuff->freeMem = buffSize;
	if (role == 0)
		varBuff->clientCounter = numberOfClients;
	if (role == 1)
		varBuff->clientCounter--;
	this->clientCount = numberOfClients;
	lTail = varBuff->tailPos;
	
	msgCounter = 0;

	mutex1 = Mutex(LPCWSTR(TEXT("Mutex1")));
}

void circularBuffer::stopCircBuffer()
{
}

bool circularBuffer::push(const void * msg, size_t length)
{
	if (varBuff->clientCounter == 0)
	{
		//printf("Producer got in\n");
		size_t padding = *const_cast<size_t*>(chunkSize) - length - sizeof(sMsgHeader);
		size_t totMsgLen = sizeof(sMsgHeader) + length + padding;
		sMsgHeader* newMsg = (sMsgHeader*)msgBuff;

		size_t buffTailDiff = *buffSize - varBuff->tailPos;
		size_t buffHeadDiff = *buffSize - varBuff->headPos;

		size_t tailHeadDiff = varBuff->headPos - varBuff->tailPos;

		//if there's enough space for the message
		if (varBuff->freeMem >= totMsgLen)
		{
			//if there's enough space at end of buffer
			if(totMsgLen <= buffHeadDiff)
				return pushMsg(false, false, msg, length);
			//if there's enough space at start of buffer
			if (varBuff->freeMem - buffHeadDiff >= totMsgLen)
				return pushMsg(true, true, msg, length);
		}
		
	}
	else {
		printf("Producer: Waiting for clients \n");
	}
	//Hmm... If (oldDiff < 0 && (head + msgSize) - tail > 0)
	//				return false;
	
	return false;
}

bool circularBuffer::pop(char * msg, size_t & length)
{
	if (varBuff->clientCounter == 0)
	{
		//printf("gTailPos: %d lTailPos: %d headPos: %d freeMem: %d \n", varBuff->tailPos, lTail, varBuff->headPos, varBuff->freeMem);
		//If the head has catched up to the tail
		if (lTail == varBuff->headPos && varBuff->freeMem == 0)
		{
			mutex1.lock();
			bool res;
			res = procMsg(msg, &length);
			mutex1.unlock();
			return res;
		}

		if (lTail != varBuff->headPos)
		{
			mutex1.lock();
			bool res;
			res = procMsg(msg, &length);
			mutex1.unlock();
			return res;
		}
			

		//varBuff->freeMem < *buffSize makes the tails read... wrongly
	}
	else {
		printf("Consumer: Waiting for clients \n");
	}
	return false;
}

bool circularBuffer::procMsg(char * msg, size_t * length)
{
	char* tempCast = (char*)msgBuff;
	tempCast += lTail;
	sMsgHeader* readMsg = (sMsgHeader*)tempCast;
	*length = readMsg->length - sizeof(sMsgHeader);

	printf("msgID %d	", readMsg->id);

	char* yolo = (char*)readMsg;
	yolo += sizeof(sMsgHeader);
	memcpy(msg, yolo, *length);

	readMsg->consumerPile--;

	if (readMsg->consumerPile == 0)
	{
		varBuff->freeMem += readMsg->length + readMsg->padding;

		if ((size_t)(varBuff->tailPos + readMsg->length + readMsg->padding) >= *buffSize)
		{
			printf("Tail to start\n");
			varBuff->tailPos = 0;
		}
		else
		{
			printf("Tail to forward\n");
			varBuff->tailPos += readMsg->length + readMsg->padding;
		}
	}
	//If the tail jumps to the end of the buffer
	if ((size_t)(lTail + readMsg->length + readMsg->padding) >= *buffSize)
	{
		lTail = 0;
		return true;
	}
	//if the tail jump is within the buffer
	else if (((size_t)(lTail + readMsg->length + readMsg->padding) <= *buffSize))
	{
		lTail += readMsg->length + readMsg->padding;
		return true;
	}
	return true;
}

bool circularBuffer::pushMsg(bool reset, bool start, const void * msg, size_t & length)
{
	size_t padding = *const_cast<size_t*>(chunkSize) - length - sizeof(sMsgHeader);
	size_t totMsgLen = sizeof(sMsgHeader) + length + padding;
	sMsgHeader* newMsg = (sMsgHeader*)msgBuff;
	if (!reset)
	{
		char* tempCast = (char*)newMsg;
		//(char) or not (char)
		if (!start)
			tempCast += varBuff->headPos;
		newMsg = (sMsgHeader*)tempCast;
		newMsg->consumerPile = clientCount;
		newMsg->id = msgCounter;
		msgCounter++;
		newMsg->length = sizeof(sMsgHeader) + length;
		newMsg->padding = padding;
		//newMsg->message = (char*)msg;
		char* msgPointer = (char*)newMsg;
		msgPointer += sizeof(sMsgHeader);
		memcpy(msgPointer, msg, length);

		char* debugMsg = (char*)msg;
		/*newMsg->message = (char*)newMsg;
		newMsg->message += (char)(sizeof(sMsgStruct));
		memcpy(newMsg->message, msg, length);*/

		size_t lHeadPos = varBuff->headPos;
		lHeadPos += totMsgLen;
		varBuff->headPos = lHeadPos;
		
		varBuff->freeMem -= totMsgLen;

		if (varBuff->headPos == *buffSize)
			varBuff->headPos = 0;

		return true;
	}
	if (reset)
	{
		//Make sure that headPos is set last, since it determines when tails are beginning to read
		//First make a dummy message at the end
		size_t lHeadPos = varBuff->headPos;
		char* tempCast = (char*)newMsg;
		tempCast += lHeadPos;
		newMsg = (sMsgHeader*)tempCast;
		newMsg->consumerPile = clientCount;
		newMsg->id = msgCounter;
		msgCounter++;
		newMsg->length = 0;
		newMsg->padding = (*buffSize - lHeadPos) - sizeof(sMsgHeader);
		
		varBuff->freeMem -= sizeof(sMsgHeader) + newMsg->padding;

		//Move the head to the start
		newMsg = (sMsgHeader*)msgBuff;
		lHeadPos = 0;

		//Make a message at the start
		newMsg->consumerPile = clientCount;
		newMsg->id = msgCounter;
		msgCounter++;
		newMsg->length = sizeof(sMsgHeader) + length;
		newMsg->padding = padding;

		char* msgPointer = (char*)newMsg;
		msgPointer += (char)(sizeof(sMsgHeader));
		memcpy(msgPointer, msg, length);

		lHeadPos = totMsgLen;
		varBuff->headPos = lHeadPos;

		varBuff->freeMem -= newMsg->length + newMsg->padding;

		return true;
	}
	return false;
}

circularBuffer::circularBuffer()
{
}

circularBuffer::~circularBuffer()
{
	CloseHandle(msgFileMap);
	CloseHandle(varFileMap);
}
