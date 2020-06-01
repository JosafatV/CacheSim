#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include "../include/linked_list.h"
#include "../include/jvlogger.h"

// Configuration
#define proc_cycles 20
#define op_process 0
#define op_read 1
#define op_write 2
#define proc_time 100
#define dir_penalty 600
#define l1_penalty 400
#define l2_penalty 1200
#define mem_penalty 15000

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
    //printf("MEM:\n"); print_mem(MEM, 2);
    //printf("DIR:\n"); print_mem(DIR, 3);
    printf("L2a:\n"); print_mem(L2a, 1);
    printf("L2b:\n"); print_mem(L2b, 1);
    printf("L1a:\n"); print_mem(L1a, 0);
    printf("L1b:\n"); print_mem(L1b, 0);
    printf("L1c:\n"); print_mem(L1c, 0);
    printf("L1d:\n"); print_mem(L1d, 0);
    printf("\n");
}

/**  finds a data object in memory, memory access penalty changes to directory access mem
 * \param level the level of memory is being accessed: 0==L1, 1==L2 & 2==MEM
 * \param cpu which processor is doing the search (especifies which L1 cache to search)
 * \param chip which processor is doing the search (especifies which L2 cache to search)
 * \param addr which memory we are looking for
 * \return At what level was the data found
 */ 
