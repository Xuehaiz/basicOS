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
	FILE *fp = fopen(argv[1], "r");
	int numprograms;
	char *token;
	size_t len = 256; 
	pid_t *pid = (pid_t *)malloc(5 * sizeof(pid_t));
	if (pid == NULL) {
		fprintf(stderr, "PID allocation failure\n");
		exit(EXIT_FAILURE);
	}
	char *line = (char *)malloc(len * sizeof(char));
	if (line == NULL) {
		fprintf(stderr, "line allocation failure\n");
		exit(EXIT_FAILURE);
	}
	char **arg_arr = (char **)malloc(len * sizeof(char *));
	if (arg_arr == NULL) {
		fprintf(stderr, "Buffer allocation failure\n");
		exit(EXIT_FAILURE);
	}

	char *args[] = {"-a", "-l", NULL};
	printf("My pid is %d\n\n", getpid());
	numprograms = 0;
	//while (getline(&line, &len, fp) != EOF) {
	pid[numprograms] = fork();
	printf("pid[%d]: %d\n", numprograms, pid[numprograms]);
	if (pid[numprograms] < 0) {
		perror("fork error, no child process created");
		exit(EXIT_FAILURE);
	}
	if (pid[numprograms] == 0) {
		execvp("ls", args);
	}
	numprograms++;
	//}
	for (int i = 0; i < numprograms; i++) {
		wait(NULL);
	}

	return 0;
}