#pragma once

#include "paging.h"

void* vmm_get_new_page(VirtualAddressSpace*, bool, bool, void*);
void vmm_free_page(VirtualAddressSpace*, void*);
bool vmm_map_frame(VirtualAddressSpace*, void*, uint32_t, bool, bool);
VirtualAddressSpace* vmm_create_virtual_address_space(void*, size_t, bool);
uint32_t vmm_get_physical_address(VirtualAddressSpace* space, void* virtual_address);
VirtualAddressSpace* vmm_create_kernel_thread_space(void);