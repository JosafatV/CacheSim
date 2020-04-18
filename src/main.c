#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include "../include/linked_list.h"
#include "../include/jvlogger.h"

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
Node_t *DIR = NULL;
    
pthread_t cpu1;
pthread_t cpu2;
pthread_t cpu3;
pthread_t cpu0;
pthread_t l1;
pthread_t l2;
pthread_t mem;

pthread_mutex_t mem_lock;

void print_all(){
    //print_mem(MEM, 2);
    printf("DIR:\n"); print_mem(DIR, 2);
    printf("L2a:\n"); print_mem(L2a, 1);
    printf("L2b:\n"); print_mem(L2b, 1);
    printf("L1a:\n"); print_mem(L1a, 0);
    printf("L1b:\n"); print_mem(L1b, 0);
    printf("L1c:\n"); print_mem(L1c, 0);
    printf("L1d:\n"); print_mem(L1d, 0);
    printf("\n");
}

/**  finds a data object in memory, simulates memory access penalty
 * \param level the level of memory is being accessed: 0==L1, 1==L2 & 2==MEM
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
            memory_t * searched_data = get_at(L2a, addr%4);
            if (searched_data->status == Invalid || searched_data->dir_data != addr) { // data is invalid or its not it
                return check_mem(level+1, cpu, chip, addr%4);
            } else {
                return level;
            }
        } else {
            memory_t * searched_data = get_at(L2b, addr%4);
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


/** Updates the status in the directory after a write op has been made.
 * MEM 00-15 | L2a 16-19 | L2b 20-23 | L1a 24-25 | L1b 26-27 | L1c 28-29 | L1d 30-31
 * \param cpu the cpu that is writting the data
 * \param chip the chip that is writting the data
 * \param addr the address that is being modified
*/
int update_dir (int cpu, int chip, int addr) {
    int addr2 = addr%2;
    int addr4 = addr%4;
    int l2a_addr = 16+addr4;
    int l2b_addr = 20+addr4;
    int l1a_addr = 24+addr2;
    int l1b_addr = 26+addr2;
    int l1c_addr = 28+addr2;
    int l1d_addr = 30+addr2;

    // If L2a is updated (cpu_0 or cpu_1 write) and the data is in L2b, update status in L2b
    if (chip == 0 && get_at(DIR, l2b_addr)->dir_data == addr) {
        memory_t *new_l2 = get_at(DIR, l2b_addr);
        new_l2->status = 0;
        set_at(DIR, l2b_addr, new_l2);
    } else if ( chip == 1 && get_at(DIR, l2a_addr)->dir_data == addr) {
        memory_t *new_l2 = get_at(DIR, l2a_addr);
        new_l2->status = 0;
        set_at(DIR, l2a_addr, new_l2);
    }

    // Update the other L1 cache blocks if the data is in them
    if (cpu!=0, get_at(DIR, l1a_addr)->dir_data == addr) {
        memory_t *new_l1 = get_at(DIR, l1a_addr);
        new_l1->status = 0;
        set_at(DIR, l1a_addr, new_l1);
        //set_at(L1a, addr%2, new_l1); // Update MEM
    }
    if (cpu!=1, get_at(DIR, l1b_addr)->dir_data == addr) {
        memory_t *new_l1 = get_at(DIR, l1b_addr);
        new_l1->status = 0;
        set_at(DIR, l1b_addr, new_l1);
        //set_at(L1b, addr%2, new_l1); // Update MEM
    }    
    if (cpu!=2, get_at(DIR, l1c_addr)->dir_data == addr) {
        memory_t *new_l1 = get_at(DIR, l1c_addr);
        new_l1->status = 0;
        set_at(DIR, l1c_addr, new_l1);
        //set_at(L1c, addr%2, new_l1); // Update MEM
    }
    if (cpu!=3, get_at(DIR, l1d_addr)->dir_data == addr) {
        memory_t *new_l1 = get_at(DIR, l1d_addr);
        new_l1->status = 0;
        set_at(DIR, l1d_addr, new_l1);
        //set_at(L1d, addr%2, new_l1); // Update MEM
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
			set_at(L2a, addr%4, data_mem);
        } else {
            set_at(L2b, addr%4, data_mem);
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

	// Send to BUS (Write-through L2)
    
	usleep(l2_penalty);
	if (chip == 0) {
		set_at(L2a, addr%4, new_data);
        update_dir(cpu, chip, addr);
	} else {
		set_at(L2b, addr%4, new_data);
		update_dir(cpu, chip, addr);
	}

	// Send to BUS (Write-through MEM)
	usleep(mem_penalty);
	set_at(MEM, addr, new_data);
    //pthread_mutex_unlock(&mem_lock);
}

void* processor (void* params) {
    processor_params *p = (processor_params*) params;
    int n_core = p->id;
    int n_chip = p->chip;

   instr_t* current;
   current = malloc(sizeof(instr_t));
    int total_cycles = 5;
    int cycles = total_cycles;
    printf("+++ Starting core %d +++\n \n", n_core);
    logg(3, "+++ Starting core", itoc(n_core)," +++");
    while (cycles){
        // create instruction
        current->core = n_core;
        current->chip = n_chip;
        current->op = (rand() % 3);
        current->dir = (rand() % 16);
        current->data = (rand() % 1024);

        if (current->op == op_write) {     
            pthread_mutex_lock(&mem_lock);
            printf( "core %d, cycle %d: writting %d to memory\n", current->core, total_cycles-cycles, current->data);
			printf(" └─> data written to 0x%d\n", current->dir);
			mmu_write (current->core, current->chip, current->dir, current->data);
            print_all();
            pthread_mutex_unlock(&mem_lock);
        }
        else if (current->op == op_read) {
            int location = check_mem(0, current->core, current->chip, current->dir);
            pthread_mutex_lock(&mem_lock);
            printf("core %d, cycle %d: reading 0x%d from memory\n", current->core, total_cycles-cycles, current->dir);
            printf(" └─> data found on level %d\n", location);
            mmu_read(location, current->core, current->chip, current->dir); // take data form lower levels to higher levels (update cache)
            print_all();
            pthread_mutex_unlock(&mem_lock);
        } else { //(current->op == op_process)
            printf( "core %d, cycle %d: processing\n", current->core, total_cycles-cycles);
            usleep (proc_time);
        }
        cycles--;
    }
}

int main () {
    printf( " =============== POST ===============\n");
    start_logg();
    srand(time(0));

    // initialize memory
    for (int i = 0; i < 16; i++) {
        memory_t *memblock;
        memblock = malloc(sizeof(memory_t));
        memblock->block = 0;
        memblock->status = Valid;
        memblock->core = 0;
        memblock->shared = 0;
        memblock->dir_data = i;
        memblock->data = rand()%256;
        
        push_back(&MEM, memblock);
    }

        // initialize directory
    for (int i = 0; i < 32; i++) {
        memory_t *memblock;
        memblock = malloc(sizeof(memory_t));
        memblock->block = 0;
        memblock->status = Valid;
        memblock->core = 0;
        memblock->shared = 0;
        memblock->dir_data = i;
        memblock->data = rand()%256;
        
        push_back(&DIR, memblock);
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
    for (int i = 0; i < 4; i++) {
        memory_t *l2block;
        l2block = malloc(sizeof(memory_t));
        l2block->block = 0;
        l2block->status = Invalid;
        l2block->core = 0;
        l2block->shared = 0;
        l2block->dir_data = -1;
        l2block->data = 0;
        
        push_back(&L2b, l2block);
    }

    // initialize l1 cache blocks 
    for (int i = 0; i < 2; i++) {
        memory_t *l1block;
        l1block = malloc(sizeof(memory_t));
        l1block->block = 0;
        l1block->status = Invalid;
        l1block->core = 0;
        l1block->dir_data = -1;
        l1block->data = 0;
        
        push_back(&L1a, l1block);
    }
    for (int i = 0; i < 2; i++) {
        memory_t *l1block;
        l1block = malloc(sizeof(memory_t));
        l1block->block = 1;
        l1block->status = Invalid;
        l1block->core = 1;
        l1block->shared = 0;
        l1block->dir_data = -1;
        l1block->data = 0;
        
        push_back(&L1b, l1block);
    } 
    for (int i = 0; i < 2; i++) {
        memory_t *l1block;
        l1block = malloc(sizeof(memory_t));
        l1block->block = 2;
        l1block->status = Invalid;
        l1block->core = 2;
        l1block->shared = 0;
        l1block->dir_data = -1;
        l1block->data = 0;
        
        push_back(&L1c, l1block);
    }
    for (int i = 0; i < 2; i++) {
        memory_t *l1block;
        l1block = malloc(sizeof(memory_t));
        l1block->block = 3;
        l1block->status = Invalid;
        l1block->core = 3;
        l1block->shared = 0;
        l1block->dir_data = -1;
        l1block->data = 0;
        
        push_back(&L1d, l1block);
    }

    // Initialize processor parameters
    processor_params *proc0;
    proc0 = malloc(sizeof(processor_params));
    proc0->id = 0;
    proc0->chip = 1;

    processor_params *proc1;
    proc1 = malloc(sizeof(processor_params));
    proc1->id = 1;
    proc1->chip = 0;

    processor_params *proc2;
    proc2 = malloc(sizeof(processor_params));
    proc2->id = 2;
    proc2->chip = 0;

    processor_params *proc3;
    proc3 = malloc(sizeof(processor_params));
    proc3->id = 3;
    proc3->chip = 1;
    
    print_all();

    printf(" =============== Starting simulation =============== \n \n");
    logg(1, " =============== Starting simulation =============== ");

    int ret0 = pthread_create (&cpu0, NULL, processor, proc0);
    if(ret0) { printf("Error creating core 0: = %d\n", ret0); }
        
    int ret1 = pthread_create (&cpu1, NULL, processor, proc1);
    if(ret1) { printf("Error creating core 1: = %d\n", ret1); }

    int ret2 = pthread_create (&cpu2, NULL, processor, proc2);
    if(ret2) { printf("Error creating core 2: = %d\n", ret2); }

    int ret3 = pthread_create (&cpu3, NULL, processor, proc3);
    if(ret3) { printf("Error creating core 3: = %d\n", ret3); }

    pthread_join(cpu0, NULL);
    pthread_join(cpu1, NULL);
    pthread_join(cpu2, NULL);
    pthread_join(cpu3, NULL);

    printf("  +++ Final memory status +++  \n");
    print_all();

    return 0;
}

