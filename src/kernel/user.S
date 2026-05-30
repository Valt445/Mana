.global drop_to_el0

// void drop_to_el0(uint64_t pc, uint64_t sp);
// x0 = Program Counter (Address of the user app)
// x1 = Stack Pointer (Address of the user stack)
drop_to_el0:
    // 1. Set the Exception Link Register to our program's start address
    msr elr_el1, x0

    // 2. Set the User Space (EL0) stack pointer
    msr sp_el0, x1

    // 3. Set SPSR_EL1 to 0 
    // (0b0000 means EL0t mode, with all interrupts unmasked)
    mov x2, #0
    msr spsr_el1, x2

    // 4. Blast off! The CPU reads SPSR and ELR, and downgrades to EL0.
    eret
