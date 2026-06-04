#include "types.h"

int64_t strlen(const char* str) {
    int64_t len = 0;

    while (*(str + len)) {
        len++;
    }

    return len;
}
