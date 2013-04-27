#include "buffer.h"
#include <assert.h>
#include <stdlib.h>


#define DEFAULT_BUFFER_SIZE 1024

static void testBufferIsInitiallyEmpty(void) {
	Buffer buffer = createBuffer(DEFAULT_BUFFER_SIZE);
	assert(buffer != NULL);
	
	assert(getFreeSpaceInBuffer(buffer) == DEFAULT_BUFFER_SIZE);
	destroyBuffer(buffer);
}

static void testAddingAnElementToTheBuffer(void) {
	int initialSize = 1;
	Buffer buffer = createBuffer(initialSize);
	assert(buffer != NULL);
	assert(getFreeSpaceInBuffer(buffer) == initialSize);
	
	char valueToAdd = "a";
	
	addToBuffer(buffer, valueToAdd);
	
	
	destroyBuffer(buffer);
}

int main(void) {
	testBufferIsInitiallyEmpty();
	testAddingAnElementToTheBuffer();
	
	//TODO: handling of adding too much etc
	
	return 0;
}
