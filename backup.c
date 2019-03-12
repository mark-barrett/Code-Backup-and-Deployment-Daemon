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
#include <string.h>
#include "lock_files.h"

int performBackup() {
	// Firstly lets define the time for right now
	time_t now = time(NULL);
	
	// Move to the root directory so we can move freely
	if(chdir("/") < 0) {
		printf("Cannot get to root, try running with sudo\n");
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
		if(mkdir("/var/lib/backup-daemon", 0777) == -1) {
			printf("Error creating app folder in /var/lib.\n");
		} else {
			// Create backups directory inside that
			if(mkdir("/var/lib/backup-daemon/backups", 0777) == -1) {
				printf("Error creating backups folder");
			}
		}
	} else {
		printf("Cant open the backup directory\n");
		exit(EXIT_FAILURE);
	}

	// If we got here we have either found the already existing directory
	// OR we have created it.
	// Now lets lock the directories.
	if(lockFiles("/var/www/html/intranet", "0000") == 0 && lockFiles("/var/www/html/live", "0000") == 0) {
		printf("Live and Intranet files locked\n");
	} else {
		printf("Unable to lock Live and Intranet files. Aborting backup.\n");
		exit(EXIT_FAILURE);
	}
	
	// Get a string representation of the current time
	char buff[20];

	strftime(buff, 20, "%Y-%m-%d", localtime(&now));
	
	// Lets create a directory in the backups folder with todays timestamp
	char backup_folder_path[200] = "/var/lib/backup-daemon/backups/";
	
	// Concatenate todays timestamp to the path for making the directory
	strcat(backup_folder_path, buff);

	printf("Creating Backup at:\n%s\n", backup_folder_path);
	
	// Check if the folder already exists	
	DIR* backup_dir = opendir(backup_folder_path);

	if(backup_dir) {
		// Already exists
		printf("Backup folder already exists, overwriting it.\n");
		
		// Delete the folder
		char command_buff[220] = "rm -r ";

		// Copy the command and the file name (which is todays date)
		strcat(command_buff, backup_folder_path);
		
		// Execute the rm command
		system(command_buff);
	}
	
	// If we have got here either the folder existed and we deleted it,
	// or it doesn't exist so lets create it.
	if(mkdir(backup_folder_path, 0777) == -1) {
		printf("Can't create today's backup folder.\n");
		exit(EXIT_FAILURE);
	} else {
		printf("Created today's backup folder.\n");

		// Now lets copy the intranet and the live folders to the backup folder
		char intranet_path[400] = "cp -a /var/www/html/intranet/. ";
		char live_path[400] = "cp -a /var/www/html/live/. ";

		// Concat the start of the intranet and live path with the path of the
		// backup folder destination
		strcat(intranet_path, backup_folder_path);
		strcat(live_path, backup_folder_path);

		// Need to concat each individual folder now
		strcat(intranet_path, "/intranet");
		strcat(live_path, "/live");

		// Now execute both
		system(intranet_path);
		system(live_path);
	}

	// Unlock the files, give them like 0444 mode
	if(lockFiles("/var/www/html/intranet", "0444") == 0 && lockFiles("/var/www/html/live", "0444") == 0) {
		printf("Live and Intranet files unlocked\n");
	} else {
		printf("Unable to unlock Live and Intranet files. Abortinf backup.\n");
		exit(EXIT_FAILURE);
	}
}
