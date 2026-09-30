#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

/*
Emily Hull
Int counter is decremented by the child process and the address and value is printed out. Any extra arguments by the child are executed before the child process ends. The parent
prints last and prints the contents of counter and the address of counter to demonstrate the child and parent's separate memory despite the same address value.
*/
int main(int argc, char* argv []){
    int counter = 5;
    pid_t pid = fork();

    if(pid == 0){
        printf("Child Process started, decrementing counter... \n");
        counter--;
        printf("Counter value after decrement: %d\n", counter);
        printf("Counter memory address: %p\n", &counter);

    if(argc == 1){
        printf("No arguments. Terminate.\n");
    }else if(argc == 2){
        execlp(argv[1], argv[1], (char *)NULL);
        perror("execvp");
        return 1;
    }else{
        execvp(argv[1], &argv[1]);

        perror("execvp");
        return 1;
    }
    }
    else{
        printf("Write by parent process waiting on child process. \n");

        waitpid(pid, NULL, 0);

        printf("Counter value after child process execution: %d\n", counter);
        printf("Counter memory address: %p\n", &counter);
    }

    return 0;
}