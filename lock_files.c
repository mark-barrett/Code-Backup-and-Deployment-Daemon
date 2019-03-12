/* This file is reponsible for locking the intranet and live folders in the /var/html/www directories when a backup or a transfer is being performed.
 * Author: Mark Barrett
 * Date: 12/03/2019
 * Git Repo: https://github.com/mark-barrett/Code-Backup-and-Deployment-Daemon
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>

// Function that takes both the folder path and the mode and locks the file
int lockFiles(char folder[], char mode[]) {
	// Ensure we are at the root directory
	if(chdir("/") < 0) {
		printf("Can't lock files as can't get to root. Try running with sudo\n");
		exit(EXIT_FAILURE);
	} else {
		// We can access root.
		// Lock the files
		int i = strtol(mode, 0, 8);
		if(chmod(folder, i) < 0) {
			printf("Unable to lock folder for backup. Try running with sudo.\n");
			exit(EXIT_FAILURE);
		} else {
			return 0;
		}

	}	
}

