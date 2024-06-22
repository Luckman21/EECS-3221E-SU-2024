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
    //dif = min - max

//Output for each process
    //child_name SUM DIF MIN MAX
//Output at the end
    //MINIMUM MAXIMUM

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

//Define constants for pipe
//#define BUFFER_SIZE 8   //Number of bytes in a pointer (x64)
#define READ_END 0
#define WRITE_END 1

typedef struct process {
    char name[50];  //Process name (name of the dataset it's assigned to)
    int pid;        //Process ID
    double min;     //Minimum value of the dataset
    double max;     //Maximum value of the dataset
    double sum;     //SUM = Min + Max
    double dif;     //DIF = Min - Max
    int pipe;       //Index of the pipe for this process
}Process;

typedef struct HashtableNode {
    char value[50]; //Name of the dataset
    int key;        //PID
}Hashtable;

int main(int argc, char* argv[]) {

    FILE* dataset;
    int fd[argc - 1][2];    //File descriptor, number of elements depends on the number of arguments

    //Allocate heap memory for a process struct (each child process can manipulate their own copy of this struct)
    Process *p = malloc(sizeof(Process));

    //Create an array of Hashtable Node elements to store PID as the key and dataset name as the value
    Hashtable hash[argc - 1];

    /*
    Create n processes for n datasets using argc and argv
     - argc is the number of parameters in the terminal when the program is run
        - This includes the program name as the 1st element
     - argv is a "string" (char[]) array of each element in the CLI
    */
    for (int i = 1; i < argc; i++) {
        
        //Create a pipe before the fork so it is accessible to the child
        if (pipe(fd[i-1]) == -1) {
            printf("\n --------------------------------------");
            printf("| ERROR: Pipe #%d failed, terminating. |", i);
            printf("\n --------------------------------------");
            exit(1);
        }

        p->pid = fork();    //fork() a process

        //Unsuccessful process creation
        if (p->pid < 0) {
            printf("\n --------------------------------------");
            printf("| ERROR: Fork #%d failed, terminating. |", i);
            printf("\n --------------------------------------");
            exit(1);
        }

        //Child Process
        else if (p->pid == 0){
            strcpy(p->name, argv[i]);
            p->pipe = i-1;  //Pipe index from array of fd
            close(fd[i-1][READ_END]);  //Child process close the unused read end of the pipe
            break;
        }

        //Parent Process
        else {
            //In the parent process, add child PID to a Hashtable to track the list of processes
            hash[i-1].key = p->pid;
            strcpy(hash[i-1].value, argv[i]);

            close(fd[i-1][WRITE_END]);  //Parent process close the unused write end of the pipe
        }
    }

    //If child process
    if (p->pid == 0) {

        double num;
        int first = 1;

	    dataset = fopen(p->name,"r");   //Read data from dataset n file

        //For loop to parse data, runs until end of file
        while (fscanf(dataset, "%lf", &num) != EOF) {

            //Set the first max and min to actual values of the dataset instead of having them initialized with garbage values
            if (first == 1) {
                p->min = num;
                p->max = num;
                first = 0;
            }

            if (num < p->min)
                p->min = num;

            if (num > p->max)
                p->max = num;
        }

        fclose(dataset);    //Close file after using

        //Calculate sum and dif
        p->sum = p->min + p->max;
        p->dif = p->min - p->max;

        if (p->dif < 0)
            p->dif *= (double)(-1);

        //Write only sum and dif to buffer
        write(fd[p->pipe][WRITE_END], &p->sum, sizeof(p->sum));
        write(fd[p->pipe][WRITE_END], &p->dif, sizeof(p->dif));

        close(fd[p->pipe][WRITE_END]);  //Parent process close the unused read end of the pipe after extracting data

        free(p);    //Free memory
        exit(0);    //Exit with a successful status
    }

    //Parent process execution
    else {
        //Track the current child process that has finished executing
        Process *child_p = malloc(sizeof(Process));
        int status;
        
        p->pid = getpid();  //Get actual PID of the parent process (not necessary, just for accuracy)

        for (int i = 0; i < argc - 1; i++) {
            
            child_p->pid = wait(&status); //Wait for a child to finish execution

            //Find the dataset name based on the child PID returned (linear search algorithm)
            for (int j = 0; j < argc - 1; j++) {
                if (hash[j].key == child_p->pid) {
                    strcpy(child_p->name, hash[j].value);
                    read(fd[j][READ_END], &child_p->sum, sizeof(child_p->sum));
                    read(fd[j][READ_END], &child_p->dif, sizeof(child_p->dif));
                    close(fd[j][READ_END]);  //Close the unused read end of the pipe after extracting data
                }
            }
            /*Calculate max and min (use double precision)
                max = (sum + dif) / 2
                min = (sum - dif) / 2
            Print: name SUM=sum DIF=dif MIN=min MAX=max
            printf("%s SUM=%d DIF=%d MIN=%d MAX=%d", p->name, p->sum, p->dif, p->max);*/

            child_p->max = ((child_p->sum + child_p->dif) / (double)(2));
            child_p->min = ((child_p->sum - child_p->dif) / (double)(2));
            
            if (i == 0) {
                //Set min and max values to the values from the first process to finish execution
                p->min = child_p->min;
                p->max = child_p->max;
            }

            else {
                //If child min < parent min | parent min = child min
                if (child_p->min < p->min)
                    p->min = child_p->min;

                //If child max > parent max | parent max = child max
                if (child_p->max > p->max)
                    p->max = child_p->max;
            }

            //Print all the significant values for the child process
            printf("%s SUM=%lf DIF=%lf MIN=%lf MAX=%lf\n", child_p->name, child_p->sum, child_p->dif, child_p->min, child_p->max);
        }

        free(child_p);  //Free memory from the heap
    }
    /*All children finished executing
        Output MAXIMUM and MINIMUM*/
    printf("MINIMUM=%lf MAXIMUM=%lf\n", p->min, p->max);

    free(p);    //Free memory from the heap

    return 0;
}