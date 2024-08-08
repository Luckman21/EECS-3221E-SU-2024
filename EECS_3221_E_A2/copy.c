/*
Family Name: Irshad
Given Name(s): Luqmaan
Student Number: 217222365
EECS Login ID: luq21
YorkU email address: luq21@my.yorku.ca
*/

#define TEN_MILLIS_IN_NANOS 100000000

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <semaphore.h>
#include <time.h>

/**
 * @brief A struct to represent data from the buffer.
 */
typedef struct buffer_item {
    char data;      //A byte of data from the source file
    off_t offset;   //The offset of this byte of data from the first byte in the source file
} BufferItem;

int dataset, copy;              //Create pointers for the dataset, copy and log files
FILE *logfile;
BufferItem *buffer;             //Ring Buffer
int in, out, buf_size;          //Number of producer (IN), consumer (out) threads to make and buffer size
int i_read = 0, i_write = 0;    //Integers to point to the current index of read and write
pthread_mutex_t lock, w_lock;   //Read and Write mutex locks
sem_t emp, full;                //Semaphores to indicate free space in the ring buffer (emp = empty, full)
char *arg_pointers[3];          //An array of char pointers used for strtol to convert the arg parameter from char array pointer to int
int terminate = 0;              //An int counter to count the number of in threads that have terminated.  
int sem_wait_full = 0;          //An int counter to represent a semaphore of CTs waiting on a full signal.  Once sem_wait = out, terminate all out threads.

//List all function headers at the top to remove implicit function definitions
void *producer(void *param);
void *consumer(void *param);
void transaction(int op, int t_type, int n, BufferItem *item, int index);
void produce(int tid, BufferItem *item);
void consume(int tid, BufferItem *item);
void read_byte(int tid, BufferItem *item);
void write_byte(int tid, BufferItem *item);
void nanos();

/**
 * @brief A function to modularize logfile updates.  Pass specific values to the function to determine the output of the transaction.
 * 
 * @param op <Operation> the type of operation (read_byte = 0, write_byte = 1, produce = 2, consume = 3)
 * @param t_type <thread_type> either PT (Producer IN Thread) or CT (Consumer OUT Thread)
 * @param n <n> the thread number
 * @param item used to get the <offset> the offset in the file (item->offset)
 * @param index <i> Either -1 if read_byte or write_byte, otherwise index in the buffer for produce / consume operations
 */
void transaction(int op, int t_type, int n, BufferItem *item, int index) {

    char operation[20];
    char type;

    if (op == 0)      strcpy(operation,  "read_byte");
    else if (op == 1) strcpy(operation, "write_byte");
    else if (op == 2) strcpy(operation,    "produce");
    else if (op == 3) strcpy(operation,    "consume");

    if      (t_type == 0) type = 'P';
    else if (t_type == 1) type = 'C';
    
    fprintf(logfile, "%s %cT%d O%jd B%d I%d\n", operation, type, n, (intmax_t)(item->offset), item->data, index);
}

/**
 * @brief A function that manages producing data in the global circular ring buffer.  The index at which data is produced is determined by the current write index, i_write
 * 
 * @param tid Thread ID of the thread invoking the function
 * @param item A local buffer of the thread to store data that is to be written to the circular buffer
 */
void produce(int tid, BufferItem *item) {
    
    sem_wait(&emp); //Wait until empty semaphore is acquired
    pthread_mutex_lock(&lock);  //Wait until lock is acquired before entering the critical section

    //Add item to the buffer
    buffer[i_write] = *item;    //Store the item at the current write index of the buffer
    transaction(2, 0, tid, item, i_write);  //Record produce transaction to the logfile
    i_write = (i_write + 1) % buf_size; //Increment the write pointer by 1, set to 0 by % if i_write + 1 = buf_size

    //Release lock and signal full after exiting the critical section
    pthread_mutex_unlock(&lock);
    sem_post(&full);
}

