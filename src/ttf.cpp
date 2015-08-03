#include <nds.h>
#include <fat.h>
#include "ttf.h"
#include "ndsbuf.h"
#include "alpha.h"

FT_Library Text::library = NULL;

Text::Text(char *fontpath)
{
    if(!library) {
        if(FT_Init_FreeType(&library))
            library = NULL;
    }

    error = 0;
    cachenext = 0;
    pixelsize = PIXELSIZE;
    fontfilename = new char[strlen(fontpath)];
    strncpy(fontfilename, fontpath, strlen(fontpath));
}

Text::~Text()
{
    delete fontfilename;
}

int Text::InitDefault()
{
    if(!library) // FT_Init_FreeType failed
        return 1;
    if(FT_New_Face(library, fontfilename, 0, &face))
        return 2;

    FT_Select_Charmap(face, FT_ENCODING_UNICODE);
    FT_Set_Pixel_Sizes(face, 0, pixelsize);

    return 0;
}

int Text::CacheGlyph(u16 ucs)
{
    // Cache glyph at ucs if there's space.
    // Does not check if this is a duplicate entry.

    if(cachenext == CACHESIZE) return -1;

    FT_Load_Char(face, ucs,
        FT_LOAD_RENDER|FT_LOAD_TARGET_NORMAL);
    FT_GlyphSlot src = face->glyph;
    FT_GlyphSlot dst = &glyphs[cachenext];
    int x = src->bitmap.rows;
    int y = src->bitmap.width;
    dst->bitmap.buffer = new unsigned char[x*y];
    memcpy(dst->bitmap.buffer, src->bitmap.buffer, x*y);
    dst->bitmap.rows = src->bitmap.rows;
    dst->bitmap.width = src->bitmap.width;
    dst->bitmap_top = src->bitmap_top;
    dst->bitmap_left = src->bitmap_left;
    dst->advance = src->advance;
    cache_ucs[cachenext] = ucs;
    cachenext++;
    return cachenext-1;
}

FT_GlyphSlot Text::GetGlyph(u16 ucs, int flags)
{
    int i;
    for(i=0;i<cachenext;i++)
    {
        if(cache_ucs[i] == ucs) return &glyphs[i];
    }

    i = CacheGlyph(ucs);
    if(i > -1) return &glyphs[i];

    FT_Load_Char(face, ucs, flags);
    return face->glyph;
}

void Text::ClearCache()
{
    cachenext = 0;
}

void Text::CacheString(const char *string)
{
    // this probably isn't that efficient, but it's called once at the start so
    // people can wait - why cache so much at the start?
    // 20:36:16 <xyz`> I tried turning it off once
    // 20:36:17 <Alvin^>       but 160 is ungodly
    // 20:36:18 <xyz`> OH GOD SLOW
    u8 i;
    for (i=0;i<strlen((char *)string);i++) {
        u16 c = string[i];
        if(c == '\n') continue;
        else {
            if (c > 127) {
                /** this guy is multibyte UTF-8. **/
                i+=GetCharCode(&(string[i]),&c);
                i--;
            }
            int n;
            for(n=0;n<cachenext;n++)
                if(cache_ucs[i] == c) break;
            if(n == cachenext) // needs caching
                CacheGlyph(c);
        }
    }
}

u8 Text::GetStringWidth(const char *str)
{
    u8 width = 0;
    const char *c;
    for(c = str; c != NULL; c++) {
        u16 ucs;
        GetCharCode(c, &ucs);
        width += GetAdvance(ucs);
    }

    return width;
}

u8 Text::GetCharCode(const char *utf8, u16 *ucs) const
{
    // given a UTF-8 encoding, fill in the Unicode/UCS code point.
    // returns the bytelength of the encoding, for advancing
    // to the next character.
    // returns 0 if encoding could not be translated.
    // TODO - handle 4 byte encodings.

    if (utf8[0] < 0x80) { // ASCII
        *ucs = utf8[0];
        return 1;

    } else if (utf8[0] > 0xc1 && utf8[0] < 0xe0) { // latin
        *ucs = ((utf8[0]-192)*64) + (utf8[1]-128);
        return 2;

    } else if (utf8[0] > 0xdf && utf8[0] < 0xf0) { // asian
        *ucs = (utf8[0]-224)*4096 + (utf8[1]-128)*64 + (utf8[2]-128);
        return 3;

    } else if (utf8[0] > 0xef) { // rare
        return 4;

    }
    return 0;
}

u8 Text::GetHeight() const
{
    return (face->size->metrics.height >> 6);
}

u8 Text::GetPixelSize() const
{
    return pixelsize;
}

void Text::SetPixelSize(u8 size)
{
    if(!size) {
        FT_Set_Pixel_Sizes(face, 0, PIXELSIZE);
        pixelsize = PIXELSIZE;
    }
    else {
        FT_Set_Pixel_Sizes(face, 0, size);
        pixelsize = size;
    }

    ClearCache();
}

