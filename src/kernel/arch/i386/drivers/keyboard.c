#include "keyboard.h"
#include "vterm.h"
#include "pic.h"
#include "asm.h"

static bool shift_pressed = false;
static bool caps_lock_active = false;
static bool is_extended = false;

static const char kbd_us_map[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
  '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0,  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',   0,
 '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',   0, '*',   0, ' ',
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
};

static const char kbd_us_shifted_map[128] = {
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
  '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0,  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',   0,
 '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?',   0, '*',   0, ' ',
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
};

void keyboard_handler() {
    uint8_t status = inb(KEYBOARD_STATUS_PORT);

    if (status & 0x01) {
        uint8_t scancode = inb(KEYBOARD_DATA_PORT);

        pic_send_eoi(1);

        if (scancode == 0xE0) {
            is_extended = true;
            return;
        }

        if (is_extended) {
            is_extended = false;

            if (scancode & 0x80) return;

            switch (scancode) {
                case 0x48: // Up Arrow
                    vterm_move_cursor(vterm_get_cursor_row() - 1, vterm_get_cursor_col());
                    break;
                case 0x50: // Down Arrow
                    vterm_move_cursor(vterm_get_cursor_row() + 1, vterm_get_cursor_col());
                    break;
                case 0x4B: // Left Arrow
                    vterm_move_cursor(vterm_get_cursor_row(), vterm_get_cursor_col() - 1);
                    break;
                case 0x4D: // Right Arrow
                    vterm_move_cursor(vterm_get_cursor_row(), vterm_get_cursor_col() + 1);
                    break;
            }
            return;
        }

        if (scancode & 0x80) {
            uint8_t released_scancode = scancode & 0x7F;
            
            if (released_scancode == 0x2A || released_scancode == 0x36) {
                shift_pressed = false;
            }
            return;
        } 
        else {
            if (scancode == 0x2A || scancode == 0x36) {
                shift_pressed = true;
                return;
            }
            if (scancode == 0x3A) {
                caps_lock_active = !caps_lock_active;
                return;
            }

            if (scancode >= 128) return;

            char ascii = 0;
            bool use_uppercase = shift_pressed;
            
            char base_char = kbd_us_map[scancode];
            if (base_char >= 'a' && base_char <= 'z') {
                if (caps_lock_active) {
                    use_uppercase = !shift_pressed;
                }
            }

            if (use_uppercase) {
                ascii = kbd_us_shifted_map[scancode];
            } else {
                ascii = kbd_us_map[scancode];
            }
            
            if (ascii != 0) {
                vterm_write_char(ascii);
            }
        }
    }
}

void init_keyboard(void) {
    register_irq_handler(33, keyboard_handler);
}