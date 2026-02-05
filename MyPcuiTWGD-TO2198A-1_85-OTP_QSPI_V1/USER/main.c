/********************************* SKYCODE ************************************
* Copyright (c) 2018, 深圳思凯测试技术有限公司
* All rights reserved.
*
* 文件名   ：main.c
* 描述     ：S10 快速点屏演示程序
*
* 版    本 ：V1.0.7
* 作    者 ：
* 完成日期 ：2018
* 修改描述 ：

******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "sys.h"
#include "S10.h"
#include "Bus.h"
#include "sd_file.h"
#include "ssd2828.h"
#include "in_img.h"
#include "showtext.h"
#include "cfl.h"
#include "lwipUser.h"
#include "ff.h"
#include "stm_spi.h"
#include "exti.h"
#include "init_code.h"
#include "stdarg.h"
#include "pcui.h"
#include "driverTCPServer.h"
#include "driverUsart1.h"
#include "user_control.h"
#include "in_img2.h"
#include "scanner.h"
#include "math.h"
#include "User_UI.h"
// #include "app_gamma.h"
// #include "gammaLinear.h"
// #include "gammaRun.h"
#include <string.h>
#include "qspi_lcd.h"
#include "in_img2.h"
// #include "lmt_gamma.h"

#define HOT_KEY 0 // 热拔插


#define Normal_Black_Lv_max 0.005
#define Normal_White_Lv_min 470
#define Normal_White_Lv_max 530
#define Normal_White_X_min 0.29F
#define Normal_White_X_max 0.31F
#define Normal_White_Y_min 0.30F
#define Normal_White_Y_max 0.32F
#define Normal_Red_X_min 0.666F
#define Normal_Red_X_max 0.706F
#define Normal_Red_Y_min 0.290F
#define Normal_Red_Y_max 0.330F
#define Normal_Green_X_min 0.195F
#define Normal_Green_X_max 0.275F
#define Normal_Green_Y_min 0.68F
#define Normal_Green_Y_max 0.760F
#define Normal_Blue_X_min 0.123F
#define Normal_Blue_X_max 0.163F
#define Normal_Blue_Y_min 0.024F
#define Normal_Blue_Y_max 0.064F

/*---------------------------------------------------------------------------*/

extern uint8_t pack_data_C2[260];
extern uint8_t pack_data_C3[260];
extern uint8_t pack_data_C5[260];
extern uint8_t pack_data_C6[260];
extern uint8_t pack_data_C7[260];
extern uint8_t pack_data_C8[260];
extern uint8_t pack_data_C9[260];
extern uint8_t pack_data_CA[260];
extern uint8_t pack_data_CB[260];

FlagStatus Flag_EXTI_Line0 = RESET;
extern unsigned int _10ms_ok;
// extern char Code_file[128];
unsigned int display_on = 0;
u8 frame = 0;
unsigned int frame_max = 0;
unsigned int frame_hold_ms[64]; // 画面保持时间
unsigned int frame_hold_cnt;
unsigned int gamma_hold_cnt;
unsigned int all_hold_cnt;
char frame_debug[64][16]; // 画面显示名称
u8 auto_switch_mode = 0;  //=0 按键切换； =1自动切换
u8 OTP_times = 0;
unsigned char OTP_flag = 0;
uint8_t l_OTP_times = 0;
uint8_t  White_frame  = 0;
uint8_t  Black_frame    = 0;
uint8_t  OK_frame     = 0;
uint8_t  NG_frame     = 0;
uint8_t  CH_NUM = 0 ;//通道 
uint8_t  OTP_Flag     = 0;

unsigned char OTP_HUO = 0; // 烧录后的光学标志位
unsigned int CHECKER_FLAG = 0;
int EOTP_times;
u8 allcode_falg1 = 0;
u8 allcode_falg2 = 0;
PowerMeasureTypeDef g_power_meas;
WaveMeasureTypeDef g_meter_meas;
XYLvTypeDef XYLv;
InImg2_Adapter img2_adapter;		//QSPI的送图

unsigned int ID_1 = 0;
unsigned int ID_2 = 0;
u8 sleep_flag = 0;

/**************OTPgamma部分的相关定义*********************/
int rrr; // 判断gamma调试结果
sky_comDriver Gamma_dev;

userContrlStruct User;

u8 num = 0;
char message[4096];
char str[256];
/* --------------------------------------------------------------------------*/
void S20_Init(void);
void NVIC_Configuration(void);
void SwitchFrame(unsigned int frame);
void Display_ON(void);
void Display_OFF(void);
void Sleep_In(void);
void Sleep_Out(void);
void Panel_Init(void);
void code_init(void);
void LCD_RST_0(void);
void LCD_RST_1(void);
void Check_gamma_Register(userContrlStruct *user);
int NVM_ALL(userContrlStruct *user); 
void Panel_Read(void);
void Display_ON1(void);

unsigned int _100ms_STA = 0;
unsigned int Alarm_STA  = 0;
unsigned int NormalR26;
unsigned char Readgamma_flag = 0;
unsigned char COFid[3];
char scanner[50];
char scancode[50];
unsigned char scanner_flag = 0;  // 扫码枪标志位
uint16_t data[3]           = {1121, 1130, 1268};
uint16_t data1[3]          = {0x459, 0x45A, 0x4E4};
uint16_t data2[3]          = {0x449, 0x44A, 0x4D4};
/* --------------------------------------------------------------------------*/

/*不可修改*/
#define IO_OE_H() GPIO_SetBits(GPIOE, GPIO_Pin_11)
#define IO_OE_L() GPIO_ResetBits(GPIOE, GPIO_Pin_11)

/* --------------------------------------------------------------------------*/

/*通用IO初始化*/
void IO_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOG, ENABLE);

    // OE 不可修改
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOE, &GPIO_InitStructure);
    IO_OE_L();

    /*------------------------------------------------*/
    /*用户IO*/

    // RST->PB7
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_ResetBits(GPIOB, GPIO_Pin_7);

    // VCI_EN
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_ResetBits(GPIOB, GPIO_Pin_1);
}

void LCD_RST_0(void)
{
    GPIO_ResetBits(GPIOB, GPIO_Pin_7);
}

void LCD_RST_1(void)
{
    GPIO_SetBits(GPIOB, GPIO_Pin_7);
}

