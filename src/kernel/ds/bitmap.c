#include "bitmap.h"

void bitmap_clear_all(Bitmap* b) {
    size_t words = b->total_bits / 32;
    if (b->total_bits % 32 != 0) words++;
    for (size_t i = 0; i < words; i++) {
        b->storage[i] = 0;
    }
}

void bitmap_set(Bitmap* b, size_t bit_index) {
    if (bit_index >= b->total_bits) return;
    b->storage[bit_index / 32] |= (1U << (bit_index % 32));
}

void bitmap_clear(Bitmap* b, size_t bit_index) {
    if (bit_index >= b->total_bits) return;
    b->storage[bit_index / 32] &= ~(1U << (bit_index % 32));
}

bool bitmap_test(Bitmap* b, size_t bit_index) {
    if (bit_index >= b->total_bits) return false;
    return (b->storage[bit_index / 32] & (1U << (bit_index % 32))) != 0;
}