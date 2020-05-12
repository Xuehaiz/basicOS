#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>


void signaler(pid_t *process_pool, int sign){
    int i = 0;
    while(process_pool[i] != -1){
        sleep(1);

        if(sign == SIGUSR1){
            printf("Parent process: %d - Sending signal: SIGUSR1 to child process: %d\n", getpid(), process_pool[i]);
            kill(process_pool[i], sign);
        }

        else if (sign == SIGSTOP){
            printf("Parent process %d - Sending signal: SIGSTOP to child process: %d\n", getpid(), process_pool[i]);
            kill(process_pool[i], sign);
        }

        else if (sign == SIGCONT){
            printf("Parent process %d - Sending signal: SIGCONT to child process: %d\n", getpid(), process_pool[i]);
            kill(process_pool[i], sign);
        }
        i++;
    }
}


int main(int argc __attribute__ ((unused)), char *argv[]){

    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGUSR1);
    sigaddset(&sigset, SIGCONT);
    sigaddset(&sigset, SIGSTOP);
    sigaddset(&sigset, SIGALRM);
    sigprocmask(SIG_BLOCK,&sigset,NULL);

    // char input_file[64];
    int size = 1024;
    char buffer[size];
    size_t nread;
    int is_command = 1;
    char command[128][128];
    char parameter[128][128];
    // int index;
    int count_command = 0;
    //int num_process = 5;

    int file = open(argv[1], O_RDONLY);

    if(file == -1){
        close(file);
        const char error_report[] = "ERROR! cont open the input file!\n";
        write(1, error_report, strlen(error_report));
        exit(1);
    }

    while((nread = read(file, &buffer, size)) != 0){
        char *line;
        char *other_line = buffer;
        while((line = strtok_r(other_line, "\n", &other_line))){
            char *command_line;
            char *parameter_line = line;
            while((command_line = strtok_r(parameter_line, " ", &parameter_line))){
                if(is_command){
                    strcpy(command[count_command], command_line);
                    is_command = 0;
                }
                else {
                    strcpy(parameter[count_command] + strlen(parameter[count_command]), command_line);
                    strcat(parameter[count_command], " ");
                    nread++;
                }
            }
            is_command = 1;
            count_command++;
            nread = 0;
        }
    }
    close(file);

    pid_t pid[count_command];
    char current[128];
    getcwd(current,128);
    for(int i = 0; i < count_command; i++) {
        pid[i] = fork();
        if(pid[i] < 0){
            const char error_fork[] = "ERROR fork.\n";
            write(1, error_fork, strlen(error_fork));
            exit(1);
        }
        else if (pid[i] == 0){
            int signal = SIGUSR1;
            char *take_para;
            char *rest = parameter[i];
            char *collect_para[128];
            int status;
            int a = 0;
            sigwait(&sigset, &signal);
      
            if (WIFEXITED(status)) {
                printf("Children process: %d -received signal: SIGUSR1 -Calling exec().\n", getpid());
                take_para = strtok(rest, " ");
                while(take_para != NULL){
                    collect_para[a] = take_para;
                    take_para = strtok(NULL," ");
                    a++;
                }

                collect_para[a] = NULL;
                char *commands[10];
                commands[0] = command[i];
                for (int i = 1; i < 10; i++){
                    commands[i] = collect_para[i - 1];
                }
                execvp(command[i], commands);
            }
            exit(1);
        }
    }
    pid[count_command] = -1;
    signaler(pid,SIGUSR1);
    //signaler(pid,SIGSTOP);
    //signaler(pid,SIGCONT);

    int a = 0;
    int status;
    pid_t wpid;
    while(1){
        wpid = waitpid(pid[a], &status, WNOHANG);
        if(WIFEXITED(status)){
            printf("the childred: %d is finished.\n", pid[a]);
            for(int index = a; index < count_command; index++){
                pid[index] = pid[index + 1];
            }
            count_command--;
            if(count_command == 0){
                printf("all children processs are now finished.\n");
                break;
            }
        }

        kill(pid[a], SIGSTOP);
        printf("Parent process %d - Sending signal: SIGSTOP to child process: %d\n", getpid(), pid[a]);
        a++;

        if(a == count_command){
            a = 0;
        }

        kill(pid[a], SIGCONT);
            printf("Parent process %d - Sending signal: SIGSTOP to child process: %d\n", getpid(), pid[a]);

    }

    alarm(2);
    pause();

    for (int z = 0; z < count_command; z++){
        wait(NULL);
    }

    return 0;
}
