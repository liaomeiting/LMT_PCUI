/********************************* SKYCODE *************************************
* Copyright (c) 2020, 深圳思凯测试技术有限公司
*
* 文件名   ：in_img2.c   
* 描述     ：软件产生画面，第二代写法。         
*
* 版    本 ：V1.0.0
* 作    者 ：ma_xiaoteng
* 完成日期 ：2020-6-2
* 修改描述 ：          
*******************************************************************************/

#include "in_img2.h"

/*
*函数名: InImg2_Init
*描述  : 软件产生画面程序的初始化。
*参数  : 1. img2_adapter - 上下文结构体（句柄），
                    之后调用画面函数，都要用到这个变量。
         2. x_size - 屏分辨率
         3. y_size - 屏分辨率
         4. (*pf_wr_rgb)(unsigned int rgb) - 写像素的函数，参数格式要和这个一样
         5. (*pf_draw_block)(unsigned int x_start, unsigned int y_start, unsigned int x_stop,unsigned int y_stop)) - 
             跳坐标 或 声明显示范围的函数，点MCU接口和COMMAND MIPI屏经常用到的2A 2B写坐标的最后2C的那个

*返回  : 无
*/
void InImg2_Init(InImg2_Adapter *img2_adapter, 
                 unsigned int x_size, unsigned int y_size, //显示屏分辨率
                 void (*pf_wr_rgb)(unsigned int rgb), 
                 void (*pf_draw_block)(unsigned int x_start, unsigned int y_start, unsigned int x_stop,unsigned int y_stop))
{
    img2_adapter->x_size = x_size;
    img2_adapter->y_size = y_size;
    img2_adapter->wr_rgb =  pf_wr_rgb;
    img2_adapter->draw_block = pf_draw_block;
}


void InImg2_CT(InImg2_Adapter *img2_adapter)
{
    unsigned int y,x;

    img2_adapter->draw_block(0,0,img2_adapter->x_size-1,img2_adapter->y_size-1);
    
    for(y=0; y<img2_adapter->y_size / 4; y++)
    {
        for(x=0; x<img2_adapter->x_size; x++)
		{ 
			img2_adapter->wr_rgb(0x7F7F7F);
		}
    }
    
    for(; y<img2_adapter->y_size *3/ 4; y++)
    {
        for(x=0; x<img2_adapter->x_size/4; x++)
		{
			img2_adapter->wr_rgb(0x7F7F7F); 
		} 
        
        for(; x<img2_adapter->x_size*3/4; x++)
		{
			img2_adapter->wr_rgb(0x000000); 
		}

        for(; x<img2_adapter->x_size*4/4; x++)
		{
			img2_adapter->wr_rgb(0x7F7F7F); 
		} 
    }
    
    for(; y<img2_adapter->y_size; y++)
    {
        for(x=0; x<img2_adapter->x_size; x++)
		{ 
			img2_adapter->wr_rgb(0x7F7F7F);
		}
    }
    
}

void InImg2_Flicker(InImg2_Adapter *img2_adapter)
{
    unsigned int i, j;
    img2_adapter->draw_block(0,0,img2_adapter->x_size-1,img2_adapter->y_size-1);

    for(j=0;j<img2_adapter->y_size/2;j++)
	{	
		for(i=0;i<img2_adapter->x_size;i++)
		{
			img2_adapter->wr_rgb(0x7F007F);
            img2_adapter->wr_rgb(0x007F00);
		}
	}
    
}


void InImg2_Full(InImg2_Adapter *img2_adapter, unsigned char r, unsigned char g, unsigned char b)
{
    unsigned int i, j;
    unsigned int pix;
    img2_adapter->draw_block(0,0,img2_adapter->x_size-1,img2_adapter->y_size-1);
    pix = (r<<16)|(g<<8)|b;
    
	for(j=0;j<img2_adapter->y_size;j++)
	{	
		for(i=0;i<img2_adapter->x_size;i++)
		{
			img2_adapter->wr_rgb(pix);
        }
	}
}


