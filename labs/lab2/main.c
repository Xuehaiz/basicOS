#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
	FILE *fp = stdin;
	char *text = NULL;
	char *text_cp = NULL;
	size_t len = 0; 
	char *token = NULL;
	char delimiter = ' ';
	char exit[5] = "exit";
	while (1) { 
		printf(">>> ");
		getline(&text, &len, fp);
		text[strlen(text) - 1] = '\0';
		if (strlen(text) > 0) printf("\n");
		if (strcmp(text, exit) == 0) break;
		else {
			int i = 0;
			text_cp = text;
			token = strtok_r(text_cp, " ", &text_cp);
			while (token != NULL) {
				printf("T%d: %s\n", i, token);
				i++;
				token = strtok_r(NULL, " ", &text_cp);
			}
		}
	}
	free(text);
	return 0;
}