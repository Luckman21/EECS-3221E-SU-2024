/*
Family Name: Irshad
Given Name(s): Luqmaan
Student Number: 217222365
EECS Login ID: luq21
YorkU email address: luq21@my.yorku.ca
*/

#define TEN_MILLIS_IN_NANOS 100000000

#include <stdio.h>
#include <pthread.h>
#include <time.h>

/**
 * @brief A struct to represent data from the buffer.
 */
typedef struct buffer_item {
    char data;      //A byte of data from the source file
    off_t offset;   //The offset of this byte of data from the first byte in the source file
} BufferItem;

FILE *dataset, *copy, *log; //Create pointers for the dataset, copy and log files
int in, out, buf_size;      //Number of producer (IN), consumer (out) threads to make and buffer size
int read, write;            //Integers to point to the current index of read and write

/**
 * @brief A function used by threads to represent a producer (IN Thread).
 * 
 * @param param 
 */
void *producer(void *param) {

    while (1 == 1) {
        //if (nanosleep(rand(2)) != 0) {
            printf("\n --------------------------------------");
            printf("| ERROR: Pipe #%d failed, terminating. |", i);
            printf("\n --------------------------------------");
            exit(1);
        //}
        //read_byte();
        //nsleep();
        //produce();
    }
}

/**
 * @brief A function used by threads to represent a consumer (OUT Thread).
 * 
 * @param param 
 */
void *consumer(void *param) {

}

/**
 * @brief The main function, responsible for the following:
 * 1. Open the source dataset file
 * 2. Initialize a circular buffer
 * 3. Create all IN and OUT threads
 * 4. Wait for all threads to finish execution
 * 
 * @param argc Not directly used, since we have a known fixed number of arguments being passed beforehand
 * @param argv Data appears in the following order: <nIN> <nOUT> <file> <copy> <bufSize> <Log>
 * @return 0
 */
int main (int argc, char *argv[]) {

    //Initialize data

    //The number of IN and OUT threads to create
    in = argv[1];
    out = argv[2];

    buf_size = argv[5]; //Assign a value for buf_size

    //Error check to ensure the parameters are legal
    if (in < 1 || out < 1 || buf_size < 1) {
        printf("\n --------------------------------------");
        if (in < 1) printf("ERROR: At least 1 IN thread required, currently %d specified.  Terminating", in);
        else if (out < 1) printf("ERROR: At least 1 OUT thread required, currently %d specified.  Terminating", out);
        else if (buf_size < 1) printf("ERROR: Size of buffer must be at least 1, currently %d specified.  Terminating", buf_size);
        else printf("ERROR: Parameter size is not legal.  Terminating");
        printf("\n --------------------------------------");
        exit(1);
    }

    //Circular buffer
    BufferItem *buffer[buf_size];
    
    //Open source, copy and log files
    dataset = fopen(argv[3], "r");
    copy = fopen(argv[4], "w");
    log = fopen(argv[6], "w");

    //Create circular buffer
    BufferItem buffer[buf_size];
    
    //Point read and write to index 0 to initialize
    read = 0;
    write = 0;

    //Create IN and OUT threads
    for (int i = 0; i < in; i++) {

    }

    for (int i = 0; i < out; i++) {

    }

    //Close source, copy and log files
    fclose(dataset);
    fclose(copy);
    fclose(log);

    return 0;
}