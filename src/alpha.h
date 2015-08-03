#ifndef _ALPHA_H_
#define _ALPHA_H_

#include <nds.h>
#include "rgb.h"

inline u16 AlphaBlend(u16 dst, u16 src, u16 a)
{
    const int shift = 8; // alpha bits
    const int round = 1 << (shift - 1);

    u16 r = RGB_R(dst);
    r += ((RGB_R(src) - r) * a + round) >> shift;
    u16 g = RGB_G(dst);
    g += ((RGB_G(src) - g) * a + round) >> shift;
    u16 b = RGB_B(dst);
    b += ((RGB_B(src) - b) * a + round) >> shift;

    return (RGB15(r, g, b) | BIT(15));
}

#endif /* _ALPHA_H_ */