/**
 * @brief A function that manages consuming data from the global circular ring buffer.  The data consumed is determined by the global read index, i_read
 * 
 * @param tid Thread ID of the thread invoking the function
 * @param item A local buffer of the thread to copy the data from an index of the ring buffer
 */
void consume(int tid, BufferItem *item) {

    sem_wait_full++;    //An implementation of sem_get_value to count the number of threads waiting on a semaphore
    while (sem_trywait(&full) < 0) {  //Wait until full semaphore is acquired
        if (terminate >= in) {
            printf("consume CT%d all PTs terminated, pthread_exit(0)\n", tid);
            terminate++;
            pthread_exit(0);
        }
    }
    sem_wait_full--;    //Decrement the number of threads waiting by 1 since the thread is not waiting anymore

    pthread_mutex_lock(&lock);  //Wait until lock is acquired before entering the critical section

    //remove item from the buffer
    *item = buffer[i_read]; //Copy the value of the buffer at the current index
    transaction(3, 1, tid, item, i_read);   //Record consume transaction to the logfile
    i_read = (i_read + 1) % buf_size;   //Increment the read pointer by 1, set to 0 by % if i_read + 1 = buf_size

    //Release the mutex lock and signal empty after exiting the critical section
    pthread_mutex_unlock(&lock);
    sem_post(&emp);
}

/**
 * @brief A function that sets the cursor to the current read index in the file, and copies the value to a local buffer.
 * 
 * @param tid Thread ID of the thread invoking the function
 * @param item A BufferItem struct element, containing memory to store the read byte data and the current offset the byte is located at
 */
void read_byte(int tid, BufferItem *item) {

    pthread_mutex_lock(&lock);    //Acquire read lock to enter critical section

    //Try acquiring the current offset
    if ((item->offset = lseek(dataset, 0, SEEK_CUR)) < 0) {
        pthread_mutex_unlock(&lock);   //Release mutex lock due to error
        printf("Cannot seek output file.  Terminating\n");
        exit(1);    //Exit with error status 1
    }

    //Try reading the byte
    if (read(dataset, &(item->data), 1) < 1) {
        printf("read_byte PT%d EOF pthread_exit(0)\n", tid);
        pthread_mutex_unlock(&lock);  //Release mutex lock due to EOF
        terminate++;
        pthread_exit(0);    //Close thread since EOF reached
    }

    transaction(0, 0, tid, item, -1);  //Log the current transaction
    pthread_mutex_unlock(&lock);  //Release read lock upon completion of critical section
}

/**
 * @brief A function that sets the cursor position to write to the output file.  It writes the data stored in it's buffer to the output file at the specified offset.
 * 
 * @param tid Thread ID of the thread invoking the function
 * @param item A BufferItem struct element, containing the offset the data should be placed at, and the data in byte value to write at that position
 */
void write_byte(int tid, BufferItem *item) {

    pthread_mutex_lock(&lock);    //Acquire write lock to enter critical section
    
    //Try setting the current file offset
    if (lseek(copy, item->offset, SEEK_SET) < 0) {
        pthread_mutex_unlock(&lock);   //Release mutex lock due to error
        printf("Cannot seek output file.  Terminating\n");
        exit(1);    //Exit with error status 1
    }

    //Try to write the byte
    if (write(copy, &(item->data), 1) < 1) {
        pthread_mutex_unlock(&lock);    //Release mutex lock due to error
        printf("Cannot write to output file.  Terminating\n");
        exit(1);    //Exit with error status 1
    }

    transaction(1, 1, tid, item, -1);   //Log the current transaction
    pthread_mutex_unlock(&lock);  //Release write lock upon completion of critical section
}

/**
 * @brief A function to modularize random time and error handling for nanosleep. Prints out an error message and exits with status 1 if failed to sleep for the set time.
 */
