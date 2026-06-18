#include "paging.h"
#include "pmm.h"
#include "memory.h"
#include "panic.h"

static VirtualAddressSpace* kernel_directory = NULL;
static PageTable* kernel_page_table = NULL;

void paging_init(void) {
    uint32_t kernel_directory_frame_addr = pmm_allocate_frame();
    uint32_t kernel_page_table_frame_addr = pmm_allocate_frame();

    kernel_directory = (VirtualAddressSpace*)kernel_directory_frame_addr;
    kernel_page_table = (PageTable*)kernel_page_table_frame_addr;

    memset(kernel_directory, 0, sizeof(VirtualAddressSpace));
    memset(kernel_page_table, 0, sizeof(PageTable));

    for (uint32_t i = 0; i < TOTAL_ENTRIES; i++) {
        uint32_t physical_address = i * PAGE_SIZE;

        kernel_page_table->pages[i].present = 1;
        kernel_page_table->pages[i].rw = 1;
        kernel_page_table->pages[i].user = 0;
        kernel_page_table->pages[i].frame_addr = physical_address >> 12;
    }

    kernel_directory->directory_entries[0].present = 1;
    kernel_directory->directory_entries[0].rw = 1;
    kernel_directory->directory_entries[0].user = 0;
    kernel_directory->directory_entries[0].page_table_addr = kernel_page_table_frame_addr >> 12;

    register_cpu_handler(14, paging_page_fault_handler);

    asm volatile("mov %0, %%cr3" :: "r"(kernel_directory_frame_addr));

    uint32_t cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;
    asm volatile("mov %0, %%cr0" :: "r"(cr0));
}

void paging_switch_address_space(VirtualAddressSpace* space) {
    asm volatile("mov %0, %%cr3" :: "r"(space));
}

void paging_invalidate_page_cache(uint32_t virtual_addr) {
    asm volatile("invlpg (%0)" :: "r"(virtual_addr) : "memory");
}

void paging_page_fault_handler(CpuRegisters* regs) {
    kernel_panic(regs);
}