void AVDD_EN_0(void)
{
	GPIO_ResetBits(GPIOB, GPIO_Pin_1);
}

void AVDD_EN_1(void)
{
	GPIO_SetBits(GPIOB, GPIO_Pin_1);
}

char dataStr[128];
int Gamma_printf(const char *fmt, ...)
{

    int ff;
    va_list arg_ptr;

    dataStr[0] = 0;

    va_start(arg_ptr, fmt);
    ff = vsprintf((char *)dataStr, fmt, arg_ptr);
    va_end(arg_ptr);

    Gamma_dev.write(&Gamma_dev, (uint8_t *)dataStr, strlen(dataStr));
}

/********************************************************************************
 *函數名：logab
 *描述  ：计算以a为底b的对数
 *返回  ：结果
 ********************************************************************************/
float logab(float a, float b)
{
    float x1 = log(a);
    float x2 = log(b);

    return x2 / x1;
}

int sateext  = 1;
int yanstate = 1;

int ssss(void)
{
    if (yanstate != 0)
    {
        if (sateext == 0)
        {
            //			normalGammaConfig.Defensive = -1;
            //		  HBMGammaConfig.Defensive = -1;
            yanstate = 0;

            Display_OFF();  // 调用关显示和掉电顺序函数

            Delay_ms(20);
            sateext = 1;
        }
    }
}

/* --------------------------------------------------------------------------*/

char procjetName[64];
char date[32];
char logfilename[48];

char parameter[5] = {1, 2, 3, 4, 5};
int parameter_flag;
int ret;
unsigned int wet;
///////////////////////////////////////////////////////////////////////////////
u8 g_PN                = 1;
int editon_flag        = 1;
uint8_t pc_remoteIP[4] = {0, 0, 0, 0};  // PC远端IP
uint16_t pc_remotePort = 0;             // PC远端端口

int main(void)
{
	int res;
	u8 key;
	int i;
	unsigned int ts;
	int rrr;
	// AOI机台通讯上用到
	char get_cmd_str[64] = {0};
	char id_str[64] = {0};
	int frame_ms;
	float G_x[25], G_y[25], G_Lv[25], G_Gamma[25];
	char string_buffer[64];

	/*S10测机硬件初始化*/
	S10_Init();
	Spi_Init();

	/*通用IO初始化*/
	IO_Configuration();
	IOVOL_Set(1.8); // IO输出电压设定 =模组IOVCC

	IO_OE_H();

#if HOT_KEY
	EXTI_PC3_Config();
#endif

	driverUSART1_Init();

#if PCUI_FLAG
	while (pcui_init(driverUSART1_Init()) != 0)
	{
		_DEBUG("pcui_init USART1 NG\r\n");
	} // 绑定串口
	_DEBUG("pcui_init USART1 OK\r\n");
	User_UI();
#endif

	// GAMMA_Init();
	Gamma_dev.write = usart1_write;
	/**/
    MONITOR_SetName(WAVE1_NAME, "PWM");
	MONITOR_SetName(WAVE2_NAME, "TE");
	MONITOR_SetName(VDD1_NAME, "VDDI");
	MONITOR_SetName(VDD2_NAME, "VBAT");
	MONITOR_SetName(VDD3_NAME, "MTP");
	MONITOR_SetName(VDD4_NAME, "  ");
	MONITOR_SetName(VDD5_NAME, "  ");

	CFL_Set_CH(CFL_ID0, &CH_NUM); /////切换通道
	/*显示项目名称*/
	Power_SetMonitorStringName(1, "TWGD-SA2057A-MIPI-V01");
	/*-----LCD 参数设定---*/
	/*LCD 接口模式*/
   gLCD_IFMODE = MIPI_VIDEO_1LANE;

	/*?????????*/
	gLCD_XSIZE = 340;
	gLCD_YSIZE = 340;

	/*VIDEO?? RGB?????????VIDEO????Ч*/
	gLCD_PCLK = 30; // ??λMHz ????0.1MHz

	gLCD_HBPD = 18;
	gLCD_HFPD = 18;
	gLCD_HSPW = 4;

	gLCD_VBPD = 32;
	gLCD_VFPD = 32;
	gLCD_VSPW = 10;

	gLCD_PCLK_EDGE = FALLING_EDGE;	// lcd锁存数据时的PCLK边沿
	gLCD_HS_POLARITY = ACTIVE_LOW;	// HSYNC脉冲极性
	gLCD_VS_POLARITY = ACTIVE_LOW;	// VSYNC脉冲极性
	gLCD_DE_POLARITY = ACTIVE_HIGH; // DE信号有效电平

	/*COMMAND 模式每lane数据率设置，只COMMAND模式有效*/
	gHS_Mbps = 800;

	GPU_Config();
	//    GPU_SetCommandWrSpeed(14);
	/*切换画面的方式*/
	auto_switch_mode = 0; //=0 按键切换； =1自动切换

	/*加载画面, 和设置画面停留时间*/
	_DEBUG("Load image..\n\r");

	frame_max = 14;

	frame_hold_ms[0] = 1000; // 第一张画面延时
	frame_hold_ms[1] = 1000; // 第二张画面延时
	frame_hold_ms[2] = 1000;
	frame_hold_ms[3] = 1000;
	frame_hold_ms[4] = 1000;
	frame_hold_ms[5] = 1000;
	frame_hold_ms[6] = 1000;
	frame_hold_ms[7] = 1000;
	frame_hold_ms[8] = 1000;
	frame_hold_ms[9] = 1000;
	frame_hold_ms[10] = 1000;
	frame_hold_ms[11] = 1000;
	frame_hold_ms[12] = 1000;
	frame_hold_ms[13] = 1000;

	_DEBUG("Load image OK\n\r");

	MONITOR_SetBee(1);
	Delay_ms(200);
	MONITOR_SetBee(0);

	ShowString_2_Config(SPI_RGB, SPI_DispArea);
	SDCard_2_Confg(1, SPI_Send_Dat, SPI_DispArea);
	// QSPI_LCD_IO_Init();
    // InImg2_Init(&img2_adapter, 390, 450, QSPI_LCD_WriteRGB, QSPI_LCD_GotoXY); //要用到in_img2 软件产生画面
    // ShowString_2_Config(QSPI_LCD_WriteRGB, QSPI_LCD_GotoXY); //字符串显示
    // SDCard_2_Confg(1,QSPI_LCD_WrByte,QSPI_LCD_GotoXY);   //SD卡SPI模式
    while (1)
    {

#if PCUI_FLAG
        User_Event();
#endif

////////////////////////////////////////热拔插控制///////////////////////////////////////////////////////
        key = KEY_Read();

        switch (key)
        {
		case KEY_ONOFF:
			if (display_on == 0)
			{
				PowerOn();
			}
			else
			{
				PowerOFF();
			}
			break;

		case KEY_DOWN: // 下翻
			KEYDOWN();
			break;

		case (KEY_UP | KEY_DOWN):
			if ((auto_switch_mode == 1) || (display_on == 0))
			{
				break;
			} // 在自动切换画面 或 DisplayOFF此按键无
			break;

		case KEY_UP: // 上翻
			KEYUP();
			break;

		case KEY_OTP:
			KEY_Enter();
			break;
		}

        /*自动切换画面和画面保持*/
        if ((auto_switch_mode == 1) && (_1ms_ok == 1) && (display_on == 1))
        {
            _1ms_ok = 0;
            if (++frame_hold_cnt >= frame_hold_ms[frame])
            {
                frame_hold_cnt = 0;
                if (frame < (frame_max - 1))
                {
                    frame++;
                }
                else
                {
                    frame = 0;
                }
                SwitchFrame(frame);
            }
        }

    }
}

