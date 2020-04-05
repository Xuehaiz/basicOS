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
	setbuf(stdout, NULL);
	// Variable declarations
	FILE *fp = stdin;
	int counter = 0;
	int condition = 1;
	char *line_cp = NULL;
	size_t len = 256; 
	char *token = NULL;
	char *token_cp = NULL;
	char *wrd;
	
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
		fp = fopen(argv[2], "r");
		if (fp == NULL) {
              fprintf(stderr, "File <%s> create failure.\n", argv[2]);
              exit(EXIT_FAILURE);
        }
        freopen("output.txt", "w", stdout);
	} else if (argc != 1) {
		fprintf(stderr, "usage: Interactive mode: ./lab3.exe\n File mode./lab3.exe -f output.txt\n");
		exit(EXIT_FAILURE);
	}

	while (condition) {
		printf(">>> ");
		getline(&line, &len, fp);
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
				if (strcmp(wrd_in_token[0], "lfcat") == 0) {
					if (counter - 1 == 0) {
						lfcat();
					} else {
						error_handler(wrd_in_token);
						break;
					}
				}
				else {
					fprintf(stderr, "Error! Unrecognized command: %s\n", wrd_in_token[0]);
					break;
				}
			}
		}
	}

	// free memory and close file 
	free(line);
	free(wrd_in_token);

	return 0;
}

/*-----------------------------Program End-----------------------------------*/
