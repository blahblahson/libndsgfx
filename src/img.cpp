#include "img.h"

#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

ImageMonolith::ImageMonolith()
{
    for(unsigned int i = 0; i < sizeof(images); i++)
        images[i].data = NULL;
}

ImageMonolith::~ImageMonolith()
{
    UnloadAll();
}

// 0 = success; -1 = I/O error; 1 = out of bounds (theoretically 1 shouldn't
// happen)
int ImageMonolith::LoadAll()
{
    int n;
    for(int i = IMG_ENUM_START; i < IMG_ENUM_END; i++) {
        n = LoadImage(i);
        if(!n) continue;
        return n;
    }

    return 0;
}

// 0 = success; -1 = I/O error; 1 = out of bounds
int ImageMonolith::LoadImage(int id)
{
    if(OOB(id)) return 1;
    FILE *f = fopen(IMG_PATH[id], "r");
    if(!f) {
        errid = id;
        return -1;
    }
    u32 w, h;
    if(!fread(&w, sizeof(u32), 1, f)) {
        errid = id;
        return -1;
    }
    if(!fread(&h, sizeof(u32), 1, f)) {
        errid = id;
        return -1;
    }
    images[id].data = new u16[w*h];
    images[id].w = w;
    images[id].h = h;
    if(fread(&images[id].data, sizeof(u16), w*h, f) < w*h) {
        fclose(f);
        delete[] images[id].data;
        errid = id;
        return -1;
    }

    fclose(f);

    return 0;
}

// 0 = success; -1 = I/O error; 1 = out of bounds
// note: range is inclusive
int ImageMonolith::LoadRange(int a, int b)
{
    int n;
    for(int i = MIN(a, b); i <= MAX(a, b); i++) {
        n = LoadImage(i);
        if(!n) continue;
        return n;
    }

    return 0;
}

// 0 = success, -1 = error, 1 = out of bounds
int ImageMonolith::UnloadImage(int id)
{
    if(OOB(id)) return 1;
    delete[] images[id].data;
    images[id].w = images[id].h = 0;

    return 0;
}

// 0 = success, -1 = error, 1 = out of bounds
int ImageMonolith::UnloadAll()
{
    int n;
    for(int i = IMG_ENUM_START; i < IMG_ENUM_END; i++) {
        n = UnloadImage(i);
        if(!n) continue;
        return n;
    }

    return 0;
}

// 0 = success, -1 = error, 1 = out of bounds
int ImageMonolith::UnloadRange(int a, int b)
{
    int n;
    for(int i = MIN(a, b); i <= MAX(a, b); i++) {
        n = UnloadImage(i);
        if(!n) continue;
        return n;
    }

    return 0;
}

