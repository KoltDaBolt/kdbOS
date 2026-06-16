#pragma once

#include "types.h"

typedef struct gdt_descriptor {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
} __attribute__((packed)) GdtDescriptor;

typedef struct gdt_register {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) GdtRegister;

#define GDT_ENTRIES 5

void init_gdt(void);