void nanos() {

    struct timespec timed = {0, 0};
    timed.tv_sec = 0;
    timed.tv_nsec = rand() % TEN_MILLIS_IN_NANOS;

    if (nanosleep(&timed, NULL) < 0) {
        printf("\n --------------------------------------");
        printf("ERROR: Nanosleep failed to sleep for set time.  Terminating");
        printf("\n --------------------------------------");
        exit(1);    //Exit with error status 1
    }
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
    in =  strtol(argv[1], &arg_pointers[0], 10);
    out = strtol(argv[2], &arg_pointers[1], 10);

    pthread_t readers[in];
    pthread_t writers[out];

    //Create arrays for thread IDs
    int tid_in[in];
    int tid_out[out];

    buf_size = strtol(argv[5], &arg_pointers[2], 10); //Assign a value for buf_size

    //Error check to ensure the parameters are legal
    if (in < 1 || out < 1 || buf_size < 1) {
        printf("\n --------------------------------------\n");
        if (in < 1) printf("ERROR: At least 1 IN thread required, currently %d specified.  Terminating", in);
        else if (out < 1) printf("ERROR: At least 1 OUT thread required, currently %d specified.  Terminating", out);
        else if (buf_size < 1) printf("ERROR: Size of buffer must be at least 1, currently %d specified.  Terminating", buf_size);
        else printf("ERROR: Parameter size is not legal.  Terminating");
        printf("\n --------------------------------------\n");
        exit(1);    //Exit with error status 1
    }
    
    //Create and open source, copy and log files
    dataset = open(argv[3], O_RDONLY);
    copy = open(argv[4], O_WRONLY | O_CREAT | O_TRUNC, 0666);
    logfile = fopen(argv[6], "w");

    //Create circular buffer
    buffer = malloc(buf_size * sizeof(BufferItem));

    //Initialize mutex lock and semaphores
    if (pthread_mutex_init(&lock, NULL) != 0) {
        printf("\n --------------------------------------\n");
        printf("ERROR: Mutex lock initialization failed.  Terminating");
        printf("\n --------------------------------------\n");
        exit(1);    //Exit with error status 1
    }

    sem_init(&emp, 0, buf_size);
    sem_init(&full, 0, 0);

    //Create IN and OUT threads
    for (int i = 0; i < in; i++)  {
        tid_in[i] = i;
        pthread_create(&writers[i], NULL, producer, &tid_in[i]);
    }
    for (int i = 0; i < out; i++) {
        tid_out[i] = i;
        pthread_create(&readers[i], NULL, consumer, &tid_out[i]);
    }

    while (terminate < (in + out) && sem_wait_full < out) {}    //Loop until all threads terminate (Advanced Solution)

    //Destroy the mutex lock and semaphores
    pthread_mutex_destroy(&lock);
    sem_destroy(&emp);
    sem_destroy(&full);

    //Close source, copy and log files
    close(dataset);
    close(copy);
    fclose(logfile);

    //free dynamically allocated memory
    free(buffer);

    return 0;
}


/**
 * @brief A function used by threads to represent a producer (IN Thread).
 * 
 * @param param 
 */
void *producer(void *param) {

    int *tid = param;   //Set the thread ID of the current thread to the index passed to the thread
    BufferItem *p_data = malloc(sizeof(BufferItem));

    while (1 == 1) {
        nanos();
        read_byte(*tid, p_data);
        nanos();
        produce(*tid, p_data);
    }

    return NULL;    //Returning implicitly closes the thread
}

/**
 * @brief A function used by threads to represent a consumer (OUT Thread).
 * 
 * @param param 
 */
void *consumer(void *param) {

    int *tid = param;   //Set the thread ID of the current thread to the index passed to the thread
    BufferItem *c_data = malloc(sizeof(BufferItem));

    while (1 == 1) {
        nanos();
        consume(*tid, c_data);
        nanos();
        write_byte(*tid, c_data);
    }
    terminate++;
    printf("CT%d all PTs terminated return NULL\n", *tid);
    return NULL;    //Returning implicitly closes the thread
}
