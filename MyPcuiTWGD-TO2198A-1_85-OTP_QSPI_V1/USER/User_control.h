#ifndef __USER_CONTROL_H
#define	__USER_CONTROL_H


#define OTP_Times_EN 2 //管控烧录次数的

/*******************相关的卡控设置***********************/

#define WNOR_LV_MAX		650*1.03 //W画面的最大亮度  normal模式
#define WNOR_LV_MIN		650*0.97 //W画面的最大亮度  normal模式

#define WNOR2_LV_MAX		NOR2_DEF_GAMMA_TARGET_MAX_LV+50.0F //W画面的最大亮度  normal模式
#define WNOR2_LV_MIN		NOR2_DEF_GAMMA_TARGET_MAX_LV-50.0F //W画面的最大亮度  normal模式

#define WHBM_LV_MAX 	HBM_DEF_GAMMA_TARGET_MAX_LV+50.0F //W画面的最大亮度  HBM
#define WHBM_LV_MIN 	HBM_DEF_GAMMA_TARGET_MAX_LV-50.0F //W画面的最大亮度  HBM

#define WIDLE_LV_MAX 	IDLE_DEF_GAMMA_TARGET_MAX_LV+50.0F //W画面的最大亮度  HBM
#define WIDLE_LV_MIN 	IDLE_DEF_GAMMA_TARGET_MAX_LV-50.0F //W画面的最大亮度  HBM


//色坐标一般都是每一个模式都是一样，特殊情况再另外加
#define W_X_MAX		0.29+0.003F
#define W_X_MIN		0.29-0.003F

#define W_Y_MAX		0.310+0.003F
#define W_Y_MIN		0.310-0.003F

 /*********相关的功能标志 注: 以下的宏定义如不需要的功能，直接屏蔽即可***********************/
 
//#define _AUTO_OTP_MODE //与众志的自动烧录模式，打开，是自动烧录设备上使用的，屏蔽则按确认按键烧录
 
#define _LOG_printf //打印LOG的功能 打开，则在上位机上打印，屏蔽则不打印

#define _OTP_Enable //使能烧录功能 一般会使能 在自动烧录机台上一般用不上

#define _Code_Check_Alarm //这里会开启全代码check 功能  不需要则屏蔽
 
#define _WHITE_NOR_CHECK //检测normal白画面的光学   不需要则屏蔽
#define _WHITE_HBM_CHECK //检测HBM白画面的光学 	  不需要则屏蔽
#define _WHITE_NOR2_CHECK //检测NOR2白画面的光学 	  不需要则屏蔽
#define _WHITE_idle_CHECK //检测idle白画面的光学 	  不需要则屏蔽
typedef struct
{
	/*监控报警相关变量*/

	//白点标志
	int WP_XY_Alarm;       //标志位
	int WP_LV_Alarm;       //标志位

	int _Allread_check_Alarm;// 0 表示回读code正确  1 表示回读code错误
	int current_Alarm;
	
	int OTP_Times;
	int ID1_val;
	int ID2_val;
	int ID3_val;
	
	int OTP_flag;
	int otp_ts;
	int OTP_Flag_Alarm;  //定义check项是否OK   只要有一项NG则为 1
	
	//NG事项类型 0：initial code  1: gamma code 2: OTP_times 3:normal_White_xylv  4:HBM_White_xylv
	int type_Alarm[10];
	
	int count; //一般是用于计算次数的变量
//	uint8_t R_num,G_num,B_num,FLK_num;

	float	Panel_x[3];	//mearsure White x  HBM/Normal/AOD
	float	Panel_y[3]; //mearsure White y  HBM/Normal/AOD
	float	Panel_L[3]; //mearsure White Lv HBM/Normal/AOD



}userContrlStruct;
 typedef enum {
	White_Frame = 0,
	Black_Frame,
	NG_Frame,
	OK_Frame,
}FRAME;
#endif