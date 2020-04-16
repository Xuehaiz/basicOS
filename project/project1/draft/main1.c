#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>

#include "command.h"


int main(int argc, char const *argv[])
{
	/* Main Function Variables */
	FILE *fp;
	FILE *fout;
	int condition = 1;
	char *line;  
	char *line_cp = NULL;
	size_t len = 256; 
	char *token = NULL;
	char *token_cp = NULL;
	char *wrd_in_section = NULL;
	char *command = NULL;
	char *arg1 = NULL;
	char *arg2 = NULL;

	line = (char *)malloc(len * sizeof(char));
	if (line == NULL) {
		fprintf(stderr, "line allocation failure\n");
		exit(1);
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

			line_cp = line;
			
			// tokenize by ;
			token = strtok_r(line_cp, ";", &line_cp);
			token_cp = token;
			// for each sun-token tokenize by space
			wrd_in_section = strtok_r(token_cp, " ", &token_cp);

			// when token is null or empty,
			// convert it to be NULL
			if (token_cp == NULL) token_cp = "";
			if (strlen(token_cp) == 0) {
				token_cp = NULL;
			} 
			// since the first word is the command
			command = wrd_in_section;

			while (token != NULL) {
				if (strcmp(command, "ls") == 0) {
					if (token_cp == NULL) {
						listDir();
					}
					else {
						arg1 = strtok_r(token_cp, " ", &token_cp);
						// check repeat command
						if (strcmp(arg1, "ls") == 0) {
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
				else if (strcmp(command, "pwd") == 0) {
					if (strlen(token_cp) > 0) {
						arg1 = strtok_r(token_cp, " ", &token_cp);
						if (strcmp(arg1, "pwd") == 0) {
							printf("Error! Incorrect syntax, No control code found\n");
						}
						else {
							if (arg1 != NULL)
							printf("Error! Unsupported parameters for command: %s\n", command);
						}
					}
					if (strtok_r(token_cp, " ", &token_cp) != NULL) {
						printf("Error! Unsupported parameters for command: %s\n", command);
					}
					showCurrentDir();
				}
				else if (strcmp(command, "mkdir") == 0) {
					wrd_in_section = strtok_r(token_cp, " ", &token_cp);
					arg1 = wrd_in_section;
					makeDir(arg1); 
				}
				else if (strcmp(command, "cd") == 0) {
					wrd_in_section = strtok_r(token_cp, " ", &token_cp);
					arg1 = wrd_in_section;
					changeDir(arg1);
				}
				else if (strcmp(command, "cp") == 0) {
					wrd_in_section = strtok_r(token_cp, " ", &token_cp);
					arg1 = wrd_in_section;
					wrd_in_section = strtok_r(token_cp, " ", &token_cp);
					arg2 = wrd_in_section;
					copyFile(arg1, arg2);
				}
				else if (strcmp(command, "mv") == 0) {
					wrd_in_section = strtok_r(token_cp, " ", &token_cp);
					arg1 = wrd_in_section;
					wrd_in_section = strtok_r(token_cp, " ", &token_cp);
					arg2 = wrd_in_section;
					moveFile(arg1, arg2);
				}
				else if (strcmp(command, "rm") == 0) {
					wrd_in_section = strtok_r(token_cp, " ", &token_cp);
					arg1 = wrd_in_section;
					deleteFile(arg1);
				}
				else if (strcmp(command, "cat") == 0) {
					wrd_in_section = strtok_r(token_cp, " ", &token_cp);
					arg1 = wrd_in_section;
					displayFile(arg1);
				}
				else {
					printf("Error! Unrecognized command: %s\n", command);
				}
				if (line_cp != NULL) {
					token = strtok_r(line_cp, ";", &line_cp);
					token_cp = token;
					wrd_in_section = strtok_r(token_cp, " ", &token_cp);
					if (wrd_in_section != NULL) {
						command = wrd_in_section;
					}
				} else {
					command = NULL; 
					token = NULL;
					token_cp = NULL;
					wrd_in_section = NULL;
				}
				//printf("token: %s\n", token);
				
			}
		}
	}
	// free memory and close file 
	free(line);

	return 0;
}