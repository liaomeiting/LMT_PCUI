/******************************* 广州天码电子 *********************************
 * 文件名   ：in_img.c        
 * 作者     ：天码电子
 * 版本     ：
 * 完成日期 ：2014 
 * 描述     ：内置图像
 * 修改记录 ：|    作者    |   日期    |    版本    |    描述    |				
*******************************************************************************/
#include "in_img.h"
#include "gpu.h"
#include "stm_spi.h"


void crosstalk_black(void)
{
		unsigned int i, j, y=0, x=0;
		for(i=0;i<gLCD_YSIZE;i++)
		{
			if((i>0||i==0) && (i<=gLCD_YSIZE/3))
			{
				for(j = 0; j < gLCD_XSIZE; j++)
				{
					SPI_RGB(0x000000);
				}
			}
			else if(i>gLCD_YSIZE/3 && i<=gLCD_YSIZE*2/3)
			{
				for(j = 0; j < gLCD_XSIZE; j++)
				{
					if(j>=gLCD_XSIZE/3 && j<=gLCD_XSIZE*2/3)
					{
						SPI_RGB(0x7f7f7f);
					}
					else
					{
						SPI_RGB(0x000000);
					}

				}
			}
			else if(i>gLCD_YSIZE*2/3 && i<=gLCD_YSIZE)
			{
				for(j = 0; j < gLCD_XSIZE; j++)
				{
					SPI_RGB(0x000000);
				}
			}
		}
}




void crosstalk_black_white(void)
{
   unsigned int i, j, y=0, x=0;
   for(y=0;y<gLCD_YSIZE;y++)
   {
   		for(x=0;x<gLCD_XSIZE;x++)
		 {
				if((x>gLCD_XSIZE*1/8)&&(x<gLCD_XSIZE*3/8)&&(y>gLCD_YSIZE*1/8)&&(y<gLCD_YSIZE*3/8))
				{
					SPI_RGB(0x000000);
				}
				else if((x>gLCD_XSIZE*5/8)&&(x<gLCD_XSIZE*7/8)&&(y>gLCD_YSIZE*5/8)&&(y<gLCD_YSIZE*7/8))
				{
					SPI_RGB(0xffffff);
				}
				else
				{
					SPI_RGB(0x7f7f7f);
				}
		 }
   }
}

void Img_Chcker44(void)
{
	unsigned int y;	
	unsigned int i;
    SPI_DispArea(0,0,gLCD_XSIZE-1,gLCD_YSIZE-1);

		for(y=0;y<gLCD_YSIZE*1/4;y++)
		{
			for(i=0	;i<gLCD_XSIZE/4;  i++){SPI_RGB(0xFFFFFF);}
			for(	;i<gLCD_XSIZE*2/4;i++){SPI_RGB(0x000000);}
			for(	;i<gLCD_XSIZE*3/4;i++){SPI_RGB(0xFFFFFF);}
			for(	;i<gLCD_XSIZE*4/4;i++){SPI_RGB(0x000000);}
		
		}

		for(   ;y<gLCD_YSIZE*2/4;y++)
		{
			for(i=0	;i<gLCD_XSIZE/4;  i++){SPI_RGB(0x000000);}
			for(	;i<gLCD_XSIZE*2/4;i++){SPI_RGB(0xFFFFFF);}
			for(	;i<gLCD_XSIZE*3/4;i++){SPI_RGB(0x000000);}
			for(	;i<gLCD_XSIZE*4/4;i++){SPI_RGB(0xFFFFFF);}
			
                                         
            
		}

		for( ;y<gLCD_YSIZE*3/4;y++)
		{
			for(i=0	;i<gLCD_XSIZE/4;  i++){SPI_RGB(0xFFFFFF);}
			for(	;i<gLCD_XSIZE*2/4;i++){SPI_RGB(0x000000);}
			for(	;i<gLCD_XSIZE*3/4;i++){SPI_RGB(0xFFFFFF);}
			for(	;i<gLCD_XSIZE*4/4;i++){SPI_RGB(0x000000);}
			
		}

		for(   ;y<gLCD_YSIZE*4/4;y++)
		{
			for(i=0	;i<gLCD_XSIZE/4;  i++){SPI_RGB(0x000000);}
			for(	;i<gLCD_XSIZE*2/4;i++){SPI_RGB(0xFFFFFF);}
			for(	;i<gLCD_XSIZE*3/4;i++){SPI_RGB(0x000000);}
			for(	;i<gLCD_XSIZE*4/4;i++){SPI_RGB(0xFFFFFF);}
			
		}

}




