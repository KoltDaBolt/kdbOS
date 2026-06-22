#pragma once

#include "types.h"

#define PMM_FRAME_SIZE 4096

uint32_t pmm_init(size_t, uint32_t*);
uint32_t pmm_allocate_frame(void);
void pmm_free_frame(uint32_t);