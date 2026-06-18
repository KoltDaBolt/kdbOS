#include "vmm.h"
#include "pmm.h"
#include "memory.h"

void vmm_map_page(VirtualAddressSpace* space, uint32_t virtual_addr, uint32_t physical_addr, bool is_rw, bool is_user) {
    uint32_t directory_index = (virtual_addr >> 22) & 0x3FF;
    uint32_t page_index = (virtual_addr >> 12) & 0x3FF;

    if (!space->directory_entries[directory_index].present) {
        uint32_t new_table_frame = pmm_allocate_frame();
        
        PageTable* new_table = (PageTable*)new_table_frame;
        
        memset(new_table, 0, sizeof(PageTable));

        space->directory_entries[directory_index].present = 1;
        space->directory_entries[directory_index].rw = is_rw ? 1 : 0;
        space->directory_entries[directory_index].user = is_user ? 1 : 0;
        space->directory_entries[directory_index].page_table_addr = new_table_frame >> 12;
    }

    uint32_t table_physical_address = space->directory_entries[directory_index].page_table_addr << 12;
    PageTable* table = (PageTable*)table_physical_address;

    table->pages[page_index].present = 1;
    table->pages[page_index].rw = is_rw ? 1 : 0;
    table->pages[page_index].user = is_user ? 1 : 0;
    table->pages[page_index].frame_addr = physical_addr >> 12;

    paging_invalidate_page_cache(virtual_addr);
}