/*切换画面控制，画面测量 报警设置等*/
void SwitchFrame(unsigned int frame)
{   
    switch (frame)
	{
	case 0:
		Img_Full(255, 255, 255);
		Power_SetMonitorStringName(1, "white");
		break;
	case 1:
		Img_Full(0, 0, 0);
		Power_SetMonitorStringName(1, "black");
		break;
	case 2:
		Img_Full(255, 0, 0);
		Power_SetMonitorStringName(1, "red");
		Power_SetMonitorStringName(2, "");
		Power_SetMonitorStringName(3, "");
		break;
	case 3:
		Img_Full(0, 255, 0);
		Power_SetMonitorStringName(1, "green");
		Power_SetMonitorStringName(2, "");
		Power_SetMonitorStringName(3, "");
		break;

	case 4:
		Img_Full(0, 0, 255);
		Power_SetMonitorStringName(1, "blue");
		Power_SetMonitorStringName(2, "");
		Power_SetMonitorStringName(3, "");
		break;

	case 5:
		Img_Gray256_V();
		Power_SetMonitorStringName(1, "Gray256_V");
		Power_SetMonitorStringName(2, "");
		Power_SetMonitorStringName(3, "");
		break;

	case 30:
		Img_Full(0, 255, 0);
		Power_SetMonitorStringName(2, "");
		Power_SetMonitorStringName(3, "");
		break;
	}
}
/*上电顺序*/
void Display_ON(void)
{
	POWER_SetIxMeaRange(1, RANGE_MA);
	POWER_SetIxMeaRange(2, RANGE_MA);
	POWER_SetIxMeaRange(4, RANGE_MA);
	POWER_SetIxMeaRange(5, RANGE_MA);
	IO_OE_H();
	//    SSD2828_POWER_ON();
	KEY_SetLED(KEY_ONOFF, KEYLED_ON);
	/*LCM 上电顺序*/

	LCD_RST_0();
	// POWER_SetVDD(6, 3.3); // VCI
	// Delay_ms(10);
	// POWER_SetVDD(7, 1.8); // VCI
	// Delay_ms(10);
	POWER_SetVDD(2, 3.7); // VBAT
	Delay_ms(50);

	POWER_SetVDD(1, 1.8); // VDDI
	Delay_ms(50);
	AVDD_EN_1();
	Delay_ms(50);
	//		POWER_SetVDD(2,3.3);   //VDDI
	//    Delay_ms(20);

	LCD_RST_1();
	Delay_ms(10);
	LCD_RST_0();
	Delay_ms(60);
	LCD_RST_1();
	Delay_ms(100);

	Panel_Read();
	
	Panel_Init();  //初始化
	Delay_ms(100);
	SwitchFrame(frame = 0);




	display_on = 1;
	_DEBUG("Display ON\n\r");
}
/*上电顺序(不下初始化)*/
void Display_ON1(void)
{
	POWER_SetIxMeaRange(1, RANGE_MA);
	POWER_SetIxMeaRange(2, RANGE_MA);
	POWER_SetIxMeaRange(4, RANGE_MA);
	POWER_SetIxMeaRange(5, RANGE_MA);
	IO_OE_H();
	//    SSD2828_POWER_ON();
	KEY_SetLED(KEY_ONOFF, KEYLED_ON);
	/*LCM 上电顺序*/

	LCD_RST_0();
	// POWER_SetVDD(6, 3.3); // VCI
	// Delay_ms(10);
	// POWER_SetVDD(7, 1.8); // VCI
	// Delay_ms(10);

	POWER_SetVDD(1, 1.8); // VDDI
	Delay_ms(100);
	POWER_SetVDD(2, 3.7); // VBAT
	Delay_ms(100);
	AVDD_EN_1();
	Delay_ms(50);
	//		POWER_SetVDD(2,3.3);   //VDDI
	//    Delay_ms(20);

	LCD_RST_1();
	Delay_ms(10);
	LCD_RST_0();
	Delay_ms(60);
	LCD_RST_1();
	Delay_ms(100);

	// Panel_Read();
	
	SPI_Write(0x00,0x00);
	SPI_Write(0xFF,0x23,0x90,0x01,0x00);

	SPI_Write(0x00,0x80);
	SPI_Write(0xFF,0x23,0x90);

	SPI_Write(0x00,0x00);														
	SPI_Write(0x51,0xff);  

	SPI_Write(0x00,0x00);														
	SPI_Write(0x63,0xff);
	// if(All_GAMMA_OTP_times == 0) code_init();
	// GammaConfig_Normal.gammaRegWriteAll(GammaConfig_Normal.gammaDefReg);
	// gammaRegReadAll_Normal();

	SPI_Write_cmd(0x11);
	Delay_ms(120);
	SPI_Write_cmd(0x29);
	SPI_Write(0x35,0x00);
	Delay_ms(300);
	SwitchFrame(frame = 0);




	display_on = 1;
	_DEBUG("Display ON\n\r");
}

