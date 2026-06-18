#pragma once

#include "types.h"

typedef struct memory_block_header {
    size_t size;
    bool is_free;
    struct memory_block_header* next;
} MemoryBlockHeader;

void* kmemset(void*, uint8_t, size_t);
void* kmemcpy(const void*, void*, size_t);

void kmalloc_init(uint32_t);
void* kmalloc(size_t);
void kfree(void*);