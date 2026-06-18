#pragma once

#include "types.h"
#include "isr.h"

#define PAGE_SIZE 4096
#define TOTAL_ENTRIES 1024

typedef struct {
    uint32_t present            : 1;  // Bit 0: Is this page mapped to a physical frame? (0 = Not Mapped, 1 = Mapped)
    uint32_t rw                 : 1;  // Bit 1: 0 = Read-Only, 1 = Read-Write
    uint32_t user               : 1;  // Bit 2: 0 = Kernel Only, 1 = User Application
    uint32_t reserved           : 9;  // Bits 3-11: Ignored/Padded hardware bits
    uint32_t frame_addr         : 20; // Bits 12-31: The physical address of the frame in RAM (Physical Address >> 12)
} __attribute__((packed)) Page;

typedef struct {
    uint32_t present            : 1;  // Bit 0: Is this segment of the address space active? (0 = Not Active, 1 = Active)
    uint32_t rw                 : 1;  // Bit 1: 0 = Read-Only, 1 = Read-Write (permissions apply to the entire 4 MB chunk)
    uint32_t user               : 1;  // Bit 2: 0 = Kernel Only, 1 = User Application
    uint32_t reserved           : 9;  // Bits 3-11: Ignored/Padded hardware bits
    uint32_t page_table_addr    : 20; // Bits 12-31: The physical address where the PageTable sits (Physical Address >> 12)
} __attribute__((packed)) PageTableDirectoryEntry;

typedef struct {
    Page pages[TOTAL_ENTRIES];
} __attribute__((aligned(PAGE_SIZE))) PageTable;

typedef struct {
    PageTableDirectoryEntry directory_entries[TOTAL_ENTRIES];
} __attribute__((aligned(PAGE_SIZE))) VirtualAddressSpace;

void paging_init(void);
void paging_switch_address_space(VirtualAddressSpace*);
void paging_invalidate_page_cache(uint32_t);
void paging_page_fault_handler(CpuRegisters*);


// 31          22 21          12 11                            0
// ┌──────────────┬──────────────┬──────────────────────────────┐
// │  0000000001  │  0000000001  │         000000000100         │  <- Binary bits of virtual address 0x00401004
// │ (Index = 1)  │ (Index = 1)  │         (Offset = 4)         │
// └──────┬───────┴──────┬───────┴──────────────┬───────────────┘
//        │              │                      │
//        ▼              │                      │
//  1. CHOOSE ENTRY      │                      │
//   [VirtualAddressSpace]                      │
//   ┌─────────────────────────────┐            │
//   │ directory_entries[0]        │            │
//   │ directory_entries[1] ───────┼┐           │
//   │   [page_table_addr=0x3000]  ││           │
//   │ ...                         ││           │
//   │ directory_entries[1023]     ││           │
//   └─────────────────────────────┘│           │
//                                  │           │
//        ┌─────────────────────────┘           │
//        ▼                                     │
//  2. FOLLOW page_table_addr                   │
//   [PageTable @ Physical 0x3000]              │
//   ┌─────────────────────────────┐            │
//   │ pages[0]                    │            │
//   │ pages[1] ───────────────────┼┐           │
//   │   [frame_addr=0x5000]       ││           │
//   │ ...                         ││           │
//   │ pages[1023]                 ││           │
//   └─────────────────────────────┘│           │
//                                  │           │
//        ┌─────────────────────────┘           │
//        ▼                                     │
//  3. COMBINE BASE + OFFSET                    │
//   [Physical Frame @ 0x5000]                  │
//   ┌─────────────────────────────┐            │
//   │ byte[0]                     │            │
//   │ ...                         │            │
//   │ byte[4] <───────────────────┼────────────┘
//   │ ...                         │  (TARGET BYTE ACQUIRED!)
//   └─────────────────────────────┘
