/*
Family Name: Irshad
Given Name(s): Luqmaan
Student Number: 217222365
EECS Login ID: luq21
YorkU email address: luq21@my.yorku.ca
*/

//argc and argv, count the number of datasets
//Create n threads for n datasets
    //thread name = dataset file name
    //sum = min + max
    //dif = max - min

//Output for each thread
    //child_name SUM DIF MIN MAX
//Output at the end
    //MINIMUM MAXIMUM

#define _GNU_SOURCE     //Defined for pthread_tryjoin_np()

//Header files to include
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

typedef struct thread {
    char name[50];  //Thread name (name of the dataset it's assigned to)
    int index;      //Tracks the index of the thread's memory struct in the t_mem array
    int done;       //Tracks whether the thread has completed execution.  1 For running, 0 for complete, 2 for read.
    double sum;     //SUM = Min + Max
    double dif;     //DIF = Min - Max
}Thread;

void *thread_function(void *param);

int main(int argc, char* argv[]) {

    pthread_t workers[argc - 1];    //Create the number of threads based on the number of datasets input
    Thread t_mem[argc - 1];         //Allocate heap memory for threads to store their memory to (heap shared between threads, create a struct instance)
    int thrnum = argc - 1;          //Tracks the number of threads

    /*
    Create n threads for n datasets using argc and argv
     - argc is the number of parameters in the terminal when the program is run
        - This includes the program name as the 1st element
     - argv is a "string" (char[]) array of each element in the CLI
    */
    for (int i = 1; i < argc; i++) {
        strcpy(t_mem[i-1].name, argv[i]);   //Make dataset name available to the thread
        t_mem[i-1].index = i-1;             //Keep track of index in thread struct array for main thread to pull data when thread finishes execution
        t_mem[i-1].done = 1;                //Sets the done status to "running".
        pthread_create(&workers[i-1], NULL, thread_function, &t_mem[i-1]);  //Create a thread of thread_function function, pass the struct memory to it to pass multiple pieces of data
        pthread_tryjoin_np(workers[i-1], NULL);
    }

    double maxF = 0, minF = 0;  //Define total max and min values

    //Loops through all thread data to print values
    while (thrnum > 0) {
        for (int i = 0; i < argc - 1; i++) {
            if (t_mem[i].done == 0) {
                double max, min, sum, dif;  //Instantiate max, min, sum and dif values for each dataset
                Thread *tx = malloc(sizeof(Thread));    //Use a struct to copy struct data temporarily
                
                //Wait for a thread to finish execution

                /*Calculate max and min (use double precision)
                    max = (sum + dif) / 2
                    min = (sum - dif) / 2
                Print: name SUM=sum DIF=dif MIN=min MAX=max
                printf("%s SUM=%d DIF=%d MIN=%d MAX=%d", p->name, p->sum, p->dif, p->max);*/

                *tx = t_mem[i]; //Copy struct data of thread that finished for processing

                sum = tx->sum;
                dif = tx->dif;

                //Make the difference positive for correct calculation
                if (dif < 0) {
                    max = ((sum + (dif * (double)(-1))) / (double)(2));
                    min = ((sum - (dif * (double)(-1))) / (double)(2));
                }

                if (i == 0) {
                    //Set min and max values to the values from the first process to finish execution
                    minF = min;
                    maxF = max;
                }

                else {
                    //If thread min < total min | total min = thread min
                    if (min < minF)
                        minF = min;

                    //If thread max > total max | total max = thread max
                    if (max > maxF)
                        maxF = max;
                }

                //Print all the significant values for the thread
                printf("%s SUM=%lf DIF=%lf MIN=%lf MAX=%lf\n", tx->name, sum, dif, min, max);
                
                t_mem[i].done = 2;
                thrnum--;
                free(tx);  //Free memory from the heap
            }

        }
    }
    /*All threads finished execution
        Output MAXIMUM and MINIMUM*/
    printf("MINIMUM=%lf MAXIMUM=%lf\n", minF, maxF);

    return 0;
}

/**
 * @brief Function that runs as a thread when called using pthread_create and join.  
 * It computes the sum and dif between the max and min values of the dataset.
 * 
 * @param param The parameter passed to the thread.  In this case, I pass a Thread struct I created to pass a set of data to the thread.
 * @param index The index assigned to this thread from the threads struct array.
 * @param t The thread struct array index passed as a pointer, so the thread can write to shared heap memory.
 * @return void* 
 */
void *thread_function(void *param) {

    //Init values like the file, min, max and current numbers from the dataset
    FILE* dataset;
    double num;
    double min;
    double max;
    int first = 1;  //A "boolean" to identify if it's the first iteration of the loop.  Set "off" (to 0) after iteration 0.

    Thread *t = malloc(sizeof(Thread));     //Allocate memory for the thread
    t = param;                              //Point all values of t to the passed struct

    dataset = fopen(t->name,"r");   //Read data from dataset n file

    //For loop to parse data, runs until end of file
    while (fscanf(dataset, "%lf", &num) != EOF) {

        //Set the first max and min to actual values of the dataset instead of having them initialized with garbage values
        if (first == 1) {
            min = num;
            max = num;
            first = 0;
        }

        if (num < min)
            min = num;

        if (num > max)
            max = num;
    }

    fclose(dataset);    //Close file after using

    //Calculate sum and dif
    t->sum = min + max;
    t->dif = min - max;
    
    //Update value in struct to inform main thread that this thread has completed execution
    t->done = 0;

    return NULL;
}