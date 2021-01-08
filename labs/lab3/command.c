#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "command.h"


void lfcat()
{
	// Define your variables here
	FILE *fp;
	DIR *dir;
	//int fnamelen;
	size_t len = 1024;
	char *currDir;
	static char *buffer;
	struct dirent *sd;

	// memory allcation
	char *line = (char *)malloc(len * sizeof(char));
	if (line == NULL) {
		write(1, "line allocation failure\n", strlen("line allocation failure\n"));
		exit(EXIT_FAILURE);
	}

	// Get the current directory
	currDir = getcwd(buffer, len);
	//printf("current working directory: %s\n", currDir);
	
	// Open the dir using opendir()
	dir = opendir(currDir);
	if (dir == NULL) {
        write(1, "Error! Unable to open current directory.\n", strlen("Error! Unable to open current directory.\n"));
        exit(EXIT_FAILURE);
    }

	// use a while loop to read the dir
	while ((sd=readdir(dir)) != NULL) {
		// Hint: use an if statement to skip any names that are not readable files (e.g. ".", "..", "main.c", "a.out", "output.txt"
		if (sd->d_type == DT_REG
			&& strstr(sd->d_name, ".c") == NULL
            && strstr(sd->d_name, ".o") == NULL
            && strstr(sd->d_name, ".h") == NULL
            && strstr(sd->d_name, ".pdf") == NULL
            && strstr(sd->d_name, ".exe") == NULL
            && strcmp(sd->d_name, "output.txt")
            && strcmp(sd->d_name, "input.txt")
            && strcmp(sd->d_name, "Makefile")
            && strcmp(sd->d_name, ".DS_Store"))  {
			// Open the file
			//fnamelen = strlen(sd->d_name);
	        write(1, "File: ", 6);
	        write(1, sd->d_name, strlen(sd->d_name));
	        write(1, "\n", 1);
			fp = fopen(sd->d_name, "r");
			// Read in each line using getline()
			while (getline(&line, &len, fp) != -1) {
				// Write the line to stdout
				write(1, line, strlen(line));
			}
			write(1, "\n", 1);
			// write 80 "-" characters to stdout
			for (int i = 0; i < 80; ++i) {
				write(1, "-", 1);
			}
			write(1, "\n", 1);
			fclose(fp);
		}
	}
	// free memory 
	free(line);
	free(currDir);
	
	//close the directory you were reading from using closedir()
	closedir(dir);
}
