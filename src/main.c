#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include "../include/linked_list.h"


#define op_process 0
#define op_read 1
#define op_write 2
#define half_second 500000

volatile int bus_mem = 0;

typedef struct {
    int owner;
    int address;
    int status;
    int data;
} memory_t;

typedef struct {
    int core;
    int chip;
    int op;
    int dir;
    int data;
} instr_t;

typedef struct {
    int id;
    int chip;
} processor_params;



void processor (void* params) {

    processor_params *p = (processor_params*) params;
    int n_core = p->id;
    int n_chip = p->chip;

   instr_t* current;
   current = malloc(sizeof(instr_t));

    int cycles = 20;
    while (cycles){

        // create instruction
        current->core = n_core;
        current->chip = n_chip;
        current->op = (rand() % 3);
        current->dir = (rand() % 16);
        current->data = (rand() % 1024);

        cycles--;
        if (current->op == op_write) {
            //update_cache (current->dir);
            printf( "writting to memory\n");
            usleep (half_second);
        }
        else if (current->op == op_read) {
            // verify_cache (current->dir);
            printf( "reading from memory\n");
            usleep (half_second);
        } else { //(current->op == op_process)
            //check_l1 (current->dir);
            printf( "processing\n");
            usleep (half_second);
        }
    }
}

int main () {
    printf( "POST...\n");
    srand(time(0));

    processor_params *proc1;
    proc1 = malloc(sizeof(processor_params));
    proc1->id = 0;
    proc1->chip = 0;
        
    printf("starting cpu 1\n");

    processor ((void*) proc1);

    /*
    pthread_t cpu1;
    pthread_t cpu2;
    pthread_t cpu3;
    pthread_t cpu4;
    pthread_t l1;
    pthread_t l2;
    pthread_t mem;
    */   
    return 0;
}

