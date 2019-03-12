/* This C file is used to perform the backup of the files
 * Author: Mark Barrett
 * Date: 10/09/2019
 * Git Repo: https://github.com/mark-barrett/Code-Backup-and-Deployment-Daemon
 */
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>

int performBackup() {
	// Firstly lets define the time for right now
	time_t now = time(NULL);
	
	// Move to the root directory so we can move freely
	if(chdir("/") < 0) {
		printf("Cannot get to root, try running with sudo");
		exit(EXIT_FAILURE);
	}

	// To perform the backup lets check to see if the backups folder exists.
	// Use dirent for this
	DIR* dir = opendir("/var/lib/backup-daemon/backups");

	// Check if it exists
	if(dir) {
		printf("Found backup directory\n");
	} else if(ENOENT == errno) {
		printf("Backup directory not found, creating it.\n");

		// Create the directory because it doesn't exist
		// We want to only allow the user to read the backups not alter them.
		if(mkdir("/var/lib/backup-daemon", 0444) == -1) {
			printf("Error creating backups folder.\n");
		} else {
			// Create the backups folder inside the application folder
			if(mkdir("/var/lib/backup-daemon/backups", 0444) == -1) {
				printf("Error creating backups folder.\n");
			}
		}
	} else {
		printf("Cant open the backup directory\n");
		exit(EXIT_FAILURE);
	}

	// If we got here we have either found the already existing directory
	// OR we have created it.
	// Now lets lock the directories.
}
