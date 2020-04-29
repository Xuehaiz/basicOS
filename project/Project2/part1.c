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
	// int i = 0;
	int numprograms = 0;
	// char arg_arr[len][len];
	// open file
	FILE *fp = fopen(argv[1], "r");
	if (fp == NULL) {
		fprintf(stderr, "File <%s> open failure.\n", argv[1]);
		exit(EXIT_FAILURE);
	}
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
	if (pid == NULL) {
		fprintf(stderr, "pid list allocation failure\n");
		exit(EXIT_FAILURE);
	}

	// pid_t pid[len];
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
		arg_arr[counter] = NULL;
/*		i = 0;
		while (arg_arr[i]) {
			if (i >= counter) {
				arg_arr[i] = NULL;
		}
			// printf("arg_arr[%d]: %s\n", j, arg_arr[j]);
			i++;
		}*/
	}

	for (int i = 0; i < numprograms; i++) {
		pid[i] = fork();
		printf("forked, current process: %d\n", pid[i]);
		if (pid[i] < 0) {
			perror("fork error, no child created");
			exit(EXIT_FAILURE);
		}
		if (pid[i] == 0) { /* child process */
			fclose(fp);
			printf("Child process is %d, and parent pid is %d\n", getpid(), getppid());
			printf("My status is %d\n\n", pid[i]);
			execvp(arg_arr[0], arg_arr);
			perror("execvp");
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

	// free(line);
	free(pid);
	free(arg_arr);
	return 0;
}