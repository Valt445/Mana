#include "filesystem.h"
#include "kmalloc.h"
#include "minix.h"
#include "mmu.h"
#include "uart.h"
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#define BITMAP_WORDS 1024
uint64_t pmm_bitmap[BITMAP_WORDS] = {0};

void cmd_mkfs() {

  mkfs();
  uint8_t block[BLOCK_SIZE] = {0};
  blk_read(1, block);
  struct super_block *sb = (struct super_block *)block;
  if (sb->s_magic == 0x137f) {
    uart_printf("HAHA i have created a shitty filesystem from the 1980's");
  }
}

void cmd_echo(char *cmd) {
  char *p = cmd + 4; // skip "echo"
  while (*p == ' ')
    p++; // skip spaces
  uart_puts(p);
  uart_putc('\n');
}

void cmd_alloc_inode() {
  uint16_t re = alloc_inode();
  if (re == 2) {
    uart_puts("IT works we allocated a inode");
  }
}

void kmain(void) {
  uart_puts("--- MMU STRESS TEST ---\n");
  uart_puts("Status: Running in Physical Mode.\n");

  // No bitmap needed – page tables are in BSS
  init_mmu(pmm_bitmap);

  uart_puts("MMU is finally working!\n");

  // optional quick test
  volatile uint32_t *p = (uint32_t *)0x41000000;
  *p = 0xBEEF;
  if (*p == 0xBEEF)
    uart_puts("RAM test passed.\n");
  else
    uart_puts("RAM test failed.\n");

  void *test_page = allocate_page(pmm_bitmap, BITMAP_WORDS);
  uart_printf("Test page: %d (should end with 000)\n", test_page);

  char cmd[256];

  while (1) {
    uart_puts("> ");
    read_line(cmd, sizeof(cmd));

    if (strncmp(cmd, "echo", 4) == 0 && (cmd[4] == ' ' || cmd[4] == '\0')) {
      cmd_echo(cmd);
    } else if (strcmp(cmd, "mkfs") == 0) {
      cmd_mkfs();
    } else if (strcmp(cmd, "hello") == 0) {
      uart_puts("Hello, OS!\n");
    } else if (strcmp(cmd, "alloc_inode") == 0) {
      cmd_alloc_inode();
    } else if (cmd[0] != '\0') {
      uart_puts("Unknown command.\n");
    }
  }
}
