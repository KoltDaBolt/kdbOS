#pragma once

#include "types.h"

typedef struct {
    uint32_t* storage;
    size_t total_bits;
} Bitmap;

void bitmap_clear_all(Bitmap*);
void bitmap_set(Bitmap*, size_t);
void bitmap_clear(Bitmap*, size_t);
bool bitmap_test(Bitmap*, size_t);