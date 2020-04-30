#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc __attribute__((unused)), char const *argv[])
{
	// Variable declarations
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
		if (pid[i] == 0) {
			printf("Child process is %d, and parent pid is %d\n", getpid(), getppid());
			printf("My status is %d\n\n", pid[i]);
			printf("Running: %s\n", arg_arr[0]);
			execvp(arg_arr[0], arg_arr);
			fclose(fp);
			perror("execvp");
			free(pid);
			free(arg_arr);
			_exit(-1);
		}
		i++;
	}
	
	for (int i = 0; i < numprograms; i++) {
		waitpid(pid[i], NULL, 0);
		// wait(0);
		printf("wait pid[%d]: %d\n", i, getpid());
	}

	// exit all
	printf("All processes finished: parent exiting: my pid is %d \n\n", getpid());
	fclose(fp);
	free(pid);
	free(arg_arr);
	return 0;
}