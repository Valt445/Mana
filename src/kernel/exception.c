#include "exception.h"
#include "board.h"
#include "gic.h"
#include "psw.h"
#include "uart.h"
// borrowed from    https://github.com/NienfengYao/armv8-bare-metal

extern void timer_handler(void);

void handle_exception(exception_frame *exc) {
  uart_printf("An exception occur:\n");
  uart_printf("exc_type: 0x%x\n", exc->exc_type);

  // Group system registers
  uart_printf("ESR: 0x%x  SP: 0x%x  ELR: 0x%x  SPSR: 0x%x\n", exc->exc_esr,
              exc->exc_sp, exc->exc_elr, exc->exc_spsr);

  // Group general purpose registers (4 per line)
  uart_printf(" x0: 0x%x   x1: 0x%x   x2: 0x%x   x3: 0x%x\n", exc->x0, exc->x1,
              exc->x2, exc->x3);
  uart_printf(" x4: 0x%x   x5: 0x%x   x6: 0x%x   x7: 0x%x\n", exc->x4, exc->x5,
              exc->x6, exc->x7);
  uart_printf(" x8: 0x%x   x9: 0x%x  x10: 0x%x  x11: 0x%x\n", exc->x8, exc->x9,
              exc->x10, exc->x11);
  uart_printf("x12: 0x%x  x13: 0x%x  x14: 0x%x  x15: 0x%x\n", exc->x12,
              exc->x13, exc->x14, exc->x15);
  uart_printf("x16: 0x%x  x17: 0x%x  x18: 0x%x  x19: 0x%x\n", exc->x16,
              exc->x17, exc->x18, exc->x19);
  uart_printf("x20: 0x%x  x21: 0x%x  x22: 0x%x  x23: 0x%x\n", exc->x20,
              exc->x21, exc->x22, exc->x23);
  uart_printf("x24: 0x%x  x25: 0x%x  x26: 0x%x  x27: 0x%x\n", exc->x24,
              exc->x25, exc->x26, exc->x27);
  uart_printf("x28: 0x%x  x29: 0x%x  x30: 0x%x\n", exc->x28, exc->x29,
              exc->x30);
}
void irq_handle(exception_frame *exc) {
  psw_t psw;
  irq_no irq;
  int rc;

  psw_disable_and_save_interrupt(&psw);
  rc = gic_v3_find_pending_irq(exc, &irq);
  if (rc != IRQ_FOUND) {
    uart_puts("IRQ not found!\n");
    goto restore_irq_out;
  } else {
    uart_printf("IRQ found: %x", irq);

    uart_puts("\n");
  }
  gicd_disable_int(irq); /* Mask this irq */
  gic_v3_eoi(irq);       /* Send EOI for this irq line */
  timer_handler();
  gicd_enable_int(irq); /* unmask this irq line */

restore_irq_out:
  psw_restore_interrupt(&psw);
}

// ARM64 Exception Class for a 64-bit SVC call
#define ESR_EC_SVC64 0x15

void common_trap_handler(struct _exception_frame *exc) {
  uint32_t type = exc->exc_type & 0xff;

  // 1. Handle Synchronous Exceptions (Crashes and SVCs)
  if (type == AARCH64_EXC_SYNC_SPX || type == AARCH64_EXC_SYNC_SP0) {

    uint32_t ec = (exc->exc_esr >> 26) & 0x3F;

    if (ec == ESR_EC_SVC64) {
      uint32_t svc_num = exc->exc_esr & 0xFFFF;
      uart_printf("\n[Kernel] SVC System Call detected! Number: 0x%x\n",
                  svc_num);
      exc->exc_elr += 4;
    } else {
      uart_printf("\n[Kernel PANIC] Unhandled Synchronous Exception!\n");
      uart_printf("ESR: 0x%x (EC: 0x%x)\n", exc->exc_esr, ec);
      uart_printf("ELR: 0x%x (Address of crash)\n", exc->exc_elr);
      while (1)
        ;
    }
  }

  else if (type == AARCH64_EXC_IRQ_SPX || type == AARCH64_EXC_IRQ_SP0) {

    irq_no irq;
    if (gic_v3_find_pending_irq(exc, &irq) == IRQ_FOUND) {

      if (irq == TIMER_IRQ) {
        timer_handler();
      } else {
        uart_printf("Unknown IRQ fired: %d\n", irq);
        gic_v3_eoi(irq);
      }
    }
  }

  else {
    uart_printf("\n[Kernel PANIC] Unknown Exception Type: 0x%x\n", type);
    while (1)
      ;
  }
}
