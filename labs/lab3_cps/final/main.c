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
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>
#include "command.h"
/*---------------------------------------------------------------------------*/

/*
rm mv  no output
<cmd> <param1> <param2> ; <cmd2>
*/
const char *cmdlist[] = {"ls", "pwd", "mkdir", "cd", "cp", "mv", "rm", "cat"};
const int totalcmd = 8;

void error_handling(char** cmd) {
    if (strcmp(cmd[0], cmd[1]) == 0) {
        fprintf(stderr, "Error! Incorrect syntax. No control code found.\n");
    }
    else {
        fprintf(stderr, "Error! Unsupported parameters for command: %s\n", cmd[0]);
    }
}


int main(int argc, char const *argv[]) {
    char *exitcmd = "exit\n", *buffer, *rest, *token, *rem;
    char **cmd;
    size_t bufsize = 256;
    ssize_t nread;
    FILE *fin = NULL, *fout = NULL;
    // char params[BUFSIZ];
    int counter;

    //malloc spaces
    buffer = (char *)malloc(bufsize * sizeof(char));
  	if (buffer == NULL) {
    		fprintf(stderr, "Unable to allocate buffer\n");
    		exit(1);
  	}
    cmd = (char **)malloc(bufsize * sizeof(char *));
    if (cmd == NULL) {
    		fprintf(stderr, "Unable to allocate space for commands\n");
    		exit(1);
  	}
    // for (int i = 0; i < BUFSIZ; i++) {
    //     cmd[i] = (char *)malloc(bufsize * sizeof(char));
    //     if (cmd[i] == NULL) {
    //     		fprintf(stderr, "Unable to allocate for cmd[i]\n");
    //     		exit(1);
    //   	}
    // }

    /* Interactive mode */
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


            		while((token = strtok_r(rest, ";\n", &rest))) {
                    // int loc1 = strcspn(rest, ";");
                    // int loc2 = strcspn(rest, " ");
                    // if (loc1 == 0 || loc2 == 0) {
                    //     // printf("should be error\n");
                    //     fprintf(stderr, "Unrecognized command: %s\n", " ");
                    //     break;
                    // }
                    counter = 0;
                    /* Save each tokenized cmd and params into cmd[i] */
                    rem = token;

                    char *param;
                    while ((param = strtok_r(rem, " ", &rem))) {
                        cmd[counter] = param;
                        printf("counter: %d, param: %s\n", counter, cmd[counter]);
                        counter++;
                    }
                    if (strcmp(cmd[0], "ls") == 0) {
                        // no param
                        if (counter != 1) {
                            error_handling(cmd);
                            break;
                        }
                        // printf(". .. Project-11.c main.c pseudo-shell\n");
                        listDir();
                    }

                    else if (strcmp(cmd[0], "pwd") == 0) {
                        // no param
                        if (counter != 1) {
                            error_handling(cmd);
                            break;
                        }
                        printf("Current directory is: foo/\n");
                        // showCurrentDir();
                    }

                    else if (strcmp(cmd[0], "mkdir") == 0) {
                        // one param
                        if (counter > 2) {
                            error_handling(cmd);
                            break;
                        }
                        if (counter < 2) {
                            fprintf(stderr, "Usage: %s <name>\n", cmd[0]);
                            break;
                        }
                        printf("mkdir dirName: %s\n", cmd[1]);
                        // makeDir(dirName);
                    }

                    else if (strcmp(cmd[0], "cd") == 0) {
                        if (counter > 2) {
                            error_handling(cmd);
                            break;
                        }
                        if (counter < 2) {
                            fprintf(stderr, "Usage: %s directory\n", cmd[0]);
                            break;
                        }
                        printf("Change directory to destination: %s\n", cmd[1]);
                        // changeDir(dirName);
                    }

                    else if (strcmp(cmd[0], "cp") == 0) {
                        if (counter > 3) {
                            error_handling(cmd);
                            break;
                        }
                        if (counter < 3) {
                            fprintf(stderr, "Usage: %s <src> <dst>\n", cmd[0]);
                            break;
                        }
                        printf("Copy one file from %s to the location %s\n", cmd[1], cmd[2]);
                        // copyFile(sourcePath,destinationPath);
                    }

                    else if (strcmp(cmd[0], "mv") == 0) {
                        if (counter > 3) {
                            error_handling(cmd);
                            break;
                        }
                        if (counter < 3) {
                            fprintf(stderr, "Usage: %s <src> <dst>\n", cmd[0]);
                            break;
                        }
                        printf("Move a file from %s to %s\n", cmd[1], cmd[2]);
                        // moveFile(sourcePath,destinationPath);
                    }

                    else if (strcmp(cmd[0], "rm") == 0) {
                        if (counter > 2) {
                            error_handling(cmd);
                            break;
                        }
                        if (counter < 2) {
                            fprintf(stderr, "Usage: %s <filename>\n", cmd[0]);
                            break;
                        }
                        printf("Remove file: %s from the current directory\n", cmd[1]);
                        // deleteFile(filename);
                    }

                    else if (strcmp(cmd[0], "cat") == 0) {
                        if (counter > 2) {
                            error_handling(cmd);
                            break;
                        }
                        if (counter < 2) {
                            fprintf(stderr, "Usage: %s <filename>\n", cmd[0]);
                            break;
                        }
                        printf("Display the content of file: %s\n", cmd[1]);

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
          fin = fopen(argv[2], "r");
          if (fin == NULL) {
              fprintf(stderr, "Error! File <%s> does not exit.\n", argv[2]);
              exit(EXIT_FAILURE);
          }
          fout = fopen("output.txt", "w");  // write results to output file
          if (fout == NULL) {
              fprintf(stderr, "Error! Could not open file <output.txt>\n");
              exit(EXIT_FAILURE);
          }
          while ((nread = getline(&buffer, &bufsize, fin)) != -1) {
              rest = buffer;
              counter = 0;
              while ((token = strtok_r(rest, ";\n", &rest))) {
                  fprintf(fout, "command%d: %s\n", counter, token);
                  counter++;
              }
          }
    }


    /* Explaing usage */
    else {
        fprintf(stderr, "Interactive mode usage: %s\n", argv[0]);
        fprintf(stderr, "File mode usage: %s -f <filename>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    /*Free the allocated memory*/
    free(buffer);
    free(cmd);
    /*Close the files been opened*/
    fclose(fin);
    fclose(fout);
    return 0;
}
