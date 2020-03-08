/*


int write_op () {

    return modified_address

    }

int read_op () {
    int check = check_l1();
    if (HIT) {
        check_valid();
        if (VALID) {
            usleep();
        } else {
            l2_access();
        }
    } else {
        l2_access();
    }
}


while (true) {

    // read from data cache (not simulated)
    inst_t current;
    currernt = generate_instr()

    // process instruction
    if (op = 0){
        process_op();
    } else if () {
        read_op();
    } else if () {
        write_op();
    } else {
        exit error
    }
}

*/