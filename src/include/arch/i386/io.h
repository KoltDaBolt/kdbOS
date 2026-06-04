#pragma once

#include "types.h"

#define FRAMEBUFFER                     0x000B8000

#define CRTC_CMD_PORT                   0x3D4
#define CRTC_DATA_PORT                  0x3D5

#define CURSOR_POS_HIGH_BYTE_CMD        0x0E
#define CURSOR_POS_LOW_BYTE_CMD         0x0F
#define CURSOR_STYLE_START_CMD          0x0A
#define CURSOR_STYLE_END_CMD            0x0B
#define CURSOR_SMALL                    0x0C
#define CURSOR_BIG                      0x00
#define CURSOR_DISABLE                  0x20
#define CURSOR_ENABLE                   0xBF

#define SCREEN_START_POS_HIGH_BYTE_CMD  0x0C
#define SCREEN_START_POS_LOW_BYTE_CMD   0x0D
#define SCREEN_COLS                     80
#define SCREEN_ROWS                     25

#define BLACK                           0x0
#define BLUE                            0x1
#define GREEN                           0x2
#define CYAN                            0x3
#define RED                             0x4
#define MAGENTA                         0x5
#define BROWN                           0x6
#define LIGHTGREY                       0x7
#define DARKGREY                        0x8
#define LIGHTBLUE                       0x9
#define LIGHTGREEN                      0xA
#define LIGHTCYAN                       0xB
#define LIGHTRED                        0xC
#define LIGHTMAGENTA                    0xD
#define LIGHTBROWN                      0xE
#define WHITE                           0xF

typedef enum {
    SMALL,
    BIG,
    DISABLE,
    ENABLE
} CursorStyle;

void write_char_to_framebuffer(uint8_t, uint16_t, uint16_t, uint8_t, uint8_t);
void move_cursor(uint16_t);
void scroll(uint16_t);
void printc(const char, uint16_t);
void print(const char*, uint16_t);
void printb(uint8_t*, uint32_t);
void style_cursor(CursorStyle);