#include "ndsbuf.h"
#include "fastmem.h"

#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

NDSRECT::NDSRECT(unsigned int a, unsigned int b, unsigned int c,
        unsigned int d) : w(a), h(b), x(c), y(d)
{
}

NDSRECT::NDSRECT(NDSBUF *buf)
{
    w = buf->w();
    h = buf->h();
    x = y = 0;
}

bool NDSRECT::Fit(NDSRECT *src, bool fix)
{
    if(src->x >= w || src->y >= h) return false;
    if(fix) {
        src->w = MIN(src->w, w - src->x);
        src->h = MIN(src->h, h - src->y);
    }
    else {
        if(MIN(src->w, w-src->x) != src->w || MIN(src->h, h-src->y) != src->h)
            return false;
    }

    return true;
}

// NDSBUF from here
NDSBUF::NDSBUF(unsigned int w, unsigned int h, TextStyle *ts)
    : fillcolor(BUF_FILLCOLOR_DEFAULT), textstyle(ts)
{
    error = NDSBUF_ERROR_NOERROR;

    if(w == 0 || h == 0) {
        error = NDSBUF_ERROR_GEOMETRY;
        return;
    }

    buf = (u16 *) malloc(w*h*2);
    if(buf == NULL) {
        error = NDSBUF_ERROR_BADALLOC;
        return;
    }

    mode = NDSBUF_MODE_OWNBUF;
    rect = NDSRECT(w, h, 0, 0);
    realrect = NDSRECT(w, h, 0, 0);
}

NDSBUF::NDSBUF(u16 *abuf, NDSRECT arect, NDSRECT arrect, TextStyle *ts)
    : buf(abuf), rect(arect), realrect(arrect),
    fillcolor(BUF_FILLCOLOR_DEFAULT), textstyle(ts)
{
    error = NDSBUF_ERROR_NOERROR;

    if(rect.Error() || !realrect.Fit(&rect, false)) {
        error = NDSBUF_ERROR_GEOMETRY;
        return;
    }

    mode = NDSBUF_MODE_NONE;
}

NDSBUF::~NDSBUF()
{
    if(mode & NDSBUF_MODE_OWNBUF)
        delete buf;
}


bool NDSBUF::Blit(NDSBUF *source, NDSRECT srcrect, NDSRECT dstrect, bool
        transp)
{
    if(!dstrect.Fit(&srcrect)) return false;

    u16 *dst = buf + realrect.w * (rect.y + dstrect.y) + rect.x + dstrect.x;
    u16 *src = source->GetBuf() + source->rw() * (source->y() + srcrect.y) +
        source->x() + srcrect.x;

    size_t len = MIN(srcrect.w, dstrect.w);
    // replace these in your mind when reading this
    //const int dstinc = realrect.w;
    //const int srcinc = source->rw();

    if(transp) {
        for(unsigned int y = srcrect.h; y; y--) {
            for(unsigned int x = 0; x < len; x++) {
                if((*(src+x) >> 15) == 0) continue;
                *(dst+x) = *(src+x);
            }
            dst += realrect.w;
            src += source->rw();
        }
    }
    else {
        for(unsigned int i = srcrect.h; i; i--) {
            memmove(dst, src, len*2);
            dst += realrect.w;
            src += source->rw();
        }
    }

    return true;
}

bool NDSBUF::Blit(NDSBUF *source, bool transp)
{
    return Blit(source, NDSRECT(source->w(), source->h(), 0, 0),
            NDSRECT(rect.w, rect.h, 0, 0), transp);
}


bool NDSBUF::Fill(NDSRECT geom, u16 color)
{
    if(geom.x >= rect.w || geom.y >= rect.h) return false;
    geom.w = MIN(geom.w, rect.w);
    geom.h = MIN(geom.h, rect.h);
    u16 *dst = buf + realrect.w * (geom.y + rect.y) + geom.x + rect.x;
    //int dstinc = realrect.w;
    int len = geom.w - geom.x;
    for(unsigned int i = geom.y; i < geom.h; i++) {
        fastset16(dst, color, len);
        dst += realrect.w;
    }

    return true;
}

bool NDSBUF::Fill(NDSRECT geom)
{
    return Fill(geom, fillcolor);
}

bool NDSBUF::Fill(u16 color)
{
    return Fill(NDSRECT(rect.w, rect.h, 0, 0), color);
}

bool NDSBUF::Fill()
{
    return Fill(NDSRECT(rect.w, rect.h, 0, 0), fillcolor);
}
