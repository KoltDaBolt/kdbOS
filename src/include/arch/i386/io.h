#pragma once

#include "types.h"

#define FRAMEBUFFER                     0x000B8000

#define CRTC_CMD_PORT                   0x3D4
#define CRTC_DATA_PORT                  0x3D5

#define CURSOR_STYLE_START_CMD          0x0A
#define CURSOR_STYLE_END_CMD            0x0B
#define SCREEN_START_POS_HIGH_BYTE_CMD  0x0C
#define SCREEN_START_POS_LOW_BYTE_CMD   0x0D
#define CURSOR_POS_HIGH_BYTE_CMD        0x0E
#define CURSOR_POS_LOW_BYTE_CMD         0x0F

#define SCREEN_COLS                     80
#define SCREEN_ROWS                     25

#define VGA_COLOR_BLACK                 0x0
#define VGA_COLOR_BLUE                  0x1
#define VGA_COLOR_GREEN                 0x2
#define VGA_COLOR_CYAN                  0x3
#define VGA_COLOR_RED                   0x4
#define VGA_COLOR_MAGENTA               0x5
#define VGA_COLOR_BROWN                 0x6
#define VGA_COLOR_LIGHTGREY             0x7
#define VGA_COLOR_DARKGREY              0x8
#define VGA_COLOR_LIGHTBLUE             0x9
#define VGA_COLOR_LIGHTGREEN            0xA
#define VGA_COLOR_LIGHTCYAN             0xB
#define VGA_COLOR_LIGHTRED              0xC
#define VGA_COLOR_LIGHTMAGENTA          0xD
#define VGA_COLOR_LIGHTBROWN            0xE
#define VGA_COLOR_WHITE                 0xF

void write_char_to_framebuffer(uint8_t character, uint16_t row, uint16_t col, uint8_t fg_color, uint8_t bg_color);
void update_hardware_cursor_position(uint16_t pos);
void set_hardware_screen_start(uint16_t row);