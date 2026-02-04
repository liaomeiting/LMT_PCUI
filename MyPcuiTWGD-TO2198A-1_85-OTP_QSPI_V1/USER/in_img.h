
#ifndef _IN_IMG_H_
#define _IN_IMG_H_

#include "sys.h"

void Img_CT(void);
void Img_Full(unsigned char r, unsigned char g, unsigned char b);
void Img_Chcker58(void);
void Img_Box(void);
void Img_Gray256_V(void);
void Img_Gray256_H(void);
void Img_RED256_H(void);
void Img_GREEN256_H(void);
void Img_BLUE256_H(void);
void Img_RED256_V(void);
void Img_GREEN256_V(void);
void Img_BLUE256_V(void);
void Img_ColorBar(void); //≤ Ãı
void Img_ColorBarV(void);
void Img_Flicker(void);
void Img_ColorScale_H(void);
void Img_ColorScale_V(void);
void Img_BoxX(void);
void Img_Gray64_V(void);
void Img_Gray128_V(void);
void Img_Gray256ByTied_V(u8 *tide,int n);
void Img_CTC(void);
void Img_Gray127_V(void);
void Img_coloringbook(void);

void Img_Color_RGB(void);
void Img_Box_1(void);
void Img_Round(int r);
#endif

