#ifndef _NDSBUF_H_
#define _NDSBUF_H_

#include <nds.h>
#include <stdlib.h>
#include <string.h>
#include "ttf.h"

/* gcc auto-inlines in-class functions; design here assumes the below
 * http://gcc.gnu.org/onlinedocs/gcc/Inline.html#Inline
 */

#define BUF_FILLCOLOR_DEFAULT (RGB15(0,0,0)|BIT(15))

class NDSBUF;

struct NDSRECT {
    unsigned int w, h;
    unsigned int x, y;

    NDSRECT() { w = h = x = y = 0; };
    // w, h, x, y
    NDSRECT(unsigned int, unsigned int, unsigned int = 0, unsigned int = 0);
    NDSRECT(NDSBUF *); // buf

    bool Error() const { return (w==0||h==0) ? true : false; };
    bool Fit(NDSRECT *, bool = true);
};

enum NDSBUF_MODE {
    NDSBUF_MODE_NONE = 0x0,
    NDSBUF_MODE_OWNBUF = 0x1,
};

enum NDSBUF_ERROR {
    NDSBUF_ERROR_NOERROR = 0,
    NDSBUF_ERROR_GEOMETRY, // non-matching w, h, x or y
    NDSBUF_ERROR_BADALLOC, // memory allocation failed on buf
};

class NDSBUF {
    private:
        u16 *buf;
        NDSRECT rect;
        NDSRECT realrect;
        NDSBUF_MODE mode;
        u16 fillcolor;

        // error var
        NDSBUF_ERROR error;

    public: // vars
        // ttf stuff
        TextStyle textstyle;

    public: // functions
        NDSBUF(unsigned int, unsigned int, TextStyle * = NULL); // w, h, ts
        // buf, rect, real rect, ts
        NDSBUF(u16 *, NDSRECT, NDSRECT, TextStyle * = NULL);
        
        ~NDSBUF();

        // call only once post-constructor (will not change again)
        NDSBUF_ERROR GetError() const { return error; };

        u16 *GetBuf() const { return buf; };
        NDSRECT GetRect() const { return rect; };
        unsigned int w() const { return rect.w; };
        unsigned int h() const { return rect.h; };
        NDSRECT GetRealRect() const { return realrect; };
        unsigned int rw() const { return realrect.w; };
        unsigned int rh() const { return realrect.h; };
        NDSBUF_MODE GetMode() const { return mode; };
        unsigned int x() const { return rect.x; };
        unsigned int y() const { return rect.y; };
        u16 GetFillColor() const { return fillcolor; };
        void SetFillColor(u16 col) { fillcolor = col; };

        // leads to poor efficiency - avoid!
        inline u16& operator[](const unsigned int);

        // source, srcrect, dstrect, transp
        bool Blit(NDSBUF *, NDSRECT, NDSRECT, bool = false);
        bool Blit(NDSBUF *, bool = false); // source, transp
        bool Blit(u16 *, unsigned int, unsigned int, NDSRECT, NDSRECT, bool =
                false);
        bool Blit(u16 *, unsigned int, unsigned int, bool = false);

        bool Fill(NDSRECT, u16);
        bool Fill(NDSRECT);
        bool Fill(u16);
        bool Fill();
};

inline u16& NDSBUF::operator[](const unsigned int index)
{
    if(index >= rect.w*rect.h) return *(buf); // bad default, blame references
    return *(buf + realrect.w * (rect.y + index/rect.w) + rect.x +
        index%rect.w);
}

#endif /* _NDSBUF_H_ */
