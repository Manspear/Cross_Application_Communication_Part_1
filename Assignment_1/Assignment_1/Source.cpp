#include <cstdlib>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include "CircularBuffer.h"
using namespace std;

int main(int argc, char* argv[]) {
	
	size_t size = sizeof(size_t);
	
	circularBuffer asdf;
	LPCWSTR name = TEXT("This is a message from the producer." );
	asdf.initCircBuffer(name, 5, true, 256);

	return 0;
}