/*
* Description: take input from the console and
parsing that into understandable tokens that can be used by the system.
*
* Author: Missy Shi
*
* Date: Apr 6, 2020
*
* Notes:
* N/A
*/


/*-------------------------Preprocessor Directives---------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "command.h"
/*---------------------------------------------------------------------------*/


void error_handling(char** cmd) {
    if (strcmp(cmd[0], cmd[1]) == 0) {
        fprintf(stderr, "Error! Incorrect syntax. No control code found.\n");
    }
    else {
        fprintf(stderr, "Error! Unsupported parameters for command: %s\n", cmd[0]);
    }
}


int main(int argc, char const *argv[]) {
    char *exitcmd = "exit\n", *rest, *token, *rem;
    char **cmd;
    char *buffer;
    size_t bufsize = 256;
    int counter;

    //malloc spaces
    buffer = (char *)malloc(bufsize * sizeof(char));
  	if (buffer == NULL) {
    		fprintf(stderr, "Unable to allocate buffer\n");
    		exit(EXIT_FAILURE);
  	}
    cmd = (char **)malloc(bufsize * sizeof(char *));
    if (cmd == NULL) {
    		fprintf(stderr, "Unable to allocate space for commands\n");
    		exit(EXIT_FAILURE);
  	}

    if (argc == 1) {
        while (1) {
            printf(">>> ");
            counter = 0;
            getline(&buffer, &bufsize, stdin);
            /* If the user entered <exit> then exit the loop */
            if (strcmp(buffer, exitcmd) == 0) {
                break;
            }

            else {
                /* Tokenize the input string */
                rest = buffer;
                // while ((token = strtok_r(rest, " \n", &rest))) {
                //     printf("T%d: %s\n", counter, token);
                //     counter++;
                // }

                // printf("The first matched space in buffer is: %d\n", loc);
                while((token = strtok_r(rest, ";\n", &rest))) {
                    counter = 0;
                    /* Save each tokenized cmd and params into cmd[i] */
                    rem = token;
                    char *param;
                    while ((param = strtok_r(rem, " ", &rem))) {
                        cmd[counter] = param;
                        counter++;
                    }
                    if (strcmp(cmd[0], "lfcat") == 0) {
                        // no param
                        if (counter != 1) {
                            error_handling(cmd);
                            break;
                        }
                        // printf("HI THIS IS LFCAT\n");
                        lfcat();
                    }

                    else {
                        fprintf(stderr, "Error! Unrecognized command: %s\n", cmd[0]);
                        break;
                    }
                }
            }
        }
    }

    /* File mode */

    else if ((argc == 3) && (strcmp(argv[1], "-f") == 0)) {
        printf("redirect file <%s> to file <myoutput.txt>\n", argv[2]);
        FILE* fp = NULL, *fw = NULL;
        ssize_t nread;

        fp = fopen(argv[2], "r");
        if (fp == NULL) {
            fprintf(stderr, "Unable to open file: %s\n", argv[2]);
            exit(EXIT_FAILURE);
        }
        fw = freopen("myoutput.txt", "w", stdout);
        if (fw == NULL) {
            fprintf(stderr, "Unable to open output file: myoutput.txt\n");
            exit(EXIT_FAILURE);
        }
        int len;
        write(1, "File: ", 6);
        len = strlen(argv[2]);
        write(1, argv[2], len);
        while ((nread = getline(&buffer, &bufsize, fp)) != -1) {
            len = strlen(buffer);
            write(1, buffer, len);
        }
        write(1, "\n", 2);

        fclose(fp); 
        fclose(fw);

    }

    else {
        fprintf(stderr, "Interactive mode usage: %s\n", argv[0]);
        fprintf(stderr, "File mode usage: %s -f <filename>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    /*Free the allocated memory*/
    free(buffer);
    free(cmd);
    return 0;
}