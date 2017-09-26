/* ===-- fixdfdi.c - Implement __fixdfdi -----------------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 */

#define DOUBLE_PRECISION
#include "fp_lib.h"

#ifndef __SOFT_FP__
/* Support for systems that have hardware floating-point; can set the invalid
 * flag as a side-effect of computation.
 */

COMPILER_RT_ABI du_int __fixunsdfdi(double a);

COMPILER_RT_ABI di_int
__fixdfdi(double a)
{
    if (a < 0.0) {
        return -__fixunsdfdi(-a);
    }
    return __fixunsdfdi(a);
}

#else
/* Support for systems that don't have hardware floating-point; there are no
 * flags to set, and we don't want to code-gen to an unknown soft-float
 * implementation.
 */

typedef di_int fixint_t;
typedef du_int fixuint_t;
#include "fp_fixint_impl.inc"

COMPILER_RT_ABI di_int
__fixdfdi(fp_t a) {
    return __fixint(a);
}

#endif

#if defined(__ARM_EABI__)
AEABI_RTABI di_int
#if defined(__SOFT_FP__)
__aeabi_d2lz(fp_t a) {
#else
__aeabi_d2lz(double a) {
#endif
  return __fixdfdi(a);
}
#endif

