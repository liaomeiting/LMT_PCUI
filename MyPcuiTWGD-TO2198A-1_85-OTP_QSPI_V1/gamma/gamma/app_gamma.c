#include <math.h>
#include "app_gamma.h"
#include "gammaLinear.h"
#include "gammaRun.h"
#include "ssd2828.h"
#include "cfl.h"
//#include "ca310.h"
#include "bus_rs232_id1.h"//cl5用到�??
#include "deviceCA310.h"  //cl5用到�??
#include "def.h"

/**用户include�??**************************************************************************/
#include "in_img.h"
#include "sd_file.h"
#include "qspi_lcd.h"
#include "user_control.h"
#include "driverUsart1.h"
unsigned int count=0;
/******************************************************************************************/
#define SDCard_Log 

/**用户变量存放�??**************************************************************************/
FRAME my_frame;
u16 gamma_AllRead[81] = {0};
extern u8 CH_NUM; 
extern char scancode[50];
extern char save_log[];
extern char array[];

//定义各模式配置结构体（有多少组gamma模式就定义多少个�??
NEW_GammaConfigStruct GammaConfig_HBM;
NEW_GammaConfigStruct GammaConfig_Idle;
NEW_GammaConfigStruct GammaConfig_Normal;
NEW_GammaConfigStruct GammaConfig_Normal3;
NEW_GammaConfigStruct GammaConfig_Normal2;  
NEW_GammaConfigStruct GammaConfig_Normal4;    
/******************************************************************************************/
char Code_file[128];  
extern unsigned int NormalR26;
extern unsigned char Readgamma_flag; 
    
extern void Check_Nor51_1D_L32(void);
extern void Check_Idle51_1D_L32(void);

void Debug_target(uint8_t _rgb,uint8_t level_D);
void test_chack_awe(uint8_t _rgb,NEW_GammaConfigStruct *gammaConfig,uint16_t *ref_reg);

float Debug_targetX = 0;
float Debug_targetY = 0;
float Debug_MinX = 0;
float Debug_MaxX = 0;
float Debug_MinY = 0;
float Debug_MaxY = 0;

int normal2_Debug_flag = 0;
/**用户函数存放�??**************************************************************************/

static float loga(float a, float b)
{
	float x1=log(a);
	float x2=log(b);

	return x1/x2;
}

/*@fun : 回读0-255阶xyz,gamma2.2数据
 *@parm: gammaConfig_t模式配置结构�??
 */
void Reread_xyz(NEW_GammaConfigStruct *gammaConfig_t)
{
    uint8_t gray;
    double gamma2_2;
    double xyz[3],XYLv[3];
    for(int i = 0; i < 256; i++)
    { 
        gray = i;
        Img_Full(gray, gray, gray); 
        Delay_ms(300);
        gammaConfig_t->readLv(3, XYLv, xyz);
        gamma2_2 = loga(xyz[1]/gammaConfig_t->maxlv, gray/255.0);
        if(gray == 0 || gray == 255){
            printf("%d, x:%.4f, y:%.4f, Lv:%.4f\r\n",gray, XYLv[0], XYLv[1], XYLv[2]);        }
        else{
            printf("%d, x:%.4f, y:%.4f, Lv:%.4f, gamma:%.4f\r\n",gray, XYLv[0], XYLv[1], XYLv[2], gamma2_2);
        }
    
    }
}
  
/*@fun : 复判各灰阶绑�??
 *@parm: gammaConfig_t模式配置结构�??
 */
void debug_RECHECK(NEW_GammaConfigStruct *gammaConfig_t)
{
    uint8_t gray;
    double gamma2_2;
    double xyz[3],XYLv[3];
    char gamma_state[2];
    char filename[64];
    uint16_t band = gammaConfig_t->Order;    
    char *RECHECK_FILENAME = "20XXX_RECHECK.csv";
    sprintf(filename,"%s", RECHECK_FILENAME); 
    SDCard_Printf(filename, "ID_Number,MODE_LEVE,X,Y,Lv,Gamma\r\n");
    for(int i = 0; i < band; i++)
    {     
        gray = gammaConfig_t->OGammaConfig[i].level;
        GPU_LoadFrame(100);
        Img_Full(gray, gray, gray); 
        GPU_DisplayFrame(100);
        Delay_ms(900);
        gammaConfig_t->readLv(3, XYLv, xyz);
        gamma2_2 = loga(xyz[1]/gammaConfig_t->maxlv, gray/255.0);
        if(gray == 0 || gray == 255){
            printf("%d, x:%.4f, y:%.4f, Lv:%.4f\r\n",gray, XYLv[0], XYLv[1], XYLv[2]);                
            SDCard_Printf(filename, ",%d,%.4f,%.4f,%.4f\r\n",gray, XYLv[0], XYLv[1], XYLv[2]);}
        else{
            printf("%d, x:%.4f, y:%.4f, Lv:%.4f, gamma:%.4f\r\n",gray, XYLv[0], XYLv[1], XYLv[2], gamma2_2);
            SDCard_Printf(filename, ",%d,%.4f,%.4f,%.4f,%.4f\r\n",gray, XYLv[0], XYLv[1], XYLv[2], gamma2_2);
        }
        Delay_ms(100);
    }
}

/*
 * @function : SD卡日志打印格式重定义
 * @description ：根据已有的数据，只修改SDCard_Printf字符串格式内�??
 * @parm gammaConfig : 对应gamma(Normal、HBM、idle)模式结构�??
 * @parm i : 当前第几灰阶
 * @parm filename 保存文件�??
 * @修改时，不要改变形参形式
 * 
 * @SDCard_Printf : 该函数为通过SD卡保存打印内容，fliename是文件名及其格式eg."xxxx.txt"|"xxxx.csv"
 * 					�??.csv文件中，打印的内�??","遇到逗号使光标跳到当前行下一列格�??
 * 					"\r\n"使光标跳到下一行的第一列格�??*/
/*
 *
 * @description �?? 当前打印格式如下
 *         A       B       C       D       E               F       G
	0       ID_Num  MODE_L  X       Y       Lv              Gamma
	1               255:OK  0.30    0.31    1000.00000
	2               253:OK  0.30    0.31    982.82626       2.200
	3               249:NG  0.33    0.34    960.12300       1.800
 *
 * @description �?? �??0行的列表头由Log_hard[]控制若需自行定义请定位前往
 */
static char Log_hard[] = "ID_Number,MODE_LEVE,X,Y,Lv,Gamma\r\n";//Log文件开头，与Log_print配合自定义需要的Log格式
void Log_print(NEW_GammaConfigStruct *gammaConfig, int i, char state, char* filename)
{	
	//(state == 'Y'|'N'  判断信息用于区分卡控是否通过时的打印内容)
	if(state == 'Y')//此处'Y'判断是当卡控通过时需要打印的信息
	{
		if(gammaConfig->OGammaConfig[i].level == 0 || gammaConfig->OGammaConfig[i].level == 255)//此处判断是否需要打印gamma�??0阶和255阶没有gamma�??		
		{	
			#ifdef SDCard_Log
			sprintf(array, ",%d:OK,%.4f,%.4f,%.4f\r\n",gammaConfig->OGammaConfig[i].level,\
			gammaConfig->OGammaConfig[i].measureXYLv[0], gammaConfig->OGammaConfig[i].measureXYLv[1], gammaConfig->OGammaConfig[i].measureXYLv[2]);
			strcat(save_log, array);
			SDCard_Printf(filename, ",%d:OK,%.4f,%.4f,%.4f\r\n",gammaConfig->OGammaConfig[i].level,\
			gammaConfig->OGammaConfig[i].measureXYLv[0], gammaConfig->OGammaConfig[i].measureXYLv[1], gammaConfig->OGammaConfig[i].measureXYLv[2]);
			#endif
			
			#ifdef Pcui_Log
			pcui_writeFileContent(Code_file,",%d:OK,%.4f,%.4f,%.4f\r\n",gammaConfig->OGammaConfig[i].level,
			gammaConfig->OGammaConfig[i].measureXYLv[0], gammaConfig->OGammaConfig[i].measureXYLv[1], gammaConfig->OGammaConfig[i].measureXYLv[2]);
			#endif
		}
		else
		{
			#ifdef SDCard_Log
			sprintf(array, ",%d:OK,%.4f,%.4f,%.4f,%.4f\r\n",gammaConfig->OGammaConfig[i].level,\
			gammaConfig->OGammaConfig[i].measureXYLv[0], gammaConfig->OGammaConfig[i].measureXYLv[1], gammaConfig->OGammaConfig[i].measureXYLv[2],\
			loga(gammaConfig->OGammaConfig[i].measureXYLv[2]/gammaConfig->maxlv,gammaConfig->OGammaConfig[i].level/255.0));
			strcat(save_log, array);
			// SDCard_Printf(filename, ",%d:OK,%.4f,%.4f,%.4f,%.4f\r\n",gammaConfig->OGammaConfig[i].level,\
			// gammaConfig->OGammaConfig[i].measureXYLv[0], gammaConfig->OGammaConfig[i].measureXYLv[1], gammaConfig->OGammaConfig[i].measureXYLv[2],\
			// loga(gammaConfig->OGammaConfig[i].measureXYLv[2]/gammaConfig->maxlv,gammaConfig->OGammaConfig[i].level/255.0));
			#endif
			
			#ifdef Pcui_Log
			pcui_writeFileContent(Code_file,",%d:OK,%.4f,%.4f,%.4f,%.4f\r\n",gammaConfig->OGammaConfig[i].level,
			gammaConfig->OGammaConfig[i].measureXYLv[0], gammaConfig->OGammaConfig[i].measureXYLv[1], gammaConfig->OGammaConfig[i].measureXYLv[2],
			loga(gammaConfig->OGammaConfig[i].measureXYLv[2]/gammaConfig->maxlv,gammaConfig->OGammaConfig[i].level/255.0));
			#endif
		}
	}		
	else if(state == 'N')//此处'N'判断是当卡控不通过时需要打印的信息
	{
		 if(gammaConfig->OGammaConfig[i].level == 0 || gammaConfig->OGammaConfig[i].level == 255)//此处判断是否需要打印gamma�??0阶和255阶没有gamma�??
		 {
			#ifdef SDCard_Log
			sprintf(array, ",%d:NG,%.4f,%.4f,%.4f\r\n",gammaConfig->OGammaConfig[i].level,\
			gammaConfig->OGammaConfig[i].measureXYLv[0], gammaConfig->OGammaConfig[i].measureXYLv[1], gammaConfig->OGammaConfig[i].measureXYLv[2]);
			strcat(save_log, array);
			// SDCard_Printf(filename, ",%d:NG,%.4f,%.4f,%.4f\r\n",gammaConfig->OGammaConfig[i].level,\
			// gammaConfig->OGammaConfig[i].measureXYLv[0], gammaConfig->OGammaConfig[i].measureXYLv[1], gammaConfig->OGammaConfig[i].measureXYLv[2]);
			#endif
			 
		  #ifdef Pcui_Log
			pcui_writeFileContent(Code_file, ",%d:NG,%.4f,%.4f,%.4f\r\n",gammaConfig->OGammaConfig[i].level, 
			gammaConfig->OGammaConfig[i].measureXYLv[0], gammaConfig->OGammaConfig[i].measureXYLv[1], gammaConfig->OGammaConfig[i].measureXYLv[2]);
			#endif
		 }
		else
		{
			#ifdef SDCard_Log
			sprintf(array, ",%d:NG,%.4f,%.4f,%.4f,%.4f\r\n",gammaConfig->OGammaConfig[i].level,\
			gammaConfig->OGammaConfig[i].measureXYLv[0], gammaConfig->OGammaConfig[i].measureXYLv[1], gammaConfig->OGammaConfig[i].measureXYLv[2],\
			loga(gammaConfig->OGammaConfig[i].measureXYLv[2]/gammaConfig->maxlv,gammaConfig->OGammaConfig[i].level/255.0));
			strcat(save_log, array);
			// SDCard_Printf(filename, ",%d:NG,%.4f,%.4f,%.4f,%.4f\r\n",gammaConfig->OGammaConfig[i].level,\
			// gammaConfig->OGammaConfig[i].measureXYLv[0], gammaConfig->OGammaConfig[i].measureXYLv[1], gammaConfig->OGammaConfig[i].measureXYLv[2],\
			// loga(gammaConfig->OGammaConfig[i].measureXYLv[2]/gammaConfig->maxlv,gammaConfig->OGammaConfig[i].level/255.0));
			#endif
			
			#ifdef Pcui_Log
			pcui_writeFileContent(Code_file, ",%d:NG,%.4f,%.4f,%.4f,%.4f\r\n",gammaConfig->OGammaConfig[i].level, 
			gammaConfig->OGammaConfig[i].measureXYLv[0], gammaConfig->OGammaConfig[i].measureXYLv[1], gammaConfig->OGammaConfig[i].measureXYLv[2], 
			loga(gammaConfig->OGammaConfig[i].measureXYLv[2]/gammaConfig->maxlv,gammaConfig->OGammaConfig[i].level/255.0));
			#endif
		}
	}		
}
/******************************************************************************************/



/**系统函数声明�??**************************************************************************/
extern int ssss(void);
extern void SwitchFrame(unsigned int frame);
//低阶特殊处理
static int run_gammaSpecial(NEW_GammaConfigStruct *gammaConfig, float P, sky_comDriver *dev);

/******************************************************************************************/


/**系统变量声明�??**************************************************************************/

XYLvTypeDef cl_xylv;

#ifdef DEBUG_GAMMA_INFO
    int fun1 =1;
#else
    int fun1 =0;
#endif

#ifdef GammaRegWrite255
    int fun2 =1;
#else
    int fun1 =0;
#endif

#ifdef DEBUG_GAMMA_LOG
    int fun3 =1;
    char* LOG_NAME = LOG_FILENAME;
#else
    int fun3 =0;
    char* LOG_NAME = LOG_FILENAME;
#endif

/******************************************************************************************/


/**系统接口函数�??**************************************************************************/

/*
*函数�?? : getGammaADC
*描述   : 读镜头ADC,
*参数   : adc：adc的值， x:读的次数
*返回   : 0（无作用�??
*/
int getGammaADC(float adc[3], int x)
{
	XYZTypeDef xyz;
	float XYZ[3];
	int i = x;
	int k = 0;
	
	XYZ[0] = 0;
	XYZ[1] = 0;
	XYZ[2] = 0;
	
	while (i > 0)
	{
		i--;
		readXYZ(CFL_ID0, &xyz);
		k++;
		XYZ[0] += xyz.X;
		XYZ[1] += xyz.Y;
		XYZ[2] += xyz.Z;	
	}
	
	adc[0] = XYZ[0] / k;
	adc[1] = XYZ[1] / k;
	adc[2] = XYZ[2] / k;
	
	return 0;
}

/*
*函数�?? : getGammaXYZ_
*描述   : 读镜头ADC,
*参数   : xyz：RGB的值， x:读的次数
*返回   : 0（无作用�??
*/
int getGammaXYZ_(float xyz[3], int x)
{
	float adc[3];
	getGammaADC(adc,x);
	
	float matK1[18];
	readMatrix_Cal(0x01, (float *)matK1);
	float outTem[3] = {0};
	float out[3] = {0};
	
	for (int i = 0; i < 3; i++)
	{
		outTem[i] = 0;
		for (int j = 0; j < 3; j++)
		{
			outTem[i] += adc[j] * matK1[j + i*3];
		}
	}
	
	for (int i = 0; i < 3; i++)
	{
		out[i] = 0;
		for (int j = 0; j < 3; j++)
		{
			out[i] += outTem[j] * matK1[j + i*3 + 9];
		}
	}
	
	xyz[0] = out[0];
	xyz[1] = out[1];
	xyz[2] = out[2];
	
	return 0;
}

/*
*函数�?? : readLv  CL2
*描述   : 镜头读数�??
*参数   : n:读多少次  ，xyz[3]:R,G,B的亮�??   �?? xyLv[3]：X,Y,LV的�?
*返回   : 0（无作用）虽然返回的�??0，但是这个函数执行完毕后，会改变 xyz[3]�?? xyLv[3]的值，可以理解为返回这两个数组
*/
int readLv(int n, float xyz[3], float xyLv[3])
{
	getGammaXYZ_(xyz, n);
	
	xyLv[0] = xyz[0]/(xyz[0]+xyz[1]+xyz[2]);
	xyLv[1] = xyz[1]/(xyz[0]+xyz[1]+xyz[2]);
	xyLv[2] = xyz[1];
	return 0;
}

/*
	CL2_read_xylv 
	num 
*/
int CL2_read_xylv(uint8_t num,float xylv[3],float xyz[3])
{

	xylv[0] = 0;
	xylv[1] = 0;
	xylv[2] = 0;
	for (uint8_t i =0;i<num;i++)
	{
		CFL_GetMeasureXYLv(CFL_ID0,&cl_xylv);	

		xylv[0] += cl_xylv.X;
		xylv[1] += cl_xylv.Y;
		xylv[2] += cl_xylv.Lv;
	}

	xylv[0] =xylv[0] /num;
	xylv[1] =xylv[1] /num;
	xylv[2]=xylv[2]/num;
	

	xyz[0]= xylv[2] * xylv[0] / xylv[1];
	xyz[1]=xylv[2];
	xyz[2] =xylv[2] * (1-xylv[0]-xylv[1]) / xylv[1];
	

	return 0;

}

/*
*函数�?? : readLv_CA310    CA310
*描述   : 镜头读数�??
*参数   : n:读多少次  ，xyz[3]:R,G,B的亮�??   �?? xyLv[3]：X,Y,LV的�?
*返回   : 0（无作用）虽然返回的�??0，但是这个函数执行完毕后，会改变 xyz[3]�?? xyLv[3]的值，可以理解为返回这两个数组
*/
int readLv_CA310(uint8_t n, double xyLv[3], double xyz[3])
{
    float CA310_X[1],CA310_Y[1],CA310_Lv[1];
    xyLv[0] = 0;
    xyLv[1] = 0;
    xyLv[2] = 0;
       
    for(int i=0;i< n ;i++)
    {
    
        #ifdef CA310
        CA310_GetLvXY(&CA310_Lv[0],&CA310_X[0],&CA310_Y[0]); 
        #endif
			
        #ifdef CA410
        COMCA410_GetLvXY(&CA310_Lv[0],&CA310_X[0],&CA310_Y[0]); 
        #endif
		
				#ifdef CL5
				CA310_GetLvXY(1,&CA310_Lv[0],&CA310_X[0],&CA310_Y[0]); 
				#endif
       
        xyLv[0] +=  CA310_X[0];  
        xyLv[1] +=  CA310_Y[0]; 
        xyLv[2] +=  CA310_Lv[0];         
        
    }   
    
    xyLv[0] = xyLv[0] / n;
    xyLv[1] = xyLv[1] / n;
    xyLv[2] = xyLv[2] / n;
    
    xyz[0] = xyLv[2] * xyLv[0] / xyLv[1];
    xyz[1] = xyLv[2];
    xyz[2] = xyLv[2] * (1 - xyLv[0] - xyLv[1] ) / xyLv[1];
 
	return 0;
}

/******************************************************************************************/


