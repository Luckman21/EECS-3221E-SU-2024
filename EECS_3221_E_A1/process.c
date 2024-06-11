/*
Family Name: Irshad
Given Name(s): Luqmaan
Student Number: 217222365
EECS Login ID: luq21
YorkU email address: luq21@my.yorku.ca
*/

//argc and argv, count the number of datasets
//Create n processes for n datasets
    //child process name = dataset file name
    //sum = min + max
    //dif = max - min

//Output for each process
    //child_name SUM DIF MIN MAX
//Output at the end
    //MINIMUM MAXIMUM

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct process {
    char name[50]; //Process name (name of the dataset it's assigned to)
    int pid;    //Process ID
    int min;    //Minimum value of the dataset
    int max;    //Maximum value of the dataset
    int sum;    //SUM = Max + Min
    int dif;    //DIF = Max - Min
}Process;

typedef struct HashtableNode {
    char value[50]; //Name of the dataset
    int key;        //PID
}Hashtable;

int main(int argc, char* argv[]) {

    FILE* dataset;
    int* child_pid;
    char child_name[50];

    //Allocate heap memory for a process struct (each child process can manipulate their own copy of this struct)
    Process *p = malloc(sizeof(Process));

    //Create an array of Hashtable Node elements to store PID as the key and dataset name as the value
    Hashtable *hash[] = malloc(sizeof(Hashtable) * (argc - 1));

    /*
    Create n processes for n datasets using argc and argv
     - argc is the number of parameters in the terminal when the program is run
        - This includes the program name as the 1st element
     - argv is a "string" (char[]) array of each element in the CLI
    */
    for (int i = 1; i < argc; i++) {
        //Create a pipe before the fork so it is accessible to the child
        //Fork() a process
        //p->pid = fork()

        if (p->pid == 0){
            *p->name = argv[i];
            //Free hashtable memory for the copy of the child process
            free(hash);
            break;
        }

        else {
            //Add child PID to a Hashtable to track the list of processes
            hash[i]->key = p->pid;
            *hash[i]->value = argv[i];
        }
    }

    //If child process
        //Terminate
    if (p->pid == 0) {
        //Read data from dataset n file
	    dataset = fopen(p->name,"r");

        //For loop to parse data

        //Write only sum and dif to buffer

        //Free memory
        free(p);
        //exit()
    }

    //If parent process
    else {
        //Get actual PID of the parent process (not necessary, just for accuracy)
        p->pid = getpid();

        for (int i = 0; i < argc - 1; i++) {
            //child_pid = wait()

            //Find the dataset name based on the child PID returned (linear search algorithm)
            for (int j = 0; j < argc - 1; j++) {
                if (hash[j]->key == child_pid) {
                    *child_name = hash[j]->value;
                }
            }
            //Calculate max and min (use double precision)
                //max = (sum + dif) / 2
                //min = (sum - dif) / 2
            //Print: name SUM=sum DIF=dif MIN=min MAX=max
            //printf("%s SUM=%d DIF=%d MIN=%d MAX=%d", p->name, p->sum, p->dif, p->max);
            
            if (i == 0) {
                //Set min and max values to the values from the first process to finish execution
            }

            else {
                //If child min < parent min | parent min = child min
                //p->min = 

                //If child max > parent max | parent max = child max
                //p->max = 
            }
        }
    }
        //All children finished executing
            //Output MAXIMUM and MINIMUM
        printf("MINIMUM=%d MAXIMUM=%d", p->min, p->max);

        //Free memory from the heap
        free(hash);
        free(p);

    return 0;
}