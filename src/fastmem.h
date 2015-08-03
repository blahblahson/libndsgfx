#ifndef _FASTMEM_H_
#define _FASTMEM_H_

#include <nds.h>

//# Stuff you may not have yet.
typedef unsigned int uint;
#define BIT_MASK(len)       ( (1<<(len))-1 )
static inline u32 quad8(u8 x)   {   x |= x<<8; return x | x<<16;    }


//# Declarations and inlines.

void fastcpy(void *dst, const void *src, uint size);

void __fastset(void *dst, u32 fill, uint size);
static inline void fastset(void *dst, u8 src, uint size);
static inline void fastset16(void *dst, u16 src, uint size);
static inline void fastset32(void *dst, u32 src, uint size);


//! VRAM-safe memset, byte version. Size in bytes.
static inline void fastset(void *dst, u8 src, uint size)
{   __fastset(dst, quad8(src), size);               }

//! VRAM-safe memset, halfword version. Size in hwords.
static inline void fastset16(void *dst, u16 src, uint size)
{   __fastset(dst, src|src<<16, size*2);            }

//! VRAM-safe memset, word version. Size in words.
static inline void fastset32(void *dst, u32 src, uint size)
{   __fastset(dst, src, size*4);                    }

#endif /* _FASTMEM_H_ */
