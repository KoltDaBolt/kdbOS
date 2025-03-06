#include <stddef.h>
#include <stdint.h>

size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer;

static const size_t TERMINAL_WIDTH = 80;
static const size_t TERMINAL_HEIGHT = 25;

enum vga_color {
	VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_MAGENTA = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_LIGHT_GREY = 7,
	VGA_COLOR_DARK_GREY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN = 14,
	VGA_COLOR_WHITE = 15,
};

size_t strlen(const char* str) {
    size_t len = 0;
    while (*(str + len)) {
        len++;
    }

    return len;
}

static inline uint8_t vga_entry_color(enum vga_color foreground, enum vga_color background) {
    return foreground | background << 4;
}

static inline uint16_t vga_entry(unsigned char c, uint8_t color) {
    return (uint16_t)c | (uint16_t)color << 8;
}

void terminal_initialize(void) {
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    terminal_buffer = (uint16_t*)0xB8000;
    for (size_t y = 0; y < TERMINAL_HEIGHT; y++) {
        for (size_t x = 0; x < TERMINAL_WIDTH; x++) {
            const size_t index = y * TERMINAL_WIDTH + x;
            *(terminal_buffer + index) = vga_entry(' ', terminal_color);
        }
    }
}

void terminal_putEntryAt(char c, uint8_t color, size_t x, size_t y) {
    const size_t index = y * TERMINAL_WIDTH + x;
    *(terminal_buffer + index) = vga_entry(c, color);
}

void terminal_putChar(char c) {
    terminal_putEntryAt(c, terminal_color, terminal_column, terminal_row);
    if (++terminal_column == TERMINAL_WIDTH) {
        terminal_column = 0;
        if (++terminal_row == TERMINAL_HEIGHT) {
            terminal_row = 0;
        }
    }
}

void terminal_write(const char* data, size_t size) {
    for(size_t i = 0; i < size; i++) {
        terminal_putChar(data[i]);
    }
}

void terminal_writestring(const char* data) {
    terminal_write(data, strlen(data));
}