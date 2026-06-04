#include "types.h"
#include "io.h"
#include "asm.h"

void write_char_to_framebuffer(uint8_t character, uint16_t row, uint16_t col, uint8_t fg_color, uint8_t bg_color) {
    volatile uint16_t* framebuffer = (volatile uint16_t*) FRAMEBUFFER;

    uint16_t character_framebuffer = (0x00FF & character);
    uint16_t bg_color_framebuffer = (0x000F & bg_color) << 12;
    uint16_t fg_color_framebuffer = (0x00F & fg_color) << 8;

    framebuffer[col + row * 80] = character_framebuffer | fg_color_framebuffer | bg_color_framebuffer;
}

void move_cursor(uint16_t pos) {
    uint16_t pos_low_byte = pos & 0x00FF;
    uint16_t pos_high_byte = (pos >> 8) & 0x00FF;

    outb(CRTC_CMD_PORT, CURSOR_POS_HIGH_BYTE_CMD);
    outb(CRTC_DATA_PORT, pos_high_byte);
    outb(CRTC_CMD_PORT, CURSOR_POS_LOW_BYTE_CMD);
    outb(CRTC_DATA_PORT, pos_low_byte);
}

void scroll(uint16_t row) {
    uint16_t pos = 80 * row;
    uint16_t pos_low_byte = pos & 0x00FF;
    uint16_t pos_high_byte = (pos >> 8) & 0x00FF;

    outb(CRTC_CMD_PORT, SCREEN_START_POS_HIGH_BYTE_CMD);
    outb(CRTC_DATA_PORT, pos_high_byte);
    outb(CRTC_CMD_PORT, SCREEN_START_POS_LOW_BYTE_CMD);
    outb(CRTC_DATA_PORT, pos_low_byte);
}

void printc(const char c, uint16_t pos) {
    write_char_to_framebuffer(c, 0, pos, WHITE, BLACK);
}

void print(const char* buf, uint16_t len) {
    for (uint32_t i = 0; i < len; i++) {
        write_char_to_framebuffer(buf[i], 0, i, WHITE, BLACK);
    }
    move_cursor(len);
}

void printb(uint8_t* byte, uint32_t pos) {
    for (uint16_t bit = 0; bit < 8; bit++) {
        uint8_t mask = (uint8_t) 0x1 << (7 - bit);
        if (*byte & mask) {
            printc('1', pos + bit);
        } else {
            printc('0', pos + bit);
        }
    }
}

void style_cursor(CursorStyle style) {
    uint8_t start;
    switch (style) {
        case SMALL:
            outb(CRTC_CMD_PORT, CURSOR_STYLE_START_CMD);
            outb(CRTC_DATA_PORT, CURSOR_SMALL);
            break;
        case BIG:
            outb(CRTC_CMD_PORT, CURSOR_STYLE_START_CMD);
            outb(CRTC_DATA_PORT, CURSOR_BIG);
            break;
        case DISABLE:
            outb(CRTC_CMD_PORT, CURSOR_STYLE_START_CMD);
            start = inb(CRTC_DATA_PORT);
            outb(CRTC_DATA_PORT, start | CURSOR_DISABLE);
            break;
        case ENABLE:
            outb(CRTC_CMD_PORT, CURSOR_STYLE_START_CMD);
            start = inb(CRTC_DATA_PORT);
            outb(CRTC_DATA_PORT, start & CURSOR_ENABLE);
            break;
        default:
    }
}