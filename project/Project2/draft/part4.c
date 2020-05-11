/*
* Description: MCP v3.0
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
*    Each buf in the file contains the name of the program(command)
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
* (1) pid  %d
*       The process ID.
* 
* (2) comm  %s
*       The filename of the executable, in parentheses.
*       This is visible whether or not the executable is swapped out.

* (3) state  %c
*       One of the following characters, indicating process state:
*           R  Running
*          S  Sleeping in an interruptible wait
*           D  Waiting in uninterruptible disk sleep
*           Z  Zombie
*           T  Stopped (on a signal) or (before Linux 2.6.33) trace stopped
*           t  Tracing stop (Linux 2.6.33 onward)
*           W  Paging (only before Linux 2.6.0)
*           X  Dead (from Linux 2.6.0 onward)
*           x  Dead (Linux 2.6.33 to 3.13 only)
*
* (4) ppid  %d
*       The PID of the parent of this process.
*
* (5) pgrp  %d
*       The process group ID of the process.
*
* (7) tty_nr  %d
*       The controlling terminal of the process.  (The minor
*       device number is contained in the combination of
*       bits 31 to 20 and 7 to 0; the major device number is
*       in bits 15 to 8.)
* 
*/

#define _GNU_SOURCE
#define _POSIX_C_SOURCE

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <dirent.h>
/*---------------------------------------------------------------------------*/

/*--------------------------Function Declaration------------------------------*/
void signal_handler(int sig);
int mcp(const char* inputfile);
/*---------------------------------------------------------------------------*/

void copycat(pid_t pid)
{
    char path[20];
    char *token;
    FILE *statfile;
    char *buf = NULL;
    size_t bufsize = sizeof(char) * 256;
    char delim[2] = " ";
    int i;

    // add pid into the path name
    sprintf(path, "/proc/%d/stat", pid);
    statfile = fopen(path, "r");
    if (!statfile) { perror("fopen(path)"); exit(EXIT_FAILURE); }

    getline(&buf, &bufsize, statfile);
    
    char *comm, state;
    int ppid;
    // int pgrp;
    unsigned long utime, stime;
    long priority;
    unsigned long long starttime;
    unsigned long vsize;

    // Begin tokenizing stat 'file'. token points to pid...
    token = strtok(buf, delim);

    // (2) comm - filename of the executable
    comm = strtok(NULL, delim);
    // (3) state - process state
    sscanf(strtok(NULL, delim), "%c", &state);
    // (4) ppid - PID of the parent of this process
    sscanf(strtok(NULL, delim), "%d", &ppid);
    // (5) pgrp - process group ID of the process
    // sscanf(strtok(NULL, delim), "%d", &pgrp);
    // SKIP (5) ~ (13)
    for (i = 0; i < (13 - 4); i++) {
        strtok(NULL, delim);
    }
    // (14) utime - Amount of time that this process has been scheduled
    // in user mode, measured in clock ticks
    sscanf(strtok(NULL, delim), "%lu", &utime);
    // (15) stime - Amount of time that this process has been scheduled
    // in kernel mode, measured in clock ticks
    sscanf(strtok(NULL, delim), "%lu", &stime);
    // SKIP (16) ~ (17)
    for (i = 0; i < (17 - 15); i++) { strtok(NULL, delim); }
    // (18) priority - For processes running a real-time scheduling policy
    sscanf(strtok(NULL, delim), "%ld", &priority);
    // SKIP (19) ~ (21)
    for (i = 0; i < (21 - 18); i++) { strtok(NULL, delim); }
    // (22) starttime - The time the process started after system boot
    sscanf(strtok(NULL, delim), "%llu", &starttime);
    // (23) vsize - Virtual memory size in bytes.
    sscanf(strtok(NULL, delim), "%lu", &vsize);

    printf("%d\t%s\t%c\t%d\t%lu\t\t%lu\t\t%llu\t\t%lu\t\t%ld\t\n", 
        pid, comm, state, ppid, (utime / sysconf(_SC_CLK_TCK)),
        (stime / sysconf(_SC_CLK_TCK)), (starttime / sysconf(_SC_CLK_TCK)),
        vsize >> 10, priority);

    token = NULL;
    free(buf);
    fclose(statfile);
}

/* Handler to handle for signals except SIGSTOP */
void signal_handler(int sig) {  
    printf("Child process: %d - received signal: %d\n", getpid(), sig);
}

