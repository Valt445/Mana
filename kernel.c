#include "kmalloc.h"
#include "uart.h"
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

void kmain(void)
{
    // 1. Get your bitmap pointer and wipe it to 0xFF like you did before
    uint8_t* bitmap = (uint8_t*)kmalloc(8192);
    for(int i = 0; i < 8192; i++) bitmap[i] = 0xFF;

    // 2. Initialize your free space (Let's say from current heap up to 256MB limit)
    uintptr_t free_mem_start = (uintptr_t)bitmap + 8192; 
    uintptr_t free_mem_end   = &_heap_start + (256 * 1024 * 1024); // 256MB boundary
    
    flip_bit(bitmap, free_mem_start, free_mem_end);
    uart_printf("PMM Initialized.\n");

    // 3. Test Allocation 1
    void* page1 = allocate_page(bitmap);
    uart_printf("First allocation page address: 0x%x\n", page1);

    // 4. Test Freeing
    free_bit(bitmap, (uintptr_t)page1);
    uart_printf("Freed first page.\n");

    // 5. Test Allocation 2 (The Moment of Truth)
    void* page2 = allocate_page(bitmap);
    uart_printf("Second allocation page address: 0x%x\n", page2);

    if (page1 == page2) {
        uart_printf("SUCCESS: Memory manager successfully recycled the page!\n");
    } else {
        uart_printf("FAIL: Memory manager leaked or missed the free bit.\n");
    }

    while(1);
}
