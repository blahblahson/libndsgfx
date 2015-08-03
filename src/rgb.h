#ifndef _RGB_H_
#define _RGB_H_

#define RGB_R(c) ((c)&31)
#define RGB_G(c) (((c)>>5)&31)
#define RGB_B(c) (((c)>>10)&31)

#endif /* _RGB_H_ */
