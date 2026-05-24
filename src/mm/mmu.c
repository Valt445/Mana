#include "mmu.h"
#include "kmalloc.h"
#include <stdint.h>
#include "uart.h"

uint64_t* L0;                       // global, used by assembly
extern void enable_mmu();

void init_mmu(uint64_t* bitmap)
{
    // Allocate four page tables (each 4 KiB, 512 entries)
    L0         = (uint64_t*)allocate_page(bitmap, 1024);
    uint64_t* L1 = (uint64_t*)allocate_page(bitmap, 1024);
    uint64_t* L2_low  = (uint64_t*)allocate_page(bitmap, 1024);
    uint64_t* L2_high = (uint64_t*)allocate_page(bitmap, 1024);

    // Zero all entries (they are invalid by default)
    for (int i = 0; i < 512; i++) {
        L0[i] = 0;
        L1[i] = 0;
        L2_low[i] = 0;
        L2_high[i] = 0;
    }

    // Link tables: L0 → L1 → L2_low / L2_high
    L0[0] = (uintptr_t)L1 | 0x3;            // valid table
    L1[0] = (uintptr_t)L2_low  | 0x3;       // 0 – 1 GB
    L1[1] = (uintptr_t)L2_high | 0x3;       // 1 – 2 GB

    // ---- Low region: map only the device range (0x08000000 - 0x0A1FFFFF) ----
    // We map the 2 MB blocks that contain actual hardware.
    for (int i = 64; i < 82; i++) {         // 64 * 2MB = 0x08000000, 82 * 2MB = 0x0A400000
        uintptr_t addr = i * (2 * 1024 * 1024);
        if (addr >= 0x08000000 && addr <= 0x0A000000) {
            uint64_t entry = addr | 0x1 | 0x400 | 0x300;   // block, AF, inner shareable
            entry |= (1UL << 2);                            // Device memory (AttrIdx 1)
            L2_low[i] = entry;
        }
        // else remains 0 → invalid (unmapped) → no speculative aborts
    }

    // ---- High region: map exactly the 256 MB of real RAM ----
    for (int i = 0; i < 128; i++) {         // 128 × 2MB = 256 MB
        uintptr_t addr = 0x40000000 + i * (2 * 1024 * 1024);
        uint64_t entry = addr | 0x1 | 0x400 | 0x300;       // Normal cacheable
        entry |= (0UL << 2);                                // AttrIdx 0 (RAM)
        L2_high[i] = entry;
    }
    // All remaining entries stay 0 → invalid

    // Ensure all writes are visible before turning on the MMU
    asm volatile("DSB SY");

    // Enable the MMU (assembly routine)
    enable_mmu();
}