u8 Text::GetAdvance(u16 ucs) {
    // Caches this glyph if possible.
    return GetGlyph(ucs, FT_LOAD_DEFAULT)->advance.x >> 6;
}

void Text::PrintChar(NDSBUF *buf, u16 ucs)
{
    // Draw a character for the given UCS codepoint,
    // into the current screen buffer at the current pen position.

    // Consult the cache for glyph data and cache it on a miss
    // if space is available.
    TextStyle *style = &buf->textstyle;
    if(style->pen.y == PEN_Y_DEFAULT) style->pen.y += GetHeight();

    FT_GlyphSlot glyph = GetGlyph(ucs, FT_LOAD_RENDER|FT_LOAD_TARGET_NORMAL);

    // check if the printing goes out of bounds
    if((unsigned int) (style->pen.x + (glyph->advance.x >> 6)) > buf->w() -
            MARGIN_X_DEFAULT) {
        if(!PrintNewLine(buf)) return;
        style->pen.x = PEN_X_DEFAULT;
    }

    FT_Bitmap bitmap = glyph->bitmap;

    u16 *dst = buf->GetBuf() + buf->rw() * (buf->y() + style->pen.y -
            glyph->bitmap_top) + buf->x() + glyph->bitmap_left + style->pen.x;

    for(u16 gy = 0; gy < bitmap.rows; gy++) {
        for(u16 gx = 0; gx < bitmap.width; gx++) {
            u16 a = bitmap.buffer[gy * bitmap.width + gx] & 0xFF;
            if(a) *dst = AlphaBlend(*dst, style->color, a);
            dst++;
        }

        dst += buf->rw() - bitmap.width;
    }

    style->pen.x += glyph->advance.x >> 6;
}

bool Text::PrintNewLine(NDSBUF *buf)
{
    TextStyle *style = &buf->textstyle;
    if(style->pen.y == PEN_Y_DEFAULT) style->pen.y += GetHeight();
    style->pen.x = PEN_X_DEFAULT;

    unsigned int height = face->size->metrics.height >> 6;
    unsigned int y = style->pen.y + height + LINESPACING;
    u16 *dst = buf->GetBuf() + buf->rw() * buf->y() + buf->x();
    size_t len = buf->w();

    if(y > buf->h()) {
        if(!(style->mode & TEXTMODE_AUTOSHIFT)) return false;
        unsigned int shifty = 0;
        for( ; shifty < buf->h()-height-LINESPACING; shifty++) {
            fastcpy(dst, dst+buf->rw()*(height+LINESPACING), len*2);
            dst += buf->rw();
        }

        for( ; shifty < buf->h(); shifty++) {
            fastset16(dst, buf->GetFillColor(), len);
            dst += len;
        }
    }
    else buf->textstyle.pen.y += height + LINESPACING;

    return true;
}

void Text::PrintString(NDSBUF *buf, const char *string)
{
    // draw a character string starting at the pen position.
    for(unsigned int i = 0; i < strlen((char *) string); i++) {
        u16 c = string[i];
        if(c == '\n') PrintNewLine(buf);
        else {
            if(c > 127) {
                /* this guy is multibyte UTF-8. */
                i += GetCharCode(&(string[i]),&c);
                i--;
            }

            PrintChar(buf, c);
        }
    }
}

// if it reaches the very end of the line it doesn't count
unsigned int Text::PrintStringWrapLines(NDSBUF *buf, const char *str)
{
    // draw a character string starting at the pen position.
    char* word;
    unsigned int x;
    char string[strlen(str)];
    strcpy(string, str);
    word = strtok((char *) string, " ");
    int lines = 1;
    x = 0;
    while(word) {
        for(unsigned int i = 0; i < strlen(word); ++i)
            x += GetAdvance(word[i]);

        x += GetAdvance(' ');
        if(x > buf->w()) {
            ++lines;
            x = 0;
        }
        else word = strtok(NULL, " ");
    }

    return lines;
}

// pretty much on;y works with same-width characters (monospace)
// TODO: variable width support
void Text::PrintStringWrap(NDSBUF *buf, const char *str)
{
    // draw a character string starting at the pen position and wrap text
    char* word;
    unsigned int x;
    u16 ch;
    char string[strlen(str)];
    strcpy(string, str);
    word = strtok((char *) string, " ");
    while(word) {
        x = buf->textstyle.pen.x;
        for(unsigned int i = 0; i < strlen(word); i++) {
            if(*(word+i) == '\n') {
                word++;
                PrintNewLine(buf);
                break;
            }

            GetCharCode(word + i, &ch);
            x += GetAdvance(ch);
        }

        if(x > buf->w()) PrintNewLine(buf);

        PrintString(buf, word);
        word = strtok(NULL, " ");
        if(word) PrintString(buf, " ");
    }
}
