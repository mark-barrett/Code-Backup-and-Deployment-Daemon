/* This C file is used to get the changed files and their users from the audit log files.
 * Author: Mark Barrett
 * Date: 13/03/2019
 * Git Repo: https://github.com/mark-barrett/Code-Deployment-and-Backup-Daemon
 */
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "logger.h"

int generateAuditLogs() {

	// Get to the root directory
	if(chdir("/") < 0) {
		recordLog("Audit Log: Can't get to root, try running with sudo");
		exit(EXIT_FAILURE);
	}

	// We need pull the recent changes using ausearch and pipe them to the aureport
	// command to get a useable list of information.
	// We are going to use a pipe to execute our command and then return its result.
	
	// Because we don't need all of the data and it can look really awful, we can use
	// awk to clean things up a bit and then carry on parsing the resulting information.
	// Can specify a time if needed in the future
	FILE *fp;
	fp = popen("ausearch -f /var/www/html/ | aureport -f | awk '/^[0-9]/ {printf \"%s %s %s %s %s\\n\", $2, $3, $4, $7, $8}'", "r");
	
	char line[256];
	
	// Read line by line
	while(fgets(line, sizeof(line), fp)) {
		// We need to break this into tokens to get each element
		char *token = token = strtok(line, " ");
		
		// Variables we need to output our writing
		int counter = 0;
		char date[9] = "";
		char time[9] = "";
		char file[200] = "";
		char exe[200] = "";
		char auid[10] = "";

		// Loop through words and print
		while(token != NULL) {
			// If its the 0th, to 4th token, get the relevant column
			switch(counter) {
				case 0:
					strcpy(date, strcat(token, "\0"));
					break;
				case 1:
					strcpy(time, strcat(token, "\0"));
					break;
				case 2:
					strcpy(file, strcat(token, "\0"));
					break;
				case 3:
					strcpy(exe, strcat(token, "\0"));
					break;
				case 4:
					strcpy(auid, strcat(token, "\0"));
					break;
			}
			
			// Move to the next token
			token = strtok(NULL, " ");

			// Increment the counter for the next column
			counter++;
		}
		
		// Print the file
		printf("File: %s\n", file);
		
		// Print the program that altered it
		printf("Exe: %s\n", exe);

		// Need to get the user using the id
		if(strcmp(auid, "-1") != 0 || strcmp(auid, "1") != 0) {
			// Use a pipe to get the user using the id -nu command
			FILE *user_pipe;

			// Define the command
			char user_command[50] = "id -nu ";

			// Concat the user auid
			strcat(user_command, auid);

			// Open the pipe
			user_pipe = popen(user_command, "r");

			char username[25] = "";
			
			fgets(username, sizeof(username), user_pipe);

			printf("User: %s\n", username);

		}
		// Print the file change
		printf("[File Change]\nDate: %s @ %s\n", date, time);
	
	}

	fclose(fp);

	return 0;
}
