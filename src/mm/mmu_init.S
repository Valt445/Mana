.section .text
.global enable_mmu

enable_mmu:
    LDR x1, =0x000000000044FF00
    MSR MAIR_EL1, x1

    LDR x0, =0x0000000200003510
    MSR TCR_EL1, x0

    LDR x2, =L0
    LDR x2, [x2]
    MSR TTBR0_EL1, x2

    MRS x3, SCTLR_EL1
    ORR x3, x3, #1
    ORR x3, x3, #(1 << 2)
    ORR x3, x3, #(1 << 12)
    MSR SCTLR_EL1, x3        // ← THIS LINE MUST USE x3

    DSB SY
    ISB

    DSB SY
    TLBI VMALLE1
    DSB SY
    ISB

    RET
