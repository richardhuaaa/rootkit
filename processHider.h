//TODO: check if we should be using __exit anywhere or not

int __init processHider_init(void);
void __exit processHider_exit(void);

int hideProcess(int pidNumber); // returns error etc
int showProcess(int pidNumber); // returns error etc




