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

// The signal handler to remove the lock
void sig_handler(int sigNum) {
	// For kill
	if(sigNum == SIGTERM) {
		recordLog("Daemon: Received kill instruction, releasing lockfile.");
		if(remove("/var/run/backup-daemon.pid") == 0) {
			recordLog("Daemon: Lockfile unlocked");
		}
		
		exit(1);
	}
}

int main() {	

	// We need to setup a signal handler that will handle closing the semaphore if a kill is sent
	if(signal(SIGTERM, sig_handler) == SIG_ERR) {
		recordLog("Daemon: Cannot add signal handler for singleton. Stopping program");
		return 0;
	}

	// We need to implement the singleton pattern to ensure that there is only one instance of this
	// program running.	
	// Let's change directory
	if(chdir("/") < 0) { exit(EXIT_FAILURE); }
	
	// Try open the lock file.
	FILE *lock_file = fopen("/var/run/backup-daemon.pid", "a+");	
	long file_size = 0;

	if(lock_file != NULL) {
		fseek(lock_file, 0, SEEK_END);
		file_size = ftell(lock_file);
	}

	// Check to see if its 0
	if(file_size == 0) {
		// No other programs running, write the pid
		fprintf(lock_file, "%d", getpid());
		// Close the file
		fclose(lock_file);
	} else {
		// There is a program running, log and print
		recordLog("Daemon: Cannot start daemon as an instance is already running");
		
		fclose(lock_file);

		// Print
		printf("Cannot start daemon as there is an instance already running\n");
		return 0;
	}

	/*struct flock fl;
	int fd;

	fd = open("/var/run/backup-daemon.pid", O_RDWR | O_CREAT, 0600);

	fl.l_start = 0;
	fl.l_len = 0;
	fl.l_type = F_WRLCK;
	fl.l_whence = SEEK_SET;
	
	if(fcntl(fd, F_GETLK, &fl) < 0) {
		printf("Another instance of the program is running\n");
	}

	close(fd);	

	printf("%d", fl.l_type);
	*/

	/*
	int pid_file = open("/var/run/backup-daemon.pid", O_CREAT | O_RDWR, 0666);

	int rc = flock(pid_file, LOCK_EX);
	printf("%d", rc);
	if(rc) {
	
	} else {
	
	}
	*/

	/*
	sem_t *sem;
	int rc;

	sem = sem_open(SEMAPHORE_NAME, O_CREAT, S_IRWXU, 1);

	if(sem == SEM_FAILED) {
		recordLog("Daemon: Cannot create semaphore so cannot continue.");
	}

	rc = sem_trywait(sem);

	if(rc==0) {
		recordLog("Daemon: Obtained singleton lock.");
	} else {
		recordLog("Daemon: An instance of this daemon is already runnning.");
		return 0;
	}
	*/
	
	recordLog("Daemon: Creating daemon process\n");
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
	
	// Unlock the file as not used by deleting it
	if(remove("/var/run/backup-daemon.pid") == 0) {
		recordLog("Daemon: Lockfile unlocked");
	}
	return 0;
}
