#include "memory.h"
#include "types.h"

void* memset(void* dest, uint8_t val, size_t count) {
    uint8_t* temp = (uint8_t*)dest;
    for (size_t i = 0; i < count; i++) {
        temp[i] = val;
    }

    return dest;
}

void* memcpy(const void* src, void* dest, size_t count) {
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;
    for (size_t i = 0; i < count; i++) {
        d[i] = s[i];
    }
    return dest;
}