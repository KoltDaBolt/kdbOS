#include "asm.h"
#include "types.h"
#include "vterm.h"
#include "kstring.h"
#include "multiboot.h"
#include "gdt.h"
#include "idt.h"
#include "pic.h"
#include "isr.h"
#include "keyboard.h"
#include "pmm.h"
#include "paging.h"
#include "kmemory.h"
#include "sched.h"
#include "proc/proc.h"
#include "arch/i386/proc/proc.h"

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
    uint32_t dynamic_heap_start = pmm_init(total_ram_bytes, bitmap_addr);

    paging_init();
    
    kmalloc_init(dynamic_heap_start);
}

static void _init_isr(void) {
    init_keyboard();
    isr_register_sched_timer();

    outb(PIC1_DATA, 0xFC); 
    outb(PIC2_DATA, 0xFF);
    __asm__ volatile("sti");
}

// prints IDLE
unsigned char idle_process_code[] = {
    0xC6, 0x05, 0x00, 0x80, 0x0B, 0x00, 0x49,
    0xC6, 0x05, 0x01, 0x80, 0x0B, 0x00, 0x07,
    0xC6, 0x05, 0x02, 0x80, 0x0B, 0x00, 0x44,
    0xC6, 0x05, 0x03, 0x80, 0x0B, 0x00, 0x07,
    0xC6, 0x05, 0x04, 0x80, 0x0B, 0x00, 0x4C,
    0xC6, 0x05, 0x05, 0x80, 0x0B, 0x00, 0x07,
    0xC6, 0x05, 0x06, 0x80, 0x0B, 0x00, 0x45,
    0xC6, 0x05, 0x07, 0x80, 0x0B, 0x00, 0x07,
    0xEB, 0xC6
};

// prints P1
unsigned char process_one_code[] = {
    0xC6, 0x05, 0x00, 0x80, 0x0B, 0x00, 0x50,
    0xC6, 0x05, 0x01, 0x80, 0x0B, 0x00, 0x0A,
    0xC6, 0x05, 0x02, 0x80, 0x0B, 0x00, 0x31,
    0xC6, 0x05, 0x03, 0x80, 0x0B, 0x00, 0x0A,
    0xEB, 0xE2
};

// prints P2
unsigned char process_two_code[] = {
    0xC6, 0x05, 0x00, 0x80, 0x0B, 0x00, 0x50,
    0xC6, 0x05, 0x01, 0x80, 0x0B, 0x00, 0x0B,
    0xC6, 0x05, 0x02, 0x80, 0x0B, 0x00, 0x32,
    0xC6, 0x05, 0x03, 0x80, 0x0B, 0x00, 0x0B,
    0xEB, 0xE2
};

int kernel_main(uint32_t magic, uint32_t multiboot_ptr) {
    vterm_set_cursor_visible(false);
    vterm_set_buffering_mode(BUFFER_MODE_BLOCK);
    init_cpu_architecture();
    
    MultibootInfo* mb_info = verify_multiboot(magic, multiboot_ptr);
    if (mb_info == NULL) {
        while (1) {
            __asm__ volatile("cli");
            __asm__ volatile("hlt");
        }
    }

    init_memory_management(mb_info);

    sched_init((void*)idle_process_code, sizeof(idle_process_code), true);

    ProcessControlBlock* p1 = create_pcb((void*)process_one_code, sizeof(process_one_code), true);
    ProcessControlBlock* p2 = create_pcb((void*)process_two_code, sizeof(process_two_code), true);

    sched_enqueue(p1);
    sched_enqueue(p2);

    _init_isr();

    while (1) {
        __asm__ volatile("hlt"); 
    }

    return 0;
}