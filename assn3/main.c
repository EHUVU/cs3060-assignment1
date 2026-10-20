#include <stdio.h>
/*
Assignment 3 - Emily Hull.

The .txt files for this are:

testlist.txt
testlist2.txt

*/
const int QUANTUM = 100;
const int MAX = 100;

/*
Made this process struct to make it easier for me to visualize.
*/
typedef struct{
    int subTime;
    int runTime;
} Process;

/*
FIFO implementation.

Advances the clock based on the current process submission time if it is less than the submission time.

Calculates the response and turnaroundtimes based on the current start and end times minus the current process subtime. Wait is
calculated by the turnaround minus the current process runtime.
*/
void FIFO(int n, const Process processList[]){
    printf("FIFO\n");
    if(n == 0){
        return;
    }

    int clock = 0;
    double responseTimeTotal = 0;
    double turnAroundTotal = 0;
    double waitTimeTotal = 0;

    for(int i = 0; i < n; ++i){
        if(clock < processList[i].subTime){
            clock = processList[i].subTime;
        }

        int start = clock;
        int end = start + processList[i].runTime;

        int response = start - processList[i].subTime;
        int turnAround = end - processList[i].subTime;
        int wait = turnAround - processList[i].runTime;

        responseTimeTotal += response;
        turnAroundTotal += turnAround;
        waitTimeTotal += wait;

        clock = end;
    }

    printf("Avg. Resp.:%.2f, Avg. T. A.:%.2f, Avg. Wait:%.2f\n", responseTimeTotal /n, turnAroundTotal /n, waitTimeTotal /n);
}

/*
Round Robin implementation

Uses the global constant QUANTUM to determine the amount of time given to each process. Uses the MAX constant set the
maximum number of processes supported by the queue and other arrays.

Each process that gets completed increments finished by one until it breaks the while loop and prints the result.

Implemented the tail and head positions in a circular queue to wrap around to 0 if the tail or head reach 100.
*/
void RoundRobin(int totalJobCount, const Process processList[]){
    int queue[MAX];
    int head = 0;
    int tail = 0;
    int count = 0;

    int remaining[MAX];
    int response[MAX];
    int completion[MAX];

    int nextJob = 0;

    double responseTimeTotal = 0;
    double turnAroundTotal = 0;
    double waitTimeTotal = 0;

    int clock = 0;
    int finished = 0;

    for(int i = 0; i < totalJobCount; ++i){
        remaining[i] = processList[i].runTime;
        response[i] = -1;
        completion[i] = 0;
    }

    while(finished < totalJobCount){
            while(nextJob < totalJobCount && processList[nextJob].subTime <= clock){
                queue[tail] = nextJob;
                tail = (tail + 1) % MAX;
                count++;
                nextJob++;
        }
            if(count == 0){
            if(nextJob < totalJobCount){
                clock = processList[nextJob].subTime;
                continue;
            }
            break;
        }

        int job = queue[head];
        head = (head + 1) % MAX;
        count--;

        if(response[job] == -1){
            response[job] = clock - processList[job].subTime;
        }

        int runTime;
        if(remaining[job] < QUANTUM){
            runTime = remaining[job];
        }else{
            runTime = QUANTUM;
        }

        clock += runTime;
        remaining[job] -= runTime;

        while(nextJob < totalJobCount && processList[nextJob].subTime <= clock){
            queue[tail] = nextJob;
            tail = (tail + 1) % MAX;
            count++;
            nextJob++;
        }

        if(remaining[job] == 0){
            completion[job] = clock;
            finished++;
            
            int turnAround = completion[job] - processList[job].subTime;
            int wait = turnAround - processList[job].runTime;
            responseTimeTotal += response[job];
            turnAroundTotal += turnAround;
            waitTimeTotal += wait;
        }else{
            queue[tail] = job;
            tail = (tail + 1) % MAX;
            count++;
        }
    }
        printf("Round Robin with Time Quantum of %d\n", QUANTUM);
        if(totalJobCount > 0){
            printf("Avg. Resp.:%.2f, Avg. T. A.:%.2f, Avg. Wait:%.2f\n", responseTimeTotal/totalJobCount, turnAroundTotal/totalJobCount, waitTimeTotal/totalJobCount);
        }
}

/*
Main function

Reads the argument filename and throws them into a process array.
*/
int main(int argc, char *argv[]){
    printf("Assignment 3: Process Scheduler Simulator by Emily Hull.\n");
    Process processList[MAX];
    int jobNum = 0;

    FILE *fp = stdin;

    if(argc > 1){
        fp = fopen(argv[1], "r");
        if(fp == NULL){
            perror("Failure to open file.");
            return -1;
        }
    }

    while( jobNum < MAX && fscanf(fp, "%d%d", 
        &processList[jobNum].subTime, 
        &processList[jobNum].runTime) == 2){
            jobNum++;
    }
    
    FIFO(jobNum, processList);
    RoundRobin(jobNum, processList);

    return 0;
}