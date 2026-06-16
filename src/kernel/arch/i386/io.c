#include "io.h"
#include "asm.h"

void write_char_to_framebuffer(uint8_t character, uint16_t row, uint16_t col, uint8_t fg_color, uint8_t bg_color) {
    volatile uint16_t* framebuffer = (volatile uint16_t*) FRAMEBUFFER;

    uint16_t character_framebuffer = (0x00FF & character);
    uint16_t bg_color_framebuffer = (0x000F & bg_color) << 12;
    uint16_t fg_color_framebuffer = (0x000F & fg_color) << 8;

    framebuffer[col + row * SCREEN_COLS] = character_framebuffer | fg_color_framebuffer | bg_color_framebuffer;
}

void update_hardware_cursor_position(uint16_t pos) {
    outb(CRTC_CMD_PORT, CURSOR_POS_HIGH_BYTE_CMD);
    outb(CRTC_DATA_PORT, (pos >> 8) & 0x00FF);
    outb(CRTC_CMD_PORT, CURSOR_POS_LOW_BYTE_CMD);
    outb(CRTC_DATA_PORT, pos & 0x00FF);
}

void set_hardware_screen_start(uint16_t row) {
    uint16_t pos = SCREEN_COLS * row;
    outb(CRTC_CMD_PORT, SCREEN_START_POS_HIGH_BYTE_CMD);
    outb(CRTC_DATA_PORT, (pos >> 8) & 0x00FF);
    outb(CRTC_CMD_PORT, SCREEN_START_POS_LOW_BYTE_CMD);
    outb(CRTC_DATA_PORT, pos & 0x00FF);
}