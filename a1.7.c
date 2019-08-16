/*
    The Merge Sort to use for Operating Systems Assignment 1 2019
    written by Robert Sheehan

    Modified by: William Shin
    UPI: wshi593

    By submitting a program you are claiming that you and only you have made
    adjustments and additions to this code.
 */

#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h>
#include <sys/resource.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>

#define SIZE    2

struct block {
    int size;
    int *first;
};

long num_cores;

struct shared_data {
    int num_processes;
    pthread_mutex_t lock;
};

struct shared_data *shared;

// void print_block_data(struct block *blk) {
//     printf("size: %d address: %p\n", blk->size, blk->first);
// }

/* Combine the two halves back together. */
void merge(struct block *left, struct block *right) {
	int combined[left->size + right->size];
	int dest = 0, l = 0, r = 0;
	while (l < left->size && r < right->size) {
		if (left->first[l] < right->first[r])
			combined[dest++] = left->first[l++];
		else
			combined[dest++] = right->first[r++];
	}
	while (l < left->size)
		combined[dest++] = left->first[l++];
	while (r < right->size)
		combined[dest++] = right->first[r++];
    memmove(left->first, combined, (left->size + right->size) * sizeof(int));
}

/* Merge sort the data. */
void *merge_sort(void *data) {
    // print_block_data(my_data);
    struct block *my_data = (struct block *) data;

    if (my_data->size > 1) {
        struct block left_block;
        struct block right_block;
        left_block.size = my_data->size / 2;
        left_block.first = my_data->first;
        right_block.size = left_block.size + (my_data->size % 2);
        right_block.first = my_data->first + left_block.size;

        pthread_mutex_lock(&(shared->lock));

        if (shared->num_processes < num_cores) {
            
            shared->num_processes++;
            pthread_mutex_unlock(&(shared->lock));

            int p[2], pid;

            if (pipe(p) < 0) {
                fprintf(stderr, "Pipe failed" );
                exit(1);
            }

            pid = fork();

            if (pid < 0) { 
                fprintf(stderr, "Fork failed" );  

            } else if (pid > 0) {

                //parent process
                close(p[1]);  //close the input side
                merge_sort(&right_block);
                read(p[0], left_block.first, left_block.size * (sizeof(int)));        
                close(p[0]);
                merge(&left_block, &right_block);

            } else {

                //child process
                close(p[0]);  //close the output side
                merge_sort(&left_block);
                write(p[1], left_block.first, left_block.size * (sizeof(int)));
                close(p[1]);
                exit(0);
            }


        } else {

            pthread_mutex_unlock(&(shared->lock));
            merge_sort(&left_block);
            merge_sort(&right_block);
            merge(&left_block, &right_block);     
        }
  
    }
}

/* Check to see if the data is sorted. */
bool is_sorted(int data[], int size) {
    bool sorted = true;
    for (int i = 0; i < size - 1; i++) {
        if (data[i] > data[i + 1])
            sorted = false;
    }
    return sorted;
}

int main(int argc, char *argv[]) {

    num_cores = sysconf(_SC_NPROCESSORS_ONLN);
    printf("Number of cores: %ld\n", num_cores);

    shared = mmap(NULL, sizeof(struct shared_data), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    shared->num_processes = 1;

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);

    if (pthread_mutex_init(&(shared->lock), &attr)) {
        perror("Error creating lock \n");
    }
    
	long size;
    struct rlimit limit;    //use the rlimit struct to change the stack size 
    getrlimit(RLIMIT_STACK, &limit);
    limit.rlim_cur = 1024*1024*1024;
    setrlimit(RLIMIT_STACK, &limit);

    if (argc < 2) {
		size = SIZE;
	} else {
		size = atol(argv[1]);
	}
    struct block start_block;
    int data[size];
    start_block.size = size;
    start_block.first = data;
    for (int i = 0; i < size; i++) {
        data[i] = rand();
    }

    printf("starting---\n");
    merge_sort(&start_block);
    printf("---ending\n");
    printf(is_sorted(data, size) ? "sorted\n" : "not sorted\n");
    exit(EXIT_SUCCESS);
}