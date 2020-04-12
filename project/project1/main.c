#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>


int main(int argc, char const *argv[])
{
	/* Main Function Variables */
	FILE *fp = stdin;  
	char *line;  // stores each line's value
	size_t len = 0; 
	char *token = NULL;
	char exit[5] = "exit";
	char flag[15];
			
	/*main run loop*/
	while (1) {
		printf(">>> ");
		getline(&line, &len, fp);
		line[strlen(line) - 1] = '\0';
		if (strcmp(line, exit) == 0) break;
		else {
			printf("%s\n",line);
		}
	}
	return 0;
}