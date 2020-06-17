/*
 * \file       trc_core_arch_map.cpp
 * \brief      OpenCSD : Map core names to architecture profiles
 * 
 * \copyright  Copyright (c) 2015, ARM Limited. All Rights Reserved.
 */

/* 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, 
 * this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice, 
 * this list of conditions and the following disclaimer in the documentation 
 * and/or other materials provided with the distribution. 
 * 
 * 3. Neither the name of the copyright holder nor the names of its contributors 
 * may be used to endorse or promote products derived from this software without 
 * specific prior written permission. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 'AS IS' AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND 
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */ 

#include "common/trc_core_arch_map.h"

typedef struct _ap_map_elements {
    const char *name;
    ocsd_arch_profile_t ap;
} ap_map_elem_t;

static ap_map_elem_t ap_map_array[] = 
{
    { "Cortex-A77", { ARCH_V8r3, profile_CortexA } },
    { "Cortex-A76", { ARCH_V8r3, profile_CortexA } },
    { "Cortex-A75", { ARCH_V8r3, profile_CortexA } },
    { "Cortex-A73", { ARCH_V8, profile_CortexA } },
    { "Cortex-A72", { ARCH_V8, profile_CortexA } },
    { "Cortex-A65", { ARCH_V8r3, profile_CortexA } },
    { "Cortex-A57", { ARCH_V8, profile_CortexA } },
    { "Cortex-A55", { ARCH_V8r3, profile_CortexA } },
    { "Cortex-A53", { ARCH_V8, profile_CortexA } },
    { "Cortex-A35", { ARCH_V8, profile_CortexA } },
    { "Cortex-A32", { ARCH_V8, profile_CortexA } },
    { "Cortex-A17", { ARCH_V7, profile_CortexA } },
    { "Cortex-A15", { ARCH_V7, profile_CortexA } },
    { "Cortex-A12", { ARCH_V7, profile_CortexA } },
    { "Cortex-A9", { ARCH_V7, profile_CortexA } },
    { "Cortex-A8", { ARCH_V7, profile_CortexA } },
    { "Cortex-A7", { ARCH_V7, profile_CortexA } },
    { "Cortex-A5", { ARCH_V7, profile_CortexA } },
    { "Cortex-R52", { ARCH_V8, profile_CortexR } },
    { "Cortex-R8", { ARCH_V7, profile_CortexR } },
    { "Cortex-R7", { ARCH_V7, profile_CortexR } },
    { "Cortex-R5", { ARCH_V7, profile_CortexR } },
    { "Cortex-R4", { ARCH_V7, profile_CortexR } },
    { "Cortex-M33", { ARCH_V8, profile_CortexM } },
    { "Cortex-M23", { ARCH_V8, profile_CortexM } },
    { "Cortex-M0", { ARCH_V7, profile_CortexM } },
    { "Cortex-M0+", { ARCH_V7, profile_CortexM } },
    { "Cortex-M3", { ARCH_V7, profile_CortexM } },
    { "Cortex-M4", { ARCH_V7, profile_CortexM } }
};   

static ap_map_elem_t arch_map_array[] = 
{
    { "ARMv7-A", { ARCH_V7, profile_CortexA } },
    { "ARMv7-R", { ARCH_V7, profile_CortexR } },
    { "ARMv7-M", { ARCH_V7, profile_CortexM } },
    { "ARMv8-A", { ARCH_V8, profile_CortexA } },
    { "ARMv8.3-A", { ARCH_V8r3, profile_CortexA } },
    { "ARMv8-R", { ARCH_V8, profile_CortexR } },
    { "ARMv8-M", { ARCH_V8, profile_CortexM } },
};

CoreArchProfileMap::CoreArchProfileMap()
{
    unsigned i;
    for (i = 0; i < sizeof(ap_map_array) / sizeof(_ap_map_elements); i++)
    {
        core_profiles[ap_map_array[i].name] = ap_map_array[i].ap;
    }
    for (i = 0; i < sizeof(arch_map_array) / sizeof(_ap_map_elements); i++)
    {
        arch_profiles[arch_map_array[i].name] = arch_map_array[i].ap;
    }
}

/* End of File trc_core_arch_map.cpp */
