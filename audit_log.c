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
	fp = popen("ausearch -f /var/www/html/ | aureport -f | awk '/^[0-9]/ {printf \"%s %s %s %s %s\\n\", $2, $3, $4, $7, $8}' ", "r");
	
	// Define some variables that we want to get from each line
	char date[8] = "";
	char time[8] = "";
	char file[200] = "";
	char exe[200] = "";
	char auid[10] = "";
	int line = 0;
	int r = 0;

	// Do an fscanf to get the ball rolling
	r = fscanf(fp, "%s %s %s %s %s", date, time, file, exe, auid);

	// While we are not at the end of the file
	while(r != EOF) {
		// Increment the line as we are iterating over each line
		line++;
		
		// Do another fscanf
		r = fscanf(fp, "%s %s %s %s %s", date, time, file, exe, auid);

		// If we are past the first 3 lines then print
		printf("%s %s %s %s %s\n", date, time, file, exe, auid);
	}
		
	fclose(fp);

	return 0;
}