/*掉电顺序*/
void Display_OFF(void)
{
	_DEBUG("Display OFF\n\r");
	memset(message, 0, sizeof(message));
	memset(str, 0, sizeof(str));
	POWER_SetIxMeaRange(1, RANGE_MA);
	POWER_SetIxMeaRange(2, RANGE_MA);
	POWER_SetIxMeaRange(4, RANGE_MA);
	POWER_SetIxMeaRange(5, RANGE_MA);
	S10_StopAlarm();
	MONITOR_SetBee(0);
	sleep_flag = 0;

	GPIO_ResetBits(GPIOA, GPIO_Pin_4);
	AVDD_EN_0();
	POWER_SetVDD(7, 0);
	POWER_SetVDD(6, 0);
	POWER_SetVDD(5, 0);
	POWER_SetVDD(4, 0);

	SPI_Write(0xFE, 0x00);
	SPI_Write_cmd(0x28);
	Delay_ms(20);
	SPI_Write_cmd(0x10);
	Delay_ms(120);

	LCD_RST_0();
	Delay_ms(20);
	POWER_SetVDD(3, 0);
	Delay_ms(10);

	POWER_SetVDD(2, 0);
	POWER_SetVDD(1, 0);

	KEY_SetLED(KEY_ONOFF, KEYLED_OFF);
	display_on = 0;
	SPI_SDI = 0;
	SPI_CSX = 0;
	SPI_DCX = 0;
	SPI_SCK = 0;
}

/*休眠*/
void Sleep_In(void)
{
	_DEBUG("sleep In");
	SPI_Write_cmd(0x28);
	Delay_ms(20);
	SPI_Write_cmd(0x10);
	Delay_ms(120);
}

void Sleep_Out(void)
{

	SPI_Write_cmd(0x11);
	Delay_ms(20);
	SPI_Write_cmd(0x29);
	Delay_ms(120);
	_DEBUG("sleep Out");
}

/*写初始化代码*/
void Panel_Init(void)
{
	unsigned char i;
	unsigned char code;

	u8 read_data[4];

	/*2828进入LP 模式*/
	// SSD2828_A_CS_0();
	// SSD2828_B_CS_1();
	// SSD2828_LP();

	if(OTP_times == 0)
	{
		_DEBUG("all code init\r\n");
		code_init();
	} 
	// GammaConfig_Normal.gammaRegWriteAll(GammaConfig_Normal.gammaDefReg);
	// gammaRegReadAll_Normal();

//    bist	
//    SPI_Write(0xFE,0xD0);	
//    SPI_Write(0x4D,0x7F);	
//    SPI_Write(0x4E,0x80);	
   	
//    SPI_Write(0xFE,0x40);	
//    SPI_Write(0x54,0xAF);
	//======================== CMD1 setting ============================                                         
	SPI_Write(0xFE,0x00);     
	SPI_Write(0xC4,0x80);  //MIPI SPI_Write(0xEMOVE
	SPI_Write(0x3A,0x77);  //55:565; 77:888                                                         
	SPI_Write(0x35,0x00);                                                              
	SPI_Write(0x53,0x20);  
	SPI_Write(0x51,0xFF);                                                              
	SPI_Write(0x63,0xFF);                                                                
	SPI_Write(0x2A,0x00,0x00,0x01,0x53);                    
	SPI_Write(0x2B,0x00,0x00,0x01,0x53);                     
	SPI_Write_cmd(0x11);                                                                  
	Delay_ms(120);                                                           
	SPI_Write_cmd(0x29);       
	Delay_ms(60);                                                           
}

