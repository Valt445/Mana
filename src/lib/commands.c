#include "commands.h"
#include "filesystem.h"
#include "kmalloc.h"
#include "uart.h"
#include <stdint.h>

uint16_t cwd_inum = 1;
void dir_add_entry(struct inode *dir, uint16_t dir_inum, const char *name,
                   uint16_t inum) {
  for (uint16_t blk = 0; blk < dir->i_size / BLOCK_SIZE; blk++) {
    uint16_t block_no = bmap(dir, blk, 0);
    uint8_t block_buf[BLOCK_SIZE];
    blk_read(block_no, block_buf);
    struct dir_entry *entry = (struct dir_entry *)block_buf;

    for (int i = 0; i < 64; i++) {
      if (entry->inode == 0) {
        entry->inode = inum;
        size_t len = strlen(name);
        if (len > 14)
          len = 14;
        for (size_t j = 0; j < len; j++)
          entry->name[j] = name[j];
        for (size_t j = len; j < 14; j++)
          entry->name[j] = '\0';

        blk_write(block_no, block_buf);
        return;
      }
      entry++;
    }
  }

  uint16_t new_block = bmap(dir, dir->i_size / BLOCK_SIZE, 1);
  if (new_block == 0)
    return;

  uint8_t block_buf[BLOCK_SIZE] = {0};
  struct dir_entry *first = (struct dir_entry *)block_buf;
  first->inode = inum;
  size_t len = strlen(name);
  if (len > 14)
    len = 14;
  for (size_t j = 0; j < len; j++)
    first->name[j] = name[j];

  blk_write(new_block, block_buf);
  dir->i_size += BLOCK_SIZE;
  write_inode(dir_inum, dir);
}

uint16_t namei(const char *name) {
  struct inode root_inode;
  read_inode(cwd_inum, &root_inode);

  for (uint16_t blk = 0; blk < root_inode.i_size / BLOCK_SIZE; blk++) {
    uint16_t block_no = bmap(&root_inode, blk, 0);
    uint8_t block_buf[BLOCK_SIZE];
    blk_read(block_no, block_buf);
    struct dir_entry *entries = (struct dir_entry *)block_buf;

    for (int i = 0; i < 64; i++) {
      if (entries[i].inode == 0) {
        continue;
      }
      if (strcmp(name, entries[i].name) == 0) {
        return entries[i].inode;
      }
    }
  }
  return 0;
}

void file_write(struct inode *file, const char *buf, uint32_t count) {
  uint32_t bytes_written = 0;
  uint32_t offset = file->i_size;
  uint32_t first_blk = offset / BLOCK_SIZE;
  uint32_t last_blk = (offset + count - 1) / BLOCK_SIZE;

  for (uint32_t blk = first_blk; blk <= last_blk; blk++) {
    uint8_t buffer[1024];
    uint16_t phys_blk = bmap(file, blk, 1);
    blk_read(phys_blk, buffer);

    uint32_t block_start;
    uint32_t block_count;

    if (blk == first_blk) {
      block_start = offset % BLOCK_SIZE;
    } else {
      block_start = 0;
    }

    if (blk == last_blk) {
      block_count = ((offset + count) % BLOCK_SIZE) - block_start;
    } else {
      block_count = BLOCK_SIZE - block_start;
    }

    if (block_count == 0) {
      block_count = BLOCK_SIZE;
    }

    for (uint32_t i = 0; i < block_count; i++) {
      buffer[block_start + i] = buf[bytes_written + i];
    }

    bytes_written += block_count;

    blk_write(phys_blk, buffer);
  }

  if (offset + count > file->i_size) {
    file->i_size = offset + count;
  }
}

void cmd_ls(void) {
  struct inode root_inode;
  read_inode(cwd_inum, &root_inode);

  for (uint32_t blk = 0; blk < root_inode.i_size / BLOCK_SIZE; blk++) {
    uint16_t phys = bmap(&root_inode, blk, 0);
    if (phys == 0)
      break;

    uint8_t buffer[BLOCK_SIZE];
    blk_read(phys, buffer);
    struct dir_entry *entries = (struct dir_entry *)buffer;

    for (int i = 0; i < 64; i++) {
      if (entries[i].inode != 0) {
        uart_printf("%s ", entries[i].name);
      }
    }
  }
  uart_puts("\n");
}

void cmd_create_f(char *name) {
  uint16_t inum = alloc_inode();
  struct inode file = {0};
  file.i_mode = 0x81A4;
  file.i_nlinks = 1;
  write_inode(inum, &file);
  struct inode root;
  read_inode(cwd_inum, &root);
  dir_add_entry(&root, cwd_inum, name, inum);
  uart_puts("Created file: ");
  uart_puts(name);
  uart_puts("\n");
}

