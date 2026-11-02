#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

/*
    struct for storing the factors and the count of primes found.
*/
typedef struct{
    int factors[31];
    int count;
} PrimeData;

/*
    The function that the threads will access.

    This allocates some memory for PrimeData structs, then goes through the prime factor algorithm
    and returns the PrimeData struct.
*/
void* findPrime(void* arg){
    int value = *(int*)arg;
    int divisor = 2;

    PrimeData* data = malloc(sizeof(PrimeData));
    data->count = 0;
    
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
    Pushes numeric values from the arguments provided into an array and creates the threads based on the argument count.

    The threads join with the returned data and each result of the returned data is printed here in the main function
    and the struct at i is freed after it has printed its results.
*/
int main(int argc, char *argv[]){
    printf("Assignment 5: Multithreaded Factor Finder by Emily Hull.\n");
    pthread_t threads[argc];

    int arr[argc - 1];

    PrimeData* returnedData[argc - 1];

    for(int i = 1; i < argc; i++){
        arr[i - 1] = atoi(argv[i]);
    }

    for(int i = 0; i < argc - 1; i++){
        pthread_create(&threads[i], NULL, findPrime, &arr[i]);
    }
    for(int i = 0; i < argc - 1; i++){
        pthread_join(threads[i], (void**)&returnedData[i]);
    }

    for(int i = 0; i < argc - 1; i++){
        printf("%d: ", arr[i]);

        for(int j = 0; j < returnedData[i]->count; j++){
            printf("%d ", returnedData[i]->factors[j]);
        }

        printf("\n");

        free(returnedData[i]);
    }

    return 0;
}