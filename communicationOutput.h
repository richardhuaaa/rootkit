int __init outputDevice_init(void);
void __exit outputDevice_exit(void);


void addCharacterToOutputDevice(char ch);
void addStringToOutputDevice(char *str);

ssize_t sendOutputToUser(struct file *filp, char *userBuffer, size_t length, loff_t *offset);