/**用户接口函数�??**************************************************************************/
void read_gamma(void)
{
	int i;
	int NOR_flag,HBM_flag,NOR2_flag,AOD_flag,NOR3_flag;
	unsigned char ReadCode[NORMAL_DEF_GAMMA_ORDER_NUM*3*2];
	
	int NOR_ReadCode[NORMAL_DEF_GAMMA_ORDER_NUM*3];   //NORMAL_DEF_GAMMA_ORDER_NUM 27个绑点数�??
	int HBM_ReadCode[NORMAL_DEF_GAMMA_ORDER_NUM*3+12];
	int NOR2_ReadCode[NORMAL_DEF_GAMMA_ORDER_NUM*3];
	int NOR3_ReadCode[NORMAL_DEF_GAMMA_ORDER_NUM*3];
	int AOD_ReadCode[NORMAL_DEF_GAMMA_ORDER_NUM*3];
	const u8 reg_adrr[] = NORMAL_DEF_GAMMA_REG_ADRR;
	
	Readgamma_flag=0;  //23.12.28 XQC增加
	
	/*2828进入LP 模式*/
	SSD2828_A_CS_0();
	SSD2828_LP();
	Delay_ms(50);
	/****************************************read-HBM****************************************/
	MIPI_WR(0x15,0xFE,0x53);
	
	for(i=0;i<NORMAL_DEF_GAMMA_ORDER_NUM*3*2;i++)
	{
		SSD2828_DcsReadDT06(reg_adrr[i], 1, &ReadCode[i]);
	}
	
	for(i=0;i<NORMAL_DEF_GAMMA_ORDER_NUM*3;i++)
	{
		HBM_ReadCode[i]=(((ReadCode[2*i]<<8)&0x300)|(ReadCode[2*i+1]));	
	}
	
	printf("HBM:%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n",
									HBM_ReadCode[0],HBM_ReadCode[1],HBM_ReadCode[2],HBM_ReadCode[3],HBM_ReadCode[4],HBM_ReadCode[5],HBM_ReadCode[6],HBM_ReadCode[7],HBM_ReadCode[8],HBM_ReadCode[9],
									HBM_ReadCode[10],HBM_ReadCode[11],HBM_ReadCode[12],HBM_ReadCode[13],HBM_ReadCode[14],HBM_ReadCode[15],HBM_ReadCode[16],HBM_ReadCode[17],HBM_ReadCode[18],HBM_ReadCode[19],
									HBM_ReadCode[20],HBM_ReadCode[21],HBM_ReadCode[22],HBM_ReadCode[23],HBM_ReadCode[24],HBM_ReadCode[25],HBM_ReadCode[26],
									HBM_ReadCode[27],HBM_ReadCode[28],HBM_ReadCode[29],HBM_ReadCode[30],HBM_ReadCode[31],HBM_ReadCode[32],HBM_ReadCode[33],HBM_ReadCode[34],HBM_ReadCode[35],HBM_ReadCode[36],
									HBM_ReadCode[37],HBM_ReadCode[38],HBM_ReadCode[39],HBM_ReadCode[40],HBM_ReadCode[41],HBM_ReadCode[42],HBM_ReadCode[43],HBM_ReadCode[44],HBM_ReadCode[45],HBM_ReadCode[46],
									HBM_ReadCode[47],HBM_ReadCode[48],HBM_ReadCode[49],HBM_ReadCode[50],HBM_ReadCode[51],HBM_ReadCode[52],HBM_ReadCode[53],
									HBM_ReadCode[54],HBM_ReadCode[55],HBM_ReadCode[56],HBM_ReadCode[57],HBM_ReadCode[58],HBM_ReadCode[59],HBM_ReadCode[60],HBM_ReadCode[61],HBM_ReadCode[62],HBM_ReadCode[63],
									HBM_ReadCode[64],HBM_ReadCode[65],HBM_ReadCode[66],HBM_ReadCode[67],HBM_ReadCode[68],HBM_ReadCode[69],HBM_ReadCode[70],HBM_ReadCode[71],HBM_ReadCode[72],HBM_ReadCode[73],
									HBM_ReadCode[74],HBM_ReadCode[75],HBM_ReadCode[76],HBM_ReadCode[77],HBM_ReadCode[78],HBM_ReadCode[79],HBM_ReadCode[80]
//									HBM_ReadCode[81],HBM_ReadCode[82],HBM_ReadCode[83],HBM_ReadCode[84],HBM_ReadCode[85],HBM_ReadCode[86],HBM_ReadCode[87],HBM_ReadCode[88],HBM_ReadCode[89],HBM_ReadCode[90],
//									HBM_ReadCode[91],HBM_ReadCode[92]
							);
	//pcui_writeFileContent  SDCard_Printf
	pcui_writeFileContent(Code_file,
	"HBM:\n%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,\r\n%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,\r\n%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,\r\n",
	HBM_ReadCode[0],HBM_ReadCode[1],HBM_ReadCode[2],HBM_ReadCode[3],HBM_ReadCode[4],HBM_ReadCode[5],HBM_ReadCode[6],HBM_ReadCode[7],HBM_ReadCode[8],HBM_ReadCode[9],
	HBM_ReadCode[10],HBM_ReadCode[11],HBM_ReadCode[12],HBM_ReadCode[13],HBM_ReadCode[14],HBM_ReadCode[15],HBM_ReadCode[16],HBM_ReadCode[17],HBM_ReadCode[18],HBM_ReadCode[19],
	HBM_ReadCode[20],HBM_ReadCode[21],HBM_ReadCode[22],HBM_ReadCode[23],HBM_ReadCode[24],HBM_ReadCode[25],HBM_ReadCode[26],
	HBM_ReadCode[27],HBM_ReadCode[28],HBM_ReadCode[29],HBM_ReadCode[30],HBM_ReadCode[31],HBM_ReadCode[32],HBM_ReadCode[33],HBM_ReadCode[34],HBM_ReadCode[35],HBM_ReadCode[36],
	HBM_ReadCode[37],HBM_ReadCode[38],HBM_ReadCode[39],HBM_ReadCode[40],HBM_ReadCode[41],HBM_ReadCode[42],HBM_ReadCode[43],HBM_ReadCode[44],HBM_ReadCode[45],HBM_ReadCode[46],
	HBM_ReadCode[47],HBM_ReadCode[48],HBM_ReadCode[49],HBM_ReadCode[50],HBM_ReadCode[51],HBM_ReadCode[52],HBM_ReadCode[53],
	HBM_ReadCode[54],HBM_ReadCode[55],HBM_ReadCode[56],HBM_ReadCode[57],HBM_ReadCode[58],HBM_ReadCode[59],HBM_ReadCode[60],HBM_ReadCode[61],HBM_ReadCode[62],HBM_ReadCode[63],
	HBM_ReadCode[64],HBM_ReadCode[65],HBM_ReadCode[66],HBM_ReadCode[67],HBM_ReadCode[68],HBM_ReadCode[69],HBM_ReadCode[70],HBM_ReadCode[71],HBM_ReadCode[72],HBM_ReadCode[73],
	HBM_ReadCode[74],HBM_ReadCode[75],HBM_ReadCode[76],HBM_ReadCode[77],HBM_ReadCode[78],HBM_ReadCode[79],HBM_ReadCode[80]
//	HBM_ReadCode[81],HBM_ReadCode[82],HBM_ReadCode[83],HBM_ReadCode[84],HBM_ReadCode[85],HBM_ReadCode[86],HBM_ReadCode[87],HBM_ReadCode[88],HBM_ReadCode[89],HBM_ReadCode[90],
//	HBM_ReadCode[91],HBM_ReadCode[92]
							);
	
	/****************************************read-normal****************************************/
	MIPI_WR(0x15,0xFE,0x50);
	
	for(i=0;i<NORMAL_DEF_GAMMA_ORDER_NUM*3*2;i++)
	{
		SSD2828_DcsReadDT06(reg_adrr[i], 1, &ReadCode[i]);
	}
	
	for(i=0;i<NORMAL_DEF_GAMMA_ORDER_NUM*3;i++)
	{
		NOR_ReadCode[i]=(((ReadCode[2*i]<<8)&0x300)|(ReadCode[2*i+1]));	
	}
	
	printf("normal:%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n",
									NOR_ReadCode[0],NOR_ReadCode[1],NOR_ReadCode[2],NOR_ReadCode[3],NOR_ReadCode[4],NOR_ReadCode[5],NOR_ReadCode[6],NOR_ReadCode[7],NOR_ReadCode[8],NOR_ReadCode[9],
									NOR_ReadCode[10],NOR_ReadCode[11],NOR_ReadCode[12],NOR_ReadCode[13],NOR_ReadCode[14],NOR_ReadCode[15],NOR_ReadCode[16],NOR_ReadCode[17],NOR_ReadCode[18],NOR_ReadCode[19],
									NOR_ReadCode[20],NOR_ReadCode[21],NOR_ReadCode[22],NOR_ReadCode[23],NOR_ReadCode[24],NOR_ReadCode[25],NOR_ReadCode[26],
									NOR_ReadCode[27],NOR_ReadCode[28],NOR_ReadCode[29],NOR_ReadCode[30],NOR_ReadCode[31],NOR_ReadCode[32],NOR_ReadCode[33],NOR_ReadCode[34],NOR_ReadCode[35],NOR_ReadCode[36],
									NOR_ReadCode[37],NOR_ReadCode[38],NOR_ReadCode[39],NOR_ReadCode[40],NOR_ReadCode[41],NOR_ReadCode[42],NOR_ReadCode[43],NOR_ReadCode[44],NOR_ReadCode[45],NOR_ReadCode[46],
									NOR_ReadCode[47],NOR_ReadCode[48],NOR_ReadCode[49],NOR_ReadCode[50],NOR_ReadCode[51],NOR_ReadCode[52],NOR_ReadCode[53],
									NOR_ReadCode[54],NOR_ReadCode[55],NOR_ReadCode[56],NOR_ReadCode[57],NOR_ReadCode[58],NOR_ReadCode[59],NOR_ReadCode[60],NOR_ReadCode[61],NOR_ReadCode[62],NOR_ReadCode[63],
									NOR_ReadCode[64],NOR_ReadCode[65],NOR_ReadCode[66],NOR_ReadCode[67],NOR_ReadCode[68],NOR_ReadCode[69],NOR_ReadCode[70],NOR_ReadCode[71],NOR_ReadCode[72],NOR_ReadCode[73],
									NOR_ReadCode[74],NOR_ReadCode[75],NOR_ReadCode[76],NOR_ReadCode[77],NOR_ReadCode[78],NOR_ReadCode[79],NOR_ReadCode[80]
						 );
	//pcui_writeFileContent 		
	pcui_writeFileContent(Code_file,
	"normal:\n%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,\r\n%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,\r\n%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,\r\n",
	NOR_ReadCode[0],NOR_ReadCode[1],NOR_ReadCode[2],NOR_ReadCode[3],NOR_ReadCode[4],NOR_ReadCode[5],NOR_ReadCode[6],NOR_ReadCode[7],NOR_ReadCode[8],NOR_ReadCode[9],
	NOR_ReadCode[10],NOR_ReadCode[11],NOR_ReadCode[12],NOR_ReadCode[13],NOR_ReadCode[14],NOR_ReadCode[15],NOR_ReadCode[16],NOR_ReadCode[17],NOR_ReadCode[18],NOR_ReadCode[19],
	NOR_ReadCode[20],NOR_ReadCode[21],NOR_ReadCode[22],NOR_ReadCode[23],NOR_ReadCode[24],NOR_ReadCode[25],NOR_ReadCode[26],
	NOR_ReadCode[27],NOR_ReadCode[28],NOR_ReadCode[29],NOR_ReadCode[30],NOR_ReadCode[31],NOR_ReadCode[32],NOR_ReadCode[33],NOR_ReadCode[34],NOR_ReadCode[35],NOR_ReadCode[36],
	NOR_ReadCode[37],NOR_ReadCode[38],NOR_ReadCode[39],NOR_ReadCode[40],NOR_ReadCode[41],NOR_ReadCode[42],NOR_ReadCode[43],NOR_ReadCode[44],NOR_ReadCode[45],NOR_ReadCode[46],
	NOR_ReadCode[47],NOR_ReadCode[48],NOR_ReadCode[49],NOR_ReadCode[50],NOR_ReadCode[51],NOR_ReadCode[52],NOR_ReadCode[53],
	NOR_ReadCode[54],NOR_ReadCode[55],NOR_ReadCode[56],NOR_ReadCode[57],NOR_ReadCode[58],NOR_ReadCode[59],NOR_ReadCode[60],NOR_ReadCode[61],NOR_ReadCode[62],NOR_ReadCode[63],
	NOR_ReadCode[64],NOR_ReadCode[65],NOR_ReadCode[66],NOR_ReadCode[67],NOR_ReadCode[68],NOR_ReadCode[69],NOR_ReadCode[70],NOR_ReadCode[71],NOR_ReadCode[72],NOR_ReadCode[73],
	NOR_ReadCode[74],NOR_ReadCode[75],NOR_ReadCode[76],NOR_ReadCode[77],NOR_ReadCode[78],NOR_ReadCode[79],NOR_ReadCode[80]
						 );
						 
//	NormalR26=NOR_ReadCode[26];  //24.03.07 JWB增加

	
	/****************************************read-normal2****************************************/
	MIPI_WR(0x15,0xFE,0x51);	
	
	for(i=0;i<NORMAL_DEF_GAMMA_ORDER_NUM*3*2;i++)
	{
		SSD2828_DcsReadDT06(reg_adrr[i], 1, &ReadCode[i]);
	}
	
	for(i=0;i<NORMAL_DEF_GAMMA_ORDER_NUM*3;i++)
	{
		NOR2_ReadCode[i]=(((ReadCode[2*i]<<8)&0x300)|(ReadCode[2*i+1]));	
	}
	
	printf("normal2:%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n",
									NOR2_ReadCode[0],NOR2_ReadCode[1],NOR2_ReadCode[2],NOR2_ReadCode[3],NOR2_ReadCode[4],NOR2_ReadCode[5],NOR2_ReadCode[6],NOR2_ReadCode[7],NOR2_ReadCode[8],NOR2_ReadCode[9],
									NOR2_ReadCode[10],NOR2_ReadCode[11],NOR2_ReadCode[12],NOR2_ReadCode[13],NOR2_ReadCode[14],NOR2_ReadCode[15],NOR2_ReadCode[16],NOR2_ReadCode[17],NOR2_ReadCode[18],NOR2_ReadCode[19],
									NOR2_ReadCode[20],NOR2_ReadCode[21],NOR2_ReadCode[22],NOR2_ReadCode[23],NOR2_ReadCode[24],NOR2_ReadCode[25],NOR2_ReadCode[26],
									NOR2_ReadCode[27],NOR2_ReadCode[28],NOR2_ReadCode[29],NOR2_ReadCode[30],NOR2_ReadCode[31],NOR2_ReadCode[32],NOR2_ReadCode[33],NOR2_ReadCode[34],NOR2_ReadCode[35],NOR2_ReadCode[36],
									NOR2_ReadCode[37],NOR2_ReadCode[38],NOR2_ReadCode[39],NOR2_ReadCode[40],NOR2_ReadCode[41],NOR2_ReadCode[42],NOR2_ReadCode[43],NOR2_ReadCode[44],NOR2_ReadCode[45],NOR2_ReadCode[46],
									NOR2_ReadCode[47],NOR2_ReadCode[48],NOR2_ReadCode[49],NOR2_ReadCode[50],NOR2_ReadCode[51],NOR2_ReadCode[52],NOR2_ReadCode[53],
									NOR2_ReadCode[54],NOR2_ReadCode[55],NOR2_ReadCode[56],NOR2_ReadCode[57],NOR2_ReadCode[58],NOR2_ReadCode[59],NOR2_ReadCode[60],NOR2_ReadCode[61],NOR2_ReadCode[62],NOR2_ReadCode[63],
									NOR2_ReadCode[64],NOR2_ReadCode[65],NOR2_ReadCode[66],NOR2_ReadCode[67],NOR2_ReadCode[68],NOR2_ReadCode[69],NOR2_ReadCode[70],NOR2_ReadCode[71],NOR2_ReadCode[72],NOR2_ReadCode[73],
									NOR2_ReadCode[74],NOR2_ReadCode[75],NOR2_ReadCode[76],NOR2_ReadCode[77],NOR2_ReadCode[78],NOR2_ReadCode[79],NOR2_ReadCode[80]
						 );	
	//pcui_writeFileContent 		
	pcui_writeFileContent(Code_file,
	"nor2:\n%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,\r\n%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,\r\n%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,\r\n",
	NOR2_ReadCode[0],NOR2_ReadCode[1],NOR2_ReadCode[2],NOR2_ReadCode[3],NOR2_ReadCode[4],NOR2_ReadCode[5],NOR2_ReadCode[6],NOR2_ReadCode[7],NOR2_ReadCode[8],NOR2_ReadCode[9],
	NOR2_ReadCode[10],NOR2_ReadCode[11],NOR2_ReadCode[12],NOR2_ReadCode[13],NOR2_ReadCode[14],NOR2_ReadCode[15],NOR2_ReadCode[16],NOR2_ReadCode[17],NOR2_ReadCode[18],NOR2_ReadCode[19],
	NOR2_ReadCode[20],NOR2_ReadCode[21],NOR2_ReadCode[22],NOR2_ReadCode[23],NOR2_ReadCode[24],NOR2_ReadCode[25],NOR2_ReadCode[26],
	NOR2_ReadCode[27],NOR2_ReadCode[28],NOR2_ReadCode[29],NOR2_ReadCode[30],NOR2_ReadCode[31],NOR2_ReadCode[32],NOR2_ReadCode[33],NOR2_ReadCode[34],NOR2_ReadCode[35],NOR2_ReadCode[36],
	NOR2_ReadCode[37],NOR2_ReadCode[38],NOR2_ReadCode[39],NOR2_ReadCode[40],NOR2_ReadCode[41],NOR2_ReadCode[42],NOR2_ReadCode[43],NOR2_ReadCode[44],NOR2_ReadCode[45],NOR2_ReadCode[46],
	NOR2_ReadCode[47],NOR2_ReadCode[48],NOR2_ReadCode[49],NOR2_ReadCode[50],NOR2_ReadCode[51],NOR2_ReadCode[52],NOR2_ReadCode[53],
	NOR2_ReadCode[54],NOR2_ReadCode[55],NOR2_ReadCode[56],NOR2_ReadCode[57],NOR2_ReadCode[58],NOR2_ReadCode[59],NOR2_ReadCode[60],NOR2_ReadCode[61],NOR2_ReadCode[62],NOR2_ReadCode[63],
	NOR2_ReadCode[64],NOR2_ReadCode[65],NOR2_ReadCode[66],NOR2_ReadCode[67],NOR2_ReadCode[68],NOR2_ReadCode[69],NOR2_ReadCode[70],NOR2_ReadCode[71],NOR2_ReadCode[72],NOR2_ReadCode[73],
	NOR2_ReadCode[74],NOR2_ReadCode[75],NOR2_ReadCode[76],NOR2_ReadCode[77],NOR2_ReadCode[78],NOR2_ReadCode[79],NOR2_ReadCode[80]
						 );

	
	/****************************************read-AOD****************************************/
	MIPI_WR(0x15,0xFE,0x52);	//1.32-028 BOE
	
	for(i=0;i<NORMAL_DEF_GAMMA_ORDER_NUM*3*2;i++)
	{
		SSD2828_DcsReadDT06(reg_adrr[i], 1, &ReadCode[i]);
	}
	
	for(i=0;i<NORMAL_DEF_GAMMA_ORDER_NUM*3;i++)
	{
		AOD_ReadCode[i]=(((ReadCode[2*i]<<8)&0x300)|(ReadCode[2*i+1]));	
	}
	
	printf("AOD:%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n",
									AOD_ReadCode[0],AOD_ReadCode[1],AOD_ReadCode[2],AOD_ReadCode[3],AOD_ReadCode[4],AOD_ReadCode[5],AOD_ReadCode[6],AOD_ReadCode[7],AOD_ReadCode[8],AOD_ReadCode[9],
									AOD_ReadCode[10],AOD_ReadCode[11],AOD_ReadCode[12],AOD_ReadCode[13],AOD_ReadCode[14],AOD_ReadCode[15],AOD_ReadCode[16],AOD_ReadCode[17],AOD_ReadCode[18],AOD_ReadCode[19],
									AOD_ReadCode[20],AOD_ReadCode[21],AOD_ReadCode[22],AOD_ReadCode[23],AOD_ReadCode[24],AOD_ReadCode[25],AOD_ReadCode[26],
									AOD_ReadCode[27],AOD_ReadCode[28],AOD_ReadCode[29],AOD_ReadCode[30],AOD_ReadCode[31],AOD_ReadCode[32],AOD_ReadCode[33],AOD_ReadCode[34],AOD_ReadCode[35],AOD_ReadCode[36],
									AOD_ReadCode[37],AOD_ReadCode[38],AOD_ReadCode[39],AOD_ReadCode[40],AOD_ReadCode[41],AOD_ReadCode[42],AOD_ReadCode[43],AOD_ReadCode[44],AOD_ReadCode[45],AOD_ReadCode[46],
									AOD_ReadCode[47],AOD_ReadCode[48],AOD_ReadCode[49],AOD_ReadCode[50],AOD_ReadCode[51],AOD_ReadCode[52],AOD_ReadCode[53],
									AOD_ReadCode[54],AOD_ReadCode[55],AOD_ReadCode[56],AOD_ReadCode[57],AOD_ReadCode[58],AOD_ReadCode[59],AOD_ReadCode[60],AOD_ReadCode[61],AOD_ReadCode[62],AOD_ReadCode[63],
									AOD_ReadCode[64],AOD_ReadCode[65],AOD_ReadCode[66],AOD_ReadCode[67],AOD_ReadCode[68],AOD_ReadCode[69],AOD_ReadCode[70],AOD_ReadCode[71],AOD_ReadCode[72],AOD_ReadCode[73],
									AOD_ReadCode[74],AOD_ReadCode[75],AOD_ReadCode[76],AOD_ReadCode[77],AOD_ReadCode[78],AOD_ReadCode[79],AOD_ReadCode[80]
						 );	
//	//pcui_writeFileContent SDCard_Printf		
	pcui_writeFileContent(Code_file,
	"AOD:\n%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,\r\n%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,\r\n%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,\r\n",
	AOD_ReadCode[0],AOD_ReadCode[1],AOD_ReadCode[2],AOD_ReadCode[3],AOD_ReadCode[4],AOD_ReadCode[5],AOD_ReadCode[6],AOD_ReadCode[7],AOD_ReadCode[8],AOD_ReadCode[9],
	AOD_ReadCode[10],AOD_ReadCode[11],AOD_ReadCode[12],AOD_ReadCode[13],AOD_ReadCode[14],AOD_ReadCode[15],AOD_ReadCode[16],AOD_ReadCode[17],AOD_ReadCode[18],AOD_ReadCode[19],
	AOD_ReadCode[20],AOD_ReadCode[21],AOD_ReadCode[22],AOD_ReadCode[23],AOD_ReadCode[24],AOD_ReadCode[25],AOD_ReadCode[26],
	AOD_ReadCode[27],AOD_ReadCode[28],AOD_ReadCode[29],AOD_ReadCode[30],AOD_ReadCode[31],AOD_ReadCode[32],AOD_ReadCode[33],AOD_ReadCode[34],AOD_ReadCode[35],AOD_ReadCode[36],
	AOD_ReadCode[37],AOD_ReadCode[38],AOD_ReadCode[39],AOD_ReadCode[40],AOD_ReadCode[41],AOD_ReadCode[42],AOD_ReadCode[43],AOD_ReadCode[44],AOD_ReadCode[45],AOD_ReadCode[46],
	AOD_ReadCode[47],AOD_ReadCode[48],AOD_ReadCode[49],AOD_ReadCode[50],AOD_ReadCode[51],AOD_ReadCode[52],AOD_ReadCode[53],
	AOD_ReadCode[54],AOD_ReadCode[55],AOD_ReadCode[56],AOD_ReadCode[57],AOD_ReadCode[58],AOD_ReadCode[59],AOD_ReadCode[60],AOD_ReadCode[61],AOD_ReadCode[62],AOD_ReadCode[63],
	AOD_ReadCode[64],AOD_ReadCode[65],AOD_ReadCode[66],AOD_ReadCode[67],AOD_ReadCode[68],AOD_ReadCode[69],AOD_ReadCode[70],AOD_ReadCode[71],AOD_ReadCode[72],AOD_ReadCode[73],
	AOD_ReadCode[74],AOD_ReadCode[75],AOD_ReadCode[76],AOD_ReadCode[77],AOD_ReadCode[78],AOD_ReadCode[79],AOD_ReadCode[80]
						 );	
						 
	
	MIPI_WR(0x15,0xFE,0x00);
	/****************************************gamma RGB 大小对比****************************************/
	NOR_flag=0;
	HBM_flag=0;
	NOR2_flag=0;
	AOD_flag=0;
	for(i=0;i<NORMAL_DEF_GAMMA_ORDER_NUM-1;i++)
	{
		if((NOR_ReadCode[i]>NOR_ReadCode[i+1])
			||(NOR_ReadCode[NORMAL_DEF_GAMMA_ORDER_NUM+i])>(NOR_ReadCode[NORMAL_DEF_GAMMA_ORDER_NUM+i+1])
			||(NOR_ReadCode[2*NORMAL_DEF_GAMMA_ORDER_NUM+i])>(NOR_ReadCode[2*NORMAL_DEF_GAMMA_ORDER_NUM+i+1]))
		{
			NOR_flag=1;
		}
		
		if((HBM_ReadCode[i]>HBM_ReadCode[i+1])
			||(HBM_ReadCode[NORMAL_DEF_GAMMA_ORDER_NUM+i])>(HBM_ReadCode[NORMAL_DEF_GAMMA_ORDER_NUM+i+1])
			||(HBM_ReadCode[2*NORMAL_DEF_GAMMA_ORDER_NUM+i])>(HBM_ReadCode[2*NORMAL_DEF_GAMMA_ORDER_NUM+i+1]))
		{
			HBM_flag=1;
		}
		
		if((NOR2_ReadCode[i]>NOR2_ReadCode[i+1])
			||(NOR2_ReadCode[NORMAL_DEF_GAMMA_ORDER_NUM+i])>(NOR2_ReadCode[NORMAL_DEF_GAMMA_ORDER_NUM+i+1])
			||(NOR2_ReadCode[2*NORMAL_DEF_GAMMA_ORDER_NUM+i])>(NOR2_ReadCode[2*NORMAL_DEF_GAMMA_ORDER_NUM+i+1]))
		{
			NOR2_flag=1;		
		}
		if((AOD_ReadCode[i]>AOD_ReadCode[i+1])
			||(AOD_ReadCode[NORMAL_DEF_GAMMA_ORDER_NUM+i])>(AOD_ReadCode[NORMAL_DEF_GAMMA_ORDER_NUM+i+1])
			||(AOD_ReadCode[2*NORMAL_DEF_GAMMA_ORDER_NUM+i])>(AOD_ReadCode[2*NORMAL_DEF_GAMMA_ORDER_NUM+i+1]))
		{
			AOD_flag=1;		
		}				
	}

	printf("HBM_flag=%d,NOR_flag=%d,NOR2_flag=%d,AOD_flag=%d\r\n",HBM_flag,NOR_flag,NOR2_flag,AOD_flag);
	//SDCard_Printf(Code_file,"NOR_flag=%d,HBM_falg=%d,NOR2_falg=%d\r\n",NOR_flag,HBM_falg,NOR2_falg);
	pcui_writeFileContent(Code_file,"HBM_flag=%d,NOR_flag=%d,NOR2_flag=%d,AOD_flag=%d\r\n",HBM_flag,NOR_flag,NOR2_flag,AOD_flag);
	
	if(NOR_flag==1||HBM_flag==1||NOR2_flag==1||AOD_flag==1)
	{
		MONITOR_SetBee(1);
		Delay_ms(2000);
		MONITOR_SetBee(0);
		
		Readgamma_flag=1;   //23.12.28 XQC增加
		printf("gamma NG\r\n");
		//SDCard_Printf(Code_file,"gamma NG\r\n");
		pcui_writeFileContent(Code_file,"gamma NG,Readgamma_flag=1\r\n");
	}

	/*************************************************************************************************/
		//23.8.28增加下面代码，回读gamma寄存器后不能切屏问题
		/*--2828进入HS or video模式--*/
    if(PD_CommandMode())
    {
       SSD2828_HS();
    }
    else
    {
       SSD2828_Video();
    }
}

/*@description : go_XXX命名的为切换屏幕模式接口，根据自身项目情况对此接口函数进行增�??
 *				 命名规范遵守此规�??
 * 				 调用调gamma函数前，务必先自行测试你之接口是没有问题的，能够正常写入并反馈至屏幕�??
 *
 */
void go_idle(void)  
{
	QSPI_LCD_WriteCmd(0xF0,0x5A,0x5A);
	QSPI_LCD_WriteCmd(0xF1,0x5A,0x5A);
    QSPI_LCD_WriteCmd(0x66, 0x00);

    QSPI_LCD_WriteCmd(0x39, 0x00);
    QSPI_LCD_WriteCmd(0x4A, 0xFF);  // 进入AOD
	
    Power_SetMonitorStringName(4,"idle");
}

void go_hbm(void) 
{
	QSPI_LCD_WriteCmd(0xF0,0x5A,0x5A);
	QSPI_LCD_WriteCmd(0xF1,0x5A,0x5A);
    QSPI_LCD_WriteCmd(0x38, 0x00);
    QSPI_LCD_WriteCmd(0x66, 0x02);
    QSPI_LCD_WriteCmd(0x63, 0xFF);

	Delay_ms(100);

    Power_SetMonitorStringName(4,"HBM");
}

void go_normal(void) 
{
	QSPI_LCD_WriteCmd(0xF0,0x5A,0x5A);
	QSPI_LCD_WriteCmd(0xF1,0x5A,0x5A);
	QSPI_LCD_WriteCmd(0x66, 0x00); //Exit HBM	

    QSPI_LCD_WriteCmd(0x38, 0x00);

    QSPI_LCD_WriteCmd(0x51, 0xFF);  

	Delay_ms(100);
	Power_SetMonitorStringName(4,"normal");
}

void go_idle_5nit(void)  
{
	MIPI_WR(0x15,0xFE,0x43);//退出PLC
	MIPI_WR(0x15,0x00,0x00);
	MIPI_WR(0x15,0xFE,0x00);	//Exit  HBM
	MIPI_WR(0x15,0x66,0x00);		

	MIPI_WR(0x15,0xFE,0x00);	//Eter  IDLE
	MIPI_WR(0x15,0x51,0x52);  //0x3C 5nit
	MIPI_WR(0x05,0x39);
	
	SSD2828_WriteReg(0xb7,0x02,0x50);
  Power_SetMonitorStringName(4,"idle_5nit");
}

void go_idle_60nit(void)  
{
	MIPI_WR(0x15,0xFE,0x43);//退出PLC
	MIPI_WR(0x15,0x00,0x00);
	MIPI_WR(0x15,0xFE,0x00);	//Exit  HBM
	MIPI_WR(0x15,0x66,0x00);		

	MIPI_WR(0x15,0xFE,0x00);	//Eter  IDLE
	MIPI_WR(0x15,0x51,0xBA);  //0xBA  60nit
	MIPI_WR(0x05,0x39);
	
	SSD2828_WriteReg(0xb7,0x02,0x50);
  Power_SetMonitorStringName(4,"idle_60nit");
}


void go_plc(void) 
{
	MIPI_WR(0x15,0xFE,0x00);////退出AOD
	MIPI_WR(0x05,0x38);	
	MIPI_WR(0x15,0x51,0xFF);
	
//	MIPI_WR(0x15,0xFE,0x43);//进入PLC
//	MIPI_WR(0x15,0x00,0x01);
	
	MIPI_WR(0x15,0xFE,0x00);//进入高亮
	MIPI_WR(0x15,0x66,0x02);
	MIPI_WR(0x15,0x63,0xFF);

	MIPI_WR(0x15,0xFE,0x43);//进入PLC
	MIPI_WR(0x15,0x00,0x01);	
	MIPI_WR(0x15,0xFE,0x00);//
	SSD2828_WriteReg(0xb7,0x02,0x50);
	Power_SetMonitorStringName(4,"PLC");
}

void go_PLC(void) 
{
	MIPI_WR(0x15,0xFE,0x00);////退出AOD
	MIPI_WR(0x05,0x38);	
	MIPI_WR(0x15,0x51,0xFF);
	
//	MIPI_WR(0x15,0xFE,0x43);//进入PLC
//	MIPI_WR(0x15,0x00,0x01);
	
	MIPI_WR(0x15,0xFE,0x00);//进入高亮
	MIPI_WR(0x15,0x66,0x02);
	MIPI_WR(0x15,0x63,0xFF);

	MIPI_WR(0x15,0xFE,0x43);//进入PLC
	MIPI_WR(0x15,0x00,0x01);	
	MIPI_WR(0x15,0xFE,0x00);//
	SSD2828_WriteReg(0xb7,0x02,0x50);
	Power_SetMonitorStringName(4,"PLC");
}


void go_nor_51_1F(void) 
{
	MIPI_WR(0x15,0xFE,0x00);////退出AOD
	MIPI_WR(0x05,0x38);	
	MIPI_WR(0x15,0x51,0x1D); //1D 5nit   1D
	
	MIPI_WR(0x15,0xFE,0x00);/////退出高�??
	MIPI_WR(0x15,0x66,0x00);	
	
	SSD2828_WriteReg(0xb7,0x02,0x50);//ssd2828
	Power_SetMonitorStringName(4,"nor_5nit");
}

void go_normal3(void) 
{
	MIPI_WR(0x15,0xFE,0x43);//退出PLC
	MIPI_WR(0x15,0x00,0x00);
	MIPI_WR(0x15,0xFE,0x00);////退出AOD
	MIPI_WR(0x05,0x38);	
	MIPI_WR(0x15,0x51,0x1D); //1D 5nit
	
	MIPI_WR(0x15,0xFE,0x00);/////退出高�??
	MIPI_WR(0x15,0x66,0x00);	
    
//	POWER_SetVDD(4, 3.0);   //VBAT
//	Delay_ms(10);
//	POWER_SetVDD(5, -3.0);   //VBAT
//	Delay_ms(10);
	
	SSD2828_WriteReg(0xb7,0x02,0x50);
	Power_SetMonitorStringName(4,"normal3");
}

void go_normal2(void)  
{
	MIPI_WR(0x15,0xFE,0x43);//退出PLC
	MIPI_WR(0x15,0x00,0x00);
	MIPI_WR(0x15,0xFE,0x00);////退出AOD
	MIPI_WR(0x05,0x38);
	MIPI_WR(0x15,0x51,0x27);  //0x7A 120nit
	
	MIPI_WR(0x15,0xFE,0x00);/////退出高�??
	MIPI_WR(0x15,0x66,0x00);
	
	SSD2828_WriteReg(0xb7,0x02,0x50);
	Power_SetMonitorStringName(4,"normal2");
}


/**gamma寄存器全�??&255写接�??*******************************************************************/

/*@description : 若你的项目需要一次性写入所有gamma寄存器的值才能完成写入，则可参考以�??255�??
 * 				 若是通过地址索引，单独写对应地址的寄存器的值，则按照之前的方式索引
 * 				 若以地址列表索引�??255写的j无需关注，亦可写死在255的索引地址
 *  			 不处理也没有关系，底层已经作此操�??
 * 
 * 				 调用调gamma函数前，务必先自行测试你之接口是没有问题的，能够正常写入并反馈至屏幕�??
 * 
 *@naming notations : 全写命名格式--gammaRegWriteAll_XXX
 * 					  二五五写格式--gammaRegWrite_XXX
 *@eg : 			  gammaRegWriteAll_HBM(HBM全写)
 *					  gammaRegWrite_HBM(HBM二五五写)
 */

//HBM全写
int gammaRegWriteAll_HBM(uint16_t *data)
{
	int i,j=0;
    uint8_t arr[162] = {0};
	u8 d;
	const u8 reg_adrr[] = NORMAL_DEF_GAMMA_REG_ADRR;
	
	QSPI_LCD_WriteCmd(0xFF,0x5A,0x07);
	
	for (i = 0; i < sizeof(reg_adrr)/2/3; i++)
	{
		d = data[NORMAL_DEF_GAMMA_ORDER_NUM - 1 -i] / 256;
		arr[reg_adrr[j]] = d;
		// printf("addr:0x%02X,num:0x%02X\r\n",reg_adrr[j],d);
		QSPI_LCD_WriteCmd(0xF2,reg_adrr[j]+1);
		QSPI_LCD_WriteCmd(0xB1,d);
		d = data[NORMAL_DEF_GAMMA_ORDER_NUM - 1 -i] % 256;
		arr[reg_adrr[j+1]] = d;
		// printf("addr:0x%02X,num:0x%02X\r\n",reg_adrr[j+1],d);
		QSPI_LCD_WriteCmd(0xF2,reg_adrr[j+1]+1);
		QSPI_LCD_WriteCmd(0xB1,d);
		d = data[NORMAL_DEF_GAMMA_ORDER_NUM * 2 - 1 -i] / 256;
		arr[reg_adrr[j+2]] = d;
		// printf("addr:0x%02X,num:0x%02X\r\n",reg_adrr[j+2],d);
		QSPI_LCD_WriteCmd(0xF2,reg_adrr[j+2]+1);
		QSPI_LCD_WriteCmd(0xB1,d);
		d = data[NORMAL_DEF_GAMMA_ORDER_NUM * 2 - 1 -i] % 256;
		arr[reg_adrr[j+3]] = d;
		// printf("addr:0x%02X,num:0x%02X\r\n",reg_adrr[j+3],d);
		QSPI_LCD_WriteCmd(0xF2,reg_adrr[j+3]+1);
		QSPI_LCD_WriteCmd(0xB1,d);
		d = data[NORMAL_DEF_GAMMA_ORDER_NUM * 3 - 1 -i] / 256;
		arr[reg_adrr[j+4]] = d;
		// printf("addr:0x%02X,num:0x%02X\r\n",reg_adrr[j+4],d);
		QSPI_LCD_WriteCmd(0xF2,reg_adrr[j+4]+1);
		QSPI_LCD_WriteCmd(0xB1,d);
		d = data[NORMAL_DEF_GAMMA_ORDER_NUM * 3 - 1 -i] % 256;
		arr[reg_adrr[j+5]] = d;
		// printf("addr:0x%02X,num:0x%02X\r\n",reg_adrr[j+5],d);
		QSPI_LCD_WriteCmd(0xF2,reg_adrr[j+5]+1);
		QSPI_LCD_WriteCmd(0xB1,d);
		j+=6;
	}
    printf("Data to write:\r\n");
    for (i = 0; i < 162; i++)
    {
        printf("%02x ", arr[i]);
        if ((i + 1) % 12 == 0) printf("\n");
    }
    printf("\r\n");
	
	return 0;
}

//HBM255�??
int gammaRegWrite_HBM(int j, uint16_t *data)
{
	int i;
	u8 d;
	const u8 reg_adrr[] = HBM_DEF_GAMMA_REG_ADRR;
	u8 dL[3],dH[3];
	
	for(int i = 0; i < 3; i++)
    {
        dH[i] = ((data[i] >> 8) & 0x07) ;
        dL[i] = data[i] & 0xff;  
    }
	QSPI_LCD_WriteCmd(0xFF,0x5A,0x07);

	QSPI_LCD_WriteCmd(0xF2,0x01);
	QSPI_LCD_WriteCmd(0xB1,dH[0]);

	QSPI_LCD_WriteCmd(0xF2,0x02);
	QSPI_LCD_WriteCmd(0xB1,dL[0]);
		
	QSPI_LCD_WriteCmd(0xF2,0x03);
	QSPI_LCD_WriteCmd(0xB1,dH[1]);

	QSPI_LCD_WriteCmd(0xF2,0x04);
	QSPI_LCD_WriteCmd(0xB1,dL[1]);
		
	QSPI_LCD_WriteCmd(0xF2,0x05);
	QSPI_LCD_WriteCmd(0xB1,dH[2]);

	QSPI_LCD_WriteCmd(0xF2,0x06);
	QSPI_LCD_WriteCmd(0xB1,dL[2]);
	return 0;
}
int gammaRegReadAll_HBM(void)
{
	int i, res = 0;
	u8 d;
	const u8 reg_adrr[] = NORMAL_DEF_GAMMA_REG_ADRR;
	unsigned char ReadCode[256];
	
	QSPI_LCD_WriteCmd(0xF0,0x5A,0x5A);
	QSPI_LCD_WriteCmd(0xF1,0x5A,0x5A);
	QSPI_LCD_WriteCmd(0xFF,0x5A,0x07);
	
	QSPI_LCD_WriteCmd(0xF2,0x01);
	QSPI_LCD_ReadData(0xB1,162,ReadCode);

	for (i = 0; i < (sizeof(gamma_AllRead)/sizeof(gamma_AllRead[0]))/3; i++)
	{
		gamma_AllRead[i] = ReadCode[0x9C-6*i] << 8 | ReadCode[0x9D-6*i];
		gamma_AllRead[i+NORMAL_DEF_GAMMA_ORDER_NUM] = ReadCode[0x9E -6*i] << 8 | ReadCode[0x9F-6*i];
		gamma_AllRead[i+NORMAL_DEF_GAMMA_ORDER_NUM*2] = ReadCode[0xA0-6*i] << 8 | ReadCode[0xA1-6*i];
	}
	// for (i = 0; i < (sizeof(gamma_AllRead)/sizeof(gamma_AllRead[0])); i++)
	// {
	// 	printf("%d,",gamma_AllRead[i]);
	// 	if ((i+1)%27==0) printf("\r\n");	
	// }
	for (i = 0; i < sizeof(gamma_AllRead)/sizeof(gamma_AllRead[0]); i++)
	{
		if(gamma_AllRead[i] != GammaConfig_HBM.gammaSetReg[i])
		{
			res = -1;
			printf("%d->%d,",GammaConfig_HBM.gammaSetReg[i],gamma_AllRead[i]);
		}
		else
		{
			res = 1;
			printf("%d,",GammaConfig_HBM.gammaSetReg[i]);
		}
		if ((i+1)%27==0) printf("\r\n");	
	}
	return res;
}

//Normal全写
int gammaRegWriteAll_Normal(uint16_t *data)
{
	int i,j=0;
    uint8_t arr[162] = {0};
	u8 d;
	const u8 reg_adrr[] = NORMAL_DEF_GAMMA_REG_ADRR;
	
	QSPI_LCD_WriteCmd(0xFF,0x5A,0x07);
	
	for (i = 0; i < sizeof(reg_adrr)/2/3; i++)
	{
		d = data[NORMAL_DEF_GAMMA_ORDER_NUM - 1 -i] / 256;
		arr[reg_adrr[j]] = d;
		// printf("addr:0x%02X,num:0x%02X\r\n",reg_adrr[j],d);
		QSPI_LCD_WriteCmd(0xF2,reg_adrr[j]+1);
		QSPI_LCD_WriteCmd(0xB0,d);
		d = data[NORMAL_DEF_GAMMA_ORDER_NUM - 1 -i] % 256;
		arr[reg_adrr[j+1]] = d;
		// printf("addr:0x%02X,num:0x%02X\r\n",reg_adrr[j+1],d);
		QSPI_LCD_WriteCmd(0xF2,reg_adrr[j+1]+1);
		QSPI_LCD_WriteCmd(0xB0,d);
		d = data[NORMAL_DEF_GAMMA_ORDER_NUM * 2 - 1 -i] / 256;
		arr[reg_adrr[j+2]] = d;
		// printf("addr:0x%02X,num:0x%02X\r\n",reg_adrr[j+2],d);
		QSPI_LCD_WriteCmd(0xF2,reg_adrr[j+2]+1);
		QSPI_LCD_WriteCmd(0xB0,d);
		d = data[NORMAL_DEF_GAMMA_ORDER_NUM * 2 - 1 -i] % 256;
		arr[reg_adrr[j+3]] = d;
		// printf("addr:0x%02X,num:0x%02X\r\n",reg_adrr[j+3],d);
		QSPI_LCD_WriteCmd(0xF2,reg_adrr[j+3]+1);
		QSPI_LCD_WriteCmd(0xB0,d);
		d = data[NORMAL_DEF_GAMMA_ORDER_NUM * 3 - 1 -i] / 256;
		arr[reg_adrr[j+4]] = d;
		// printf("addr:0x%02X,num:0x%02X\r\n",reg_adrr[j+4],d);
		QSPI_LCD_WriteCmd(0xF2,reg_adrr[j+4]+1);
		QSPI_LCD_WriteCmd(0xB0,d);
		d = data[NORMAL_DEF_GAMMA_ORDER_NUM * 3 - 1 -i] % 256;
		arr[reg_adrr[j+5]] = d;
		// printf("addr:0x%02X,num:0x%02X\r\n",reg_adrr[j+5],d);
		QSPI_LCD_WriteCmd(0xF2,reg_adrr[j+5]+1);
		QSPI_LCD_WriteCmd(0xB0,d);
		j+=6;
	}
    printf("Data to write:\r\n");
    for (i = 0; i < 162; i++)
    {
        printf("%02x ", arr[i]);
        if ((i + 1) % 12 == 0) printf("\n");
    }
    printf("\r\n");
	
	return 0;
}

