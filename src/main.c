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
#define proc_time 10000
#define l1_penalty 20000
#define l2_penalty 50000
#define mem_penalty 100000

volatile int bus_mem = 0;

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

typedef struct {
    int active;
    int operation; // read or write
    memory_t data;
} bus_t;

Node_t *MEM = NULL; 
Node_t *L1a = NULL;
Node_t *L1b = NULL;
Node_t *L1c = NULL;
Node_t *L1d = NULL;
Node_t *L2a = NULL;
Node_t *L2b = NULL;

/**  finds a data object in memory, simulates memory access penalty
 * \param level the level of memory is being accessed: 0==L1, 1==L2 & 3==MEM
 * \param cpu which processor is doing the search (especifies which L1 cache to search)
 * \param chip which processor is doing the search (especifies which L2 cache to search)
 * \param addr which memory we are looking for
 * \return At what level was the data found
 */ 

int check_mem(int level, int cpu, int chip, int addr) {
    if (level == 0) { // check L1
        usleep(l1_penalty);
        if (cpu == 0){
            memory_t * searched_data = get_at(L1a, addr%2);
            if (searched_data->status == Invalid || searched_data->dir_data != addr) {
                return check_mem(level+1, cpu, chip, addr);
            } else {
                return level; // data found
            }      
            /*
        } else if (cpu == 1) {
            memory_t * searched_data = get_at(L1b, addr);
        } else if (cpu == 2) {
            memory_t * searched_data = get_at(L1c, addr);
        } else {
            memory_t * searched_data = get_at(L1d, addr);*/
        }
    } else if (level == 1) { // check L2
        usleep(l2_penalty);
        if (chip == 0) {
            // mutex
            memory_t * searched_data = get_at(L2a, addr%4);
            // unlock
            if (searched_data->status == Invalid || searched_data->dir_data != addr) { // data is invalid or its not it
                return check_mem(level+1, cpu, chip, addr%4);
            } else {
                return level;
            }
        } else {
            // mutex
            memory_t * searched_data = get_at(L2b, addr%4);
            // unlock
            if (searched_data->status == Invalid) {
                return check_mem(level+1, cpu, chip, addr%4);
            } else {
                return level;
            }
        }
    } else { // check MEM
        usleep(mem_penalty);
        return level;
    }
}

/** After a MISS, updates the data on upper levels of memory to contain the searched-for data
 * \param level in which level the data was found
 * \param cpu which cpu core is looking for the data
 * \param chip which chip is requesting data from memory
 * \param addr which data is being looked for
  */
void mmu_read (int level, int cpu, int chip, int addr) {
    if (level == 0) {
        exit(EXIT_SUCCESS);
    } else if (level == 1) {
        // data is valid and in L2 cache
        memory_t *data_l2;
        data_l2 = malloc(sizeof(memory_t));
		// update core (owner)?
        // Specify from which l2 to which l1
        if (chip == 0) {
			data_l2 = get_at(L2a, addr%4);
            if(cpu == 0){
				set_at(L1a, addr%2, data_l2);
            } else {
                set_at(L1b, addr%2, data_l2);
            }
        } else {
			data_l2 = get_at(L2b, addr%4);
            if(cpu == 0){
				set_at(L1c, addr%2, data_l2);
            } else {
                set_at(L1d, addr%2, data_l2);
            }
		}

    } else {
		
			memory_t *data_mem;
			data_mem = malloc(sizeof(memory_t));
			data_mem = get_at(MEM, addr);
		if (chip == 0) {
			data_mem->block == 0;
			data_mem->shared == 0;
			set_at(L2a, addr%4, data_mem);
			if (cpu = 0){
				set_at(L1a, addr%2, data_mem);
			} else {
				set_at(L1b, addr%2, data_mem);
			}
		}
    }
}

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

		// print_instr()

        cycles--;
        if (current->op == op_write) {
            //update_cache (current->dir);
            printf( "writting to memory\n");
            usleep (proc_time);
        }
        else if (current->op == op_read) {
            // verify_cache (current->dir);
            printf( "reading from memory\n");
            int location = check_mem(0, current->core, current->chip, current->dir);
            printf(" └─> data found on level %d\n", location);
            mmu_read(location, current->core, current->chip, current->dir); // take data form lower levels to higher levels (update cache)
        } else { //(current->op == op_process)
            //check_l1 (current->dir);
            printf( "processing\n");
            usleep (proc_time);
        }
    }
}


int main () {
    printf( "POST...\n");
    srand(time(0));

    // initialize memory
    for (int i = 0; i < 16; i++) {
        memory_t *memblock;
        memblock = malloc(sizeof(memory_t));
        memblock->status = Valid;
        memblock->core = 0;
        memblock->dir_data = i;
        memblock->data = rand()%256;
        
        push_back(&MEM, memblock);
    }


    // initialize l2 cache blocks 
    for (int i = 0; i < 4; i++) {
        memory_t *l2block;
        l2block = malloc(sizeof(memory_t));
        l2block->block = 0;
        l2block->status = Invalid;
        l2block->core = 0;
        l2block->shared = 0;
        l2block->dir_data = i;
        l2block->data = 0;
        
        push_back(&L2a, l2block);
    }

    // initialize l1 cache blocks 
    for (int i = 0; i < 2; i++) {
        memory_t *l1block;
        l1block = malloc(sizeof(memory_t));
        l1block->block = Invalid;
        l1block->core = 0;
        l1block->dir_data = i;
        l1block->data = 0;
        
        push_back(&L1a, l1block);
    }

    for (int i = 0; i < 2; i++) {
        memory_t *l1block;
        l1block = malloc(sizeof(memory_t));
        l1block->block = Invalid;
        l1block->core = 0;
        l1block->dir_data = i;
        l1block->data = 0;
        
        push_back(&L1b, l1block);
    }

    print_mem(MEM, 4);
    print_mem(L2a, 4);
    print_mem(L1a, 4);
    
    

    /*
    pthread_t cpu1;
    pthread_t cpu2;
    pthread_t cpu3;
    pthread_t cpu4;
    pthread_t l1;
    pthread_t l2;
    pthread_t mem;
    */  

    processor_params *proc1;
    proc1 = malloc(sizeof(processor_params));
    proc1->id = 0;
    proc1->chip = 0;
        
    printf("starting cpu 1\n");
    processor ((void*) proc1);


    print_mem(MEM, 4);
    print_mem(L2a, 4);
    print_mem(L1a, 4);

 
    return 0;
}

