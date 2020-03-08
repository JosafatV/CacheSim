#ifndef PROCESSOR_H
#define PROCESSOR_H

class processor
{
private:
    int p_status;
    int p_l2_address;
    l1cache_t block_0;
    l1cache_t block_1;

    instr_t generate_instruction() {return instruction}
    void process_op() {}
    int read_op() {return p_search_address;}
    int write_op() {return p_search_address;}

    processor(int pid_processor);

public:
    int pid_processor;

    void set_id (int pid_processor) {}
    int get_id () {return pid_processor;}
    void toggle_processor() {}
    int l2_access() {return p_l2_address;}
};

#endif