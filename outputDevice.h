
#define DEVICE_NAME "rootkitLog" /// Dev name as it appears in /proc/devices  

int __init outputDevice_init(void);
void __exit outputDevice_exit(void);
void addToOutputDevice(char *str);

