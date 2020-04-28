#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char const *argv[])
{
	// Variable declarations
	FILE *fp;
	char *token;
	char *line_cp;
	size_t len = 256; 
	int counter = 0;

	// memory allcation
	char *line = (char *)malloc(len * sizeof(char));
	if (line == NULL) {
		fprintf(stderr, "line allocation failure\n");
		exit(EXIT_FAILURE);
	}
	char **token_arr = (char **)malloc(len * sizeof(char *));
	if (token_arr == NULL) {
		fprintf(stderr, "Buffer allocation failure\n");
		exit(EXIT_FAILURE);
	}

	while (getline(&line, &len, fp) != EOF) {
		counter = 0;
		line_cp = line;
		while (token = strtok_r(line_cp, " ", &line_cp)) {
			token_arr[counter] = token;
		}
	}
	return 0;
}