void code_init(void)
{
unsigned char code;
// =========================================================
//Customer : BOE 1.04-1:6MUX
//Resolution : 340RGB*340
//Driver IC : ICNA3311
//PMIC : BV6802A
//Version : V0.0 - 20210513
//Version : V0.1 - 20210708(modify power off sequence)
//Version : V0.2 - 20240920
//Version : V0.3 - 20241108
// =========================================================
//  initial code start
// ==================  CMD2 password  =========================
SPI_Write(0xFE,0x20); //Pass Word Command Description in MCS (Command2)
SPI_Write(0xF4,0x5A); //CMD1(MCS) Unlocked
SPI_Write(0xF5,0x59); //CMD1(MCS) Unlocked

// ===  ID code  ===		
SPI_Write(0xFE,0x40);		
SPI_Write(0xD8,0x33);		
SPI_Write(0xD9,0x11);		
SPI_Write(0xDA,0x00);	
 
 
//======================== QSPI setting ============================
//SPI_Write(0xFE,0x20);
//SPI_Write(0x19,0x10);//QSPI setting, MIPI remove
//SPI_Write(0x1C,0xA0);//QSPI setting, MIPI remove	

// ==================  Timing Gen  =========================
SPI_Write(0xFE,0x20);
SPI_Write(0x1A,0x05);

SPI_Write(0xFE,0x40);
SPI_Write(0x01,0x50);                                                
SPI_Write(0x02,0x55);                                                
SPI_Write(0x59,0x02);                                              
SPI_Write(0x5A,0x09);      
SPI_Write(0x5B,0x04);                                                                          
SPI_Write(0x5C,0x08);                                                                          
SPI_Write(0x70,0x02);                                             
SPI_Write(0x71,0x09);     
SPI_Write(0x72,0x04);                                                                           
SPI_Write(0x73,0x08);                                                                            

// ==================  AOD setting  ========================= 
SPI_Write(0xFE,0x40);
SPI_Write(0x5D,0x24);
SPI_Write(0x60,0x08);
SPI_Write(0x61,0x01);
SPI_Write(0x62,0x5F);
SPI_Write(0x69,0x02);

SPI_Write(0x0C,0xD7);
SPI_Write(0x0D,0xF0);

// ==================  Power Settings  ========================= 
SPI_Write(0xFE,0xE0);
SPI_Write(0x00,0x14);               
SPI_Write(0x01,0x01);
SPI_Write(0x02,0x00);
SPI_Write(0x04,0x02);   
SPI_Write(0x06,0x11);       
SPI_Write(0x08,0x00);
SPI_Write(0x09,0x14);           
SPI_Write(0x0A,0x01);
SPI_Write(0x0B,0x00);                 
SPI_Write(0x0C,0x02);                                 
SPI_Write(0x0E,0x11);              
SPI_Write(0x0F,0x00);                               
SPI_Write(0x10,0x14);             
SPI_Write(0x11,0x10);              
SPI_Write(0x21,0x8F);                 
SPI_Write(0x2D,0x8F);               
SPI_Write(0x32,0x8F);                        
SPI_Write(0x24,0x01);                                      
SPI_Write(0x26,0x41);                        
SPI_Write(0x22,0x1A);                            
SPI_Write(0x23,0x15);                        
SPI_Write(0x30,0x01);                          
SPI_Write(0xFE,0x40);                     
SPI_Write(0x57,0x43);         
SPI_Write(0x58,0x33);  
SPI_Write(0x6E,0x43);    
SPI_Write(0x6F,0x33);    
SPI_Write(0x74,0x43);     
SPI_Write(0x75,0x33);       
// ===  swire setting for BV6802 ===		
SPI_Write(0xFE,0x40);
SPI_Write(0x12,0xFE);
SPI_Write(0x13,0x08);
SPI_Write(0xC9,0x5D);
SPI_Write(0x96,0x38);
SPI_Write(0x97,0x02);
SPI_Write(0xA5,0xFF);
SPI_Write(0xAA,0x38);
SPI_Write(0xAB,0x5D);
SPI_Write(0x98,0x00);
SPI_Write(0xA7,0x38);
SPI_Write(0xA9,0x5D);

//swire timing setting for BV6802/4
SPI_Write(0xFE,0x80);
SPI_Write(0x66,0x11);
SPI_Write(0x67,0x17);
SPI_Write(0x68,0x04);                       

//=================== GOA mapping =============================== 
SPI_Write(0xFE,0x70);                    
SPI_Write(0x9B,0x00);                                                                   
SPI_Write(0x9C,0x01);           
SPI_Write(0x9D,0x08);           
SPI_Write(0x9E,0x19);
SPI_Write(0x9F,0x19);
SPI_Write(0xA0,0x19);
SPI_Write(0xA2,0x19);
SPI_Write(0xA3,0x19);
SPI_Write(0xA4,0x19);
SPI_Write(0xA5,0x19);
SPI_Write(0xA6,0x0E);
SPI_Write(0xA7,0x0D);
SPI_Write(0xA9,0x0C);
SPI_Write(0xAA,0x19);
SPI_Write(0xAB,0x19);
SPI_Write(0xAC,0x19);
SPI_Write(0xAD,0x19);
SPI_Write(0xAE,0x19);
SPI_Write(0xAF,0x19);
SPI_Write(0xB0,0x19);
SPI_Write(0xB1,0x19);
SPI_Write(0xB2,0x19);
SPI_Write(0xB3,0x19);
SPI_Write(0xB4,0x19);
SPI_Write(0xB5,0x19);
SPI_Write(0xB6,0x02); 
SPI_Write(0xB7,0x03); 
SPI_Write(0xB8,0x09);            
SPI_Write(0xB9,0x19);
SPI_Write(0xBA,0x19);
SPI_Write(0xBB,0x19);
SPI_Write(0xBC,0x19);
SPI_Write(0xBD,0xF9);
SPI_Write(0xBE,0x19);
SPI_Write(0xBF,0x19);
SPI_Write(0xC0,0x0F);
SPI_Write(0xC1,0x30);
SPI_Write(0xC2,0x11);
SPI_Write(0xC3,0x19);
SPI_Write(0xC4,0x19);
SPI_Write(0xC5,0x19);
SPI_Write(0xC6,0x19);
SPI_Write(0xC7,0x19);
SPI_Write(0xC8,0x19);

//====================  source/mux sequence ==============================           
SPI_Write(0xFE,0x40);
SPI_Write(0x4C,0x21);
SPI_Write(0x53,0xF0);

// =========  SD/SW_Toggle_Sequence_Control =============  
SPI_Write(0xFE,0xF0);                                                                   
SPI_Write(0x72,0x36);                                                  
SPI_Write(0x73,0x63);                                                                   
SPI_Write(0x74,0x14);                                                                   
SPI_Write(0x75,0x41);                                                                   
SPI_Write(0x76,0x25);                                                                   
SPI_Write(0x77,0x52);                                                                   
SPI_Write(0x78,0x36);                                                  
SPI_Write(0x79,0x63);                                                                   
SPI_Write(0x7A,0x14);                                                                   
SPI_Write(0x7B,0x41);                                                                   
SPI_Write(0x7C,0x25);                                                                   
SPI_Write(0x7D,0x52);                                                                                                              
SPI_Write(0x7E,0x14);                                            
SPI_Write(0x7F,0x41);                                                         
SPI_Write(0x80,0x36);                                                          
SPI_Write(0x81,0x63);                                                          
SPI_Write(0x82,0x25);                                                           
SPI_Write(0x83,0x52);                                                          
SPI_Write(0x84,0x14);                                              
SPI_Write(0x85,0x41);                                                             
SPI_Write(0x86,0x36);                                                               
SPI_Write(0x87,0x63);                                                               
SPI_Write(0x88,0x25);                                                               
SPI_Write(0x89,0x52);                 

 
//=====================   GIP Setting  =======================                                                                                                                                                                         
SPI_Write(0xFE,0x70);                                                                                                                                                                                   
SPI_Write(0x00,0xC0); //GCK1(for GCK) EN                                                          
SPI_Write(0x01,0x00);                                                             
SPI_Write(0x02,0x02);                                                             
SPI_Write(0x03,0x01);                                                             
SPI_Write(0x04,0x08);                                                             
SPI_Write(0x05,0x01);                                                             
SPI_Write(0x06,0x0C);                                                             
SPI_Write(0x07,0x01);                                                             
SPI_Write(0x08,0x08);                                                                                                                                                                   
SPI_Write(0x09,0xC0); //GCK2(for GCB) EN                                                                           
SPI_Write(0x0A,0x00);                                                                          
SPI_Write(0x0B,0x02);                                                                          
SPI_Write(0x0C,0x01);                                                                          
SPI_Write(0x0D,0x08);                                                                              
SPI_Write(0x0E,0x02);                                                                              
SPI_Write(0x0F,0x0C);                                                                              
SPI_Write(0x10,0x01);                                                                              
SPI_Write(0x11,0x08); 

SPI_Write(0x12,0xC0); //GCK3(for ECK) EN                                                                                                       
SPI_Write(0x13,0x08);                                                                                                            
SPI_Write(0x14,0x02);                                                                                                            
SPI_Write(0x15,0x00);                                                                                                            
SPI_Write(0x16,0x00);                                                                                                            
SPI_Write(0x17,0x01);                                                                                                            
SPI_Write(0x18,0x17);                                                                                                            
SPI_Write(0x19,0x17);                                                                                                            
SPI_Write(0x1A,0x08);     

SPI_Write(0x1B,0xC0); //GCK4(for ECB) EN                                                                                                                
SPI_Write(0x1C,0x08);                                                                                                                    
SPI_Write(0x1D,0x02);                                                                                                                    
SPI_Write(0x1E,0x00);                                                                                                                    
SPI_Write(0x1F,0x00);                                                                                                                    
SPI_Write(0x20,0x00);                                                                                                                    
SPI_Write(0x21,0x17);                                                                                                                    
SPI_Write(0x22,0x17);                                                                                                                    
SPI_Write(0x23,0x08); 


SPI_Write(0x4C,0x80);  //STV_en                                                                                           
SPI_Write(0x4D,0x00);                                                                                           
SPI_Write(0x4E,0x01);                                                                                            
SPI_Write(0x4F,0x00);                                                                                        
SPI_Write(0x50,0x01);                                                                                               
SPI_Write(0x51,0x93);                                                                            
SPI_Write(0x52,0x16); 

SPI_Write(0x53,0xC6); //STE_en                                                                   
SPI_Write(0x54,0x00);                                              
SPI_Write(0x55,0x03);                                                           
SPI_Write(0x56,0x01);                                               
SPI_Write(0x58,0x01);                                                
SPI_Write(0x59,0x08);                                                            
SPI_Write(0x65,0x58);                                           
SPI_Write(0x66,0x12);                                        
SPI_Write(0x67,0x00); 
  
// MUX Sequence Control                            
//******************Normal Switch timing control************************                        
SPI_Write(0xFE,0xF0);                                                                                    
SPI_Write(0xA3,0x00);

SPI_Write(0xFE,0xF0);
SPI_Write(0xA9,0x22);
SPI_Write(0xAA,0x22);
SPI_Write(0xAB,0x22);
SPI_Write(0xAC,0x22);
SPI_Write(0xAD,0x22);
SPI_Write(0xAE,0x22);

SPI_Write(0xFE,0x70);
SPI_Write(0x76,0x05);
SPI_Write(0x77,0x00);                                                                      
SPI_Write(0x78,0x05);

SPI_Write(0xFE,0x70);
SPI_Write(0x68,0x05);
SPI_Write(0x69,0x05);
SPI_Write(0x6A,0x05);
SPI_Write(0x6B,0x05);
SPI_Write(0x6C,0x05);                                                                                    
SPI_Write(0x6D,0x05); 
               
//************************idle Switch timing control**********************                                                                      
SPI_Write(0xFE,0x70);                                                                                                                                                             
SPI_Write(0x93,0x05);                                                                            
SPI_Write(0x94,0x00);
SPI_Write(0x96,0x05);
      
SPI_Write(0xFE,0x70);
SPI_Write(0xDB,0x05);
SPI_Write(0xDC,0x05);
SPI_Write(0xDD,0x05);
SPI_Write(0xDE,0x05);
SPI_Write(0xDF,0x05);
SPI_Write(0xE0,0x05);
                                                   
SPI_Write(0xE7,0x22);
SPI_Write(0xE8,0x22);
SPI_Write(0xE9,0x22);                                                                            
SPI_Write(0xEA,0x22);                                                                            
SPI_Write(0xEB,0x22);                                                                            
SPI_Write(0xEC,0x22);
 
//===================  Power on/off sequence Blank period control  ==============================                                                                                                
SPI_Write(0xFE,0x70);                                                                                                                                                                                              
SPI_Write(0xD1,0xF0);                                                            
SPI_Write(0xD2,0xFF);                                                            
SPI_Write(0xD3,0xF0);                                                            
SPI_Write(0xD4,0xFF);                                                            
SPI_Write(0xD5,0xA0);                                                            
SPI_Write(0xD6,0xAA);                                                       
SPI_Write(0xD7,0xF0);                                                            
SPI_Write(0xD8,0xFF);                                                                                                                                                                      

//===================  Source status setting in blank period  ============================== 
SPI_Write(0xFE,0x40);
SPI_Write(0x4D,0x2A);
SPI_Write(0x4E,0x00); 
SPI_Write(0x4F,0x00); 
SPI_Write(0x50,0x00);
SPI_Write(0x51,0xF3);
SPI_Write(0x52,0x23);
SPI_Write(0x6B,0xF3);
SPI_Write(0x6C,0x13);

SPI_Write(0x8F,0xFF); 
SPI_Write(0x90,0xFF);
SPI_Write(0x91,0x3F);

SPI_Write(0x07,0x21); 
SPI_Write(0x35,0x81); 

SPI_Write(0xFE,0x40);
SPI_Write(0xA2,0x10);
                                                                        
//==================== gamma setting  ==============================                                                                                                              
SPI_Write(0xFE,0x40);
SPI_Write(0x33,0x10);		
SPI_Write(0x34,0xC1);                                                                         
SPI_Write(0xFE,0x50); //GAMMA GSPI_Write(0xOUP 1 for Normal mode                                 
SPI_Write(0xA9,0x40);    //VGMP = 6V                                             
SPI_Write(0xAA,0x90);    //VGSP=2V                                           
SPI_Write(0xAB,0x01);    //bit4 = VGMN[8]=0, bit0 = VGMP[8]=0                                                                
SPI_Write(0xFE,0x60);  //GAMMA Group2 for HBM                                      
SPI_Write(0xA9,0x40);    //VGMP = 6V                                             
SPI_Write(0xAA,0x68);    //VGMN = 1.5V                                               
SPI_Write(0xAB,0x01); // VGMP[8]=1,VGMN[8]=0                                                                                 
SPI_Write(0xFE,0x30);  //GAMMA GSPI_Write(0xOUP3 for AOD                                      
SPI_Write(0xA9,0x40);    //VGMP = 6V                                             
SPI_Write(0xAA,0x90);    //VGSP=2V          
SPI_Write(0xAB,0x01);    //VGMP[8]=1,VGMN[8]=0                                     

//////======================== POWESPI_Write(0x SAVING ============================
SPI_Write(0xFE,0x70);
SPI_Write(0x98,0x74);
SPI_Write(0xC9,0x02);
SPI_Write(0xCA,0x02);
SPI_Write(0xCB,0x02);
SPI_Write(0xCC,0x02);
SPI_Write(0xCD,0x02);                                                
SPI_Write(0xCE,0x82);                                               
SPI_Write(0xCF,0x02);
SPI_Write(0xD0,0x42);

SPI_Write(0xFE,0xE0);
SPI_Write(0x19,0x43);               
SPI_Write(0x1E,0x43);      
SPI_Write(0x1C,0x41);     
SPI_Write(0x18,0x00);   
SPI_Write(0x1B,0x0C);   
SPI_Write(0x1A,0x9A);   
SPI_Write(0x1D,0xDA);
SPI_Write(0x28,0x58);		
SPI_Write(0x05,0x04);		
SPI_Write(0x0D,0x04);                                                              

SPI_Write(0xFE,0x40);
SPI_Write(0x54,0xAC);
SPI_Write(0x55,0xA0);
SPI_Write(0x48,0xAA);

SPI_Write(0xFE,0x90);
SPI_Write(0x31,0x08);  

////****************Watch EDGE 20210224**********************
SPI_Write(0xFE,0x90);
SPI_Write(0x15,0x00);
SPI_Write(0xA4,0xAA);
SPI_Write(0xA5,0xAA);
SPI_Write(0xA6,0x00);
SPI_Write(0xA7,0xAA);
SPI_Write(0xA9,0xAA);
SPI_Write(0xAA,0x80); //00 off, 80 on
SPI_Write(0xAB,0x61);
SPI_Write(0xAC,0xff);
SPI_Write(0xAD,0x00);
SPI_Write(0xAE,0x0C);

////*************** SCC setting ***********************
SPI_Write(0xFE,0x90);
SPI_Write(0x4E,0x03); //00 OFF 03 ON

SPI_Write(0x51,0x00);
SPI_Write(0x52,0x07);
SPI_Write(0x53,0x00);
SPI_Write(0x54,0x10);
SPI_Write(0x55,0x08);
SPI_Write(0x56,0x00);
SPI_Write(0x57,0x00);
SPI_Write(0x58,0x00);
SPI_Write(0x59,0x00);
SPI_Write(0x5A,0x00);
SPI_Write(0x5B,0x10);
SPI_Write(0x5C,0x08);
SPI_Write(0x5D,0xC7);
SPI_Write(0x5E,0x40);
SPI_Write(0x5F,0x00);
SPI_Write(0x60,0x00);
SPI_Write(0x61,0x80);
SPI_Write(0x62,0x10);
SPI_Write(0x63,0x00);
SPI_Write(0x64,0x00);
SPI_Write(0x65,0x00);
SPI_Write(0x66,0x00);
SPI_Write(0x67,0x00);
SPI_Write(0x68,0x40);
SPI_Write(0x69,0x08);
SPI_Write(0x6A,0x00);
SPI_Write(0x6B,0x07);

SPI_Write(0x6C,0x00);
SPI_Write(0x6D,0x40);
SPI_Write(0x6E,0x00);
SPI_Write(0x6F,0xD0);
SPI_Write(0x70,0x44);
SPI_Write(0x71,0x00);
SPI_Write(0x72,0x00);
SPI_Write(0x73,0x00);
SPI_Write(0x74,0x08);
SPI_Write(0x75,0x04);
SPI_Write(0x76,0x10);
SPI_Write(0x77,0x04);
SPI_Write(0x78,0x00);
SPI_Write(0x79,0x00);
SPI_Write(0x7A,0x00);
SPI_Write(0x7B,0x00);
SPI_Write(0x7C,0x44);
SPI_Write(0x7D,0x10);
SPI_Write(0x7E,0x00);
SPI_Write(0x7F,0x07);
SPI_Write(0x80,0x00);
SPI_Write(0x81,0x08);
SPI_Write(0x82,0x00);
SPI_Write(0x83,0x44);
SPI_Write(0x84,0x04);
SPI_Write(0x85,0x00);
SPI_Write(0x86,0x00);

SPI_Write(0x87,0x00);
SPI_Write(0x88,0x87);
SPI_Write(0x89,0x00);
SPI_Write(0x8A,0x10);
SPI_Write(0x8B,0x00);
SPI_Write(0x8C,0x00);
SPI_Write(0x8D,0x00);
SPI_Write(0x8E,0x00);
SPI_Write(0x8F,0x00);
SPI_Write(0x90,0x08);
SPI_Write(0x91,0x10);
SPI_Write(0x92,0x00);
SPI_Write(0x93,0xC7);
SPI_Write(0x94,0x40);
SPI_Write(0x95,0x00);
SPI_Write(0x96,0x00);
SPI_Write(0x97,0x08);
SPI_Write(0x98,0x10);
SPI_Write(0x99,0x00);
SPI_Write(0x9A,0x00);
SPI_Write(0x9B,0x00);
SPI_Write(0x9C,0x00);
SPI_Write(0x9D,0x00);
SPI_Write(0x9E,0x48);
SPI_Write(0x9F,0x00);
SPI_Write(0xA0,0x00);
SPI_Write(0xA2,0x07);

}

