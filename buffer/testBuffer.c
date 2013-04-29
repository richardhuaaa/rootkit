#include "buffer.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>



#define DEFAULT_BUFFER_SIZE 1024

static void testBufferIsInitiallyEmpty(void) {
	Buffer buffer = createBuffer(DEFAULT_BUFFER_SIZE);
	assert(buffer != NULL);
	
	assert(getAndRemoveFromBuffer(buffer) == VALUE_ON_READ_FAILING);

	destroyBuffer(buffer);
}

static void testAddingAnElementToTheBuffer(void) {
	int initialSize = 2;
	Buffer buffer = createBuffer(initialSize);
	assert(buffer != NULL);
	
	char valueToAdd = 'a';
	
	addToBuffer(buffer, valueToAdd);
	assert(getAndRemoveFromBuffer(buffer) == valueToAdd); //TODO: write another element first - the buffer may need more elements..
	
	
	destroyBuffer(buffer);
}



int main(void) {
	printf("Testing buffer\n");
	testBufferIsInitiallyEmpty();
	testAddingAnElementToTheBuffer();
	printf("Passed\n");
	
	return 0;
}

