#pragma once
struct sMsgHeader
{
	size_t id;
	size_t length;
	size_t padding;
	size_t consumerPile;
};
struct sMsgStruct
{
	sMsgHeader header;
	char* message;
};
struct sSharedVars
{
	size_t headPos;
	size_t tailPos;
	size_t diff;
	size_t oldDiff;
};