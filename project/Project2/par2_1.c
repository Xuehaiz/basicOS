#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>


void sig_handler(int sig, siginfo_t *siginfo, void *context) {  // 
	printf("SIGNAL RECEIVED!\n");

	sigset_t sigset;
  	sigemptyset(&sigset);
  	sigaddset(&sigset, SIGUSR1);
  	sigprocmask(SIG_BLOCK, &sigset, NULL);

  	// unblock if block
  	if (sigwait(&sigset, &sig) == 0) {
  		printf("Unblock here\n");
  	}
}

int main(int argc __attribute__((unused)), char const *argv[])
{
	// Variable declarations
	char *token;
	int len = 256; 
	char line[len];
	int counter = 0;
	// char arg_arr[len][len];
	// int i = 0;
	int numprograms = 0;

	// sig struct and handler
	struct sigaction sa;
	if (memset (&sa, '\0', sizeof(sa)) == NULL) {
		perror("memset");
	}
	sa.sa_sigaction = &sig_handler;
	if (sigaction(SIGUSR1, &sa, NULL) == -1) {
		perror("sigaction");
	}

	// open file
	FILE *fp = fopen(argv[1], "r");
	if (fp == NULL) {
		fprintf(stderr, "File <%s> open failure.\n", argv[1]);
		exit(EXIT_FAILURE);
	}
	// memory allcation
	char ***arg_arr = (char ***)malloc(len * sizeof(char **));
	if (arg_arr == NULL) {
		fprintf(stderr, "Buffer allocation failure\n");
		fclose(fp);
		exit(EXIT_FAILURE);
	}
	pid_t *pid = (pid_t *)malloc(len * sizeof(pid_t));
	if (pid == NULL) {
		fprintf(stderr, "pid list allocation failure\n");
		fclose(fp);
		for (int i = 0; i < len; i++) {
			free(arg_arr[i]);
		}
		free(arg_arr);
		exit(EXIT_FAILURE);
	}

	while (fgets(line, len, fp) != NULL) {
		counter = 0;
		token = strtok(line, " \n");
		while (token != NULL) {
			arg_arr[numprograms][counter] = token;
			token = strtok(NULL, " \n");
			counter++;
		}
		arg_arr[numprograms][counter] = NULL;
		numprograms++;
	}

	for (int i = 0; i < numprograms; i++) {
		pid[i] = fork();
		if (pid[i] < 0) {
			perror("fork error, no child created");
			free(pid);
			for (int i = 0; i < len; i++) {
				free(arg_arr[i]);
			}
			// free(arg_arr);
			fclose(fp);
			exit(EXIT_FAILURE);
		}

		// send signal to pid[i] to stop cihld
		kill(pid[i], SIGUSR1);
		printf("Child process: <%d> - Waiting for SIGUSR1...\n", pid[i]);
		if (pid[i] == 0) {
			for (int j = 0; j < 2; j++) {
				printf("pid[%d]: %d\n", i, pid[i]);
				sleep(1);
			}
			printf("Child process is %d, and parent pid is %d\n", getpid(), getppid());
			printf("My status is %d\n\n", pid[i]);
			printf("Running: %s\n", arg_arr[i][0]);
			execvp(arg_arr[i][0], arg_arr[i]);
			fclose(fp);
			perror("execvp");
			free(pid);
			// free(arg_arr);
			_exit(-1);
		}
	}

	for (int i = 0; i < numprograms; i++) {
		kill(pid[i], SIGUSR1);
	}
	
	for (int i = 0; i < numprograms; i++) {
		waitpid(pid[i], NULL, 0);
		// wait(0);
		printf("wait pid[%d]: %d\n", i, pid[i]);
	}

	for (int i = 0; i < numprograms; i++) {
		kill(pid[i], SIGSTOP);
	}

	for (int i = 0; i < numprograms; i++) {
		kill(pid[i], SIGCONT);
	}

	// exit all
	printf("All processes finished: parent exiting: my pid is %d \n\n", getpid());
	fclose(fp);
	free(pid);
	for (int i = 0; i < len; i++) {
		free(arg_arr[i]);
	}
	free(arg_arr);
	return 0;
}