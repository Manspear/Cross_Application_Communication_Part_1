#include "CircularBuffer.h"

void circularBuffer::initCircBuffer(LPCWSTR msgBuffName, const size_t & buffSize, const int& role, const size_t & chunkSize, LPCWSTR varBuffName)
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
		varBuffName
	);
	if (varFileMap == nullptr)
	{
		LPCWSTR error = TEXT("ERROR: Failed to create FileMap for variables\n");
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
	varBuff = (char*)MapViewOfFile(
		msgFileMap,
		FILE_MAP_ALL_ACCESS,
		0,
		0,
		buffSize
	);
	

	int headCurrstep = 0;
	int diffSteps = 0; //if negative head is behind tail, if positive head is in front of tail
	//This variable determines behaviour if diffSteps == 0
	//B4 a new diffSteps is assigned, do oldDiffSteps = diffSteps
	int oldDiffSteps = 0;
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
	I add the padding as a variable 
	to the header. So that consumers needn't calculate it.
	*/
	messageStruct modMsg;
	memcpy(&modMsg, msg, length);
	modMsg.header.padding = *const_cast<size_t*>(chunkSize) - length;
	modMsg.header.consumerPile--;
	size_t totMsgLen = modMsg.header.padding + length;
	//Now try to push the message.

	sharedVariables var;
	memcpy(&var, varBuff, sizeof(sharedVariables));
	if (var.diff == 0 && var.oldDiff == 0)
	{
		memcpy(msgBuff, &modMsg, totMsgLen);
		var.headPos += totMsgLen;
		var.oldDiff = 0;
		var.diff = var.headPos + totMsgLen;
		memcpy(varBuff, &var, sizeof(sharedVariables));
		return true;
	}
	//If the tail is behind the head && If there is enough space to push at "the end" of the buffer
	if (var.diff > 0 && ((size_t)(var.headPos + totMsgLen) < (size_t)buffSize))
	{		
		memcpy(msgBuff+(char)var.headPos, &modMsg, totMsgLen);
		var.headPos += totMsgLen;
		var.oldDiff = var.diff;
		var.diff = var.headPos - var.tailPos;
		memcpy(varBuff, &var, sizeof(sharedVariables));
		return true;
	}
	//If the tail is behind the head && If there is not enough space to push at "the end" of the buffer
	if (var.diff > 0 && ((size_t)(var.headPos + totMsgLen) > (size_t)buffSize))
	{
		//See if there's room at the start of the buffer
		if (var.tailPos > totMsgLen)
		{
			memcpy(msgBuff, &modMsg, totMsgLen);
			var.headPos = totMsgLen;
			var.oldDiff = var.diff;
			var.diff = var.headPos - var.tailPos;
			memcpy(varBuff, &var, sizeof(sharedVariables));
			return true;
		}
	}
	//                                                    <= ???????????????
	if (var.diff < 0 && (size_t)(var.headPos + totMsgLen) < var.tailPos)
	{
		memcpy(msgBuff + (char)var.headPos, &modMsg, totMsgLen);
		var.headPos += totMsgLen;
		var.oldDiff = var.diff;
		var.diff = var.headPos - var.tailPos;
		memcpy(varBuff, &var, sizeof(sharedVariables));
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
