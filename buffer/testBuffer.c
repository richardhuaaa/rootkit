#include "buffer.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>



static void testBufferIsInitiallyEmpty(void) {
	struct buffer buffer_ = createBuffer();
	Buffer buffer = &buffer_;
	
	assert(getAndRemoveFromBuffer(buffer) == VALUE_ON_READ_FAILING);
}

static void testAddingAnElementToTheBuffer(void) {
	struct buffer buffer_ = createBuffer();
	Buffer buffer = &buffer_;
	
	char valueToAdd = 'a';
	
	addToBuffer(buffer, valueToAdd);
	assert(getAndRemoveFromBuffer(buffer) == valueToAdd); //TODO: write another element first - the buffer may need more elements..

}



int main(void) {
	printf("Testing buffer\n");
	testBufferIsInitiallyEmpty();
	testAddingAnElementToTheBuffer();
	printf("Passed\n");
	
	return 0;
}

