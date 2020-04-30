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
	int j = 0;
	int numprograms = 0;
	// open file
	FILE *fp = fopen(argv[1], "r");
	/*if (fp == NULL) {
		fprintf(stderr, "File <%s> open failure.\n", argv[1]);
		exit(EXIT_FAILURE);
	}*/
	// memory allcation
	/*char *line = (char *)malloc(len * sizeof(char));
	if (line == NULL) {
		fprintf(stderr, "line allocation failure\n");
		exit(EXIT_FAILURE);
	}*/
	char **arg_arr = (char **)malloc(len * sizeof(char *));
	if (arg_arr == NULL) {
		fprintf(stderr, "Buffer allocation failure\n");
		exit(EXIT_FAILURE);
	}
	pid_t *pid = (pid_t *)malloc(len * sizeof(pid_t));
	/*if (pid == NULL) {
		fprintf(stderr, "PID allocation failure\n");
		exit(EXIT_FAILURE);
	}*/
	pid_t curr_pid;
	while (fgets(line, len, fp) != NULL) {
		// printf("here?\n");
		numprograms++;
		counter = 0;
		// line[strlen(line) - 1] = '\0';
		token = strtok(line, " \n");
		while (token != NULL) {
			arg_arr[counter] = token;
			token = strtok(NULL, " \n");
			counter++;
		}
		j = 0;
		while (arg_arr[j]) {
			if (j >= counter) {
				arg_arr[j] = NULL;
			}
			// printf("arg_arr[%d]: %s\n", j, arg_arr[j]);
			j++;
		}
		arg_arr[j] = NULL;
		curr_pid = fork();
		// printf("pid: %d\n", pid[i]);
		if (curr_pid < 0) {
			perror("fork error, no child created");
			exit(EXIT_FAILURE);
		}
		if (curr_pid == 0) { /* child process */
			printf("Child process is %d, and parent pid is %d\n", getpid(), getppid());
			execvp(arg_arr[0], arg_arr);
			fprintf(stderr, "PID: %d ", getpid());
			fprintf(stderr, "log error. Failed to start program: %s\n", arg_arr[0]);
			exit(-1);
		}
		pid[i] = curr_pid;
		i++;
	}
	
	for (int i = 0; i < numprograms; i++) {
		waitpid(pid[i], NULL, 0);
		printf("wait pid[%d]: %d\n", i, getpid());
	}

	fclose(fp);

	// free(line);
	free(pid);
	free(arg_arr);
	return 0;
}