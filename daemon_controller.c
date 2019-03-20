/* This is a seperate C program that is used to send messages to the main backup daemon in order to either stop it or to force a backup etc.
 * Author: Mark Barrett
 * Date: 19/03/2019
 * Git Repo: https://github.com/mark-barrett/Code-Backup-and-Deployment-Daemon
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <semaphore.h>

int main() {
	// Create the message queue
	mqd_t mq;
	char buffer[1024];

	// Open it
	mq = mq_open("/backup-daemon", O_WRONLY);

	do {
		printf("------ Daemon Controller ------\n");
		printf("Choose one of the following options:\n");
		printf("update: forces update (includes backup)\n");
		printf("backup: forces backup\n");
		printf("lock: removes sem lock\n");
		printf("stop: stops daemon\n");
		printf("> ");
		fflush(stdout);

		memset(buffer, 0, 1024);

		fgets(buffer, 1024, stdin);
		
		// Check what the user enters for feedback
		if(strncmp(buffer, "backup", sizeof("backup")) != 0) {
			printf("\nForcing the daemon to backup....\n\n");
		} else if(strncmp(buffer, "update", sizeof("update")) != 0) {
			printf("\n Forcing the daemon to update....\n\n");
		}

		mq_send(mq, buffer, 1024, 0);

	} while(strncmp(buffer, "stop", strlen("stop")));
	
	printf("Stopping the daemon. Bye bye!\n");

	mq_close(mq);

	return 0;
}