void Img_CT(void)
{
	unsigned int h;
	unsigned int v1_2;
	unsigned int v1_4,h1_4;

    SPI_DispArea(0,0,gLCD_XSIZE-1,gLCD_YSIZE-1);
	for(v1_4=0;v1_4<gLCD_YSIZE/4;v1_4++)
	{
		for(h=0;h<gLCD_XSIZE;h++)
		{ 
			SPI_RGB(0x7F7F7F);
		}
	}

	for(v1_2=0;v1_2<gLCD_YSIZE/4;v1_2++)
	{
		for(h1_4=0;h1_4<gLCD_XSIZE/4;h1_4++)
		{
			SPI_RGB(0x7F7F7F); 
		} 

	    for(h1_4=0;h1_4<gLCD_XSIZE/4;h1_4++)
		{   
			SPI_RGB(0x000000); 
            SPI_RGB(0x000000); 
		} 
	    for(h1_4=0;h1_4<gLCD_XSIZE/4;h1_4++)
		{   
			SPI_RGB(0x7F7F7F);
		} 
	}

	for(v1_2=0;v1_2<gLCD_YSIZE/4;v1_2++)
	{
		for(h1_4=0;h1_4<gLCD_XSIZE/4;h1_4++)
		{
			SPI_RGB(0x7F7F7F);
		} 

	    for(h1_4=0;h1_4<gLCD_XSIZE/4;h1_4++)
		{   
			SPI_RGB(0x000000); 
			SPI_RGB(0x000000);

		} 
	    for(h1_4=0;h1_4<gLCD_XSIZE/4;h1_4++)
		{   
			SPI_RGB(0x7F7F7F);
		} 
	}

	for(v1_4=0;v1_4<gLCD_YSIZE/4;v1_4++)
	{
		for(h=0;h<gLCD_XSIZE;h++)
		{ 
			SPI_RGB(0x7F7F7F);
		}			
	}

}

void Img_Flicker(void)
{
    unsigned int i, j;
    SPI_DispArea(0,0,gLCD_XSIZE-1,gLCD_YSIZE-1);
/*	for(j=0;j<gLCD_YSIZE/2;j++)
	{	
		for(i=0;i<gLCD_XSIZE/2;i++)
		{
			SPI_RGB(0x7f007f);
            SPI_RGB(0x007f00);
		}
        for(i=0;i<gLCD_XSIZE/2;i++)
		{
            SPI_RGB(0x007f00);
            SPI_RGB(0x7f007f);
		}

	}*/
    
    for(j=0;j<gLCD_YSIZE/2;j++)
	{	
		for(i=0;i<gLCD_XSIZE;i++)
		{
			SPI_RGB(0x7F007F);
            SPI_RGB(0x007F00);
		}

	}
    
//    for(j=0;j<gLCD_YSIZE/2;j++)
//	{	
//		for(i=0;i<gLCD_XSIZE;i++)
//		{
//			SPI_RGB(0x000000);
//            SPI_RGB(0x7F7F7F);
//		}

//	}
    
    
}