//Normal255�??
int gammaRegWrite_Normal(int j, uint16_t *data)
{
	int i;
	u8 d;
	const u8 reg_adrr[] = NORMAL_DEF_GAMMA_REG_ADRR;
	u8 dL[3],dH[3];
	// uint8_t ReadCode[6];

	for(int i = 0; i < 3; i++)
    {
        dH[i] = ((data[i] >> 8) & 0x07) ;
        dL[i] = data[i] & 0xff;  
    }
	QSPI_LCD_WriteCmd(0xFF,0x5A,0x07);

	QSPI_LCD_WriteCmd(0xF2,0x01);
	QSPI_LCD_WriteCmd(0xB0,dH[0]);

	QSPI_LCD_WriteCmd(0xF2,0x02);
	QSPI_LCD_WriteCmd(0xB0,dL[0]);
		
	QSPI_LCD_WriteCmd(0xF2,0x03);
	QSPI_LCD_WriteCmd(0xB0,dH[1]);

	QSPI_LCD_WriteCmd(0xF2,0x04);
	QSPI_LCD_WriteCmd(0xB0,dL[1]);
		
	QSPI_LCD_WriteCmd(0xF2,0x05);
	QSPI_LCD_WriteCmd(0xB0,dH[2]);

	QSPI_LCD_WriteCmd(0xF2,0x06);
	QSPI_LCD_WriteCmd(0xB0,dL[2]);

	// QSPI_LCD_WriteCmd(0xF2,0x01);
	// QSPI_LCD_ReadData(0xB0,6,ReadCode);
	// for(i = 0;i<6;i++)
	// {
	// 	printf("0x%02X,",ReadCode[i]);
	// }
	// printf("\r\n");

	return 0;
}
int gammaRegReadAll_Normal(void)
{
	int i, res = 0;
	u8 d;
	const u8 reg_adrr[] = NORMAL_DEF_GAMMA_REG_ADRR;
	unsigned char ReadCode[256];
	
	QSPI_LCD_WriteCmd(0xF0,0x5A,0x5A);
	QSPI_LCD_WriteCmd(0xF1,0x5A,0x5A);
	QSPI_LCD_WriteCmd(0xFF,0x5A,0x07);
	
	QSPI_LCD_WriteCmd(0xF2,0x01);
	QSPI_LCD_ReadData(0xB0,162,ReadCode);

	for (i = 0; i < (sizeof(gamma_AllRead)/sizeof(gamma_AllRead[0]))/3; i++)
	{
		gamma_AllRead[i] = ReadCode[0x9C-6*i] << 8 | ReadCode[0x9D-6*i];
		gamma_AllRead[i+NORMAL_DEF_GAMMA_ORDER_NUM] = ReadCode[0x9E -6*i] << 8 | ReadCode[0x9F-6*i];
		gamma_AllRead[i+NORMAL_DEF_GAMMA_ORDER_NUM*2] = ReadCode[0xA0-6*i] << 8 | ReadCode[0xA1-6*i];
	}
	// for (i = 0; i < (sizeof(gamma_AllRead)/sizeof(gamma_AllRead[0])); i++)
	// {
	// 	printf("%d,",gamma_AllRead[i]);
	// 	if ((i+1)%27==0) printf("\r\n");	
	// }
	for (i = 0; i < sizeof(gamma_AllRead)/sizeof(gamma_AllRead[0]); i++)
	{
		if(gamma_AllRead[i] != GammaConfig_Normal.gammaSetReg[i])
		{
			res = -1;
			printf("%d->%d,",GammaConfig_Normal.gammaSetReg[i],gamma_AllRead[i]);
		}
		else
		{
			res = 1;
			printf("%d,",GammaConfig_Normal.gammaSetReg[i]);
		}
		if ((i+1)%27==0) printf("\r\n");	
	}
	return res;
}

//Idle（AOD）全�??
int gammaRegWriteAll_Idle(uint16_t *data)
{
	int i,j=0;
    uint8_t arr[162] = {0};
	u8 d;
	const u8 reg_adrr[] = NORMAL_DEF_GAMMA_REG_ADRR;
	
	QSPI_LCD_WriteCmd(0xFF,0x5A,0x07);
	
	for (i = 0; i < sizeof(reg_adrr)/2/3; i++)
	{
		d = data[NORMAL_DEF_GAMMA_ORDER_NUM - 1 -i] / 256;
		arr[reg_adrr[j]] = d;
		// printf("addr:0x%02X,num:0x%02X\r\n",reg_adrr[j],d);
		QSPI_LCD_WriteCmd(0xF2,reg_adrr[j]+1);
		QSPI_LCD_WriteCmd(0xB2,d);
		d = data[NORMAL_DEF_GAMMA_ORDER_NUM - 1 -i] % 256;
		arr[reg_adrr[j+1]] = d;
		// printf("addr:0x%02X,num:0x%02X\r\n",reg_adrr[j+1],d);
		QSPI_LCD_WriteCmd(0xF2,reg_adrr[j+1]+1);
		QSPI_LCD_WriteCmd(0xB2,d);
		d = data[NORMAL_DEF_GAMMA_ORDER_NUM * 2 - 1 -i] / 256;
		arr[reg_adrr[j+2]] = d;
		// printf("addr:0x%02X,num:0x%02X\r\n",reg_adrr[j+2],d);
		QSPI_LCD_WriteCmd(0xF2,reg_adrr[j+2]+1);
		QSPI_LCD_WriteCmd(0xB2,d);
		d = data[NORMAL_DEF_GAMMA_ORDER_NUM * 2 - 1 -i] % 256;
		arr[reg_adrr[j+3]] = d;
		// printf("addr:0x%02X,num:0x%02X\r\n",reg_adrr[j+3],d);
		QSPI_LCD_WriteCmd(0xF2,reg_adrr[j+3]+1);
		QSPI_LCD_WriteCmd(0xB2,d);
		d = data[NORMAL_DEF_GAMMA_ORDER_NUM * 3 - 1 -i] / 256;
		arr[reg_adrr[j+4]] = d;
		// printf("addr:0x%02X,num:0x%02X\r\n",reg_adrr[j+4],d);
		QSPI_LCD_WriteCmd(0xF2,reg_adrr[j+4]+1);
		QSPI_LCD_WriteCmd(0xB2,d);
		d = data[NORMAL_DEF_GAMMA_ORDER_NUM * 3 - 1 -i] % 256;
		arr[reg_adrr[j+5]] = d;
		// printf("addr:0x%02X,num:0x%02X\r\n",reg_adrr[j+5],d);
		QSPI_LCD_WriteCmd(0xF2,reg_adrr[j+5]+1);
		QSPI_LCD_WriteCmd(0xB2,d);
		j+=6;
	}
    printf("Data to write:\r\n");
    for (i = 0; i < 162; i++)
    {
        printf("%02x ", arr[i]);
        if ((i + 1) % 12 == 0) printf("\n");
    }
    printf("\r\n");
	
	return 0;
}

//Idle(AOD)255�??
int gammaRegWrite_Idle(int j, uint16_t *data)
{
	int i;
	u8 d;
	const u8 reg_adrr[] = NORMAL_DEF_GAMMA_REG_ADRR;
	u8 dL[3],dH[3];
	// uint8_t ReadCode[6];

	for(int i = 0; i < 3; i++)
    {
        dH[i] = ((data[i] >> 8) & 0x07) ;
        dL[i] = data[i] & 0xff;  
    }
	QSPI_LCD_WriteCmd(0xFF,0x5A,0x07);

	QSPI_LCD_WriteCmd(0xF2,0x01);
	QSPI_LCD_WriteCmd(0xB2,dH[0]);

	QSPI_LCD_WriteCmd(0xF2,0x02);
	QSPI_LCD_WriteCmd(0xB2,dL[0]);
		
	QSPI_LCD_WriteCmd(0xF2,0x03);
	QSPI_LCD_WriteCmd(0xB2,dH[1]);

	QSPI_LCD_WriteCmd(0xF2,0x04);
	QSPI_LCD_WriteCmd(0xB2,dL[1]);
		
	QSPI_LCD_WriteCmd(0xF2,0x05);
	QSPI_LCD_WriteCmd(0xB2,dH[2]);

	QSPI_LCD_WriteCmd(0xF2,0x06);
	QSPI_LCD_WriteCmd(0xB2,dL[2]);

	// QSPI_LCD_WriteCmd(0xF2,0x01);
	// QSPI_LCD_ReadData(0xB2,6,ReadCode);
	// for(i = 0;i<6;i++)
	// {
	// 	printf("0x%02X,",ReadCode[i]);
	// }
	// printf("\r\n");

	return 0;
}
int gammaRegReadAll_Idle(void)
{
	int i, res = 0;
	u8 d;
	const u8 reg_adrr[] = NORMAL_DEF_GAMMA_REG_ADRR;
	unsigned char ReadCode[256];

	QSPI_LCD_WriteCmd(0xF0,0x5A,0x5A);
	QSPI_LCD_WriteCmd(0xF1,0x5A,0x5A);
	QSPI_LCD_WriteCmd(0xFF,0x5A,0x07);
	
	QSPI_LCD_WriteCmd(0xF2,0x01);
	QSPI_LCD_ReadData(0xB2,162,ReadCode);

	for (i = 0; i < (sizeof(gamma_AllRead)/sizeof(gamma_AllRead[0]))/3; i++)
	{
		gamma_AllRead[i] = ReadCode[0x9C-6*i] << 8 | ReadCode[0x9D-6*i];
		gamma_AllRead[i+NORMAL_DEF_GAMMA_ORDER_NUM] = ReadCode[0x9E -6*i] << 8 | ReadCode[0x9F-6*i];
		gamma_AllRead[i+NORMAL_DEF_GAMMA_ORDER_NUM*2] = ReadCode[0xA0-6*i] << 8 | ReadCode[0xA1-6*i];
	}
	// for (i = 0; i < (sizeof(gamma_AllRead)/sizeof(gamma_AllRead[0])); i++)
	// {
	// 	printf("%d,",gamma_AllRead[i]);
	// 	if ((i+1)%27==0) printf("\r\n");	
	// }
	for (i = 0; i < sizeof(gamma_AllRead)/sizeof(gamma_AllRead[0]); i++)
	{
		if(gamma_AllRead[i] != GammaConfig_Idle.gammaSetReg[i])
		{
			res = -1;
			printf("%d->%d,",GammaConfig_Idle.gammaSetReg[i],gamma_AllRead[i]);
		}
		else
		{
			res = 1;
			printf("%d,",GammaConfig_Idle.gammaSetReg[i]);
		}
		if ((i+1)%27==0) printf("\r\n");	
	}
	
	return res;
}


//Normal3全写
int gammaRegWriteAll_Normal3(uint16_t *data)
{
  int i;
	u8 d;
	const u8 reg_adrr[] = NORMAL_DEF_GAMMA_REG_ADRR;
	
	MIPI_WR(0x15,0xFE,0x30);
	
	if (data == 0)
	{
		for (i = 0; i < sizeof(reg_adrr) / 2; i++)
		{
			MIPI_WR(0x15,reg_adrr[2*i],0);
			MIPI_WR(0x15,reg_adrr[2*i+1],0);
		}
	}else
	{
		for (i = 0; i < sizeof(reg_adrr) / 2; i++)
		{
			d = data[i] / 256;
			MIPI_WR(0x15,reg_adrr[2*i],d);
			d = data[i] % 256;
			MIPI_WR(0x15,reg_adrr[2*i+1],d);
		}
	}

	MIPI_WR(0x15,0xFE,0x00);
	return 0;
}

//Normal3 255�??
int gammaRegWrite_Normal3(int j, uint16_t *data)
{
	int i;
	u8 d;
	const u8 reg_adrr[] = NORMAL_DEF_GAMMA_REG_ADRR;
	
	MIPI_WR(0x15,0xFE,0x30);
	
	if (data == 0)
	{		
		for (i = 0; i < 3; i++)
		{
			MIPI_WR(0x15,reg_adrr[2*(i*NORMAL_DEF_GAMMA_ORDER_NUM + j)],0);
			MIPI_WR(0x15,reg_adrr[2*(i*NORMAL_DEF_GAMMA_ORDER_NUM + j)+1],0);
		}		
	}else
	{
		for (i = 0; i < 3; i++)
		{
			d = data[i] / 256;
			MIPI_WR(0x15,reg_adrr[2*(i*NORMAL_DEF_GAMMA_ORDER_NUM + j)], d);
			d = data[i] % 256;
			MIPI_WR(0x15,reg_adrr[2*(i*NORMAL_DEF_GAMMA_ORDER_NUM + j)+1], d);
		}	
	}
	
	MIPI_WR(0x15,0xFE,0x00);
	return 0;
}

//Normal2全写
int gammaRegWriteAll_Normal2(uint16_t *data)
{
   int i;
	u8 d;
	const u8 reg_adrr[] = NORMAL_DEF_GAMMA_REG_ADRR;
	
	MIPI_WR(0x15,0xFE,0x51);
	
	if (data == 0)
	{
		for (i = 0; i < sizeof(reg_adrr) / 2; i++)
		{
			MIPI_WR(0x15,reg_adrr[2*i],0);
			MIPI_WR(0x15,reg_adrr[2*i+1],0);
		}
	}else
	{
		for (i = 0; i < sizeof(reg_adrr) / 2; i++)
		{
			d = data[i] / 256;
			MIPI_WR(0x15,reg_adrr[2*i],d);
			d = data[i] % 256;
			MIPI_WR(0x15,reg_adrr[2*i+1],d);
		}
	}

	MIPI_WR(0x15,0xFE,0x00);
	return 0;
}

//Normal2 255�??
int gammaRegWrite_Normal2(int j, uint16_t *data)
{
	int i;
	u8 d;
	const u8 reg_adrr[] = NORMAL_DEF_GAMMA_REG_ADRR;
	
	MIPI_WR(0x15,0xFE,0x51);
	
	if (data == 0)
	{		
		for (i = 0; i < 3; i++)
		{
			MIPI_WR(0x15,reg_adrr[2*(i*NORMAL_DEF_GAMMA_ORDER_NUM + j)],0);
			MIPI_WR(0x15,reg_adrr[2*(i*NORMAL_DEF_GAMMA_ORDER_NUM + j)+1],0);
		}		
	}else
	{
		for (i = 0; i < 3; i++)
		{
			d = data[i] / 256;
			MIPI_WR(0x15,reg_adrr[2*(i*NORMAL_DEF_GAMMA_ORDER_NUM + j)], d);
			d = data[i] % 256;
			MIPI_WR(0x15,reg_adrr[2*(i*NORMAL_DEF_GAMMA_ORDER_NUM + j)+1], d);
		}	
	}
	
	MIPI_WR(0x15,0xFE,0x00);
	return 0;
}
/******************************************************************************************/


/**gamma调试各模式配置接�??******************************************************************/

//Idle模式调gamma配置
int gammaConfigInitIdle(void)
{
    //绑定定义的全局结构�?? GammaConfig_Idle
	NEW_GammaConfigStruct *gammaConfig_t = &GammaConfig_Idle;
	
    //详细配置new_gamma_def.h文件
	float X = IDLE_DEF_GAMMA_TARGET_X;              //目标色坐标X
	float Y = IDLE_DEF_GAMMA_TARGET_Y;              //目标色坐标Y
	float maxLv = IDLE_DEF_GAMMA_TARGET_MAX_LV;     //255阶目标亮度Lv
	
	float gamma = IDLE_DEF_GAMMA_TARGET_GAMMA;      //目标gamma(gamma决定每一阶的亮度)
	const uint16_t temReg[IDLE_DEF_GAMMA_ORDER_NUM * 3] = IDLE_DEF_GAMMA_RGB_REG;   //初值数据绑定到数组(在new_gamma_def.h配置)
	const uint8_t level[IDLE_DEF_GAMMA_ORDER_NUM] = IDLE_DEF_GAMMA_LEVEL;           //绑点数据绑定到数�??(在new_gamma_def.h配置)
	
    //将上面的配置数据绑定到Idle结构�??
    memcpy(gammaConfig_t->displaymode, "AOD", sizeof(gammaConfig_t->displaymode));  //绑定当前结构体ID名称，用于保存模型数据到内存卡时文件命名
	gammaConfig_t->maxlv=  maxLv;
	gammaConfig_t->gamma = gamma;
//	gammaConfig_t->targetX = X;
//	gammaConfig_t->targetY = Y;
	gammaConfig_t->Order = IDLE_DEF_GAMMA_ORDER_NUM;

	gammaConfig_t->RGB_MAX = 2047;
	gammaConfig_t->RGB_MIN = 0; //gamma调节中的RGB值的范围	
		
	memcpy(gammaConfig_t->gammaDefReg, temReg, gammaConfig_t->Order*3*2);
	gammaConfig_t->gammaRegWrite = gammaRegWrite_Idle;  //Idle(AOD)255写函数接�??
    gammaConfig_t->gammaRegWriteAll = gammaRegWriteAll_Idle;     //Idle(AOD)全写函数接口
	gammaConfig_t->nndate = (float *) IDLE_NNData;      //Idle模型库绑�??(def.h文件)
	
    #ifdef CL2
	gammaConfig_t->readLv = CL2_read_xylv; //CL2读取镜头�??
    #endif
    #ifdef CA310
	gammaConfig_t->readLv = readLv_CA310;//CA310读取镜头�??
    #endif
    #ifdef CA410
	gammaConfig_t->readLv = readLv_CA310;//CA310读取镜头�??
    #endif
		#ifdef CL5
	gammaConfig_t->readLv = readLv_CA310;//CA310读取镜头�??
    #endif
	
   //遍历所有绑点配置对应绑点的卡控
	for (int i = 0; i < gammaConfig_t->Order; i++)
	{

//		gammaConfig_t->OGammaConfig[i].tuningN = 100;//每阶运行调试最大次数（超过就NG�??
//		gammaConfig_t->OGammaConfig[i].level = level[i];//当前绑点对应的灰�??
//		gammaConfig_t->OGammaConfig[i].targetX = X;
//		gammaConfig_t->OGammaConfig[i].targetY = Y;
//		gammaConfig_t->OGammaConfig[i].jdxy = 0;
		gammaConfig_t->OGammaConfig[i].level = level[i];//当前绑点对应的灰�??		
		gammaConfig_t->OGammaConfig[i].tuningN = 100;//每阶运行调试最大次数（超过就NG�??
		
		gammaConfig_t->OGammaConfig[i].targetX = X;
		gammaConfig_t->OGammaConfig[i].targetY = Y;
		#ifdef CL2
		if (i == gammaConfig_t->Order - 1)
		{			
            //有带F结尾的浮点数类型修改�??? -> 一定要加F
			gammaConfig_t->OGammaConfig[i].dGamma = 0.0F;              //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 1.5F;	                //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.002F;            //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0.0;	                //DE卡控，一般小�???0.5
			gammaConfig_t->OGammaConfig[i].readD=2;                 //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=2;			        //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;                 //读镜头延�???
		}                                                       
		else if (i >= 22)  // Gray223 ~ 79                      
		{			                                            
			gammaConfig_t->OGammaConfig[i].dGamma = 0.03F;	         //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		             //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.002F;  	         //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0;                  //DE卡控，一般小�???0.5
			gammaConfig_t->OGammaConfig[i].readD=2;                  //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=2;			         //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
		}    
		else if (i >= 20)                                       
		{			                                            
			gammaConfig_t->OGammaConfig[i].dGamma = 0.03F;	         //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		             //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.002F;  	         //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0.0;                  //DE卡控，一般小�???0.5
			gammaConfig_t->OGammaConfig[i].readD=2;                  //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=2;			         //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
		}    		
		else if (i >= 18)                                       
		{			                                            
			gammaConfig_t->OGammaConfig[i].dGamma = 0.02F;	         //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		             //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.002F;  	         //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0.0;                  //DE卡控，一般小�???0.5
			gammaConfig_t->OGammaConfig[i].readD=2;                  //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=2;			         //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
		}                                                       
                                                                
		else if (i >= 17)                                       
		{			                                            
			gammaConfig_t->OGammaConfig[i].dGamma = 0.01F;	     //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		         //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.002F;          //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0.0;             //DE卡控，一般小�???0.5
			gammaConfig_t->OGammaConfig[i].readD=2;              //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=2;              //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			                                                    
		}else if (i >= 13) // Gray41 ~ 71                       
		{			                                            
			gammaConfig_t->OGammaConfig[i].dGamma = 0.01F;	     //gamma卡控 0.0035
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		         //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.002F;          //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0.0;	         //DE卡控，一般小�???0.5
			gammaConfig_t->OGammaConfig[i].readD=2;              //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=2;              //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			                                                    
		}else if (i >= 10)  // Gray31 ~ 39                      
		{			                                            
			gammaConfig_t->OGammaConfig[i].dGamma = 0.01F;	     //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		         //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.0025F;         //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0.0;	             //DE卡控，一般小�???0.5
			gammaConfig_t->OGammaConfig[i].readD=2;              //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=2;              //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			                                                    
		}else if (i >= 7)  // Gray19 ~ 27                       
		{			                                            
			gammaConfig_t->OGammaConfig[i].dGamma = 0.012F;	     //gamma卡控  0.001
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		         //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.003F;          //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0;	             //DE卡控，一般小�???0.5
			gammaConfig_t->OGammaConfig[i].readD=2;              //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=2;              //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			                                                    
		}else if (i >= 5)  // Gray15                    
		{			                                            
			gammaConfig_t->OGammaConfig[i].dGamma = 0.012F;	    //gamma卡控  0.1
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		        //亮度卡控 
			gammaConfig_t->OGammaConfig[i].dxy = 0.0035F;           //色坐标卡�???	 
			gammaConfig_t->OGammaConfig[i].De= 0;             //DE卡控，一般小�???0.5 
			gammaConfig_t->OGammaConfig[i].readD=2;             //此读镜头次数当作延时处理 
			gammaConfig_t->OGammaConfig[i].readN=2;             //此读镜头次数为真正取值读镜头次数 
            gammaConfig_t->OGammaConfig[i].delay=0;
			                                                    
		}                                                       
        else if (i >= 4)  // Gray11                              
		{			                                             
			gammaConfig_t->OGammaConfig[i].dGamma = 0.2F;	     //gamma卡控 0.15
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		         //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.01F;  	     //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0.0;		         //DE卡控，一般小�???0.5
			gammaConfig_t->OGammaConfig[i].readD=2;              //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=2;              //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			                                                     
		}else if (i >= 3)  // Gray7  0.078nit                    
		{			                                             
			gammaConfig_t->OGammaConfig[i].dGamma = 0.4F;	     //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		         //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.15F;            //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0;	         //DE卡控，一般小�???0.5
			gammaConfig_t->OGammaConfig[i].readD=2;              //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=2;              //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			                                                     
		}else if (i >= 2)  // Gray3                              
		{			                                             
			gammaConfig_t->OGammaConfig[i].dGamma = 0.5F;	     //gamma卡控 0.3
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		         //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.0F;  	         //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0;		         //DE卡控，一般小�???0.5
			gammaConfig_t->OGammaConfig[i].readD=4;              //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=3;              //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			                                                     
		}else if (i >= 1)  // Gray1                               
		{			                                  
			gammaConfig_t->OGammaConfig[i].dGamma = 0.0F;		       //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		           //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.0F;  	           //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0;				   //DE卡控，一般小�???0.5
			gammaConfig_t->OGammaConfig[i].readD=3;                //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=3;                //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			                                                      
		}else  // Gray0 ~ 1                                      
		{			                                             
			gammaConfig_t->OGammaConfig[i].dGamma = 0.0F;		     //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		         //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.0F;  	         //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0;				 //DE卡控，一般小�???0.5
		}
    #endif
		
		#ifdef CL5
		if (i == gammaConfig_t->Order - 1)
		{			
            //有带F结尾的浮点数类型修改�??? -> 一定要加F
			gammaConfig_t->OGammaConfig[i].dGamma = 0.0F;                //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.2F;	                  //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.0015F; 	          //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0.85F;	 	                  //DE卡控，一般小�???0.5
			gammaConfig_t->OGammaConfig[i].readD=2;                   //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=3;                   //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;                   //读镜头延�???    
			                                                      
		}else if (i >= 20)  // Gray223 ~ 79                       
		{			                                              
			gammaConfig_t->OGammaConfig[i].dGamma = 0.015F;		       //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		               //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.002F;  	           //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0.0F;	 	               //DE卡控，一般小�???0.5
			gammaConfig_t->OGammaConfig[i].readD=2;                    //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=3;                    //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			                                                      
		}else if (i >= 19)  // Gray223 ~ 79                       
		{			                                              
			gammaConfig_t->OGammaConfig[i].dGamma = 0.01F;	       //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;			           //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.002F;  	           //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0.0F;	 	               //DE卡控，一般小�???0.5
			gammaConfig_t->OGammaConfig[i].readD=2;                    //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=3;                    //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			                                                      
		}                                                         
		else if (i >= 13) // Gray41 ~ 71                          
		{			                                              
			gammaConfig_t->OGammaConfig[i].dGamma = 0.01F;		   //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		           //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.002F;  	       //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0.0F;	 	               //DE卡控，一般小�???0.5
			gammaConfig_t->OGammaConfig[i].readD=2;                //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=3;                //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			                                                      
		}else if (i >= 10)  // Gray31 ~ 39    10 是Gray31           
		{			                                              
			gammaConfig_t->OGammaConfig[i].dGamma = 0.01F;	       //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		           //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.0025F;  	       //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0.0F;	 		           //DE卡控，一般小�???0.5
			gammaConfig_t->OGammaConfig[i].readD=2;                //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=3;                //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			                                                      
		}else if (i >= 7)  // Gray19 ~ 27                         
		{			                                              
			gammaConfig_t->OGammaConfig[i].dGamma = 0.012F;	       //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		           //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.003F;  	       //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0.0F;	               //DE卡控，一般小�???0.5
			gammaConfig_t->OGammaConfig[i].readD=2;                //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=3;			       //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
		}else if (i >= 6)  // Gray15                              
		{			                                              
			gammaConfig_t->OGammaConfig[i].dGamma = 0.02F;	       //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		           //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.004F;  	       //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0.0F;	               //DE卡控，一般小�???0.5
			gammaConfig_t->OGammaConfig[i].readD=3;                //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=3;			       //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
		}else if (i >= 5)  // Gray11                              
		{			                                              
			gammaConfig_t->OGammaConfig[i].dGamma = 0.012F;	       //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		           //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.0035F;  	       //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0.0F;                //DE卡控，一般小�???0.5
			gammaConfig_t->OGammaConfig[i].readD=3;                //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=3;                //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			                                                      
		}else if (i >= 4)  // Gray7                               
		{			                                              
			gammaConfig_t->OGammaConfig[i].dGamma = 0.015F;	       //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		           //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.0045F;  	       //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0.0F;	               //DE卡控，一般小�???0.5
			gammaConfig_t->OGammaConfig[i].readD=3;                //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=3;			       //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
		}else if (i >= 3)  // Gray5                               
		{			                                              
			gammaConfig_t->OGammaConfig[i].dGamma = 0.15F;	       //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		           //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.01F;  	       //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0;	               //DE卡控，一般小�???0.5
			gammaConfig_t->OGammaConfig[i].readD=3;                //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=3;                //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			                                                      
		}else if (i >= 2)  // Gray3                               
		{			                                              
			gammaConfig_t->OGammaConfig[i].dGamma = 0.17F;	       //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		           //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.0F;  	       //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0;	               //DE卡控，一般小�???0.5
			gammaConfig_t->OGammaConfig[i].readD=3;                //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=3;                //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			                                                      
		}else  // Gray0 ~ 3                                       
		{			                                              
			gammaConfig_t->OGammaConfig[i].dGamma = 0.0F;		       //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		           //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.0F;  	           //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0.0F;				   //DE卡控，一般小�???0.5
		}
    #endif
		
		//getGamma2Err                                          
		
      //获取亮度范围
      gammaConfig_t->OGammaConfig[i].targetLv =  maxLv * pow((level[i])/255.0F,gamma);
      if(gammaConfig_t->OGammaConfig[i].dGamma > 1e-6)
      {			
        gammaConfig_t->OGammaConfig[i].targetlvMax = maxLv * pow((level[i])/255.0F,gamma - gammaConfig_t->OGammaConfig[i].dGamma);
        gammaConfig_t->OGammaConfig[i].targetlvMin = maxLv * pow((level[i])/255.0F,gamma + gammaConfig_t->OGammaConfig[i].dGamma);
      }
      else
      {
        gammaConfig_t->OGammaConfig[i].targetlvMax = gammaConfig_t->OGammaConfig[i].targetLv + gammaConfig_t->OGammaConfig[i].dLv;
        gammaConfig_t->OGammaConfig[i].targetlvMin = gammaConfig_t->OGammaConfig[i].targetLv - gammaConfig_t->OGammaConfig[i].dLv;
      }

	}

	return 0;
}


