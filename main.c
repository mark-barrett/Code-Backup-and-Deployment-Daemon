/* Main File for the daemon. This file is responsible for choreographing the main aspects of the application. Initially it will start as a daemon
 * and will control other processes via message queues where appropriate. It will call processes for running backups and logs etc. More info
 * here as the program develops.
 * Author: Mark Barrett
 * Date: 09/03/2019
 * Git Repo: https://github.com/mark-barrett/Code-Backup-and-Deployment-Daemon
 */
#include <stdio.h>
#include "backup.h"

int main() {
	printf("Starting a backup\n");

	performBackup();
	return 0;
}
