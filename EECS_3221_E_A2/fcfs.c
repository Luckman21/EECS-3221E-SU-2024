/*
Family Name: Irshad
Given Name(s): Luqmaan
Student Number: 217222365
EECS Login ID: luq21
YorkU email address: luq21@my.yorku.ca
*/

#define MAX_CPUS 4  //Number of simulated CPUs = 4

#include "sch-helpers.c"    //Change to h
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/**
 * @brief A struct to store a summary of information related to the CPU Scheduling.
 */
typedef struct Summary {
    double avg_wait;                    //Average waiting time.
    double avg_turn;                    //Average turnaround time.
    int cpu_runtime;                    //Total runtime of the CPU (clk).
    double cpu_util;                    //CPU Utilization (0 - 400%), 100% max per processor.  Add all the CPU util scores here, then divide by (4 * cpu_runtime) to get score.
    int context_switch;                 //How many context switches occured (not including loading new processes after another terminates).
    int last_pids[MAX_PROCESSES + 1];   //Last process(es) to finish
}summary;

process processes[MAX_PROCESSES + 1];   //A large structure array to hold all processes read from data file 
process_queue *readyQ;                  //Ready Queue
process_queue *waitQ;                   //Waiting queue
process_queue *execute;                 //Processes currently under execution (max of 4)
process_queue *tempReady;               //Stores an unsorted queue of processes that needs to be sorted before joining the ready queue
process_node *node;                     //Used as a node to iterate through queues
process *temp;                          //Represents a process temporarily for execution purposes
int numberOfProcesses = 0;              //Total number of processes
int processes_complete = 0;             //Total number of processes completed
int clk = 0;                            //A simulated representation of the clock
summary *s;                             //A log of summary events to be answered at the end of execution

//Simulates a CPU burst
int cpu(process *p) {
    p->bursts[p->currentBurst].step++;
    return 0;
}

//Simulates an I/O burst (same implementation, but wanted different names for readability)
int io(process *p) {
    p->bursts[p->currentBurst].step++;
    return 0;
}

int compareByPID(const void *aa, const void *bb) {
    process *a = (process*) aa;
    process *b = (process*) bb;
    if (a->pid < b->pid) return -1;
    if (a->pid > b->pid) return 1;
    return 0;
}

