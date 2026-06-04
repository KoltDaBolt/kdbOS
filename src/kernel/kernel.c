#include "types.h"
#include "io.h"
#include "string.h"

#define DELAY_SHORT     4000000U
#define DELAY_MEDIUM    30000000U
#define DELAY_LONG      150000000U

static void wait(uint32_t count) {
    for (volatile uint32_t i = 0; i < count; i++);
}

void draw_border(uint16_t row, uint8_t color) {
    for (uint16_t i = 0; i < SCREEN_COLS / 2; i++) {
        write_char_to_framebuffer('=', row, i, color, BLACK);
        write_char_to_framebuffer('=', row, SCREEN_COLS - 1 - i, color, BLACK);
        wait(DELAY_SHORT);
    }
}

void draw_letter(const char* string, uint16_t row, uint16_t col, uint8_t fg_color, uint8_t bg_color) {
    for (uint16_t i = 0; string[i] != '\0'; i++) {
        write_char_to_framebuffer(string[i], row, col + i, fg_color, bg_color);
        wait(DELAY_MEDIUM);
    }
}

int kernel_main() {
    style_cursor(DISABLE);

    const char* message = "[ Welcome to kdbOS v0.0.0 ]";
    const char* subtitle = "* * *";
    
    uint16_t msg_len = strlen(message);
    uint16_t sub_len = strlen(subtitle);

    const uint16_t msg_row = 11;
    const uint16_t msg_col = (SCREEN_COLS - msg_len) / 2;
    const uint16_t sub_col = (SCREEN_COLS - sub_len) / 2;

    draw_border(msg_row - 3, GREEN);
    wait(DELAY_LONG);

    draw_letter(subtitle, msg_row - 1, sub_col, GREEN, BLACK);
    wait(DELAY_LONG);

    draw_letter(message, msg_row, msg_col, LIGHTGREEN, BLACK);
    wait(DELAY_LONG);

    draw_letter(subtitle, msg_row + 1, sub_col, GREEN, BLACK);
    wait(DELAY_LONG);

    draw_border(msg_row + 3, GREEN);
    wait(DELAY_LONG);

    return 0;
}