void cmd_create_d(char *name) {
  uint16_t inum = alloc_inode();
  struct inode dir = {0};
  dir.i_mode = 0x41ED;
  dir.i_nlinks = 1;
  write_inode(inum, &dir);
  struct inode root;
  read_inode(cwd_inum, &root);
  dir_add_entry(&root, cwd_inum, name, inum);
  uart_puts("Created directory: ");
  uart_puts(name);
  uart_puts("\n");
}

void cmd_show(char *name) {
  uint16_t inum = namei(name);
  if (inum == 0) {
    uart_puts("File not found.\n");
    return;
  }
  struct inode file;
  read_inode(inum, &file);
  for (uint32_t offset = 0; offset < file.i_size;) {
    uint16_t phys = bmap(&file, offset / BLOCK_SIZE, 0);
    if (phys == 0)
      break;
    uint8_t buf[BLOCK_SIZE];
    blk_read(phys, buf);
    uint32_t chunk = file.i_size - offset;
    if (chunk > BLOCK_SIZE)
      chunk = BLOCK_SIZE;
    for (uint32_t i = 0; i < chunk; i++)
      uart_putc(buf[i]);
    offset += chunk;
  }
  uart_puts("\n");
}

void cmd_echo_to_file(char *text, char *name) {
  uint16_t inum = namei(name);
  if (inum == 0) {
    inum = alloc_inode();
    struct inode file = {0};
    file.i_mode = 0x81A4;
    file.i_nlinks = 1;
    write_inode(inum, &file);
    struct inode root;
    read_inode(1, &root);
    dir_add_entry(&root, 1, name, inum);
  }
  struct inode file;
  read_inode(inum, &file);
  file_write(&file, text, strlen(text));
  write_inode(inum, &file);
  uart_puts("Written.\n");
}

void cmd_cd(char *name) {
  uint16_t inum = namei(name);
  if (inum == 0) {
    uart_puts("Directory not found.\n");
    return;
  }
  struct inode dir;
  read_inode(inum, &dir);
  if ((dir.i_mode & 0x4000) == 0) {
    uart_puts("Not a directory.\n");
    return;
  }
  cwd_inum = inum;
  uart_puts("Changed directory.\n");
}

void cmd_mv(char *src, char *dst) {
  uint16_t src_inum = namei(src);
  if (src_inum == 0) {
    uart_puts("Source not found.\n");
    return;
  }
  struct inode dir;
  read_inode(cwd_inum, &dir);
  dir_add_entry(&dir, cwd_inum, dst, src_inum);

  read_inode(cwd_inum, &dir);
  for (uint16_t blk = 0; blk < dir.i_size / BLOCK_SIZE; blk++) {
    uint16_t phys = bmap(&dir, blk, 0);
    uint8_t buf[BLOCK_SIZE];
    blk_read(phys, buf);
    struct dir_entry *entries = (struct dir_entry *)buf;
    for (int i = 0; i < 64; i++) {
      if (entries[i].inode == src_inum && strcmp(entries[i].name, src) == 0) {
        entries[i].inode = 0;
        memset(entries[i].name, 0, 14);
        blk_write(phys, buf);
        uart_puts("Moved.\n");
        return;
      }
    }
  }
}

void cmd_rm(char *name) {
  uint16_t inum = namei(name);
  if (inum == 0) {
    uart_puts("File not found.\n");
    return;
  }
  struct inode file;
  read_inode(inum, &file);
  for (uint32_t offset = 0; offset < file.i_size; offset += BLOCK_SIZE) {
    uint16_t phys = bmap(&file, offset / BLOCK_SIZE, 0);
    if (phys)
      free_zone(phys);
  }
  free_inode(inum);

  struct inode dir;
  read_inode(cwd_inum, &dir);
  for (uint16_t blk = 0; blk < dir.i_size / BLOCK_SIZE; blk++) {
    uint16_t phys = bmap(&dir, blk, 0);
    uint8_t buf[BLOCK_SIZE];
    blk_read(phys, buf);
    struct dir_entry *entries = (struct dir_entry *)buf;
    for (int i = 0; i < 64; i++) {
      if (entries[i].inode == inum && strcmp(entries[i].name, name) == 0) {
        entries[i].inode = 0;
        memset(entries[i].name, 0, 14);
        blk_write(phys, buf);
        uart_puts("Removed.\n");
        return;
      }
    }
  }
}
