#ifndef KMALLOC_H
#define KMALLOC_H
#include <stddef.h>
#include <stdint.h>

extern char _heap_max;
extern char _heap_start;

void *kmalloc(size_t size);
void *allocate_page(uint64_t *bitmap, size_t size);
void free_bit(uint8_t *bitmap, uintptr_t addr);
void flip_bit(uint8_t *bitmap, uintptr_t free_start, uintptr_t free_end);
void *memset(void *s, int c, size_t n);
#endif
