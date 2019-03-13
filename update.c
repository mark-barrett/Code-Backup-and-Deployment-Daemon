/* This C file is responsible for updating the changes from the intranet folder to the live folder.
 * Author: Mark Barrett
 * Date: 13/03/2019
 * Git Repo: https://github.com/mark-barrett/Code-Deployment-and-Backup-Daemon
 */
#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include "logger.h"
#include "lock_files.h"

int performUpdate() {
	// Get to root just so we can move around freely.
	if(chdir("/") < 0) {
		recordLog("Update: Can't get to root, try running with sudo.");
		exit(EXIT_FAILURE);
	}

	// Do some checks to make sure that the intranet and live files exists
	DIR* intranet_folder = opendir("/var/www/html/intranet");
	DIR* live_folder = opendir("/var/www/html/live");

	// Check if they both exist
	if(intranet_folder && live_folder) {
		// They do exist lets perform the copy.
		// For fun, we are going to store the changed files in a log of its own.
		// Let's create the folder for this if it doens't exist.
		DIR* update_log_dir = opendir("/var/log/backup-daemon/update-logs");

		if(update_log_dir) {
			printf("Log file location for update exists.\n");
		} else if(ENOENT == errno) {
			// Let's create it
			if(mkdir("/var/log/backup-daemon/update-logs", 0777) == -1) {
				recordLog("Update: Error creating update logs folder");
			}
		}
		// If we are here the logs folder for the update exists or we created it.
		// Let's get the time.
		time_t now = time(NULL);

		// Convert it to a string
		char time_as_string_and_filename[100];

		strftime(time_as_string_and_filename, 100, "/var/log/backup-daemon/update-logs/%Y-%m-%d-%H:%M:%S.log ", localtime(&now));

		// The copy is going to use the -u flag to ensure only altered files are taken
		// It will also use the -r command too.
		char copy_command[200] = "cp -u -r -v > ";
		
		// Copy the update logs and time filename to the command
		strcat(copy_command, time_as_string_and_filename);

		// We need to add the files to copy from and too now.	
		// First lets lock the files so that no one can access them.
		if(lockFiles("/var/www/html/intranet", "0000") == 0 && lockFiles("/var/www/html/live", "0000") == 0) {
			// Files locked
			recordLog("Update: Locked files before performing copy.");

			// Now the files are locked.
			// We need to add the directories to the copy command
			strcat(copy_command, "/var/www/html/intranet/. /var/www/html/live");

			// Execute the command
			recordLog("Update: Starting update....");

			if(system(copy_command) == -1) {
				// There has been an error
				recordLog("Update: ERROR UPDATING SITE. Command failed :(");
			} else {
				char log_message[300] = "Update: Update successful, changes now live. Log of update files at: ";

				// Concat the message and the log file directory
				strcat(log_message, time_as_string_and_filename);

				recordLog(log_message);
				
				// All good. Now unlock the files.
				if(lockFiles("/var/www/html/intranet", "0444") == 0 && lockFiles("/var/www/html/live", "0444") == 0) {
					recordLog("Update: Folders unlocked and can now be edited");
				} else {
					recordLog("Update: Can't unlock files after the update. Sorry about that :(");
					exit(EXIT_FAILURE);
				}
			}


		} else {
			// Can't lock the folders for some reason
			recordLog("Update: Can't lock files before performing update, try running with sudo.");
			exit(EXIT_FAILURE);
		}
	} else {
		recordLog("Update: Intranet and or Live file does not exist. Aborting update");
		exit(EXIT_FAILURE);
	}
	return 0;
}
