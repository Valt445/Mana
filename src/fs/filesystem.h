#pragma once

#include "minix.h"
#include <stddef.h>
#include <stdint.h>
#define BLOCK_SIZE 1024
#define BLOCK_NUM 16384

extern uint8_t disk[16 * 1024 * 1024];

void blk_read(size_t block_num, uint8_t buffer[BLOCK_SIZE]);
void blk_write(size_t block_num, uint8_t buffer[BLOCK_SIZE]);
uint16_t alloc_inode(void);
uint16_t alloc_zone(void);
void free_inode(uint16_t inum);
void free_zone(uint16_t block_number);
void read_inode(uint16_t inum, struct inode *ip);
void write_inode(uint16_t inum, struct inode *ip);
void mkfs();
uint16_t bmap(struct inode *ip, uint16_t logical_block, int allocate);
