/*
 * MD header for contrib/gdtoa
 *
 * $FreeBSD$
 */

/*
 * NOTE: The definitions in this file must be correct or strtod(3) and
 * floating point formats in printf(3) will break!  The file can be
 * generated by running contrib/gdtoa/arithchk.c on the target
 * architecture.  See contrib/gdtoa/gdtoaimp.h for details.
 */

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define IEEE_8087
#define Arith_Kind_ASL 1
#else
#define IEEE_MC68k
#define Arith_Kind_ASL 2
#endif
#define Long int
#define Intcast (int)(long)
#define Double_Align
#define X64_bit_pointers
