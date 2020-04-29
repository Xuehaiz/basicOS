/*
* Description: Project2 - Part1
*              MCP Launches the Workload
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
/*---------------------------------------------------------------------------*/

/*-----------------------------Program Main----------------------------------*/
int main(int argc, char* argv[]){
    //char *exit_code = "exit\n";
    char *input, *program, *token, *par, *rem, **arguments;
    //char *pid_list;
    pid_t *pid_list;
    size_t bufsize = 256;
    FILE* fp = stdin;
    int count = 0;
    int i = 0;
    
    //int file = 0; //boolean value to show wether we have a file or not.
    
    //first we will allocate emory for the input, and command array(we will later store commands there)
    input = (char*)malloc(sizeof(char)*bufsize);
    if (input == NULL) {
        fprintf(stderr, "Unable to allocate memory \n");
        exit(EXIT_FAILURE);
    }
    
    arguments = (char**)malloc(sizeof(char*)*bufsize);
    if (arguments == NULL) {
        fprintf(stderr, "Unable to allocate memory \n");
        exit(EXIT_FAILURE);
    }
    
    pid_list = (pid_t *)malloc(256 * sizeof(pid_t));
    if (pid_list == NULL) {
        fprintf(stderr, "PID allocation failure\n");
        exit(EXIT_FAILURE);
    }
    
    fp = fopen(argv[1], "r");
    if(fp == NULL){
        fprintf(stderr, "Unable to open file: %s\n",argv[1]);
        exit(EXIT_FAILURE);
    }
    
    //count the number of lines
    //printf("before while\n");
    printf("My pid is %d\n\n", getpid());
    pid_t current;
    while(getline(&input, &bufsize, fp) != EOF){
        i = 0;
        rem = input;
        token = strtok(rem, "\n ");
        if (token != NULL) {
            program = token;
            arguments[i] = token;
            i++;
        }
        char *parameters;
        par = rem;
        while ((parameters = strtok(NULL, " \n"))) {
            arguments[i] = parameters;
            parameters = strtok(NULL, "\n ");
            i++;
        }
        arguments[i] = NULL;
        current = fork();
        
        if (current < 0) {
            perror("pid less than 0.");
            exit(EXIT_FAILURE);
        }
        printf("Program is %s\n", program);
        for(int l = 0; l < i; l++){
            printf("Arguments[%d] is: %s\n",l, arguments[l]);
        }
        if (current == 0) {
            execvp(program,arguments);
            exit(-1);
        }
        pid_list[count] = current;
        count++;
        
    }
    for(int l = 0; l < count; l++){
        printf("pid[%d]: %d\n", l, pid_list[l]);
    }
    
    //printf("before for loop\n");
    for(int j = 0; j < count; j++){
        //printf("inside for loop\n");
        waitpid(pid_list[j],NULL,0);
        printf("child process done!\n");
        //printf("8\n");
    }
    //printf("9\n");
    fclose(fp);
    free(input);
    free(pid_list);
    free(arguments);
    return 0;
}

/*-----------------------------Program End-----------------------------------*/