int gammaConfigInitHBM(void)
{
    //绑定定义的全局结构�?? GammaConfig_HBM
	NEW_GammaConfigStruct *gammaConfig_t = &GammaConfig_HBM;
	
    //详细配置new_gamma_def.h文件
	float X = HBM_DEF_GAMMA_TARGET_X;           //目标色坐标X
	float Y = HBM_DEF_GAMMA_TARGET_Y;           //目标色坐标Y
	float maxLv = HBM_DEF_GAMMA_TARGET_MAX_LV;  //255阶目标亮度Lv
	                                            
	float gamma = HBM_DEF_GAMMA_TARGET_GAMMA;   //目标gamma(gamma决定每一阶的亮度)
	const uint16_t temReg[HBM_DEF_GAMMA_ORDER_NUM * 3] = HBM_DEF_GAMMA_RGB_REG;     //初值数据绑定到数组(在new_gamma_def.h配置)
	const uint8_t level[HBM_DEF_GAMMA_ORDER_NUM] = HBM_DEF_GAMMA_LEVEL;             //绑点数据绑定到数�??(在new_gamma_def.h配置)
	
    //将上面的配置数据绑定到Idle结构�??
   memcpy(gammaConfig_t->displaymode, "HBM", sizeof(gammaConfig_t->displaymode));  //绑定当前结构体ID名称，用于保存模型数据到内存卡时文件命名
	gammaConfig_t->maxlv=  maxLv;
	gammaConfig_t->gamma = gamma;
//	gammaConfig_t->targetX = X;
//	gammaConfig_t->targetY = Y;
	gammaConfig_t->Order = HBM_DEF_GAMMA_ORDER_NUM;

	gammaConfig_t->RGB_MAX = 2047;      //gamma调节中的RGB值的最大范�??
	gammaConfig_t->RGB_MIN = 0; 	    //gamma调节中的RGB值的最小范�??	
		
	memcpy(gammaConfig_t->gammaDefReg, temReg, gammaConfig_t->Order*3*2);
	gammaConfig_t->gammaRegWrite = gammaRegWrite_HBM;       //255写函数接�??
  gammaConfig_t->gammaRegWriteAll = gammaRegWriteAll_HBM;          //全写函数接口
	gammaConfig_t->nndate = (float *) HBM_NNData;           //模型库绑�??(def.h文件)
	
    #ifdef CL2
	gammaConfig_t->readLv = CL2_read_xylv; //CL2读取镜头�??
    #endif                                 
    #ifdef CA310                           
	gammaConfig_t->readLv = readLv_CA310;  //CA310读取镜头�??
    #endif
    #ifdef CA410
	gammaConfig_t->readLv = readLv_CA310;//CA310读取镜头�??
    #endif
		#ifdef CL5
	gammaConfig_t->readLv = readLv_CA310;//CA310读取镜头�??
    #endif

  //遍历所有绑点配置对应绑点的卡控
	for (int i = 0; i < gammaConfig_t->Order; i++)
	{
		gammaConfig_t->OGammaConfig[i].level = level[i];		
		gammaConfig_t->OGammaConfig[i].tuningN = 100;//每阶运行次数
	gammaConfig_t->OGammaConfig[i].targetX = X;
	gammaConfig_t->OGammaConfig[i].targetY = Y;

		#ifdef CL2
		if (i == gammaConfig_t->Order - 1)
		{			
            //有带F结尾的浮点数类型修改�??? -> 一定要加F
			gammaConfig_t->OGammaConfig[i].dGamma = 0.0F;                //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 4.0F;	                  //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.0015F; 	          //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0.85F;	 	                  //DE卡控，一般小�???0.5
			gammaConfig_t->OGammaConfig[i].readD=2;                   //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=3;                   //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;                   //读镜头延�???    
			                                                      
		}else if (i >= 20)  // Gray223 ~ 79                       
		{			                                              
			gammaConfig_t->OGammaConfig[i].dGamma = 0.02F;		       //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		               //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.0025F;  	           //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0.0F;	 	               //DE卡控，一般小�???0.5
			gammaConfig_t->OGammaConfig[i].readD=2;                    //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=3;                    //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			                                                      
		}else if (i >= 19)  // Gray223 ~ 79                       
		{			                                              
			gammaConfig_t->OGammaConfig[i].dGamma = 0.01F;		       //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;			           //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.0025F;  	           //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0.0F;	 	               //DE卡控，一般小�???0.5
			gammaConfig_t->OGammaConfig[i].readD=2;                    //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=3;                    //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			                                                      
		}                                                         
		else if (i >= 13) // Gray41 ~ 71                          
		{			                                              
			gammaConfig_t->OGammaConfig[i].dGamma = 0.01F;		   //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		           //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.003F;  	       //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0.0F;	 	               //DE卡控，一般小�???0.5
			gammaConfig_t->OGammaConfig[i].readD=2;                //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=3;                //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			                                                      
		}else if (i >= 10)  // Gray31 ~ 39    10 是Gray31           
		{			                                              
			gammaConfig_t->OGammaConfig[i].dGamma = 0.01F;	       //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		           //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.003F;  	       //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0.0F;	 		           //DE卡控，一般小�???0.5
			gammaConfig_t->OGammaConfig[i].readD=2;                //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=3;                //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			                                                      
		}else if (i >= 7)  // Gray19 ~ 27                         
		{			                                              
			gammaConfig_t->OGammaConfig[i].dGamma = 0.01F;	       //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		           //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.003F;  	       //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0.0F;	               //DE卡控，一般小�???0.5
			gammaConfig_t->OGammaConfig[i].readD=2;                //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=3;			       //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
		}else if (i >= 6)  // Gray15                              
		{			                                              
			gammaConfig_t->OGammaConfig[i].dGamma = 0.02F;	       //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		           //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.004F;  	       //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0.0F;	               //DE卡控，一般小�???0.5
			gammaConfig_t->OGammaConfig[i].readD=3;                //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=3;			       //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
		}else if (i >= 5)  // Gray11                              
		{			                                              
			gammaConfig_t->OGammaConfig[i].dGamma = 0.03F;	       //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		           //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.006F;  	       //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0.0F;                //DE卡控，一般小�???0.5
			gammaConfig_t->OGammaConfig[i].readD=3;                //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=3;                //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			                                                      
		}else if (i >= 4)  // Gray7                               
		{			                                              
			gammaConfig_t->OGammaConfig[i].dGamma = 0.04F;	       //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		           //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.008F;  	       //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0.0F;	               //DE卡控，一般小�???0.5
			gammaConfig_t->OGammaConfig[i].readD=3;                //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=3;			       //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
		}else if (i >= 3)  // Gray5                               
		{			                                              
			gammaConfig_t->OGammaConfig[i].dGamma = 0.15F;	       //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		           //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.008F;  	       //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0;	               //DE卡控，一般小�???0.5
			gammaConfig_t->OGammaConfig[i].readD=3;                //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=3;                //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			                                                      
		}else if (i >= 2)  // Gray3                               
		{			                                              
			gammaConfig_t->OGammaConfig[i].dGamma = 0.2F;	       //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		           //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.04F;  	       //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0;	               //DE卡控，一般小�???0.5
			gammaConfig_t->OGammaConfig[i].readD=3;                //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=3;                //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			                                                      
		}else  // Gray0 ~ 3                                       
		{			                                              
			gammaConfig_t->OGammaConfig[i].dGamma = 0.0F;		       //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		           //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.0F;  	           //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0.0F;				   //DE卡控，一般小�???0.5
		}
		#endif
		
		#ifdef CL5
		if (i == gammaConfig_t->Order - 1)
		{			
            //有带F结尾的浮点数类型修改�??? -> 一定要加F
			gammaConfig_t->OGammaConfig[i].dGamma = 0.0F;                //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 4.0F;	                  //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.0015F; 	          //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0.85F;	 	                  //DE卡控，一般小�???0.5
			gammaConfig_t->OGammaConfig[i].readD=2;                   //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=3;                   //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;                   //读镜头延�???    
			                                                      
		}else if (i >= 20)  // Gray223 ~ 79                       
		{			                                              
			gammaConfig_t->OGammaConfig[i].dGamma = 0.02F;		       //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		               //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.002F;  	           //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0.0F;	 	               //DE卡控，一般小�???0.5
			gammaConfig_t->OGammaConfig[i].readD=2;                    //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=3;                    //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			                                                      
		}else if (i >= 19)  // Gray223 ~ 79                       
		{			                                              
			gammaConfig_t->OGammaConfig[i].dGamma = 0.01F;		       //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;			           //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.002F;  	           //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0.0F;	 	               //DE卡控，一般小�???0.5
			gammaConfig_t->OGammaConfig[i].readD=2;                    //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=3;                    //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			                                                      
		}                                                         
		else if (i >= 13) // Gray41 ~ 71                          
		{			                                              
			gammaConfig_t->OGammaConfig[i].dGamma = 0.01F;		   //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		           //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.002F;  	       //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0.0F;	 	               //DE卡控，一般小�???0.5
			gammaConfig_t->OGammaConfig[i].readD=2;                //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=3;                //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			                                                      
		}else if (i >= 10)  // Gray31 ~ 39    10 是Gray31           
		{			                                              
			gammaConfig_t->OGammaConfig[i].dGamma = 0.01F;	       //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		           //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.0025F;  	       //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0.0F;	 		           //DE卡控，一般小�???0.5
			gammaConfig_t->OGammaConfig[i].readD=2;                //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=3;                //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			                                                      
		}else if (i >= 7)  // Gray19 ~ 27                         
		{			                                              
			gammaConfig_t->OGammaConfig[i].dGamma = 0.012F;	       //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		           //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.003F;  	       //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0.0F;	               //DE卡控，一般小�???0.5
			gammaConfig_t->OGammaConfig[i].readD=2;                //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=3;			       //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
		}else if (i >= 6)  // Gray15                              
		{			                                              
			gammaConfig_t->OGammaConfig[i].dGamma = 0.02F;	       //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		           //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.004F;  	       //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0.0F;	               //DE卡控，一般小�???0.5
			gammaConfig_t->OGammaConfig[i].readD=3;                //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=3;			       //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
		}else if (i >= 5)  // Gray11                              
		{			                                              
			gammaConfig_t->OGammaConfig[i].dGamma = 0.012F;	       //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		           //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.0035F;  	       //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0.0F;                //DE卡控，一般小�???0.5
			gammaConfig_t->OGammaConfig[i].readD=3;                //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=3;                //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			                                                      
		}else if (i >= 4)  // Gray7                               
		{			                                              
			gammaConfig_t->OGammaConfig[i].dGamma = 0.015F;	       //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		           //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.0045F;  	       //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0.0F;	               //DE卡控，一般小�???0.5
			gammaConfig_t->OGammaConfig[i].readD=3;                //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=3;			       //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
		}else if (i >= 3)  // Gray5                               
		{			                                              
			gammaConfig_t->OGammaConfig[i].dGamma = 0.15F;	       //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		           //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.008F;  	       //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0;	               //DE卡控，一般小�???0.5
			gammaConfig_t->OGammaConfig[i].readD=3;                //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=3;                //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			                                                      
		}else if (i >= 2)  // Gray3                               
		{			                                              
			gammaConfig_t->OGammaConfig[i].dGamma = 0.04F;	       //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		           //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.01F;  	       //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0;	               //DE卡控，一般小�???0.5
			gammaConfig_t->OGammaConfig[i].readD=3;                //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=3;                //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			                                                      
		}else  // Gray0 ~ 3                                       
		{			                                              
			gammaConfig_t->OGammaConfig[i].dGamma = 0.0F;		       //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		           //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.0F;  	           //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0.0F;				   //DE卡控，一般小�???0.5
		}
		#endif
		
		//getGamma2Err                                         

		//获取亮度范围
		gammaConfig_t->OGammaConfig[i].targetLv =  maxLv * pow((level[i])/255.0F,gamma);
		if(gammaConfig_t->OGammaConfig[i].dGamma > 1e-6)
		{			
			gammaConfig_t->OGammaConfig[i].targetlvMax = maxLv * pow((level[i])/255.0F,gamma - gammaConfig_t->OGammaConfig[i].dGamma);
			gammaConfig_t->OGammaConfig[i].targetlvMin = maxLv * pow((level[i])/255.0F,gamma + gammaConfig_t->OGammaConfig[i].dGamma);
		}
		else
		{
			gammaConfig_t->OGammaConfig[i].targetlvMax = gammaConfig_t->OGammaConfig[i].targetLv + gammaConfig_t->OGammaConfig[i].dLv;
			gammaConfig_t->OGammaConfig[i].targetlvMin = gammaConfig_t->OGammaConfig[i].targetLv - gammaConfig_t->OGammaConfig[i].dLv;
		}
	}

	return 0;
}


int gammaConfigInitNormal(void)
{
    //绑定定义的全局结构�?? GammaConfig_Idle
	NEW_GammaConfigStruct *gammaConfig_t = &GammaConfig_Normal;
	
    //详细配置new_gamma_def.h文件
	float X = NORMAL_DEF_GAMMA_TARGET_X;            //目标色坐标X
	float Y = NORMAL_DEF_GAMMA_TARGET_Y;            //目标色坐标Y
	float maxLv = NORMAL_DEF_GAMMA_TARGET_MAX_LV;   //255阶目标亮度Lv
	                                                
	float gamma = NORMAL_DEF_GAMMA_TARGET_GAMMA;    //目标gamma(gamma决定每一阶的亮度)
	const uint16_t temReg[NORMAL_DEF_GAMMA_ORDER_NUM * 3] = NORMAL_DEF_GAMMA_RGB_REG;        //初值数据绑定到数组(在new_gamma_def.h配置)   
	const uint8_t level[NORMAL_DEF_GAMMA_ORDER_NUM] = NORMAL_DEF_GAMMA_LEVEL;                //绑点数据绑定到数�??(在new_gamma_def.h配置)
	                                                                                         
    //将上面的配置数据绑定到Idle结构�??                                                                                         
    memcpy(gammaConfig_t->displaymode, "Normal_Band1", sizeof(gammaConfig_t->displaymode));  //绑定当前结构体ID名称，用于保存模型数据到内存卡时文件命名
	gammaConfig_t->maxlv=  maxLv;
	gammaConfig_t->gamma = gamma;
//	gammaConfig_t->targetX = X;
//	gammaConfig_t->targetY = Y;
	gammaConfig_t->Order = NORMAL_DEF_GAMMA_ORDER_NUM;

	gammaConfig_t->RGB_MAX = 2047;      //gamma调节中的RGB值的最大范�??
	gammaConfig_t->RGB_MIN = 0;         //gamma调节中的RGB值的最小范�??
		
	memcpy(gammaConfig_t->gammaDefReg, temReg, gammaConfig_t->Order*3*2);
	gammaConfig_t->gammaRegWrite = gammaRegWrite_Normal;    //255写函数接�??
    gammaConfig_t->gammaRegWriteAll = gammaRegWriteAll_Normal;       //全写函数接口
	gammaConfig_t->nndate = (float *) NORMAL_NNData;	    //模型库绑�??(def.h文件)
    
    #ifdef CL2
	gammaConfig_t->readLv = CL2_read_xylv; //CL2读取镜头�??
    #endif                                 
    #ifdef CA310                           
	gammaConfig_t->readLv = readLv_CA310;  //CA310读取镜头�??
    #endif
    #ifdef CA410
	gammaConfig_t->readLv = readLv_CA310;//CA310读取镜头�??
    #endif
		#ifdef CL5
	gammaConfig_t->readLv = readLv_CA310;//CA310读取镜头�??
    #endif
	
    
  //遍历所有绑点配置对应绑点的卡控
	for (int i = 0; i < gammaConfig_t->Order; i++)
	{
		gammaConfig_t->OGammaConfig[i].level = level[i];		
		gammaConfig_t->OGammaConfig[i].tuningN = 100;//每阶运行次数
	gammaConfig_t->OGammaConfig[i].targetX = X;
	gammaConfig_t->OGammaConfig[i].targetY = Y;
		
		#ifdef CL2
		if (i == gammaConfig_t->Order - 1)
		{			
            //有带F结尾的浮点数类型修改�??? -> 一定要加F
			gammaConfig_t->OGammaConfig[i].dGamma = 0.0F;              //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 1.5F;	                //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.002F;            //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0.0;	                //DE卡控，一般小�???0.5
			gammaConfig_t->OGammaConfig[i].readD=2;                 //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=2;			        //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;                 //读镜头延�???
		}                                                       
		else if (i >= 22)  // Gray223 ~ 79                      
		{			                                            
			gammaConfig_t->OGammaConfig[i].dGamma = 0.03F;	         //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		             //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.002F;  	         //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0;                  //DE卡控，一般小�???0.5
			gammaConfig_t->OGammaConfig[i].readD=2;                  //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=2;			         //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
		}    
		else if (i >= 20)                                       
		{			                                            
			gammaConfig_t->OGammaConfig[i].dGamma = 0.03F;	         //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		             //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.002F;  	         //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0.0;                  //DE卡控，一般小�???0.5
			gammaConfig_t->OGammaConfig[i].readD=2;                  //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=2;			         //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
		}    		
		else if (i >= 18)                                       
		{			                                            
			gammaConfig_t->OGammaConfig[i].dGamma = 0.02F;	         //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		             //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.002F;  	         //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0.0;                  //DE卡控，一般小�???0.5
			gammaConfig_t->OGammaConfig[i].readD=2;                  //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=2;			         //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
		}                                                       
                                                                
		else if (i >= 17)                                       
		{			                                            
			gammaConfig_t->OGammaConfig[i].dGamma = 0.01F;	     //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		         //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.002F;          //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0.0;             //DE卡控，一般小�???0.5
			gammaConfig_t->OGammaConfig[i].readD=2;              //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=2;              //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			                                                    
		}else if (i >= 13) // Gray41 ~ 71                       
		{			                                            
			gammaConfig_t->OGammaConfig[i].dGamma = 0.01F;	     //gamma卡控 0.0035
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		         //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.002F;          //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0.0;	         //DE卡控，一般小�???0.5
			gammaConfig_t->OGammaConfig[i].readD=2;              //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=2;              //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			                                                    
		}else if (i >= 10)  // Gray31 ~ 39                      
		{			                                            
			gammaConfig_t->OGammaConfig[i].dGamma = 0.01F;	     //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		         //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.0025F;         //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0.0;	             //DE卡控，一般小�???0.5
			gammaConfig_t->OGammaConfig[i].readD=2;              //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=2;              //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			                                                    
		}else if (i >= 7)  // Gray19 ~ 27                       
		{			                                            
			gammaConfig_t->OGammaConfig[i].dGamma = 0.012F;	     //gamma卡控  0.001
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		         //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.003F;          //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0;	             //DE卡控，一般小�???0.5
			gammaConfig_t->OGammaConfig[i].readD=2;              //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=2;              //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			                                                    
		}else if (i >= 5)  // Gray15                    
		{			                                            
			gammaConfig_t->OGammaConfig[i].dGamma = 0.012F;	    //gamma卡控  0.1
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		        //亮度卡控 
			gammaConfig_t->OGammaConfig[i].dxy = 0.0035F;           //色坐标卡�???	 
			gammaConfig_t->OGammaConfig[i].De= 0;             //DE卡控，一般小�???0.5 
			gammaConfig_t->OGammaConfig[i].readD=2;             //此读镜头次数当作延时处理 
			gammaConfig_t->OGammaConfig[i].readN=2;             //此读镜头次数为真正取值读镜头次数 
            gammaConfig_t->OGammaConfig[i].delay=0;
			                                                    
		}                                                       
        else if (i >= 4)  // Gray11                              
		{			                                             
			gammaConfig_t->OGammaConfig[i].dGamma = 0.015F;	     //gamma卡控 0.15
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		         //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.0045F;  	     //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0.0;		         //DE卡控，一般小�???0.5
			gammaConfig_t->OGammaConfig[i].readD=2;              //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=2;              //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			                                                     
		}else if (i >= 3)  // Gray7  0.078nit                    
		{			                                             
			gammaConfig_t->OGammaConfig[i].dGamma = 0.015F;	     //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		         //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.008F;            //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0;	         //DE卡控，一般小�???0.5
			gammaConfig_t->OGammaConfig[i].readD=2;              //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=2;              //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			                                                     
		}else if (i >= 2)  // Gray3                              
		{			                                             
			gammaConfig_t->OGammaConfig[i].dGamma = 0.2F;	     //gamma卡控 0.3
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		         //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.1F;  	         //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0;		         //DE卡控，一般小�???0.5
			gammaConfig_t->OGammaConfig[i].readD=4;              //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=3;              //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			                                                     
		}else if (i >= 1)  // Gray1                               
		{			                                  
			gammaConfig_t->OGammaConfig[i].dGamma = 0.0F;		       //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		           //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.0F;  	           //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0;				   //DE卡控，一般小�???0.5
			gammaConfig_t->OGammaConfig[i].readD=3;                //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=3;                //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			                                                      
		}else  // Gray0 ~ 1                                      
		{			                                             
			gammaConfig_t->OGammaConfig[i].dGamma = 0.0F;		     //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		         //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.0F;  	         //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0;				 //DE卡控，一般小�???0.5
		}
		#endif
		
		#ifdef CL5
		if (i == gammaConfig_t->Order - 1)
		{			
            //有带F结尾的浮点数类型修改�??? -> 一定要加F
			gammaConfig_t->OGammaConfig[i].dGamma = 0.0F;              //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 1.5F;	                //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.002F;            //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0.0;	                //DE卡控，一般小�???0.5
			gammaConfig_t->OGammaConfig[i].readD=2;                 //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=2;			        //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;                 //读镜头延�???
		}                                                       
		else if (i >= 22)  // Gray223 ~ 79                      
		{			                                            
			gammaConfig_t->OGammaConfig[i].dGamma = 0.03F;	         //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		             //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.002F;  	         //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0;                  //DE卡控，一般小�???0.5
			gammaConfig_t->OGammaConfig[i].readD=2;                  //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=2;			         //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
		}    
		else if (i >= 20)                                       
		{			                                            
			gammaConfig_t->OGammaConfig[i].dGamma = 0.03F;	         //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		             //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.002F;  	         //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0.0;                  //DE卡控，一般小�???0.5
			gammaConfig_t->OGammaConfig[i].readD=2;                  //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=2;			         //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
		}    		
		else if (i >= 18)                                       
		{			                                            
			gammaConfig_t->OGammaConfig[i].dGamma = 0.02F;	         //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		             //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.002F;  	         //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0.0;                  //DE卡控，一般小�???0.5
			gammaConfig_t->OGammaConfig[i].readD=2;                  //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=2;			         //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
		}                                                       
                                                                
		else if (i >= 17)                                       
		{			                                            
			gammaConfig_t->OGammaConfig[i].dGamma = 0.01F;	     //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		         //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.002F;          //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0.0;             //DE卡控，一般小�???0.5
			gammaConfig_t->OGammaConfig[i].readD=2;              //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=2;              //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			                                                    
		}else if (i >= 13) // Gray41 ~ 71                       
		{			                                            
			gammaConfig_t->OGammaConfig[i].dGamma = 0.01F;	     //gamma卡控 0.0035
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		         //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.002F;          //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0.0;	         //DE卡控，一般小�???0.5
			gammaConfig_t->OGammaConfig[i].readD=2;              //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=2;              //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			                                                    
		}else if (i >= 10)  // Gray31 ~ 39                      
		{			                                            
			gammaConfig_t->OGammaConfig[i].dGamma = 0.01F;	     //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		         //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.0025F;         //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0.0;	             //DE卡控，一般小�???0.5
			gammaConfig_t->OGammaConfig[i].readD=2;              //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=2;              //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			                                                    
		}else if (i >= 7)  // Gray19 ~ 27                       
		{			                                            
			gammaConfig_t->OGammaConfig[i].dGamma = 0.012F;	     //gamma卡控  0.001
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		         //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.003F;          //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0;	             //DE卡控，一般小�???0.5
			gammaConfig_t->OGammaConfig[i].readD=2;              //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=2;              //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			                                                    
		}else if (i >= 5)  // Gray15                    
		{			                                            
			gammaConfig_t->OGammaConfig[i].dGamma = 0.012F;	    //gamma卡控  0.1
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		        //亮度卡控 
			gammaConfig_t->OGammaConfig[i].dxy = 0.0035F;           //色坐标卡�???	 
			gammaConfig_t->OGammaConfig[i].De= 0;             //DE卡控，一般小�???0.5 
			gammaConfig_t->OGammaConfig[i].readD=2;             //此读镜头次数当作延时处理 
			gammaConfig_t->OGammaConfig[i].readN=2;             //此读镜头次数为真正取值读镜头次数 
            gammaConfig_t->OGammaConfig[i].delay=0;
			                                                    
		}                                                       
        else if (i >= 4)  // Gray11                              
		{			                                             
			gammaConfig_t->OGammaConfig[i].dGamma = 0.015F;	     //gamma卡控 0.15
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		         //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.0045F;  	     //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0.0;		         //DE卡控，一般小�???0.5
			gammaConfig_t->OGammaConfig[i].readD=2;              //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=2;              //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			                                                     
		}else if (i >= 3)  // Gray7  0.078nit                    
		{			                                             
			gammaConfig_t->OGammaConfig[i].dGamma = 0.015F;	     //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		         //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.008F;            //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0;	         //DE卡控，一般小�???0.5
			gammaConfig_t->OGammaConfig[i].readD=2;              //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=2;              //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			                                                     
		}else if (i >= 2)  // Gray3                              
		{			                                             
			gammaConfig_t->OGammaConfig[i].dGamma = 0.06F;	     //gamma卡控 0.3
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		         //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.01F;  	         //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0;		         //DE卡控，一般小�???0.5
			gammaConfig_t->OGammaConfig[i].readD=4;              //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=3;              //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			                                                     
		}else if (i >= 1)  // Gray1                               
		{			                                  
			gammaConfig_t->OGammaConfig[i].dGamma = 0.0F;		       //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		           //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.0F;  	           //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0;				   //DE卡控，一般小�???0.5
			gammaConfig_t->OGammaConfig[i].readD=3;                //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=3;                //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			                                                      
		}else  // Gray0 ~ 1                                      
		{			                                             
			gammaConfig_t->OGammaConfig[i].dGamma = 0.0F;		     //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		         //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.0F;  	         //色坐标卡�???	
			gammaConfig_t->OGammaConfig[i].De= 0;				 //DE卡控，一般小�???0.5
		}
		#endif
				
		//getGamma2Err
		
      //获取亮度范围
      gammaConfig_t->OGammaConfig[i].targetLv =  maxLv * pow((level[i])/255.0F,gamma);
      if(gammaConfig_t->OGammaConfig[i].dGamma > 1e-6)
      {			
        gammaConfig_t->OGammaConfig[i].targetlvMax = maxLv * pow((level[i])/255.0F,gamma - gammaConfig_t->OGammaConfig[i].dGamma);
        gammaConfig_t->OGammaConfig[i].targetlvMin = maxLv * pow((level[i])/255.0F,gamma + gammaConfig_t->OGammaConfig[i].dGamma);
      }
      else
      {
        gammaConfig_t->OGammaConfig[i].targetlvMax = gammaConfig_t->OGammaConfig[i].targetLv + gammaConfig_t->OGammaConfig[i].dLv;
        gammaConfig_t->OGammaConfig[i].targetlvMin = gammaConfig_t->OGammaConfig[i].targetLv - gammaConfig_t->OGammaConfig[i].dLv;
      }

	}
	
	return 0;
}


int gammaConfigInitNormal3(void)
{
    //绑定定义的全局结构�?? GammaConfig_Idle
	NEW_GammaConfigStruct *gammaConfig_t = &GammaConfig_Normal3;
	
	float X = NOR3_DEF_GAMMA_TARGET_X;              //目标色坐标X
	float Y = NOR3_DEF_GAMMA_TARGET_Y;              //目标色坐标Y
	float maxLv = NOR3_DEF_GAMMA_TARGET_MAX_LV;     //255阶目标亮度Lv
	                                                
	float gamma = NOR3_DEF_GAMMA_TARGET_GAMMA;      //目标gamma(gamma决定每一阶的亮度)
	const uint16_t temReg[NOR3_DEF_GAMMA_ORDER_NUM * 3] = NOR3_DEF_GAMMA_RGB_REG;           //初值数据绑定到数组(在new_gamma_def.h配置)
	const uint8_t level[NOR3_DEF_GAMMA_ORDER_NUM] = NOR3_DEF_GAMMA_LEVEL;                   //绑点数据绑定到数�??(在new_gamma_def.h配置)
	                                                                                        
    //将上面的配置数据绑定到Idle结构�??                                                                                        
    memcpy(gammaConfig_t->displaymode, "Normal_Band3", sizeof(gammaConfig_t->displaymode)); //绑定当前结构体ID名称，用于保存模型数据到内存卡时文件命名
	gammaConfig_t->maxlv=  maxLv;
	gammaConfig_t->gamma = gamma;
//	gammaConfig_t->targetX = X;
//	gammaConfig_t->targetY = Y;
	gammaConfig_t->Order = NOR3_DEF_GAMMA_ORDER_NUM;

	gammaConfig_t->RGB_MAX = 1023;  //gamma调节中的RGB值的最大范�??
	gammaConfig_t->RGB_MIN = 0; 	//gamma调节中的RGB值的最小范�??
		
	memcpy(gammaConfig_t->gammaDefReg, temReg, gammaConfig_t->Order*3*2);
	gammaConfig_t->gammaRegWrite = gammaRegWrite_Normal3; //255写函数接�??
    gammaConfig_t->gammaRegWriteAll = gammaRegWriteAll_Normal3;    //全写函数接口
	gammaConfig_t->nndate = (float *) NOR3_NNData; 	    //模型库绑�??(def.h文件)
    
    #ifdef CL2
	gammaConfig_t->readLv = CL2_read_xylv; //CL2读取镜头�??
    #endif                                 
    #ifdef CA310                           
	gammaConfig_t->readLv = readLv_CA310;  //CA310读取镜头�??
    #endif
    #ifdef CA410
	gammaConfig_t->readLv = readLv_CA310;//CA310读取镜头�??
    #endif
		#ifdef CL5
	gammaConfig_t->readLv = readLv_CA310;//CA310读取镜头�??
    #endif
                    
	for (int i = 0; i < gammaConfig_t->Order; i++)
	{
		gammaConfig_t->OGammaConfig[i].level = level[i];		
		gammaConfig_t->OGammaConfig[i].tuningN = 100;//每阶运行次数
	gammaConfig_t->OGammaConfig[i].targetX = X;
	gammaConfig_t->OGammaConfig[i].targetY = Y;
		
		if (i == gammaConfig_t->Order - 1)
		{			
      //有带F结尾的浮点数类型修改�?? -> 一定要加F
			gammaConfig_t->OGammaConfig[i].dGamma = 0.0F;              //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 30.0F;	                //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.01F;            //色坐标卡�??	
			gammaConfig_t->OGammaConfig[i].De= 0;	                //DE卡控，一般小�??0.5
			gammaConfig_t->OGammaConfig[i].readD=1;                 //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=2;			        //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;                 //读镜头延�??
		}else if (i == gammaConfig_t->Order - 3)  // Gray191 ~ 223 
		{			
			gammaConfig_t->OGammaConfig[i].dGamma = 0.06F;	    //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		        //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.0025F;  	    //色坐标卡�??	
			gammaConfig_t->OGammaConfig[i].readD = 2;           //DE卡控，一般小�??0.5
			gammaConfig_t->OGammaConfig[i].readN = 3;			//此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].De = 0.5F;			//此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
		}else if (i >= 7) // Gray19 ~ 159
		{			
			gammaConfig_t->OGammaConfig[i].dGamma = 0.06F;       //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		        //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.0025F;        //色坐标卡�??	
			gammaConfig_t->OGammaConfig[i].readD = 2;           //DE卡控，一般小�??0.5
			gammaConfig_t->OGammaConfig[i].readN = 3;			//此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].De = 0.5F;			//此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
		}else if (i >= 6)  // Gray15 
		{			
			gammaConfig_t->OGammaConfig[i].dGamma = 0.06F;	    //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		        //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.0052F ;  	    //色坐标卡�??	
			gammaConfig_t->OGammaConfig[i].readD = 2;           //DE卡控，一般小�??0.5
			gammaConfig_t->OGammaConfig[i].readN = 3;			//此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].De = 0.5F;		    //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
		}else if (i >= 5)  // Gray11
		{			
			gammaConfig_t->OGammaConfig[i].dGamma = 0.02F;	    //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		        //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.0082F;  	    //色坐标卡�??	
			gammaConfig_t->OGammaConfig[i].readD = 2;           //DE卡控，一般小�??0.5
			gammaConfig_t->OGammaConfig[i].readN = 3;			//此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].De = 0.5F;			//此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
		}else if (i >= 3)  // Gray5 ~ 7
		{			
			gammaConfig_t->OGammaConfig[i].dGamma = 0.01F;	    //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		        //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.02F;  	    //色坐标卡�??	
			gammaConfig_t->OGammaConfig[i].readD = 2;           //DE卡控，一般小�??0.5
			gammaConfig_t->OGammaConfig[i].readN = 3;			//此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].De = 0.5F;			//此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
		}else if (i >= 1)  // Gray5                               
		{			                                  
			gammaConfig_t->OGammaConfig[i].dGamma = 0.0F;		       //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		           //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.0F;  	           //色坐标卡�??	
			gammaConfig_t->OGammaConfig[i].De= 0;				   //DE卡控，一般小�??0.5
			gammaConfig_t->OGammaConfig[i].readD=3;                //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=3;                //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			                                                      
		}else  // Gray0 ~ 3
		{			
			gammaConfig_t->OGammaConfig[i].dGamma = 0.0F;	    //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		    //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.0F;  	    //色坐标卡�??	
			gammaConfig_t->OGammaConfig[i].De = 0.0F;		//DE卡控，一般小�??0.5		
		}                                                   
                                                            
		//getGamma2Err
		
		//获取亮度范围
		gammaConfig_t->OGammaConfig[i].targetLv =  maxLv * pow((level[i])/255.0F,gamma);
		if(gammaConfig_t->OGammaConfig[i].dGamma > 1e-6)
		{			
			gammaConfig_t->OGammaConfig[i].targetlvMax = maxLv * pow((level[i])/255.0F,gamma - gammaConfig_t->OGammaConfig[i].dGamma);
			gammaConfig_t->OGammaConfig[i].targetlvMin = maxLv * pow((level[i])/255.0F,gamma + gammaConfig_t->OGammaConfig[i].dGamma);
		}
		else
		{
			gammaConfig_t->OGammaConfig[i].targetlvMax = gammaConfig_t->OGammaConfig[i].targetLv + gammaConfig_t->OGammaConfig[i].dLv;
			gammaConfig_t->OGammaConfig[i].targetlvMin = gammaConfig_t->OGammaConfig[i].targetLv - gammaConfig_t->OGammaConfig[i].dLv;
		}

	}

	return 0;
}

