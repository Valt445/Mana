#include "uart.h"
#include <stdint.h>

// Dedicated stack for IRQ handler (used by vectors.S)
uint8_t irq_stack[4096] __attribute__((aligned(16)));

void irq_handler_c(void) {
  volatile uint32_t *gicc = (volatile uint32_t *)0x08010000;
  uint32_t iar = gicc[3];
  uint32_t irq_id = iar & 0x3FF;

  if (irq_id == 1022 || irq_id == 1023) {
    gicc[4] = iar; // EOI
    return;
  }

  if (irq_id == 30) {
    uart_puts("Timer Tick!\n");

    uint64_t freq;
    asm volatile("mrs %0, cntfrq_el0" : "=r"(freq));
    asm volatile("msr cntp_tval_el0, %0" ::"r"(freq / 10));
    asm volatile("msr cntp_ctl_el0, %0" ::"r"(1));
  }

  gicc[4] = iar; // EOI
}
