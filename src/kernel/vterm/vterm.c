#include "vterm.h"
#include "io.h"
#include "asm.h"
#include "string.h"

static VTermCell screen_buffer[SCREEN_ROWS][SCREEN_COLS];

static uint16_t cursor_row = 0;
static uint16_t cursor_col = 0;
static VTermColor current_fg = COLOR_WHITE;
static VTermColor current_bg = COLOR_BLACK;
static VTermCursor active_cursor_style = { .shape = CURSOR_SHAPE_LINE, .visible = true };

static VTermBufferingMode current_mode = BUFFER_MODE_STREAM;

static const uint8_t vga_color_lookup[] = {
    VGA_COLOR_BLACK,
    VGA_COLOR_BLUE,
    VGA_COLOR_GREEN,
    VGA_COLOR_CYAN,
    VGA_COLOR_RED,
    VGA_COLOR_MAGENTA,
    VGA_COLOR_BROWN,
    VGA_COLOR_LIGHTGREY,
    VGA_COLOR_DARKGREY,
    VGA_COLOR_LIGHTBLUE,
    VGA_COLOR_LIGHTGREEN,
    VGA_COLOR_LIGHTCYAN,
    VGA_COLOR_LIGHTRED,
    VGA_COLOR_LIGHTMAGENTA,
    VGA_COLOR_LIGHTBROWN,
    VGA_COLOR_WHITE
};

void vterm_configure_cursor(VTermCursor config) {
    active_cursor_style = config;
    
    if (!config.visible) {
        outb(CRTC_CMD_PORT, CURSOR_STYLE_START_CMD);
        outb(CRTC_DATA_PORT, 0x20);
        return;
    }

    uint8_t start_scanline = 14; 
    uint8_t end_scanline = 15;

    if (config.shape == CURSOR_SHAPE_BLOCK) {
        start_scanline = 0; 
    }

    outb(CRTC_CMD_PORT, CURSOR_STYLE_START_CMD);
    outb(CRTC_DATA_PORT, start_scanline & 0x1F);

    outb(CRTC_CMD_PORT, CURSOR_STYLE_END_CMD);
    outb(CRTC_DATA_PORT, end_scanline & 0x1F);

    vterm_move_cursor(cursor_row, cursor_col);
}

void vterm_move_cursor(uint16_t row, uint16_t col) {
    if (row < SCREEN_ROWS) cursor_row = row;
    if (col < SCREEN_COLS) cursor_col = col;
    
    if (active_cursor_style.visible) {
        uint16_t position = (cursor_row * SCREEN_COLS) + cursor_col;
        outb(CRTC_CMD_PORT, CURSOR_POS_HIGH_BYTE_CMD);
        outb(CRTC_DATA_PORT, (uint8_t)((position >> 8) & 0xFF));
        outb(CRTC_CMD_PORT, CURSOR_POS_LOW_BYTE_CMD);
        outb(CRTC_DATA_PORT, (uint8_t)(position & 0xFF));
    }
}

void vterm_set_buffering_mode(VTermBufferingMode mode) {
    current_mode = mode;
}

void vterm_init(VTermColor fg, VTermColor bg) {
    current_mode = BUFFER_MODE_STREAM;
    vterm_clear(fg, bg);
    vterm_flush();
    
    VTermCursor default_cursor = { .shape = CURSOR_SHAPE_LINE, .visible = true };
    vterm_configure_cursor(default_cursor);
}

void vterm_clear(VTermColor fg, VTermColor bg) {
    current_fg = fg;
    current_bg = bg;
    for (uint16_t r = 0; r < SCREEN_ROWS; r++) {
        for (uint16_t c = 0; c < SCREEN_COLS; c++) {
            screen_buffer[r][c].character = ' ';
            screen_buffer[r][c].fg = fg;
            screen_buffer[r][c].bg = bg;
        }
    }
    vterm_move_cursor(0, 0);
    
    if (current_mode == BUFFER_MODE_STREAM) {
        vterm_flush();
    }
}

static void vterm_scroll(void) {
    for (uint16_t r = 1; r < SCREEN_ROWS; r++) {
        for (uint16_t c = 0; c < SCREEN_COLS; c++) {
            screen_buffer[r - 1][c] = screen_buffer[r][c];
        }
    }
    for (uint16_t c = 0; c < SCREEN_COLS; c++) {
        screen_buffer[SCREEN_ROWS - 1][c].character = ' ';
        screen_buffer[SCREEN_ROWS - 1][c].fg = current_fg;
        screen_buffer[SCREEN_ROWS - 1][c].bg = current_bg;
    }
    vterm_move_cursor(SCREEN_ROWS - 1, 0);
    
    if (current_mode == BUFFER_MODE_STREAM) {
        vterm_flush();
    }
}

