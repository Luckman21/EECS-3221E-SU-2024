/*
Family Name: Irshad
Given Name(s): Luqmaan
Student Number: 217222365
EECS Login ID: luq21
YorkU email address: luq21@my.yorku.ca
*/

#define MAX_CPUS 4 // Number of simulated CPUs = 4

#include "sch-helpers.h" //Change to h on Linux, c on Windows
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/**
 * @brief A struct to store a summary of information related to the CPU Scheduling.
 */
typedef struct Summary
{
    float avg_wait;                   // Average waiting time.
    float avg_turn;                   // Average turnaround time.
    int cpu_runtime;                  // Total runtime of the CPU (clk).
    float cpu_util;                   // CPU Utilization (0 - 400%), 100% max per processor.  Add all the CPU util scores here, then divide by (4 * cpu_runtime) to get score.
    int context_switch;               // How many context switches occured (not including loading new processes after another terminates).
    int last_pids[MAX_PROCESSES + 1]; // Last process(es) to finish
} summary;

process processes[MAX_PROCESSES + 1]; // A large structure array to hold all processes read from data file
process tempArr[MAX_PROCESSES + 1];   // An array to temporarily store processes entering the ready queue.  Used to impose qsort upon.
process_queue *readyQ;                // Ready Queue
process_queue *waitQ;                 // Waiting queue
process_queue *execute;               // Processes currently under execution (max of 4)
process_queue *tempReady;             // Stores an unsorted queue of processes that needs to be sorted before joining the ready queue
process_node *node;                   // Used as a node to iterate through queues
process *temp;                        // Represents a process temporarily for execution purposes
int numberOfProcesses = -1;           // Total number of processes
int processes_complete = 0;           // Total number of processes completed
int clk = 0;                          // A simulated representation of the clock
int quantum;                          // The time quantum specified by terminal parameters
char *q_pointer;                      // A char pointer used for strtol to convert the arg parameter from char array pointer to int
summary *s;                           // A log of summary events to be answered at the end of execution

/**
 * @brief A function used to simulate a CPU burst.  Increments the current CPU burst by 1 and decrements the remaining quantum time, simulating 1 execution cycle.
 *
 * @param p The specified process executing on the CPU.
 */
void cpu(process *p)
{
    p->bursts[p->currentBurst].step++; // Increment a burst by 1
    p->quantumRemaining--;             // Decrement the time quantum remaining by 1
}

/**
 * @brief A function used to simulate an I/O burst.  Increments the current I/O burst by 1, simulating 1 wait cycle.
 *
 * @param p The specified process waiting for I/O in the wait queue.
 */
void io(process *p)
{
    p->bursts[p->currentBurst].step++;
}

/**
 * @brief A function that lists all the processes in the specified queue.
 *
 * @param q A process queue struct, the process queue you want to investigate.
 * @param name The name of the queue you want to display when printing the contents.
 */
void listProcesses(process_queue *q, char name[20])
{
    process_node *n = malloc(sizeof(process_node));

    n = q->front;
    printf("\n%s Queue\n-----\n", name);

    for (int i = 0; i < q->size; i++)
    {
        printf("P%d\n", n->data->pid);
        n = n->next;
    }
}

/**
 * @brief A function used to output a list of all PIDs currently in a queue.  Used soley for debugging.
 *
 * @param q A process queue
 * @param name The specified queue name
 * @return int Returns -1 if there are duplicate processes in the queue.  0 if the back process is not the same as the last process in the queue.  1 otherwise.
 */
int monitorQueue(process_queue *q, char name[20])
{
    process_node *n = malloc(sizeof(process_node));

    int pids[q->size];

    n = q->front;
    for (int i = 1; i < q->size; i++)
    {
        pids[i - 1] = n->data->pid;
        n = n->next;
    }
    pids[q->size - 1] = n->data->pid;

    for (int i = 0; i < q->size; i++)
    {
        for (int j = i + 1; j < q->size; j++)
        {
            if (pids[i] == pids[j])
                return -1;
        }
    }

    if (n->data->pid != q->back->data->pid)
        return 0;
    else
        return 1;
}

/**
 * @brief Checks if there are duplicate PIDs in a queue (bad case).  Used soley for debugging.
 *
 * @param q A process queue
 * @param p A process to compare
 * @return int -1 if the process specified shares a PID with another process already in the queue. 0 otherwise.
 */
