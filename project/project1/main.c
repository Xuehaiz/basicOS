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
	
	
	int i;

	char *line;  // stores each line's value
	char *line_cp = NULL;
	size_t len = 0; 
	char *token = NULL;
	char *token_cp = NULL;
	char *cmd_in_line = NULL;
	char *command = NULL;
	char *arg1 = NULL;
	char *arg2 = NULL;

	/* read file */
	if (argv[1] != NULL) {
		fp = fopen(argv[1], "r");
	}
	else {
		fp = stdin;
	}

	/*main run loop*/

	while (1) {
		printf(">>> ");
		getline(&line, &len, fp);
		line[strlen(line) - 1] = '\0';
		// handle if line starts with a ;
		if (line[0] == ';') {
			printf("Error! Unrecognized command: %s\n", command);
			line++;
		}
		printf("%s\n", line);

		if (strcmp(line, "exit") == 0) break;
		else {
			i = 0;
			line_cp = line;
			
			// tokenize by ;
			token = strtok_r(line_cp, ";", &line_cp);
			token_cp = token;
			
			// for each sun-token tokenize by space
			cmd_in_line = strtok_r(token_cp, " ", &token_cp);

			// since the first word is the command
			command = cmd_in_line;

			while (token != NULL) {
				
				if (strcmp(command, "ls") == 0) {
					printf("here?\n");
					if (strlen(token_cp) > 0) {
						arg1 = strtok_r(token_cp, " ", &token_cp);
						// check repeat command
						if (strcmp(arg1, "ls") == 0) {
							printf("Error! Incorrect syntax, No control code found\n");
						}
						// check unvalid argument
						else {
							if (arg1 != NULL)
							printf("Error! Unsupported parameters for command: %s\n", command);
						}
					} 
					listDir();
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
					cmd_in_line = strtok_r(token_cp, " ", &token_cp);
					arg1 = cmd_in_line;
					makeDir(arg1); 
				}
				else if (strcmp(command, "cd") == 0) {
					cmd_in_line = strtok_r(token_cp, " ", &token_cp);
					arg1 = cmd_in_line;
					changeDir(arg1);
				}
				else if (strcmp(command, "cp") == 0) {
					cmd_in_line = strtok_r(token_cp, " ", &token_cp);
					arg1 = cmd_in_line;
					cmd_in_line = strtok_r(token_cp, " ", &token_cp);
					arg2 = cmd_in_line;
					copyFile(arg1, arg2);
				}
				else if (strcmp(command, "mv") == 0) {
					cmd_in_line = strtok_r(token_cp, " ", &token_cp);
					arg1 = cmd_in_line;
					cmd_in_line = strtok_r(token_cp, " ", &token_cp);
					arg2 = cmd_in_line;
					moveFile(arg1, arg2);
				}
				else if (strcmp(command, "rm") == 0) {
					cmd_in_line = strtok_r(token_cp, " ", &token_cp);
					arg1 = cmd_in_line;
					deleteFile(arg1);
				}
				else if (strcmp(command, "cat") == 0) {
					cmd_in_line = strtok_r(token_cp, " ", &token_cp);
					arg1 = cmd_in_line;
					displayFile(arg1);
				}
				else {
					printf("Error! Unrecognized command: %s\n", command);
				}
				if (line_cp != NULL) {
					token = strtok_r(line_cp, ";", &line_cp);
				}
				token_cp = token;
				cmd_in_line = strtok_r(token_cp, " ", &token_cp);
				if (cmd_in_line != NULL) {
					command = cmd_in_line;
				}
			}
		}
	}
	return 0;
}