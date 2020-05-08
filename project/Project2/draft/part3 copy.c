/*
* Description: Project2 - Part3
*              MCP Schedules Processes
*
* Author: Donna Hooshmand
*
* Date: 4/25/2020
*
* Notes:
* 1.
*/

/*-------------------------Preprocessor Directives---------------------------*/
#define _GNU_SOURCE

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
/*---------------------------------------------------------------------------*/

/*-----------------------------Program Main----------------------------------*/

void signalHandler(int sig) {
  printf("Child process: %d - recieved signal: %d", getpid(), sig);
}





int main(int argc, char* argv[]){
    //char *exit_code = "exit\n";
    char *input = NULL;
    size_t bufsize = 256 * sizeof(char);
    char *program, *token, *arguments[bufsize];
    //char *pid_list;
    pid_t pid_list[100];
    FILE* fp;
    int count = 0;
    int i = 0;
    
    pid_t current;
    pid_t ppid = getpid();
    pid_t pid;
    
    input = (char*)malloc(sizeof(char)*bufsize);
    if (input == NULL) {
        fprintf(stderr, "Unable to allocate memory \n");
        exit(EXIT_FAILURE);
    }
    
    fp = fopen(argv[1], "r");
    if(fp == NULL){
        fprintf(stderr, "Unable to open file: %s\n",argv[1]);
        exit(EXIT_FAILURE);
    }
    
    int status;
    sigset_t signalSet;
    sigemptyset(&signalSet);
    sigaddset(&signalSet, SIGALRM);
    //sigaddset()
    sigprocmask(SIG_BLOCK, &signalSet, NULL);
    // struct sigaction action;
    // memset(&action, '\0', sizeof(action));
    // action.sa_handler = &signalHandler;
    // action.sa_flags = SA_SIGINFO;

    //if(sigaction(SIGUSR1, &action, NULL) < 0){
    //    perror("sigaction error!");
    //    return 1;
    //}
    //sigaction(SIGUSR1, &action, NULL);
    
    while(getline(&input, &bufsize, fp) != EOF){
        i = 0;
        token = strtok(input, "\n ");
        if (token != NULL) {
            program = token;
            arguments[i] = token;
            i++;
            token = strtok(NULL, "\n ");
        }

        while (token!= NULL) {
            arguments[i] = token;
            token = strtok(NULL, "\n ");
            i++;
        }
        arguments[i] = NULL;
        pid_list[count] = fork();
        
        if (pid_list[count] < 0) {
            printf("Error! Process: %d - Unable to create child process.\n", getpid());
            exit(EXIT_FAILURE);
        }
        if (pid_list[count] == 0) {
            //CHILD PROCESS
            printf("Child process: %d - Starting execution. The Command is %s\n", getpid(),program);
            
            // Waiting for SIGUSR1
            printf("Child process rasing SIGSTOP\n");
            raise(SIGSTOP);
            // int signal = SIGSTOP;
            // sigwait(&signalSet, &signal);
            
            // Exec call
            printf("Child process: %d - calling exec().\n", getpid());
            
            
            if(execvp(program, arguments) == -1){
                perror("exec");
            }      
 
            exit(1);
        }
        //pid_list[count] = current;
        printf("parent: PID %d created child: PID %d\n", ppid, pid_list[count]);
        count++;
        
    }

    printf("Parent process starting execution \n");
    //run until everything is finished
    // int condition = 1;
    int r = 0;
    // int next = 1;
    // pid_t wpid;
    // int alive[count+1];
    // for(int s; s < count ; s++){
    //     alive[s] = 0;  //meaning process is alive
    // }

    //int done_childern;
    int escape = 0;
    while (escape == 0){
        int done_childern = 0;
        waitpid(-1, NULL, WNOHANG);
        escape = 1;
        for (r = 0; r < count; r++) {
            if (kill(pid_list[r], 0) != -1) {
                kill(pid_list[r], SIGCONT);
                alarm(2);
                printf("Parent (PID %d) resumed suspended child (PID %d)\n", ppid, pid_list[r]);
                int sig = SIGALRM;
                sigwait(&signalSet, &sig);
                if (kill(pid_list[r], 0) != -1) {
                    kill(pid_list[r], SIGSTOP);
                    printf("Parent (PID %d) suspended unfinished child (PID %d)\n", ppid, pid_list[r]);
                    escape = 0;
                }
            } 
        }
    }
    // if(done_childern = count -1){
    //     kill(pid_list[r], SIGCONT);
    // }
    free(input);
    fclose(fp);
    printf("All processes are finished! Parent (PID %d) exiting...\n", ppid);
    exit(EXIT_SUCCESS);

    
    // while(condition){
    //     int done_childern = 0;
    //     printf("=============================INSIDE WHILE\n");
    //     if(alive[j] == 0){
    //         printf("***********************INSIDE IF\n");
    //         wpid = waitpid(pid_list[j], &status, WNOHANG);
    //         if(!(wpid ==0 && wpid == -1)){
    //             printf("chilf process: %d has exited\n", pid_list[j]);  
    //             alive[j] = 1;    
    //             printf("alive[j] is %d\n", alive[j]);
    //         } else {
    //             printf("child process: %d being stopped\n", pid_list[j]);
    //             kill(pid_list[j], SIGSTOP);
    //             int r = 1;
    //             // wpid = waitpid(pid_list[j+r], &status, WNOHANG);
    //             // while(!(wpid ==0 && wpid == -1)){
    //             //     r++;
    //             //     wpid = waitpid(pid_list[j+r], &status, WNOHANG);
    //             // }
    //             while(alive[r+j] == 1){
    //                 r++;
    //             }
    //             printf("chilf process: %d starting again\n", pid_list[j+1]);
    //             kill(pid_list[j+r], SIGCONT);
    //         }
    //         printf("Parent waiting for 2 seconds\n");
    //         alarm(5);
    //         int sig = SIGALRM;
    //         sigwait(&signalSet, &sig);
    //         for (int y = 0; y < count ; y ++){
    //             if(alive[y] == 1){
    //                 done_childern++;
    //             }
    //         }
    //         if(done_childern == count){
    //             condition = 0;
    //         }
    //         if(done_childern == count - 1 ){
    //             printf("one child left. no need to schedule. breaking out\n");
    //             break;
    //         }
    //         printf("$$$$$$$$$$$$$$$$$$$$$$ of done children is: %d\n",done_childern );
    //     }

    //     if ( j > count - 1 ){
    //         j = 0;
    //         next = j + 1;
    //     } else {
    //         j++;
    //         next = j+1;
    //     }

    // }

    // for(int p = 0; p<count; p++){
    //     //waitpid(pid_list[j], &status,0);
    //     kill(pid_list[p], SIGCONT);
    //     waitpid(pid_list[p], NULL, 0);
    // }

    printf("=======sleeping. Parent finished signaling children\n");
    sleep(10);
    return 0;
}

/*-----------------------------Program End-----------------------------------*/