void InImg2_Chcker58(InImg2_Adapter *img2_adapter)
{
	unsigned int i;
	unsigned int y;
    img2_adapter->draw_block(0,0,img2_adapter->x_size-1,img2_adapter->y_size-1);
//	for(j=0;j<4;j++)
	{
		for(y=0;y<img2_adapter->y_size*1/8;y++)
		{
			for(i=0	;i<img2_adapter->x_size/5;	i++){img2_adapter->wr_rgb(0x000000);}
			for(	;i<img2_adapter->x_size*2/5;i++){img2_adapter->wr_rgb(0xFFFFFF);}
			for(	;i<img2_adapter->x_size*3/5;i++){img2_adapter->wr_rgb(0x000000);}
			for(	;i<img2_adapter->x_size*4/5;i++){img2_adapter->wr_rgb(0xFFFFFF);}
			for(	;i<img2_adapter->x_size;	i++){img2_adapter->wr_rgb(0x000000);}
		}

		for(   ;y<img2_adapter->y_size*2/8;y++)
		{
			for(i=0	;i<img2_adapter->x_size/5;	i++){img2_adapter->wr_rgb(0xFFFFFF);}
			for(	;i<img2_adapter->x_size*2/5;i++){img2_adapter->wr_rgb(0x000000);}
			for(	;i<img2_adapter->x_size*3/5;i++){img2_adapter->wr_rgb(0xFFFFFF);}
			for(	;i<img2_adapter->x_size*4/5;i++){img2_adapter->wr_rgb(0x000000);}
			for(	;i<img2_adapter->x_size;	i++){img2_adapter->wr_rgb(0xFFFFFF);}
		}

		for( ;y<img2_adapter->y_size*3/8;y++)
		{
			for(i=0	;i<img2_adapter->x_size/5;	i++){img2_adapter->wr_rgb(0x000000);}
			for(	;i<img2_adapter->x_size*2/5;i++){img2_adapter->wr_rgb(0xFFFFFF);}
			for(	;i<img2_adapter->x_size*3/5;i++){img2_adapter->wr_rgb(0x000000);}
			for(	;i<img2_adapter->x_size*4/5;i++){img2_adapter->wr_rgb(0xFFFFFF);}
			for(	;i<img2_adapter->x_size;	i++){img2_adapter->wr_rgb(0x000000);}
		}

		for(   ;y<img2_adapter->y_size*4/8;y++)
		{
			for(i=0	;i<img2_adapter->x_size/5;	i++){img2_adapter->wr_rgb(0xFFFFFF);}
			for(	;i<img2_adapter->x_size*2/5;i++){img2_adapter->wr_rgb(0x000000);}
			for(	;i<img2_adapter->x_size*3/5;i++){img2_adapter->wr_rgb(0xFFFFFF);}
			for(	;i<img2_adapter->x_size*4/5;i++){img2_adapter->wr_rgb(0x000000);}
			for(	;i<img2_adapter->x_size;	i++){img2_adapter->wr_rgb(0xFFFFFF);}
		}

		for( ;y<img2_adapter->y_size*5/8;y++)
		{
			for(i=0	;i<img2_adapter->x_size/5;	i++){img2_adapter->wr_rgb(0x000000);}
			for(	;i<img2_adapter->x_size*2/5;i++){img2_adapter->wr_rgb(0xFFFFFF);}
			for(	;i<img2_adapter->x_size*3/5;i++){img2_adapter->wr_rgb(0x000000);}
			for(	;i<img2_adapter->x_size*4/5;i++){img2_adapter->wr_rgb(0xFFFFFF);}
			for(	;i<img2_adapter->x_size;	i++){img2_adapter->wr_rgb(0x000000);}
		}

		for(   ;y<img2_adapter->y_size*6/8;y++)
		{
			for(i=0	;i<img2_adapter->x_size/5;	i++){img2_adapter->wr_rgb(0xFFFFFF);}
			for(	;i<img2_adapter->x_size*2/5;i++){img2_adapter->wr_rgb(0x000000);}
			for(	;i<img2_adapter->x_size*3/5;i++){img2_adapter->wr_rgb(0xFFFFFF);}
			for(	;i<img2_adapter->x_size*4/5;i++){img2_adapter->wr_rgb(0x000000);}
			for(	;i<img2_adapter->x_size;	i++){img2_adapter->wr_rgb(0xFFFFFF);}
		}

		for(   ;y<img2_adapter->y_size*7/8;y++)
		{
			for(i=0	;i<img2_adapter->x_size/5;	i++){img2_adapter->wr_rgb(0x000000);}
			for(	;i<img2_adapter->x_size*2/5;i++){img2_adapter->wr_rgb(0xFFFFFF);}
			for(	;i<img2_adapter->x_size*3/5;i++){img2_adapter->wr_rgb(0x000000);}
			for(	;i<img2_adapter->x_size*4/5;i++){img2_adapter->wr_rgb(0xFFFFFF);}
			for(	;i<img2_adapter->x_size;	i++){img2_adapter->wr_rgb(0x000000);}
		}

		for(   ;y<img2_adapter->y_size*8/8;y++)
		{
			for(i=0	;i<img2_adapter->x_size/5;	i++){img2_adapter->wr_rgb(0xFFFFFF);}
			for(	;i<img2_adapter->x_size*2/5;i++){img2_adapter->wr_rgb(0x000000);}
			for(	;i<img2_adapter->x_size*3/5;i++){img2_adapter->wr_rgb(0xFFFFFF);}
			for(	;i<img2_adapter->x_size*4/5;i++){img2_adapter->wr_rgb(0x000000);}
			for(	;i<img2_adapter->x_size;	i++){img2_adapter->wr_rgb(0xFFFFFF);}
		}
	}
}


