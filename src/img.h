#ifndef _IMG_H_
#define _IMG_H_

/* this requires the file defined by IMG_ENUM_H to define what files to load
 * and their IDs */

// for information on the image format, see /RAW16

#include <nds.h>
#include <stdio.h>

#define IMG_ENUM_H "img_enum.h"
#include IMG_ENUM_H
#undef IMG_ENUM_H

struct Image {
    u16 *data;
    unsigned int w, h;
};

class ImageMonolith {
    private:
        Image images[IMG_ENUM_END+1];
        int errid;

    public:
        ImageMonolith();
        ~ImageMonolith();
        int LoadAll();
        int LoadImage(int);
        int LoadRange(int, int);
        int UnloadAll();
        int UnloadImage(int);
        int UnloadRange(int, int);

        Image GetImage(int id)
            { return OOB(id) ? images[0] : images[id]; };
        const char *GetImagePath(int id)
            { return OOB(id) ? IMG_PATH[0] : IMG_PATH[id]; };
        inline int GetErrID()
            { return errid; };
        inline bool OOB(int a)
            { return (a >= IMG_ENUM_END || a < 0) ? false : true; };
};


/*
class ImageMonolith (singleton) {
    Image images[IMGENUM_END+1];

    to load all:
        go from 0 to IMGENUM_END in a loop, doing:
            load into slot $i of images from file in imgpath[$i]
            parse w/h (omit from data), put in w/h of struct

    to address:
        Image GetImage(IMGENUM);
        char *GetImagePath(IMGENUM);
}
*/

#endif /* _IMG_H_ */
