#ifndef UART_H
#define UART_H

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#define UART_BASE 0x09000000
#define UART_PTR ((volatile unsigned char *)UART_BASE)

enum UART_REGS {
  UART_DR = 0x00,
  UART_FR = 0x18,
};

// const uint32_t UART_FR_RXFE = (1 << 4);

void uart_putc(char a);
void uart_puts(char *string);
void uart_putd(int n);
void uart_printf(char *string, ...);
char uart_getchar();
void read_line(char *buf, int size);
int strcmp(const char *a, const char *b);
int strncmp(const char *a, const char *b, int n);
#endif
