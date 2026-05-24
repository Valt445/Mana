#include "kmalloc.h"

#define ALIGN(val, alignment) (((val) + (alignment) - 1) & ~((alignment) - 1));
void *heap = (void *)&_heap_start;

void *kmalloc(size_t size) {
  size_t aligned_size = ALIGN(size, 8);

  if ((uintptr_t)heap + aligned_size > (uintptr_t)&_heap_max) {
    return NULL;
  }
  void *ptr = heap;
  heap = (void *)((uintptr_t)heap + aligned_size);
  return ptr;
}

void flip_bit(uint8_t *bitmap, uintptr_t free_start, uintptr_t free_end) {
  size_t start_no = (free_start - (uintptr_t)&_heap_start) / 4096;
  size_t end_no = (free_end - (uintptr_t)&_heap_start) / 4096;

  for (size_t i = start_no; i < end_no; i++) {
    size_t byte_index = i / 8;
    size_t bit_position = i % 8;

    bitmap[byte_index] &= ~(1 << bit_position);
  }
}

void free_bit(uint8_t *bitmap, uintptr_t addr) {
  size_t page_no = (addr - (uintptr_t)&_heap_start) / 4096;

  size_t byte_index = page_no / 8;
  size_t bit_position = page_no % 8;

  bitmap[byte_index] &= ~(1 << bit_position);
}

void *allocate_page(uint64_t *bitmap, size_t total_words) {
  for (size_t i = 0; i < total_words; i++) {
    if (bitmap[i] != ~0UL) {

      int bit = __builtin_ctzll(~bitmap[i]);

      bitmap[i] |= (1UL << bit);

      size_t page_num = (i * 64) + bit;
      return (void *)((uintptr_t)&_heap_start + (page_num * 4096));
    }
  }

  return NULL;
}

// Simple memset for a freestanding kernel
void *memset(void *s, int c, size_t n) {
  uint8_t *p = s;
  while (n--) {
    *p++ = (uint8_t)c;
  }
  return s;
}
