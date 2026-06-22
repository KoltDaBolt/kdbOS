#include "proc/sched.h"
#include "isr.h"
#include "pic.h"
#include "proc/proc.h"

extern void context_switch(void** old_stack_prt, void* new_stack_ptr, uint32_t new_cr3);
extern void bootstrap_first_process(void* new_stack_ptr, uint32_t new_cr3);

static Scheduler scheduler;

void sched_init(void* entry_point, size_t size, bool is_user_process) {
    uint32_t err;

    for (int i = 0; i < MLFQ_LEVELS; i++) {
        scheduler.priority_queues[i] = list_init(&err);
    }

    scheduler.waiting_queue = list_init(&err);

    ProcessControlBlock* idle_pcb = create_pcb(entry_point, size, is_user_process);

    scheduler.current_pcb = NULL;
    scheduler.idle_pcb = idle_pcb;
}

void sched_enqueue(ProcessControlBlock* pcb) {
    if (!pcb) return;

    switch (pcb->state) {
        case STATE_READY:
            list_push_tail(scheduler.priority_queues[pcb->priority], &pcb->sched_node);
            break;
        case STATE_WAITING:
            list_push_tail(scheduler.waiting_queue, &pcb->sched_node);
            break;
        default: break;
    }
}

void sched_pick_next_process() {
    ProcessControlBlock* next_pcb = NULL;

    for (int i = 0; i < MLFQ_LEVELS; i++) {
        if (list_get_size(scheduler.priority_queues[i]) > 0) {
            ListNode* node = list_pop_head(scheduler.priority_queues[i]);
            next_pcb = container_of(node, ProcessControlBlock, sched_node);
            break;
        }
    }

    if (!next_pcb) {
        next_pcb = scheduler.idle_pcb;
    }

    scheduler.current_pcb = next_pcb;
    next_pcb->state = STATE_RUNNING;
}

void sched_tick() {
    if (scheduler.current_pcb == NULL) {
        return;
    }

    ProcessControlBlock* current_pcb = scheduler.current_pcb;
    
    if (current_pcb != scheduler.idle_pcb) {
        current_pcb->quantum_remaining--;
    } else {
        return;
    }

    if (current_pcb->quantum_remaining == 0) {
        current_pcb->state = STATE_READY;

        if (current_pcb->priority < (MLFQ_LEVELS - 1)) {
            current_pcb->priority++;
        }

        current_pcb->quantum_remaining = MLFQ_QUANTUMS[current_pcb->priority];
        sched_enqueue(current_pcb);
        scheduler.current_pcb = NULL;
        return;
    }

    for (int i = 1; i < MLFQ_LEVELS; i++) {
        List* q = scheduler.priority_queues[i];
        ListNode* current_node = q->root.next;

        while (current_node != &q->root) {
            ListNode* next_node = current_node->next;
            ProcessControlBlock* waiting_pcb = container_of(current_node, ProcessControlBlock, sched_node);
            waiting_pcb->age_ticks++;

            if (waiting_pcb->age_ticks >= AGE_TICK_MAX) {
                waiting_pcb->age_ticks = 0;
                list_remove_node(q, current_node);
                waiting_pcb->priority--;
                waiting_pcb->quantum_remaining = MLFQ_QUANTUMS[waiting_pcb->priority];
                
                list_push_tail(scheduler.priority_queues[waiting_pcb->priority], &waiting_pcb->sched_node);
            }
            current_node = next_node;
        }
    }
}

void sched_block_process() {
    ProcessControlBlock* current_pcb = scheduler.current_pcb;
    if (!current_pcb || current_pcb == scheduler.idle_pcb) return;

    current_pcb->state = STATE_WAITING;
    sched_enqueue(current_pcb);
}

void sched_unblock_process(ProcessControlBlock* pcb) {
    if (!pcb || pcb->state != STATE_WAITING) return;

    list_remove_node(scheduler.waiting_queue, &pcb->sched_node);

    pcb->state = STATE_READY;
    pcb->age_ticks = 0;
    pcb->quantum_remaining = MLFQ_QUANTUMS[pcb->priority];

    sched_enqueue(pcb);
}

void sched_timer_handler() {
    ProcessControlBlock* old_pcb = scheduler.current_pcb;
    
    sched_tick();
    pic_send_eoi(0);
    
    if (scheduler.current_pcb == NULL || old_pcb == NULL) {
        sched_pick_next_process();
        ProcessControlBlock* next_pcb = scheduler.current_pcb;

        if (old_pcb == NULL) {
            bootstrap_first_process(next_pcb->kernel_stack_ptr, next_pcb->address_space_physical);
            return;
        }

        context_switch(
            &old_pcb->kernel_stack_ptr,
            next_pcb->kernel_stack_ptr,
            next_pcb->address_space_physical
        );
    }
}

void isr_register_sched_timer() {
    register_irq_handler(32, sched_timer_handler);
}