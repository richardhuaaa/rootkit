#include "communication.h"


int communication_init(void) {

	return 0;
}

void communication_exit() {

}
/*

// based on http://stackoverflow.com/questions/8516021/proc-create-example-for-kernel-module


struct proc_dir_entry *proc_file_entry;

static const struct file_operations proc_file_fops = {
 .owner = THIS_MODULE,
 .open  = open_callback,
 .read  = read_callback,
};

int __init init_module(void){
  proc_file_entry = proc_create("proc_file_name", 0, NULL, &proc_file_fops);
  if(proc_file_entry == NULL)
   return -ENOMEM;
  return 0;
}
*/