void Img_Full(unsigned char r, unsigned char g, unsigned char b)
{
    unsigned int i, j;
    unsigned int pix;
    SPI_DispArea(0,0,gLCD_XSIZE-1,gLCD_YSIZE-1);
    pix = (r<<16)|(g<<8)|b;
    
	for(j=0;j<gLCD_YSIZE;j++)
	{	
		for(i=0;i<gLCD_XSIZE;i++)
		{
			SPI_RGB(pix);
        }
	}
}


void Img_Chcker58(void)
{
	unsigned int i;
	unsigned int y;
    SPI_DispArea(0,0,gLCD_XSIZE-1,gLCD_YSIZE-1);
//	for(j=0;j<4;j++)
	{
		for(y=0;y<gLCD_YSIZE*1/8;y++)
		{
			for(i=0	;i<gLCD_XSIZE/5;	i++){SPI_RGB(0x000000);}
			for(	;i<gLCD_XSIZE*2/5;i++){SPI_RGB(0xFFFFFF);}
			for(	;i<gLCD_XSIZE*3/5;i++){SPI_RGB(0x000000);}
			for(	;i<gLCD_XSIZE*4/5;i++){SPI_RGB(0xFFFFFF);}
			for(	;i<gLCD_XSIZE;	i++){SPI_RGB(0x000000);}
		}

		for(   ;y<gLCD_YSIZE*2/8;y++)
		{
			for(i=0	;i<gLCD_XSIZE/5;	i++){SPI_RGB(0xFFFFFF);}
			for(	;i<gLCD_XSIZE*2/5;i++){SPI_RGB(0x000000);}
			for(	;i<gLCD_XSIZE*3/5;i++){SPI_RGB(0xFFFFFF);}
			for(	;i<gLCD_XSIZE*4/5;i++){SPI_RGB(0x000000);}
			for(	;i<gLCD_XSIZE;	i++){SPI_RGB(0xFFFFFF);}
		}

		for( ;y<gLCD_YSIZE*3/8;y++)
		{
			for(i=0	;i<gLCD_XSIZE/5;	i++){SPI_RGB(0x000000);}
			for(	;i<gLCD_XSIZE*2/5;i++){SPI_RGB(0xFFFFFF);}
			for(	;i<gLCD_XSIZE*3/5;i++){SPI_RGB(0x000000);}
			for(	;i<gLCD_XSIZE*4/5;i++){SPI_RGB(0xFFFFFF);}
			for(	;i<gLCD_XSIZE;	i++){SPI_RGB(0x000000);}
		}

		for(   ;y<gLCD_YSIZE*4/8;y++)
		{
			for(i=0	;i<gLCD_XSIZE/5;	i++){SPI_RGB(0xFFFFFF);}
			for(	;i<gLCD_XSIZE*2/5;i++){SPI_RGB(0x000000);}
			for(	;i<gLCD_XSIZE*3/5;i++){SPI_RGB(0xFFFFFF);}
			for(	;i<gLCD_XSIZE*4/5;i++){SPI_RGB(0x000000);}
			for(	;i<gLCD_XSIZE;	i++){SPI_RGB(0xFFFFFF);}
		}

		for( ;y<gLCD_YSIZE*5/8;y++)
		{
			for(i=0	;i<gLCD_XSIZE/5;	i++){SPI_RGB(0x000000);}
			for(	;i<gLCD_XSIZE*2/5;i++){SPI_RGB(0xFFFFFF);}
			for(	;i<gLCD_XSIZE*3/5;i++){SPI_RGB(0x000000);}
			for(	;i<gLCD_XSIZE*4/5;i++){SPI_RGB(0xFFFFFF);}
			for(	;i<gLCD_XSIZE;	i++){SPI_RGB(0x000000);}
		}

		for(   ;y<gLCD_YSIZE*6/8;y++)
		{
			for(i=0	;i<gLCD_XSIZE/5;	i++){SPI_RGB(0xFFFFFF);}
			for(	;i<gLCD_XSIZE*2/5;i++){SPI_RGB(0x000000);}
			for(	;i<gLCD_XSIZE*3/5;i++){SPI_RGB(0xFFFFFF);}
			for(	;i<gLCD_XSIZE*4/5;i++){SPI_RGB(0x000000);}
			for(	;i<gLCD_XSIZE;	i++){SPI_RGB(0xFFFFFF);}
		}

		for(   ;y<gLCD_YSIZE*7/8;y++)
		{
			for(i=0	;i<gLCD_XSIZE/5;	i++){SPI_RGB(0x000000);}
			for(	;i<gLCD_XSIZE*2/5;i++){SPI_RGB(0xFFFFFF);}
			for(	;i<gLCD_XSIZE*3/5;i++){SPI_RGB(0x000000);}
			for(	;i<gLCD_XSIZE*4/5;i++){SPI_RGB(0xFFFFFF);}
			for(	;i<gLCD_XSIZE;	i++){SPI_RGB(0x000000);}
		}

		for(   ;y<gLCD_YSIZE*8/8;y++)
		{
			for(i=0	;i<gLCD_XSIZE/5;	i++){SPI_RGB(0xFFFFFF);}
			for(	;i<gLCD_XSIZE*2/5;i++){SPI_RGB(0x000000);}
			for(	;i<gLCD_XSIZE*3/5;i++){SPI_RGB(0xFFFFFF);}
			for(	;i<gLCD_XSIZE*4/5;i++){SPI_RGB(0x000000);}
			for(	;i<gLCD_XSIZE;	i++){SPI_RGB(0xFFFFFF);}
		}
	}
}


