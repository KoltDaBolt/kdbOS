#include "paging.h"
#include "pmm.h"
#include "kmemory.h"
#include "panic.h"

static VirtualAddressSpace* kernel_directory = NULL;
static PageTable* kernel_page_table = NULL;

extern uint32_t _text_start;
extern uint32_t _rodata_start;
extern uint32_t _data_start;

void paging_init(void) {
    uint32_t text_start = (uint32_t)&_text_start;
    uint32_t rodata_start = (uint32_t)&_rodata_start;
    uint32_t data_start = (uint32_t)&_data_start;

    uint32_t kernel_directory_frame_addr = pmm_allocate_frame();
    uint32_t kernel_page_table_frame_addr = pmm_allocate_frame();
    uint32_t second_kernel_table_frame_addr = pmm_allocate_frame();

    kernel_directory = (VirtualAddressSpace*)kernel_directory_frame_addr;
    kernel_page_table = (PageTable*)kernel_page_table_frame_addr;

    PageTable* second_page_table = (PageTable*)second_kernel_table_frame_addr;

    kmemset(kernel_directory, 0, sizeof(VirtualAddressSpace));
    kmemset(kernel_page_table, 0, sizeof(PageTable));
    kmemset(second_page_table, 0, sizeof(PageTable));

    for (uint32_t i = 0; i < TOTAL_ENTRIES; i++) {
        uint32_t physical_address = i * PAGE_SIZE;

        kernel_page_table->pages[i].present = 1;
        kernel_page_table->pages[i].user = 0;

        uint32_t is_writeable = 1;
        if ((physical_address >= text_start && physical_address < rodata_start) ||
            (physical_address >= rodata_start && physical_address < data_start)) {
                is_writeable = 0;
            }

        kernel_page_table->pages[i].rw = is_writeable;
        kernel_page_table->pages[i].frame_addr = physical_address >> 12;
    }

    kernel_page_table->pages[0].present = 0;

    for (uint32_t i = 0; i < TOTAL_ENTRIES; i++) {
        uint32_t physical_address = (4 * 1024 * 1024) + (i * PAGE_SIZE);

        second_page_table->pages[i].present = 1;
        second_page_table->pages[i].rw = 1;
        second_page_table->pages[i].user = 0;
        second_page_table->pages[i].frame_addr = physical_address >> 12;
    }

    kernel_directory->directory_entries[0].present = 1;
    kernel_directory->directory_entries[0].rw = 1;
    kernel_directory->directory_entries[0].user = 0;
    kernel_directory->directory_entries[0].page_table_addr = kernel_page_table_frame_addr >> 12;

    kernel_directory->directory_entries[1].present = 1;
    kernel_directory->directory_entries[1].rw = 1;
    kernel_directory->directory_entries[1].user = 0;
    kernel_directory->directory_entries[1].page_table_addr = second_kernel_table_frame_addr >> 12;

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
    kernel_panic(regs); // TODO: DON'T KERNEL PANIC, KILL THE ILLEGAL PROCESS
}