int gammaConfigInitNormal2(void)
{
    //绑定定义的全局结构�?? GammaConfig_Idle
	NEW_GammaConfigStruct *gammaConfig_t = &GammaConfig_Normal2;
	
    //详细配置new_gamma_def.h文件
	float X = NOR2_DEF_GAMMA_TARGET_X;              //目标色坐标X
	float Y = NOR2_DEF_GAMMA_TARGET_Y;              //目标色坐标Y
	float maxLv = NOR2_DEF_GAMMA_TARGET_MAX_LV;     //255阶目标亮度Lv
	                                                
	float gamma = NOR2_DEF_GAMMA_TARGET_GAMMA;      //目标gamma(gamma决定每一阶的亮度)
	const uint16_t temReg[NOR2_DEF_GAMMA_ORDER_NUM * 3] = NORMAL2_DEF_GAMMA_RGB_REG;           //初值数据绑定到数组(在new_gamma_def.h配置)
	const uint8_t level[NOR2_DEF_GAMMA_ORDER_NUM] = NOR2_DEF_GAMMA_LEVEL;                   //绑点数据绑定到数�??(在new_gamma_def.h配置)
	                                                                                        
    //将上面的配置数据绑定到Idle结构�??                                                        
    memcpy(gammaConfig_t->displaymode, "Normal_Band2", sizeof(gammaConfig_t->displaymode)); //绑定当前结构体ID名称，用于保存模型数据到内存卡时文件命名
	gammaConfig_t->maxlv=  maxLv;
	gammaConfig_t->gamma = gamma;
//	gammaConfig_t->targetX = X;
//	gammaConfig_t->targetY = Y;
	gammaConfig_t->Order = NOR2_DEF_GAMMA_ORDER_NUM;

	gammaConfig_t->RGB_MAX = 1023;  //gamma调节中的RGB值的最大范�??
	gammaConfig_t->RGB_MIN = 0; 	//gamma调节中的RGB值的最小范�??
		
	memcpy(gammaConfig_t->gammaDefReg, temReg, gammaConfig_t->Order*3*2);
	gammaConfig_t->gammaRegWrite = gammaRegWrite_Normal2; //255写函数接�??
    gammaConfig_t->gammaRegWriteAll = gammaRegWriteAll_Normal2;    //全写函数接口
	gammaConfig_t->nndate = (float *) NOR2_NNData;	    //模型库绑�??(def.h文件)
    
    #ifdef CL2
	gammaConfig_t->readLv = CL2_read_xylv;//CL2读取镜头�??
    #endif                                
    #ifdef CA310                          
	gammaConfig_t->readLv = readLv_CA310; //CA310读取镜头�??
    #endif	
    #ifdef CA410
	gammaConfig_t->readLv = readLv_CA310;//CA310读取镜头�??
    #endif
		#ifdef CL5
	gammaConfig_t->readLv = readLv_CA310;//CA310读取镜头�??
    #endif
                 
 //遍历所有绑点配置对应绑点的卡控
	for (int i = 0; i < gammaConfig_t->Order; i++)
	{
		gammaConfig_t->OGammaConfig[i].level = level[i];		
		gammaConfig_t->OGammaConfig[i].tuningN = 100;//每阶运行次数
	gammaConfig_t->OGammaConfig[i].targetX = X;
	gammaConfig_t->OGammaConfig[i].targetY = Y;
		
		#ifdef CL2
		if (i == gammaConfig_t->Order - 1)
		{
      //有带F结尾的浮点数类型修改�?? -> 一定要加F
			gammaConfig_t->OGammaConfig[i].dGamma = 0.0F;              //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.8F;	                //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.002F;            //色坐标卡�??	
			gammaConfig_t->OGammaConfig[i].De= 0;	                //DE卡控，一般小�??0.5
			gammaConfig_t->OGammaConfig[i].readD=2;                 //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=3;                 //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;                 //读镜头时延时时间
			
		}else if (i >= 25)  // Gray239 
		{			
			gammaConfig_t->OGammaConfig[i].dGamma = 0.05F;	       //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		           //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.002F;  	       //色坐标卡�??	
			gammaConfig_t->OGammaConfig[i].De= 0;	               //DE卡控，一般小�??0.5
			gammaConfig_t->OGammaConfig[i].readD=2;                //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=3;                //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			
		}else if (i >= 20)  
		{			
			gammaConfig_t->OGammaConfig[i].dGamma = 0.03F;	        //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		            //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.002F;  	        //色坐标卡�??	
			gammaConfig_t->OGammaConfig[i].De= 0;	                //DE卡控，一般小�??0.5
			gammaConfig_t->OGammaConfig[i].readD=2;                 //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=3;                 //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			
		}else if (i >= 19)  
		{			
			gammaConfig_t->OGammaConfig[i].dGamma = 0.02F;	        //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		            //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.002F;  	        //色坐标卡�??	
			gammaConfig_t->OGammaConfig[i].De= 0;	                //DE卡控，一般小�??0.5
			gammaConfig_t->OGammaConfig[i].readD=2;                 //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=3;                 //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			
		}else if (i >= 13) 
		{			
			gammaConfig_t->OGammaConfig[i].dGamma = 0.02F;	    //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		        //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.002F;  	    //色坐标卡�??	
			gammaConfig_t->OGammaConfig[i].De= 0;	            //DE卡控，一般小�??0.5
			gammaConfig_t->OGammaConfig[i].readD=2;             //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=3;             //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			
		}else if (i >= 10)  //Gray35
		{			
			gammaConfig_t->OGammaConfig[i].dGamma = 0.02F;	    //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		        //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.002F;  	    //色坐标卡�??	
			gammaConfig_t->OGammaConfig[i].De= 0;		        //DE卡控，一般小�??0.5
			gammaConfig_t->OGammaConfig[i].readD=2;             //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=3;             //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			
		}else if (i >= 7)  //Gray23,27,31 
		{			
			gammaConfig_t->OGammaConfig[i].dGamma = 0.02F;	    //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		        //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.003F;  	    //色坐标卡�??	
			gammaConfig_t->OGammaConfig[i].De= 0;	            //DE卡控，一般小�??0.5
			gammaConfig_t->OGammaConfig[i].readD=3;             //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=3;			    //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
		}else if (i >= 6)  //Gray19 
		{			
			gammaConfig_t->OGammaConfig[i].dGamma = 0.02F;	    //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		        //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.000F;  	    //色坐标卡�??	0.003
			gammaConfig_t->OGammaConfig[i].De= 0;	            //DE卡控，一般小�??0.5
			gammaConfig_t->OGammaConfig[i].readD=3;             //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=3;			    //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
		}else if (i >= 5)  //Gray15  0.236nit
		{			
			gammaConfig_t->OGammaConfig[i].dGamma = 0.03F;	    //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		        //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.004F;  	    //色坐标卡�??	
			gammaConfig_t->OGammaConfig[i].De= 0;             //DE卡控，一般小�??0.5
			gammaConfig_t->OGammaConfig[i].readD=3;             //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=3;             //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			
		}else if (i >= 4)  //Gray11 0.119nit
		{			
			gammaConfig_t->OGammaConfig[i].dGamma = 0.05F;       //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		        //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.004F;         //色坐标卡�??	
			gammaConfig_t->OGammaConfig[i].De= 0;	            //DE卡控，一般小�??0.5
			gammaConfig_t->OGammaConfig[i].readD=3;             //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=3;		        //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
		}else if (i >= 3)  //Gray7  0.044nit
		{			
			gammaConfig_t->OGammaConfig[i].dGamma = 0.08F;       //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		        //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.005F;         //色坐标卡�??	
			gammaConfig_t->OGammaConfig[i].De= 0;	            //DE卡控，一般小�??0.5
			gammaConfig_t->OGammaConfig[i].readD=3;             //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=3;             //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			
		}else if (i >= 2)  //Gray3  0.0068nit
		{			
			gammaConfig_t->OGammaConfig[i].dGamma = 0.0F;        //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		        //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.0F;  	        //色坐标卡�??	
			gammaConfig_t->OGammaConfig[i].De= 0;	            //DE卡控，一般小�??0.5
			gammaConfig_t->OGammaConfig[i].readD=3;             //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=3;             //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			
		}else if (i >= 1)  //Gray1                               
		{			                                  
			gammaConfig_t->OGammaConfig[i].dGamma = 0.0F;		       //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		           //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.0F;  	           //色坐标卡�??	
			gammaConfig_t->OGammaConfig[i].De= 0;				   //DE卡控，一般小�??0.5
			gammaConfig_t->OGammaConfig[i].readD=3;                //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=3;                //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			                                                      
		}else  // Gray0
		{			
			gammaConfig_t->OGammaConfig[i].dGamma = 0.0F;	        //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		        //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.0F;  	        //色坐标卡�??	
			gammaConfig_t->OGammaConfig[i].De= 0;		        //DE卡控，一般小�??0.5
		}                                                                                              
    #endif
		
		#ifdef CL5
		if (i == gammaConfig_t->Order - 1)
		{			
            //有带F结尾的浮点数类型修改�?? -> 一定要加F
			gammaConfig_t->OGammaConfig[i].dGamma = 0.0F;      //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.05F;	    //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.003F;    //色坐标卡�??	
			gammaConfig_t->OGammaConfig[i].readD = 1;       //DE卡控，一般小�??0.5
			gammaConfig_t->OGammaConfig[i].readN = 2;		//此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].De = 0;		//此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;         //读镜头延时ms
		}
		else if (i >= 25)  // Gray239   
		{			                                             
			gammaConfig_t->OGammaConfig[i].dGamma = 0.03F;   //ZUOZUO�??2024.8.11�??0.04改成0.07//gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		    //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.003F;     //色坐标卡�??	
			gammaConfig_t->OGammaConfig[i].readD = 1;       //DE卡控，一般小�??0.5
			gammaConfig_t->OGammaConfig[i].readN = 2;		//此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].De = 0;		//此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
		} 
		else if (i >= 23)  // Gray223   //
		{			                                             
			gammaConfig_t->OGammaConfig[i].dGamma = 0.04F;   //ZUOZUO�??2024.8.11新增gray223//gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		    //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.003F;     //色坐标卡�??	
			gammaConfig_t->OGammaConfig[i].readD = 1;       //DE卡控，一般小�??0.5
			gammaConfig_t->OGammaConfig[i].readN = 2;		//此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].De = 0;		//此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
		} 
		else if (i >= 17)  // Gray223 ~ 79 
		{			
			gammaConfig_t->OGammaConfig[i].dGamma = 0.02F;   //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		    //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.003F;     //色坐标卡�??	
			gammaConfig_t->OGammaConfig[i].readD = 1;       //DE卡控，一般小�??0.5
			gammaConfig_t->OGammaConfig[i].readN = 2;		//此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].De = 0;		//此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
		}
		else if (i >= 13) // Gray41 ~ 71 
		{			
			gammaConfig_t->OGammaConfig[i].dGamma = 0.02F;   //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		    //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.003F;    //色坐标卡�??	
			gammaConfig_t->OGammaConfig[i].readD = 1;       //DE卡控，一般小�??0.5
			gammaConfig_t->OGammaConfig[i].readN = 2;		//此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].De = 0;		//此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
		}else if (i >= 10)  // Gray35 ~ 39 
		{			
			gammaConfig_t->OGammaConfig[i].dGamma = 0.025F;	//gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		    //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.005F;     //色坐标卡�??	
			gammaConfig_t->OGammaConfig[i].readD = 1;       //DE卡控，一般小�??0.5
			gammaConfig_t->OGammaConfig[i].readN = 2;		//此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].De = 0;		//此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
		}
		else if (i >= 9)  // Gray31  
		{			
			gammaConfig_t->OGammaConfig[i].dGamma = 0.028F;	//gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		    //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.005F;  	//色坐标卡�??	
			gammaConfig_t->OGammaConfig[i].readD = 1;       //DE卡控，一般小�??0.5
			gammaConfig_t->OGammaConfig[i].readN = 2;		//此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].De = 0;		//此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
		}
		else if (i >= 8)  // Gray27
		{			
			gammaConfig_t->OGammaConfig[i].dGamma = 0.00F;	//gamma卡控  0.04
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		    //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.007F;  	//色坐标卡�??	 0.005   0.007
			gammaConfig_t->OGammaConfig[i].readD = 1;       //DE卡控，一般小�??0.5
			gammaConfig_t->OGammaConfig[i].readN = 2;		//此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].De = 0;		//此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
		}else if (i >= 7)  // Gray23   0.05nit
		{			
			gammaConfig_t->OGammaConfig[i].dGamma = 0.00F;	//gamma卡控  250621 0.075
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		    //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.007F;  	//色坐标卡�??	 0.007
			gammaConfig_t->OGammaConfig[i].readD = 1;       //DE卡控，一般小�??0.5
			gammaConfig_t->OGammaConfig[i].readN = 2;		//此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].De = 0;		//此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
		}
//		else if (i >= 6)  // Gray19   0.05nit
//		{			
//			gammaConfig_t->OGammaConfig[i].dGamma = 0.00F;	//gamma卡控  250621 0.075
//			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		    //亮度卡控
//			gammaConfig_t->OGammaConfig[i].dxy = 0.007F;  	//色坐标卡�??	 0.01
//			gammaConfig_t->OGammaConfig[i].readD = 1;       //DE卡控，一般小�??0.5
//			gammaConfig_t->OGammaConfig[i].readN = 2;		//此读镜头次数当作延时处理
//			gammaConfig_t->OGammaConfig[i].De = 0;		//此读镜头次数为真正取值读镜头次数
//            gammaConfig_t->OGammaConfig[i].delay=0;
//		}
			else if (i >= 5)  // Gray15   0.05nit
		{			
			gammaConfig_t->OGammaConfig[i].dGamma = 0.075F;	//gamma卡控   0.08
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		    //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.00F;  	//色坐标卡�??	 0.01
			gammaConfig_t->OGammaConfig[i].readD = 2;       //DE卡控，一般小�??0.5
			gammaConfig_t->OGammaConfig[i].readN = 2;		//此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].De = 0;		//此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
		}
		else  // Gray0 ~ 1
		{					
			gammaConfig_t->OGammaConfig[i].dGamma = 0.0F;		       //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		           //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.0F;  	           //色坐标卡�??	
			gammaConfig_t->OGammaConfig[i].De= 0;				   //DE卡控，一般小�??0.5
			gammaConfig_t->OGammaConfig[i].readD=1;                //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=2;                //此读镜头次数为真正取值读镜头次数
			gammaConfig_t->OGammaConfig[i].delay=0;
		}  
		#endif
		//getGamma2Err                                      
		
//		//获取亮度范围
//		gammaConfig_t->OGammaConfig[i].targetLv =  maxLv * pow((level[i])/255.0F,gamma);
//		if(gammaConfig_t->OGammaConfig[i].dGamma > 1e-6)
//		{			
//			gammaConfig_t->OGammaConfig[i].targetlvMax = maxLv * pow((level[i])/255.0F,gamma - gammaConfig_t->OGammaConfig[i].dGamma);
//			gammaConfig_t->OGammaConfig[i].targetlvMin = maxLv * pow((level[i])/255.0F,gamma + gammaConfig_t->OGammaConfig[i].dGamma);
//		}
//		else
//		{
//			gammaConfig_t->OGammaConfig[i].targetlvMax = gammaConfig_t->OGammaConfig[i].targetLv + gammaConfig_t->OGammaConfig[i].dLv;
//			gammaConfig_t->OGammaConfig[i].targetlvMin = gammaConfig_t->OGammaConfig[i].targetLv - gammaConfig_t->OGammaConfig[i].dLv;
//		}
		
		if (i == 8)  // Gray3   0.0342nit                           
		{			                                             
			// 更改目标
			gammaConfig_t->OGammaConfig[i].targetX = 0.300F;
			gammaConfig_t->OGammaConfig[i].targetY = 0.315F;

			gammaConfig_t->OGammaConfig[i].targetLv = 0.078F;//maxLv * pow((level[i])/255.0F,gamma);
      gammaConfig_t->OGammaConfig[i].dLv = 0.005F;	
      
			gammaConfig_t->OGammaConfig[i].targetlvMax = 0.078f+gammaConfig_t->OGammaConfig[i].dLv;//maxLv * pow((level[i])/255.0F,gamma - gammaConfig_t->OGammaConfig[i].dGamma);
			gammaConfig_t->OGammaConfig[i].targetlvMin = 0.078f-gammaConfig_t->OGammaConfig[i].dLv;//maxLv * pow((level[i])/255.0F,gamma + gammaConfig_t->OGammaConfig[i].dGamma);                                             
		}else if(i == 7)
		{			                                             
			// 更改目标
			gammaConfig_t->OGammaConfig[i].targetX = 0.300F;
			gammaConfig_t->OGammaConfig[i].targetY = 0.315F;

			gammaConfig_t->OGammaConfig[i].targetLv = 0.070F;//maxLv * pow((level[i])/255.0F,gamma);
      gammaConfig_t->OGammaConfig[i].dLv = 0.005F;	
      
			gammaConfig_t->OGammaConfig[i].targetlvMax = 0.070f+gammaConfig_t->OGammaConfig[i].dLv;//maxLv * pow((level[i])/255.0F,gamma - gammaConfig_t->OGammaConfig[i].dGamma);
			gammaConfig_t->OGammaConfig[i].targetlvMin = 0.070f-gammaConfig_t->OGammaConfig[i].dLv;//maxLv * pow((level[i])/255.0F,gamma + gammaConfig_t->OGammaConfig[i].dGamma);                                             
		}
    else
    {
      //获取亮度范围
      gammaConfig_t->OGammaConfig[i].targetLv =  maxLv * pow((level[i])/255.0F,gamma);
      if(gammaConfig_t->OGammaConfig[i].dGamma > 1e-6)
      {			
        gammaConfig_t->OGammaConfig[i].targetlvMax = maxLv * pow((level[i])/255.0F,gamma - gammaConfig_t->OGammaConfig[i].dGamma);
        gammaConfig_t->OGammaConfig[i].targetlvMin = maxLv * pow((level[i])/255.0F,gamma + gammaConfig_t->OGammaConfig[i].dGamma);
      }
      else
      {
        gammaConfig_t->OGammaConfig[i].targetlvMax = gammaConfig_t->OGammaConfig[i].targetLv + gammaConfig_t->OGammaConfig[i].dLv;
        gammaConfig_t->OGammaConfig[i].targetlvMin = gammaConfig_t->OGammaConfig[i].targetLv - gammaConfig_t->OGammaConfig[i].dLv;
      }
    }			

	}
	
	return 0;
}

int gammaConfigInitNormal4(void)
{
    //绑定定义的全局结构�?? GammaConfig_Idle
	NEW_GammaConfigStruct *gammaConfig_t = &GammaConfig_Normal4;
	
    //详细配置new_gamma_def.h文件
	float X = NORMAL_DEF_GAMMA_TARGET_X;            //目标色坐标X
	float Y = NORMAL_DEF_GAMMA_TARGET_Y;            //目标色坐标Y
	float maxLv = NORMAL_DEF_GAMMA_TARGET_MAX_LV;   //255阶目标亮度Lv
	                                                
	float gamma = NORMAL_DEF_GAMMA_TARGET_GAMMA;    //目标gamma(gamma决定每一阶的亮度)
	const uint16_t temReg[NORMAL_DEF_GAMMA_ORDER_NUM * 3] = NORMAL_DEF_GAMMA_RGB_REG;        //初值数据绑定到数组(在new_gamma_def.h配置)   
	const uint8_t level[NORMAL_DEF_GAMMA_ORDER_NUM] = NORMAL_DEF_GAMMA_LEVEL;                //绑点数据绑定到数�??(在new_gamma_def.h配置)
	                                                                                         
    //将上面的配置数据绑定到Idle结构�??                                                                                         
    memcpy(gammaConfig_t->displaymode, "Normal_Band1", sizeof(gammaConfig_t->displaymode));  //绑定当前结构体ID名称，用于保存模型数据到内存卡时文件命名
	gammaConfig_t->maxlv=  maxLv;
	gammaConfig_t->gamma = gamma;
//	gammaConfig_t->targetX = X;
//	gammaConfig_t->targetY = Y;
	gammaConfig_t->Order = NORMAL_DEF_GAMMA_ORDER_NUM;

	gammaConfig_t->RGB_MAX = 1023;      //gamma调节中的RGB值的最大范�??
	gammaConfig_t->RGB_MIN = 0;         //gamma调节中的RGB值的最小范�??
		
	memcpy(gammaConfig_t->gammaDefReg, temReg, gammaConfig_t->Order*3*2);
	gammaConfig_t->gammaRegWrite = gammaRegWrite_Normal;    //255写函数接�??
    gammaConfig_t->gammaRegWriteAll = gammaRegWriteAll_Normal;       //全写函数接口
	gammaConfig_t->nndate = (float *) NORMAL_NNData;	    //模型库绑�??(def.h文件)
    
    #ifdef CL2
	gammaConfig_t->readLv = CL2_read_xylv; //CL2读取镜头�??
    #endif                                 
    #ifdef CA310                           
	gammaConfig_t->readLv = readLv_CA310;  //CA310读取镜头�??
    #endif
    #ifdef CA410
	gammaConfig_t->readLv = readLv_CA310;//CA310读取镜头�??
    #endif
		#ifdef CL5
	gammaConfig_t->readLv = readLv_CA310;//CA310读取镜头�??
    #endif
	
    
  //遍历所有绑点配置对应绑点的卡控
	for (int i = 0; i < gammaConfig_t->Order; i++)
	{
		gammaConfig_t->OGammaConfig[i].level = level[i];		
		gammaConfig_t->OGammaConfig[i].tuningN = 100;//每阶运行次数
	gammaConfig_t->OGammaConfig[i].targetX = X;
	gammaConfig_t->OGammaConfig[i].targetY = Y;
		
		#ifdef CL2
		if (i == gammaConfig_t->Order - 1)
		{			
      //有带F结尾的浮点数类型修改�?? -> 一定要加F
			gammaConfig_t->OGammaConfig[i].dGamma = 0.0F;              //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 3.0F;	                //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.002F;            //色坐标卡�??	
			gammaConfig_t->OGammaConfig[i].De= 0;	                //DE卡控，一般小�??0.5
			gammaConfig_t->OGammaConfig[i].readD=2;                 //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=3;			        //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;                 //读镜头延�??
		}
		else if (i >= 25)  // Gray239                      
		{			                                            
			gammaConfig_t->OGammaConfig[i].dGamma = 0.05F;	         //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		             //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.002F;  	         //色坐标卡�??	
			gammaConfig_t->OGammaConfig[i].De= 0;                  //DE卡控，一般小�??0.5
			gammaConfig_t->OGammaConfig[i].readD=3;                  //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=3;			         //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
		} 		
		else if (i >= 22)                      
		{			                                            
			gammaConfig_t->OGammaConfig[i].dGamma = 0.03F;	         //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		             //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.002F;  	         //色坐标卡�??	
			gammaConfig_t->OGammaConfig[i].De= 0;                  //DE卡控，一般小�??0.5
			gammaConfig_t->OGammaConfig[i].readD=2;                  //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=3;			         //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
		}    
		else if (i >= 20)                                       
		{			                                            
			gammaConfig_t->OGammaConfig[i].dGamma = 0.01F;	         //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		             //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.002F;  	         //色坐标卡�??	
			gammaConfig_t->OGammaConfig[i].De= 0;                  //DE卡控，一般小�??0.5
			gammaConfig_t->OGammaConfig[i].readD=2;                  //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=3;			         //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
		}    		
		else if (i >= 18)                                       
		{			                                            
			gammaConfig_t->OGammaConfig[i].dGamma = 0.01F;	         //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		             //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.002F;  	         //色坐标卡�??	
			gammaConfig_t->OGammaConfig[i].De= 0;                  //DE卡控，一般小�??0.5
			gammaConfig_t->OGammaConfig[i].readD=2;                  //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=3;			         //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
		}                                                                                                                      
		else if (i >= 17)                                       
		{			                                            
			gammaConfig_t->OGammaConfig[i].dGamma = 0.01F;	     //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		         //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.002F;          //色坐标卡�??	
			gammaConfig_t->OGammaConfig[i].De= 0;             //DE卡控，一般小�??0.5
			gammaConfig_t->OGammaConfig[i].readD=2;              //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=3;              //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			                                                    
		}else if (i >= 13) //                      
		{			                                            
			gammaConfig_t->OGammaConfig[i].dGamma = 0.01F;	     //gamma卡控 0.0035
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		         //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.002F;          //色坐标卡�??	
			gammaConfig_t->OGammaConfig[i].De= 0;	         //DE卡控，一般小�??0.5
			gammaConfig_t->OGammaConfig[i].readD=2;              //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=3;              //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			                                                    
		}else if (i >= 10)  //Gray35                      
		{			                                            
			gammaConfig_t->OGammaConfig[i].dGamma = 0.01F;	     //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		         //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.002F;         //色坐标卡�??	
			gammaConfig_t->OGammaConfig[i].De= 0;	             //DE卡控，一般小�??0.5
			gammaConfig_t->OGammaConfig[i].readD=2;              //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=3;              //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			                                                    
		}else if (i >= 7)  // Gray23,27,31                       
		{			                                            
			gammaConfig_t->OGammaConfig[i].dGamma = 0.01F;	     //gamma卡控  0.001
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		         //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.003F;          //色坐标卡�??	
			gammaConfig_t->OGammaConfig[i].De= 0;	             //DE卡控，一般小�??0.5
			gammaConfig_t->OGammaConfig[i].readD=2;              //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=3;              //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			                                                    
		}else if (i >= 5)  // Gray15,19                    
		{			                                            
			gammaConfig_t->OGammaConfig[i].dGamma = 0.015F;	    //gamma卡控  0.1
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		        //亮度卡控 
			gammaConfig_t->OGammaConfig[i].dxy = 0.004F;           //色坐标卡�??	 
			gammaConfig_t->OGammaConfig[i].De= 0;             //DE卡控，一般小�??0.5 
			gammaConfig_t->OGammaConfig[i].readD=2;             //此读镜头次数当作延时处理 
			gammaConfig_t->OGammaConfig[i].readN=3;             //此读镜头次数为真正取值读镜头次数 
            gammaConfig_t->OGammaConfig[i].delay=0;
			                                                    
		}else if (i >= 4)  // Gray11                              
		{			                                             
			gammaConfig_t->OGammaConfig[i].dGamma = 0.015F;	     //gamma卡控 0.15
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		         //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.005F;  	     //色坐标卡�??	
			gammaConfig_t->OGammaConfig[i].De= 0;		         //DE卡控，一般小�??0.5
			gammaConfig_t->OGammaConfig[i].readD=3;              //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=3;              //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			                                                     
		}else if (i >= 3)  // Gray7  0.22nit                    
		{			                                             
			gammaConfig_t->OGammaConfig[i].dGamma = 0.03F;	     //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		         //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.006F;            //色坐标卡�??	
			gammaConfig_t->OGammaConfig[i].De= 0;	         //DE卡控，一般小�??0.5
			gammaConfig_t->OGammaConfig[i].readD=3;              //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=3;              //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			                                                     
		}else if (i >= 2)  // Gray3   0.034nit                          
		{			                                             
			gammaConfig_t->OGammaConfig[i].dGamma = 0.08F;	     //gamma卡控 0.3
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		         //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.01F;  	         //色坐标卡�??	
			gammaConfig_t->OGammaConfig[i].De= 0;		         //DE卡控，一般小�??0.5
			gammaConfig_t->OGammaConfig[i].readD=3;              //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=3;              //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			                                                     
		}else if (i >= 1)  // Gray1                               
		{			                                  
			gammaConfig_t->OGammaConfig[i].dGamma = 0.0F;		       //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		           //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.0F;  	           //色坐标卡�??	
			gammaConfig_t->OGammaConfig[i].De= 0;				   //DE卡控，一般小�??0.5
			gammaConfig_t->OGammaConfig[i].readD=0;                //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=1;                //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			                                                      
		}else  // Gray0                                     
		{			                                             
			gammaConfig_t->OGammaConfig[i].dGamma = 0.0F;		     //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		         //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.0F;  	         //色坐标卡�??	
			gammaConfig_t->OGammaConfig[i].De= 0;				 //DE卡控，一般小�??0.5
		}
		#endif
		
		#ifdef CL5
		if (i == gammaConfig_t->Order - 1)
		{			
      //有带F结尾的浮点数类型修改�?? -> 一定要加F
			gammaConfig_t->OGammaConfig[i].dGamma = 0.0F;              //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 30.0F;	                //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.01F;            //色坐标卡�??	
			gammaConfig_t->OGammaConfig[i].De= 0;	                //DE卡控，一般小�??0.5
			gammaConfig_t->OGammaConfig[i].readD=1;                 //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=2;			        //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;                 //读镜头延�??
		}
		else if (i >= 25)  // Gray239                      
		{			                                            
			gammaConfig_t->OGammaConfig[i].dGamma = 0.04F;	         //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		             //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.002F;  	         //色坐标卡�??	
			gammaConfig_t->OGammaConfig[i].De= 0;                  //DE卡控，一般小�??0.5
			gammaConfig_t->OGammaConfig[i].readD=1;                  //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=2;			         //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
		} 		
		else if (i >= 22)                      
		{			                                            
			gammaConfig_t->OGammaConfig[i].dGamma = 0.03F;	         //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		             //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.002F;  	         //色坐标卡�??	
			gammaConfig_t->OGammaConfig[i].De= 0;                  //DE卡控，一般小�??0.5
			gammaConfig_t->OGammaConfig[i].readD=1;                  //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=2;			         //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
		}    
		else if (i >= 20)                                       
		{			                                            
			gammaConfig_t->OGammaConfig[i].dGamma = 0.01F;	         //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		             //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.002F;  	         //色坐标卡�??	
			gammaConfig_t->OGammaConfig[i].De= 0;                  //DE卡控，一般小�??0.5
			gammaConfig_t->OGammaConfig[i].readD=1;                  //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=2;			         //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
		}    		
		else if (i >= 18)                                       
		{			                                            
			gammaConfig_t->OGammaConfig[i].dGamma = 0.01F;	         //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		             //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.002F;  	         //色坐标卡�??	
			gammaConfig_t->OGammaConfig[i].De= 0;                  //DE卡控，一般小�??0.5
			gammaConfig_t->OGammaConfig[i].readD=1;                  //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=2;			         //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
		}                                                                                                                      
		else if (i >= 17)                                       
		{			                                            
			gammaConfig_t->OGammaConfig[i].dGamma = 0.01F;	     //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		         //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.002F;          //色坐标卡�??	
			gammaConfig_t->OGammaConfig[i].De= 0;             //DE卡控，一般小�??0.5
			gammaConfig_t->OGammaConfig[i].readD=1;              //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=2;              //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			                                                    
		}else if (i >= 13) //                      
		{			                                            
			gammaConfig_t->OGammaConfig[i].dGamma = 0.01F;	     //gamma卡控 0.0035
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		         //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.002F;          //色坐标卡�??	
			gammaConfig_t->OGammaConfig[i].De= 0;	         //DE卡控，一般小�??0.5
			gammaConfig_t->OGammaConfig[i].readD=1;              //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=2;              //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			                                                    
		}else if (i >= 10)  //Gray35                      
		{			                                            
			gammaConfig_t->OGammaConfig[i].dGamma = 0.01F;	     //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		         //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.002F;         //色坐标卡�??	
			gammaConfig_t->OGammaConfig[i].De= 0;	             //DE卡控，一般小�??0.5
			gammaConfig_t->OGammaConfig[i].readD=1;              //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=2;              //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			                                                    
		}else if (i >= 7)  // Gray23,27,31                       
		{			                                            
			gammaConfig_t->OGammaConfig[i].dGamma = 0.01F;	     //gamma卡控  0.001
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		         //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.003F;          //色坐标卡�??	
			gammaConfig_t->OGammaConfig[i].De= 0;	             //DE卡控，一般小�??0.5
			gammaConfig_t->OGammaConfig[i].readD=1;              //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=2;              //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			                                                    
		}else if (i >= 5)  // Gray15,19                    
		{			                                            
			gammaConfig_t->OGammaConfig[i].dGamma = 0.015F;	    //gamma卡控  0.1
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		        //亮度卡控 
			gammaConfig_t->OGammaConfig[i].dxy = 0.004F;           //色坐标卡�??	 
			gammaConfig_t->OGammaConfig[i].De= 0;             //DE卡控，一般小�??0.5 
			gammaConfig_t->OGammaConfig[i].readD=1;             //此读镜头次数当作延时处理 
			gammaConfig_t->OGammaConfig[i].readN=2;             //此读镜头次数为真正取值读镜头次数 
            gammaConfig_t->OGammaConfig[i].delay=0;
			                                                    
		}else if (i >= 4)  // Gray11                              
		{			                                             
			gammaConfig_t->OGammaConfig[i].dGamma = 0.015F;	     //gamma卡控 0.15
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		         //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.005F;  	     //色坐标卡�??	
			gammaConfig_t->OGammaConfig[i].De= 0;		         //DE卡控，一般小�??0.5
			gammaConfig_t->OGammaConfig[i].readD=1;              //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=2;              //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			                                                     
		}else if (i >= 3)  // Gray7  0.22nit                    
		{			                                             
			gammaConfig_t->OGammaConfig[i].dGamma = 0.03F;	     //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		         //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.006F;            //色坐标卡�??	
			gammaConfig_t->OGammaConfig[i].De= 0;	         //DE卡控，一般小�??0.5
			gammaConfig_t->OGammaConfig[i].readD=1;              //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=2;              //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			                                                     
		}else if (i == 2)  // Gray3   0.034nit                          
		{			                                             
			gammaConfig_t->OGammaConfig[i].dGamma = 0.0F;	     //gamma卡控 0.3
			gammaConfig_t->OGammaConfig[i].dLv = 0.01F;		         //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.008F;  	         //色坐标卡�??	  0.03   250512 X�??0.45调不�??
			gammaConfig_t->OGammaConfig[i].De= 0;		         //DE卡控，一般小�??0.5
			gammaConfig_t->OGammaConfig[i].readD=1;              //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=2;              //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			                                                     
		}else if (i >= 1)  // Gray1                               
		{			                                  
			gammaConfig_t->OGammaConfig[i].dGamma = 0.0F;		       //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		           //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.0F;  	           //色坐标卡�??	
			gammaConfig_t->OGammaConfig[i].De= 0;				   //DE卡控，一般小�??0.5
			gammaConfig_t->OGammaConfig[i].readD=0;                //此读镜头次数当作延时处理
			gammaConfig_t->OGammaConfig[i].readN=1;                //此读镜头次数为真正取值读镜头次数
            gammaConfig_t->OGammaConfig[i].delay=0;
			                                                      
		}else  // Gray0                                     
		{			                                             
			gammaConfig_t->OGammaConfig[i].dGamma = 0.0F;		     //gamma卡控
			gammaConfig_t->OGammaConfig[i].dLv = 0.0F;		         //亮度卡控
			gammaConfig_t->OGammaConfig[i].dxy = 0.0F;  	         //色坐标卡�??	
			gammaConfig_t->OGammaConfig[i].De= 0;				 //DE卡控，一般小�??0.5
		}
		#endif
				
		//getGamma2Err
		
//		//获取亮度范围
//		gammaConfig_t->OGammaConfig[i].targetLv =  maxLv * pow((level[i])/255.0F,gamma);
//		if(gammaConfig_t->OGammaConfig[i].dGamma > 1e-6)
//		{			
//			gammaConfig_t->OGammaConfig[i].targetlvMax = maxLv * pow((level[i])/255.0F,gamma - gammaConfig_t->OGammaConfig[i].dGamma);
//			gammaConfig_t->OGammaConfig[i].targetlvMin = maxLv * pow((level[i])/255.0F,gamma + gammaConfig_t->OGammaConfig[i].dGamma);
//		}
//		else
//		{
//			gammaConfig_t->OGammaConfig[i].targetlvMax = gammaConfig_t->OGammaConfig[i].targetLv + gammaConfig_t->OGammaConfig[i].dLv;
//			gammaConfig_t->OGammaConfig[i].targetlvMin = gammaConfig_t->OGammaConfig[i].targetLv - gammaConfig_t->OGammaConfig[i].dLv;
//		}
   	if (i == 2)  // Gray3   0.0342nit                           
		{			                                             
			// 更改目标
			gammaConfig_t->OGammaConfig[i].targetX = 0.300F;
			gammaConfig_t->OGammaConfig[i].targetY = 0.315F;

			gammaConfig_t->OGammaConfig[i].targetLv = 0.070F;//maxLv * pow((level[i])/255.0F,gamma);
      gammaConfig_t->OGammaConfig[i].dLv = 0.01F;	
      
			gammaConfig_t->OGammaConfig[i].targetlvMax = 0.065f+gammaConfig_t->OGammaConfig[i].dLv;//maxLv * pow((level[i])/255.0F,gamma - gammaConfig_t->OGammaConfig[i].dGamma);
			gammaConfig_t->OGammaConfig[i].targetlvMin = 0.075f-gammaConfig_t->OGammaConfig[i].dLv;//maxLv * pow((level[i])/255.0F,gamma + gammaConfig_t->OGammaConfig[i].dGamma);                                             
		}
    else
    {
      //获取亮度范围
      gammaConfig_t->OGammaConfig[i].targetLv =  maxLv * pow((level[i])/255.0F,gamma);
      if(gammaConfig_t->OGammaConfig[i].dGamma > 1e-6)
      {			
        gammaConfig_t->OGammaConfig[i].targetlvMax = maxLv * pow((level[i])/255.0F,gamma - gammaConfig_t->OGammaConfig[i].dGamma);
        gammaConfig_t->OGammaConfig[i].targetlvMin = maxLv * pow((level[i])/255.0F,gamma + gammaConfig_t->OGammaConfig[i].dGamma);
      }
      else
      {
        gammaConfig_t->OGammaConfig[i].targetlvMax = gammaConfig_t->OGammaConfig[i].targetLv + gammaConfig_t->OGammaConfig[i].dLv;
        gammaConfig_t->OGammaConfig[i].targetlvMin = gammaConfig_t->OGammaConfig[i].targetLv - gammaConfig_t->OGammaConfig[i].dLv;
      }
    }		

	}
	
	return 0;
}


