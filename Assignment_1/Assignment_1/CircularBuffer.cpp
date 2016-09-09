#include "CircularBuffer.h"

void circularBuffer::initCircBuffer(LPCWSTR msgBuffName, const size_t & buffSize, const int& role, const size_t & chunkSize, LPCWSTR varBuffBuffName)
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
		sizeof(sharedVariables),
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
	varBuff = (sharedVariables*)MapViewOfFile(
		varFileMap,
		FILE_MAP_ALL_ACCESS,
		0,
		0,
		sizeof(sharedVariables)
	);
	varBuff->headPos = 0;
	varBuff->tailPos = 0;
	varBuff->diff = 0;
	varBuff->oldDiff = 0;
	
	int headCurrstep = 0;
	int diffSteps = 0; //if negative head is behind tail, if positive head is in front of tail
	//This varBuffiable determines behaviour if diffSteps == 0
	//B4 a new diffSteps is assigned, do oldDiffSteps = diffSteps
	int oldDiffSteps = 0;
	msgCounter = 0;
	clientCount = 1;
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
	size_t padding = *const_cast<size_t*>(chunkSize) - length - sizeof(messageHeader);
	size_t totMsgLen = sizeof(messageHeader) + length + padding;
	messageStruct* newMsg = (messageStruct*)msgBuff;

	//Now try to push the message.
	if (varBuff->diff == 0 && varBuff->oldDiff == 0)
	{	
		//newMsg += (char)varBuff->headPos;
		newMsg->header.consumerPile = clientCount;
		newMsg->header.id = msgCounter;
		msgCounter++;
		newMsg->header.length = sizeof(messageHeader) + length;
		newMsg->header.padding = padding;
		newMsg->message = (char*)msg;
		varBuff->headPos += totMsgLen;

		varBuff->oldDiff = varBuff->diff;
		varBuff->diff = varBuff->headPos - varBuff->tailPos;
		
		return true;
	}//If the tail is behind the head && If there is enough space to push at "the end" of the buffer
	else if (varBuff->diff > 0 && ((size_t)(varBuff->headPos + totMsgLen) < (size_t)buffSize) || 
			 varBuff->diff == 0 && varBuff->oldDiff > 0 && ((size_t)(varBuff->headPos + totMsgLen) < (size_t)buffSize) ||
			 //If the tail was in front of the head but is now at the head
		     varBuff->diff == 0 && varBuff->oldDiff < 0 && (size_t)(varBuff->headPos + totMsgLen) < (size_t)buffSize)
	{		
		newMsg += (char)varBuff->headPos;
		newMsg->header.consumerPile = clientCount;
		newMsg->header.id = msgCounter;
		msgCounter++;
		newMsg->header.length = sizeof(messageHeader) + length;
		newMsg->header.padding = padding;
		newMsg->message = (char*)msg;
		varBuff->headPos += totMsgLen;
		varBuff->oldDiff = varBuff->diff;
		varBuff->diff = varBuff->headPos - varBuff->tailPos;

		return true;
	}//If the tail is behind the head && If there is not enough space to push at "the end" of the buffer
	else if (varBuff->diff > 0 && ((size_t)(varBuff->headPos + totMsgLen) > (size_t)buffSize) ||
		     varBuff->diff == 0 && varBuff->oldDiff > 0 && ((size_t)(varBuff->headPos + totMsgLen) > (size_t)buffSize))
	{
		//See if there's room at the start of the buffer
		if (varBuff->tailPos > totMsgLen)
		{
			//First make a dummy message at the end
			newMsg += (char)varBuff->headPos;
			newMsg->header.consumerPile = clientCount;
			newMsg->header.id = msgCounter;
			msgCounter++;
			newMsg->header.length = 0;
			newMsg->header.padding =  ((size_t)buffSize - varBuff->headPos) - sizeof(messageHeader);
			
			//Move the head to the start
			newMsg = (messageStruct*)msgBuff;
			varBuff->headPos = 0;
			varBuff->oldDiff = varBuff->diff;
			varBuff->diff = varBuff->headPos - varBuff->tailPos;
			
			//Make a message at the start
			newMsg->header.consumerPile = clientCount;
			newMsg->header.id = msgCounter;
			msgCounter++;
			newMsg->header.length = sizeof(messageHeader) + length;
			newMsg->header.padding = padding;
			newMsg->message = (char*)msg;

			varBuff->headPos = totMsgLen;
			varBuff->oldDiff = varBuff->diff;
			varBuff->diff = varBuff->headPos - varBuff->tailPos;

			return true;
		}
	}//                                                              <= ???????????????
	else if (varBuff->diff < 0 && (size_t)(varBuff->headPos + totMsgLen) <= varBuff->tailPos)
	{
		newMsg->header.consumerPile = clientCount;
		newMsg->header.id = msgCounter;
		msgCounter++;
		newMsg->header.length = sizeof(messageHeader) + length;
		newMsg->header.padding = padding;
		newMsg->message = (char*)msg;

		varBuff->headPos += totMsgLen;
		varBuff->oldDiff = varBuff->diff;
		varBuff->diff = varBuff->headPos - varBuff->tailPos;
		return true;
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