void Img_Box(void)
{
// 	char str[128]={0};
	unsigned int x,y;
    unsigned int lx, ly;
    SPI_DispArea(0,0,gLCD_XSIZE-1,gLCD_YSIZE-1);
    lx = gLCD_XSIZE - 1;
    ly = gLCD_YSIZE - 1;
    
	for(y = 0; y < gLCD_YSIZE; y++)
	{
		for(x = 0; x < gLCD_XSIZE; x++)
		{
			if((x==0) || (x==lx) || (y==0) || (y==ly))
			{
                SPI_RGB(0xffffff);
			}
			else
			{
				SPI_RGB(0x000000);
			}
		
		}
	}
}



void Box_RBW()
{
	unsigned int x,y;
	for(y = 0; y < gLCD_YSIZE; y++)
	{
		for(x = 0; x < gLCD_XSIZE; x++)
		{
			if((y == 0)||(y == gLCD_YSIZE -1))
			{
				SPI_RGB(0xffffff);
			}		
			else if(x == 0)
			{
				SPI_RGB(0xff0000);
			}
			else if(x == gLCD_XSIZE - 1)
			{
				SPI_RGB(0x0000ff);
			}			
			else
			{
				SPI_RGB(0x000000);
			}		
		}
	}	
}



void Img_Gray256_H(void)
{
	unsigned int x, y;
	unsigned int lcd_h;
	u32 w;
	lcd_h =  gLCD_XSIZE - 1;
    SPI_DispArea(0,0,gLCD_XSIZE-1,gLCD_YSIZE-1);
	for(y = 0; y <gLCD_YSIZE; y++)
	{
		for(x = 0; x < gLCD_XSIZE; x++)
		{ 
			w =(x*255)/(lcd_h); 
            w = (w<<16) | (w<<8) | w;
			SPI_RGB(w);
        }
	}	
}
/*
void Img_RED256_H(void)
{
	unsigned int x, y;
	unsigned int lcd_h;
	unsigned char w;
	lcd_h =  gLCD_XSIZE - 1;

	for(y = 0; y <gLCD_YSIZE; y++)
	{
		for(x = 0; x < gLCD_XSIZE; x++)
		{ 
			w=(x*255)/(lcd_h); 
			LcdDrvWriteData(w);
			LcdDrvWriteData(0);
			LcdDrvWriteData(0);
        }
	}	
}

void Img_GREEN256_H(void)
{
	unsigned int x, y;
	unsigned int lcd_h;
	unsigned char w;
	lcd_h =  gLCD_XSIZE - 1;

	for(y = 0; y <gLCD_YSIZE; y++)
	{
		for(x = 0; x < gLCD_XSIZE; x++)
		{ 
			w=(x*255)/(lcd_h); 
			LcdDrvWriteData(0);
			LcdDrvWriteData(w);
			LcdDrvWriteData(0);
        }
	}	
}

void Img_BLUE256_H(void)
{
	unsigned int x, y;
	unsigned int lcd_h;
	unsigned char w;
	lcd_h =  gLCD_XSIZE - 1;

	for(y = 0; y <gLCD_YSIZE; y++)
	{
		for(x = 0; x < gLCD_XSIZE; x++)
		{ 
			w=(x*255)/(lcd_h); 
			LcdDrvWriteData(0);
			LcdDrvWriteData(0);
			LcdDrvWriteData(w);
        }
	}	
}*/

