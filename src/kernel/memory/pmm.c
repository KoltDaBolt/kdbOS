#include "pmm.h"

static uint32_t* pmm_bitmap = NULL;
static uint32_t total_frames = 0;
static uint32_t bitmap_size_uint32s = 0;

static void local_clear_bytes(void* ptr, uint32_t num_bytes) {
    uint8_t* byte_pointer = (uint8_t*)ptr;
    for (uint32_t i = 0; i < num_bytes; i++) {
        byte_pointer[i] = 0;
    }
}

uint32_t pmm_init(size_t total_ram_bytes, uint32_t* bitmap_start_addr) {
    total_frames = total_ram_bytes / PMM_FRAME_SIZE;

    bitmap_size_uint32s = total_frames / 32;
    if (total_frames % 32 != 0) {
        bitmap_size_uint32s++;
    }

    pmm_bitmap = bitmap_start_addr;

    uint32_t total_bitmap_bytes = bitmap_size_uint32s * sizeof(uint32_t);
    local_clear_bytes(pmm_bitmap, total_bitmap_bytes);

    uint32_t bitmap_size_aligned = (total_bitmap_bytes + 4095) & ~4095;
    uint32_t next_free_address = (uint32_t)bitmap_start_addr + bitmap_size_aligned;

    return next_free_address;
}

uint32_t pmm_allocate_frame(void) {
    for (uint32_t i = 0; i < bitmap_size_uint32s; i++) {
        if (pmm_bitmap[i] != 0xFFFFFFFF) {
            for (int bit = 0; bit < 32; bit++) {
                uint32_t bit_mask = (1 << bit);

                if (!(pmm_bitmap[i] & bit_mask)) {
                    pmm_bitmap[i] |= bit_mask;
                    uint32_t frame_index = (i * 32) + bit;
                    return frame_index * PMM_FRAME_SIZE;
                }
            }
        }
    }

    return 0;
}

void pmm_reserve_region(uint32_t start_addr, size_t size_bytes) {
    uint32_t start_frame = start_addr / PMM_FRAME_SIZE;
    uint32_t end_frame = (start_addr + size_bytes + PMM_FRAME_SIZE - 1) / PMM_FRAME_SIZE;

    for (uint32_t frame = start_frame; frame < end_frame; frame++) {
        uint32_t bitmap_index = frame / 32;
        uint32_t bit_position = frame % 32;
        
        pmm_bitmap[bitmap_index] |= (1 << bit_position);
    }
}

void pmm_free_frame(uint32_t frame_physical_addr) {
    uint32_t frame_index  = frame_physical_addr / PMM_FRAME_SIZE;
    uint32_t bitmap_index = frame_index / 32;
    uint32_t bit_position = frame_index % 32;
    
    pmm_bitmap[bitmap_index] &= ~(1 << bit_position);
}