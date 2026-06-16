#include "asm.h"
#include "types.h"
#include "vterm.h"
#include "string.h"
#include "multiboot.h"
#include "gdt.h"
#include "idt.h"
#include "pic.h"
#include "isr.h"
#include "keyboard.h"

int kernel_main(uint32_t magic, uint32_t multiboot_ptr) {
    init_gdt();
    init_isr();
    pic_remap();
    init_idt();

    vterm_init(COLOR_WHITE, COLOR_BLACK);
    init_keyboard();

    VTermCursor cursor = { .shape = CURSOR_SHAPE_LINE, .visible = false };
    vterm_configure_cursor(cursor);

    if (magic != 0x2BADB002) {
        vterm_move_cursor(0, 0);
        vterm_set_color(COLOR_RED, COLOR_BLACK);
        kprint_c('M');
        return 0;
    }

    MultibootInfo* mb_info = (MultibootInfo*)multiboot_ptr;
    if ((mb_info->flags & (1 << 6)) == 0) {
        vterm_move_cursor(0, 0);
        vterm_set_color(COLOR_RED, COLOR_BLACK);
        kprint_c('E');
        return 0;
    }

    // -=-=-=-=-= HARDWARE EXCEPTION TEST =-=-=-=-=-
    // Uncomment these three lines to test division by zero crash
    // volatile int num1 = 10;
    // volatile int num2 = 0;
    // volatile int crash = num1 / num2;

    outb(PIC1_DATA, 0xFD); 
    outb(PIC2_DATA, 0xFF);
    __asm__ volatile("sti");

    vterm_set_color(COLOR_WHITE, COLOR_BLACK);
    vterm_print_aligned(2, "Welcome to kdbOS!", VTERM_ALIGN_CENTER);
    vterm_print_aligned(3, "----------------------------------------", VTERM_ALIGN_CENTER);

    vterm_set_color(COLOR_WHITE, COLOR_BLACK);
    vterm_move_cursor(5, 0);
    vterm_write("Test keyboard input here: ");

    while (1) {
        __asm__ volatile("hlt"); 
    }

    return 0;
}