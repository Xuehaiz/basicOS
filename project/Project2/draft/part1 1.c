/*
* Description: MCP v1.0
*
* Author: Missy Shi
*
* Course: CIS 415
*
* Date: 04/21/2020
*
* Requirements:
* 1. Read the program workload from the specified input file 
*    (This should work just like the file mode from Project 1).
*    Each line in the file contains the name of the program(command)
*    and its arguments.
* 2. For each program, your MCP must launch the program to run as 
*    a separate process using some variant of the following system calls:
*       a. fork(2)​: ​http://man7.org/linux/man-pages/man2/fork.2.html
*       b. One of the exec() system calls (see exec)​:
*           i. http://man7.org/linux/man-pages/man3/exec.3.html
* 3. Once all of the programs are running, your MCP must wait for 
*    each program to terminate using the system call wait().
*       a. wait(2)​: ​http://man7.org/linux/man-pages/man2/waitid.2.html
* 4. After all programs have terminated, your MCP must exit 
*    using the exit() system call. 
*       a. exit()​: ​http://man7.org/linux/man-pages/man2/exit.2.html
* 
* Notes:
* 1. 
*/

/*-------------------------Preprocessor Directives---------------------------*/
#define _GNU_SOURCE

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
// #include <sys/stat.h>

/*---------------------------------------------------------------------------*/

/*--------------------------Function Declaration------------------------------*/


/*---------------------------------------------------------------------------*/




int mcp(const char* inputfile) 
{
    FILE* fin;
    size_t bufsize = 256 * sizeof(char);
    char* lines = NULL;     // lines read from input file
    char* program = NULL;   // name of programs
    char** args = NULL;      // arguments of the program
    int numprograms = 0;    // counter for number of programs
    int numargs;         // counter for number of arguments
    char *saveptr = NULL;
    char *pt = NULL;
    pid_t *pid;             // array of pid
    int npid = 0;           // number of pid
    char *token = NULL;

    /* Malloc spaces */
    token = (char *)malloc(bufsize * sizeof(char));
    lines = (char *)malloc(bufsize * sizeof(char));
    pt = (char*)malloc(bufsize * sizeof(char));
    saveptr = (char*)malloc(bufsize * sizeof(char));
    program = (char *)malloc(bufsize * sizeof(char));
    args = (char **)malloc(bufsize * sizeof(char*));
    if (lines == NULL || pt == NULL || saveptr == NULL 
        || program == NULL || args == NULL || token == NULL) {
        fprintf(stderr, "Unable to allocate space\n");
        exit(EXIT_FAILURE);
    }

    fin = fopen(inputfile, "r");
    if (fin == NULL) {
        fprintf(stderr, "Unable to open file: %s\n", inputfile);
        exit(EXIT_FAILURE);
    }
    
    pid = (int *)malloc(sizeof(int));
    if (pid == NULL) {
        fprintf(stderr, "Unable to allocate space for pid\n");
        exit(EXIT_FAILURE);
    }
    /* Read the program workload from the specified input file */
    while (getline(&lines, &bufsize, fin) != EOF) {
        numargs = 0;
        strtok_r(lines, "\n", &saveptr);
        // strcpy(pt, lines);
        saveptr = lines;

        // pt = strtok_r(pt, " ", &saveptr);
        // strcpy(program, pt);
        // strcpy(args, saveptr);
        while (token = (strtok_r(saveptr, " ", &saveptr))) {
            args[numargs] = token;
            printf("CURRENT ARGUMENT: %s\n", args[numargs]);
            numargs++;
        }
        // printf("Current program: %s, arguments: %s\n", program, args);
        numprograms++;

        pid[npid] = fork();
        if (pid[npid] < 0) { 
            perror("fork"); 
            exit(EXIT_FAILURE);
        }

        if (pid[npid] == 0) {
            // printf("------Current program: %s, arguments: %s------\n", program, args);
            
            // execvp(program, args);
            if (execvp(args[0], args) < 0) { 
                perror("execvp"); 
                exit(EXIT_FAILURE);
            }
            printf("This is the child process, pid: %d, ", getpid());
            printf("my parent pid is: %d\n", getppid());
            printf("My status is %d\n", pid);
            exit(-1);
        }

        npid++;

        printf("\n");
    }

    // printf("number of programs: %d\n\n", numprograms);


    for (int i = 0; i < numprograms; i++) {
        // printf("pid[%d] is: %d\n", i, pid[i]);
        waitpid(pid[i], NULL, 0);
        // sleep(1);
    }


    /* Free the allocated memory */
    free(lines);
    free(pt);
    free(program);
    free(args);
    free(pid);
    
    /* Close the files been opened */
    fclose(fin);

    return 1;
}



int main(int argc, char const *argv[])
{
    // int mcp_result;
    if (argc != 2) {	// if no filename provided
        fprintf(stderr, "Usage: %s <filename>.\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // mcp_result = mcp(argv[1]);
    mcp(argv[1]);
    return 0;
}

/*---------------------------------------------------------------------------*/


