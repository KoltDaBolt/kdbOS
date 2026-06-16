#pragma once

#include "types.h"

#define CPU_ENTRIES     32
#define IRQ_ENTRIES     16
#define SYSCALL_ENTRIES 256

typedef struct cpu_registers {
    uint32_t ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t interrupt_number, error_code;
    uint32_t eip, cs, eflags, useresp, ss;
} CpuRegisters;

typedef void (*IsrHandler)(CpuRegisters* regs);
typedef void (*IrqHandler)(void);

void init_isr(void);

void register_cpu_handler(uint8_t vector, IsrHandler handler);
void register_irq_handler(uint8_t irq_line, IrqHandler handler);
void register_syscall_handler(uint8_t vector, IsrHandler handler);

void master_exception_handler(CpuRegisters*);