void vterm_write_char(char c) {
    uint16_t target_row = cursor_row;
    uint16_t target_col = cursor_col;

    if (c == '\n') {
        vterm_move_cursor(cursor_row + 1, 0);
    } else if (c == '\b') {
        if (cursor_col > 0) {
            vterm_move_cursor(cursor_row, cursor_col - 1);
            screen_buffer[cursor_row][cursor_col].character = ' ';
            screen_buffer[cursor_row][cursor_col].fg = current_fg;
            screen_buffer[cursor_row][cursor_col].bg = current_bg;
            
            if (current_mode == BUFFER_MODE_STREAM) {
                write_char_to_framebuffer(' ', cursor_row, cursor_col, vga_color_lookup[current_fg], vga_color_lookup[current_bg]);
            }
        } else if (cursor_row > 0) {
            vterm_move_cursor(cursor_row - 1, SCREEN_COLS - 1);
        }
    } else {
        if (cursor_row < SCREEN_ROWS && cursor_col < SCREEN_COLS) {
            screen_buffer[cursor_row][cursor_col].character = c;
            screen_buffer[cursor_row][cursor_col].fg = current_fg;
            screen_buffer[cursor_row][cursor_col].bg = current_bg;
            vterm_move_cursor(cursor_row, cursor_col + 1);
        }
        if (cursor_col >= SCREEN_COLS) {
            vterm_move_cursor(cursor_row + 1, 0);
        }
    }

    if (cursor_row >= SCREEN_ROWS) {
        vterm_scroll();
    } else {
        if (current_mode == BUFFER_MODE_STREAM && c != '\n') {
            write_char_to_framebuffer(c, target_row, target_col, vga_color_lookup[current_fg], vga_color_lookup[current_bg]);
        }
    }
    
    vterm_move_cursor(cursor_row, cursor_col);
}

void vterm_write(const char* str) {
    for (uint32_t i = 0; str[i] != '\0'; i++) {
        vterm_write_char(str[i]);
    }
}

void vterm_write_binary(uint8_t byte) {
    for (uint16_t bit = 0; bit < 8; bit++) {
        uint8_t mask = (uint8_t)0x1 << (7 - bit);
        vterm_write_char((byte & mask) ? '1' : '0');
    }
}

void vterm_write_hex(uint32_t val, VTermColor fg, VTermColor bg) {
    char hex_chars[] = "0123456789ABCDEF";
    VTermColor old_fg = current_fg;
    VTermColor old_bg = current_bg;
    
    vterm_set_color(fg, bg);
    vterm_write_char('0');
    vterm_write_char('x');
    
    for (int i = 0; i < 8; i++) {
        uint8_t nibble = (val >> (28 - (i * 4))) & 0xF;
        vterm_write_char(hex_chars[nibble]);
    }
    
    vterm_set_color(old_fg, old_bg);
}

void vterm_print_at(uint16_t row, uint16_t col, const char* text) {
    vterm_move_cursor(row, col);
    vterm_write(text);
    
    if (current_mode == BUFFER_MODE_STREAM) {
        vterm_flush();
    }
}

void vterm_print_aligned(uint16_t row, const char* text, VTermAlignment align) {
    uint32_t len = strlen(text);
    uint16_t col = 0;

    switch (align) {
        case VTERM_ALIGN_LEFT:
            col = 0;
            break;
        case VTERM_ALIGN_CENTER:
            col = (len < SCREEN_COLS) ? (SCREEN_COLS - len) / 2 : 0;
            break;
        case VTERM_ALIGN_RIGHT:
            col = (len < SCREEN_COLS) ? (SCREEN_COLS - len) : 0;
            break;
    }

    vterm_print_at(row, col, text);
}

void vterm_set_color(VTermColor fg, VTermColor bg) {
    current_fg = fg;
    current_bg = bg;
}

uint16_t vterm_get_cursor_row(void) {
    return cursor_row;
}

uint16_t vterm_get_cursor_col(void) {
    return cursor_col;
}

void vterm_flush(void) {
    for (uint16_t r = 0; r < SCREEN_ROWS; r++) {
        for (uint16_t c = 0; c < SCREEN_COLS; c++) {
            VTermCell cell = screen_buffer[r][c];
            write_char_to_framebuffer(cell.character, r, c, vga_color_lookup[cell.fg], vga_color_lookup[cell.bg]);
        }
    }
    vterm_move_cursor(cursor_row, cursor_col);
}