void Img_Gray256_V(void)
{
	unsigned int x, y;
	unsigned int lcd_y;
	u32 w;
	lcd_y =  gLCD_YSIZE - 1;
    SPI_DispArea(0,0,gLCD_XSIZE-1,gLCD_YSIZE-1);
	for(y = 0; y <gLCD_YSIZE; y++)
	{	
		w=(y*255)/(lcd_y);
        w = (w<<16) | (w<<8) | w;
		for(x = 0; x < gLCD_XSIZE; x++)
		{ 
			SPI_RGB(w);
        }
	}	
}





void Img_Gray240_V(void)
{
	unsigned int x, y;
	unsigned int lcd_y;
	u32 w;
	lcd_y =  gLCD_YSIZE - 1;
	SPI_DispArea(0,0,gLCD_XSIZE-1,gLCD_YSIZE-1);
	for(y = 0; y <gLCD_YSIZE; y++)
	{	
		w=y;
		
        w = (w<<16) | (w<<8) | w;
		for(x = 0; x < gLCD_XSIZE; x++)
		{ 
			SPI_RGB(w);
        }
	}	
}

void Img_Gray32_V(void)
{
	unsigned int x, y;
	unsigned int lcd_y;
	u32 w;
	lcd_y =  gLCD_YSIZE - 1;
	SPI_DispArea(0,0,gLCD_XSIZE-1,gLCD_YSIZE-1);
	for(y = 0; y <gLCD_YSIZE; y++)
	{	
		w=((y*31)/(lcd_y));
		
        w = (w<<16) | (w<<8) | w;
		for(x = 0; x < gLCD_XSIZE; x++)
		{ 
			SPI_RGB(w);
        }
	}	
}

void Img_Gray64_V(void)
{
	unsigned int x, y;
	unsigned int lcd_y;
	u32 w;
	lcd_y =  gLCD_YSIZE - 1;
	SPI_DispArea(0,0,gLCD_XSIZE-1,gLCD_YSIZE-1);
	for(y = 0; y <gLCD_YSIZE; y++)
	{	
		w=((y*63)/(lcd_y));
		
        w = (w<<16) | (w<<8) | w;
		for(x = 0; x < gLCD_XSIZE; x++)
		{ 
			SPI_RGB(w);
        }
	}	
}


void Img_Gray128_V(void)
{
	unsigned int x, y;
	unsigned int lcd_y;
	u32 w;
	lcd_y =  gLCD_YSIZE - 1;
	SPI_DispArea(0,0,gLCD_XSIZE-1,gLCD_YSIZE-1);
	for(y = 0; y <gLCD_YSIZE; y++)
	{	
		w=((y*127)/(lcd_y));
		
        w = (w<<16) | (w<<8) | w;
		for(x = 0; x < gLCD_XSIZE; x++)
		{ 
			SPI_RGB(w);
        }
	}	
}

