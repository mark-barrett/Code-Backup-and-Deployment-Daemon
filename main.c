/* Main File for the daemon. This file is responsible for choreographing the main aspects of the application. Initially it will start as a daemon
 * and will control other processes via message queues where appropriate. It will call processes for running backups and logs etc. More info
 * here as the program develops.
 * Author: Mark Barrett
 * Date: 09/03/2019
 * Git Repo: https://github.com/mark-barrett/Code-Backup-and-Deployment-Daemon
 */
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <mqueue.h>
#include "backup.h"
#include "logger.h"
#include "update.h"
#include "audit_log.h"

int main() {

	// Put a watch on the /var/www/html directory.
	// If it is already there then don't worry.
	system("auditctl -w /var/www/html -p rwxa");
	
	// We need to setup a message queue so that the force backup program
	// can tell us to force a backup
	mqd_t mq;
	struct mq_attr queue_attributes;
	char buffer[1024+1];
	int terminate = 0;

	// Set the queue attributes
	queue_attributes.mq_flags = 0;
	queue_attributes.mq_maxmsg = 10;
	queue_attributes.mq_msgsize = 1024;
	queue_attributes.mq_curmsgs = 0;

	// Open the message queue
	// Use O_NONBLOCK for asynchronous operations
	mq = mq_open("/backup-daemon", O_CREAT | O_RDONLY | O_NONBLOCK, 0644, &queue_attributes);

	// Define the time
	time_t now;
	char time_as_string[50];
	
	do {
		now = time(NULL);
		sleep(1);

		strftime(time_as_string, 50, "%H:%M:%S", localtime(&now));
		printf("%s\n", time_as_string);

		if(strcmp(time_as_string, "11:23:00") == 0) {
			// First perform the backup
			performBackup();

			// Then update
			performUpdate();

			// Generate the audit logs
			generateAuditLogs();
		}

		// Try read from the message queue
		ssize_t bytes_read;
		
		bytes_read = mq_receive(mq, buffer, 1024, NULL);
		
	       	buffer[bytes_read] = '\0';

		// Check if its a terminate instruction
		if(!strncmp(buffer, "stop", strlen("stop"))) {
			terminate = 1;
		} else if(!strncmp(buffer, "backup", strlen("backup"))) {
			printf("Forcing Backup\n");

			// We have forcing backup done, now set the buffer back to empty.
			memset(buffer, 0, 1024+1);
		}
	} while(!terminate);
	
	return 0;
}
