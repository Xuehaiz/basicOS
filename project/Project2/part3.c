#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>


void sig_handler(int sig) {  
    printf("Child process: %d - received signal: %d\n", getpid(), sig);
}

int main(int argc __attribute__((unused)), char const *argv[])
{
    // Variable declarations
    char *token;
    int len = 256; 
    char line[len];
    int counter = 0;
    int i = 0;
    int numprograms = 0;
    // open file
    FILE *fp = fopen(argv[1], "r");
    if (fp == NULL) {
        fprintf(stderr, "File <%s> open failure.\n", argv[1]);
        exit(EXIT_FAILURE);
    }
    // memory allcation
    char **arg_arr = (char **)malloc(len * sizeof(char *));
    if (arg_arr == NULL) {
        fprintf(stderr, "Buffer allocation failure\n");
        fclose(fp);
        exit(EXIT_FAILURE);
    }
    pid_t *pid = (pid_t *)malloc(len * sizeof(pid_t));
    if (pid == NULL) {
        fprintf(stderr, "pid list allocation failure\n");
        fclose(fp);
        free(arg_arr);
        exit(EXIT_FAILURE);
    }

    // sig struct and handler
    // int status;
    int signal;
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGALRM);
    sigaddset(&sigset, SIGUSR1);
    sigaddset(&sigset, SIGCONT);
    sigaddset(&sigset, SIGSTOP);
    sigprocmask(SIG_BLOCK, &sigset, NULL);
    struct sigaction sa;

    if (memset (&sa, '\0', sizeof(sa)) == NULL) {
        perror("memset");
    }
    sa.sa_handler = &sig_handler;
    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("SIGUSR1");
    }
    if (sigaction(SIGALRM, &sa, NULL) == -1) {
        perror("SIGALRM");
    }
    if (sigaction(SIGCONT, &sa, NULL) == -1) {
        perror("SIGCONT");
    }

    while (fgets(line, len, fp) != NULL) {
        numprograms++;
        counter = 0;
        token = strtok(line, " \n");
        while (token != NULL) {
            arg_arr[counter] = token;
            token = strtok(NULL, " \n");
            counter++;
        }
        arg_arr[counter] = NULL;
        printf("arg_arr: %s %s \n", arg_arr[0], arg_arr[1]);

        pid[i] = fork();
        if (pid[i] < 0) {
            perror("fork error, no child created");
            fclose(fp);
            free(pid);
            free(arg_arr);
            exit(EXIT_FAILURE);
        }
        if (pid[i] == 0) {
            printf("Child process: %d - Starting executing %s.\n", getpid(), arg_arr[0]);
            // raise SIGSTOP
            printf("Child process: %d - waiting SIGUSR1\n", getpid());
            sigwait(&sigset, &signal);
            // Exec call
            printf("Child process: %d - calling exec().\n", getpid());
            execvp(arg_arr[0], arg_arr);
            // error report and free 
            perror("execvp");
            fclose(fp);
            free(pid);
            free(arg_arr);

            // End the child process
            _exit(-1);
        }
        i++;
    }
    
    int condition = 1;
    int alive = 0;
    int status;
    pid_t wpid;

    for (int i = 0; i < numprograms; i++) {
        printf("Sending signal: %d to child process: %d\n", SIGUSR1, pid[i]);
        kill(pid[i], SIGUSR1);
        printf("Signal SIGUSR1 sent.\n");
    }

    for (int i = 0; i < numprograms; i++) {
        printf("Sending signal: %d to pid: %d\n", SIGSTOP, pid[i]);
        kill(pid[i], SIGSTOP);
        printf("Signal SIGSTOP sent.\n");
    }

    while (condition) {
        condition = 0;
        for (int k = 0; k < numprograms; k++) {
            wpid = waitpid(pid[k], &status, WNOHANG);
            if (wpid != -1) {  // determine if the process is alive
                kill(pid[k], SIGCONT);
                alarm(1);
                printf("Parent PID %d resumed suspended child PID %d\n", getpid(), pid[k]);
                if (sigwait(&sigset, &signal) == 0) {
                    printf("Child process: %d - Received signal: SIGALRM\n", pid[k]);
                }
                if (WIFEXITED(status)) {
                    kill(pid[k], SIGSTOP);
                    printf("Parent PID %d suspended unfinished child PID %d\n", getpid(), pid[k]);
                    condition = 1; 
                }
            }
            // count the number of the alive processes
            alive = 0;
            for (int i = 0; i < numprograms; i++) {
                wpid = waitpid(pid[i], &status, WNOHANG);
                if (wpid != -1) {
                    alive++;
                }
            }
            if (alive <= 1) {
                for (int i = 0; i < numprograms; i++) {
                    wpid = waitpid(pid[i], &status, WNOHANG);
                    if (wpid != -1) {
                        kill(pid[i], SIGCONT);
                        printf("Finishing up the last process: %d\n", pid[i]);
                    }
                }
                condition = 0; 
                break;
            }
        }
    }
    
    for (int i = 0; i < numprograms; i++) {
        waitpid(pid[i], NULL, 0);
        // wait(0);
        printf("wait pid[%d]: %d\n", i, pid[i]);
    }

    // exit all

    printf("All processes finished: parent exiting: my pid is %d \n\n", getpid());
    fclose(fp);
    free(pid);
    free(arg_arr);
    return 0;
}