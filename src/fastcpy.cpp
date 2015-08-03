#include <stdio.h>
#include "fastmem.h"

void fastcpy(void *dst, const void *src, uint size)
{
    if(size==0 || dst==NULL || src==NULL)
        return;

    uint count;
    u16 *dst16;     // hword destination
    u8  *src8;      // byte source

    // Ideal case: copy by 4x words. Leaves tail for later.
    if( ((u32)src|(u32)dst)%4==0 && size>=4)
    {
        u32 *src32= (u32*)src, *dst32= (u32*)dst;

        count= size/4;
        uint tmp= count&3;
        count /= 4;

        // Duff, bitch!
        switch(tmp) {
            do {    *dst32++ = *src32++;
        case 3:     *dst32++ = *src32++;
        case 2:     *dst32++ = *src32++;
        case 1:     *dst32++ = *src32++;
        case 0:     ; } while(count--);
        }

        // Check for tail
        size &= 3;
        if(size == 0)
            return;

        src8= (u8*)src32;
        dst16= (u16*)dst32;
    }
    else        // Unaligned.
    {
        uint dstOfs= (u32)dst&1;
        src8= (u8*)src;
        dst16= (u16*)(((uint) dst)-dstOfs);

        // Head: 1 byte.
        if(dstOfs != 0)
        {
            *dst16= (*dst16 & 0xFF) | *src8++<<8;
            dst16++;
            if(--size==0)
                return;
        }
    }

    // Unaligned main: copy by 2x byte.
    count= size/2;
    while(count--)
    {
        *dst16++ = src8[0] | src8[1]<<8;
        src8 += 2;
    }

    // Tail: 1 byte.
    if(size&1)
        *dst16= (*dst16 &~ 0xFF) | *src8;
}