/*
void Img_RED256_V(void)
{
	unsigned int x, y;
	unsigned int lcd_y;
	unsigned char w;
	lcd_y =  gLCD_YSIZE - 1;

	for(y = 0; y <gLCD_YSIZE; y++)
	{	
		w=(y*255)/(lcd_y);
		for(x = 0; x < gLCD_XSIZE; x++)
		{ 
			 
			LcdDrvWriteData(w);
			LcdDrvWriteData(0);
			LcdDrvWriteData(0);
        }
	}	
}

void Img_GREEN256_V(void)
{
	unsigned int x, y;
	unsigned int lcd_y;
	unsigned char w;
	lcd_y =  gLCD_YSIZE - 1;

	for(y = 0; y <gLCD_YSIZE; y++)
	{	
		w=(y*255)/(lcd_y);
		for(x = 0; x < gLCD_XSIZE; x++)
		{ 
			 
			LcdDrvWriteData(0);
			LcdDrvWriteData(w);
			LcdDrvWriteData(0);
        }
	}	
}

void Img_BLUE256_V(void)
{
	unsigned int x, y;
	unsigned int lcd_y;
	unsigned char w;
	lcd_y =  gLCD_YSIZE - 1;

	for(y = 0; y <gLCD_YSIZE; y++)
	{	
		w=(y*255)/(lcd_y);
		for(x = 0; x < gLCD_XSIZE; x++)
		{ 
			 
			LcdDrvWriteData(0);
			LcdDrvWriteData(0);
			LcdDrvWriteData(w);
        }
	}	
}*/

void Img_ColorBar(void)
{
    unsigned int i, j, x;
    SPI_DispArea(0,0,gLCD_XSIZE-1,gLCD_YSIZE-1);
 	x = gLCD_XSIZE / 8;
	for(j=0;j<gLCD_YSIZE;j++)
	{	
		for(i=0;i<x;i++){ SPI_RGB(0xFFFFFF);}	
		for(i=0;i<x;i++){ SPI_RGB(0xFFFF00);}
		for(i=0;i<x;i++){ SPI_RGB(0x00FFFF);}
		for(i=0;i<x;i++){ SPI_RGB(0x00FF00);}
		for(i=0;i<x;i++){ SPI_RGB(0xFF00FF);}
		for(i=0;i<x;i++){ SPI_RGB(0xFF0000);}
		for(i=0;i<x;i++){ SPI_RGB(0x0000FF);}
		for(i=0;i<x;i++){ SPI_RGB(0x000000);}
		
	}
}

void photo_bmp(const  unsigned char bmp[])
{
	 unsigned int i;
	 u32 pix;

		SPI_DispArea(0,0,gLCD_XSIZE-1,gLCD_YSIZE-1);

	 for(i=0;i<gLCD_XSIZE*gLCD_YSIZE*3;i+=3)		
	{ 			
		pix = (bmp[i+2]<<16)|((bmp[i+1])<<8)|(bmp[i]);
			SPI_RGB(pix);
		
	 }

}

/*
void MAX_Current(void)
{
    unsigned int i, j;

	for(j=0;j<gLCD_YSIZE/2;j++)
	{	
		for(i=0;i<gLCD_XSIZE/2;i++)
		{
			LcdDrvWriteData(0xff);LcdDrvWriteData(0x00);LcdDrvWriteData(0xff);
			LcdDrvWriteData(0x00);LcdDrvWriteData(0xff);LcdDrvWriteData(0x00);
		 }
	   for(i=0;i<gLCD_XSIZE/2;i++)
		{
			LcdDrvWriteData(0x00);LcdDrvWriteData(0xff);LcdDrvWriteData(0x00);
			LcdDrvWriteData(0xff);LcdDrvWriteData(0x00);LcdDrvWriteData(0xff);
			
		 }
	}
}*/



/******************************* 广州天码电子 *********************************/
