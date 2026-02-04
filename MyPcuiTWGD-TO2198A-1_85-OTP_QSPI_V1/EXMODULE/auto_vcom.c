/********************************* SKYCODE ************************************
* Copyright (c) 2015-2018, 深圳思凯测试技术有限公司
* All rights reserved.
*
* 文件名   ：auto_vcom.c    
* 描述     ：
*            1.自动查找最佳VCOM
*
* 版    本 ：V1.0.0
* 作    者 ：
* 完成日期 ：2018-5-14
* 修改描述 : 
*                                                                     
******************************************************************************/

#include "auto_vcom.h"
#include "systick.h"
#include <stdio.h>
#include "showtext.h"
#include "key.h"


/*------------------------------ DeBug define -------------------------------*/
/*程序中使用 _DEBUG_SSD28() 来调试代码*/

#define __DEBUG_VCOM__ 1   //注释掉这句 则代码不执行_DEBUG_SSD28()

#ifdef __DEBUG_VCOM__
#include "debug_def.h"
#define _DEBUG_VCOM _DEBUG 
#else
#define _DEBUG_VCOM(format,...)   
#endif

/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/

static void(*sg_pfunc_set_vcom)(unsigned int vcom);     //指向 set vcom函数指针
static int(*sg_pfunc_get_flcker)(float*);



/*---------------------------------------------------------------------------*/

/*******************************************************************************

*******************************************************************************/
void AutoVcomInit( void(*pfunc_set_vcom)(unsigned int vcom), 
                  int(*pfunc_get_flcker)(float*))
{
    sg_pfunc_set_vcom = pfunc_set_vcom; 
    sg_pfunc_get_flcker = pfunc_get_flcker;    
}

/*
*函数名 : AutoVcom_ScanRange
*描述   : 在规定的范围内 从小到大扫描，找到最佳VCOM。(是VCOM从小到底 不是闪烁从小到大)
*参数   : 1.vcom_min - 最小VCOM值，开始值
          2.vcom_max - 最大VCOM值，结束值
          3.step     - VCOM递增的阶数
          4.step_delay_ms - 每一步间隔时时间
          5.*vcom - 最终找到的最佳vcom值
*返回   : 1. AUTOVCOM_OK- 扫描完成，查找到最佳VCOM
          2. AUTOVCOM_ERROR - 出错，结束。
          3. AUTOVCOM_EXIT - 按键退出
*/
int AutoVcom_ScanRange(unsigned int vcom_min, unsigned int vcom_max, 
                       unsigned int step, unsigned int step_delay_ms,
                       unsigned int *vcom)
{
    unsigned int state=0;
    unsigned int ts;
    int res;
    float flicker;
    float flicker_min=200.0F;
    unsigned int vcom_good, vcom_tmp;
    
    vcom_tmp = vcom_min;
    
    while(1)
    {
        /*按键*/
        if(KEY_Read() == KEY_ONOFF){return AUTOVCOM_EXIT;} //按键退出
        
        /*扫描*/
        if(GetMeasureTime_ms(ts) >= step_delay_ms)
        {
            switch(state)
            {
                case 0:
                    (*sg_pfunc_set_vcom)(vcom_tmp); //vcom值写入模组 Drive IC
                    state = 1;
                    break;
                
                case 1:
                    res=(*sg_pfunc_get_flcker)(&flicker); //回读flicke 测量值
                    if((res!=1)||(flicker<0.1F)){return AUTOVCOM_ERROR;}
                    _DEBUG_VCOM("vcom=%d, flic=%3.1f\n\r",vcom_tmp, flicker);
                    
                    if(flicker_min > flicker)     //保存最优VCOM 和 flicker
                    {
                        flicker_min = flicker;   //
                        vcom_good = vcom_tmp;   //最好vcom
                    }
                    
                    vcom_tmp += step;
                    if(vcom_tmp <= vcom_max)
                    {
                        (*sg_pfunc_set_vcom)(vcom_tmp);
                    }
                    else
                    {   /*结束*/
                        *vcom = vcom_good;
                        state = 2;
                    }
                    break;
                    
                case 2: //慢慢恢复到最佳值
                    if(vcom_tmp > (vcom_good+8))
                    {
                        vcom_tmp -= 8;
                        (*sg_pfunc_set_vcom)(vcom_tmp); 
                    }
                    else
                    {
                        (*sg_pfunc_set_vcom)(vcom_tmp=vcom_good);
                        state = 3;
                    }
                    break;
                 
                case 3: //完成
                    res=(*sg_pfunc_get_flcker)(&flicker); //回读flicke 测量值
                    if((res!=1)||(flicker<0.1F)){return AUTOVCOM_ERROR;}
                    _DEBUG_VCOM("good vcom=%d, flic=%3.1f\n\r",vcom_good, flicker);
                    
                    return AUTOVCOM_OK;
//                    break;
            }
            
            ts = MeasureTimeStart_ms();
        }
    }
    
}


