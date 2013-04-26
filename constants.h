// The system being compiled on
#define RICHARD
// Needed if you intend to be able to unload/reload the module without rebooting
#define DEV_MODE

// System dependent; get this by running: sudo grep sys_call_table /boot/System.map-$(uname -r) | awk '{print $1}'
#ifdef RICHARD
   #define SYSCALL_TABLE   0xc15b3020  // Richard
#elif RUTH
   #define SYSCALL_TABLE   0xc15d4040  // Ruth
#elif ADAM
#endif

// Dev name as it appears in /proc/devices  
#define DEVICE_NAME "rootkitLog" 


