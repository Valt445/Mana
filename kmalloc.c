#include "kmalloc.h"
#define ALIGN(val, alignment) (((val) + (alignment) - 1) & ~((alignment) - 1));
void* heap = (void*)&_heap_start;
 
void* kmalloc(size_t size)
{
  size_t aligned_size = ALIGN(size, 8);

  if((uintptr_t)heap + aligned_size > (uintptr_t)&_heap_max)
  {
    return NULL;
  }
  void* ptr = heap;
  heap += aligned_size;
  return ptr;
}

void* flip_bit(uint8_t* bitmap, uintptr_t free_start, uintptr_t free_end)
{
  size_t start_no = (free_start - &_heap_start) / 4096;
  size_t end_no = (free_end - &_heap_start) / 4096;

  for(int i = start_no; i < end_no; i++)
  {
    size_t byte_index = i / 8;
    size_t bit_position = i % 8;
    
    bitmap[byte_index] &= ~(1 << bit_position);
  }
}

void* free_bit(uint8_t* bitmap, uintptr_t addr)
{
  size_t page_no = (addr - &_heap_start) / 4096;
  
  size_t byte_index = page_no / 8;
  size_t bit_position = page_no % 8;
  
  bitmap[byte_index] &= ~(1 << bit_position);
  
}
