#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char const *argv[])
{
	// Variable declarations
	if (argc != 0) {
		printf("haha\n");;
	}
	FILE *fp = fopen(argv[1], "r");
	char *token;
	size_t len = 256; 
	int counter = 0;
	int i = 0;
	int j = 0;
	int numprograms = 0;
	// memory allcation
	char *line = (char *)malloc(256 * sizeof(char));
	if (line == NULL) {
		fprintf(stderr, "line allocation failure\n");
		exit(EXIT_FAILURE);
	}
	char **arg_arr = (char **)malloc(len * sizeof(char *));
	if (arg_arr == NULL) {
		fprintf(stderr, "Buffer allocation failure\n");
		exit(EXIT_FAILURE);
	}

	pid_t *pid = (pid_t *)malloc(256 * sizeof(pid_t));
	if (pid == NULL) {
		fprintf(stderr, "PID allocation failure\n");
		exit(EXIT_FAILURE);
	}
	pid_t curr_pid;
	while (getline(&line, &len, fp) != EOF) {
		// printf("here?\n");
		numprograms++;
		counter = 0;
		line[strlen(line) - 1] = '\0';
		while ((token = strtok_r(line, " \n", &line))) {
			arg_arr[counter] = token;
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
			printf("Executing: %s\n", arg_arr[0]);
			execvp(arg_arr[0], arg_arr);
			fprintf(stderr, "PID: %d ", getpid());
			fprintf(stderr, "log error. Failed to start program: %s\n", arg_arr[0]);
			exit(-1);
		}
<<<<<<< HEAD
		i++;
	}
	
	
=======
		pid[i] = curr_pid;
		i++;
	}
	
>>>>>>> bc8ec6d02a72822f1d05573ca7e3fc465b93e184
	for (int i = 0; i < numprograms; i++) {
		waitpid(pid[i], NULL, 0);
		printf("wait pid[%d]: %d\n", i, getpid());
	}


	fclose(fp);

	free(line);
	printf("here?\n");
	free(pid);
	free(arg_arr);
	return 0;
}