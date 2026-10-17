#include <genesis.h>

/* GCC 16's generic m68k libgcc archive contains 32-bit division helpers with
   BSR.L instructions. Those are not valid on the Genesis 68000, which treats
   the 0xFF displacement as a short branch. Supply equivalent 68000-safe
   helpers so ordinary C / and % expressions cannot branch into odd ROM bytes. */
static u32 divideUnsigned32(u32 numerator, u32 denominator, u32 *remainder)
{
    u32 quotient = 0;
    u32 rest = 0;
    s16 bit;

    if (denominator == 0)
    {
        if (remainder) *remainder = numerator;
        return 0;
    }

    for (bit = 31; bit >= 0; bit--)
    {
        u32 carry = numerator >> 31;
        bool overflow = (rest & 0x80000000UL) != 0;
        numerator <<= 1;
        rest = (rest << 1) | carry;
        quotient <<= 1;

        if (overflow || rest >= denominator)
        {
            rest -= denominator;
            quotient |= 1;
        }
    }

    if (remainder) *remainder = rest;
    return quotient;
}

__attribute__((used, externally_visible, noinline))
u32 __udivsi3(u32 numerator, u32 denominator)
{
    return divideUnsigned32(numerator, denominator, 0);
}

__attribute__((used, externally_visible, noinline))
u32 __umodsi3(u32 numerator, u32 denominator)
{
    u32 remainder;
    divideUnsigned32(numerator, denominator, &remainder);
    return remainder;
}

__attribute__((used, externally_visible, noinline))
s32 __divsi3(s32 numerator, s32 denominator)
{
    bool negative = (numerator < 0) != (denominator < 0);
    u32 unsignedNumerator = numerator < 0 ? 0UL - (u32)numerator : (u32)numerator;
    u32 unsignedDenominator = denominator < 0 ? 0UL - (u32)denominator : (u32)denominator;
    u32 quotient = divideUnsigned32(unsignedNumerator, unsignedDenominator, 0);

    return negative ? (s32)(0UL - quotient) : (s32)quotient;
}

__attribute__((used, externally_visible, noinline))
s32 __modsi3(s32 numerator, s32 denominator)
{
    u32 unsignedNumerator = numerator < 0 ? 0UL - (u32)numerator : (u32)numerator;
    u32 unsignedDenominator = denominator < 0 ? 0UL - (u32)denominator : (u32)denominator;
    u32 remainder;
    divideUnsigned32(unsignedNumerator, unsignedDenominator, &remainder);

    return numerator < 0 ? (s32)(0UL - remainder) : (s32)remainder;
}
