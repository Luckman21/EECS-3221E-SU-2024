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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
//#include <sys/wait.h>
#include <pthread.h>

typedef struct thread {
    char name[50];  //Thread name (name of the dataset it's assigned to)
    int index;      //
    double sum;     //SUM = Min + Max
    double dif;     //DIF = Min - Max
}Thread;

/**
 * @brief Function that runs as a thread when called using pthread_create.  
 * It computes the sum and dif between the max and min values of the dataset.
 * 
 * @param param 
 * @param index The index assigned to this thread from the threads struct array.
 * @param t The thread struct array index passed as a pointer, so the thread can write to shared heap memory.
 * @return void* 
 */

void *thread_function(void *param);

int main(int argc, char* argv[]) {

    pthread_t workers[argc - 1];    //Create the number of threads based on the number of datasets input
    Thread t_mem[argc - 1];         //Allocate heap memory for threads to store their memory to (heap shared between threads, create a struct instance)

    /*
    Create n threads for n datasets using argc and argv
     - argc is the number of parameters in the terminal when the program is run
        - This includes the program name as the 1st element
     - argv is a "string" (char[]) array of each element in the CLI
    */
    for (int i = 1; i < argc; i++) {
        strcpy(t_mem[i-1].name, argv[i]);
        t_mem[i-1].index = i-1;
        pthread_create(&workers[i-1], NULL, thread_function, &t_mem[i-1]);
        pthread_join(workers[i-1], NULL);
    }

    double maxF, minF = 0;

    for (int i = 0; i < argc - 1; i++) {

        double max, min, sum, dif;
        Thread *tx = malloc(sizeof(Thread));
        
        //Wait for a thread to finish execution

        /*Calculate max and min (use double precision)
            max = (sum + dif) / 2
            min = (sum - dif) / 2
        Print: name SUM=sum DIF=dif MIN=min MAX=max
        printf("%s SUM=%d DIF=%d MIN=%d MAX=%d", p->name, p->sum, p->dif, p->max);*/

        sum = tx->sum;
        dif = tx->dif;

        max = ((sum + dif) / (double)(2));
        min = ((sum - dif) / (double)(2));
        
        if (i == 0) {
            //Set min and max values to the values from the first process to finish execution
            minF = min;
            maxF = max;
        }

        else {
            //If child min < parent min | parent min = child min
            if (min < minF)
                minF = min;

            //If child max > parent max | parent max = child max
            if (max > maxF)
                maxF = max;
        }

        //Print all the significant values for the child process
        printf("%s SUM=%lf DIF=%lf MIN=%lf MAX=%lf\n", tx->name, sum, dif, min, max);
        
        free(tx);  //Free memory from the heap
    }
    /*All children finished executing
        Output MAXIMUM and MINIMUM*/
    printf("MINIMUM=%lf MAXIMUM=%lf\n", minF, maxF);

    return 0;
}

void *thread_function(void *param) {

    FILE* dataset;
    double num;
    double min;
    double max;
    int first = 1;

    Thread *t = malloc(sizeof(Thread));
    t = param;

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

    return NULL;
}