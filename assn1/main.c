#include <stdio.h>
#include <string.h>

int main(int argc, char* argv []){

    //leftovers from the first step of writing this program.
    //printf("The argc parameter is %d\n", argc);
    //printf("The first argv parameter is %s\n", argv[0]);
    //printf("The second argv parameter is %s\n", argv[1]);

    printf("Assignment 1 by Emily Hull\n");
    
    printf("Argument 1: %s\n", argv[1]);
    printf("--- System Report ---\n");

    FILE *file = fopen("/proc/version", "r");
    char line[1024];
    fgets(line, sizeof(line), file);
    printf("Kernel: %s", line);
    fclose(file);

    file = fopen("/proc/cpuinfo", "r");
    while(fgets(line, sizeof(line), file) != NULL){
        if(strncmp(line, "model name", 10) ==  0){
            printf("CPU: %s", line);
            break;
        }
    }
    
    fclose(file);

    return 0;
}