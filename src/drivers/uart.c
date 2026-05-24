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
      }
    } else {
      uart_putc(string[i]);
    }
  }

  va_end(args);
}
// uart_getchar: poll the UART until a character arrives
char uart_getchar(void) {
  volatile uint32_t *dr = (uint32_t *)(UART_BASE + 0x00); // DR
  volatile uint32_t *fr = (uint32_t *)(UART_BASE + 0x18); // FR
  while (*fr & (1 << 4)) {
  } // wait while RXFE (bit 4) is set
  return (char)(*dr & 0xFF);
}

// read_line: read a line of input with echo and basic editing
// buf: pointer to output buffer
// size: maximum bytes to store (including null terminator)
void read_line(char *buf, int size) {
  int i = 0;
  while (i < size - 1) {
    char c = uart_getchar();

    // Enter (carriage return or line feed) -> finish
    if (c == '\r' || c == '\n') {
      buf[i] = '\0';
      uart_putc('\r');
      uart_putc('\n');
      return;
    }
    // Backspace (ASCII 0x08 or 0x7F)
    else if (c == 0x08 || c == 0x7F) {
      if (i > 0) {
        i--;
        // Erase the character on terminal: backspace, space, backspace
        uart_putc('\b');
        uart_putc(' ');
        uart_putc('\b');
      }
    }
    // Printable characters (space to ~)
    else if (c >= ' ' && c <= '~') {
      buf[i] = c;
      i++;
      uart_putc(c); // echo
    }
    // Ignore all other control characters
  }
  // Buffer full without Enter – null-terminate anyway
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
