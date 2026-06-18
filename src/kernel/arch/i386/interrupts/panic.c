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

static void hex_to_string(uint32_t value, char* buffer) {
    const char* hex_digits = "0123456789ABCDEF";
    buffer[0] = '0';
    buffer[1] = 'x';
    for (int i = 7; i >= 0; i--) {
        buffer[2 + i] = hex_digits[value & 0xF];
        value >>= 4;
    }
    buffer[10] = '\0';
}

void kernel_panic(CpuRegisters* regs) {
    uint32_t interrupt = regs->interrupt_number;
    const char* message = exception_messages[interrupt];

    vterm_set_buffering_mode(BUFFER_MODE_BLOCK);
    vterm_clear(COLOR_WHITE, COLOR_BLUE);
    vterm_set_color(COLOR_WHITE, COLOR_BLUE);
    vterm_set_cursor_visible(false);

    char panic_buffer[SCREEN_COLS];

    int current_row = 2;

    strncpy(":( A problem has been detected and kdbOS has been shut down.", panic_buffer, SCREEN_COLS);
    vterm_print_aligned(current_row, panic_buffer, VTERM_ALIGN_CENTER);
    
    current_row += 2;
    strncpy("Reason: ", panic_buffer, SCREEN_COLS);
    strncat(panic_buffer, message, SCREEN_COLS);
    vterm_print_aligned(current_row, panic_buffer, VTERM_ALIGN_CENTER);

    if (interrupt == 14) {
        uint32_t faulting_address;
        __asm__ volatile("mov %%cr2, %0" : "=r"(faulting_address));

        current_row += 2;
        char addr_str[12];
        hex_to_string(faulting_address, addr_str);
        strncpy("Fault Address: ", panic_buffer, SCREEN_COLS);
        strncat(panic_buffer, addr_str, SCREEN_COLS);
        vterm_print_aligned(current_row, panic_buffer, VTERM_ALIGN_CENTER);

        uint32_t err_code = regs->error_code; 
        bool present   = err_code & (1 << 0);
        bool write     = err_code & (1 << 1);
        bool user      = err_code & (1 << 2);
        bool inst_exec = err_code & (1 << 4);

        current_row += 2;
        if (user) {
            strncpy("Context: A user-mode program tried to ", panic_buffer, SCREEN_COLS);
        } else {
            strncpy("Context: The kernel tried to ", panic_buffer, SCREEN_COLS);
        }

        if (inst_exec) {
            strncat(panic_buffer, "EXECUTE code at this address.", SCREEN_COLS);
        } else if (write) {
            strncat(panic_buffer, "WRITE data to this address.", SCREEN_COLS);
        } else {
            strncat(panic_buffer, "READ data from this address.", SCREEN_COLS);
        }
        vterm_print_aligned(current_row, panic_buffer, VTERM_ALIGN_CENTER);

        current_row += 1;
        if (!present) {
            strncpy("Cause: This virtual address is NOT mapped in the page tables.", panic_buffer, SCREEN_COLS);
        } else {
            strncpy("Cause: Access denied! Page is protected (e.g., writing to Read-Only).", panic_buffer, SCREEN_COLS);
        }
        vterm_print_aligned(current_row, panic_buffer, VTERM_ALIGN_CENTER);
    }

    current_row += 3;
    strncpy("If this is the first time you've seen this, restart your computer.", panic_buffer, SCREEN_COLS);
    vterm_print_aligned(current_row, panic_buffer, VTERM_ALIGN_CENTER);

    current_row += 1;
    strncpy("If errors persist, check your build.", panic_buffer, SCREEN_COLS);
    vterm_print_aligned(current_row, panic_buffer, VTERM_ALIGN_CENTER);

    vterm_flush();

    while (1) {
        __asm__ volatile("cli");
        __asm__ volatile("hlt");
    }
}