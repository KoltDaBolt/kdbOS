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
#include "pmm.h"
#include "paging.h"

static void init_cpu_architecture(void) {
    init_gdt();
    init_isr();
    pic_remap();
    init_idt();
}

static MultibootInfo* verify_multiboot(uint32_t magic, uint32_t multiboot_ptr) {
    vterm_init(COLOR_WHITE, COLOR_BLACK);

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

    return mb_info;
}

static void init_memory_management(MultibootInfo* mb_info) {
    size_t total_ram_bytes = (mb_info->mem_lower + mb_info->mem_upper) * 1024;
    uint32_t* bitmap_addr = (uint32_t*)0x00400000;
    pmm_init(total_ram_bytes, bitmap_addr);

    pmm_reserve_region(0x00000000, 0x100000); // 1. Protect the first 1MB completely (Includes Interrupt Vectors, BIOS, and VGA Space)
    pmm_reserve_region(0x00100000, 0x300000); // 2. Protect your kernel's code footprint and your PMM bitmap allocation block

    paging_init();
}

static void init_hardware_drivers(void) {
    init_keyboard();

    outb(PIC1_DATA, 0xFD); 
    outb(PIC2_DATA, 0xFF);
    __asm__ volatile("sti");
}

static void display_welcome_screen(void) {
    vterm_set_color(COLOR_WHITE, COLOR_BLACK);
    vterm_set_cursor_visible(false);
    vterm_print_aligned(2, "Welcome to kdbOS!", VTERM_ALIGN_CENTER);
    vterm_print_aligned(3, "----------------------------------------", VTERM_ALIGN_CENTER);

    vterm_set_color(COLOR_WHITE, COLOR_BLACK);
    vterm_move_cursor(5, 0);
    vterm_write("Test keyboard input here: ");
    vterm_set_cursor_visible(true);
    vterm_set_cursor_shape(CURSOR_SHAPE_BLOCK);
}

int kernel_main(uint32_t magic, uint32_t multiboot_ptr) {
    init_cpu_architecture();
    
    MultibootInfo* mb_info = verify_multiboot(magic, multiboot_ptr);
    if (mb_info == NULL) {
        while (1) {
            __asm__ volatile("cli");
            __asm__ volatile("hlt");
        }
    }
    
    init_memory_management(mb_info);
    init_hardware_drivers();

    display_welcome_screen();

    // =================================================================
    // HARDWARE INTERCEPT TEST
    // =================================================================
    // 0x00500000 is the 5MB line. This is completely unmapped!
    volatile uint32_t* unmapped_pointer = (uint32_t*)0x00500000;
    
    // Attempting to write here forces the MMU to look up the entry,
    // see present = 0, stop execution, and trigger Interrupt 14.
    *unmapped_pointer = 0xDEADC0DE;

    while (1) {
        __asm__ volatile("hlt"); 
    }

    return 0;
}