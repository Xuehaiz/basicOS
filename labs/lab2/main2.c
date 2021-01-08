/*
* Description: read a file from argv. Then splits words or characters by the space delimiter
*
* Author: Xuehai Zhou
*
* Date: Apr 9, 2020
*
* Notes: 
*/

/*-------------------------Preprocessor Directives---------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*---------------------------------------------------------------------------*/

/*-----------------------------Program Main----------------------------------*/
int main(int argc, char const *argv[]) {
	setbuf(stdout, NULL);
	/* Main Function Variables */
	FILE *fp = fopen(argv[1], "r");
	char *text = NULL;
	char *text_cp = NULL;
	size_t len = 0; 
	char *token = "NULL";
	char exit[5] = "exit";
	
	/* Allocate memory for the input buffer. */
	char* input = (char*)malloc(sizeof(char)*BUFSIZ);
	
	/*main run loop*/
	while (token != NULL) { 
		/* Print >>> then get the input string */
		//printf(">>> ");
		getline(&text, &len, fp);
		//text[strlen(text) - 1] = '\0';
		//if (strlen(text) > 0) printf("\n");
		/* If the user entered <exit> then exit the loop */
		if (strcmp(text, exit) == 0) break;
		else {
			int i = 0;
			text_cp = text;
			/* Tokenize the input string */
			token = strtok_r(text_cp, " ", &text_cp);
			/* Display each token */
			while (token != NULL) {
				printf("T%d: %s\n", i, token);
				i++;
				token = strtok_r(NULL, " ", &text_cp);
			}
		}
	}
	/*Free the allocated memory*/
	free(input);
	free(text);

	return 1;
}
/*-----------------------------Program End-----------------------------------*/