int dupe(process_queue *q, process *p)
{
    process_node *n = malloc(sizeof(process_node));

    n = q->front;
    for (int i = 0; i < q->size; i++)
    {
        if (n->data->pid == p->pid)
            return -1;
        n = n->next;
    }
    return 0;
}

/**
 * @brief Compares two processes by their PID.  Lower PID comes first.
 *
 * @param aa Process A
 * @param bb Process B
 * @return int -1 if A.PID < B.PID.  1 if opposite.  0 if equal.
 */
int compareByPID(const void *aa, const void *bb)
{
    process *a = (process *)aa;
    process *b = (process *)bb;
    if (a->pid < b->pid)
        return -1;
    if (a->pid > b->pid)
        return 1;
    return 0;
}

int main(int argc, char *argv[])
{

    // Initialize Summary struct values
    s = malloc(sizeof(summary));
    s->avg_wait = 0;
    s->avg_turn = 0;
    s->cpu_runtime = 0;
    s->cpu_util = 0;
    s->context_switch = 0;

    quantum = strtol(argv[1], &q_pointer, 10); // Assigns the quantum based on the specified input.  strtol converts char array pointer to int, in specified base 10

    int info = 1;  // Stores the return value from readProcess to determine if there is still more input to be parsed
    int index = 0; // Points to the current index of processes[] array to be filled

    // For loop, store process data for all processes in the processes array
    while (info == 1)
    {
        info = readProcess(&processes[index]);
        processes[index].quantumRemaining = quantum; // Give each process the max quantum upon initialization
        numberOfProcesses++;
        index++;
    }

    process finish[numberOfProcesses]; // Create an array to keep track of the completion order for each process

    // Order by arrival time (or lowest PID if the arrival time is the same)
    qsort(processes, numberOfProcesses, sizeof(process), compareByArrival);

    // Set up wait and ready queues
    readyQ = malloc(sizeof(process_queue));
    waitQ = malloc(sizeof(process_queue));
    execute = malloc(sizeof(process_queue));
    tempReady = malloc(sizeof(process_queue));
    node = malloc(sizeof(process_node));
    temp = malloc(sizeof(process));

    initializeProcessQueue(readyQ);
    initializeProcessQueue(waitQ);
    initializeProcessQueue(execute);
    initializeProcessQueue(tempReady);

    index = 0; // Index pointing to the next process to arrive

    // While loop, clk, add processes to ready queue based on FCFS scheduling
    while (processes_complete < numberOfProcesses)
    {

        // Add arrived processes to the ready queue
        for (int i = 0; i < numberOfProcesses; i++)
        {
            if (processes[i].arrivalTime == clk)
            {
                enqueueProcess(readyQ, &processes[i]);
                index++;
            }
        }

        for (int i = 0; i < MAX_CPUS; i++)
        {
            if (readyQ->size == 0 || execute->size == 4)
                break; // If there are no processes in the ready queue or execute queue is full
            else
            {
                node = readyQ->front;
                temp = node->data;
                enqueueProcess(execute, temp);
                dequeueProcess(readyQ);
            }
        }

        // Increment CPU Util based on the number of processes executing
        s->cpu_util += execute->size;

        // If there are processes waiting in the ready queue, add to the total average waiting time
        s->avg_wait += readyQ->size;

        // Simulate a CPU burst x 4 CPUs.  Only runs up to 4 instructions per clk cycle, since we are simulating 4 homogenous CPUs.
        node = execute->front;
        for (int i = 0; i < execute->size; i++)
        {
            temp = node->data;

            cpu(temp); // Complete 1 cycle of a CPU burst on the chosen process

            // If the process has completed it's entire CPU burst
            if (temp->bursts[temp->currentBurst].step == temp->bursts[temp->currentBurst].length)
            {
                temp->currentBurst++; // Increment the burst index

                // If this is not the last burst for this process, context switch and enqueue back to the appropriate queue
                if (temp->currentBurst < temp->numberOfBursts)
                {

                    if (temp->bursts[temp->currentBurst].length == 0)
                    {
                        temp->currentBurst++;

                        enqueueProcess(tempReady, temp); // No I/O burst, not blocked, so place in temp ready queue for sorting before placing in ready queue
                    }
                    else
                        enqueueProcess(waitQ, temp); // I/O burst, so place in wait queue
                }

                // The process has completed it's final burst (CPU)
                else
                {
                    s->avg_turn += (double)(clk - temp->arrivalTime);
                    temp->endTime = clk;
                    finish[processes_complete] = *temp;
                    processes_complete++;
                }
                node = node->next;
                removeProcess(execute, temp);
            }

            else if (temp->quantumRemaining == 0)
            {
                s->context_switch++;             //(For preemtive scheduling only) increment total CS by 1, since we are preemptively switching out a process based on exceeding the time quantum
                enqueueProcess(tempReady, temp); // Still in CPU burst, not blocked by I/O, switched off due to exceeding time quantum, keep in ready queue
                node = node->next;
                removeProcess(execute, temp); // Remove from the execution queue
            }

            else
                node = node->next;
        }

        // Complete an I/O burst for each process in the wait queue
        node = waitQ->front;
        for (int i = 0; i < waitQ->size; i++)
        {
            temp = node->data;

            io(temp); // Complete 1 cycle of an I/O burst on the chosen process

            // If the process has completed it's entire I/O burst
            if (temp->bursts[temp->currentBurst].step == temp->bursts[temp->currentBurst].length)
            {
                temp->currentBurst++; // Increment the burst index

                // If this is not the last burst for this process, context switch and enqueue back to the appropriate queue
                if (temp->currentBurst < temp->numberOfBursts)
                {

                    if (temp->bursts[temp->currentBurst].length == 0)
                    {
                        temp->currentBurst++; // No CPU burst, blocked, so keep in wait queue
                    }
                    else
                        enqueueProcess(tempReady, temp); // Enqueue in the temp ready queue to be sorted before being queued into the ready queue
                }

                // The process has completed it's final burst (I/O)
                else
                {
                    s->avg_turn += (double)(clk - temp->arrivalTime);
                    temp->endTime = clk;
                    finish[processes_complete] = *temp;
                    processes_complete++;
                }
                node = node->next;
                removeProcess(waitQ, temp);
            }
            else
                node = node->next;
        }

        if (tempReady->size > 0)
        {

            if (tempReady->size > 1)
            {

                node = tempReady->front;
                for (int i = 0; i < tempReady->size; i++)
                {
                    tempArr[i] = *node->data;
                    node = node->next;
                }

                qsort(tempArr, tempReady->size, sizeof(process), compareByPID); // Sort in order of PID (since they all arrive on the same clock cycle)

                for (int i = 0; i < tempReady->size; i++)
                {

                    node = tempReady->front;
                    for (int j = 0; j < tempReady->size; j++)
                    {
                        if (node->data->pid == tempArr[i].pid)
                        {
                            node->data->quantumRemaining = quantum; // Reset the quantum
                            enqueueProcess(readyQ, node->data);     // Place in ready queue
                            break;
                        }
                        else
                            node = node->next;
                    }
                }
            }
            else
                enqueueProcess(readyQ, tempReady->front->data);
            while (tempReady->size > 0)
                dequeueProcess(tempReady);
        }
        clk++;
    }

    // All processes have finished execution, compute summary values
    s->cpu_runtime = clk;                      // Compute total CPU runtime
    s->cpu_util /= (float)(s->cpu_runtime);    // Compute the Average CPU Utilization. (cpu_util / (4 * clk))
    s->cpu_util *= (float)(100);               // Average CPU Utilization, multiplied by 100 to get a % value
    s->avg_wait /= (float)(numberOfProcesses); // Average wait = Total wait / # of processes
    s->avg_turn /= (float)(numberOfProcesses); // Average turn = Total turn / # of processes

    int last = 1; // The number of processes that finished last

    s->last_pids[0] = finish[numberOfProcesses - 1].pid;

    // Add processes to the final process array if they have the same finish time
    for (int i = numberOfProcesses - 2; i > 0; i--)
    {
        if (finish[i].endTime == finish[numberOfProcesses - 1].endTime)
        {
            s->last_pids[last] = finish[i].pid;
            last++;
        }
        else
            break;
    }

    printf("Average waiting time : %.2f units\nAverage turnaround time : %.2f units\nTime all processes finished : %d\nAverage CPU utilization : %.1f%%\nNumber of context switches : %d\nPID(s) of last process(es) to finish : ", s->avg_wait, s->avg_turn, s->cpu_runtime, s->cpu_util, s->context_switch);
    for (int i = 0; i < last; i++)
    {
        printf("%d", s->last_pids[i]);

        if (i < last - 1)
            printf(", "); // Formatting output of the print all final PIDs
    }
    printf("\n");

    // Free dynamically allocated memory from the heap to prevent a memory leak
    free(readyQ);
    free(waitQ);
    free(execute);
    free(tempReady);
    free(node);
    return 0;
}