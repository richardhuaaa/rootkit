#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "buffer.h"

//TODO: use gcc atomic buildins  (hopefully these will work in the kernel)

/* TODO: use sync primatives etc
 race conditions are allowed to cause the loss of data in this buffer provided the stability of the kernel is not affected.
 
 
 The reader should never read the most recently written value by the write as there is no guarantee it has been written completely at he time of attempting a read.
 
 
 TODO: remove use of assert - change to returning an error!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 TODO: deal with race condition between reader / writer on present write
 */

struct buffer { //TODO: rename
	int totalLengthThatShouldNotBeChangedAndIsNotZero;
	volatile int readPosition;
	volatile int writePosition;
	bufferEntry *data;
};

static int getValueIncrementedWrappingToSizeOfBuffer(Buffer buffer, int value);


Buffer createBuffer(int size) {
	assert(size > 0);
	
	Buffer buffer = malloc(sizeof(struct buffer));
	if (buffer == NULL) {	
		return NULL;
	}
	
	buffer->data = calloc(size, sizeof(bufferEntry));
	if (buffer->data == NULL) {
		free(buffer);
		return NULL;
	}
	
	
	buffer->totalLengthThatShouldNotBeChangedAndIsNotZero = size;
	
	buffer->readPosition = 0;
	buffer->writePosition = 0;
	
	return buffer;
}

void destroyBuffer(Buffer buffer) {
	if (buffer != NULL) {
		free(buffer->data);
		buffer->data = NULL;
		free(buffer);
	}
}



void addToBuffer(Buffer buffer, bufferEntry valueToAdd) {
	//TODO: add a space e.g. to prevent write position becoming read position which gives the appearance of the buffer then being empty rather than full.
	int expectedOldWritePosition = buffer->writePosition;
	int nextWritePosition = getValueIncrementedWrappingToSizeOfBuffer(buffer, expectedOldWritePosition);
	
	int isFullNow = (expectedOldWritePosition == buffer->readPosition);
	int wouldAppearEmptyIfAnotherItemWereToBeAdded = (nextWritePosition == buffer->readPosition);
	
	if (!isFullNow && !wouldAppearEmptyIfAnotherItemWereToBeAdded) {
		int correctlyGrabbedSpace = __sync_bool_compare_and_swap (&(buffer->writePosition), expectedOldWritePosition, nextWritePosition);
		
		if (correctlyGrabbedSpace) {
			buffer->data[expectedOldWritePosition] = valueToAdd;  //TODO: rename variables so that this makes sense - e.g. initially writing is done at position 0 not newWritePosition
		} else {
			// fail silently
		}
	} else {
		// fail silently
	}
}

bufferEntry getAndRemoveFromBuffer(Buffer buffer) {
	int expectedOldReadPosition = buffer->readPosition;
	// reading write position is assumed to be atomic. If write position updates so there is more space this should be ok..
	
	int isBufferFullNow = (expectedOldReadPosition == buffer->writePosition);
	bufferEntry result;
	
	if (isBufferFullNow) {
		result = VALUE_ON_READ_FAILING;
	} else {
		int nextReadPosition = getValueIncrementedWrappingToSizeOfBuffer(buffer, expectedOldReadPosition);
		//TODO: prevent reading the most recent value from the writer. e.g. as it may not have been written yet
		
		int correctlyRemovedAnElement = __sync_bool_compare_and_swap (&(buffer->readPosition), expectedOldReadPosition, nextReadPosition);
		
		if (correctlyRemovedAnElement) {
			result = buffer->data[expectedOldReadPosition];
		} else {
			result = VALUE_ON_READ_FAILING;	
		}
	}
	
	return result;
}

// Preconditions buffer->totalLengthThatShouldNotBeChangedAndIsNotZero > 0
static int getValueIncrementedWrappingToSizeOfBuffer(Buffer buffer, int value) {
	return (value + 1) % buffer->totalLengthThatShouldNotBeChangedAndIsNotZero;
}

