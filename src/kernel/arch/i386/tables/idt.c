#include "idt.h"

static IdtDescriptor idt[IDT_ENTRIES];
static IdtRegister idt_pointer;

extern uint32_t idt_stub_table[];

static void idt_set_gate(uint8_t num, uint32_t base, uint16_t selector, uint8_t flags) {
    idt[num].base_low  = (base & 0xFFFF);
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].selector  = selector;
    idt[num].always0   = 0;
    idt[num].flags     = flags;
}

void init_idt(void) {
    idt_pointer.limit = (sizeof(IdtDescriptor) * IDT_ENTRIES) - 1;
    idt_pointer.base  = (uint32_t)&idt;

    for (int i = 0; i < IDT_ENTRIES; i++) {
        idt_set_gate(i, idt_stub_table[i], 0x08, 0x8E);
    }

    __asm__ volatile("lidt %0" : : "m"(idt_pointer));
}