#pragma once

#include "types.h"
#include "isr.h"

#define KEYBOARD_DATA_PORT      0x60
#define KEYBOARD_STATUS_PORT    0x64

void init_keyboard(void);
void keyboard_handler(void);