void Panel_Read(void)
{
	unsigned char EBdata,ECdata,EDdata;
	unsigned char ReadCode[2];
	  SPI_Write(0xFE, 0x20);
  	SPI_Write(0xF4, 0x5A);
  	SPI_Write(0xF5, 0x59);
	
	// SPI_Write(0xFE, 0x20);	
	// SPI_Write(0x19, 0x10);	//QSPI setting, MIPI remove
	// SPI_Write(0x1C, 0xA0);	//QSPI setting, MIPI remove

	SPI_Write(0xFE, 0x20);
	
	// ReadCode[0] = SPI_Read(0xEB);
	// EBdata = ReadCode[0];
	// _DEBUG("0xEB=0x%02x\r\n",ReadCode[0]);

	ReadCode[0] = SPI_Read(0xEC);
	ECdata = ReadCode[0];
	_DEBUG("0xEC=0x%02x\r\n",ReadCode[0]);
	
	// ReadCode[0] = SPI_Read(0xED);
	// EDdata = ReadCode[0];
	// _DEBUG("0xED=0x%02x\r\n",ReadCode[0]);	
	
	// ReadCode[0] = SPI_Read(0xEE);
	// _DEBUG("0xEE=0x%02x\r\n",ReadCode[0]);	

    switch(ECdata & 0x03)
    {
        case 0x00:
            OTP_times = 0;
        break;
        case 0x01:
            OTP_times = 1;
        break;
        case 0x03:
            OTP_times = 2;
        break;
        default:
            OTP_times = 255;
        break;
    }
	_DEBUG("OTP_times:%d\n\r",OTP_times);
}


