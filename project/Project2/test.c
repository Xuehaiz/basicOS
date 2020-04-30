#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/wait.h>

void signalHandler(int sig) {
  printf("Child process: %d - recieved signal: %d", getpid(), sig);
}

int main() {

  //Variable Declarations
  pid_t pid;
  int status;
  sigset_t signalSet;
  char *command = "ls"; //put any command here
  char *arguments[4] = {"-a", "-r", "-s", NULL}; //these are the arguments

  //init signal set for sigwait
  sigemptyset(&signalSet);
  sigaddset(&signalSet, SIGUSR1);
  struct sigaction action;
  action.sa_handler = signalHandler;
  sigaction(SIGUSR1, &action, NULL);


  // Create a child process
  pid = fork();
  if(pid < 0) {
    // This block will only be executed if an error happened
    printf("Error! Process: %d - Unable to create child process.\n", getpid());
  }else if (pid == 0) {
    // This block will only be executed by the child
    printf("Child process: %d - Starting execution.\n", getpid());

    // Waiting for SIGUSR1
    printf("Child process: %d - Waiting for SIGUSR1...\n", getpid());
    int signal;
    sigwait(&signalSet, &signal);

    // Exec call
    printf("Child process: %d - Received signal: SIGUSR1 - calling exec().\n", getpid());
    execvp(command, arguments);

    // End the child process
    exit(1);

  } else {
    //This block of code will only be run by the parent

    //Send the signals to the children
    printf("Parent process: %d - Sending signal: %d to child process: %d\n", getpid(), SIGUSR1, pid);
    sleep(2);
    kill(pid, SIGUSR1);
    printf("Parent process: %d - Signal sent.\n", getpid());

    // Join child processes
    printf("Parent process: %d - Joining child process: %d\n", getpid(), pid);
    pid = waitpid(pid, &status, 0);
    printf("Parent process: %d - child process: %d joined\n", getpid(), pid);
  }

  printf("Parent process: %d - Finished, exiting...\n", getpid());
  return 1;
}
