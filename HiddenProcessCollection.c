#include <linux/slab.h>
#include "HiddenProcessCollection.h"
#include "messagesToUser.h"

#define MAXIMUM_NUMBER_OF_ENTRIES 100

#define NO_ENTRIES_AVAILABLE -1
#define NOT_FOUND -1

static int getIndexOfFreeSpot(HiddenProcessCollection collection);

//TODO: improve the efficiency of this

struct entry {
	struct restorableHiddenTask restorableHiddenTask;
	int isInUse;
};

struct hiddenProcessCollection {
	struct entry restorableTasks[MAXIMUM_NUMBER_OF_ENTRIES];
};

// returns NULL on failing
HiddenProcessCollection createHiddenProcessCollection(void) {
	HiddenProcessCollection result = kcalloc(MAXIMUM_NUMBER_OF_ENTRIES, sizeof(struct hiddenProcessCollection), __GFP_NOWARN);

	if (result == NULL) {
		return NULL;
	}

	return result;
}

int isHiddenProcessCollectionFull(HiddenProcessCollection collection) {
	int freeIndex = getIndexOfFreeSpot(collection);
	return freeIndex != NO_ENTRIES_AVAILABLE;
}

static struct entry *getEntry(HiddenProcessCollection collection, int index) {
	return &(collection->restorableTasks[index]);
}

static int isTaskInUse(HiddenProcessCollection collection, int index) {
	struct entry *entry = getEntry(collection, index);
	return entry->isInUse;
}

static int getIndexOfFreeSpot(HiddenProcessCollection collection) {
	int i;
	for (i = 0; i < MAXIMUM_NUMBER_OF_ENTRIES; i++) {
		if (!isTaskInUse(collection, i)) {
			return i;
		}
	}
	return NO_ENTRIES_AVAILABLE;
}

static int getIndexOfEntryWithPid(HiddenProcessCollection collection, int pid) {
	int i;
	for (i = 0; i < MAXIMUM_NUMBER_OF_ENTRIES; i++) {
		struct entry *entry = getEntry(collection, i);
		return entry->restorableHiddenTask.pidNumber == pid;
	}
	return NOT_FOUND;
}

int isPidInCollection(HiddenProcessCollection collection, int pid) {
	return getIndexOfEntryWithPid(collection, pid) != NOT_FOUND;
}

static struct restorableHiddenTask removeEntry(HiddenProcessCollection collection, int index) {
	struct restorableHiddenTask result = {0};
	if (index <= 0) {
		printError("attempting to remove an entry that does not exist");
	} else {
		struct entry *entry = getEntry(collection, index);
		result = entry->restorableHiddenTask;
		entry->isInUse = 0;
	}
	return result;
}
struct restorableHiddenTask removePidFromCollection(HiddenProcessCollection collection, int pid) {
	int index = getIndexOfEntryWithPid(collection, pid);
	return removeEntry(collection, index);
}

static int getIndexOfEntryWithTask(HiddenProcessCollection collection, void *task) {
	int i;
	for (i = 0; i < MAXIMUM_NUMBER_OF_ENTRIES; i++) {
		struct entry *entry = getEntry(collection, i);
		return entry->restorableHiddenTask.task == task;
	}
	return NOT_FOUND;
}

int isTaskInCollection(HiddenProcessCollection collection, void *task) {
	return getIndexOfEntryWithTask(collection, task) != NOT_FOUND;
}

struct restorableHiddenTask removeTaskFromCollection(HiddenProcessCollection collection, void *task) {
	int index = getIndexOfEntryWithTask(collection, task);
	return removeEntry(collection, index);
}

void addHiddenProcessToCollection(HiddenProcessCollection collection, struct restorableHiddenTask restorableHiddenTask) {
	int index = getIndexOfFreeSpot(collection);
	if (index <= 0) {
		printError("failed to hidden process to collection as it was full. This should have been checked earlier\n");
		return;
	}

	{
		struct entry *entry = &collection->restorableTasks[index];
		entry->isInUse = 1;
		entry->restorableHiddenTask = restorableHiddenTask;
	}
}

void destoryHiddenProcessCollection(HiddenProcessCollection collection) {
	kfree(collection);
}
