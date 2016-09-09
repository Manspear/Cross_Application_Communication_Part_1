#pragma once
struct messageHeader
{
	size_t id;
	size_t length;
	size_t padding;
	size_t consumerPile;
};
struct messageStruct
{
	messageHeader header;
	char* message;
};
struct sharedVariables
{
	size_t headPos;
	size_t tailPos;
	size_t diff;
	size_t oldDiff;
};