#include "filesystem.h"
#include "minix.h"
#include <stdint.h>

static struct super_block sb;

uint8_t disk[16 * 1024 * 1024];

void blk_read(size_t block_num, uint8_t *buffer) {
  if (block_num >= BLOCK_NUM) {
    return;
  }
  size_t offset = block_num * BLOCK_SIZE;

  for (int i = 0; i < BLOCK_SIZE; i++) {
    buffer[i] = disk[offset + i];
  }
}

void blk_write(size_t block_num, uint8_t *buffer) {
  if (block_num >= BLOCK_NUM) {
    return;
  }
  size_t offset = block_num * BLOCK_SIZE;

  for (int i = 0; i < BLOCK_SIZE; i++) {
    disk[offset + i] = buffer[i];
  }
}

void mkfs() {
  uint8_t block[BLOCK_SIZE] = {0};
  blk_read(1, block);
  struct super_block *sba = (struct super_block *)block;

  sba->s_ninodes = 512;
  sba->s_nzones = 16364;
  sba->s_imap_blocks = 1;
  sba->s_zmap_blocks = 2;
  sba->s_firstdatazone = 20;
  sba->s_log_zone_size = 0;
  sba->s_max_size = 0;
  sba->s_magic = 0x137f;

  blk_write(1, block);

  uint8_t imap_buf[BLOCK_SIZE] = {0};
  imap_buf[0] |= (1 << 0);
  blk_write(2, imap_buf);

  uint8_t zmap_buf_A[BLOCK_SIZE] = {0};
  uint8_t zmap_buf_B[BLOCK_SIZE] = {0};

  blk_write(3, zmap_buf_A);
  blk_write(4, zmap_buf_B);

  uint8_t inode_root[BLOCK_SIZE] = {0};
  blk_read(5, inode_root);

  struct inode *root = (struct inode *)inode_root;

  root->i_mode = 0x41ED;
  root->i_uid = 0;
  root->i_gid = 0;
  root->i_size = 0;
  root->i_time = 0;
  root->i_nlinks = 1;

  blk_write(5, inode_root);

  // At the end of mkfs():
  sb.s_ninodes = 512;
  sb.s_nzones = 16364;
  sb.s_imap_blocks = 1;
  sb.s_zmap_blocks = 2;
  sb.s_firstdatazone = 20;
  sb.s_magic = 0x137F;
}

// Allocate a free inode, return its number (1-based), or 0 if full
uint16_t alloc_inode(void) {
  // Buffer large enough to hold the entire inode bitmap
  uint8_t bitmap[BLOCK_SIZE * sb.s_imap_blocks];

  // Read the inode bitmap from disk (starting at block 2)
  for (uint16_t blk = 0; blk < sb.s_imap_blocks; blk++) {
    blk_read(2 + blk, &bitmap[blk * BLOCK_SIZE]);
  }

  // Scan for a free inode (bit = 0)
  for (uint16_t ino = 0; ino < sb.s_ninodes; ino++) {
    uint16_t byte_index = ino / 8;
    uint8_t bit_pos = ino % 8;

    if (!(bitmap[byte_index] & (1 << bit_pos))) {
      // Found a free inode — mark it used (set bit to 1)
      bitmap[byte_index] |= (1 << bit_pos);

      // Write the modified block(s) back to disk
      // For simplicity, write all bitmap blocks (only one block in our case)
      for (uint16_t blk = 0; blk < sb.s_imap_blocks; blk++) {
        blk_write(2 + blk, &bitmap[blk * BLOCK_SIZE]);
      }

      // Inode numbers are 1‑based
      return ino + 1;
    }
  }

  // No free inode found
  return 0;
}

// AI function i made my own also but was verry bad compared to this so i am
// using this one only i understand it so good for me
uint16_t alloc_zone(void) {
  uint8_t bitmap[BLOCK_SIZE * sb.s_zmap_blocks];

  for (uint16_t blk = 0; blk < sb.s_zmap_blocks; blk++) {
    blk_read(3 + blk, &bitmap[blk * BLOCK_SIZE]);
  }

  for (uint16_t b = 0; b < sb.s_nzones; b++) {
    uint16_t byte_index = b / 8;
    uint8_t bit_pos = b % 8;

    if (!(bitmap[byte_index] & (1 << bit_pos))) {
      bitmap[byte_index] |= (1 << bit_pos);

      for (uint16_t blk = 0; blk < sb.s_zmap_blocks; blk++) {
        blk_write(3 + blk, &bitmap[blk * BLOCK_SIZE]);
      }

      return sb.s_firstdatazone + b;
    }
  }

  return 0; // disk full
}

void free_zone(uint16_t block_number) {
  uint16_t z = block_number - sb.s_firstdatazone;
  uint8_t bitmap[2 * BLOCK_SIZE];
  blk_read(3, &bitmap[0]);
  blk_read(4, &bitmap[BLOCK_SIZE]);

  uint16_t byte_number = z / 8;
  uint8_t bit_pos = z % 8;

  bitmap[byte_number] &= ~(1 << bit_pos);

  blk_write(3, &bitmap[0]);
  blk_write(4, &bitmap[BLOCK_SIZE]);
}

void free_inode(uint16_t inum) {
  uint16_t ino = inum - 1; // bit index
  uint8_t bitmap[BLOCK_SIZE];
  blk_read(2, bitmap);

  uint16_t byte_number = ino / 8;
  uint8_t bit_pos = ino % 8;

  // Clear the bit (set to 0)
  bitmap[byte_number] &= ~(1 << bit_pos);

  blk_write(2, bitmap);
}



  
  
  