int check_mem(int level, int cpu, int chip, int addr) {
    
    int addr4 = addr%4;    
    
    if (level == 0) { // check L1
        memory_t * searched_data;
		int addr2 = addr%2;
		usleep(l1_penalty);
        switch (cpu) {
        case 0:
            searched_data = get_at(L1a, addr2);
            if (searched_data->status == Invalid || searched_data->dir_data != addr) {
                return check_mem(level+1, cpu, chip, addr);
            } else {
                return level; // data found
            } 
            break;
        case 1:
            searched_data = get_at(L1b, addr2);
            if (searched_data->status == Invalid || searched_data->dir_data != addr) {
                return check_mem(level+1, cpu, chip, addr);
            } else {
                return level; // data found
            } 
            break;
        case 2:
            searched_data = get_at(L1c, addr2);
            if (searched_data->status == Invalid || searched_data->dir_data != addr) {
                return check_mem(level+1, cpu, chip, addr);
            } else {
                return level; // data found
            } 
            break;
        case 3:
            searched_data = get_at(L1d, addr2);
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
    } else if (level == 1) { // check DIR instead of L2
		int l2a_addr = addr4;
		int l2b_addr = 4+addr4;
		usleep(dir_penalty);
        if (chip == 0) {
            memory_t * searched_data = get_at(DIR, l2a_addr);
            if (searched_data->status == Invalid || searched_data->dir_data != addr) { // data is invalid or its not it
                return check_mem(level+1, cpu, chip, addr);
            } else {
                return level;
            }
        } else {
            memory_t * searched_data = get_at(DIR, l2b_addr);
            if (searched_data->status == Invalid) {
                return check_mem(level+1, cpu, chip, addr);
            } else {
                return level;
            }
        }
    } else { // check MEM
        return level;
    }
}

/** Checks the other chip to set the data in cache and update it. Monitoring algorithm with invalidation
 * \param cpu the cpu that is writting the data
 * \param chip the chip that is writting the data
 * \param addr the address that is being written
 */
void invalidation_monitor(int cpu, int chip, int addr, memory_t* new_data) {
	int addr2 = addr%2;
	// Access penalty
	usleep(l1_penalty);
	usleep(l1_penalty);
	usleep(l1_penalty);
	if (!chip) {
		if (get_at(L1c, addr2)->dir_data == addr) {
			usleep(l1_penalty);
			memory_t* new_data = get_at(L1c, addr2);
			new_data->status = Invalid;
			set_at(L1c, addr2, new_data);
			printf("+++ IM updated status of 0x%d in L1c +++\n", addr);
			logg(3,"+++ IM updated status of 0x", itoc(addr)," in L1c +++");
			//set_at(L1c, addr%2, new_data);
		}
		if (get_at(L1d, addr2)->dir_data == addr) {
			usleep(l1_penalty);
			memory_t* new_data = get_at(L1d, addr2);
			new_data->status = Invalid;
			set_at(L1d, addr2, new_data);
			printf("+++ IM updated status of 0x%d in L1d +++\n", addr);
			logg(3,"+++ IM updated status of 0x", itoc(addr)," in L1d +++");
			//set_at(L1d, addr%2, new_data);
		}
		// Update the status of the L1 that is not writting
		if (cpu) { 
			if (get_at(L1a, addr%2)->dir_data == addr) {
				usleep(l1_penalty);
				memory_t* new_data = get_at(L1a, addr2);
				new_data->status = Invalid;
				set_at(L1a, addr2, new_data);
				printf("+++ IM updated status of 0x%d in L1a +++\n", addr);
				logg(3,"+++ IM updated status of 0x", itoc(addr)," in L1a +++");
				//set_at(L1a, addr%2, new_data);
			}            
		} else {
			if (get_at(L1b, addr%2)->dir_data == addr) {
				usleep(l1_penalty);
				memory_t* new_data = get_at(L1b, addr2);
				new_data->status = Invalid;
				set_at(L1b, addr2, new_data);
				printf("+++ IM updated status of 0x%d in L1b +++\n", addr);
				logg(3,"+++ IM updated status of 0x", itoc(addr)," in L1b +++");
				//set_at(L1b, addr%2, new_data);
			}
		}
	} else {
		if (get_at(L1a, addr%2)->dir_data == addr) {
			usleep(l1_penalty);
			memory_t* new_data = get_at(L1a, addr2);
			new_data->status = Invalid;
			set_at(L1a, addr2, new_data);
			printf("+++ IM updated status of 0x%d in L1a +++\n", addr);
			logg(3,"+++ IM updated status of 0x", itoc(addr)," in L1a +++");
		}   
		if (get_at(L1b, addr%2)->dir_data == addr) {
			usleep(l1_penalty);
			memory_t* new_data = get_at(L1b, addr2);
			new_data->status = Invalid;
			set_at(L1b, addr2, new_data);
			printf("+++ IM updated status of 0x%d in L1b +++\n", addr);
			logg(3,"+++ IM updated status of 0x", itoc(addr)," in L1b +++");
		}
		// Update the status of the L1 that is not writting
		if (cpu==2) {
			if (get_at(L1d, addr%2)->dir_data == addr) {
			usleep(l1_penalty);
				memory_t* new_data = get_at(L1d, addr%2);
				new_data->status = Invalid;
				set_at(L1d, addr2, new_data);
				printf("+++ IM updated status of 0x%d in L1d +++\n", addr);
				logg(3,"+++ IM updated status of 0x", itoc(addr)," in L1d +++");
			}
		} else {
			if (get_at(L1c, addr%2)->dir_data == addr) {
			usleep(l1_penalty);
			memory_t* new_data = get_at(L1c, addr2);
			new_data->status = Invalid;
			set_at(L1c, addr2, new_data);
			printf("+++ IM updated status of 0x%d in L1c +++\n", addr);
			logg(3,"+++ IM updated status of 0x", itoc(addr)," in L1c +++");
			}
		}
	}
}


/** Updates the status in the directory after a write op has been made.
 * Abstracts L2 cache block into 1 space L2a 0-3 | L2b 4-7 and acts as the connection between chips
 * \param cpu the cpu that is writting the data
 * \param chip the chip that is writting the data
 * \param addr the address that is being modified
*/
int update_dir (int cpu, int chip, int addr) {
    int addr4 = addr%4;
    int l2a_addr = addr4;
    int l2b_addr = 4+addr4;

    usleep(dir_penalty); // get_at check

    // If L2b is updated (cpu_0 or cpu_1 write) and the data is in L2b, update status of L2b in Dir
   if (chip==1 && get_at(DIR, l2a_addr)->dir_data == addr) {
        memory_t *new_l2 = get_at(DIR, l2a_addr);
        new_l2->status = Invalid;
        set_at(DIR, l2a_addr, new_l2);
		printf("+++ UD updated status of %d in l2a +++\n", addr);
		logg(3,"+++ UD updated status of 0x", itoc(addr), " in l2a +++");
		usleep(l2_penalty);
    } else if (chip==0 && get_at(DIR, l2b_addr)->dir_data == addr) {
        memory_t *new_l2 = get_at(DIR, l2b_addr);
        new_l2->status = Invalid;
        set_at(DIR, l2b_addr, new_l2);
		printf("+++ UD updated status of %d in l2b +++\n", addr);
		logg(3,"+++ UD updated status of 0x", itoc(addr), " in l2b +++");
		usleep(l2_penalty);
    }
        
}

/** After a MISS, updates the data on upper levels of memory and dir to contain the searched-for data. Access penalties are considered
 * \param level in which level the valid data was found
 * \param cpu which cpu core is looking for the data
 * \param chip which chip is requesting data from memory
 * \param addr which data is being looked for
  */
void mmu_read (int level, int cpu, int chip, int addr) {
	int addr2 = addr%2;
	int addr4 = addr%4;
	int l2a_addr = addr4;
	int l2b_addr = 4+addr4;
	
    if (level == 1) {
        // Data is in L2. Read and elevate to L1
		//pthread_mutex_lock(&mem_lock);
        usleep(l2_penalty);
        memory_t *data_l2;
        data_l2 = (memory_t*) malloc(sizeof(memory_t));
        
        if (chip == 0) {
			data_l2 = get_at(L2a, addr4);
			data_l2->status = Valid;
			data_l2->shared = Shared;
        } else {
            data_l2 = get_at(L2b, addr4);
			data_l2->status = Valid;
			data_l2->shared = Shared;
        }

		usleep(l1_penalty);

        switch (cpu) {
            case 0:
                set_at(L1a, addr2, data_l2);
                break;
            case 1:
                set_at(L1b, addr2, data_l2);
                break;
            case 2:
                set_at(L1c, addr2, data_l2);
                break;
            case 3:
                set_at(L1d, addr2, data_l2);
                break;
            default:
                break;
        }
		//pthread_mutex_unlock(&mem_lock);
    } else if (level == 2) {
        // Data is in MEM. Read and elevate to L2 and L1. Update DIR to match L2
		//pthread_mutex_lock(&mem_lock);
        usleep(mem_penalty);
        memory_t *data_mem;
        data_mem = (memory_t*) malloc(sizeof(memory_t));
	    data_mem = get_at(MEM, addr);
		data_mem->status = Valid;
		data_mem->shared = Shared;

		usleep(l2_penalty);
		usleep(dir_penalty);
        if (chip == 0) {
			set_at(L2a, addr4, data_mem);
			set_at(DIR, l2a_addr, data_mem);
        } else {
            set_at(L2b, addr4, data_mem);
			set_at(DIR, l2b_addr, data_mem);
        }

		usleep(l1_penalty);
        switch (cpu) {
            case 0:
                set_at(L1a, addr2, data_mem);
                break;
            case 1:
                set_at(L1b, addr2, data_mem);
                break;
            case 2:
                set_at(L1c, addr2, data_mem);
                break;
            case 3:
                set_at(L1d, addr2, data_mem);
                break;
            default:
                break;
        }
		//pthread_mutex_unlock(&mem_lock);
	} else {
        // if (level == 0) Data is in L1 (Read-hit). No action needed
        usleep(l1_penalty);
	}
}

/** The CPU sends a write order, the data is written following the write-through protocol and the directory is updated
 * \param cpu which cpu core is writting the data
 * \param chip to what chip does the core belong
 * \param addr the address that is being written
 * \param data the data being written
 */
void mmu_write (int cpu, int chip, int addr, int data){
	memory_t * new_data;
	new_data = (memory_t *) malloc(sizeof(memory_t));
	new_data->block = chip;		// Confirm
	new_data->status = Valid;
	new_data->core = cpu;
	new_data->shared = Modified;
	new_data->dir_data = addr;
	new_data->data = data;

	usleep(dir_penalty);
	int addr2 = addr%2;
	int addr4 = addr%4;
	int l2a_addr = addr4;
	int l2b_addr = 4+addr4;

	// Update L1 (Write-back)
	usleep(l1_penalty);
	switch (cpu) {
	case 0:
		set_at(L1a, addr2, new_data);
		break;
	case 1:
		set_at(L1b, addr2, new_data);
		break;
	case 2:
		set_at(L1c, addr2, new_data);
		break;
	case 3:
		set_at(L1d, addr2, new_data);
		break;
	default:
		printf("Unexped error: too many cores");
		break;
	}

	//pthread_mutex_lock(&mem_lock);

	// Monitor BUS for the write
	invalidation_monitor(cpu, chip, addr, new_data);

	// Send to BUS (Write-through L2)
	usleep(l2_penalty);
	if (chip == 0) {
		set_at(L2a, addr4, new_data);
		set_at(DIR, l2a_addr, new_data);
	} else {
		set_at(L2b, addr4, new_data);
		set_at(DIR, l2b_addr, new_data);		
	}

	// Update status of other memory in directory
	update_dir(cpu, chip, addr);

	// Send to BUS (Write-through MEM)
	usleep(mem_penalty);
	set_at(MEM, addr, new_data);

	//pthread_mutex_unlock(&mem_lock);
}

/** Returns the probability of a instruction being a read instruction within a processing cycle.
 * \param i the current position in the cycle
 * \param n the total iterations of the program cycle
 * \return the percentage probability
 */
int read_p (int i, int n){
    int progress = ((100*i)/n);
    
    if (progress<20) {
        return 80;
    } else if (progress<40) {
        return 60;
    } else if (progress<60) {
        return 33;
    } else if (progress<80) {
        return 20;
    } else {
        return 10;
    }
}

/** Returns the probability of a instruction being a write instruction within a processing cycle.
 * \param i the current position in the cycle
 * \param n the total iterations of the program cycle
 * \return the percentage probability
 */
int write_p (int i, int n){
    int progress = ((100*i)/n);
    
    if (progress<20) {
        return 10;
    } else if (progress<40) {
        return 20;
    } else if (progress<60) {
        return 33;
    } else if (progress<80) {
        return 60;
    } else {
        return 80;
    }
}

/** Select an instruction by a based on 3 normal distributons. Reads are skewed left to represent
 * the program loading memory and writes are skewed right to represent the program completing operations
 * \param i the current position in the cycle
 * \param n the total iterations of the program cycle
 * \return the type of operation
 */
int select_instr (int i, int n) {
    int roll100 = (rand() % 101);
    int pread = read_p(i, n);    // probability it is a read_op
    int pwrte = write_p(i, n);   // probability it is a write_op

    //printf("p_read = %d, p_write = %d, p_proc = %d. Rolled = %d", pread, pwrte, (100-pread-pwrte), roll100);

    if (roll100<pread){
        //printf(" | chose read\n");
        return op_read;
    } else if (roll100<(pread+pwrte)) {
        //printf(" | chose write\n");
        return op_write;
    } else {
        //printf(" | chose proc\n");
        return op_process;
    }
    
}

/** Each processor is treated as it's own thread. It will create and "processs" instructions that the bus will manage
 * in order to mantiain cache coeherency
 * \param params is a processor_params struct casted to void* for use in pthread. Contains info for the specific core
 */
void* processor (void* params) {
	processor_params *p = (processor_params*) params;
	int n_core = p->id;
	int n_chip = p->chip;

	instr_t* current;
	current = malloc(sizeof(instr_t));
	int total_cycles = proc_cycles;
	int cycles = total_cycles;
	printf("+++ Starting core %d +++\n \n", n_core);
	logg(3,"+++ Starting core ", itoc(n_core)," +++");
	while (cycles){
        // create instruction
        current->core = n_core;
        current->chip = n_chip;
        current->op = select_instr(total_cycles-cycles, total_cycles);
        current->dir = (rand() % 16);
        current->data = (rand() % 1024);

        if (current->op == op_write) {     
			char buffer[128];
            pthread_mutex_lock(&mem_lock);
			sprintf(buffer, "core %d, cycle %d: writting %d to memory", current->core, total_cycles-cycles, current->data);
			printf("%s\n", buffer); logg(1, buffer);
			mmu_write(current->core, current->chip, current->dir, current->data);
			printf(" └─> data written to 0x%d\n", current->dir);
			logg(2," └─> data written to 0x", itoc(current->dir));
            print_all();
            pthread_mutex_unlock(&mem_lock);
        }
        else if (current->op == op_read) {
            int location = check_mem(0, current->core, current->chip, current->dir);
			char buffer[128];
        	pthread_mutex_lock(&mem_lock);
	        sprintf(buffer, "core %d, cycle %d: reading 0x%d from memory", current->core, total_cycles-cycles, current->dir);
			printf("%s\n", buffer); logg(1, buffer);
            printf(" └─> data found on level %d\n", location);
			logg(2," └─> data found on level ", itoc(location));
            mmu_read(location, current->core, current->chip, current->dir); // take data form lower levels to higher levels (update cache)
            print_all();
            pthread_mutex_unlock(&mem_lock);
        } else { //(current->op == op_process)
		char buffer[128];
            sprintf(buffer, "core %d, cycle %d: processing", current->core, total_cycles-cycles);
			printf("%s\n", buffer); logg(1, buffer);
            usleep (proc_time);
        }
        cycles--;
    }
}

int main () {
    printf( " =============== POST =============== \n");
    start_logg();
    srand(time(0));

    // initialize memory
    for (int i = 0; i < 16; i++) {
        memory_t *memblock;
        memblock = malloc(sizeof(memory_t));
        memblock->block = 0;
        memblock->status = Valid;
        memblock->core = 0;
        memblock->shared = Modified;
        memblock->dir_data = i;
        memblock->data = rand()%256;
        
        push_back(&MEM, memblock);
    }

        // initialize directory
    for (int i = 0; i < 8; i++) {
        memory_t *memblock;
        memblock = malloc(sizeof(memory_t));
        memblock->block = 0;
        memblock->status = Invalid;
        memblock->core = 0;
        memblock->shared = Modified;
        memblock->dir_data = -1;
        memblock->data = -1;
        
        push_back(&DIR, memblock);
    }

    // initialize l2 cache blocks 
    for (int i = 0; i < 4; i++) {
        memory_t *l2block;
        l2block = malloc(sizeof(memory_t));
        l2block->block = 0;
        l2block->status = Invalid;
        l2block->core = 0;
        l2block->shared = Modified;
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
        l2block->shared = Modified;
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
        l1block->shared = Modified;
        l1block->dir_data = -1;
        l1block->data = 0;
        
        push_back(&L1a, l1block);
    }
    for (int i = 0; i < 2; i++) {
        memory_t *l1block;
        l1block = malloc(sizeof(memory_t));
        l1block->block = 0;
        l1block->status = Invalid;
        l1block->core = 1;
        l1block->shared = Modified;
        l1block->dir_data = -1;
        l1block->data = 0;
        
        push_back(&L1b, l1block);
    } 
    for (int i = 0; i < 2; i++) {
        memory_t *l1block;
        l1block = malloc(sizeof(memory_t));
        l1block->block = 1;
        l1block->status = Invalid;
        l1block->core = 2;
        l1block->shared = Modified;
        l1block->dir_data = -1;
        l1block->data = 0;
        
        push_back(&L1c, l1block);
    }
    for (int i = 0; i < 2; i++) {
        memory_t *l1block;
        l1block = malloc(sizeof(memory_t));
        l1block->block = 1;
        l1block->status = Invalid;
        l1block->core = 3;
        l1block->shared = Modified;
        l1block->dir_data = -1;
        l1block->data = 0;
        
        push_back(&L1d, l1block);
    }

    // Initialize processor parameters
    processor_params *proc0;
    proc0 = malloc(sizeof(processor_params));
    proc0->id = 0;
    proc0->chip = 0;

    processor_params *proc1;
    proc1 = malloc(sizeof(processor_params));
    proc1->id = 1;
    proc1->chip = 0;

    processor_params *proc2;
    proc2 = malloc(sizeof(processor_params));
    proc2->id = 2;
    proc2->chip = 1;

    processor_params *proc3;
    proc3 = malloc(sizeof(processor_params));
    proc3->id = 3;
    proc3->chip = 1;
    
    print_all();

    printf(" =============== Starting simulation =============== \n \n");
    logg(1," =============== Starting simulation =============== ");

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

    printf(" +++ Final memory status +++  \n");
	printf("DIR:\n"); print_mem(DIR, 3);
	printf("MEM:\n"); print_mem(MEM, 2);
    print_all();

    return 0;
}