int NVM_ALL(userContrlStruct *user)  
{
		int res=1;
		int us;
		uint8_t l_OTP_times=OTP_times;
		unsigned char ReadCode[32];
		unsigned char C9_data,C4_data,AE_data1,AE_data2,AE_data3,AE_data4;

#if GAMMA_FLAG == 0
		// ===  CMD2 password  ===
		QSPI_LCD_WriteCmd(0xFE, 0x20);
		QSPI_LCD_WriteCmd(0xF4, 0x5A);
		QSPI_LCD_WriteCmd(0xF5, 0x59);
	
	
		_DEBUG("OTP_START\r\n");

		QSPI_LCD_WriteCmd(0xFE, 0x00);
		QSPI_LCD_WriteCmd(0x11,0x00);	
		Delay_ms(120);
		QSPI_LCD_WriteCmd(0x29,0x00);	
		Delay_ms(20);
		QSPI_LCD_WriteCmd(0xFE, 0x00);
		QSPI_LCD_WriteCmd(0x28,0x00);	
		Delay_ms(150);
		POWER_SetVDD(3, 8.3);//MTP_PWR  ON
		Delay_ms(1000);
	

		QSPI_LCD_WriteCmd(0xFE,0x40); 
		QSPI_LCD_WriteCmd(0xED,0x9B);  
		QSPI_LCD_WriteCmd(0xEE,0xFF);  


		// 3）Enable OTP_Power
		QSPI_LCD_WriteCmd(0xFE,0x40); 
		QSPI_LCD_WriteCmd(0xF2,0x03);


		// 4) Star Programming Command Sequence :
		QSPI_LCD_WriteCmd(0xFE,0x40); 
		QSPI_LCD_WriteCmd(0xF3,0xA5);
		QSPI_LCD_WriteCmd(0xF4,0x5A);
		QSPI_LCD_WriteCmd(0xF5,0x3C);
		
		Delay_ms(4500);
		
		POWER_SetVDD(3, 0);//MTP_PWR  OFF
		Delay_ms(500);
		_DEBUG("Display_OFF\r\nDisplay_ON\r\n");
		Display_OFF();
		Delay_ms(500);
		Display_ON();

		if(OTP_times - l_OTP_times != 1)
		{
			_DEBUG("OTP NG %d\r\n", OTP_times - l_OTP_times);
			SDCard_Printf(scancode,"OTP NG %d\r\n", OTP_times - l_OTP_times);
			return -1;
		}
		else
		{
			_DEBUG("OTP OK %d\r\n", OTP_times - l_OTP_times);
			SDCard_Printf(scancode,"OTP OK %d\r\n", OTP_times - l_OTP_times);
		}
#endif
/************************************ReadALLcode*************************************/		
		_DEBUG("ReadALLcode Cheak Start\r\n");
		SDCard_Printf(scancode,"ReadALLcode Cheak Start\r\n");
		us=ReadALLcode();        //回读全code
		if(us<0)
		{
			res = -1;
		}
		_DEBUG("ReadALLcode Cheak end\r\n\r\n");
		SDCard_Printf(scancode,"ReadALLcode Cheak end\r\n\r\n");
/**********************************Gamma Cheak**************************************/		
		_DEBUG("Gamma Cheak Start\r\n");
		SDCard_Printf(scancode,"Gamma Cheak Start\r\n");
		us=Check_GAMMA();
		if(us>0)
		{
			_DEBUG("Gamma Cheak OK\r\n");
			SDCard_Printf(scancode,"Gamma Cheak OK\r\n");
		}
		else
		{
			_DEBUG("Gamma Cheak NG\r\n");
			SDCard_Printf(scancode,"Gamma Cheak NG\r\n");
			res = -1;
		}
		_DEBUG("Gamma Cheak end\r\n\r\n");
		SDCard_Printf(scancode,"Gamma Cheak end\r\n\r\n");

		return res;
}  


