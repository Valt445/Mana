.section .text
.global _start
_start:

ldr x0, =stack_top
mov sp, x0

bl kmain

hang:
        b hang

.section .bss
.align 16
stack_bottom:
        .space 4096
stack_top:
