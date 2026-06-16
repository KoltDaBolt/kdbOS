#include "vterm.h"
#include "isr.h"
#include "string.h"
#include "panic.h"

static IsrHandler cpu_exception_handlers[CPU_ENTRIES]   = {0};
static IrqHandler irq_handlers[IRQ_ENTRIES]             = {0};
static IsrHandler syscall_handlers[SYSCALL_ENTRIES]     = {0};

void init_isr(void) {
    for (int i = 0; i < CPU_ENTRIES; i++)     cpu_exception_handlers[i] = 0;
    for (int i = 0; i < IRQ_ENTRIES; i++)     irq_handlers[i] = 0;
    for (int i = 0; i < SYSCALL_ENTRIES; i++) syscall_handlers[i] = 0;

    for (int i = 0; i < 32; i++) {
        register_cpu_handler(i, kernel_panic);
    }
}

void register_cpu_handler(uint8_t vector, IsrHandler handler) {
    if (vector < 32) {
        cpu_exception_handlers[vector] = handler;
    }
}

void register_irq_handler(uint8_t irq_line, IrqHandler handler) {
    if (irq_line >= 32 && irq_line < 48) {
        irq_handlers[irq_line] = handler;
    }
}

void register_syscall_handler(uint8_t vector, IsrHandler handler) {
    if (vector >= 48) {
        syscall_handlers[vector] = handler;
    }
}

void master_exception_handler(CpuRegisters* regs) {
    uint32_t vector = regs->interrupt_number;

    if (vector < 32) {
        IsrHandler handler = cpu_exception_handlers[vector];
        if (handler != 0) {
            handler(regs);
        }
        return;
    }

    if (vector >= 32 && vector < 48) {
        IrqHandler handler = irq_handlers[vector];
        if (handler != 0) {
            handler();
        }
        return;
    }

    if (vector >= 48 && vector < 256) {
        IsrHandler handler = syscall_handlers[vector];
        if (handler != 0) {
            handler(regs);
        }
        return;
    }
}