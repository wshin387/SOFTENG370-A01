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

#define SIZE    2

struct block {
    int size;
    int *first;
};


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
        merge_sort(&left_block);
        merge_sort(&right_block);
        merge(&left_block, &right_block);       
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
	long size;
    struct rlimit limit; //use the rlimit struct to change the stack size 
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
        data[i] = rand()/100000;
    }

    struct block left_block;
    struct block right_block;

    left_block.size = start_block.size / 2;     //set up left block
    left_block.first = start_block.first;

    right_block.size = left_block.size + (start_block.size % 2);    //set up right block
    right_block.first = start_block.first + left_block.size;
    
    int p[2], pid;

    if (pipe(p) < 0) {
        fprintf(stderr, "Pipe failed");
        return 1;
    }

    printf("starting---\n");

    pid = fork();

    if (pid < 0) { 
        fprintf(stderr, "Fork failed");  

    } else if (pid > 0) {

        //parent process
        close(p[1]);    //close the input side
        merge_sort(&right_block);
        read(p[0], left_block.first, left_block.size * (sizeof(int)));
        close(p[0]);

        merge(&left_block, &right_block);
        printf("---ending\n");
        printf(is_sorted(data, size) ? "sorted\n" : "not sorted\n");
        exit(EXIT_SUCCESS);

    } else {

        //child process
        close(p[0]);    //close the output side
        merge_sort(&left_block);
        write(p[1], left_block.first, left_block.size * (sizeof(int)));
        close(p[1]);
        return(0);
    }

}