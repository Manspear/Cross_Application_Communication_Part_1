#pragma once
#include <cstdlib>
#include <stdlib.h>
#include <iostream>
#include <vector>
using namespace std;
typedef struct circular_buffer
{
	void *buffer;     // data buffer
	void *buffer_end; // end of data buffer
	size_t capacity;  // maximum number of items in the buffer
	size_t count;     // number of items in the buffer
	size_t si;		  // size of each item in the buffer
	void *head;       // pointer to head
	void *tail;       // pointer to tail
} circular_buffer;

void cb_init(circular_buffer *cb, size_t capacity, size_t si)
{
	cb->buffer = malloc(capacity * si);
	if (cb->buffer == NULL)
		// handle error
		cout << "astma" << endl;
	cb->buffer_end = (char *)cb->buffer + capacity * si;
	cb->capacity = capacity;
	cb->count = 0;
	cb->si = si;
	cb->head = cb->buffer;
	cb->tail = cb->buffer;
}

void cb_free(circular_buffer *cb)
{
	free(cb->buffer);
	// clear out other fields too, just to be safe
}

void cb_push_back(circular_buffer *cb, const void *item)
{
	if (cb->count == cb->capacity)
		// handle error
		memcpy(cb->head, item, cb->si);
	cb->head = (char*)cb->head + cb->si;
	if (cb->head == cb->buffer_end)
		cb->head = cb->buffer;
	cb->count++;
}

void cb_pop_front(circular_buffer *cb, void *item)
{
	if (cb->count == 0)
		// handle error
		memcpy(item, cb->tail, cb->si);
	cb->tail = (char*)cb->tail + cb->si;
	if (cb->tail == cb->buffer_end)
		cb->tail = cb->buffer;
	cb->count--;
}
