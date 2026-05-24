#include "filesystem.h"
#include "minix.h"

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
  struct super_block *sb = (struct super_block *)block;

  sb->s_ninodes = 512;
  sb->s_nzones = 16364;
  sb->s_imap_blocks = 1;
  sb->s_zmap_blocks = 2;
  sb->s_firstdatazone = 20;
  sb->s_log_zone_size = 0;
  sb->s_max_size = 0;
  sb->s_magic = 0x137f;

  blk_write(1, block);

  uint8_t imap_buf[BLOCK_SIZE] = {0};
  imap_buf[0] |= (1 << 0);
  blk_write(2, imap_buf);

  uint8_t zmap_buf_A[BLOCK_SIZE] = {0};
  uint8_t zmap_buf_B[BLOCK_SIZE] = {0};

  blk_write(3, zmap_buf_A);
  blk_write(4, zmap_buf_B);

  uint8_t inode_buf[16 * BLOCK_SIZE] = {0};
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
}
