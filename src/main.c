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

Node_t *L1a = NULL;
Node_t *L1b = NULL;
Node_t *L1c = NULL;
Node_t *L1d = NULL;
Node_t *L2a = NULL;
Node_t *L2b = NULL;
Node_t *MEM = NULL;

    
pthread_t cpu1;
pthread_t cpu2;
/*
pthread_t cpu3;
pthread_t cpu4;
pthread_t l1;
pthread_t l2;
pthread_t mem;
*/ 
pthread_mutex_t l2a_lock;
pthread_mutex_t l2b_lock;
pthread_mutex_t mem_lock;

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
        switch (cpu) {
        memory_t * searched_data;
        case 0:
            searched_data = get_at(L1a, addr%2);
            if (searched_data->status == Invalid || searched_data->dir_data != addr) {
                return check_mem(level+1, cpu, chip, addr);
            } else {
                return level; // data found
            } 
            break;
        case 1:
            searched_data = get_at(L1b, addr%2);
            if (searched_data->status == Invalid || searched_data->dir_data != addr) {
                return check_mem(level+1, cpu, chip, addr);
            } else {
                return level; // data found
            } 
            break;
        case 2:
            searched_data = get_at(L1c, addr%2);
            if (searched_data->status == Invalid || searched_data->dir_data != addr) {
                return check_mem(level+1, cpu, chip, addr);
            } else {
                return level; // data found
            } 
            break;
        case 3:
            searched_data = get_at(L1d, addr%2);
            if (searched_data->status == Invalid || searched_data->dir_data != addr) {
                return check_mem(level+1, cpu, chip, addr);
            } else {
                return level; // data found
            } 
            break;
        default:
            return check_mem(level+1, cpu, chip, addr);
            break;
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

/** After a MISS, updates the data on upper levels of memory to contain the searched-for data. Access penalties are in check_mem
 * \param level in which level the data was found
 * \param cpu which cpu core is looking for the data
 * \param chip which chip is requesting data from memory
 * \param addr which data is being looked for
  */
void mmu_read (int level, int cpu, int chip, int addr) {
    if (level == 1) {
        // Data is in L2. Read and elevate to L1
        memory_t *data_l2;
        data_l2 = (memory_t*) malloc(sizeof(memory_t));
        
        if (chip == 0) {
			data_l2 = get_at(L2a, addr%4);
        } else {
            data_l2 = get_at(L2b, addr%4);
        }

        switch (cpu) {
            case 0:
                set_at(L1a, addr%2, data_l2);
                break;
            case 1:
                set_at(L1b, addr%2, data_l2);
                break;
            case 2:
                set_at(L1c, addr%2, data_l2);
                break;
            case 3:
                set_at(L1d, addr%2, data_l2);
                break;
            default:
                break;
        }
    } else if (level == 2) {
        // Data is in MEM. Read and elevate to L2 and L1
        memory_t *data_mem;
        data_mem = (memory_t*) malloc(sizeof(memory_t));
	    data_mem = get_at(MEM, addr);

        if (chip == 0) {
            pthread_mutex_lock(&l2a_lock);
			set_at(L2a, addr%4, data_mem);
            pthread_mutex_unlock(&l2a_lock);
        } else {
            pthread_mutex_lock(&l2b_lock);
            set_at(L2b, addr%4, data_mem);
            pthread_mutex_unlock(&l2b_lock);
        }

        switch (cpu) {
            case 0:
                set_at(L1a, addr%2, data_mem);
                break;
            case 1:
                set_at(L1b, addr%2, data_mem);
                break;
            case 2:
                set_at(L1c, addr%2, data_mem);
                break;
            case 3:
                set_at(L1d, addr%2, data_mem);
                break;
            default:
                break;
        }
	} else {
		; // if (level == 0) Data is in L1. No action needed
	}
}

/** The CPU sends a write order, the data is written following the write-through protocol
 * \param cpu which cpu core is writting the data
 * \param chip to what chip does the core belong
 * \param addr the address that is being written
 * \param data the data being written
 */
void mmu_write (int cpu, int chip, int addr, int data){
	memory_t * new_data;
	new_data = (memory_t *) malloc(sizeof(memory_t));
	new_data->block = 0;		// ???
	new_data->status = Valid;	// Update
	new_data->core = cpu;
	new_data->shared = 0;		// Update
	new_data->dir_data = addr;
	new_data->data = data;    

	usleep(l1_penalty);
	switch (cpu) {
	case 0:
		set_at(L1a, addr%2, new_data);
		break;
	case 1:
		set_at(L1b, addr%2, new_data);
		break;
	case 2:
		set_at(L1c, addr%2, new_data);
		break;
	case 3:
		set_at(L1d, addr%2, new_data);
		break;
	default:
		printf("Unexped error");
		break;
	}

	// Send to BUS
    pthread_mutex_lock(&l2a_lock);
	usleep(l2_penalty);
	if (chip == 0){
		set_at(L2a, addr%4, new_data);
	} else {
		set_at(L2b, addr%4, new_data);
	}
    pthread_mutex_unlock(&l2a_lock);
	// Send to BUS

    pthread_mutex_lock(&mem_lock);
	usleep(mem_penalty);
	set_at(MEM, addr, new_data);
    pthread_mutex_unlock(&mem_lock);
}

void* processor (void* params) {
    processor_params *p = (processor_params*) params;
    int n_core = p->id;
    int n_chip = p->chip;

   instr_t* current;
   current = malloc(sizeof(instr_t));
    int total_cycles = 20;
    int cycles = total_cycles;
    while (cycles){
        // create instruction
        current->core = n_core;
        current->chip = n_chip;
        current->op = (rand() % 3);
        current->dir = (rand() % 16);
        current->data = (rand() % 1024);

        if (current->op == op_write) {
            //update_cache (current->dir);
			mmu_write (current->core, current->chip, current->dir, current->data);
            printf( "core %d, cycle %d: writting to memory\n", current->core, total_cycles-cycles);
			printf(" └─> data written to %d\n", current->dir);
        }
        else if (current->op == op_read) {
            // verify_cache (current->dir);
            int location = check_mem(0, current->core, current->chip, current->dir);
            printf("core %d, cycle %d: reading %d from memory\n", current->core, total_cycles-cycles, current->dir);
            printf(" └─> data found on level %d\n", location);
            mmu_read(location, current->core, current->chip, current->dir); // take data form lower levels to higher levels (update cache)
        } else { //(current->op == op_process)
            printf( "core %d, cycle %d: processing\n", current->core, total_cycles-cycles);
            usleep (proc_time);
        }
        cycles--;
    }
}


int main () {
    printf( " =============== POST ===============\n");
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
        l2block->dir_data = -1;
        l2block->data = 0;
        
        push_back(&L2a, l2block);
    }

    // initialize l1 cache blocks 
    for (int i = 0; i < 2; i++) {
        memory_t *l1block;
        l1block = malloc(sizeof(memory_t));
        l1block->block = Invalid;
        l1block->core = 0;
        l1block->dir_data = -1;
        l1block->data = 0;
        
        push_back(&L1a, l1block);
    }

    for (int i = 0; i < 2; i++) {
        memory_t *l1block;
        l1block = malloc(sizeof(memory_t));
        l1block->block = Invalid;
        l1block->core = 0;
        l1block->dir_data = -1;
        l1block->data = 0;
        
        push_back(&L1b, l1block);
    }

    processor_params *proc1;
    proc1 = malloc(sizeof(processor_params));
    proc1->id = 0;
    proc1->chip = 0;

    processor_params *proc2;
    proc2 = malloc(sizeof(processor_params));
    proc2->id = 1;
    proc2->chip = 0;

    // =============== SIMULATION EXECUTION ===============

    printf(" =============== Starting simulation =============== \n");
        
    int ret1 = pthread_create (&cpu1, NULL, processor, proc1);
    if(ret1) {
        printf("Error creating core 0: = %d\n", ret1);
    }

    int ret2 = pthread_create (&cpu2, NULL, processor, proc2);
    if(ret2) {
        printf("Error creating core 1: = %d\n", ret2);
    }

    pthread_join(cpu1, NULL);
    pthread_join(cpu2, NULL);

    print_mem(MEM, 4);
    print_mem(L2a, 4);
    printf("L1a: "); print_mem(L1a, 4);
    printf("L1b: "); print_mem(L1b, 4);

    return 0;
}

