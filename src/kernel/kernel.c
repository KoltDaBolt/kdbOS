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

    pmm_reserve_region(0x00000000, 0x100000); // 1. Protect the first 1MB completely (Includes Interrupt Vectors, BIOS, and VGA Space)
    pmm_reserve_region(0x00100000, 0x300000); // 2. Protect your kernel's code footprint and your PMM bitmap allocation block

    paging_init();
    kmalloc_init(dynamic_heap_start);
}

static void init_hardware_drivers(void) {
    init_keyboard();

    outb(PIC1_DATA, 0xFD); 
    outb(PIC2_DATA, 0xFF);
    __asm__ volatile("sti");
}

// static void display_welcome_screen(void) {
//     vterm_set_color(COLOR_WHITE, COLOR_BLACK);
//     vterm_set_cursor_visible(false);
//     vterm_print_aligned(2, "Welcome to kdbOS!", VTERM_ALIGN_CENTER);
//     vterm_print_aligned(3, "----------------------------------------", VTERM_ALIGN_CENTER);

//     vterm_set_color(COLOR_WHITE, COLOR_BLACK);
//     vterm_move_cursor(5, 0);
//     vterm_write("Test keyboard input here: ");
//     vterm_set_cursor_visible(true);
//     vterm_set_cursor_shape(CURSOR_SHAPE_BLOCK);
// }

void run_heap_tests() {
    vterm_set_color(COLOR_WHITE, COLOR_BLACK);
    vterm_move_cursor(1, 0);
    vterm_write("--- Starting Heap Tests ---\n\n");

    // TEST 1: Multi-Page Block Allocation
    vterm_write("Test 1: Allocating 64KB multi-page block... ");
    uint32_t* multi_page_ptr = (uint32_t*)kmalloc(16384 * sizeof(uint32_t)); 

    if (multi_page_ptr == NULL) {
        vterm_set_color(COLOR_RED, COLOR_BLACK);
        vterm_write("\n[FAIL]: 64KB allocation returned NULL!\n");
        return;
    }

    // Verify page permissions across the 16 hardware pages
    multi_page_ptr[0] = 0xAAAAAA01;     
    multi_page_ptr[4096] = 0xAAAAAA02;  
    multi_page_ptr[8192] = 0xAAAAAA03;  
    multi_page_ptr[16383] = 0xAAAAAA04; 

    vterm_set_color(COLOR_GREEN, COLOR_BLACK);
    vterm_write("[PASSED]\n");
    vterm_set_color(COLOR_WHITE, COLOR_BLACK);


    // TEST 2: Page-Spanning Fragmentation & Alignment Guard
    vterm_write("Test 2: Page-spanning chunk validation... ");
    
    // Allocate two awkward blocks that will bridge across individual 4KB page steps
    void* awkward_block1 = kmalloc(5000); 
    void* awkward_block2 = kmalloc(5000); 

    if (!awkward_block1 || !awkward_block2) {
        vterm_set_color(COLOR_RED, COLOR_BLACK);
        vterm_write("\n[FAIL]: Awkward size allocations failed.\n");
        return;
    }

    kfree(awkward_block1);
    kfree(awkward_block2);

    vterm_set_color(COLOR_GREEN, COLOR_BLACK);
    vterm_write("[PASSED]\n");
    vterm_set_color(COLOR_WHITE, COLOR_BLACK);


    // TEST 3: Coalescing Check Across Page Boundaries
    vterm_write("Test 3: Verifying cross-page header coalescing... ");

    // 1. Create three consecutive blocks that sit back-to-back across page steps
    void* merge1 = kmalloc(4096);
    void* merge2 = kmalloc(8192); // Spans at least one 4KB page boundary
    void* merge3 = kmalloc(4096);

    if (!merge1 || !merge2 || !merge3) {
        vterm_set_color(COLOR_RED, COLOR_BLACK);
        vterm_write("\n[FAIL]: Setup for coalescing test failed.\n");
        return;
    }

    // 2. Free them in an order that forces your coalescer to merge left and right
    kfree(merge2); // Free the middle piece
    kfree(merge1); // Free the left piece (should merge with middle)
    kfree(merge3); // Free the right piece (should merge everything into one giant node)

    // 3. Try to allocate a block equal to the combined size of all three chunks + headers.
    // If coalescing failed or corrupted a tracking header on a page break, this will fail.
    size_t combined_total = 4096 + 8192 + 4096 + (sizeof(MemoryBlockHeader) * 2);
    void* massive_merged_chunk = kmalloc(combined_total);

    if (massive_merged_chunk == NULL) {
        vterm_set_color(COLOR_RED, COLOR_BLACK);
        vterm_write("\n[FAIL]: Heap fragmented! Coalescer failed to stitch nodes back together.\n");
        return;
    }

    vterm_set_color(COLOR_GREEN, COLOR_BLACK);
    vterm_write("[PASSED]\n");
    vterm_set_color(COLOR_WHITE, COLOR_BLACK);


    // TEST 4: Cleanup and Final Reclamation
    vterm_write("Test 4: Reclaiming all remaining test memory... ");
    kfree(multi_page_ptr);
    kfree(massive_merged_chunk);

    vterm_set_color(COLOR_GREEN, COLOR_BLACK);
    vterm_write("[PASSED]\n\n");
    vterm_set_color(COLOR_WHITE, COLOR_BLACK);

    vterm_set_color(COLOR_LIGHTBLUE, COLOR_BLACK);
    vterm_write("--- All Accurate Heap & Coalescing Tests Succeeded! ---");
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

    // display_welcome_screen();
    run_heap_tests();

    while (1) {
        __asm__ volatile("hlt"); 
    }

    return 0;
}