void InImg2_Box(InImg2_Adapter *img2_adapter)
{
	unsigned int x,y;
    unsigned int lx, ly;
    
    img2_adapter->draw_block(0,0,img2_adapter->x_size-1,img2_adapter->y_size-1);
    
    lx = img2_adapter->x_size - 1;
    ly = img2_adapter->y_size - 1;
    
	for(y = 0; y < img2_adapter->y_size; y++)
	{
		for(x = 0; x < img2_adapter->x_size; x++)
		{
			if((x==0) || (x==lx) || (y==0) || (y==ly))
			{
                img2_adapter->wr_rgb(0xffffff);
			}
			else
			{
				img2_adapter->wr_rgb(0x000000);
			}
		}
	}
}


void InImg2_Gray256_H(InImg2_Adapter *img2_adapter)
{
	unsigned int x, y;
	u32 w;
    
    img2_adapter->draw_block(0,0,img2_adapter->x_size-1,img2_adapter->y_size-1);
    
	for(y = 0; y <img2_adapter->y_size; y++)
	{
		for(x = 0; x < img2_adapter->x_size; x++)
		{ 
			w =(x*256)/(img2_adapter->x_size); 
            w = (w<<16) | (w<<8) | w;
			img2_adapter->wr_rgb(w);
        }
	}	
}

void InImg2_RED256_H(InImg2_Adapter *img2_adapter)
{
	unsigned int x, y;
	u32 w;

    img2_adapter->draw_block(0,0,img2_adapter->x_size-1,img2_adapter->y_size-1);
    
	for(y = 0; y <img2_adapter->y_size; y++)
	{
		for(x = 0; x < img2_adapter->x_size; x++)
		{ 
			w =(x*256)/(img2_adapter->x_size); 
            w = (w<<16);// | (w<<8) | w;
			img2_adapter->wr_rgb(w);
        }
	}	
}

void InImg2_GREEN256_H(InImg2_Adapter *img2_adapter)
{
	unsigned int x, y;
	u32 w;

    img2_adapter->draw_block(0,0,img2_adapter->x_size-1,img2_adapter->y_size-1);
    
	for(y = 0; y <img2_adapter->y_size; y++)
	{
		for(x = 0; x < img2_adapter->x_size; x++)
		{ 
			w =(x*256)/(img2_adapter->x_size); 
            w = (w<<8);
			img2_adapter->wr_rgb(w);
        }
	}	
}

void InImg2_BLUE256_H(InImg2_Adapter *img2_adapter)
{
	unsigned int x, y;
	u32 w;

    img2_adapter->draw_block(0,0,img2_adapter->x_size-1,img2_adapter->y_size-1);
    
	for(y = 0; y <img2_adapter->y_size; y++)
	{
		for(x = 0; x < img2_adapter->x_size; x++)
		{ 
			w =(x*256)/(img2_adapter->x_size); 
 			img2_adapter->wr_rgb(w);
        }
	}	
}

void InImg2_Gray256_V(InImg2_Adapter *img2_adapter)
{
	unsigned int x, y;
	u32 w;
    
    img2_adapter->draw_block(0,0,img2_adapter->x_size-1,img2_adapter->y_size-1);
    
	for(y = 0; y <img2_adapter->y_size; y++)
	{	
		w=(y*256)/(img2_adapter->y_size);
        w = (w<<16) | (w<<8) | w;
		for(x = 0; x < img2_adapter->x_size; x++)
		{ 
			img2_adapter->wr_rgb(w);
        }
	}	
}

