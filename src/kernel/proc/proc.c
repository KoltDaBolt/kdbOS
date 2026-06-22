#include "proc/proc.h"
#include "arch/i386/proc/proc.h"
#include "vmm.h"
#include "kmemory.h"

static uint32_t next_pid = 0;
extern VirtualAddressSpace* kernel_directory;

ProcessControlBlock* create_pcb(void* entry_point, size_t size, bool is_user_process) {
    ProcessControlBlock* pcb = (ProcessControlBlock*)kmalloc(sizeof(ProcessControlBlock));
    if (!pcb) return NULL;

    pcb->pid = next_pid++;
    pcb->sched_node.next = NULL;
    pcb->sched_node.prev = NULL;
    pcb->state = STATE_NEW;
    pcb->priority = 0;
    pcb->quantum_remaining = 40;
    pcb->age_ticks = 0;

    VirtualAddressSpace* process_address_space = vmm_create_virtual_address_space(entry_point, size, is_user_process);
    pcb->address_space = process_address_space;

    uint32_t address_space_physical = vmm_get_physical_address(kernel_directory, process_address_space);
    pcb->address_space_physical = address_space_physical;

    uint32_t stack_frame_physical = 0;
    void* process_esp = create_process_stack((void*)0xC0000000, &stack_frame_physical);
    vmm_map_frame(pcb->address_space, (void*)0xC0001000, stack_frame_physical, true, is_user_process);
    pcb->kernel_stack_ptr = process_esp;
    if (!pcb->kernel_stack_ptr) {
        kfree(pcb);
        return NULL;
    }

    pcb->state = STATE_READY;

    return pcb;
}