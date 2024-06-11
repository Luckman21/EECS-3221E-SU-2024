# EECS-3221E-SU-2024
 A repository of my assignments for EECS 3221E during the SU 2024 semester.

### Assignment 1

The goal of this assignment is to compare two methods of computation: mulitprocessing and multiprogramming.

In each program, we need to do the following:
- Create multiple processes / threads to accomplish the main computation
- For each process / thread, find the max and min values of the dataset it parses
- For each process / thread, compute the sum and difference of the max and min values of the dataset it parses
- Print out the name of the dataset, along with the min, max, sum and difference values
- Print out the overall max and min values between all of the datasets
- Use unix pipes to pass information from processes / threads to the main thread of execution

Using process.c, we accomplish these goals using multiprocessing.  We have a parent process, which creates multiple children.  We create these children using the fork() system call, which creates a child process that continues execution on the line after it's creation.  fork() returns 0 to the child process, and the PID to the parent process.  

Using thread.c, we accomplish these goals using multithreading.  We have the main process, which creates multiple threads.

Each child / thread is assigned its own dataset.  The number of children / threads created depends on the number of datasets specified in the terminal.

Once the children / threads complete their execution, they upload their information through a unix pipe, which the main process will collect.  From here, the main process will print out this information, and use it to determine the overall minimum and maximum values.