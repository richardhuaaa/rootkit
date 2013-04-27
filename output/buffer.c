#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "buffer.h"

//TODO: use gcc atomic buildins  (hopefully these will work in the kernel)

struct buffer {
	int totalLengthThatShouldNotBeChanged;
	int currentLength; ///TODO: enable writing past this in a nice way e.g. removing previous contents..
};

Buffer createBuffer(int size) {
	assert(size > 0);
	
	Buffer buffer = malloc(sizeof(struct buffer));
	assert(buffer != NULL);
	
	buffer->totalLengthThatShouldNotBeChanged = size;
	buffer->currentLength = 0;
	
	return buffer;
}

void destroyBuffer(Buffer buffer) {
	free(buffer);
}

int getFreeSpaceInBuffer(Buffer buffer) {
	return buffer->currentLength - totalLengthThatShouldNotBeChanged;
}

void addToBuffer(Buffer buffer, bufferEntry valueToAdd) {
	int expectedSpaceUsed = buffer->currentLength + 1;
	
	//__sync_val_compare_and_swap(
}
