#pragma once

#include "paging.h"

void vmm_map_page(VirtualAddressSpace*, uint32_t, uint32_t, bool, bool);