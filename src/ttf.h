#ifndef _TTF_H_
#define _TTF_H_

/* FREETYPE IS SLOW
 * when using, blit to a surface and _store in memory for later use_! don't
 * render on-the-fly or it will be morbidly inefficient
 */

#include <nds.h>
#include <map>
#include <ft2build.h>
#include FT_FREETYPE_H
#include "fastmem.h"

#define PIXELSIZE 12
#define CACHESIZE 512
#define LINESPACING 0

#define MARGIN_X_DEFAULT 0
#define MARGIN_Y_DEFAULT 0
#define PEN_X_DEFAULT (0 + (MARGIN_X_DEFAULT))
#define PEN_Y_DEFAULT (0 + (MARGIN_Y_DEFAULT))
/* technically PEN_Y_DEFAULT can't be defined, but Text functions check for
 * this and adjust it accordingly */
#define TEXT_COLOR_DEFAULT (RGB15(31,31,31)) /* white */

using namespace std;

class NDSBUF;

enum TEXTMODE {
    TEXTMODE_NONE = 0x0,        // no special properties
    // TEXTMODE_DEFAULT IS NOT IMPLEMENTED
    TEXTMODE_DEFAULT = 0x1,     // ignore data, use TEXTSTYLE_DEFAULT
                                // reverse effect: use struct's data
    TEXTMODE_AUTOSHIFT = 0x2,   // shift text up (and clip) for more space
                                // reverse effect: don't blit any more
};

struct TextStyle {
    TEXTMODE mode;
    FT_Vector pen;
    u16 color;

    TextStyle() {
        mode = TEXTMODE_NONE; pen.x = PEN_X_DEFAULT; pen.y = PEN_Y_DEFAULT;
        color = TEXT_COLOR_DEFAULT; }

    TextStyle(TextStyle *a = NULL) {
        if(a == NULL) {
            mode = TEXTMODE_NONE; pen.x = PEN_X_DEFAULT; pen.y = PEN_Y_DEFAULT;
            color = TEXT_COLOR_DEFAULT;
        }
        else { mode = a->mode; pen = a->pen; color = a->color; } };
};

class Text {
    // use one library instance for all Text instances
    static FT_Library library;

    private:
        FT_Face face;
        // NOTE: different sizes on same font.. could be a RAM hog
        FT_GlyphSlotRec glyphs[CACHESIZE];	
        u16 cache_ucs[CACHESIZE];
        FT_Vector pen;
        FT_Error error;

        // associates each glyph cache index (value)
        // with it's Unicode code point (key).
        map<u16,u16> cachemap;
        u16 cachenext;

        char *fontfilename;
        u8 pixelsize;

    public:
        Text(char *);
        ~Text();
        int InitDefault();

        u8 GetAdvance(u16 code);
        u8 GetCharCode(const char *, u16 *) const;
        u8 GetHeight(void) const;
        u8 GetPixelSize() const;
        u8 GetStringWidth(const char *);

        void SetPixelSize(u8);

        int CacheGlyph(u16 codepoint);
        FT_GlyphSlot GetGlyph(u16 ucs, int);
        void ClearCache();
        void CacheString(const char *);

        unsigned int PrintStringWrapLines(NDSBUF *, const char *);

        void PrintChar(NDSBUF *, u16);
        bool PrintNewLine(NDSBUF *); // false = did not print new line
        void PrintString(NDSBUF *, const char *);
        void PrintStringWrap(NDSBUF *, const char *);
} ;

#endif /* _TTF_H_ */
