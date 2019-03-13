/* Test C file to see if I can parse audit logs
 * Author: Mark Barrett
 * Date: 13/03/2019
 * Git Repo: https://github.com/mark-barrett/Code-Deployment-and-Backup-Daemon
 */
#include <stdio.h>

int main() {
	// Open the log file
	FILE *log_file = fopen("test-log.txt", "r");

	char line[256];

	while(fgets(line, sizeof(line), log_file)) {
		printf("Printing one thing\n\n\n\n");
		printf("%s", line);
	}	
}
