#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>

#include "command.h"


void error_handler(char **wrd_in_token) {
	if (strcmp(wrd_in_token[0], wrd_in_token[1]) == 0) {
		fprintf(stderr, "Error! Incorrect syntax. No control code found.\n");
	}
	else {
		fprintf(stderr, "Error! Unsupported parameters for command: %s\n", wrd_in_token[0]);
	}
}


int main(int argc, char const *argv[])
{
	/* Main Function Variables */
	FILE *fp = stdin;
	int counter = 0;
	int condition = 1;
	char *line_cp = NULL;
	size_t len = 256; 
	char *token = NULL;
	char *token_cp = NULL;
	char *wrd;
	int interactive = 1;
	
	// memory allcation
	char *line = (char *)malloc(len * sizeof(char));
	if (line == NULL) {
		fprintf(stderr, "line allocation failure\n");
		exit(EXIT_FAILURE);
	}
	char **wrd_in_token = (char **)malloc(len * sizeof(char *));
	if (wrd_in_token == NULL) {
		fprintf(stderr, "Buffer allocation failure\n");
		exit(EXIT_FAILURE);
	}


	if (argc == 3 && strcmp(argv[1],"-f") == 0) {
		interactive = 0;
		fp = fopen(argv[2], "r");
		if (fp == NULL) {
              fprintf(stderr, "File <%s> create failure.\n", argv[2]);
              exit(EXIT_FAILURE);
        }
        freopen("output.txt", "w", stdout);
	} 
	else if (argc != 1) {
		fprintf(stderr, "usage: Interactive mode: ./lab3.exe\n File mode: ./lab3.exe -f output.txt\n");
		exit(EXIT_FAILURE);
	}

	while (getline(&line, &len, fp) != EOF) {
		if (interactive) {
			printf(">>> ");
		}
	
		if(!feof(fp)) {
			line[strlen(line) - 1] = '\0';
		}

		if (strcmp(line, "exit") == 0) break;
		else {
			line_cp = line;
			// split the line by ; 
			while ((token = strtok_r(line_cp, ";", &line_cp))) {
				counter = 0;
				if (strcmp(token, "") == 0) {
					fprintf(stderr, "Error! Unrecognized command: %s\n", wrd_in_token[0]);
					break;
				}
				token_cp = token;
				// split each token and save them into another buffer (wrd_in_token)
				while((wrd = strtok_r(token_cp, " ", &token_cp))) {
					if (strcmp(wrd, "") == 0) {
						fprintf(stderr, "Error! Unrecognized command: %s\n", wrd_in_token[0]);
						break;
					}
					wrd_in_token[counter] = wrd;
					counter++;
				}
				if (strcmp(wrd_in_token[0], "ls") == 0) {
					if (counter - 1 == 0) {
						listDir();
					}
					else {
						error_handler(wrd_in_token);
						break;
					}
				}
				else if (strcmp(wrd_in_token[0], "pwd") == 0) {
					if (counter - 1 == 0) {
						showCurrentDir();
					}
					else {
						error_handler(wrd_in_token);
						break;
					}
				}
				else if (strcmp(wrd_in_token[0], "mkdir") == 0) {
					if (counter - 2 == 0) {
						makeDir(wrd_in_token[1]); 
					} else if (counter - 1 == 0){
						fprintf(stderr, "Error! usage: mkdir directory ...\n");
						break;
					} else {
						error_handler(wrd_in_token);
						break;
					}
					
				}
				else if (strcmp(wrd_in_token[0], "cd") == 0) {
					if (counter - 1 == 0) {
						changeDir(NULL);
					} else if (counter - 2 == 0){
						changeDir(wrd_in_token[1]);
					} else {
						error_handler(wrd_in_token);
						break;
					}
				}
				else if (strcmp(wrd_in_token[0], "cp") == 0) {
					if (counter - 1 == 0) {
						fprintf(stderr, "Error! usage: cp source_file target_file\n");
						break;
					} else if (counter - 3 == 0) {
						copyFile(wrd_in_token[1], wrd_in_token[2]);
					} else {
						error_handler(wrd_in_token);
						break;
					}
				}
				else if (strcmp(wrd_in_token[0], "mv") == 0) {
					if (counter - 1 == 0) {
						fprintf(stderr, "Error! usage: mv source_file target_file\n");
						break;
					} else if (counter - 3 == 0) {
						moveFile(wrd_in_token[1], wrd_in_token[2]);
					} else {
						error_handler(wrd_in_token);
						break;
					}
				}
				else if (strcmp(wrd_in_token[0], "rm") == 0) {
					if (counter - 2 == 0) {
						deleteFile(wrd_in_token[1]);
					} else {
						error_handler(wrd_in_token);
						break;
					}
					
				}
				else if (strcmp(wrd_in_token[0], "cat") == 0) {
					if (counter - 2 == 0) {
						displayFile(wrd_in_token[1]);
					} 
					else {
						error_handler(wrd_in_token);
						break;
					}
				}
				else {
					printf("Error! Unrecognized command: %s\n", wrd_in_token[0]);
					break;
				}
			}
		}
	}

	
	// free memory and close file 
	free(line);
	free(wrd_in_token);
	fclose(fp);

	return 0;
}