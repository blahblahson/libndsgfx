#include <nds.h>
#include <string.h>
#include <stdio.h>
#include <fat.h>
#include "ndsgfx.h"
#include "img.h"

int main()
{
    irqInit();
    irqEnable(IRQ_VBLANK);

    u16 *vb_main = (u16 *) BG_BMP_RAM(0);
    u16 *vb_sub = (u16 *) BG_BMP_RAM_SUB(0);

    videoSetMode(MODE_5_2D | DISPLAY_BG3_ACTIVE);	
    videoSetModeSub(MODE_5_2D | DISPLAY_BG3_ACTIVE);

    vramSetBankA(VRAM_A_MAIN_BG_0x06000000);
    vramSetBankC(VRAM_C_SUB_BG_0x06200000);

    BG3_CR = BG_BMP16_256x256 | BG_BMP_BASE(0);
    SUB_BG3_CR = BG_BMP16_256x256 | BG_BMP_BASE(0);

    // no rotation or shearing
    BG3_XDY = 0;
    BG3_XDX = 1 << 8;
    BG3_YDX = 0;
    BG3_YDY = 1 << 8;
    SUB_BG3_XDY = 0;
    SUB_BG3_XDX = 1 << 8;
    SUB_BG3_YDX = 0;
    SUB_BG3_YDY = 1 << 8;


    /*u16 somebuf[256*192];
    for(int i = 0; i < 256*192; i++)
        somebuf[i] = RGB15(31,0,0) | BIT(15);

    NDSBUF *buf = new NDSBUF(vb_main, NDSRECT(256,192,0,0), NDSRECT(256,192,0,0));
    NDSBUF *sbuf = new NDSBUF(vb_sub, NDSRECT(256,192,0,0), NDSRECT(256,192,0,0));

    u16 color = RGB15(31,0,0) | BIT(15);
    u16 subcolor = RGB15(31,0,0)|BIT(15);

    unsigned int x1, y1, x2, y2;
    x1 = x2 = y1 = y2 = 257;

    NDSBUF *tmpbuf = new NDSBUF(50, 50); //NDSBUF(vb_sub, NDSRECT(50,50,50,50), NDSRECT(256, 192));
    tmpbuf->Fill(subcolor);
    buf->Fill(RGB15(0,0,31)|BIT(15));
    tmpbuf->Blit(buf);
    buf->Fill(RGB15(31,0,31)|BIT(15));
    if(!buf->Blit(tmpbuf, NDSRECT(20,50,0,0), NDSRECT(100, 30, 50, 50), true)) while(1) 1+1;
    delete tmpbuf;

    while(1) {
        scanKeys();
        u32 keys = keysDown();

        if(keys & KEY_A) {
            buf->Fill(color);
            if(color & RGB15(31,0,0)) color = RGB15(0,0,31) | BIT(15);
            else color = RGB15(31,0,0) | BIT(15);
        }
        if(keys & KEY_TOUCH) {
            touchPosition pos = touchReadXY();

            if(x1 == 257 && x2 == 257) {
                x1 = pos.px;
                y1 = pos.py;
            }
            else {
                x2 = pos.px;
                y2 = pos.py;
                tmpbuf = new NDSBUF(vb_sub, NDSRECT(x1, y1, x1-x2, y1-y2), NDSRECT(256, 192));
                tmpbuf->Fill(subcolor);
                if(subcolor & RGB15(31,0,0)) subcolor = RGB15(0,0,31) | BIT(15);
                else subcolor = RGB15(31,0,0) | BIT(15);
                x1 = x2 = y1 = y2 = 257;
                delete tmpbuf;
            }
        }

        swiWaitForVBlank();
    }*/

    fatInitDefault();
/*
    Text *text = new Text("default.ttf");
    int n = text->InitDefault();
    if(n == 1) {
        fastset16(vb_main, RGB15(31,0,0)|BIT(15), 256*192);
        while(1) swiWaitForVBlank();
    }

    if(n == 2) {
        fastset16(vb_main, RGB15(0,0,31)|BIT(15), 256*192);
        while(1) swiWaitForVBlank();
    }

    text->SetPixelSize(12);

    NDSBUF *buf = new NDSBUF(vb_sub, NDSRECT(100,100,50,50), NDSRECT(256,192));
    buf->textstyle.mode = (TEXTMODE) (buf->textstyle.mode |
            TEXTMODE_AUTOSHIFT);
    buf->Fill(RGB15(0,0,0)|BIT(15));
    text->PrintString(buf, "混全帯幺九\n");
    text->PrintString(buf, "四暗刻単騎\n");
    text->PrintString(buf, "国士無双１３面");
    fastset16(vb_main, RGB15(0,31,0)|BIT(15), 256*192);

    while(1) {
        swiWaitForVBlank();
        scanKeys();
        u16 keys = keysDown();
        if(keys & KEY_A) {
            text->PrintStringWrap(buf, "\nnewline 1");
            break;
        }
    }

    while(1) {
        swiWaitForVBlank();
        scanKeys();
        u16 keys = keysDown();
        if(keys & KEY_A) {
            text->PrintStringWrap(buf, "\nnewline 2");
            break;
        }
    }

    while(1) {
        swiWaitForVBlank();
        scanKeys();
        u16 keys = keysDown();
        if(keys & KEY_A) {
            text->PrintStringWrap(buf, "\nnewline i");
        }
    }*/

    NDSBUF *buf = new NDSBUF(vb_main, NDSRECT(256, 192), NDSRECT(256, 192));
    ImageMonolith *imagemonolith = new ImageMonolith();
    imagemonolith->LoadAll();
    Image imgdata = imagemonolith->GetImage(IMG_ENUM_TESTIMG);
    NDSBUF *img = new NDSBUF(imgdata.data, NDSRECT(imgdata.w, imgdata.h), NDSRECT(imgdata.w, imgdata.h));
    buf->Blit(img);

    return 0;
}
