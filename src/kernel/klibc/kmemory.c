#include "kmemory.h"

static MemoryBlockHeader* heap_start = NULL;

void* kmemset(void* dest, uint8_t val, size_t count) {
    uint8_t* temp = (uint8_t*)dest;
    for (size_t i = 0; i < count; i++) {
        temp[i] = val;
    }

    return dest;
}

void* kmemcpy(const void* src, void* dest, size_t count) {
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;
    for (size_t i = 0; i < count; i++) {
        d[i] = s[i];
    }
    return dest;
}

void kmalloc_init(uint32_t dynamic_heap_start) {
    heap_start = (MemoryBlockHeader*)dynamic_heap_start;

    uint32_t identity_mapped_cieling = 0x007FFFFF;
    size_t kernel_heap_size = identity_mapped_cieling - dynamic_heap_start;

    heap_start->size = kernel_heap_size - sizeof(MemoryBlockHeader);
    heap_start->is_free = true;
    heap_start->next = NULL;
}

void* kmalloc(size_t size) {
    if (size == 0) return NULL;

    MemoryBlockHeader* current_memory_block = heap_start;

    while(current_memory_block != NULL) {
        if (current_memory_block->is_free && current_memory_block->size >= size) {
            if (current_memory_block->size >= (size + sizeof(MemoryBlockHeader) + 4)) {
                uint32_t new_header_address = (uint32_t)current_memory_block + sizeof(MemoryBlockHeader) + size;
                MemoryBlockHeader* new_memory_block = (MemoryBlockHeader*)new_header_address;

                new_memory_block->size = current_memory_block->size - size - sizeof(MemoryBlockHeader);
                new_memory_block->is_free = true;
                new_memory_block->next = current_memory_block->next;

                current_memory_block->size = size;
                current_memory_block->next = new_memory_block;
            }

            current_memory_block->is_free = false;

            return (void*)((uint32_t)current_memory_block + sizeof(MemoryBlockHeader));
        }

        current_memory_block = current_memory_block->next;
    }

    return NULL; // TODO: ADD IN FUNCTIONALITY TO ADD MORE PAGES
}

void kfree(void* ptr) {
    if (ptr == NULL) return;

    MemoryBlockHeader* block = (MemoryBlockHeader*)((uint32_t)ptr - sizeof(MemoryBlockHeader));
    block->is_free = true;

    MemoryBlockHeader* current_memory_block = heap_start;
    while(current_memory_block != NULL && current_memory_block->next != NULL) {
        if (current_memory_block->is_free && current_memory_block->next->is_free) {
            current_memory_block->size = sizeof(MemoryBlockHeader) + current_memory_block->next->size;
            current_memory_block->next = current_memory_block->next->next;
            continue;
        }

        current_memory_block = current_memory_block->next;
    }
}