/**
 * 函数�?? : GAMMA_Inint
 * 描述   : gamma调试初始化函�??
 * 参数   : �??
 * 返回   : 无返�??   
*/
void GAMMA_Inint(void)
{
    
		int res;
		#ifdef CL2
		if(CFL_Init()<0)
			_DEBUG("CL2 init NG\r\n");
		else
			_DEBUG("CL2 init OK\r\n");
    #endif
    
    #ifdef CA310
    _DEBUG("CA310_Init....\n\r"); 
    int res_ca310=CA310_Init();
    if(res_ca310>0){_DEBUG("CA310_Init-OK\n\r");}
    else {_DEBUG("CA310_Init-NG\n\r");};
    res_ca310=CA310_Init();
    if(res_ca310>0){_DEBUG("CA310_Init-OK\n\r");}
    else {_DEBUG("CA310_Init-NG\n\r");};
    #endif
    
    #ifdef CA410
    int res_ca410;
    _DEBUG("CA410_Init....\n\r"); 
     res_ca410=CA410_Init();
    if(res_ca410>0){_DEBUG("CA410_Init-OK\n\r");}
    else {_DEBUG("CA410_Init-NG\n\r");};
    COMCA410_SetMeasureCH(1);//选择通道
    #endif
	
	#ifdef CL5 
		CA310BindDriver(CA310_BusRS232_ID1_Init());  
		_DEBUG("CA410_Init s\n\r");
		res=CA410_Init(CH_NUM,0);  //设置通道
		_DEBUG("CA410_Init end= %d\n\r",res);
		CA310_Cal0();//调零
	#endif
    
	/*gamma配置初始�??*/
	gammaConfigInitHBM();	
	gammaConfigInitIdle();	
	gammaConfigInitNormal();
	// gammaConfigInitNormal3();
	// gammaConfigInitNormal2();
	// gammaConfigInitNormal4();		
	_DEBUG("%s\r\n",gamma_version());
}


/**
 * 函数名：Gamma_Correction
 * 描述：gamma调试接口函数
 * 参数�??
 * Pid_Net�?? 该选项选择调节gamma方式
 *			 =1 -执行模型调试
 *			 =0 -执行PID调试
 * Get_Data: 该选项配合PID调节使用
 * 			 =0 -不执行跑模型数据操作
 * 			 =1 -执行跑模型数据（配合开启PID调节，则执行完本次PID调节后讲本次的初值进行跑模型数据�??
 *			 =2 -如果已经有初值，存放到new_gamma_def.h的XXX模式_DEF_GAMMA_RGB_REG里，则直接执行跑数据不进行PID调节
 * RUN_IDLE: 该选项及其它相关模式选项选择开启调节的模式（需根据项目需要自行增减RUN_XXX参数�??
 *			 =1 -执行调试
 *			 =0 -不执行调�??
 * dev: 	 该参数为打印串口对象
*/
int Gamma_Correction(int Pid_Net, int Get_Data, int RUN_IDLE,int RUN_HBM,int RUN_Normal,int RUN_NOR5,int RUN_NOR4,int RUN_NOR3,int RUN_NOR2,sky_comDriver *dev)
{
		int ret;
    int ff;
    float W_Lv[2];
		XYLvTypeDef bl;
	
    my_frame = White_Frame;
    Rename_LOG(Code_file);
    sprintf(Code_file,scancode);
		
	//******************调试Idle模式***********************
	if(RUN_IDLE)
	{
		ff=MeasureTimeStart_ms();	
		_DEBUG("IDLE\n");
		printf("IDLE\n");
		go_idle();
		Frame_Switch(my_frame);//白色画面
		Delay_ms(200);
        
		if(Get_Data == 2)
				Get_Model(&GammaConfig_Idle, Get_Data, dev);
		else
		{
				#ifdef SDCard_Log
				sprintf(array, "RUN_IDLE\r\nID_Number,MODE_LEVE,X,Y,Lv,Gamma\r\n");	
				strcat(save_log, array);
				// SDCard_Printf(Code_file,"RUN_IDLE\r\nID_Number,MODE_LEVE,X,Y,Lv,Gamma\r\n");	
				#endif
				#ifdef Pcui_Log
				pcui_writeFileContent(Code_file,"RUN_IDLE\r\n");	
				#endif						
            Pid_Net == 1 ? (ret=run_gammaN(&GammaConfig_Idle,dev)) : (ret = run_gammaP(&GammaConfig_Idle,0.3,dev));
		}
		if(ret<0)
		{
			GammaConfig_Idle.gammaRegWriteAll(GammaConfig_Idle.gammaDefReg);
			return ret; //直接返回，不进行下一步了
		}else
		{
			GammaConfig_Idle.gammaRegWriteAll(GammaConfig_Idle.gammaSetReg);
            if(Get_Data ==2)
                GammaConfig_Idle.gammaRegWriteAll(GammaConfig_Idle.gammaDefReg);
		
			ff=GetMeasureTime_ms(ff);
			#ifdef Pcui_Log
			pcui_writeFileContent(Code_file,"AOD(ms):,%d\r\n",ff);	
			#endif	
			printf("AOD:%dms\r\n",ff);
		}
//			Check_Idle51_1D_L32();
	}

	
	//******************调试normal2模式***********************
	if(RUN_NOR2)
	{			
		ff=MeasureTimeStart_ms();
		
		_DEBUG("RUN_NOR2\n");
//		MIPI_WR(0x15,0xFE,0x40);	
//		MIPI_WR(0x15,0x1F,0x01);	

		
		MIPI_WR(0x15,0xFE,0x00);////退出AOD
		MIPI_WR(0x05,0x38);
		MIPI_WR(0x15,0x51,0x27);
	
		MIPI_WR(0x15,0xFE,0x00);/////退出高�??
		MIPI_WR(0x15,0x66,0x00);
//		go_normal2();
//		MIPI_WR(0x15,0xFE,0x51);
		
		SwitchFrame(0);//白色画面
		Delay_ms(500);
        
		if(Get_Data == 2)
				Get_Model(&GammaConfig_Normal2, Get_Data, dev);
		else		
		{
				#ifdef SDCard_Log
				SDCard_Printf(Code_file,"RUN_NOR2\r\n");	
				#endif
				#ifdef Pcui_Log
				pcui_writeFileContent(Code_file,"RUN_NOR2\r\n");	
				#endif	
            Pid_Net == 1 ? (ret=run_gammaN(&GammaConfig_Normal2,dev)) : (ret = run_gammaP(&GammaConfig_Normal2,0.3,dev));
		}
		if(ret<0)
		{
			GammaConfig_Normal2.gammaRegWriteAll(GammaConfig_Normal2.gammaDefReg);
			return ret; //直接返回，不进行下一步了
		}else
		{
//				GammaConfig_Normal2.gammaSetReg[5] = GammaConfig_Normal2.gammaSetReg[6]-1;
//				if(GammaConfig_Normal2.gammaSetReg[5]<0) GammaConfig_Normal2.gammaSetReg[5] = 0;
//				GammaConfig_Normal2.gammaSetReg[5+NORMAL_DEF_GAMMA_ORDER_NUM] = GammaConfig_Normal2.gammaSetReg[6+NORMAL_DEF_GAMMA_ORDER_NUM]-1;
//				GammaConfig_Normal2.gammaSetReg[5+NORMAL_DEF_GAMMA_ORDER_NUM*2] = GammaConfig_Normal2.gammaSetReg[6+NORMAL_DEF_GAMMA_ORDER_NUM*2]-1;
				
				GammaConfig_Normal2.gammaSetReg[4] = GammaConfig_Normal2.gammaSetReg[5]-1;
				if(GammaConfig_Normal2.gammaSetReg[4]<0) GammaConfig_Normal2.gammaSetReg[4] = 0;
				GammaConfig_Normal2.gammaSetReg[4+NORMAL_DEF_GAMMA_ORDER_NUM] = GammaConfig_Normal2.gammaSetReg[5+NORMAL_DEF_GAMMA_ORDER_NUM]-1;
				GammaConfig_Normal2.gammaSetReg[4+NORMAL_DEF_GAMMA_ORDER_NUM*2] = GammaConfig_Normal2.gammaSetReg[5+NORMAL_DEF_GAMMA_ORDER_NUM*2]-1;
//				
				GammaConfig_Normal2.gammaSetReg[3] = GammaConfig_Normal2.gammaSetReg[4] - 1;
				GammaConfig_Normal2.gammaSetReg[3+NORMAL_DEF_GAMMA_ORDER_NUM] = GammaConfig_Normal2.gammaSetReg[4+NORMAL_DEF_GAMMA_ORDER_NUM]-1;
				GammaConfig_Normal2.gammaSetReg[3+NORMAL_DEF_GAMMA_ORDER_NUM*2] = GammaConfig_Normal2.gammaSetReg[4+NORMAL_DEF_GAMMA_ORDER_NUM*2]-1;        
				
			GammaConfig_Normal2.gammaSetReg[2] = GammaConfig_Normal2.gammaSetReg[3]- 1;
			if(GammaConfig_Normal2.gammaSetReg[2]<0) GammaConfig_Normal2.gammaSetReg[2] = 0;
			GammaConfig_Normal2.gammaSetReg[2+NORMAL_DEF_GAMMA_ORDER_NUM] = GammaConfig_Normal2.gammaSetReg[3+NORMAL_DEF_GAMMA_ORDER_NUM] - 1;
			GammaConfig_Normal2.gammaSetReg[2+NORMAL_DEF_GAMMA_ORDER_NUM*2] = GammaConfig_Normal2.gammaSetReg[3+NORMAL_DEF_GAMMA_ORDER_NUM*2] - 1;		

			GammaConfig_Normal2.gammaSetReg[1] = 0;
			if(GammaConfig_Normal2.gammaSetReg[1]<0) GammaConfig_Normal2.gammaSetReg[1] = 0;
			GammaConfig_Normal2.gammaSetReg[1+NORMAL_DEF_GAMMA_ORDER_NUM] = GammaConfig_Normal2.gammaSetReg[2+NORMAL_DEF_GAMMA_ORDER_NUM] -1;
			GammaConfig_Normal2.gammaSetReg[1+NORMAL_DEF_GAMMA_ORDER_NUM*2] = 0;
				


//			GammaConfig_Normal2.gammaSetReg[1] = 0;
//			GammaConfig_Normal2.gammaSetReg[1+NORMAL_DEF_GAMMA_ORDER_NUM] = 0;
//			GammaConfig_Normal2.gammaSetReg[1+NORMAL_DEF_GAMMA_ORDER_NUM*2] = 0;				
			
			GammaConfig_Normal2.gammaRegWriteAll(GammaConfig_Normal2.gammaSetReg);
            if(Get_Data ==2)
                GammaConfig_Normal2.gammaRegWriteAll(GammaConfig_Normal2.gammaDefReg);
		}
//			/******1F40h复位******/
//			MIPI_WR(0x15,0xFE,0x40);
//			MIPI_WR(0x15,0x1F,0x05);
//			MIPI_WR(0x15,0xFE,0x00);
//			/**********************/
			ff=GetMeasureTime_ms(ff);
			pcui_writeFileContent(Code_file,"nor2(ms):,%d\r\n",ff);
			printf("nor2:%dms\r\n",ff);
			
//			Check_Nor51_1D_L32();
	}


	//******************调试normal模式***********************
	if(RUN_Normal)
	{
		ff=MeasureTimeStart_ms();
		
		_DEBUG("RUN_Normal\n");
		printf("RUN_Normal\n");
		go_normal();
		Frame_Switch(my_frame);//白色画面
		Delay_ms(200);
        
		if(Get_Data == 2)
				Get_Model(&GammaConfig_Normal, Get_Data, dev);
		else
		{
			#ifdef SDCard_Log
			sprintf(array, "RUN_Normal\r\nID_Number,MODE_LEVE,X,Y,Lv,Gamma\r\n");
			strcat(save_log, array);
			// SDCard_Printf(Code_file,"RUN_Normal\r\nID_Number,MODE_LEVE,X,Y,Lv,Gamma\r\n");	
			#endif
			#ifdef Pcui_Log
			pcui_writeFileContent(Code_file,"RUN_Normal\r\n");	
			#endif	
            Pid_Net == 1 ? (ret=run_gammaN(&GammaConfig_Normal,dev)) : (ret = run_gammaP(&GammaConfig_Normal,0.3,dev));
		}
		if(ret<0)
		{
			GammaConfig_Normal.gammaRegWriteAll(GammaConfig_Normal.gammaDefReg);
			return ret; //直接返回，不进行下一步了
		}else
		{
			GammaConfig_Normal.gammaRegWriteAll(GammaConfig_Normal.gammaSetReg);
            if(Get_Data ==2)
                GammaConfig_Normal.gammaRegWriteAll(GammaConfig_Normal.gammaDefReg);
		}
		ff=GetMeasureTime_ms(ff);
		#ifdef Pcui_Log
		pcui_writeFileContent(Code_file,"NOR(ms):,%d\r\n",ff);	
		#endif	
		printf("NOR:%dms\r\n",ff);
	}

	
  //******************调试HBM模式***********************
	if(RUN_HBM)
	{
		ff=MeasureTimeStart_ms();	
		_DEBUG("HBM\n");
		printf("HBM\n");
		go_hbm();
		Frame_Switch(my_frame);//白色画面
		Delay_ms(200);
        
		if(Get_Data == 2)
				Get_Model(&GammaConfig_HBM, Get_Data, dev);
		else
		{
				#ifdef SDCard_Log
				sprintf(array, "RUN_HBM\r\nID_Number,MODE_LEVE,X,Y,Lv,Gamma\r\n");	
				strcat(save_log, array);
				// SDCard_Printf(Code_file,"RUN_HBM\r\nID_Number,MODE_LEVE,X,Y,Lv,Gamma\r\n");	
				#endif
				#ifdef Pcui_Log
				pcui_writeFileContent(Code_file,"RUN_HBM\r\n");	
				#endif	
            Pid_Net == 1 ? (ret=run_gammaN(&GammaConfig_HBM,dev)) : (ret = run_gammaP(&GammaConfig_HBM,0.1,dev));
		}
		if(ret<0)
		{
			GammaConfig_HBM.gammaRegWriteAll(GammaConfig_HBM.gammaDefReg);
			return ret; //直接返回，不进行下一步了
		}else
		{
			GammaConfig_HBM.gammaRegWriteAll(GammaConfig_HBM.gammaSetReg);
            if(Get_Data ==2)
                GammaConfig_HBM.gammaRegWriteAll(GammaConfig_HBM.gammaDefReg);
		}	

		ff=GetMeasureTime_ms(ff);
		#ifdef Pcui_Log
		pcui_writeFileContent(Code_file,"HBM(ms):,%d\r\n",ff);    
		#endif	
		printf("HBM:%dms\r\n",ff);
	}
	
		
 //******************调试normal3模式***********************
	if(RUN_NOR3)
	{
		ff=MeasureTimeStart_ms();	
		_DEBUG("RUN_NOR3\n");
		go_PLC();
		SwitchFrame(0);//白色画面
		Delay_ms(500);
        
		if(Get_Data == 2)
				Get_Model(&GammaConfig_Normal3, Get_Data, dev);
		else
		{
//				#ifdef SDCard_Log
//				SDCard_Printf(Code_file,"RUN_NOR3\r\n");	
//				#endif
				#ifdef Pcui_Log
				pcui_writeFileContent(Code_file,"RUN_NOR3\r\n");	
				#endif	
            Pid_Net == 1 ? (ret=run_gammaN(&GammaConfig_Normal3,dev)) : (ret = run_gammaP(&GammaConfig_Normal3,0.1,dev));
		}
		if(ret<0)
		{
			GammaConfig_Normal3.gammaRegWriteAll(GammaConfig_Normal3.gammaDefReg);
			return ret; //直接返回，不进行下一步了
		}else
		{
			GammaConfig_Normal3.gammaRegWriteAll(GammaConfig_Normal3.gammaSetReg);
            if(Get_Data ==2)
                GammaConfig_Normal3.gammaRegWriteAll(GammaConfig_Normal3.gammaDefReg);
		}
		
		ff=GetMeasureTime_ms(ff);
		pcui_writeFileContent(Code_file,"Nor3(ms):,%d\r\n",ff);    
		printf("Nor3:%dms\r\n",ff);		
	}
 //******************调试normal4模式***********************
	if(RUN_NOR4)
	{
		ff=MeasureTimeStart_ms();	
		
		_DEBUG("RUN_NOR4\n");
		go_PLC();
		SwitchFrame(0);//白色画面
		Delay_ms(500);
        
		if(Get_Data == 2)
				Get_Model(&GammaConfig_Normal4, Get_Data, dev);
		else
		{
//				#ifdef SDCard_Log
//				SDCard_Printf(Code_file,"RUN_NOR4\r\n");	
//				#endif
				#ifdef Pcui_Log
				pcui_writeFileContent(Code_file,"RUN_NOR4\r\n");	
				#endif	
          //  Pid_Net == 1 ? (ret=run_gammaN(&GammaConfig_Normal4,dev)) : (ret = run_gammaP(&GammaConfig_Normal4,0.1,dev));
			 ret = run_gammaP(&GammaConfig_Normal4,0.1,dev);
		}
		if(ret<0)
		{
			GammaConfig_Normal4.gammaRegWriteAll(GammaConfig_Normal4.gammaDefReg);
			return ret; //直接返回，不进行下一步了
		}else
		{
			GammaConfig_Normal4.gammaRegWriteAll(GammaConfig_Normal4.gammaSetReg);
            if(Get_Data ==2)
                GammaConfig_Normal4.gammaRegWriteAll(GammaConfig_Normal4.gammaDefReg);
		}
		
		ff=GetMeasureTime_ms(ff);
		pcui_writeFileContent(Code_file,"Nor4(ms):,%d\r\n",ff);    
		printf("Nor4:%dms\r\n",ff);		
	}
		
    //*************************************获取模型******************************************     
    if(Get_Data == 1)
    {        
        //******************获取idle模型***********************
        if(RUN_IDLE)
        {
            _DEBUG("IDLE\n");
            go_idle();
            Frame_Switch(my_frame);//白色画面
            Delay_ms(500);
            Get_Model(&GammaConfig_Idle, Get_Data, dev);
            GammaConfig_Idle.gammaRegWriteAll(GammaConfig_Idle.gammaSetReg);
        }
        //******************获取HBM模型***********************
        if(RUN_HBM)
        {
            _DEBUG("HBM\n");
            go_hbm();
            Frame_Switch(my_frame);//白色画面
            Delay_ms(500);
            Get_Model(&GammaConfig_HBM, Get_Data, dev);	
            GammaConfig_HBM.gammaRegWriteAll(GammaConfig_HBM.gammaSetReg);
        }

        //******************获取Normal1模型***********************
        if(RUN_Normal)
        {
            _DEBUG("RUN_Normal\n");
            go_normal();
            Frame_Switch(my_frame);//白色画面
            Delay_ms(500);
            Get_Model(&GammaConfig_Normal, Get_Data, dev);
            GammaConfig_Normal.gammaRegWriteAll(GammaConfig_Normal.gammaSetReg);
        }

        //******************获取Normal3模型***********************
        if(RUN_NOR3)
        {
            _DEBUG("RUN_NOR3\n");
            go_normal3();
            SwitchFrame(0);//白色画面
            Delay_ms(500);
            Get_Model(&GammaConfig_Normal3, Get_Data, dev);
            GammaConfig_Normal3.gammaRegWriteAll(GammaConfig_Normal3.gammaSetReg);
        }

        //******************获取Normal2模型***********************
        if(RUN_NOR2)
        {
            _DEBUG("RUN_NOR2\n");
            go_normal2();
            SwitchFrame(0);//白色画面
            Delay_ms(500);
            Get_Model(&GammaConfig_Normal2, Get_Data, dev);
            GammaConfig_Normal2.gammaRegWriteAll(GammaConfig_Normal2.gammaSetReg);
        }    
    }

	//返回标准normal，如果你的normal有多个模式，请根据需要自行修�??
	go_normal();

	return ret;
}

/**/
#include "sd_file.h"
extern int getTargetXYZ(uint8_t num,NEW_GammaConfigStruct *gammaConfig,double date[3]);
extern int checkGamma(uint8_t num, NEW_GammaConfigStruct *gammaConfig, double xyz[3],sky_comDriver *dev);
extern double dElab(double XYZ[3],double txyz[3]);
extern float loga(float a, float b);

static int cmp_historyData(int Reg[], int historyData[][3], int argc);
static void getNotRepeatReg(int NowReg[], int historyData[][3]);

