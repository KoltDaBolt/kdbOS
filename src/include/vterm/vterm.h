#pragma once

#include "types.h"

#define SCREEN_COLS 80
#define SCREEN_ROWS 25

typedef enum {
    COLOR_BLACK,
    COLOR_BLUE,
    COLOR_GREEN,
    COLOR_CYAN,
    COLOR_RED,
    COLOR_MAGENTA,
    COLOR_BROWN,
    COLOR_LIGHTGREY,
    COLOR_DARKGREY,
    COLOR_LIGHTBLUE,
    COLOR_LIGHTGREEN,
    COLOR_LIGHTCYAN,
    COLOR_LIGHTRED,
    COLOR_LIGHTMAGENTA,
    COLOR_LIGHTBROWN,
    COLOR_WHITE
} VTermColor;

typedef enum {
    VTERM_ALIGN_LEFT,
    VTERM_ALIGN_CENTER,
    VTERM_ALIGN_RIGHT
} VTermAlignment;

typedef struct {
    char character;
    uint8_t fg;
    uint8_t bg;
} VTermCell;

typedef enum {
    CURSOR_SHAPE_LINE,
    CURSOR_SHAPE_BLOCK
} CursorShape;

typedef struct {
    CursorShape shape;
    bool visible;
} VTermCursor;

typedef enum {
    BUFFER_MODE_STREAM,
    BUFFER_MODE_BLOCK
} VTermBufferingMode;

void vterm_set_cursor_visible(bool);
void vterm_set_cursor_shape(CursorShape);
void vterm_move_cursor(uint16_t, uint16_t);
uint16_t vterm_get_cursor_row(void);
uint16_t vterm_get_cursor_col(void);
VTermCursor vterm_get_cursor_config(void);
bool vterm_is_cursor_active(void);

void vterm_set_buffering_mode(VTermBufferingMode);

void vterm_init(VTermColor, VTermColor);
void vterm_clear(VTermColor, VTermColor);
void vterm_write_char(char);
void vterm_write(const char*);
void vterm_write_binary(uint8_t);
void vterm_write_hex(uint32_t, VTermColor, VTermColor);

void vterm_print_at(uint16_t, uint16_t, const char*);
void vterm_print_aligned(uint16_t, const char*, VTermAlignment);
void vterm_set_color(VTermColor, VTermColor);


void vterm_flush(void);

static inline void kprint(const char* str) { vterm_write(str); }
static inline void kprint_c(char c) { vterm_write_char(c); }
static inline void kprint_hex(uint32_t val, VTermColor fg, VTermColor bg) { vterm_write_hex(val, fg, bg); }

static inline void kprint_at(uint16_t r, uint16_t c, const char* text) { vterm_print_at(r, c, text); }
static inline void kprint_aligned(uint16_t r, const char* text, VTermAlignment align) { vterm_print_aligned(r, text, align); }