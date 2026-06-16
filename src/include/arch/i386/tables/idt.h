#pragma once

#include "types.h"

typedef struct idt_descriptor {
    uint16_t base_low;
    uint16_t selector;
    uint8_t  always0;
    uint8_t  flags;
    uint16_t base_high;
} __attribute__((packed)) IdtDescriptor;

typedef struct idt_register {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) IdtRegister;

#define IDT_ENTRIES 256

void init_idt(void);