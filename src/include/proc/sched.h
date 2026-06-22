#pragma once

#include "types.h"
#include "list.h"
#include "proc.h"

#define MLFQ_LEVELS 4
#define AGE_TICK_MAX 600

static const uint32_t MLFQ_QUANTUMS[MLFQ_LEVELS] = { 40, 80, 120, 160 };

typedef struct {
    List* priority_queues[MLFQ_LEVELS];
    List* waiting_queue;
    ProcessControlBlock* current_pcb;
    ProcessControlBlock* idle_pcb;
} Scheduler;

void sched_init(void*, size_t, bool);
void sched_enqueue(ProcessControlBlock*);
void sched_pick_next(void);
void sched_tick(void);
void sched_block_process(void);
void sched_unblock_process(ProcessControlBlock*);
void sched_timer_handler();
void isr_register_sched_timer(void);