/**********************************Gamma Cheak**************************************/

int Check_GAMMA(void)
{
	float G_x, G_y, G_Lv, G_Gamma;
	int res = 1;
	SwitchFrame(White_Frame);
	Delay_ms(400);
//	if (CFL_GetMeasureXYLv(CFL_ID0,&XYLv) >= 0)
	 if (CA310_GetLvXY(CH_NUM, &G_Lv, &G_x, &G_y) >= 0)
	{
//		G_Lv = XYLv.Lv;
//		G_x = XYLv.X;
//		G_y = XYLv.Y;
		if ((G_Lv >= WNOR_LV_MIN) && (G_Lv <= WNOR_LV_MAX) && (G_x >= W_X_MIN) && (G_x <= W_X_MAX) && (G_y >= W_Y_MIN) && (G_y <= W_Y_MAX)) // 高亮卡控
		{
			_DEBUG("Normal White OK X:%.3f Y%.3f Lv%.3f\r\n", G_x, G_y, G_Lv);
			printf("Normal White OK X:%.3f Y%.3f Lv%.3f\r\n", G_x, G_y, G_Lv);
		}
		else
		{
			_DEBUG("Normal White NG X:%.3f Y%.3f Lv%.3f\r\n", G_x, G_y, G_Lv);
			printf("Normal White NG X:%.3f Y%.3f Lv%.3f\r\n", G_x, G_y, G_Lv);
			res = -1;
		}
	}
	else
	{
		res = -1;
	}
	return res;
}// 外部中断3服务程序
void EXTI3_IRQHandler(void)
{
    if (display_on == 1)
    {
        if (EXTI_GetITStatus(EXTI_Line3) != RESET)  // 判断某个线上的中断是否发生
        {
            Delay_ms(1);  // 延时10ms防止抖动影响掉电判断
            if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_3) == SET)
            {
                sateext = 0;
            }
            else
            {
                Flag_EXTI_Line0 = SET;
            }
        }
    }

    EXTI_ClearITPendingBit(EXTI_Line3);  // 清除LINE0上的中断标志位
}

void read_power(void)
{

}

