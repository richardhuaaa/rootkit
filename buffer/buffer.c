#include "buffer.h"

//TODO: use gcc atomic buildins  (hopefully these will work in the kernel)

/* TODO: use sync primatives etc
 race conditions are allowed to cause the loss of data in this buffer provided the stability of the kernel is not affected.
 
 The reader should avoid reading the most recently written value by the write as there is no guarantee it has been written completely at he time of attempting a read.
 
 TODO: deal with race condition between reader / writer on present write
 */



static int getValueIncrementedWrappingToSizeOfBuffer(int value);


struct buffer createBuffer(void) {
	struct buffer buffer; 
	//TODO: clear all elements of buffer initially (e.g. data)
	
	buffer.readPosition = 0;
	buffer.writePosition = 0;
	
	return buffer;
}


void addToBuffer(Buffer buffer, bufferEntry valueToAdd) {
	if (valueToAdd == VALUE_ON_READ_FAILING) {
		return;
	}
	
	//TODO: add a space e.g. to prevent write position becoming read position which gives the appearance of the buffer then being empty rather than full.
	int expectedOldWritePosition = buffer->writePosition;
	int nextWritePosition = getValueIncrementedWrappingToSizeOfBuffer(expectedOldWritePosition);
	
	int isFullNow = (nextWritePosition == buffer->readPosition);
	
	if (!isFullNow) {
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
	
	int isBufferEmpty = (expectedOldReadPosition == buffer->writePosition);
	bufferEntry result;
	
	if (isBufferEmpty) {
		result = VALUE_ON_READ_FAILING;
	} else {
		int nextReadPosition = getValueIncrementedWrappingToSizeOfBuffer(expectedOldReadPosition);
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

// Preconditions OUTPUT_BUFFER_SIZE > 0
static int getValueIncrementedWrappingToSizeOfBuffer(int value) {
	return (value + 1) % OUTPUT_BUFFER_SIZE;
}
