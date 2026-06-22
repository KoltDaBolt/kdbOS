#include "pmm.h"
#include "bitmap.h"

static Bitmap frame_bitmap = {0};
static uint32_t total_frames = 0;

uint32_t pmm_init(size_t total_ram_bytes, uint32_t* bitmap_start_addr) {
    total_frames = total_ram_bytes / PMM_FRAME_SIZE;

    frame_bitmap.storage = bitmap_start_addr;
    frame_bitmap.total_bits = total_frames;

    size_t bitmap_size_uint32s = total_frames / 32;
    if (total_frames % 32 != 0) {
        bitmap_size_uint32s++;
    }

    uint32_t total_bitmap_bytes = bitmap_size_uint32s * sizeof(uint32_t);
    bitmap_clear_all(&frame_bitmap);

    uint32_t next_free_address = (uint32_t)bitmap_start_addr + total_bitmap_bytes;

    uint32_t frames_to_reserve = next_free_address / PMM_FRAME_SIZE;

    for (uint32_t frame = 0; frame < frames_to_reserve; frame++) {
        bitmap_set(&frame_bitmap, frame);
    }

    return next_free_address;
}

uint32_t pmm_allocate_frame(void) {
    for (size_t bit_index = 0; bit_index < frame_bitmap.total_bits; bit_index++) {
        if (!bitmap_test(&frame_bitmap, bit_index)) {
            bitmap_set(&frame_bitmap, bit_index);
            return (uint32_t)bit_index * PMM_FRAME_SIZE;
        }
    }

    return 0;
}

void pmm_free_frame(uint32_t frame_physical_addr) {
    uint32_t bit_index = frame_physical_addr / PMM_FRAME_SIZE;
    bitmap_clear(&frame_bitmap, bit_index);
}