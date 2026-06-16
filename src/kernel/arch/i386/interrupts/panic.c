#include "vterm.h"
#include "isr.h"
#include "panic.h"
#include "string.h"

static const char* exception_messages[32] = {
    "Division By Zero",
    "Debug Exception",
    "Non-Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Exception",
    "x87 FPU Floating-Point Error",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Control Protection Exception",
    "Reserved", "Reserved", "Reserved",
    "Reserved", "Reserved", "Reserved",
    "Hypervisor Injection Exception",
    "VMM Communication Exception",
    "Security Exception",
    "Reserved"
};

void kernel_panic(CpuRegisters* regs) {
    uint32_t interrupt = regs->interrupt_number;
    const char* message = exception_messages[interrupt];

    vterm_set_buffering_mode(BUFFER_MODE_BLOCK);

    vterm_clear(COLOR_WHITE, COLOR_BLUE);
    vterm_set_color(COLOR_WHITE, COLOR_BLUE);

    char panic_buffer[SCREEN_COLS];

    strncpy(":( A problem has been detected and kdbOS has been shut down.", panic_buffer, SCREEN_COLS);
    vterm_print_aligned(2, panic_buffer, VTERM_ALIGN_CENTER);
    
    strncpy("Reason: ", panic_buffer, SCREEN_COLS);
    strncat(panic_buffer, message, SCREEN_COLS);
    vterm_print_aligned(4, panic_buffer, VTERM_ALIGN_CENTER);

    strncpy("If this is the first time you've seen this, restart your computer.", panic_buffer, SCREEN_COLS);
    vterm_print_aligned(6, panic_buffer, VTERM_ALIGN_CENTER);

    strncpy("If errors persist, check your build.", panic_buffer, SCREEN_COLS);
    vterm_print_aligned(7, panic_buffer, VTERM_ALIGN_CENTER);

    vterm_flush();

    while (1) {
        __asm__ volatile("cli");
        __asm__ volatile("hlt");
    }
}