/**/
int run_gammaP(NEW_GammaConfigStruct *gammaConfig, float P, sky_comDriver *dev)
{
	uint16_t ref_reg[3];
    int out_reg[3];
	double target_xyz[3];
	double measure_xyz[3];

	/*历史数据
	 *遍历历史次数内N组数据与当前数据比较，确保当前数据不与历史重�??
	 *若重复则通过+1 +0 -1等数据对重复数据进行微调
	 *historyData[n1][n2]
	 *@parm n1:决定几组历史数据存储
	 *@parm n2:rgb*/
    int historyData[3][3] = {
        {0, 0, 0},
        {0, 0, 0},
        {0, 0, 0}
    };
    int historyN = 0;
	
	float dE;	
	float gammaMes;
	float errData[3];
	int PN;//系数变化因子
	int ff;//字符串长�??
	char str_data[1024];//print info string
    char filename[128];//Log file name
	
	// init data register
	memset(str_data, 0, sizeof str_data);
	memset(filename, 0, sizeof filename);
	PN = 1;

	// Log print, save to sd
    if(fun3)
    {    
        sprintf(filename,"%s", LOG_NAME); 
        SDCard_Printf(filename, Log_hard);
        SDCard_Printf(filename, "\r\n");
    }
    
    //全写清空gamma寄存�??
    memset(gammaConfig->gammaSetReg, 0, sizeof(gammaConfig->gammaSetReg));
    gammaConfig->gammaRegWriteAll(gammaConfig->gammaSetReg);
    
	// 获取初�?
	out_reg[0] = gammaConfig->gammaDefReg[1*gammaConfig->Order-1];
	out_reg[1] = gammaConfig->gammaDefReg[2*gammaConfig->Order-1];
	out_reg[2] = gammaConfig->gammaDefReg[3*gammaConfig->Order-1];
	ref_reg[0] = gammaConfig->gammaDefReg[1*gammaConfig->Order-1];
	ref_reg[1] = gammaConfig->gammaDefReg[2*gammaConfig->Order-1];
	ref_reg[2] = gammaConfig->gammaDefReg[3*gammaConfig->Order-1];
    
	// �??255阶开始调
	for (int i = gammaConfig->Order -1; i > 0; i--)
	{
        gammaConfig->num = i;//当前是多少阶
		getTargetXYZ(i,gammaConfig,target_xyz);//获取该阶目标xyz
		if(fun2)
			gammaConfig->gammaRegWrite(gammaConfig->Order -1,ref_reg);//255�??
		else
		{
			uint8_t gray = gammaConfig->OGammaConfig[i].level;
			uint16_t zero[255];
			memset(zero, 0, sizeof zero);
			gammaConfig->gammaRegWriteAll(zero);			
        	Img_Full(gray, gray, gray); 
			gammaConfig->gammaRegWrite(gammaConfig->num, ref_reg);//写对应绑点寄存器
		}
			
		gammaConfig->readLv(gammaConfig->OGammaConfig[i].readD,gammaConfig->OGammaConfig[i].measureXYLv,measure_xyz);//读n次数据时间做延时 数据无效
		Delay_ms(gammaConfig->OGammaConfig[i].delay);		//延时
		gammaConfig->readLv(gammaConfig->OGammaConfig[i].readN,gammaConfig->OGammaConfig[i].measureXYLv,measure_xyz);//读镜头数�??
		
		// 若de卡控�??0或xy卡控�??0,不卡de
		if(fabs(gammaConfig->OGammaConfig[i].De) < 1e-6 || fabs(gammaConfig->OGammaConfig[i].dxy) < 1e-6 )
            dE = 0;
        else
            dE = dElab(measure_xyz,target_xyz);

		// now gamma
		if(gammaConfig->OGammaConfig[i].level != 255 && gammaConfig->OGammaConfig[i].level != 0)
			gammaMes = loga(measure_xyz[1]/(gammaConfig->maxlv*1.0),(gammaConfig->OGammaConfig[i].level*1.0)/255.0); //2.2
		else
			gammaMes = 0;

        //Debug info
		if(fun1){
            ff = sprintf(str_data,"\r\nlevel       : %d\n",gammaConfig->OGammaConfig[i].level);
            if(dev != 0)
            {
                dev->write(dev,(uint8_t*)str_data,ff);			
            }
            
            ff = sprintf(str_data,	
			"Delta E     : %.4f(de %.2f)\
             \r\ntargetxyz   : %.4f(x)     \t%0.4f(y)       \t%0.4f(z)\
             \r\nmesaurexyz  : %.4f(x)     \t%0.4f(y)       \t%0.4f(z)\
             \r\ntargetxyL   : %.2fx(%.4f) \t%.2fy(%.4f)      \t%.4f(Lmax %.4f Lmin %.4f)\
             \r\nmesaurexyL  : %.4f(x)     \t%.4f(y)          \t%.4f(L)\
             \r\nnow rgbReg  : %d(r)          \t%d(g)            \t%d(b)\
             \r\nnow gamma   : %.4f(%.2f+-%.2f)\n",
            dE, gammaConfig->OGammaConfig[i].De,
            target_xyz[0], target_xyz[1], target_xyz[2],
            measure_xyz[0], measure_xyz[1], measure_xyz[2],
			gammaConfig->OGammaConfig[i].targetX, gammaConfig->OGammaConfig[i].dxy, gammaConfig->OGammaConfig[i].targetY, gammaConfig->OGammaConfig[i].dxy, 
			gammaConfig->OGammaConfig[i].targetLv, gammaConfig->OGammaConfig[i].targetlvMax, gammaConfig->OGammaConfig[i].targetlvMin,			
            gammaConfig->OGammaConfig[i].measureXYLv[0],gammaConfig->OGammaConfig[i].measureXYLv[1],gammaConfig->OGammaConfig[i].measureXYLv[2],
            ref_reg[0],ref_reg[1],ref_reg[2],
			gammaMes, gammaConfig->gamma, gammaConfig->OGammaConfig[i].dGamma);
            if(dev != 0)
            {
                dev->write(dev,(uint8_t*)str_data,ff);			
            }
        }
        
		// Check
        if ( (checkGamma(i,gammaConfig,measure_xyz,dev) > 0) && (dE <= gammaConfig->OGammaConfig[i].De))
        {
            if(fun1){
				ff = sprintf(str_data,"\ntimes: %d ,leave: %d  ok\n",i,gammaConfig->OGammaConfig[i].level);
				if(dev != 0)
				{
					dev->write(dev,(uint8_t*)str_data,ff);			
				}
            }   

			//   自定义打印Log格式：采用函数传参，用户自定义打印函�??
            if(fun3){
				Log_print(gammaConfig, i, 'Y', filename);
            }
            
            for(int i = 0; i < 3; i++){            
				//判断RGB不是255阶的RGB时，设置的RGB不能超过上一阶设置的RGB�??
				if (gammaConfig->num != gammaConfig->Order-1)
				{
					if(ref_reg[i] >= gammaConfig->gammaSetReg[(gammaConfig->num+1) +gammaConfig->Order*i])
						ref_reg[i] = gammaConfig->gammaSetReg[(gammaConfig->num+1)+gammaConfig->Order*i]-1;
				}
            }
            
            //基于255阶调完的亮度效果作gamma2.2的基�??
            if(i == gammaConfig->Order -1){
                gammaConfig->maxlv = measure_xyz[1];
            }
            //保存调好的rgb数据
            for(uint8_t g=0;g<3;g++)
            {
                gammaConfig->gammaSetReg[i+gammaConfig->Order*g]=ref_reg[g];					
            }
            continue;
        }
        else if(dE>gammaConfig->OGammaConfig[i].De)
        {
            ff = sprintf(str_data,"check       : De NG\r\n");
            if(dev != 0)
            {
                dev->write(dev,(uint8_t*)str_data,ff);			
            }            
        }
		
		//循环调N次调好退�??
		for(uint8_t k=0;k<gammaConfig->OGammaConfig[i].tuningN;k++)
		{
			gammaConfig->ci = k;
            for(uint8_t j=0;j<3;j++)
            {
                errData[j] = target_xyz[j]-measure_xyz[j];
                errData[j] *= P;
            }
            while(1)
            {
                int res = 0, re = 0;
                for(uint8_t j=0;j<3;j++)
                {
                    if (errData[j] < 0)
                        PN += 1;
                    else
                        PN -= 5;

                    if (PN > 12)
                    {
                        PN = 12;
                        P *= 1.03f;
                        if (P > 200)
                            P = 200;
                    }
                    
                    if (errData[j] > 50)
                        errData[j] = 50;
                    if (errData[j] < -50)
                        errData[j] = -50;
                    res = round(errData[j]);
                    if(res != 0)
                        re++;

                        out_reg[j]+=lround(errData[j]);
                    
                    if (out_reg[j]>gammaConfig->RGB_MAX)
                        out_reg[j] =gammaConfig->RGB_MAX;
                    
                    if (out_reg[j]<gammaConfig->RGB_MIN)
                        out_reg[j] =gammaConfig->RGB_MIN;    
                    
                    if(out_reg[j]<0)
                        out_reg[j] = 0;
                    
              /*       if(fun1){
                    ff = sprintf(str_data,"\nerrData%d:%0.4f, ",j,errData[j]);
                    if(dev != 0)
                    {
                        dev->write(dev,(uint8_t*)str_data,ff);			
                    }
                    } */
            
                }
                if(re != 0)
                    break;
                else
                {
                    for(int i = 0; i < 3; i++)
                    {
                        errData[i] *= 1.2f;
                    }
                }
            
            }
            
            for(int i = 0; i < 3; i++){            
				//判断RGB不是255阶的RGB时，设置的RGB不能超过上一阶设置的RGB�??
				if (gammaConfig->num != gammaConfig->Order-1)
				{
					if(out_reg[i] >= gammaConfig->gammaSetReg[(gammaConfig->num+1) +gammaConfig->Order*i])
						out_reg[i] = gammaConfig->gammaSetReg[(gammaConfig->num+1)+gammaConfig->Order*i]-1;
				}
            }
   
            
            // 历史数据处理
            getNotRepeatReg(out_reg, historyData);
            for(int i = 0; i < 3; i++){
                ref_reg[i] = out_reg[i];    
                historyData[historyN][i] = out_reg[i];
            }
            if(++historyN > 2)
                historyN = 0;
			
			if(fun2)
				gammaConfig->gammaRegWrite(gammaConfig->Order -1,ref_reg);//255 write
			else
			{
				uint8_t gray = gammaConfig->OGammaConfig[i].level;
				uint16_t zero[255];
				memset(zero, 0, sizeof zero);
				gammaConfig->gammaRegWriteAll(zero);			
				Img_Full(gray, gray, gray); 
				gammaConfig->gammaRegWrite(gammaConfig->num, ref_reg);//写对应绑点寄存器
			}
			
            gammaConfig->readLv(gammaConfig->OGammaConfig[i].readD,gammaConfig->OGammaConfig[i].measureXYLv,measure_xyz);		
			Delay_ms(gammaConfig->OGammaConfig[i].delay);		
            gammaConfig->readLv(gammaConfig->OGammaConfig[i].readN,gammaConfig->OGammaConfig[i].measureXYLv,measure_xyz);

            if(fabs(gammaConfig->OGammaConfig[i].De) < 1e-6 || fabs(gammaConfig->OGammaConfig[i].dxy) < 1e-6 )
                dE = 0;
            else
                dE = dElab(measure_xyz,target_xyz);
			
            // now gamma
            if(gammaConfig->OGammaConfig[i].level != 255 && gammaConfig->OGammaConfig[i].level != 0)
                gammaMes = loga(measure_xyz[1]/(gammaConfig->maxlv*1.0),(gammaConfig->OGammaConfig[i].level*1.0)/255.0); //2.2
            else
                gammaMes = 0;

            if(fun1){				
				ff = sprintf(str_data,	
				"\
				 \r\ntimes       : %d\
				 \r\nP           : %0.4f\
				 \r\nDelta E     : %.4f(de %.2f)\
				 \r\ntargetxyz   : %.4f(x)     \t%0.4f(y)       \t%0.4f(z)\
				 \r\nmesaurexyz  : %.4f(x)     \t%0.4f(y)       \t%0.4f(z)\
				 \r\ntargetxyL   : %.2fx(%.4f) \t%.2fy(%.4f)      \t%.4f(Lmax %.4f Lmin %.4f)\
				 \r\nmesaurexyL  : %.4f(x)     \t%.4f(y)          \t%.4f(L)\
				 \r\nnow rgbReg  : %d(r)          \t%d(g)            \t%d(b)\
				 \r\nnow gamma   : %.4f(%.2f+-%.2f)\n",
				k,
				P,
				dE, gammaConfig->OGammaConfig[i].De,
				target_xyz[0], target_xyz[1], target_xyz[2],
				measure_xyz[0], measure_xyz[1], measure_xyz[2],
				gammaConfig->OGammaConfig[i].targetX, gammaConfig->OGammaConfig[i].dxy,gammaConfig->OGammaConfig[i].targetY, gammaConfig->OGammaConfig[i].dxy, 
				gammaConfig->OGammaConfig[i].targetLv, gammaConfig->OGammaConfig[i].targetlvMax, gammaConfig->OGammaConfig[i].targetlvMin,			
				gammaConfig->OGammaConfig[i].measureXYLv[0],gammaConfig->OGammaConfig[i].measureXYLv[1],gammaConfig->OGammaConfig[i].measureXYLv[2],
				ref_reg[0],ref_reg[1],ref_reg[2],
				gammaMes, gammaConfig->gamma, gammaConfig->OGammaConfig[i].dGamma);
				if(dev != 0)
				{
					dev->write(dev,(uint8_t*)str_data,ff);			
				}			
            }

				
			if ( (checkGamma(i,gammaConfig,measure_xyz,dev) > 0) && (dE <= gammaConfig->OGammaConfig[i].De))
			{
                if(fun1){
				ff = sprintf(str_data,"\nnum: %d ,leave: %d  ok\n",i,gammaConfig->OGammaConfig[i].level);
				if(dev != 0)
				{
					dev->write(dev,(uint8_t*)str_data,ff);			
				}
                }
                if(fun3){
					Log_print(gammaConfig, i, 'Y', filename);
                }
                
                //基于255阶调完的亮度效果作gamma2.2的基�??
                if(i == gammaConfig->Order -1){
                    gammaConfig->maxlv = measure_xyz[1];
                }
                //保存调好的rgb数据
				for(uint8_t g=0; g<3; g++)
				{
					gammaConfig->gammaSetReg[i+gammaConfig->Order*g] = ref_reg[g];					
				}           
				break;
			}
            else if(dE>gammaConfig->OGammaConfig[i].De)
            {
                ff = sprintf(str_data,"check       : De NG\r\n");
                if(dev != 0)
                {
                    dev->write(dev,(uint8_t*)str_data,ff);			
                }            
            }
			 
			if(k > gammaConfig->OGammaConfig[i].tuningN-2)
            {            
                if(fun1){
					ff = sprintf(str_data,"\nnum: %d ,leave: %d  NG\n",i,gammaConfig->OGammaConfig[i].level);
					if(dev != 0)
					{
						dev->write(dev,(uint8_t*)str_data,ff);			
					}
                }
                if(fun3){
					Log_print(gammaConfig, i, 'N', filename);
                }
                return -1;
            }
		}
	}
    
    if(fun1){
		_DEBUG("123456");
    ff = sprintf(str_data, "\r\n%s:\r\n",gammaConfig->displaymode);
        dev->write(dev, (uint8_t*)str_data, ff);
		SDCard_Printf(Code_file,str_data);	
    for(int i =1; i < gammaConfig->Order*3+1; i++){          
        ff = sprintf(str_data, "%d,",gammaConfig->gammaSetReg[i-1]);
        dev->write(dev, (uint8_t*)str_data, ff); 
		SDCard_Printf(Code_file,str_data);			
        if(i % gammaConfig->Order == 0 && i != 0){        
            ff = sprintf(str_data, "\r\n");
            dev->write(dev, (uint8_t*)str_data, ff); 
			SDCard_Printf(Code_file,str_data);				
        }
    }
    }
	return 1;	
}



static int cmp_historyData(int Reg[], int historyData[][3], int argc)
{
    for(int i = 0; i < argc; i++)
    {
        if(Reg[0] != historyData[i][0])
            ;
        else
            return 1;
    }
    for(int i = 0; i < argc; i++)
    {
        if(Reg[1] != historyData[i][1])
            ;
        else
            return 1;
    }
    for(int i = 0; i < argc; i++)
    {
        if(Reg[2] != historyData[i][2])
            ;
        else return 1;
    }
    return 0;
}

// 实现：将十进制整数转换并拆分成两个字�??
void ConvertAndSplit(int decimal, unsigned char* high, unsigned char* low) {
    unsigned int hexValue = (unsigned int)decimal;

    *high = (hexValue >> 8) & 0xFF; // 取高8�??
    *low  = hexValue & 0xFF;        // 取低8�??
}

static void getNotRepeatReg(int NowReg[], int historyData[][3])
{
    int l[] = {0, 1, -1};
    int Reg[] = {0, 0, 0};
    for(int i = 0; i < 3; i++)
        for(int j = 0; j < 3; j++)
            for(int k = 0; k < 3; k++)
            {
                Reg[0] = NowReg[0] + l[i];
                Reg[1] = NowReg[1] + l[j];
                Reg[2] = NowReg[2] + l[k];
                int n = cmp_historyData(Reg, historyData, 3);
                if(n < 1)
                {
                    if(Reg[0] == -1)
                        Reg[0] = 0;
                    if(Reg[1] == -1)
                        Reg[1] = 0;
                    if(Reg[2] == -1)
                        Reg[2] = 0;
                    
                    NowReg[0] = Reg[0];
                    NowReg[1] = Reg[1];
                    NowReg[2] = Reg[2];
                    return;
                }
            }
}

/**/
// r->b->g
typedef enum{
	 _green  = 0,
	_blue,
	_red,
}_rgb_state;

static char _get_rgb_char(uint8_t _rgb)
{
	if(_rgb == _red){
		return 'R';
	}
	if(_rgb == _blue){
		return 'B';
	}
	if(_rgb == _green){
		return 'G';
	}
}
	
static int _getTargetXYZ(uint8_t num,NEW_GammaConfigStruct *gammaConfig,double date[3], uint8_t _rgb)
{
	float lv, targetX, targetY;
	uint8_t level;
	level = gammaConfig->OGammaConfig[num].level;
	lv=pow(level/255.0,2.2)*gammaConfig->maxlv;
	/*****************************awe******************* */
	
	/*
	if(level == 19){
		if(_rgb == _red){
			targetX = 0.268f;
			targetY = 0.315f;
		}
		if(_rgb == _blue){
			targetX = 0.294f;
			targetY = 0.315f;
		}
		if(_rgb == _green){
			targetX = 0.300f;
			targetY = 0.313f;
		}
	}

	if(level == 15){
		if(_rgb == _red){
			targetX = 0.247f;
			targetY = 0.315f;
		}
		if(_rgb == _blue){
			targetX = 0.292f;
			targetY = 0.315f;
		}
		if(_rgb == _green){
			targetX = 0.300f;
			targetY = 0.313f;
		}
	}

	if(level == 11){
		if(_rgb == _red){
			targetX = 0.225f;
			targetY = 0.315f;
		}
		if(_rgb == _blue){
			targetX = 0.285f;
			targetY = 0.315f;
		}
		if(_rgb == _green){
			targetX = 0.300f;
			targetY = 0.312f;
		}
	}
	
	
	*/

	if(level == 7){
		if(_rgb == _red){
			targetX = 0.300f;
			targetY = 0.322f;  //250511 0.320-0.322
		}
		if(_rgb == _blue){
			targetX = 0.280f;
			targetY = 0.322f;
		}
		if(_rgb == _green){
			targetX = 0.300f;
			targetY = 0.181f; //24.12.3 0.181->0.188  250511 0.188-0.181
		}
	}
	/************************************************ */
	
	date[1] = lv;
	date[0] = lv * targetX/targetY;	
	date[2] = lv * (1-targetX-targetY)/targetY;
	
	Debug_targetX = targetX;
	Debug_targetY = targetY;
	Debug_target(_rgb,level);
	
	return 0;	
}

void PLC_calculation(void)
{
			//调试完之后先读取V25,V26寄存器的�??
	MIPI_WR(0x15,0xFE,0x53);
		
	int i;
	unsigned char ReadCode[NORMAL_DEF_GAMMA_ORDER_NUM*3*2];

	unsigned int V24_CODE_R,V24_CODE_G,V24_CODE_B,V25_CODE_R,V25_CODE_G,V25_CODE_B,V26_CODE_R,V26_CODE_G,V26_CODE_B;
	unsigned int V27_CODE_R_1,V27_CODE_G_1,V27_CODE_B_1,V28_CODE_R_1,V28_CODE_G_1,V28_CODE_B_1,V27_CODE_R_2,V27_CODE_G_2,V27_CODE_B_2,V28_CODE_R_2,V28_CODE_G_2,V28_CODE_B_2;
	unsigned int V27_CODE_R,V27_CODE_G,V27_CODE_B,V28_CODE_R,V28_CODE_G,V28_CODE_B;
	
	int HBM_ReadCode[NORMAL_DEF_GAMMA_ORDER_NUM*3];
	const u8 reg_adrr[] = HBM_DEF_GAMMA_REG_ADRR;	  
	const u8 reg_gray[] = HBM_DEF_GAMMA_LEVEL;
	unsigned int reg_gray_2728[] = {287,319};
	printf("reg_gray_2728:%d,%d\r\n",reg_gray_2728[0],reg_gray_2728[1]);
	Delay_ms(120);			
	for(i=0;i<NORMAL_DEF_GAMMA_ORDER_NUM*3*2;i++)
	{
		SSD2828_DcsReadDT06(reg_adrr[i], 1, &ReadCode[i]);
	}
	for(i=0;i<NORMAL_DEF_GAMMA_ORDER_NUM*3;i++)
	{
		HBM_ReadCode[i]=(((ReadCode[2*i]<<8)&0x300)|(ReadCode[2*i+1]));	
	}
	V24_CODE_R = HBM_ReadCode[24];  //223
	V24_CODE_G = HBM_ReadCode[24+NORMAL_DEF_GAMMA_ORDER_NUM];
	V24_CODE_B = HBM_ReadCode[24+NORMAL_DEF_GAMMA_ORDER_NUM*2];
	
	V25_CODE_R = HBM_ReadCode[25];  //239
	V25_CODE_G = HBM_ReadCode[25+NORMAL_DEF_GAMMA_ORDER_NUM];
	V25_CODE_B = HBM_ReadCode[25+NORMAL_DEF_GAMMA_ORDER_NUM*2];
	
	V26_CODE_R = HBM_ReadCode[26];  //255
	V26_CODE_G = HBM_ReadCode[26+NORMAL_DEF_GAMMA_ORDER_NUM];
	V26_CODE_B = HBM_ReadCode[26+NORMAL_DEF_GAMMA_ORDER_NUM*2];
	
//	int a = reg_gray[25];
	
//	printf("nor_G0:%d,nor_G1:%d\r\n",HBM_ReadCode[27],HBM_ReadCode[28]);
		
		printf("V24-R-G-B:%d,%d,%d\r\n",HBM_ReadCode[24],HBM_ReadCode[24+NORMAL_DEF_GAMMA_ORDER_NUM],HBM_ReadCode[24+NORMAL_DEF_GAMMA_ORDER_NUM*2]);
		printf("V25-R-G-B:%d,%d,%d\r\n",HBM_ReadCode[25],HBM_ReadCode[25+NORMAL_DEF_GAMMA_ORDER_NUM],HBM_ReadCode[25+NORMAL_DEF_GAMMA_ORDER_NUM*2]);
		printf("V26-R-G-B:%d,%d,%d\r\n",HBM_ReadCode[26],HBM_ReadCode[26+NORMAL_DEF_GAMMA_ORDER_NUM],HBM_ReadCode[26+NORMAL_DEF_GAMMA_ORDER_NUM*2]);

		
		//根据V24\25\26的值计算出V27\28的�?
	//	float slope_1 = (HBM_ReadCode[24]-HBM_ReadCode[24])/(HBM_ReadCode[25]-HBM_ReadCode[24]);
		
		//V24\25 外插27
		V27_CODE_R_1 = ((float)(HBM_ReadCode[25]-HBM_ReadCode[24])/(reg_gray[25]-reg_gray[24]))*(reg_gray_2728[0]-reg_gray[25])+HBM_ReadCode[25];
		V27_CODE_G_1 = ((float)(HBM_ReadCode[25+NORMAL_DEF_GAMMA_ORDER_NUM]-HBM_ReadCode[24+NORMAL_DEF_GAMMA_ORDER_NUM])/(reg_gray[25]-reg_gray[24]))*(reg_gray_2728[0]-reg_gray[25])+HBM_ReadCode[25+NORMAL_DEF_GAMMA_ORDER_NUM];	
		V27_CODE_B_1 = ((float)(HBM_ReadCode[25+NORMAL_DEF_GAMMA_ORDER_NUM*2]-HBM_ReadCode[24+NORMAL_DEF_GAMMA_ORDER_NUM*2])/(reg_gray[25]-reg_gray[24]))*(reg_gray_2728[0]-reg_gray[25])+HBM_ReadCode[25+NORMAL_DEF_GAMMA_ORDER_NUM*2];	

		printf("HBM_ReadCode25:%d,HBM_ReadCode24:%d,reg_gray:%d\r\n",HBM_ReadCode[25],HBM_ReadCode[24],reg_gray[25]);
		printf("HBM_ReadCode25:%d,HBM_ReadCode24:%d,reg_gray:%d\r\n",HBM_ReadCode[25],HBM_ReadCode[24],reg_gray[25]);		
		printf("HBM_ReadCode[25]-HBM_ReadCode[24]�??%.4f\r\n",(float)(HBM_ReadCode[25]-HBM_ReadCode[24])/(reg_gray[25]-reg_gray[24]));
		printf("HBM_ReadCode[25]-HBM_ReadCode[24]�??%d\r\n",(reg_gray[25]-reg_gray[24]));
		printf("HBM_ReadCode[25]-HBM_ReadCode[24]�??%d\r\n",(reg_gray_2728[0]-reg_gray[25]));
		printf("V27_CODE_R_1:%d,V27_CODE_G_1:%d,V27_CODE_B_1:%d\r\n",V27_CODE_R_1,V27_CODE_G_1,V27_CODE_B_1);	
		//V24\25 外插28
		V28_CODE_R_1 = ((float)(HBM_ReadCode[25]-HBM_ReadCode[24])/(reg_gray[25]-reg_gray[24]))*(reg_gray_2728[1]-reg_gray[25])+HBM_ReadCode[25];
		V28_CODE_G_1 = ((float)(HBM_ReadCode[25+NORMAL_DEF_GAMMA_ORDER_NUM]-HBM_ReadCode[24+NORMAL_DEF_GAMMA_ORDER_NUM])/(reg_gray[25]-reg_gray[24]))*(reg_gray_2728[1]-reg_gray[25])+HBM_ReadCode[25+NORMAL_DEF_GAMMA_ORDER_NUM];	
		V28_CODE_B_1 = ((float)(HBM_ReadCode[25+NORMAL_DEF_GAMMA_ORDER_NUM*2]-HBM_ReadCode[24+NORMAL_DEF_GAMMA_ORDER_NUM*2])/(reg_gray[25]-reg_gray[24]))*(reg_gray_2728[1]-reg_gray[25])+HBM_ReadCode[25+NORMAL_DEF_GAMMA_ORDER_NUM*2];	
		printf("V28_CODE_R_1:%d,V28_CODE_G_1:%d,V28_CODE_B_1:%d\r\n",V28_CODE_R_1,V28_CODE_G_1,V28_CODE_B_1);
		

		//V25\26 外插27
		V27_CODE_R_2 = ((float)(HBM_ReadCode[26]-HBM_ReadCode[25])/(reg_gray[26]-reg_gray[25]))*(reg_gray_2728[0]-reg_gray[26])+HBM_ReadCode[26];
		V27_CODE_G_2 = ((float)(HBM_ReadCode[26+NORMAL_DEF_GAMMA_ORDER_NUM]-HBM_ReadCode[25+NORMAL_DEF_GAMMA_ORDER_NUM])/(reg_gray[26]-reg_gray[25]))*(reg_gray_2728[0]-reg_gray[26])+HBM_ReadCode[26+NORMAL_DEF_GAMMA_ORDER_NUM];	
		V27_CODE_B_2 = ((float)(HBM_ReadCode[26+NORMAL_DEF_GAMMA_ORDER_NUM*2]-HBM_ReadCode[25+NORMAL_DEF_GAMMA_ORDER_NUM*2])/(reg_gray[26]-reg_gray[25]))*(reg_gray_2728[0]-reg_gray[26])+HBM_ReadCode[26+NORMAL_DEF_GAMMA_ORDER_NUM*2];	
		printf("V27_CODE_R_2:%d,V27_CODE_G_2:%d,V27_CODE_B_2:%d\r\n",V27_CODE_R_2,V27_CODE_G_2,V27_CODE_B_2);
		//V25\26 外插28
		V28_CODE_R_2 = ((float)(HBM_ReadCode[26]-HBM_ReadCode[25])/(reg_gray[26]-reg_gray[25]))*(reg_gray_2728[1]-reg_gray[26])+HBM_ReadCode[26];
		V28_CODE_G_2 = ((float)(HBM_ReadCode[26+NORMAL_DEF_GAMMA_ORDER_NUM]-HBM_ReadCode[25+NORMAL_DEF_GAMMA_ORDER_NUM])/(reg_gray[26]-reg_gray[25]))*(reg_gray_2728[1]-reg_gray[26])+HBM_ReadCode[26+NORMAL_DEF_GAMMA_ORDER_NUM];	
		V28_CODE_B_2 = ((float)(HBM_ReadCode[26+NORMAL_DEF_GAMMA_ORDER_NUM*2]-HBM_ReadCode[25+NORMAL_DEF_GAMMA_ORDER_NUM*2])/(reg_gray[26]-reg_gray[25]))*(reg_gray_2728[1]-reg_gray[26])+HBM_ReadCode[26+NORMAL_DEF_GAMMA_ORDER_NUM*2];	
		printf("V28_CODE_R_2:%d,V28_CODE_G_2:%d,V28_CODE_B_2:%d\r\n",V28_CODE_R_2,V28_CODE_G_2,V28_CODE_B_2);
		
		V27_CODE_R = round((V27_CODE_R_1+V27_CODE_R_2)/2);
		V27_CODE_G = round((V27_CODE_G_1+V27_CODE_G_2)/2);
		V27_CODE_B = round((V27_CODE_B_1+V27_CODE_B_2)/2);  //此处�??10进制，需要拆分为俩个16进制写法

		V28_CODE_R = (V28_CODE_R_1+V28_CODE_R_2)/2;
		V28_CODE_G = (V28_CODE_G_1+V28_CODE_G_2)/2;
		V28_CODE_B = (V28_CODE_B_1+V28_CODE_B_2)/2;
		printf("V27_CODE_R:%d,V27_CODE_G:%d,V27_CODE_B:%d\r\n",V27_CODE_R,V27_CODE_G,V27_CODE_B);
		printf("V28_CODE_R:%d,V28_CODE_G:%d,V28_CODE_B:%d\r\n",V28_CODE_R,V28_CODE_G,V28_CODE_B);
		unsigned char h_ac,l_ad; //V27R
		unsigned char h_ae,l_af; //V28R
		unsigned char h_b2,l_b3; //V27G
		unsigned char h_b4,l_b5; //V28G
		unsigned char h_b8,l_b9; //V27B
		unsigned char h_ba,l_bb; //V28B	
		
		ConvertAndSplit(V27_CODE_R,&h_ac,&l_ad);
		printf("h_ac_V27R:%x,l_ad:%x\r\n",h_ac,l_ad); //V27R
		
		ConvertAndSplit(V28_CODE_R,&h_ae,&l_af);
		printf("h_ae_V28R:%x,l_af:%x\r\n",h_ae,l_af); //V28R
		
		ConvertAndSplit(V27_CODE_G,&h_b2,&l_b3);
		printf("h_b2_V27G:%x,l_b3:%x\r\n",h_b2,l_b3); //V27G
		
		ConvertAndSplit(V28_CODE_G,&h_b4,&l_b5);
		printf("h_b4_V28G:%x,l_b5:%x\r\n",h_b4,l_b5);  //V28G
		
		ConvertAndSplit(V27_CODE_B,&h_b8,&l_b9);
		printf("h_b8:%x,l_b9:%x\r\n",h_b8,l_b9);   //V27B
		
		ConvertAndSplit(V28_CODE_B,&h_ba,&l_bb);
		printf("h_ba:%x,l_bb:%x\r\n",h_ba,l_bb);	 //V28B		
//		ConvertAndSplit(V27_CODE_R,&h_ac,&l_ad);
//		printf("h_ac:%x,l_ad:%x\r\n",h_ac,l_ad);	
		printf("!!V27_CODE_R:%d,V27_CODE_G:%d,V27_CODE_B:%d\r\n",V27_CODE_R,V27_CODE_G,V27_CODE_B);
		printf("!!V28_CODE_R:%d,V28_CODE_G:%d,V28_CODE_B:%d\r\n",V28_CODE_R,V28_CODE_G,V28_CODE_B);
		printf("V27_CODE_R:%d\r\n",h_ac+l_ad);
		//gamma外插写入V27、V28俩个绑点
		//V27 r
		MIPI_WR(0x15,0xfe,0x53);
		MIPI_WR(0x15,0xac,h_ac);
		MIPI_WR(0x15,0xad,l_ad);
		//V28 r
		MIPI_WR(0x15,0xae,h_ae);
		MIPI_WR(0x15,0xaf,l_af);
		//V27 g
		MIPI_WR(0x15,0xb2,h_b2);
		MIPI_WR(0x15,0xb3,l_b3);
		//V28 g
		MIPI_WR(0x15,0xb4,h_b4);
		MIPI_WR(0x15,0xb5,l_b5);
		//v27 b
		MIPI_WR(0x15,0xb8,h_b8);
		MIPI_WR(0x15,0xb9,l_b9);
		//V28 b
		MIPI_WR(0x15,0xba,h_ba);
		MIPI_WR(0x15,0xbb,l_bb);
		
		//计算完后将寄存器值写�??
		GammaConfig_HBM.gammaRegWriteAll(GammaConfig_HBM.gammaSetReg);
		
		if(V27_CODE_R >= V28_CODE_R || V27_CODE_G >= V28_CODE_G || V27_CODE_B >= V28_CODE_B)
		{
			SwitchFrame(8); //red
			MONITOR_SetBee(1);
			Delay_ms(2000);
			MONITOR_SetBee(0);
			Readgamma_flag = 1;
		}
}

static float _errdataxAdd(uint8_t num,NEW_GammaConfigStruct *gammaConfig, uint8_t _rgb)
{
	float lv, targetX, targetY;
	uint8_t level;
	level = gammaConfig->OGammaConfig[num].level;
	lv=pow(level/255.0,2.2)*gammaConfig->maxlv;
	
	/*****************************awe******************* */
	/*************************
	if(level == 19){
		if(_rgb == _red){
			targetX = 0.268f;
			targetY = 0.315f;
		}
		if(_rgb == _blue){
			targetX = 0.294f;
			targetY = 0.315f;
		}
		if(_rgb == _green){
			targetX = 0.300f;
			targetY = 0.313f;
		}
	}

	if(level == 15){
		if(_rgb == _red){
			targetX = 0.247f;
			targetY = 0.315f;
		}
		if(_rgb == _blue){
			targetX = 0.292f;
			targetY = 0.315f;
		}
		if(_rgb == _green){
			targetX = 0.300f;
			targetY = 0.313f;
		}
	}

	if(level == 11){
		if(_rgb == _red){
			targetX = 0.225f;
			targetY = 0.315f;
		}
		if(_rgb == _blue){
			targetX = 0.285f;
			targetY = 0.315f;
		}
		if(_rgb == _green){
			targetX = 0.300f;
			targetY = 0.312f;
		}
	}
	*******************/
	if(level == 7){
		if(_rgb == _red){
			targetX = 0.300f;
			targetY = 0.322f;  //250511 0.320-0.322
		}
		if(_rgb == _blue){
			targetX = 0.280f;
			targetY = 0.322f;
		}
		if(_rgb == _green){
			targetX = 0.300f;
			targetY = 0.181f; //250511
		}
	}
/************************************************ */
	
	if(_rgb == _red ){
		
		return (targetX - gammaConfig->OGammaConfig[num].measureXYLv[0]);
	}
	else{
		
		return (targetY - gammaConfig->OGammaConfig[num].measureXYLv[1]);
	}
	return 0;
}

