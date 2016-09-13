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
		printf("Producer got in\n");
		size_t padding = *const_cast<size_t*>(chunkSize) - length - sizeof(sMsgStruct);
		size_t totMsgLen = sizeof(sMsgStruct) + length + padding;
		sMsgStruct* newMsg = (sMsgStruct*)msgBuff;

		//Now try to push the message.
		if (varBuff->diff == 0 && varBuff->oldDiff == 0)
		{
			return pushMsg(false, true, msg, length);
		}//If the tail is behind the head && If there is enough space to push at "the end" of the buffer
		else if (varBuff->diff > 0 && ((size_t)(varBuff->headPos + totMsgLen) < (size_t)buffSize) ||
			varBuff->diff == 0 && varBuff->oldDiff > 0 && ((size_t)(varBuff->headPos + totMsgLen) < (size_t)buffSize) ||
			//If the tail was in front of the head but is now at the head
			varBuff->diff == 0 && varBuff->oldDiff < 0 && (size_t)(varBuff->headPos + totMsgLen) < (size_t)buffSize)
		{
			return pushMsg(false, false, msg, length);
		}//If the tail is behind the head && If there is not enough space to push at "the end" of the buffer
		else if (varBuff->diff > 0 && ((size_t)(varBuff->headPos + totMsgLen) > (size_t)buffSize) ||
			varBuff->diff == 0 && varBuff->oldDiff > 0 && ((size_t)(varBuff->headPos + totMsgLen) > (size_t)buffSize))
		{
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
		printf("Client got in\n");
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
	sMsgStruct* readMsg = (sMsgStruct*)msgBuff;
	readMsg += (char)lTail.lPos;
	*length = readMsg->header.length - sizeof(sMsgStruct);
	//msg = new char[*length];
	//msg has allocated space. 
	//But it crashes here on the memcopy.
	//Which means that there is no message here. The message is not put onto the shared memory

	//Why does this work{
	char* yolo = (char*)readMsg;
	//readMsg->message += -(char)length;

	yolo += (char)(sizeof(sMsgStruct));
	bool popo = false;
	if ((void*)yolo == (void*)readMsg->message)
	{
		popo = true;
	}
	memcpy(msg, yolo, *length);
	//}
	//But not this:
	//memcpy(msg, (void*)readMsg->message, *length);
	readMsg->header.consumerPile--;

	//My theory:
	/*
	... The spot readMsg->message points to gets changed, like the FileMap changing location
	*/

	if (readMsg->header.consumerPile == 0)
	{
		if ((size_t)(varBuff->tailPos + readMsg->header.length + readMsg->header.padding) >= (size_t)buffSize)
		{
			varBuff->tailPos = 0;
		}
		else
		{
			varBuff->tailPos += readMsg->header.length + readMsg->header.padding;
			varBuff->oldDiff = varBuff->diff;
			varBuff->diff = varBuff->headPos - varBuff->tailPos;
		}
	}
	//If the tail jumps to the end of the buffer
	if ((size_t)(lTail.lPos + readMsg->header.length + readMsg->header.padding) >= (size_t)buffSize)
	{
		lTail.lOldDiff = lTail.lDiff;
		lTail.lPos = 0;
		return true;
	}
	//if the tail jump is within the buffer
	else if (((size_t)(lTail.lPos + readMsg->header.length + readMsg->header.padding) < (size_t)buffSize))
	{
		lTail.lOldDiff = lTail.lDiff;
		lTail.lPos += readMsg->header.length + readMsg->header.padding;
		return true;
	}
	return true;
}

bool circularBuffer::pushMsg(bool reset, bool start, const void * msg, size_t & length)
{
	size_t padding = *const_cast<size_t*>(chunkSize) - length - sizeof(sMsgStruct);
	size_t totMsgLen = sizeof(sMsgStruct) + length + padding;
	sMsgStruct* newMsg = (sMsgStruct*)msgBuff;
	if (!reset)
	{
		if (!start)
			newMsg += (char)varBuff->headPos;
		newMsg->header.consumerPile = clientCount;
		newMsg->header.id = msgCounter;
		msgCounter++;
		newMsg->header.length = sizeof(sMsgStruct) + length;
		newMsg->header.padding = padding;
		//newMsg->message = (char*)msg;
		newMsg->message = (char*)newMsg;
		newMsg->message += (char)(sizeof(sMsgStruct));
		memcpy(newMsg->message, msg, length);


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
		newMsg += (char)lHeadPos;
		newMsg->header.consumerPile = clientCount;
		newMsg->header.id = msgCounter;
		msgCounter++;
		newMsg->header.length = 0;
		newMsg->header.padding = ((size_t)buffSize - lHeadPos) - sizeof(sMsgStruct);

		//Move the head to the start
		newMsg = (sMsgStruct*)msgBuff;
		lHeadPos = 0;
		varBuff->oldDiff = varBuff->diff;
		varBuff->diff = varBuff->headPos - varBuff->tailPos;

		//Make a message at the start
		newMsg->header.consumerPile = clientCount;
		newMsg->header.id = msgCounter;
		msgCounter++;
		newMsg->header.length = sizeof(sMsgStruct) + length;
		newMsg->header.padding = padding;
		/**newMsg->message = *(char*)msg;
		memcpy(newMsg->message, msg, length);*/
		newMsg->message = (char*)newMsg;
		newMsg->message += (char)(sizeof(sMsgStruct));
		memcpy(newMsg->message, msg, length);

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
