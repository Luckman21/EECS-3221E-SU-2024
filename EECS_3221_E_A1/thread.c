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
    //thread_name SUM DIF MIN MAX
//Output at the end
    //MINIMUM MAXIMUM

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct thread {
    char name[50]; //Thread name (name of the dataset it's assigned to)
    int tid;    //Thread ID
    int min;    //Minimum value of the dataset
    int max;    //Maximum value of the dataset
    int sum;    //SUM = Max + Min
    int dif;    //DIF = Max - Min
}Thread;

int main(int argc, char* argv[]) {

    FILE* dataset;

    //Allocate heap memory for a process struct (each child process can manipulate their own copy of this struct)
    Thread *t = malloc(sizeof(Thread));

    /*
    Create n threads for n datasets using argc and argv
     - argc is the number of parameters in the terminal when the program is run
        - This includes the program name as the 1st element
     - argv is a "string" (char[]) array of each element in the CLI
    */
    for (int i = 1; i < argc; i++) {
        //Create a thread
    }

    //If parent process
    for (int i = 0; i < argc - 1; i++) {
        //wait()

        //Print: name SUM=sum DIF=dif MIN=min MAX=max
        //printf("%s SUM=%d DIF=%d MIN=%d MAX=%d", p->name, p->sum, p->dif, p->max);
        
        if (i == 0) {
            //Set min and max values to the values from the first process to finish execution
        }

        else {
            //If child min < parent min | parent min = child min

            //If child max > parent max | parent max = child max
        }
    }
    //All children finished executing
        //Output MAXIMUM and MINIMUM
    printf("MINIMUM=%d MAXIMUM=%d", t->min, t->max);

    return 0;
}