int main(int argc, char* argv[]) {

    s = malloc(sizeof(summary));

    int info = 1;   //Stores the return value from readProcess to determine if there is still more input to be parsed
    int index = 0;  //Points to the current index of processes[] array to be filled

    //For loop, store process data for all processes in the processes array
    while (info == 1) {
        info = readProcess(&processes[index]);
        numberOfProcesses++;
        index++;
    }

    process finish[numberOfProcesses];  //Create an array to keep track of the completion order for each process

    //Order by arrival time (or lowest PID if the arrival time is the same)
    qsort(processes, numberOfProcesses, sizeof(process), compareByArrival);

    //s->cpu_start = 

    //Set up wait and ready queues
    readyQ = malloc(sizeof(process_queue));
    waitQ = malloc(sizeof(process_queue));
    execute = malloc(sizeof(process_queue));
    tempReady = malloc(sizeof(process_queue));
    node = malloc(sizeof(process_node));

    initializeProcessQueue(readyQ);
    initializeProcessQueue(waitQ);
    initializeProcessQueue(execute);
    initializeProcessQueue(tempReady);

    index = 0;  //Index pointing to the next process to arrive

    //While loop, clk, add processes to ready queue based on FCFS scheduling
    while (processes_complete < numberOfProcesses) {

        //Add arrived processes to the ready queue
        while (index < numberOfProcesses) {
            if (processes[index].arrivalTime == clk) {
                enqueueProcess(readyQ, &processes[index]);
                index++;
            }
            else break;
        }

        for (int i = 0; i < MAX_CPUS; i++) {
            if (i > readyQ->size || readyQ->size == 0) break;   //If there are less processes in the ready queue than the number of CPUs
            else {
                temp = malloc(sizeof(process));
                node = readyQ->front;
                temp = node->data;
                enqueueProcess(execute, temp);
                dequeueProcess(readyQ);
            }
        }

        //Move processes to waiting queue if blocked
        s->cpu_util += execute->size;

        //Simulate a CPU burst x 4 CPUs.  Only runs up to 4 instructions per clk cycle, since we are simulating 4 homogenous CPUs.
        node = execute->front;
        for (int i = 0; i < execute->size; i++) {

            temp = malloc(sizeof(process));
            temp = node->data;

            cpu(temp);  //Complete 1 cycle of a CPU burst on the chosen process
            
            //If the process has completed it's entire CPU burst
            if (temp->bursts[temp->currentBurst].step >= temp->bursts[temp->currentBurst].length) {
                temp->currentBurst++;   //Increment the burst index

                //If this is not the last burst for this process, context switch and enqueue back to the appropriate queue
                if (temp->currentBurst < temp->numberOfBursts) {
                    s->context_switch++;
                    
                    if (temp->bursts[temp->currentBurst].length == 0) {
                        temp->currentBurst++;
                        enqueueProcess(tempReady, temp);  //No I/O burst, not blocked, so place in temp ready queue for sorting before placing in ready queue
                    }
                    else enqueueProcess(waitQ, temp);   //I/O burst, so place in wait queue
                }

                //The process has completed it's final burst (CPU)
                else {
                    s->avg_wait += (double)(temp->waitingTime);
                    s->avg_turn += (double)(clk - temp->arrivalTime);
                    temp->endTime = clk;
                    finish[processes_complete] = *temp;
                    processes_complete++;
                }
                dequeueProcess(execute);
                node = execute->front;
            }
            else node = node->next;
        }

        //Complete an I/O burst for each process in the wait queue
        node = waitQ->front;
        for (int i = 0; i < waitQ->size; i++) {
            temp = malloc(sizeof(process));
            temp = node->data;

            io(temp);  //Complete 1 cycle of an I/O burst on the chosen process
            
            //If the process has completed it's entire I/O burst
            if (temp->bursts[temp->currentBurst].step >= temp->bursts[temp->currentBurst].length) {
                temp->currentBurst++;   //Increment the burst index

                //If this is not the last burst for this process, context switch and enqueue back to the appropriate queue
                if (temp->currentBurst < temp->numberOfBursts) {
                    
                    if (temp->bursts[temp->currentBurst].length == 0) {
                        temp->currentBurst++;
                        enqueueProcess(waitQ, temp); //No CPU burst, blocked, so place in wait queue
                    }
                    else enqueueProcess(tempReady, temp);    //Enqueue in the temp ready queue to be sorted before being queued into the ready queue
                }

                //The process has completed it's final burst (I/O)
                else {
                    s->avg_wait += (double)(temp->waitingTime);
                    s->avg_turn += (double)(clk - temp->arrivalTime);
                    temp->endTime = clk;
                    finish[processes_complete] = *temp;
                    processes_complete++;
                }
                dequeueProcess(waitQ);
                node = waitQ->front;
            }
            else node = node->next;
        }

        qsort(tempReady, tempReady->size, sizeof(process), compareByPID);   //Sort in order of PID (since they all arrive on the same clock cycle)
        for (int i = 0; i < tempReady->size; i++) enqueueProcess(readyQ, temp);   //Place in ready queue
        while (tempReady->size > 0) dequeueProcess(tempReady);

        clk++;
    }

    //All processes have finished execution, compute summary values
    s->cpu_runtime = clk; //Compute total CPU runtime
    s->cpu_util /= (MAX_CPUS * s->cpu_runtime); //Compute the Average CPU Utilization. (cpu_util / (4 * clk))
    s->avg_wait /= (double)(numberOfProcesses); //Average wait = Total wait / # of processes
    s->avg_turn /= (double)(numberOfProcesses); //Average turn = Total turn / # of processes

    int last = 1;   //The number of processes that finished last

    s->last_pids[0] = finish[numberOfProcesses - 1].pid;

    //Add processes to the final process array if they have the same finish time
    for (int i = numberOfProcesses - 1; i > 0; i--) {
        if (finish[i].endTime == finish[numberOfProcesses - 1].endTime) {
             s->last_pids[last] = finish[i].pid;
             last++;
        }
    }

    printf("Average waiting time: %lf units\nAverage turnaround time: %lf units\nTime all processes finished: %d\nAverage CPU utilization: %lf%%\nNumber of context switches: %d\nPID(s) of last process(es) to finish: ", s->avg_wait, s->avg_turn, s->cpu_runtime, s->cpu_util, s->context_switch);
    for (int i = 0; i < last; i++) {
        printf("%d", s->last_pids[i]);

        if (i < last - 1) printf(", ");  //Formatting output of the print all final PIDs
    }

    //Free dynamically allocated memory from the heap to prevent a memory leak
    free(readyQ);
    free(waitQ);
    free(execute);
    free(tempReady);
    free(node);
    free(temp);

    return 0;
}