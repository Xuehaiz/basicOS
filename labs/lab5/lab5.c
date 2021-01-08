#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>


void sig_handler(int mysignal) {
	printf("my signal: %d\n", signal);
}

int main(int argc, char const *argv[])
{
	pid_t *pid = (pid_t *)malloc(256 * sizeof(pid_t));
	if (pid == NULL) {
		fprintf(stderr, "PID allocation failure\n");
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < 5; i++) {
		pid[i] = fork();
		if (pid[i] < 0) {
			perror("fork");
			exit(EXIT_FAILURE);
		}
		else if (pid[i] == 0) {
			printf("Child process created\n");
			execlp("./iobound", "iobound", "-minutes", "1", NULL);
			fprintf(stderr, "Didn't enter exec(), report error\n");
		}
		else {
			printf("in parent process\n");
		}
	}
	return 0;
}