#pragma once

#include "types.h"
#include "paging.h"

typedef struct {
    uint32_t eflags;
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp_dummy;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t eip;
} __attribute__((packed)) CpuContext;

void* create_process_stack(void*, uint32_t*);