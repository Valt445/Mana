#ifndef KMALLOC_H
#define KMALLOC_H
#include <stddef.h>
#include <stdint.h>

extern char _heap_max;
extern char _heap_start;
void *kmalloc(size_t size);

#endif
