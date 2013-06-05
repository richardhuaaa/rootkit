#ifndef COMMUNICATION_H_
#define COMMUNICATION_H_

#include <linux/init.h>


#define PROC_ENTRY "kit"

int __init communication_init(void);
void communication_exit(void);




#endif
