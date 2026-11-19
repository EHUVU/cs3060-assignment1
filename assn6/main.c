#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <semaphore.h>


#define BUFFER_SIZE 20

/*
    struct for storing the factors and the count of primes found.
*/
typedef struct{
    int factors[31];
    int count;
    int task;
} PrimeData;

int inputBuffer[BUFFER_SIZE];
PrimeData* outputBuffer[BUFFER_SIZE];

sem_t inEmpty;
sem_t inFull;

sem_t outEmpty;
sem_t outFull;
/*
    The function that the threads will access.

    This allocates some memory for PrimeData structs, then goes through the prime factor algorithm
    and returns the PrimeData struct.
*/
PrimeData* findPrime(int value){
   int divisor = 2;

    PrimeData* data = malloc(sizeof(PrimeData));
    data->count = 0;
    data->task = value;
    
    while(divisor * divisor <= value){
        while(value % divisor == 0){
            data->factors[data->count] = divisor;
            data->count++;
            value = value / divisor;
        }
        divisor++;
    }

    if(value > 1){
        data->factors[data->count] = value;
        data->count++;
    }

    return data;
}

/*
Compute thread function. Reads the values from the input buffer, calls the findPrime and saves the original value to the output buffer.
*/
void* computeThread(void* arg){
    int taskCount = *(int*)arg;
    int inIndex = 0;
    int outIndex = 0;

    for(int i = 0; i < taskCount; i++){
        sem_wait(&inFull);

        int value = inputBuffer[inIndex];
        inIndex = (inIndex + 1) % BUFFER_SIZE;

        sem_post(&inEmpty);

        PrimeData* data = findPrime(value);

        sem_wait(&outEmpty);

        outputBuffer[outIndex] = data;
        outIndex = (outIndex + 1) % BUFFER_SIZE;

        sem_post(&outFull);
    }

    return NULL;
}
 /*
 Output thread function prints title and displays the output from the shared output buffer.
 */
void* outputThread(void* arg){
    int taskCount = *(int*)arg;
    int outIndex = 0;
    printf("Assignment 6: Multithreaded Factor Finder by Emily Hull.\n");

    for(int i = 0; i < taskCount; i++){
        sem_wait(&outFull);

        PrimeData* data = outputBuffer[outIndex];
        outIndex = (outIndex + 1) % BUFFER_SIZE;

        sem_post(&outEmpty);

        printf("%d: ", data->task);

        for(int j = 0; j < data->count; j++){
            printf("%d ", data->factors[j]);
        }

        printf("\n");

        free(data);
    }

    return NULL;
}

/*
    Pushes numeric values from the arguments provided into an array and creates the threads based on the argument count.

    The threads join with the returned data and each result of the returned data is printed here in the main function
    and the struct at i is freed after it has printed its results.
*/
int main(int argc, char *argv[]){
    pthread_t compute;
    pthread_t output;

    int taskCount = argc - 1;
    int inIndex = 0;

    sem_init(&inEmpty, 0, BUFFER_SIZE);
    sem_init(&inFull, 0, 0);

    sem_init(&outEmpty, 0, BUFFER_SIZE);
    sem_init(&outFull, 0, 0);

    pthread_create(&output, NULL, outputThread, &taskCount);
    pthread_create(&compute, NULL, computeThread, &taskCount);

    for(int i = 1; i < argc; i++){
        sem_wait(&inEmpty);

        inputBuffer[inIndex] = atoi(argv[i]);
        inIndex = (inIndex + 1) % BUFFER_SIZE;

        sem_post(&inFull);
    }

    pthread_join(compute, NULL);
    pthread_join(output, NULL);

    sem_destroy(&inEmpty);
    sem_destroy(&inFull);
    sem_destroy(&outEmpty);
    sem_destroy(&outFull);

    return 0;
}