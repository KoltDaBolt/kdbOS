#pragma once

#include "types.h"
#include "paging.h"
#include "list.h"

typedef enum {
    STATE_NEW,
    STATE_READY,
    STATE_RUNNING,
    STATE_WAITING,
    STATE_TERMINATING
} ProcessState;

typedef struct {
    uint32_t pid;
    ListNode sched_node;
    ProcessState state;
    uint32_t priority;
    uint32_t quantum_remaining;
    uint32_t age_ticks;
    VirtualAddressSpace* address_space;
    uint32_t address_space_physical;
    void* kernel_stack_ptr;
} ProcessControlBlock;

ProcessControlBlock* create_pcb(void*, size_t, bool);