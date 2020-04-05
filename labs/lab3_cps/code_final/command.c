#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
// #include <limits.h>
#include <sys/types.h>
#include "command.h"

void lfcat()
{
  	// Define your variables here
    char* buf = NULL;
    char* cwd = NULL;
    size_t bufsize = 2056;
    ssize_t nread;
    size_t count;
    FILE *fin = NULL;
    char* lines;

    DIR *dir;
    struct dirent *sd;

    lines = (char *)malloc(bufsize * sizeof(char));
    buf = (char *)malloc(bufsize * sizeof(char));
    // cwd = (char *)malloc(bufsize * sizeof(char));
  	// Get the current directory
    cwd = getcwd(buf, bufsize);
    // printf("cwd: %s\n", cwd);
    if (cwd == NULL) {
        fprintf(stderr, "Error! Unable to get current directory.\n");
        exit(EXIT_FAILURE);
    }
  	// Open the dir using opendir()
    dir = opendir(cwd);
    if (dir == NULL) {
        fprintf(stderr, "Error! Unable to open directory.\n");
        exit(EXIT_FAILURE);
    }
  	// use a while loop to read the dir
    while ((sd = readdir(dir)) != NULL) {

        if (sd->d_type == DT_REG
            && strstr(sd->d_name, ".c") == NULL
            && strstr(sd->d_name, ".o") == NULL
            && strstr(sd->d_name, ".h") == NULL
            && strstr(sd->d_name, ".pdf") == NULL
            && strstr(sd->d_name, ".exe") == NULL
            && (strcmp(sd->d_name, "output.txt"))
            && (strcmp(sd->d_name, "Makefile"))
            && (strcmp(sd->d_name, ".DS_Store"))) {
              count = strlen(sd->d_name);
              write(1, "File: ", 6);
              write(1, sd->d_name, count);
              write(1, "\n", 2);
              fin = fopen(sd->d_name, "r");
              // if (fin == NULL) {
              //     fprintf(stderr, "Error! File <%s> does not exit.\n", sd->d_name);
              //     exit(EXIT_FAILURE);
              // }
              while ((nread = getline(&lines, &bufsize, fin)) != -1) {
              // while ((getline(&lines, &bufsize, fin))) {
                  count = strlen(lines);
                  write(1, lines, count);
              }
              fclose(fin);
              fin = NULL;
              write(1, "\n", 2);
              for (int i = 0; i < 80; i++) {
                  write(1, "-", 2);
              }
              write(1, "\n", 2);

        }
    }

		// Hint: use an if statement to skip any names that are not readable files (e.g. ".", "..", "main.c", "a.out", "output.txt"

		// Open the file

		// Read in each line using getline()
		// Write the line to stdout

		// write 80 "-" characters to stdout

		// close the read file and free/null assign your line buffer
    // fclose(fin);
    free(lines);
    free(buf);
    // free(cwd);
  	//close the directory you were reading from using closedir()
    closedir(dir);
}
