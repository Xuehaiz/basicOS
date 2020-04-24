#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main()
{
	pid_t pid;

	printf("My pid is %d\n\n", getpid());

	pid = fork();

	if (pid == 0) {
		printf("Child process is %d, and parent pid is %d\n\n", getpid(), getppid());
	}
	else {
		wait(0);
		printf("Child finished, main exiting, my pid is %d\n\n", getpid());
	}
	return 0;
}