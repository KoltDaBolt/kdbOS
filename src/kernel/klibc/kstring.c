#include "kstring.h"

int64_t kstrlen(const char* str) {
    int64_t len = 0;

    while (*(str + len)) {
        len++;
    }

    return len;
}

char* kstrncpy(const char* src, char* dest, uint32_t n) {
    if (n == 0) return dest;

    uint32_t i = 0;
    while (src[i] != '\0' && i < (n - 1)) {
        dest[i] = src[i];
        i++;
    }
    
    dest[i] = '\0';
    return dest;
}

char* kstrncat(char* dest, const char* src, uint32_t n) {
    uint32_t dest_len = kstrlen(dest);
    
    if (dest_len >= n || n == 0) {
        return dest;
    }

    uint32_t i = 0;
    uint32_t max_allowed = n - dest_len - 1;

    while (src[i] != '\0' && i < max_allowed) {
        dest[dest_len + i] = src[i];
        i++;
    }
    
    dest[dest_len + i] = '\0';
    return dest;
}