int mcp(const char* inputfile)
{
	char *token;
    size_t bufsize = 256; 
    char buf[bufsize];
    int counter = 0, i = 0, j = 0, nprogs = 0;

    int catched = 1, alive = 0, signal, status;

    /* Open the input files */
    FILE *fin = fopen(inputfile, "r");
    if (fin == NULL) {
        fprintf(stderr, "File <%s> open failure.\n", inputfile);
        exit(EXIT_FAILURE);
    }
    char **args = (char **)malloc(bufsize * sizeof(char *));
    if (args == NULL) {
        fprintf(stderr, "Buffer allocation failure\n");
        fclose(fin);
        exit(EXIT_FAILURE);
    }
    pid_t *pid = (pid_t *)malloc(bufsize * sizeof(pid_t));
    if (pid == NULL) {
        fprintf(stderr, "pid list allocation failure\n");
        fclose(fin);
        free(args);
        exit(EXIT_FAILURE);
    }

    /* PART 3: create sig struct and handler */
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGALRM);
    sigprocmask(SIG_BLOCK, &sigset, NULL);
    struct sigaction sa;
    if (memset (&sa, '\0', sizeof(sa)) == NULL) {
        perror("memset");
    }
    sa.sa_handler = &signal_handler;
    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("SIGUSR1");
    }
    if (sigaction(SIGCONT, &sa, NULL) == -1) {
        perror("SIGCONT");
    }

    while (fgets(buf, bufsize, fin) != NULL) {
        nprogs++;
        counter = 0;
        token = strtok(buf, " \n");
        while (token != NULL) {
            args[counter] = token;
            token = strtok(NULL, " \n");
            counter++;
        }
        args[counter] = NULL;

        pid[i] = fork();
        if (pid[i] < 0) {
            perror("fork error, no child created");
            fclose(fin);
            free(pid);
            free(args);
            exit(EXIT_FAILURE);
        }
        if (pid[i] == 0) {
            printf("Child process: <%d> - Starting executing program: <%s>\n", getpid(), args[0]);
            printf("Child process: <%d> - Received <SIGSTOP>\n", getpid());
            raise(SIGSTOP);
            /* PART 2 Child process wait for a SIGUSR1 signal before calling exec() */
            printf("Child process: <%d> - Received signal: SIGUSR1 - Calling exec()\n", getpid());
            execvp(args[0], args);
            perror("execvp");
            fclose(fin);
            free(pid);
            free(args);

            _exit(-1);
        }
        i++;
    }

    while (catched) {
        catched = 0;
        for (int k = 0; k < nprogs; k++) {
            if (waitpid(pid[k], &status, WNOHANG) != -1) {  // determine if the process is alive
                kill(pid[k], SIGCONT);
                alarm(1);
                printf("Parent process <%d> resumed suspended child process %d\n", getpid(), pid[k]);
                if (sigwait(&sigset, &signal) == 0) {
                    printf("Child process: %d - Received signal: SIGALRM\n", pid[k]);
                }
                if (waitpid(pid[k], &status, WNOHANG) != -1) {
                    kill(pid[k], SIGSTOP);
                    printf("Parent process <%d> - Suspended unfinished Child process <%d>\n", getpid(), pid[k]);
                    catched = 1; 
                }
            }

            /* Checking how many child process still alive */
            alive = 0;
            for (int i = 0; i < nprogs; i++) {
                if (waitpid(pid[i], &status, WNOHANG) != -1) {
                    alive++;
                }
            }
            if (alive <= 1) {
                // run the last process 
                for (int i = 0; i < nprogs; i++) {
                    if (waitpid(pid[i], &status, WNOHANG) == 0) {
                    	printf("Parent process - Waiting for the last process: <%d>\n", pid[i]);
                        kill(pid[i], SIGCONT);
                        
                    }
                }
                break;
            }
            if (j % 2 == 0) {
                // system("clear");
                printf("=======================================================================================================\n");
                printf("PID\tCommand\t\tState\tPPID\tUserTime (s)\tKernelTime (s)\tStartTime (s)\tVMSize (KB)\tPriority\n");
                copycat(getpid());
                for (int i = 0; i < nprogs; i++) {
                    if (waitpid(pid[i], &status, WNOHANG) != -1) {
                        copycat(pid[i]);
                    }
                }
                printf("\n=======================================================================================================\n");
            }
            j++;
        } 
    }
    // WIFEXITED

    
    for (int i = 0; i < nprogs; i++) {
        waitpid(pid[i], NULL, 0);
        printf("Parent process - Waiting for the Child process: <%d> to finish\n",pid[i]);
    }


    printf("--- All the child processes are finished --- Parent process: <%d> is exiting.\n", getpid());
    fclose(fin);
    free(pid);
    free(args);
    return 0;
}

int main(int argc, char const *argv[])
{
	if (argc != 2) {
		fprintf(stderr, "Usage: %s <filename>.\n", argv[0]);
		exit(EXIT_FAILURE);
	}    

	if (mcp(argv[1]) != 0) {
		return EXIT_FAILURE;
	}

	return 0;
}