static int _checkGamma(uint8_t num, NEW_GammaConfigStruct *gammaConfig, double xyz[3],sky_comDriver *dev, uint8_t _rgb)
{
	float targetX, targetY;
	float lv;
	char str_data[128] = {0};
	int ff;
	
	double max_D = 0;
	double min_D = 0;

	/*获取level卡控*/
	uint8_t level;
	level = gammaConfig->OGammaConfig[num].level;
	/**************************************************awe***************** */
	
	/********************
	if(level == 19){
		if(_rgb == _red){
			max_D = 0.270;
			min_D = 0.266; //0.266
			if((gammaConfig->OGammaConfig[num].measureXYLv[0] > max_D) || (gammaConfig->OGammaConfig[num].measureXYLv[0] < min_D)){ //0.239-0.243
				if(fun1){
					ff = sprintf(str_data,"check       : X NG\r\n");
					if(dev != 0){
						dev->write(dev,(uint8_t*)str_data,ff);			
					}}		
				return -3;
			}
		}
		if(_rgb == _blue){
			max_D = 0.297;  //0.296
			min_D = 0.291;  //0.292
			if((gammaConfig->OGammaConfig[num].measureXYLv[0] > max_D) || (gammaConfig->OGammaConfig[num].measureXYLv[0] < min_D)){ //0.239-0.243
				if(fun1){
					ff = sprintf(str_data,"check       : X NG\r\n");
					if(dev != 0){
						dev->write(dev,(uint8_t*)str_data,ff);			
					}}		
				return -3;
			}
		}
		if(_rgb == _green){
			max_D = 0.320;  //0.317
			min_D = 0.306;  //0.307
			if((gammaConfig->OGammaConfig[num].measureXYLv[1] > max_D) || (gammaConfig->OGammaConfig[num].measureXYLv[1] < min_D)){ //0.239-0.243
				if(fun1){
					ff = sprintf(str_data,"check       : Y NG\r\n");
					if(dev != 0){
						dev->write(dev,(uint8_t*)str_data,ff);			
					}}		
				return -4;
			}
		}
	}

	if(level == 15){
		if(_rgb == _red){
			max_D = 0.251;  //0.249
			min_D = 0.243;  //0.245
			if((gammaConfig->OGammaConfig[num].measureXYLv[0] > max_D) || (gammaConfig->OGammaConfig[num].measureXYLv[0] < min_D)){ //0.239-0.243
				if(fun1){
					ff = sprintf(str_data,"check       : X NG\r\n");
					if(dev != 0){
						dev->write(dev,(uint8_t*)str_data,ff);			
					}}		
				return -3;
			}
		}
		if(_rgb == _blue){
			max_D = 0.296;  //0.289
			min_D = 0.288;  //0.285
			if((gammaConfig->OGammaConfig[num].measureXYLv[0] > max_D) || (gammaConfig->OGammaConfig[num].measureXYLv[0] < min_D)){ //0.239-0.243
				if(fun1){
					ff = sprintf(str_data,"check       : X NG\r\n");
					if(dev != 0){
						dev->write(dev,(uint8_t*)str_data,ff);			
					}}		
				return -3;
			}
		}
		if(_rgb == _green){
			max_D = 0.322;  //0.315
			min_D = 0.305;  //0.305
			if((gammaConfig->OGammaConfig[num].measureXYLv[1] > max_D) || (gammaConfig->OGammaConfig[num].measureXYLv[1] < min_D)){ //0.239-0.243
				if(fun1){
					ff = sprintf(str_data,"check       : Y NG\r\n");
					if(dev != 0){
						dev->write(dev,(uint8_t*)str_data,ff);			
					}}		
				return -4;
			}
		}
	}

	if(level == 11){
		if(_rgb == _red){
			max_D = 0.228;  //0.227
			min_D = 0.222;  //0.223
			if((gammaConfig->OGammaConfig[num].measureXYLv[0] > max_D) || (gammaConfig->OGammaConfig[num].measureXYLv[0] < min_D)){ //0.239-0.243
				if(fun1){
					ff = sprintf(str_data,"check       : X NG\r\n");
					if(dev != 0){
						dev->write(dev,(uint8_t*)str_data,ff);			
					}}		
				return -3;
			}
		}
		if(_rgb == _blue){
			max_D = 0.289;  //0.285
			min_D = 0.281;  //0.277
			if((gammaConfig->OGammaConfig[num].measureXYLv[0] > max_D) || (gammaConfig->OGammaConfig[num].measureXYLv[0] < min_D)){ //0.239-0.243
				if(fun1){
					ff = sprintf(str_data,"check       : X NG\r\n");
					if(dev != 0){
						dev->write(dev,(uint8_t*)str_data,ff);			
					}}		
				return -3;
			}
		}
		if(_rgb == _green){
			max_D = 0.326;  //0.318
			min_D = 0.302;  //0.302
			if((gammaConfig->OGammaConfig[num].measureXYLv[1] > max_D) || (gammaConfig->OGammaConfig[num].measureXYLv[1] < min_D)){ //0.239-0.243
				if(fun1){
					ff = sprintf(str_data,"check       : Y NG\r\n");
					if(dev != 0){
						dev->write(dev,(uint8_t*)str_data,ff);			
					}}		
				return -4;
			}
		}
	}

		**********/
	if(level == 7){
		
		if(_rgb == _red){
			max_D = 0.315;  //250511  0.315 - 0.33
			min_D = 0.285;  //
			if((gammaConfig->OGammaConfig[num].measureXYLv[0] > max_D) || (gammaConfig->OGammaConfig[num].measureXYLv[0] < min_D)){ //0.239-0.243
				if(fun1){
					ff = sprintf(str_data,"check       : X NG\r\n");
					if(dev != 0){
						dev->write(dev,(uint8_t*)str_data,ff);			
					}}		
				return -3;
			}
		}
		if(_rgb == _blue){
			max_D = 0.330;  //0.281
			min_D = 0.315;  //0.267
			if((gammaConfig->OGammaConfig[num].measureXYLv[1] > max_D) || (gammaConfig->OGammaConfig[num].measureXYLv[1] < min_D)){ //0.239-0.243
				if(fun1){
					ff = sprintf(str_data,"check       :Y NG\r\n");
					if(dev != 0){
						dev->write(dev,(uint8_t*)str_data,ff);			
					}}		
				return -3;
			}
		}
		if(_rgb == _green){
			max_D = 0.180; //250511 0.194-0.180 
			min_D = 0.165; //250511 0.182-0.165
			if((gammaConfig->OGammaConfig[num].measureXYLv[1] > max_D) || (gammaConfig->OGammaConfig[num].measureXYLv[1] < min_D)){ //0.239-0.243
				if(fun1){
					ff = sprintf(str_data,"check       : Y NG\r\n");
					if(dev != 0){
						dev->write(dev,(uint8_t*)str_data,ff);			
					}}		
				return -4;
			}
		}
	}	
		/******************************************************************* */
	return 1;		
}



int run_gammaSpecial(NEW_GammaConfigStruct *gammaConfig, float P, sky_comDriver *dev)
{
	uint16_t ref_reg[3];
    int out_reg[3];
	double target_xyz[3];
	double measure_xyz[3];
	float errData[3];
	int PN;//系数变化因子
	int ff;//字符串长�??
	char str_data[1024];//print info string
	char filename[64];//Log file name
	
	char last_rgb = 0;
	
	float P_base = P;
	float errorSum = 0;
	/*历史数据
	 *遍历历史次数内N组数据与当前数据比较，确保当前数据不与历史重�??
	 *若重复则通过+1 +0 -1等数据对重复数据进行微调
	 *historyData[n1][n2]
	 *@parm n1:决定几组历史数据存储
	 *@parm n2:rgb*/
    int historyData[3][3] = {
        {0, 0, 0},
        {0, 0, 0},
        {0, 0, 0}
    };
    int historyN = 0;
	
	// init data register
	memset(str_data, 0, sizeof str_data);
	PN = 1;

	memset(filename, 0, sizeof filename);
	// Log print, save to sd
    if(fun3)
    {    
        sprintf(filename,"%s", LOG_NAME); 
		pcui_writeFileContent(Code_file, Log_hard);
    }

	// �??7阶开始调[0,1,3,7]
	//[0,1,3,7,11,15,19,23,]
	
	// 11灰阶直接赋�? �??7灰阶开始调�??
	for (int i = 3; i > 0; i--)
	{
		gammaConfig->num = i;//当前是多少阶
		
		/************awe*********************/
		//1,3阶不调了 11
		if(gammaConfig->num == 2)  // 3灰阶赋�?
		{		
			
			{	/* 处理3灰阶的时候需要先处理11灰阶的�? */
				// G11 = G7 + (G15 - G7)/2
				gammaConfig->gammaSetReg[(4)+gammaConfig->Order*1] = gammaConfig->gammaSetReg[(3)+gammaConfig->Order*1] +
				(gammaConfig->gammaSetReg[(5)+gammaConfig->Order*1] -  gammaConfig->gammaSetReg[(3)+gammaConfig->Order*1]) /2;
				//小于1报调试NG				
				if(((gammaConfig->gammaSetReg[(5)+gammaConfig->Order*1] -  gammaConfig->gammaSetReg[(3)+gammaConfig->Order*1])/2) < 1)
				{
					gammaConfig->gammaSetReg[(4)+gammaConfig->Order*1]  = 0;
					_DEBUG("\r\n(G15-G7)/2 < 1\r\n");
					return -1;
				}
		
				// R11 = R7 + (R15 - R7) * (G11-G7) / (G15-G7)
				gammaConfig->gammaSetReg[(4)] = gammaConfig->gammaSetReg[(3)] +
				((gammaConfig->gammaSetReg[(5)] - gammaConfig->gammaSetReg[(3)]) *  (gammaConfig->gammaSetReg[(4)+gammaConfig->Order*1] - gammaConfig->gammaSetReg[(3)+gammaConfig->Order*1] ))/
				(gammaConfig->gammaSetReg[(5)+gammaConfig->Order*1] - gammaConfig->gammaSetReg[(3)+gammaConfig->Order*1]);

				// B11 = B7 + (B15 - B7) * (G11-G7) / (G15-G7)
				gammaConfig->gammaSetReg[(4) + gammaConfig->Order*2] = gammaConfig->gammaSetReg[(3)+gammaConfig->Order*2] +
				((gammaConfig->gammaSetReg[(5) +gammaConfig->Order*2] - gammaConfig->gammaSetReg[(3)+gammaConfig->Order*2]) *  (gammaConfig->gammaSetReg[(4)+gammaConfig->Order*1] - gammaConfig->gammaSetReg[(3)+gammaConfig->Order*1] ))/
				(gammaConfig->gammaSetReg[(5)+gammaConfig->Order*1] - gammaConfig->gammaSetReg[(3)+gammaConfig->Order*1]);
				
			}

			//_rgb1[i]=_rgb3[i]-(_rgb7[i]-_rgb3[i])  // 3 灰阶
			for(uint8_t g=0;g<3;g++)
			{
				if((
					gammaConfig->gammaSetReg[gammaConfig->num+2+gammaConfig->Order*g] - 
					gammaConfig->gammaSetReg[gammaConfig->num+1+gammaConfig->Order*g]) == 0 )
				{
					gammaConfig->gammaSetReg[gammaConfig->num+gammaConfig->Order*g] = 
					gammaConfig->gammaSetReg[gammaConfig->num+1+gammaConfig->Order*g] -1 ;
				
				}else if((
					gammaConfig->gammaSetReg[gammaConfig->num+2+gammaConfig->Order*g] - 
					gammaConfig->gammaSetReg[gammaConfig->num+1+gammaConfig->Order*g]) > 0 )
				{
					  // R7 < (R11 -R7)
					if(
					gammaConfig->gammaSetReg[gammaConfig->num+1+gammaConfig->Order*g] < 
						(
						gammaConfig->gammaSetReg[gammaConfig->num+2+gammaConfig->Order*g] - 
						gammaConfig->gammaSetReg[gammaConfig->num+1+gammaConfig->Order*g]
						)
					   )
					{
						gammaConfig->gammaSetReg[gammaConfig->num+gammaConfig->Order*g] = 0;
						_DEBUG("\r\nVGMP configuration should be optimized\r\n");
					
					}else
						// R7- (R11 - R7)
						gammaConfig->gammaSetReg[gammaConfig->num+gammaConfig->Order*g] = 
						gammaConfig->gammaSetReg[gammaConfig->num+1+gammaConfig->Order*g] - (
						gammaConfig->gammaSetReg[gammaConfig->num+2+gammaConfig->Order*g] - 
						gammaConfig->gammaSetReg[gammaConfig->num+1+gammaConfig->Order*g]);	
								
				}else  // (R11 - R7) <0
				{
//					//小于0报调试NG				
//					if(gammaConfig->gammaSetReg[gammaConfig->num+gammaConfig->Order*g] < 0)
//					{
						gammaConfig->gammaSetReg[gammaConfig->num+gammaConfig->Order*g] = 0;
						_DEBUG("\r\nVGMP configuration should be optimized\r\n");
						return -1;
//					}				
				}
			}

			
			//_rgb1[i]=_rgb3[i]-(_rgb7[i]-_rgb3[i])  // 1 灰阶
			gammaConfig->num = 1;  // 1灰阶赋�?
			for(uint8_t g=0;g<3;g++)
			{
				gammaConfig->gammaSetReg[gammaConfig->num+gammaConfig->Order*g] = 
				gammaConfig->gammaSetReg[gammaConfig->num+1+gammaConfig->Order*g] - (
				gammaConfig->gammaSetReg[gammaConfig->num+2+gammaConfig->Order*g] - 
				gammaConfig->gammaSetReg[gammaConfig->num+1+gammaConfig->Order*g]);	

				//小于0赋�?0 (R7 -R3) > R3
				if((
				gammaConfig->gammaSetReg[gammaConfig->num+2+gammaConfig->Order*g] - 
				gammaConfig->gammaSetReg[gammaConfig->num+1+gammaConfig->Order*g]) > gammaConfig->gammaSetReg[gammaConfig->num+1+gammaConfig->Order*g])
				{
						gammaConfig->gammaSetReg[gammaConfig->num+gammaConfig->Order*g] = 0;
				}				
			}
			
			

		if(fun1){
			ff = sprintf(str_data, "\r\n%s:\r\n",gammaConfig->displaymode);
				dev->write(dev, (uint8_t*)str_data, ff); 
			for(int i =1; i < gammaConfig->Order*3+1; i++){          
				ff = sprintf(str_data, "%d,",gammaConfig->gammaSetReg[i-1]);
				dev->write(dev, (uint8_t*)str_data, ff);  
				if(i % gammaConfig->Order == 0 && i != 0){        
					ff = sprintf(str_data, "\r\n");
					dev->write(dev, (uint8_t*)str_data, ff);  
				}
			}
		}
			
			

			//结束 看你要打印什么内容了
			_DEBUG("123456");
			return 1;
		}
		/************awe*********************/
		
		//继承23阶的�?? RGB
		// out_reg[0] = ref_reg[0] = gammaConfig->gammaSetReg[(7)+gammaConfig->Order*0]; 
		// out_reg[1] = ref_reg[1] = gammaConfig->gammaSetReg[(7)+gammaConfig->Order*1];
		// out_reg[2] = ref_reg[2] = gammaConfig->gammaSetReg[(7)+gammaConfig->Order*2];	
		//继承上一阶的�?? RGB
		// out_reg[0] = ref_reg[0] = gammaConfig->gammaSetReg[(gammaConfig->num-1)+gammaConfig->Order*0];
		// out_reg[1] = ref_reg[1] = gammaConfig->gammaSetReg[(gammaConfig->num-1)+gammaConfig->Order*1];
		// out_reg[2] = ref_reg[2] = gammaConfig->gammaSetReg[(gammaConfig->num-1)+gammaConfig->Order*2];
		
//		out_reg[0] = ref_reg[0] = gammaConfig->gammaSetReg[(gammaConfig->num+1)+gammaConfig->Order*0];
//		out_reg[1] = ref_reg[1] = gammaConfig->gammaSetReg[(7)+gammaConfig->Order*1];
//		out_reg[2] = ref_reg[2] = gammaConfig->gammaSetReg[(7)+gammaConfig->Order*2];	
		
		//继承23阶的�?? RGB
		 // R23
		out_reg[0] = ref_reg[0] = gammaConfig->gammaSetReg[(7)+gammaConfig->Order*0]; 
		 // B23
		out_reg[2] = ref_reg[2] = gammaConfig->gammaSetReg[(7)+gammaConfig->Order*2]; 
		 // G15 -(G23-G15)
		out_reg[1] = ref_reg[1] = gammaConfig->gammaSetReg[(5)+gammaConfig->Order*1] - (gammaConfig->gammaSetReg[(7)+gammaConfig->Order*1] - gammaConfig->gammaSetReg[(5)+gammaConfig->Order*1]); 
		
		
				
		//R->B->G分开�??
		for(uint8_t _rgb=  _green ; _rgb<=_red; _rgb++)
		{
			
			if((last_rgb != _rgb))
			{
				if(_rgb == _blue)
				{
					// G7 锁定 步骤1 所�?? R7 锁定初始化不�?? 
					out_reg[0] = ref_reg[0] = gammaConfig->gammaSetReg[(7)+gammaConfig->Order*0]; 
					out_reg[1] = ref_reg[1] = gammaConfig->gammaSetReg[(3)+gammaConfig->Order*1]; 
					// B
//                  out_reg[2] = ref_reg[2] = gammaConfig->gammaSetReg[(5)+gammaConfig->Order*2] - (gammaConfig->gammaSetReg[(7)+gammaConfig->Order*2] - gammaConfig->gammaSetReg[(5)+gammaConfig->Order*2]);
				    out_reg[2] = ref_reg[2] = gammaConfig->gammaSetReg[(5)+NOR2_DEF_GAMMA_ORDER_NUM*2] - (gammaConfig->gammaSetReg[(7)+NOR2_DEF_GAMMA_ORDER_NUM*2] - gammaConfig->gammaSetReg[(5)+NOR2_DEF_GAMMA_ORDER_NUM*2]);

					//NOR2_DEF_GAMMA_ORDER_NUM
//					out_reg[2] = ref_reg[2] = gammaConfig->gammaSetReg[(gammaConfig->num+1)+gammaConfig->Order*2];
				}else if(_rgb == _red)
				{
					// B7 G7 锁定已调�??
					out_reg[2] = ref_reg[2] = gammaConfig->gammaSetReg[(3)+gammaConfig->Order*2]; 
					out_reg[1] = ref_reg[1] = gammaConfig->gammaSetReg[(3)+gammaConfig->Order*1]; 
					
//					//调R
//					out_reg[0] = ref_reg[0] = gammaConfig->gammaSetReg[(gammaConfig->num+1)+gammaConfig->Order*0];
			//调R
           out_reg[0] = ref_reg[0] = gammaConfig->gammaSetReg[(5)+gammaConfig->Order*0] - (gammaConfig->gammaSetReg[(7)+gammaConfig->Order*0] - gammaConfig->gammaSetReg[(5)+gammaConfig->Order*0]); ;
				}
				last_rgb = _rgb;
			}
			
			PN = 1;
			P = P_base;//awe
			errorSum = 0;
			printf("\r\nP_remake:%.3f,PN_remake = %d\r\n",P,PN);
			
			_getTargetXYZ(i,gammaConfig,target_xyz, _rgb);//获取该阶目标xyz
				
			gammaConfig->gammaRegWrite(gammaConfig->Order -1,ref_reg);//255�??
				
			gammaConfig->readLv(gammaConfig->OGammaConfig[i].readD,gammaConfig->OGammaConfig[i].measureXYLv,measure_xyz);//读n次数据时间做延时 数据无效
			Delay_ms(gammaConfig->OGammaConfig[i].delay);		//延时
			gammaConfig->readLv(gammaConfig->OGammaConfig[i].readN,gammaConfig->OGammaConfig[i].measureXYLv,measure_xyz);//读镜头数�??
			
			//Debug info
			if(fun1){
				ff = sprintf(str_data,"\r\n%clevel       : %d\n",_get_rgb_char(_rgb),gammaConfig->OGammaConfig[i].level);
				if(dev != 0){
					dev->write(dev,(uint8_t*)str_data,ff);			
				}

				ff = sprintf(str_data,	
				"targetxyz   : %.4f(x)     \t%0.4f(y)       \t%0.4f(z)\
				\r\nmesaurexyz  : %.4f(x)     \t%0.4f(y)       \t%0.4f(z)\
				\r\ntargetxyL   : %.4fx(%.4f~%.4f) \t%.4fy(%.4f~%.4f)   \t%.4f(Lmax %.4f Lmin %.4f)\
				\r\nmesaurexyL  : %.4f(x)     \t%.4f(y)          \t%.4f(L)\
				\r\nnow rgbReg  : %d(r)          \t%d(g)            \t%d(b)\n",
				target_xyz[0], target_xyz[1], target_xyz[2],
				measure_xyz[0], measure_xyz[1], measure_xyz[2],
				Debug_targetX,Debug_MaxX,Debug_MinX,Debug_targetY,Debug_MaxY,Debug_MinY,
				gammaConfig->OGammaConfig[i].targetLv, gammaConfig->OGammaConfig[i].targetlvMax, gammaConfig->OGammaConfig[i].targetlvMin,			
				gammaConfig->OGammaConfig[i].measureXYLv[0],gammaConfig->OGammaConfig[i].measureXYLv[1],gammaConfig->OGammaConfig[i].measureXYLv[2],
				ref_reg[0],ref_reg[1],ref_reg[2]);
				if(dev != 0){
					dev->write(dev,(uint8_t*)str_data,ff);			
				}	
			}
        
			// Check
			if (_checkGamma(i,gammaConfig,measure_xyz,dev,_rgb) > 0)
			{
				if(fun1){
					ff = sprintf(str_data,"\ntimes: %d ,%cleave: %d  ok\n",i,_get_rgb_char(_rgb),gammaConfig->OGammaConfig[i].level);
					if(dev != 0)
					{
						dev->write(dev,(uint8_t*)str_data,ff);			
					}
				}   
				if(fun3){
						//Log_print(gammaConfig, i, 'Y', filename,dev);
						Log_print(gammaConfig, i, 'Y', filename);
				}
//				for(int i = 0; i < 3; i++){            
//					//判断RGB不是255阶的RGB时，设置的RGB不能超过上一阶设置的RGB�??
//					if (gammaConfig->num != gammaConfig->Order-1){
//						if(ref_reg[i] > gammaConfig->gammaSetReg[(gammaConfig->num+1) +gammaConfig->Order*i])
//							ref_reg[i] = gammaConfig->gammaSetReg[(gammaConfig->num+1)+gammaConfig->Order*i]-1;
//					}
//				}
				
				//保存调好的rgb数据
				for(uint8_t g=0;g<3;g++)
				{
						gammaConfig->gammaSetReg[i+gammaConfig->Order*g]=ref_reg[g];					
						out_reg[g] = ref_reg[g];
				}
				continue;
			}	
			
		
			//循环调N次调好退�??
			for(uint8_t k=0;k<gammaConfig->OGammaConfig[i].tuningN;k++)
			{
				gammaConfig->ci = k;				
				for(uint8_t j=0;j<3;j++){
					errData[j] = target_xyz[j]-measure_xyz[j];
					errData[j] *= P;
				}
//				printf("new: %f,%f,%f,%d,%d,%d,P_Data:%.4f\n", errData[0], errData[1], errData[2], out_reg[0], out_reg[1], out_reg[2],P);
				
				
				
				//引入积分解决稳态误�??
				errorSum = _errdataxAdd(i,gammaConfig,_rgb);
				if(_rgb==_red)
					errData[0] = P*errorSum;
				if(_rgb==_green)
					errData[1] = P*errorSum;
				if(_rgb==_blue)
					errData[2] = -P*errorSum;		
				
//				printf("new: %f,%f,%f,nerrorSum %.4f\r\n", errData[0], errData[1], errData[2],errorSum);
				
				while(1)
				{
					int res = 0, re = 0;
					for(uint8_t j=0;j<3;j++)
					{
							if (errData[j] < 0)
									PN += 1;
							else
									PN -= 5;

							if (PN > 12)
							{
									PN = 12;
									P *= 1.03f;
									if (P > 200)
											P = 200;
							}
							
							if (errData[j] > 50)
									errData[j] = 50;
							if (errData[j] < -50)
									errData[j] = -50;
							
//							if (j == _rgb)
//								res = round(errData[j]);
							
							
							if(_rgb == _red && j == 0)
								res = round(errData[j]);
							else if(_rgb == _green && j == 1)
								res = round(errData[j]);
							else if(_rgb == _blue && j == 2)
								res = round(errData[j]);
							
							
							if(res != 0)
									re++;

									out_reg[j]+=lround(errData[j]);
//							printf("%f,%f,%f,%d,%d,%d\n", errData[0], errData[1], errData[2], out_reg[0], out_reg[1], out_reg[2]);

							
							if (out_reg[j]>gammaConfig->RGB_MAX)
									out_reg[j] =gammaConfig->RGB_MAX;
							
							if (out_reg[j]<gammaConfig->RGB_MIN)
									out_reg[j] =gammaConfig->RGB_MIN;    
							
							if(out_reg[j]<0)
									out_reg[j] = 0;
					}
					if(re != 0)
							break;
					else
					{
							for(int i = 0; i < 3; i++)
							{
									errData[i] *= 1.2f;
							}
					}
				}
							
//				for(int i = 0; i < 3; i++){            
//					//判断RGB不是255阶的RGB时，设置的RGB不能超过23阶设置的RGB�??
//					if (gammaConfig->num != gammaConfig->Order-1)
//					{
//						if(out_reg[i] > gammaConfig->gammaSetReg[(7) +gammaConfig->Order*i])
//							out_reg[i] = gammaConfig->gammaSetReg[(7)+gammaConfig->Order*i]-1;
//					}
//				}
				
				
				// 历史数据处理
				getNotRepeatReg(out_reg, historyData);
				for(int i = 0; i < 3; i++){ 
						historyData[historyN][i] = out_reg[i];
				}
				if(++historyN > 2)
						historyN = 0;
		 
				//只改调的对应的�?
				if(_rgb == _red){
					ref_reg[0] = out_reg[0];
				}
				if(_rgb == _blue){
					ref_reg[2] = out_reg[2];
				}
				if(_rgb == _green){
					ref_reg[1] = out_reg[1];
				}
				
				gammaConfig->gammaRegWrite(gammaConfig->Order -1,ref_reg);//255 write

				gammaConfig->readLv(gammaConfig->OGammaConfig[i].readD,gammaConfig->OGammaConfig[i].measureXYLv,measure_xyz);		
				Delay_ms(gammaConfig->OGammaConfig[i].delay);		
				gammaConfig->readLv(gammaConfig->OGammaConfig[i].readN,gammaConfig->OGammaConfig[i].measureXYLv,measure_xyz);

				if(fun1){				
					ff = sprintf(str_data,	
					"\
					 \r\ntimes       : %d\
					 \r\nP           : %0.4f\
					 \r\ntargetxyz   : %.4f(x)     \t%0.4f(y)       \t%0.4f(z)\
					 \r\nmesaurexyz  : %.4f(x)     \t%0.4f(y)       \t%0.4f(z)\
					 \r\ntargetxyL   : %.4fx(%.4f~%.4f) \t%.4fy(%.4f~%.4f)      \t%.4f(Lmax %.4f Lmin %.4f)\
					 \r\nmesaurexyL  : %.4f(x)     \t%.4f(y)          \t%.4f(L)\
					 \r\nnow rgbReg  : %d(r)          \t%d(g)            \t%d(b)\n",
					k,
					P,
					target_xyz[0], target_xyz[1], target_xyz[2],
					measure_xyz[0], measure_xyz[1], measure_xyz[2],
					Debug_targetX,Debug_MaxX,Debug_MinX,Debug_targetY,Debug_MaxY,Debug_MinY,
					gammaConfig->OGammaConfig[i].targetLv, gammaConfig->OGammaConfig[i].targetlvMax, gammaConfig->OGammaConfig[i].targetlvMin,			
					gammaConfig->OGammaConfig[i].measureXYLv[0],gammaConfig->OGammaConfig[i].measureXYLv[1],gammaConfig->OGammaConfig[i].measureXYLv[2],
					ref_reg[0],ref_reg[1],ref_reg[2]);
					if(dev != 0)
					{
						dev->write(dev,(uint8_t*)str_data,ff);			
					}			
				}

				if (_checkGamma(i,gammaConfig,measure_xyz,dev,_rgb) > 0)
				{
					if(fun1){
						ff = sprintf(str_data,"\nnum: %d ,leave%c: %d  ok\n",i,_get_rgb_char(_rgb),gammaConfig->OGammaConfig[i].level);
						if(dev != 0)
						{
							dev->write(dev,(uint8_t*)str_data,ff);			
						}
					}

					if(fun3){
					//	Log_print(gammaConfig, i, 'Y', filename,dev);
						Log_print(gammaConfig, i, 'Y', filename);
					}

					//保存调好的rgb数据
					for(uint8_t g=0; g<3; g++)
					{
						gammaConfig->gammaSetReg[i+gammaConfig->Order*g] = ref_reg[g];					
						out_reg[g] = ref_reg[g];
					}			
					break;
				}
				 
				if(k > gammaConfig->OGammaConfig[i].tuningN-2)
				{            
					if(fun1){
					ff = sprintf(str_data,"\nnum: %d ,leave%c: %d  NG\n",i,_get_rgb_char(_rgb),gammaConfig->OGammaConfig[i].level);
					if(dev != 0){
						dev->write(dev,(uint8_t*)str_data,ff);			
					}}
					if(fun3){
						//Log_print(gammaConfig, i, 'N', filename,dev);
						Log_print(gammaConfig, i, 'N', filename);
					}
					return -1;
				}
			}
				
			}
		
	}
	return 1;	
}


//*********************mode *********************************** */
void Debug_target(uint8_t _rgb,uint8_t level_D){

	double max_D = 0;
	double min_D = 0;

	uint8_t level = level_D;
	
	/***
	if(level == 19){
		if(_rgb == _red){
			max_D = 0.270;
			min_D = 0.266;
		}
		if(_rgb == _blue){
			max_D = 0.297;
			min_D = 0.291;
		}
		if(_rgb == _green){
			max_D = 0.320;
			min_D = 0.306;
		}
	}

	if(level == 15){
		if(_rgb == _red){
			max_D = 0.251;
			min_D = 0.243;
		}
		if(_rgb == _blue){
			max_D = 0.296;
			min_D = 0.288;
		}
		if(_rgb == _green){
			max_D = 0.322;
			min_D = 0.305;
		}
	}

	if(level == 11){
		if(_rgb == _red){
			max_D = 0.228;
			min_D = 0.222;
		}
		if(_rgb == _blue){
			max_D = 0.289;
			min_D = 0.281;
		}
		if(_rgb == _green){
			max_D = 0.326;
			min_D = 0.306;
		}
	}
	*********************/
	if(level == 7){
		if(_rgb == _red){
			max_D = 0.330;  //250511 0.315-0.330
			min_D = 0.285;
		}
		if(_rgb == _blue){
			max_D = 0.330;
			min_D = 0.315;
		}
		if(_rgb == _green){
			max_D = 0.180; //250511 0.194-0.180
			min_D = 0.165; //250511 0.182-0.165
		}
	}	
	
	if(_rgb == _green){
		Debug_MaxX = 0;
		Debug_MinX = 0;
		Debug_MaxY = max_D;
		Debug_MinY = min_D;
	}else if(_rgb == _blue){
		Debug_MaxX = 0;
		Debug_MinX = 0;
		Debug_MaxY = max_D;
		Debug_MinY = min_D;
	}else if(_rgb == _red){
		Debug_MaxX = max_D;
		Debug_MinX = min_D;
		Debug_MaxY = 0;
		Debug_MinY = 0;
	}
	
	/******************************************************************* */
}
