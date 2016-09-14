#include "CircularBuffer.h"

void circularBuffer::initCircBuffer(LPCWSTR msgBuffName, const size_t & buffSize, const int& role, const size_t & chunkSize, LPCWSTR varBuffBuffName, size_t numberOfClients)
{
	this->msgBuffName = msgBuffName;
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
	varBuff->diff = 0;
	varBuff->oldDiff = 0;
	if (role == 0)
		varBuff->clientCounter = numberOfClients;
	if (role == 1)
		varBuff->clientCounter--;
	lTail.lPos = varBuff->tailPos;
	lTail.lDiff = varBuff->diff;
	lTail.lOldDiff = varBuff->oldDiff;
	
	int headCurrstep = 0;
	int diffSteps = 0; //if negative head is behind tail, if positive head is in front of tail
	//This varBuffiable determines behaviour if diffSteps == 0
	//B4 a new diffSteps is assigned, do oldDiffSteps = diffSteps
	int oldDiffSteps = 0;
	msgCounter = 0;
	clientCount = 1;

	mutex1 = Mutex(LPCWSTR(TEXT("Mutex1")));

}

void circularBuffer::stopCircBuffer()
{
}

bool circularBuffer::push(const void * msg, size_t length)
{
	/*
	push() will attempt to push the input message into the 
	"headStep" position. First it will make it aligned to
	chunkSize(256 bytes) by calculating the padding.
	It will create a header, fill the header, and put it
	in front of the message.
	*/

	/*
	I got a message.
	I know how long that message is.
	I add the padding as a varBuffiable 
	to the header. So that consumers needn't calculate it.
	*/
	if (varBuff->clientCounter == 0)
	{
		//printf("Producer got in\n");
		size_t padding = *const_cast<size_t*>(chunkSize) - length - sizeof(sMsgHeader);
		size_t totMsgLen = sizeof(sMsgHeader) + length + padding;
		sMsgHeader* newMsg = (sMsgHeader*)msgBuff;

		//Now try to push the message.
		if (varBuff->diff == 0 && varBuff->oldDiff == 0)
		{
			return pushMsg(false, true, msg, length);
		}//If the tail is behind the head && If there is enough space to push at "the end" of the buffer
		else if (varBuff->diff > 0 && ((size_t)(varBuff->headPos + totMsgLen) <= *buffSize) ||
			varBuff->diff == 0 && varBuff->oldDiff > 0 && ((size_t)(varBuff->headPos + totMsgLen) <= *buffSize)// ||
			//If the tail was in front of the head but is now at the head
			//varBuff->diff == 0 && varBuff->oldDiff < 0 && (size_t)(varBuff->headPos + totMsgLen) <= *buffSize)
		){
			return pushMsg(false, false, msg, length);
		}//If the tail is behind the head && If there is not enough space to push at "the end" of the buffer
		else if (varBuff->diff > 0 && ((size_t)(varBuff->headPos + totMsgLen) >= *buffSize) ||
			varBuff->diff == 0 && varBuff->oldDiff > 0 && ((size_t)(varBuff->headPos + totMsgLen) >= *buffSize))
		{
			//printf("TotalMsgLen: %d TotalMsgLen + all: %d \n", totMsgLen, ((size_t)(varBuff->headPos + totMsgLen)));
			//See if there's room at the start of the buffer
			if (varBuff->tailPos > totMsgLen)
			{
				return pushMsg(true, false, msg, length);
			}
		}
		else if (varBuff->diff < 0 && (size_t)(varBuff->headPos + totMsgLen) <= varBuff->tailPos)
		{
			return pushMsg(false, false, msg, length);
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
	/*
	pop() will attempt to read from the msgFileMap at tailposition.
	If it succeeds, it fills the parameters with messagedata,
	and it moves the tail forward one step. The tail being expressed as
	tailStep saying "where" in the buffer it is.
	*/
	/*
	First step: read the message at tail position:
	*/
	//Calc new diff

	if (varBuff->clientCounter == 0)
	{
		//printf("Client got in\n");
		lTail.lDiff = varBuff->headPos - lTail.lPos;
		//To start off, tail waits for head to write something
		if (lTail.lDiff == 0 && lTail.lOldDiff == 0 && varBuff->headPos == 0)
		{
			return false;
		}
		if (lTail.lDiff > 0 || lTail.lDiff < 0)
		{
			return procMsg(msg, &length);
		}
		if (lTail.lDiff == 0)
		{
			//if tail was behind head
			if (lTail.lOldDiff > 0)
			{
				//wait
				return false;
			}
			//if tail was ahead of head
			if (lTail.lOldDiff < 0)
			{
				return procMsg(msg, &length);
			}
		}
	}
	else {
		printf("Consumer: Waiting for clients \n");
	}
	return false;
}

bool circularBuffer::procMsg(char * msg, size_t * length)
{
	char* tempCast = (char*)msgBuff;
	tempCast += lTail.lPos;
	sMsgHeader* readMsg = (sMsgHeader*)tempCast;
	*length = readMsg->length - sizeof(sMsgHeader);
	//msg has allocated space. 
	//But it crashes here on the memcopy.
	//Which means that there is no message here. The message is not put onto the shared memory

	//Why does this work{
	char* yolo = (char*)readMsg;
	yolo += sizeof(sMsgHeader);
	memcpy(msg, yolo, *length);
	//}
	//But not this:
	//memcpy(msg, (void*)readMsg->message, *length);
	readMsg->consumerPile--;

	//My theory:
	/*
	... The spot readMsg->message points to gets changed, like the FileMap changing locationwhen more applications read from it.
	*/

	if (readMsg->consumerPile == 0)
	{
		if ((size_t)(varBuff->tailPos + readMsg->length + readMsg->padding) >= *buffSize)
		{
			printf("Tail to start\n");
			varBuff->tailPos = 0;
		}
		else
		{
			printf("Tail to forward\n");
			varBuff->tailPos += readMsg->length + readMsg->padding;
			varBuff->oldDiff = varBuff->diff;
			varBuff->diff = varBuff->headPos - varBuff->tailPos;
		}
	}
	//If the tail jumps to the end of the buffer
	if ((size_t)(lTail.lPos + readMsg->length + readMsg->padding) >= *buffSize)
	{
		lTail.lOldDiff = lTail.lDiff;
		lTail.lPos = 0;
		return true;
	}
	//if the tail jump is within the buffer
	else if (((size_t)(lTail.lPos + readMsg->length + readMsg->padding) <= *buffSize))
	{
		lTail.lOldDiff = lTail.lDiff;
		lTail.lPos += readMsg->length + readMsg->padding;
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
		varBuff->oldDiff = varBuff->diff;
		varBuff->diff = lHeadPos - varBuff->tailPos;
		varBuff->headPos = lHeadPos;
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

		//Move the head to the start
		newMsg = (sMsgHeader*)msgBuff;
		lHeadPos = 0;
		varBuff->oldDiff = varBuff->diff;
		varBuff->diff = lHeadPos - varBuff->tailPos;

		//Make a message at the start
		newMsg->consumerPile = clientCount;
		newMsg->id = msgCounter;
		msgCounter++;
		newMsg->length = sizeof(sMsgHeader) + length;
		newMsg->padding = padding;
		/**newMsg->message = *(char*)msg;
		memcpy(newMsg->message, msg, length);*/
		char* msgPointer = (char*)newMsg;
		msgPointer += (char)(sizeof(sMsgHeader));
		memcpy(msgPointer, msg, length);
		
		/*newMsg->message = (char*)newMsg;
		newMsg->message += (char)(sizeof(sMsgStruct));
		memcpy(newMsg->message, msg, length);*/

		varBuff->oldDiff = varBuff->diff;
		lHeadPos = totMsgLen;
		varBuff->diff = lHeadPos - varBuff->tailPos;
		varBuff->headPos = lHeadPos;
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
