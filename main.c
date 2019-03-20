/* Main File for the daemon. This file is responsible for choreographing the main aspects of the application. Initially it will start as a daemon
 * and will control other processes via message queues where appropriate. It will call processes for running backups and logs etc. More info
 * here as the program develops.
 * Author: Mark Barrett
 * Date: 09/03/2019
 * Git Repo: https://github.com/mark-barrett/Code-Backup-and-Deployment-Daemon
 */
#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <mqueue.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/types.h>
#include <semaphore.h>
#include "backup.h"
#include "logger.h"
#include "update.h"
#include "audit_log.h"

#define SEMAPHORE_NAME "/backup-daemon-semaphore"

// The signal handler
void sig_handler(int sigNum) {
	// For kill
	if(sigNum == SIGTERM) {
		recordLog("Daemon: Received kill instruction, releasing semaphore");	
		sem_unlink(SEMAPHORE_NAME);
		exit(1);
	}
}

int main() {
	
	// We need to setup a signal handler that will handle closing the semaphore if a kill is sent
	if(signal(SIGTERM, sig_handler) == SIG_ERR) {
		recordLog("Daemon: Cannot add signal handler for semaphore. Stopping program");
		return 0;
	}

	// We need to implement the singleton pattern to ensure that there is only one instance of this
	// program running.	
	// Let's change directory
	if(chdir("/") < 0) { exit(EXIT_FAILURE); }
	
	sem_t *sem;
	int rc;

	sem = sem_open(SEMAPHORE_NAME, O_CREAT, S_IRWXU, 1);

	if(sem == SEM_FAILED) {
		printf("Failed Sem: %d\n", errno);
	}

	rc = sem_trywait(sem);

	if(rc==0) {
		printf("Lock obtained \n");
	} else {
		printf("Sem not obtained\n");
		return 0;
	}

	// Code to make this a daemon
	int pid = fork();

	// Check for an error
	if(pid < 0) {
		// There was an error
		exit(EXIT_FAILURE);
	} else if(pid > 0) {
		// Is the parent
		exit(EXIT_SUCCESS);
	}

	// Step 1. Create the Orphan Child - If we have gotten to here then we are the child process
	// Done!
	
	// Step 2. Eleveate the orphan to the session leader.
	if(setsid() < 0) { exit(EXIT_FAILURE); }

	// Fork again to ensure that the process is not a session leader
	pid = fork();
	if(pid > 0) {
		exit(EXIT_SUCCESS);
	}

	// Step 3. Call unmask
	umask(0);

	// Step 4.  Change the current working directory to root.
	// Already done above.
	
	// Step 5. Close down file descriptors.
	int x;
	for(x = sysconf(_SC_OPEN_MAX); x>=0; x--) {
		close(x);
	}

	// Thats it!
	
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
		// Get the current time
		now = time(NULL);

		// Sleep for 1 second
		sleep(1);
		
		// Convert the time to a string
		strftime(time_as_string, 50, "%H:%M:%S", localtime(&now));

		printf("Time: %s\n", time_as_string);
		
		// Check to see if the time is midnight.
		// If it is then perform the backup
		if(strcmp(time_as_string, "14:40:00") == 0) {
			// First perform the backup
			performBackup();

			// Then update
			performUpdate();
		}

		// Try read from the message queue
		ssize_t bytes_read;
		
		bytes_read = mq_receive(mq, buffer, 1024, NULL);
		
	       	buffer[bytes_read] = '\0';
			
		// Check if its a terminate instruction
		if(!strncmp(buffer, "stop", strlen("stop"))) {
			terminate = 1;
		} else if(!strncmp(buffer, "backup", strlen("backup"))) {
			// If we need to force the backup
			recordLog("Forcing backup....");

			// Call the backup
			performBackup();

			// Record that we have done it
			recordLog("Backup force....");

			// We have forcing backup done, now set the buffer back to empty.
			memset(buffer, 0, 1024+1);
		} else if(!strncmp(buffer, "update", strlen("update"))) {
			// We want to force a transfer, but a backup is also part of this.
			// Lets log it
			
			recordLog("Forcing update (includes backup)....");

			// Call the backup
			performBackup();
			
			// Then update
			performUpdate();
			
			// Record that we have done it
			recordLog("Updated forced....");

			memset(buffer, 0, 1024+1);
			
		}
	} while(!terminate);
	
	// Unlink the semaphore
	sem_unlink(SEMAPHORE_NAME);

	return 0;
}
