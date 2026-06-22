#include "vmm.h"
#include "pmm.h"
#include "kmemory.h"

extern VirtualAddressSpace* kernel_directory;

#define VMM_TEMP_WINDOW_VIRTUAL  0x007FF000
#define VMM_TEMP_WINDOW_PAGE_IDX 1023
#define KERNEL_PT1_DIR_IDX       1

static PageTable* vmm_map_to_temp_window(uint32_t table_physical_addr) {
    uint32_t k_pt1_phys = kernel_directory->directory_entries[KERNEL_PT1_DIR_IDX].page_table_addr << 12;
    PageTable* kernel_pt1 = (PageTable*)k_pt1_phys;

    kernel_pt1->pages[VMM_TEMP_WINDOW_PAGE_IDX].present = 1;
    kernel_pt1->pages[VMM_TEMP_WINDOW_PAGE_IDX].rw = 1;
    kernel_pt1->pages[VMM_TEMP_WINDOW_PAGE_IDX].frame_addr = table_physical_addr >> 12;
    
    paging_invalidate_page_cache(VMM_TEMP_WINDOW_VIRTUAL);
    return (PageTable*)VMM_TEMP_WINDOW_VIRTUAL;
}

static void vmm_unmap_temp_window() {
    uint32_t k_pt1_phys = kernel_directory->directory_entries[KERNEL_PT1_DIR_IDX].page_table_addr << 12;
    PageTable* kernel_pt1 = (PageTable*)k_pt1_phys;

    kernel_pt1->pages[VMM_TEMP_WINDOW_PAGE_IDX].present = 0;
    paging_invalidate_page_cache(VMM_TEMP_WINDOW_VIRTUAL);
}

void* vmm_get_new_page(VirtualAddressSpace* space, bool is_rw, bool is_user, void* explicit_virtual) {
    uint32_t target_virtual = 0;

    if (explicit_virtual != NULL) {
        target_virtual = (uint32_t)explicit_virtual;
    } else {
        bool found_slot = false;
        for (uint32_t pd_idx = 2; pd_idx < 1024; pd_idx++) {
            if (pd_idx == 768 || pd_idx == 769) continue;

            if (!space->directory_entries[pd_idx].present) {
                target_virtual = pd_idx << 22;
                found_slot = true;
                break;
            }

            uint32_t pt_phys = space->directory_entries[pd_idx].page_table_addr << 12;
            PageTable* pt = (PageTable*)pt_phys;
            for (uint32_t pt_idx = 0; pt_idx < 1024; pt_idx++) {
                if (!pt->pages[pt_idx].present) {
                    target_virtual = (pd_idx << 22) | (pt_idx << 12);
                    found_slot = true;
                    break;
                }
            }
            if (found_slot) break;
        }
        if (!found_slot) return NULL;
    }

    uint32_t directory_index = (target_virtual >> 22) & 0x3FF;
    uint32_t page_index = (target_virtual >> 12) & 0x3FF;

    uint32_t physical_frame = pmm_allocate_frame();
    if (physical_frame == 0) return NULL;

    if (!space->directory_entries[directory_index].present) {
        uint32_t new_table_frame = pmm_allocate_frame();
        if (!new_table_frame) return NULL;
        
        space->directory_entries[directory_index].present = 1;
        space->directory_entries[directory_index].rw = is_rw;
        space->directory_entries[directory_index].user = is_user;
        space->directory_entries[directory_index].page_table_addr = new_table_frame >> 12;

        PageTable* temp_table = vmm_map_to_temp_window(new_table_frame);
        kmemset(temp_table, 0, sizeof(PageTable));
        vmm_unmap_temp_window();
    }

    uint32_t table_physical_address = space->directory_entries[directory_index].page_table_addr << 12;
    
    PageTable* table = vmm_map_to_temp_window(table_physical_address);
    table->pages[page_index].present = 1;
    table->pages[page_index].rw = is_rw;
    table->pages[page_index].user = is_user;
    table->pages[page_index].frame_addr = physical_frame >> 12;
    vmm_unmap_temp_window();

    paging_invalidate_page_cache(target_virtual);
    return (void*)target_virtual;
}

void vmm_free_page(VirtualAddressSpace* space_physical_ptr, void* virtual_address) {
    uint32_t vaddr = (uint32_t)virtual_address;
    uint32_t directory_index = (vaddr >> 22) & 0x3FF;
    uint32_t page_index = (vaddr >> 12) & 0x3FF;

    VirtualAddressSpace* space = (VirtualAddressSpace*)space_physical_ptr;

    if (!space->directory_entries[directory_index].present) {
        return;
    }

    uint32_t table_physical_address = space->directory_entries[directory_index].page_table_addr << 12;
    PageTable* table = (PageTable*)table_physical_address;

    table->pages[page_index].present = 0;
    table->pages[page_index].frame_addr = 0;

    paging_invalidate_page_cache(vaddr);
}