/*
*函数名 : AutoVcom_ScanDef
*描述   : 从默认值开始扫描,寻找最佳VCOM
*参数   : 1.vcom_def - VCOM默认值，以这个值为中心值寻找最佳VCOM
          2.step     - VCOM递增的阶数
          3.step_delay_ms - 每一步间隔时时间
          4.*vcom - 最终找到的最佳vcom值
*返回   : 1. AUTOVCOM_OK- 扫描完成，查找到最佳VCOM
          2. AUTOVCOM_ERROR - 出错，结束。
          3. AUTOVCOM_EXIT - 按键退出
*/
int AutoVcom_ScanDef(unsigned int vcom_def,
                       unsigned int step, unsigned int step_delay_ms,
                       unsigned int *vcom)
{
    unsigned int state=0;
    unsigned int ts;
    int res;
    float flicker;
    float flicker_l;
    unsigned int vcom_tmp;
    
    vcom_tmp = vcom_def;
    
    while(1)
    {
        /*按键*/
        if(KEY_Read() == KEY_ONOFF){return AUTOVCOM_EXIT;} //按键退出
        
        /*扫描*/
        if(GetMeasureTime_ms(ts) >= step_delay_ms)
        {
            switch(state)
            {
                case 0:
                    (*sg_pfunc_set_vcom)(vcom_tmp); //vcom值写入模组 Drive IC
                    state = 1;
                    break;
                
                case 1:
                    res=(*sg_pfunc_get_flcker)(&flicker); //回读flicke 测量值
                    if((res!=1)||(flicker<0.1F)){return AUTOVCOM_ERROR;}
                    _DEBUG_VCOM("vcom=%d, flic=%3.1f\n\r",vcom_tmp, flicker);
                    
                    (*sg_pfunc_set_vcom)(vcom_tmp+=step);
                    
                    state = 2;
                    break;
                    
                case 2:
                    flicker_l = flicker;
                    res=(*sg_pfunc_get_flcker)(&flicker); //回读flicke 测量值
                    if((res!=1)||(flicker<0.1F)){return AUTOVCOM_ERROR;}
                    _DEBUG_VCOM("vcom=%d, flic=%3.1f\n\r",vcom_tmp, flicker);
                    if(flicker < flicker_l) //加vcom flicker变小
                    {
                        (*sg_pfunc_set_vcom)(vcom_tmp+=step);
                        state = 3;
                    }
                    else if(flicker > flicker_l) //加vcom flicker变大
                    {
//                        vcom_tmp-=step;
//                        vcom_tmp-=step; //减两次恢复回去
                        (*sg_pfunc_set_vcom)(vcom_tmp-=step);
                        state = 4;
                    }
                    else  //已经找到
                    {
                        state = 5;
                    }
                    break;
            
                case 3:  //加VCOM 使FLCIKER减小
                    flicker_l = flicker;
                    res=(*sg_pfunc_get_flcker)(&flicker); //回读flicke 测量值
                    if((res!=1)||(flicker<0.1F)){return AUTOVCOM_ERROR;}
                    _DEBUG_VCOM("vcom=%d, flic=%3.1f\n\r",vcom_tmp, flicker);
                    
                    if(flicker < flicker_l) //加vcom flicker变小
                    {
                        (*sg_pfunc_set_vcom)(vcom_tmp+=step);
                    }
                    else    //减VCOM已经无法减小闪烁了
                    {
                        vcom_tmp-=step;
                        (*sg_pfunc_set_vcom)(vcom_tmp);
                        state = 5;
                    }
                    break;
            
                case 4:  //减VCOM使 flcker减小
                    flicker_l = flicker;
                    res=(*sg_pfunc_get_flcker)(&flicker); //回读flicke 测量值
                    if((res!=1)||(flicker<0.1F)){return AUTOVCOM_ERROR;}
                    _DEBUG_VCOM("vcom=%d, flic=%3.1f\n\r",vcom_tmp, flicker);
                    
                    if(flicker < flicker_l) 
                    {
                        (*sg_pfunc_set_vcom)(vcom_tmp-=step);
                    }
                    else  //如果VCOM值减小，已经无法让闪烁变小
                    {
                        vcom_tmp+=step;
                        (*sg_pfunc_set_vcom)(vcom_tmp);
                        state = 5;
                    }
                    break;
                    
                case 5:
                    res=(*sg_pfunc_get_flcker)(&flicker); //回读flicke 测量值
                    if((res!=1)||(flicker<0.1F)){return AUTOVCOM_ERROR;}
                    _DEBUG_VCOM("good vcom=%d, flic=%3.1f\n\r",vcom_tmp, flicker);
                    *vcom = vcom_tmp;
                    return AUTOVCOM_OK;
//                    break;
            }
            
            ts = MeasureTimeStart_ms();
        }
    }
}


/********************************* SKYCODE ***********************************/
