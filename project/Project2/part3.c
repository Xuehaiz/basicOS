#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>


/*void sig_handler(int sig, siginfo_t *siginfo, void *context) {  // 
	printf("SIGNAL RECEIVED!\n");

	sigset_t sigset;
  	sigemptyset(&sigset);
  	sigaddset(&sigset, SIGUSR1);
  	sigprocmask(SIG_BLOCK, &sigset, NULL);

  	// unblock if block
  	if (sigwait(&sigset, &sig) == 0) {
  		printf("Unblock here\n");
  	}
}*/


void sig_handler(int sig) {  // 
	printf("Child process: %d - recieved signal: %d", getpid(), sig);

  	// unblock if block
  	/*if (sigwait(&sigset, &sig) == 0) {
  		printf("Unblock here\n");
  	}*/
}

int main(int argc __attribute__((unused)), char const *argv[])
{
	// Variable declarations
	int condition = 1;
	char *token;
	int len = 256; 
	char line[len];
	int counter = 0;
	int i = 0;
	int numprograms = 0;
	// open file
	FILE *fp = fopen(argv[1], "r");
	if (fp == NULL) {
		fprintf(stderr, "File <%s> open failure.\n", argv[1]);
		exit(EXIT_FAILURE);
	}
	// memory allcation
	char **arg_arr = (char **)malloc(len * sizeof(char *));
	if (arg_arr == NULL) {
		fprintf(stderr, "Buffer allocation failure\n");
		exit(EXIT_FAILURE);
	}
	pid_t *pid = (pid_t *)malloc(len * sizeof(pid_t));
	if (pid == NULL) {
		fprintf(stderr, "pid list allocation failure\n");
		exit(EXIT_FAILURE);
	}

	// sig struct and handler
	int status;
	sigset_t sigset;
  	sigemptyset(&sigset);
  	sigaddset(&sigset, SIGUSR1);
  	sigprocmask(SIG_BLOCK, &sigset, NULL);
	struct sigaction sa;
	if (memset (&sa, '\0', sizeof(sa)) == NULL) {
		perror("memset");
	}
	sa.sa_handler = &sig_handler;
	if (sigaction(SIGUSR1, &sa, NULL) == -1) {
		perror("SIGUSR1");
	}
	if (sigaction(SIGCONT, &sa, NULL) == -1) {
		perror("SIGCONT");
	}

	while (fgets(line, len, fp) != NULL) {
		numprograms++;
		counter = 0;
		token = strtok(line, " \n");
		while (token != NULL) {
			arg_arr[counter] = token;
			token = strtok(NULL, " \n");
			counter++;
		}
		arg_arr[counter] = NULL;

		pid[i] = fork();
		if (pid[i] < 0) {
			perror("fork error, no child created");
			fclose(fp);
			exit(EXIT_FAILURE);
		}

		// send signal to pid[i] to stop cihld
		// kill(pid[i], SIGUSR1);

		if (pid[i] == 0) {
    		printf("Child process: %d - Starting execution.\n", getpid());
		    // Waiting for SIGUSR1
		    printf("Child process: %d - Waiting for SIGALRM...\n", getpid());
		    int signal = SIGALRM;
		    if (sigwait(&sigset, &signal) == 0) {
		    	printf("Child process: %d - Received signal: SIGUSR1 - calling exec().\n", getpid());
		    }
		    // Exec call
		    execvp(arg_arr[0], arg_arr);
		    // error report and free 
		    perror("execvp");
		    fclose(fp);
			free(pid);
			free(arg_arr);
		    // End the child process
		    _exit(-1);
		}
		else {
		    //Send SIGUSR1 to the children
		    printf("Parent process: %d - Sending signal: %d to child process: %d\n", getpid(), SIGUSR1, pid[i]);
		    kill(pid[i], SIGUSR1);
		    printf("Parent process: %d - Signal SIGUSR1 sent.\n", getpid());
  		}
		i++;
	}
	
	for (int i = 0; i < numprograms; i++) {
		waitpid(pid[i], NULL, 0);
		// wait(0);
		printf("wait pid[%d]: %d\n", i, pid[i]);
	}
	for (int i = 0; i < numprograms; i++) {
		printf("Sending signal: %d to pid: %d\n", SIGSTOP, pid[i]);
		kill(pid[i], SIGSTOP);
		printf("Signal SIGSTOP sent.\n");
	}
	for (int i = 0; i < numprograms; i++) {
		printf("Sending signal: %d to pid: %d\n", SIGCONT, pid[i]);
		kill(pid[i], SIGCONT);
		printf("Signal SIGCONT sent.\n");
	}

	// exit all
	printf("All processes finished: parent exiting: my pid is %d \n\n", getpid());
	fclose(fp);
	free(pid);
	free(arg_arr);
	return 0;
}