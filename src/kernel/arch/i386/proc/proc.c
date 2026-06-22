#include "arch/i386/proc/proc.h"
#include "pmm.h"
#include "vmm.h"
#include "kmemory.h"

extern VirtualAddressSpace* kernel_directory;

void* create_process_stack(void* entry_point, uint32_t* out_physical_frame) {
    void* local_stack_window = vmm_get_new_page(kernel_directory, true, false, NULL);
    if (!local_stack_window) return NULL;
    kmemset(local_stack_window, 0, 4096);

    CpuContext* context = (CpuContext*)((uint32_t)local_stack_window + 4096 - sizeof(CpuContext));

    context->eflags = 0x0202;
    context->eax = 0;
    context->ecx = 0;
    context->edx = 0;
    context->ebx = 0;
    context->esp_dummy = 0;
    context->ebp = 0;
    context->esi = 0;
    context->edi = 0;
    context->eip = (uint32_t)entry_point;

    uint32_t stack_pd_idx = ((uint32_t)local_stack_window) >> 22;
    PageTable* stack_pt_virt = (PageTable*)(kernel_directory->directory_entries[stack_pd_idx].page_table_addr << 12);
    uint32_t stack_idx = ((uint32_t)local_stack_window >> 12) & 0x3FF;
    
    uint32_t stack_phys_frame = stack_pt_virt->pages[stack_idx].frame_addr << 12;
    if (out_physical_frame) {
        *out_physical_frame = stack_phys_frame;
    }

    uint32_t offset_within_page = (uint32_t)context & 0xFFF;
    void* target_process_esp = (void*)(0xC0001000 + offset_within_page);

    vmm_free_page(kernel_directory, local_stack_window);

    return target_process_esp;
}

static inline uint32_t read_cr3(void) {
    uint32_t val;
    __asm__ volatile("mov %%cr3, %0" : "=r"(val));
    return val;
}

static inline void write_cr3(uint32_t val) {
    __asm__ volatile("mov %0, %%cr3" :: "r"(val));
}