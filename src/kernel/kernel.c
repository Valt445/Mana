#include "filesystem.h"
#include "kmalloc.h"
#include "minix.h"
#include "mmu.h"
#include "uart.h"
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include "commands.h"
#define BITMAP_WORDS 1024
uint64_t pmm_bitmap[BITMAP_WORDS] = {0};
struct inode root;

void cmd_mkfs() {

  mkfs();
  uint8_t block[BLOCK_SIZE] = {0};
  blk_read(1, block);
  struct super_block *sb = (struct super_block *)block;
  if (sb->s_magic == 0x137f) {
    uart_printf("HAHA i have created a shitty filesystem from the 1980's");
  }
}

void test_inode()
{
  read_inode(1, &root);
  uart_printf("Root mode: 0x%d (expected 0x41ED) \n", root.i_mode);
  uart_printf("Root size: %d (expected 0) \n", root.i_size);

  root.i_size = 128;
  write_inode(1, &root);
  
  struct inode check;
  read_inode(1, &check);
  if(check.i_size == 128)
  {
    uart_puts("THE INODE TEST PASSED I HAVE SUCCESFULLY MANAGED TO MANIPULATE A INODE \n");

  }
  else{
    uart_puts("FAILED I HAVE SUCCESFULLY MANAGED TO WASTE MY LAST 10 BRAIN CELLS");

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
    uart_puts("--- MANA OS ---\n");
    uart_puts("Status: Running in Physical Mode.\n");
    char cmd[256];
    while (1) {
        uart_puts("> ");
        read_line(cmd, sizeof(cmd));

        // ── echo (with optional redirection) ────────
        if (strncmp(cmd, "echo ", 5) == 0) {
            char *gt = strchr(cmd, '>');
            if (gt) {
                *gt = '\0';
                char *text = cmd + 5;
                while (*text == ' ') text++;
                char *name = gt + 1;
                while (*name == ' ') name++;
                cmd_echo_to_file(text, name);
            } else {
                cmd_echo(cmd);
            }
        }
        // ── ls ──────────────────────────────────────
        else if (strcmp(cmd, "ls") == 0) {
            cmd_ls();
        }
        // ── create -f <name> ────────────────────────
        else if (strncmp(cmd, "create -f ", 10) == 0) {
            cmd_create_f(cmd + 10);
        }
        // ── create -d <name> ────────────────────────
        else if (strncmp(cmd, "create -d ", 10) == 0) {
            cmd_create_d(cmd + 10);
        }
        // ── show <name> ─────────────────────────────
        else if (strncmp(cmd, "show ", 5) == 0) {
            cmd_show(cmd + 5);
        }
        // ── cd <dir> ────────────────────────────────
        else if (strncmp(cmd, "cd ", 3) == 0) {
            char *dir = cmd + 3;
            while (*dir == ' ') dir++;
            cmd_cd(dir);
        }
        // ── rm <name> ───────────────────────────────
        else if (strncmp(cmd, "rm ", 3) == 0) {
            char *name = cmd + 3;
            while (*name == ' ') name++;
            cmd_rm(name);
        }
        // ── mv <src> <dst> ──────────────────────────
        else if (strncmp(cmd, "mv ", 3) == 0) {
            char *src, *dst;
            if (parse_two_args(cmd + 3, &src, &dst))
                cmd_mv(src, dst);
            else
                uart_puts("Usage: mv <src> <dst>\n");
        }
        // ── cp <src> <dst> ──────────────────────────
        else if (strncmp(cmd, "cp ", 3) == 0) {
            char *src, *dst;
            if (parse_two_args(cmd + 3, &src, &dst))
                cmd_cp(src, dst);
            else
                uart_puts("Usage: cp <src> <dst>\n");
        }
        // ── mkfs ────────────────────────────────────
        else if (strcmp(cmd, "mkfs") == 0) {
            cmd_mkfs();
        }
        // ── hello ───────────────────────────────────
        else if (strcmp(cmd, "hello") == 0) {
            uart_puts("Hello, OS!\n");
        }
        // ── debug / test commands ──────────────────
        else if (strcmp(cmd, "alloc_inode") == 0) {
            cmd_alloc_inode();
        }
        else if (strcmp(cmd, "test_inode") == 0) {
            test_inode();
        }
       
        // ── empty input? ────────────────────────────
        else if (cmd[0] != '\0') {
            uart_puts("Unknown command.\n");
        }
    }
}
