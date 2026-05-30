#include "uart.h"
#include <stdint.h>

void uart_putc(char a) { *UART_PTR = a; }

void uart_puts(char *string) {
  for (int i = 0; string[i] != '\0'; i++) {
    uart_putc(string[i]);
  }
}

void uart_putd(int n) {
  char buf[20];
  int i = 0;

  if (n < 0) {
    uart_putc('-');
    n = -n;
  }

  if (n == 0) {
    uart_putc('0');
    return;
  }

  while (n > 0) {
    buf[i++] = '0' + (n % 10);
    n /= 10;
  }

  // print in reverse
  for (int j = i - 1; j >= 0; j--)
    uart_putc(buf[j]);
}

void uart_puthex64(unsigned long long val) {
    // A 64-bit integer has 16 hex digits
    // We start shifting from bit 60 down to 0
    for (int i = 60; i >= 0; i -= 4) {
        
        unsigned int digit = (val >> i) & 0xF; 
        
        if (digit < 10) {
            uart_putc('0' + digit);
        } else {
            uart_putc('a' + digit - 10);
        }
    }
}
void uart_printf(char *string, ...) {
  va_list args;
  va_start(args, string);

  for (int i = 0; string[i] != '\0'; i++) {
    if (string[i] == '%') {
      i++;
      if (string[i] == 'd') {
        int num_arg = va_arg(args, int);
        uart_putd(num_arg);
      } else if (string[i] == 's') {
        char *char_arg = va_arg(args, char *);
        uart_puts(char_arg);
      } else if (string[i] == 'x') {
        // Extract the argument as a 64-bit unsigned int
        unsigned long long hex_arg = va_arg(args, unsigned long long);
        uart_puthex64(hex_arg);
      }
    } else {
      uart_putc(string[i]);
    }
  }

  va_end(args);
}

size_t strlen(const char *s) {
  size_t n = 0;
  while (s[n] != '\0') {
    n++;
  }
  return n;
}
// uart_getchar: poll the UART until a character arrives
char uart_getchar(void) {
  volatile uint32_t *dr = (uint32_t *)(UART_BASE + 0x00); // DR
  volatile uint32_t *fr = (uint32_t *)(UART_BASE + 0x18); // FR
  while (*fr & (1 << 4)) {
  }
  return (char)(*dr & 0xFF);
}

void read_line(char *buf, int size) {
  int i = 0;
  while (i < size - 1) {
    char c = uart_getchar();

    if (c == '\r' || c == '\n') {
      buf[i] = '\0';
      uart_putc('\r');
      uart_putc('\n');
      return;
    }

    else if (c == 0x08 || c == 0x7F) {
      if (i > 0) {
        i--;
        uart_putc('\b');
        uart_putc(' ');
        uart_putc('\b');
      }
    }

    else if (c >= ' ' && c <= '~') {
      buf[i] = c;
      i++;
      uart_putc(c); // echo
    }
  }

  buf[i] = '\0';
}

int strcmp(const char *a, const char *b) {
  while (*a && *a == *b) {
    a++;
    b++;
  }
  return *(unsigned char *)a - *(unsigned char *)b;
}

int strncmp(const char *a, const char *b, int n) {
  while (n && *a && *a == *b) {
    a++;
    b++;
    n--;
  }
  return (n == 0) ? 0 : *(unsigned char *)a - *(unsigned char *)b;
}

char *strchr(const char *s, int c) {
  while (*s != '\0') {
    if (*s == c)
      return (char *)s;
    s++;
  }
  return (c == '\0') ? (char *)s : NULL;
}

int parse_two_args(char *input, char **arg1, char **arg2) {
    while (*input == ' ') input++;
    if (*input == '\0') return 0;

    *arg1 = input;
    while (*input != ' ' && *input != '\0') input++;
    if (*input == ' ') {
        *input = '\0';
        input++;
    }
    while (*input == ' ') input++;
    if (*input == '\0') return 0;

    *arg2 = input;
    while (*input != ' ' && *input != '\0') input++;
    if (*input == ' ') *input = '\0';

    return 1;
}
