/*
* Description: <write a brief description of your lab>
*
* Author: Xuehai Zhou
*
* Date: Apr 14, 2020
*
* Notes:
* 1. <add notes we should consider when grading>
*/

/*-------------------------Preprocessor Directives---------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "command.h"
/*---------------------------------------------------------------------------*/

/*-----------------------------Program Main----------------------------------*/


int main(int argc, char const *argv[])
{
	// Variable declarations
	int condition = 1;
	FILE *fp;
	int i;
	char *line_cp = NULL;
	size_t len = 0; 
	char *token = NULL;
	char *token_cp = NULL;
	char *cmd_in_line = NULL;
	char *command = NULL;
	char *arg1 = NULL;
	char *arg2 = NULL;

	// memory allcation
	char *line = (char *)malloc(len * sizeof(char));
	if (line == NULL) {
		fprintf(stderr, "line allocation failure\n");
		exit(EXIT_FAILURE);
	}

	/* read file */
	if (argv[1] != NULL) {
		fp = fopen(argv[1], "r");
	}
	else {
		fp = stdin;
	}

	/*main run loop*/
	while (condition) {
		printf(">>> ");
		getline(&line, &len, fp);
		line[strlen(line) - 1] = '\0';
		// handle if line starts with a ;
		if (line[0] == ';') {
			printf("Error! Unrecognized command: %s\n", command);
			line++;
		}

		if (strcmp(line, "exit") == 0) break;
		else {
			i = 0;
			line_cp = line;
			
			// tokenize by ;
			token = strtok_r(line_cp, ";", &line_cp);
			token_cp = token;
			// for each sun-token tokenize by space
			cmd_in_line = strtok_r(token_cp, " ", &token_cp);

			// when token is null or empty,
			// convert it to be NULL
			if (token_cp == NULL) token_cp = "";
			if (strlen(token_cp) == 0) {
				token_cp = NULL;
			} 
			// since the first word is the command
			command = cmd_in_line;

			while (token != NULL) {
				if (strcmp(command, "lfcat") == 0) {
					if (token_cp == NULL) {
						lfcat();
					}
					else {
						arg1 = strtok_r(token_cp, " ", &token_cp);
						// check repeat command
						if (strcmp(arg1, "lfcat") == 0) {
							printf("Error! Incorrect syntax, No control code found\n");
							break;
						}
						// check unvalid argument
						else {
							if (arg1 != NULL)
							printf("Error! Unsupported parameters for command: %s\n", command);
							break;
						}
					} 
				}
				else {
					printf("Error! Unrecognized command: %s\n", command);
				}
				if (line_cp != NULL) {
					token = strtok_r(line_cp, ";", &line_cp);
					token_cp = token;
					cmd_in_line = strtok_r(token_cp, " ", &token_cp);
					if (cmd_in_line != NULL) {
						command = cmd_in_line;
					}
				} else {
					command = NULL; 
					token = NULL;
					token_cp = NULL;
					cmd_in_line = NULL;
				}
			}
		}
	}
	if (argc == 3 && strcmp(argv[1], "-f") == 0) {
		FILE *fout;
		fout = freopen(stdout, "w+", argv[2]);
		fclose(fout);
	}
	/*Free the allocated memory and close any open files*/	

	return 0;
}

/*-----------------------------Program End-----------------------------------*/
