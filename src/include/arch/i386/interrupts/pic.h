#pragma once

#include "types.h"

#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1

#define ICW1_INIT    0x11
#define ICW4_8086    0x01
#define PIC_EOI      0x20

void pic_remap(void);
void pic_send_eoi(uint8_t irq_line);
void pic_set_irq_mask(uint8_t irq_line, bool masked);