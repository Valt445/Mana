#pragma once
#include "minix.h"
#include <stdint.h>
void dir_add_entry(struct inode *dir, uint16_t dir_inum, const char *name,
                   uint16_t inum);
uint16_t namei(const char *name);
void file_write(struct inode *file, const char *buf, uint32_t count);
void cmd_ls(void);
void cmd_create_f(char *name);
void cmd_create_d(char *name);
void cmd_show(char *name);
void cmd_echo_to_file(char *text, char *name);
void cmd_cd(char *name);
void cmd_mv(char *src, char *dst);
// void cmd_rename(char *old_name, char *new_name);
void cmd_rm(char *name);
void cmd_cp(char *src_name, char *dst_name);
void dummy_user_app(void);
void cmd_run_test(void);