bool vmm_map_frame(VirtualAddressSpace* target_dir, void* virtual_addr, uint32_t physical_frame, bool is_writeable, bool is_user_process) {
    uint32_t v_addr = (uint32_t)virtual_addr;
    
    uint32_t pd_idx = v_addr >> 22;
    uint32_t pt_idx = (v_addr >> 12) & 0x3FF;

    PageTable* pt_virt = NULL;

    if (target_dir->directory_entries[pd_idx].present) {
        uint32_t pt_phys = target_dir->directory_entries[pd_idx].page_table_addr << 12;
        pt_virt = (PageTable*)pt_phys; 
    } else {
        PageTable* local_pt_window = (PageTable*)vmm_get_new_page(kernel_directory, true, false, NULL);
        if (!local_pt_window) return false;
        kmemset(local_pt_window, 0, sizeof(PageTable));

        uint32_t pt_phys = vmm_get_physical_address(kernel_directory, (void*)local_pt_window);

        target_dir->directory_entries[pd_idx].present = 1;
        target_dir->directory_entries[pd_idx].rw = 1;
        target_dir->directory_entries[pd_idx].user = is_user_process;
        target_dir->directory_entries[pd_idx].page_table_addr = pt_phys >> 12;

        pt_virt = local_pt_window;
    }

    pt_virt->pages[pt_idx].present = 1;
    pt_virt->pages[pt_idx].rw = is_writeable;
    pt_virt->pages[pt_idx].user = is_user_process;
    pt_virt->pages[pt_idx].frame_addr = physical_frame >> 12;

    paging_invalidate_page_cache(v_addr);

    return true;
}

VirtualAddressSpace* vmm_create_virtual_address_space(void* entry_point, size_t size, bool is_user_process) {
    VirtualAddressSpace* new_address_space = (VirtualAddressSpace*)vmm_get_new_page(kernel_directory, true, false, NULL);
    if (!new_address_space) return NULL;
    kmemset(new_address_space, 0, sizeof(VirtualAddressSpace));

    for (uint32_t i = 0; i <= 1; i++) {
        new_address_space->directory_entries[i] = kernel_directory->directory_entries[i];
    }

    uint32_t num_pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;

    for (uint32_t p = 0; p < num_pages; p++) {
        void* local_user_code = vmm_get_new_page(kernel_directory, true, false, NULL);
        if (!local_user_code) return NULL;

        void* src_chunk = (void*)((uint32_t)entry_point + (p * PAGE_SIZE));
        
        size_t bytes_to_copy = PAGE_SIZE;
        if (p == num_pages - 1) {
            bytes_to_copy = size - (p * PAGE_SIZE);
            kmemset(local_user_code, 0, PAGE_SIZE);
        }

        kmemcpy(src_chunk, local_user_code, bytes_to_copy);

        uint32_t user_code_physical_frame = vmm_get_physical_address(kernel_directory, local_user_code);

        uint32_t target_virtual_address = 0xC0000000 + (p * PAGE_SIZE);

        vmm_map_frame(new_address_space, (void*)target_virtual_address, user_code_physical_frame, true, is_user_process);

        vmm_free_page(kernel_directory, local_user_code);
    }

    return new_address_space;
}

uint32_t vmm_get_physical_address(VirtualAddressSpace* space, void* virtual_address) {
    uint32_t vaddr = (uint32_t)virtual_address;
    uint32_t dir_idx = (vaddr >> 22) & 0x3FF;
    uint32_t page_idx = (vaddr >> 12) & 0x3FF;

    if (!space->directory_entries[dir_idx].present) return 0;

    uint32_t table_phys = space->directory_entries[dir_idx].page_table_addr << 12;
    
    uint32_t k_pt1_phys = kernel_directory->directory_entries[1].page_table_addr << 12;
    PageTable* kernel_pt1 = (PageTable*)k_pt1_phys;

    kernel_pt1->pages[1023].present = 1;
    kernel_pt1->pages[1023].rw = 0;
    kernel_pt1->pages[1023].frame_addr = table_phys >> 12;
    paging_invalidate_page_cache(0x007FF000);

    PageTable* table = (PageTable*)0x007FF000;
    uint32_t phys_frame = table->pages[page_idx].frame_addr << 12;

    kernel_pt1->pages[1023].present = 0;
    paging_invalidate_page_cache(0x007FF000);

    return phys_frame;
}