void InImg2_RED256_V(InImg2_Adapter *img2_adapter)
{
	unsigned int x, y;
	u32 w;
    
    img2_adapter->draw_block(0,0,img2_adapter->x_size-1,img2_adapter->y_size-1);
    
	for(y = 0; y <img2_adapter->y_size; y++)
	{	
		w=(y*256)/(img2_adapter->y_size);
        w = (w<<16);// | (w<<8) | w;
		for(x = 0; x < img2_adapter->x_size; x++)
		{ 
			img2_adapter->wr_rgb(w);
        }
	}	
}

void InImg2_GREEN256_V(InImg2_Adapter *img2_adapter)
{
	unsigned int x, y;
	u32 w;
    
    img2_adapter->draw_block(0,0,img2_adapter->x_size-1,img2_adapter->y_size-1);
    
	for(y = 0; y <img2_adapter->y_size; y++)
	{	
		w=(y*256)/(img2_adapter->y_size);
        w = (w<<8);
		for(x = 0; x < img2_adapter->x_size; x++)
		{ 
			img2_adapter->wr_rgb(w);
        }
	}	
}

void InImg2_BLUE256_V(InImg2_Adapter *img2_adapter)
{
	unsigned int x, y;
	u32 w;
    
    img2_adapter->draw_block(0,0,img2_adapter->x_size-1,img2_adapter->y_size-1);
    
	for(y = 0; y <img2_adapter->y_size; y++)
	{	
		w=(y*256)/(img2_adapter->y_size);
		for(x = 0; x < img2_adapter->x_size; x++)
		{ 
			img2_adapter->wr_rgb(w);
        }
	}	
}

void InImg2_ColorBar(InImg2_Adapter *img2_adapter)
{
    unsigned int i, j, x;
    
    img2_adapter->draw_block(0,0,img2_adapter->x_size-1,img2_adapter->y_size-1);
    
    x=img2_adapter->x_size;

    
    for(j=0; j<img2_adapter->y_size; j++)
    {	
        for(i=0;i<x/8;i++){ img2_adapter->wr_rgb(0xFFFFFF);}	
        for(;i<x*2/8;i++){ img2_adapter->wr_rgb(0xFFFF00);}
        for(;i<x*3/8;i++){ img2_adapter->wr_rgb(0x00FFFF);}
        for(;i<x*4/8;i++){ img2_adapter->wr_rgb(0x00FF00);}
        for(;i<x*5/8;i++){ img2_adapter->wr_rgb(0xFF00FF);}
        for(;i<x*6/8;i++){ img2_adapter->wr_rgb(0xFF0000);}
        for(;i<x*7/8;i++){ img2_adapter->wr_rgb(0x0000FF);}
        for(;i<x*8/8;i++){ img2_adapter->wr_rgb(0x000000);}

    }	
}

void InImg2_ColorBarV(InImg2_Adapter *img2_adapter)
{
    unsigned int i, j, x,y;
    
    img2_adapter->draw_block(0,0,img2_adapter->x_size-1,img2_adapter->y_size-1);
    
    x=img2_adapter->x_size;
    y=img2_adapter->y_size;
    
    for(i=0; i<y*1/8; i++){for(i=0;j<x;j++){ img2_adapter->wr_rgb(0xFFFFFF);}}	
    for(   ; i<y*2/8; i++){for(i=0;j<x;j++){ img2_adapter->wr_rgb(0xFFFF00);}}	
    for(   ; i<y*3/8; i++){for(i=0;j<x;j++){ img2_adapter->wr_rgb(0x00FFFF);}}	
    for(   ; i<y*4/8; i++){for(i=0;j<x;j++){ img2_adapter->wr_rgb(0x00FF00);}}	
    for(   ; i<y*5/8; i++){for(i=0;j<x;j++){ img2_adapter->wr_rgb(0xFF00FF);}}	
    for(   ; i<y*6/8; i++){for(i=0;j<x;j++){ img2_adapter->wr_rgb(0xFF0000);}}	
    for(   ; i<y*7/8; i++){for(i=0;j<x;j++){ img2_adapter->wr_rgb(0x0000FF);}}	
    for(   ; i<y*8/8; i++){for(i=0;j<x;j++){ img2_adapter->wr_rgb(0x000000);}}		
}


/********************************* SKYCODE **************************************/
