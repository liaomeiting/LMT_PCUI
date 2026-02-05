/**
* @file User_UI.c
 * @Author: MiTing(廖美婷)
 * @brief 上位机实现基础功能(目前只实现SPI信号)
 * @note 可用上位机实时切所需画面(纯色或该程序所含绘制画面)，实时发送指令及回读寄存器
 * @Version: V0.0.1
 * @Last Updated: 2026/02/04
 */
#include "User_UI.h"
extern unsigned int display_on;
extern u8 frame;
extern unsigned int frame_max;
extern u8 auto_switch_mode;
extern unsigned int Alarm_STA;
extern unsigned int frame_hold_cnt;
extern unsigned int frame_hold_ms[];
extern int yanstate;
extern InImg2_Adapter img2_adapter;
extern void Sleep_In(void);
extern void Sleep_Out(void);
PcuiConfigStruct UserPcuiConfig;
#define MAX_PARAMS 20  // 支持最多n个参数

void PowerOn(void)
{
	Spi_Init();
	frame = 0;
	Alarm_STA = 1;
	yanstate = 0;
	Display_ON();
	ReadALLcode();
}

void PowerOFF(void)
{
	Display_OFF();
	Power_SetMonitorStringName(1, "    ");
	Power_SetMonitorStringName(2, "    ");
	Power_SetMonitorStringName(3, "    ");
	Power_SetMonitorStringName(4, "    ");
}

void KEYDOWN(void)
{
	if ((auto_switch_mode == 1) || (display_on == 0) || (Alarm_STA == 0))
	{
		return;
	} // 在自动切换画面 或 DisplayOFF此按键无

	//
	//				/////新的锁秒方式
	if (GetMeasureTime_ms(frame_hold_cnt) < frame_hold_ms[frame])
		return; // 画面保持时间处理
	frame_hold_cnt = MeasureTimeStart_ms();

	if (++frame >= frame_max)
	{
		// 									Display_OFF();
		// //									GPIO_ResetBits(GPIOB, GPIO_Pin_5);////这个脚接强制使能了，需要强关
		// //									GPIO_ResetBits(GPIOA, GPIO_Pin_4);////这个脚接强制使能了，需要强关
		// 									Power_SetMonitorStringName(1,"    ");
		// 									Power_SetMonitorStringName(2,"    ");
		// 									Power_SetMonitorStringName(3,"    ");
		// 									Power_SetMonitorStringName(4,"    ");
		frame = 0;
	}
	SwitchFrame(frame);
}

void KEYUP(void)
{
	if ((auto_switch_mode == 1) || (display_on == 0) || (Alarm_STA == 0))
	{
		return;
	} // 在自动切换画面 或 DisplayOFF此按键无

	/////新的锁秒方式
	if (GetMeasureTime_ms(frame_hold_cnt) < frame_hold_ms[frame])
		return; // 画面保持时间处理
	frame_hold_cnt = MeasureTimeStart_ms();

	if (frame == 0)
	{
		frame = frame_max - 1;
	}
	else
	{
		frame--;
	}

	SwitchFrame(frame);
}

void KEY_Enter(void)
{
	if ((auto_switch_mode == 1) || (display_on == 0))
	{
		return;
	} // 在自动切换画面 或 DisplayOFF此按键
	KEY_SetLED(KEY_OTP, KEYLED_ON);

	KEY_SetLED(KEY_OTP, KEYLED_OFF);
}

void User_UI(void)
{
	// 所有按钮名称数组
    char* buttonNames[] = {
        "PowerOn", "PowerOFF", "KEY_DOWN", "KEY_UP", "KEY_Enter",
        "Sleep In", "Sleep Out"
    };
	const int totalButtons = sizeof(buttonNames) / sizeof(buttonNames[0]);
	const int buttonsPerRow = 5;  // 每行n个按钮
    int x = 50, y = 50, x_spacing = 150, y_spacing = 70, box_width = 130, box_height = 50, font_size = 14;
	selectShowPage(2);
	clearWidget();
	for (int i = 0; i < totalButtons; i++) {
        int row = i / buttonsPerRow;
        int col = i % buttonsPerRow;
        
        creatButtonPage4(
            buttonNames[i],
            x + (col * x_spacing),   
            y + (row * y_spacing),   
            box_height,
            box_width,
            font_size
        );
    }

    setLineEdit_ParameterPage4(x,y+(y_spacing*2), 150, font_size,0xffffff,1,"SwitchFrame","255,255,255");
	creatButtonPage4("Change", x+315, 46+(y_spacing*2), 45, 100, font_size);
	addComboBox(x+450, 55+(y_spacing*2), x+450+160, 55+(y_spacing*2)+80,font_size,"SwitchFrame2","Img_Gray256_V");
	addComboBox(x+450, 55+(y_spacing*2), x+450+160, 55+(y_spacing*2)+80,font_size,"SwitchFrame2","Img_Gray256_H");
	addComboBox(x+450, 55+(y_spacing*2), x+450+160, 55+(y_spacing*2)+80,font_size,"SwitchFrame2","Img_CT");
	addComboBox(x+450, 55+(y_spacing*2), x+450+160, 55+(y_spacing*2)+80,font_size,"SwitchFrame2","Img_Box");
	addComboBox(x+450, 55+(y_spacing*2), x+450+160, 55+(y_spacing*2)+80,font_size,"SwitchFrame2","Img_Chcker58");
	addComboBox(x+450, 55+(y_spacing*2), x+450+160, 55+(y_spacing*2)+80,font_size,"SwitchFrame2","Img_ColorBar");
	addComboBox(x+450, 55+(y_spacing*2), x+450+160, 55+(y_spacing*2)+80,font_size,"SwitchFrame2","Img_Flicker");
	creatButtonPage4("Change2", x+450+180, 46+(y_spacing*2), 45, 100, font_size);

	MysetLineEdit_ParameterPage4(x,y+(y_spacing*2.6),600,250,font_size,0xffffff,1,"Send_Cmd","");
	creatButtonPage4("OK", x+760, y+(y_spacing*2.6)+215, 45, 100, font_size);

    setLineEdit_ParameterPage4(x,y+450, 150, font_size,0xffffff,1,"Read_Reg","0x01,1");
	creatButtonPage4("Read", x+355, 46+450, 45, 100, font_size);
    setLineEdit_ParameterPage4(50,550, 300, font_size,0xffffff,1,"result","");

	UserPcuiConfig.poweron = PowerOn;
	UserPcuiConfig.poweroff = PowerOFF;
	UserPcuiConfig.keydown = KEYDOWN;
	UserPcuiConfig.keyenter = KEY_Enter;
	UserPcuiConfig.keyup = KEYUP;
	UserPcuiConfig.sleepin = Sleep_In;
	UserPcuiConfig.sleepout = Sleep_Out;
}
int User_Event(void)
{
	static char pcui_cmd[512];
	static char get_line_str[2048];
	uint8_t frame_Set = 50; //MIPI
	int r, g, b;
    int result,len;
	uint8_t ReadCode[12];
	uint16_t ReadCode1[12];
    uint8_t Reg;
	int online = userUI_Online(); // 检测上位机是否在线
	if (online >= 0 && pcui_getCmd(pcui_cmd) >= 0)
	{
		_DEBUG("%s\r\n", pcui_cmd);
		memset(get_line_str,0,sizeof(get_line_str));
		////////////////////// 上位机按钮处理 ///////////////////////////
		if (pcui_Scan(pcui_cmd, "button:PowerOn") >= 0)
		{
			if (display_on == 0)
			{
				UserPcuiConfig.poweron();
			}
		}
		else if (pcui_Scan(pcui_cmd, "button:PowerOFF") >= 0)
		{
			if (display_on == 1)
			{
				UserPcuiConfig.poweroff();
			}
		}
		else if (pcui_Scan(pcui_cmd, "button:KEY_DOWN") >= 0)
		{
			UserPcuiConfig.keydown();
		}
		else if (pcui_Scan(pcui_cmd, "button:KEY_UP") >= 0)
		{
			UserPcuiConfig.keyup();
		}
		else if (pcui_Scan(pcui_cmd, "button:KEY_Enter") >= 0)
		{
			UserPcuiConfig.keyenter();
		}
		else if (pcui_Scan(pcui_cmd, "button:Sleep In") >= 0)
		{
			UserPcuiConfig.sleepin();
		}
		else if (pcui_Scan(pcui_cmd, "button:Sleep Out") >= 0)
		{
			UserPcuiConfig.sleepout();
		}
		else if (pcui_Scan(pcui_cmd, "button:Change") >= 0)
		{
            getLineEdit_ContentPage4("SwitchFrame",get_line_str);
			_DEBUG("%s\r\n",get_line_str);
			result = sscanf(get_line_str, "%d,%d,%d", &r, &g, &b);
			if (result != 3) {
				setDialog("格式不符合!",0);
			}
			if(display_on==1)
			{
#if SIGNAL==1 // SPI
				Img_Full(r, g, b);
#elif SIGNAL==2 // QSPI
				QSPI_LCD_CS_AllOut(QSPI_LCD_A_CS_BIT, 0);
				InImg2_Full(&img2_adapter,r,g,b);
				QSPI_LCD_CS_AllOut(QSPI_LCD_A_CS_BIT, 1); 
#elif SIGNAL==3 // MIPI
				GPU_LoadFrame(frame_Set); // 1  normal模式
				Img_Full(r, g, b);
				GPU_DisplayFrame(frame_Set);
#endif
			}
			else
			{
				setDialog("当前未点亮!",0);
			}
        }
		else if (pcui_Scan(pcui_cmd, "button:OK") >= 0)
		{
            getLineEdit_ContentPage4("Send_Cmd",get_line_str);
			parse_and_execute_commands(get_line_str);
		}
		else if (pcui_Scan(pcui_cmd, "button:Read") >= 0)
		{
            getLineEdit_ContentPage4("Read_Reg",get_line_str);
			if (sscanf(get_line_str, "0x%x,%d", &Reg, &len) == 2) 
			{
                if(len>30 || len< 1 ) setDialog("回读长度不符合",0);
                else
                {
#if SIGNAL==1
                    SPI_Read_Buffer(Reg,ReadCode1,len); 
#elif SIGNAL==2
                    QSPI_LCD_ReadData(Reg,len,ReadCode);
#elif SIGNAL==3
                    SSD2828_DcsReadDT06(Reg,len,ReadCode);
#endif
                    sprintf(get_line_str,"%s:",get_line_str);
                    for (uint8_t i = 0; i < len; i++)
                    {
#if SIGNAL==1
                        if(i == len-1) sprintf(get_line_str,"%s0x%02x",get_line_str,ReadCode1[i]);
                        else sprintf(get_line_str,"%s0x%02x,",get_line_str,ReadCode1[i]);
#else
                        if(i == len-1) sprintf(get_line_str,"%s0x%02x",get_line_str,ReadCode[i]);
                        else sprintf(get_line_str,"%s0x%02x,",get_line_str,ReadCode[i]);
#endif
					}				
                    setLineEdit_ParameterPage4(50,550, 300, 14,0xffffff,1,"result",get_line_str);
                }
			}	
			else
			{
				setDialog("格式不符合!",0);
                _DEBUG("%s\r\n",get_line_str);
			}
		}
		else if (pcui_Scan(pcui_cmd, "button:Change2") >= 0)
		{
			if(display_on)
			{
				getComboBox("SwitchFrame2",get_line_str);
#if SIGNAL==1
                if(strcmp(get_line_str,"Img_Gray256_V")==0)
				{
					Img_Gray256_V();
				}
				else if(strcmp(get_line_str,"Img_Gray256_H")==0)
				{
					Img_Gray256_H();
				}
				else if(strcmp(get_line_str,"Img_CT")==0)
				{
					Img_CT();
				}
				else if(strcmp(get_line_str,"Img_Box")==0)
				{
					Img_Box();
				}
				else if(strcmp(get_line_str,"Img_Chcker58")==0)
				{
					Img_Chcker58();
				}
				else if(strcmp(get_line_str,"Img_ColorBar")==0)
				{
					Img_ColorBar();
				}
				else if(strcmp(get_line_str,"Img_Flicker")==0)
				{
					Img_Flicker();
				}
#elif SIGNAL==2
                if(strcmp(get_line_str,"Img_Gray256_V")==0)
				{
                    QSPI_LCD_CS_AllOut(QSPI_LCD_A_CS_BIT, 0);
					InImg2_Gray256_V(&img2_adapter);
                    QSPI_LCD_CS_AllOut(QSPI_LCD_A_CS_BIT, 1); 
				}
				else if(strcmp(get_line_str,"Img_Gray256_H")==0)
				{
                    QSPI_LCD_CS_AllOut(QSPI_LCD_A_CS_BIT, 0);
					InImg2_Gray256_H(&img2_adapter);
                    QSPI_LCD_CS_AllOut(QSPI_LCD_A_CS_BIT, 1); 
				}
				else if(strcmp(get_line_str,"Img_CT")==0)
				{
                    QSPI_LCD_CS_AllOut(QSPI_LCD_A_CS_BIT, 0);
					InImg2_CT(&img2_adapter);
                    QSPI_LCD_CS_AllOut(QSPI_LCD_A_CS_BIT, 1); 
				}
				else if(strcmp(get_line_str,"Img_Box")==0)
				{
                    QSPI_LCD_CS_AllOut(QSPI_LCD_A_CS_BIT, 0);
					InImg2_Box(&img2_adapter);
                    QSPI_LCD_CS_AllOut(QSPI_LCD_A_CS_BIT, 1); 
				}
				else if(strcmp(get_line_str,"Img_Chcker58")==0)
				{
                    QSPI_LCD_CS_AllOut(QSPI_LCD_A_CS_BIT, 0);
					InImg2_Chcker58(&img2_adapter);
                    QSPI_LCD_CS_AllOut(QSPI_LCD_A_CS_BIT, 1); 
				}
				else if(strcmp(get_line_str,"Img_ColorBar")==0)
				{
                    QSPI_LCD_CS_AllOut(QSPI_LCD_A_CS_BIT, 0);
					InImg2_ColorBar(&img2_adapter);
                    QSPI_LCD_CS_AllOut(QSPI_LCD_A_CS_BIT, 1); 
				}
				else if(strcmp(get_line_str,"Img_Flicker")==0)
				{
                    QSPI_LCD_CS_AllOut(QSPI_LCD_A_CS_BIT, 0);
					InImg2_Flicker(&img2_adapter);
                    QSPI_LCD_CS_AllOut(QSPI_LCD_A_CS_BIT, 1); 
				}
#elif SIGNAL==3
                if(strcmp(get_line_str,"Img_Gray256_V")==0)
				{
					GPU_LoadFrame(frame_Set); // 1  normal模式
					Img_Gray256_V();
					GPU_DisplayFrame(frame_Set);
				}
				else if(strcmp(get_line_str,"Img_Gray256_H")==0)
				{
					GPU_LoadFrame(frame_Set); // 1  normal模式
					Img_Gray256_H();
					GPU_DisplayFrame(frame_Set);
				}
				else if(strcmp(get_line_str,"Img_CT")==0)
				{
					GPU_LoadFrame(frame_Set); // 1  normal模式
					Img_CT();
					GPU_DisplayFrame(frame_Set);
				}
				else if(strcmp(get_line_str,"Img_Box")==0)
				{
					GPU_LoadFrame(frame_Set); // 1  normal模式
					Img_Box();
					GPU_DisplayFrame(frame_Set);
				}
				else if(strcmp(get_line_str,"Img_Chcker58")==0)
				{
					GPU_LoadFrame(frame_Set); // 1  normal模式
					Img_Chcker58();
					GPU_DisplayFrame(frame_Set);
				}
				else if(strcmp(get_line_str,"Img_ColorBar")==0)
				{
					GPU_LoadFrame(frame_Set); // 1  normal模式
					Img_ColorBar();
					GPU_DisplayFrame(frame_Set);
				}
				else if(strcmp(get_line_str,"Img_Flicker")==0)
				{
					GPU_LoadFrame(frame_Set); // 1  normal模式
					Img_Flicker();
					GPU_DisplayFrame(frame_Set);
				}
#endif
				else
				{
					setDialog("函数未添加!",0);
				}
			}
			else
			{
				setDialog("当前未点亮!",0);
			}
		}
	}
	return online;
}
int userUI_Online(void)
{
	static int online = -1;
	static uint8_t bee = 0;
	static uint32_t ts;
	if (GetMeasureTime_ms(ts) > 700) // 每 700 ms检查上位机是否在线
	{
		if (online >= 0) // 在线时检测
		{
			online = pcuiDetection();
		}
		else // 上位机不在线
		{
			// 屏幕先下电
			// if (display_on) Display_OFF();
			// 蜂鸣器报警
			// MONITOR_SetBee(bee = !bee);
			// 连接上位机
			online = pcuiDetection();
			if (online >= 0) // 上位机连接成功
			{
				MONITOR_SetBee(bee = 0);
				_DEBUG("pcui_init ---> OK\r\n");
				User_UI(); // 上位机界面初始化
			}
			else
			{
				_DEBUG("pcui_init ---> NG\r\n");
			}
		}
		ts = MeasureTimeStart_ms(); // 重新计时
	}
	return online;
}
int MysetLineEdit_ParameterPage4(int x1, int y1, int width, int height,int size, int bgcolor, int type, char *name, char *content)
{
	if((pcui.pcuiRecType.dev == 0) || (pcui.communication_flag == 1))
		return -1;
	
	char dataStr[128] = {0};
	
	sprintf(dataStr,"%d,%d$%d$%d$%d$#%06X$%d$",x1, y1, width, height, size, bgcolor, type);
	
	char *setDataStr[] = {"createLineEditPage4$", dataStr, name, "$", content};
	
	return pcui_recData(&pcui.pcuiRecType, setDataStr, sizeof(setDataStr) / sizeof(setDataStr[0]), 0, 2000);
}


// 辅助函数：去除字符串中的所有空格
void remove_all_spaces(char *str) {
    if (!str || *str == '\0') return;

    char *dest = str;
    for (char *src = str; *src; src++) {
        if (!isspace((unsigned char)*src)) {
            *dest++ = *src;
        }
    }
    *dest = '\0';
}

// 辅助函数：解析十六进制字符串（支持 "0xFE"、"FE" 或 "FE"）
// 返回 -1 表示失败，否则返回 0~255 的值
int parse_hex(const char *str, uint8_t *out_value) {
    if (!str || !out_value) return -1;

    char buf[16] = {0};
    strncpy(buf, str, sizeof(buf) - 1);
    remove_all_spaces(buf); // 确保参数中没有空格

    uint32_t value;
    if (sscanf(buf, "0x%x", &value) == 1 || sscanf(buf, "%x", &value) == 1) {
        if (value <= 0xFF) {  // 确保值在 0~255 范围内
            *out_value = (uint8_t)value;
            return 0; // 成功
        }
    }
    return -1; // 解析失败
}

// 解析并执行 SPI 或 QSPI 指令
void parse_and_execute_commands(const char *input) {
    if (!input) return;

    const char *current = input;
    char line[128] = {0};

    while (*current) {
        // 读取一行（跳过前导空格）
        size_t line_len = 0;
        while (*current && *current != '\n') {
            if (line_len < sizeof(line) - 1) {
                line[line_len++] = *current;
            }
            current++;
        }
        line[line_len] = '\0';
        if (*current == '\n') current++; // 跳过换行符

        // 去除整行空格和注释
        remove_all_spaces(line);
        if (strlen(line) == 0 || strstr(line, "//") == line) {
            continue; // 跳过空行和注释
        }

        // 检查指令类型
        bool is_spi = (strstr(line, "SPI_Write(") == line);
        bool is_qspi = (strstr(line, "QSPI_LCD_WriteCmd(") == line);
		bool is_mipi = (strstr(line, "MIPI_WR(") == line);

        if (!is_spi && !is_qspi && !is_mipi) {
            continue; // 不是有效指令行
        }

        // 提取括号内的参数
        char *start = strchr(line, '(');
        char *end = strchr(line, ')');

        if (!start || !end || start >= end) {
            _DEBUG("Error: Invalid syntax -> %s\n", line);
            continue;
        }

        start++; // 跳过 '('
        *end = '\0'; // 临时截断字符串

        // 分割参数（以逗号分隔）
        char *params[MAX_PARAMS] = {0};
        int param_count = 0;
        char *token = strtok(start, ",");

        while (token && param_count < MAX_PARAMS) {
            params[param_count++] = token;
            token = strtok(NULL, ",");
        }

        *end = ')'; // 恢复原始字符串

        // 解析参数并调用对应函数
        uint8_t parsed_params[MAX_PARAMS] = {0};
        bool valid = true;

        for (int i = 0; i < param_count; i++) {
            if (parse_hex(params[i], &parsed_params[i]) != 0) {
                _DEBUG("Error: Invalid parameter -> %s\n", params[i]);
                valid = false;
                break;
            }
        }

        if (valid) {
            if (is_spi && param_count >= 2) {
				SPI_WriteParams(parsed_params[0],&parsed_params[1],param_count-1);
            } 
            else if (is_qspi && param_count >= 2) {
                if (param_count == 2) {
                    QSPI_LCD_WriteCmd(parsed_params[0], parsed_params[1]);
                } 
                else if (param_count == 3) {
                    QSPI_LCD_WriteCmd(parsed_params[0], parsed_params[1], parsed_params[2]);
                } 
                else {
                    _DEBUG("Error: Invalid parameter count for QSPI_LCD_WriteCmd\n");
                }
            }
			else if (is_mipi && param_count >= 2) {
                // 第一个参数是数据类型(DT)
                uint8_t dt = parsed_params[0];
                // 第二个参数是命令(cmd)
                uint8_t cmd = parsed_params[1];
                // 剩余参数是数据
                uint8_t data[MAX_PARAMS - 2];
                int data_count = param_count - 2;
                
                // 复制数据部分
                for (int i = 0; i < data_count; i++) {
                    data[i] = parsed_params[i + 2];
                }
                
                // 调用MIPI写入函数
                MIPI_WrArray(dt, cmd, data_count, data);
                
                // 调试输出格式化的MIPI指令
                _DEBUG("MIPI_WR(0x%02X, 0x%02X", dt, cmd);
                for (int i = 0; i < data_count; i++) {
                    _DEBUG(", 0x%02X", data[i]);
                }
                _DEBUG(");\n");
            }
        }
    }
}
int pcui_setCmd(const char *format, ...)
{
    /* 没有初始化或已经初始化但没接线或ARM板端没反应超时 */
    if ((pcui.pcuiRecType.dev == 0) || (pcui.communication_flag == 1))
        return -1;

    char buffer[256]; // 假设一个合理的缓冲区大小
    va_list args;
    
 	pcui.dataStr[0] = 0;
   // 初始化可变参数列表
    va_start(args, format);
    
    // 将格式化字符串和参数写入缓冲区
    vsprintf(pcui.dataStr, format, args);
    
    // 清理可变参数列表
    va_end(args);

    char *setDataStr[] = {pcui.dataStr};
    
    return pcui_recData(&pcui.pcuiRecType, setDataStr, sizeof(setDataStr) / sizeof(setDataStr[0]), 0, 2000);
}
void SPI_WriteParams(unsigned char  DT, unsigned char *params, int count) 
{
    if (count == 0) return;
    if (count == 1) {
        SPI_SendX(DT, params[0], -1);
    } else {
        // 递归或循环展开（需根据 SPI_SendX 的实际行为调整）
        for (int i = 0; i < count; i++) {
            SPI_SendX(DT, params[i], (i == count - 1) ? -1 : 0); // 假设 -1 是终止符
        }
    }
}

/*需要放入main.c的代码*/

// 放在main函数while循环前的初始化
// #if PCUI_FLAG
// 	while (pcui_init(driverUSART1_Init()) != 0)
// 	{
// 		_DEBUG("pcui_init USART1 NG\r\n");
// 	} // 绑定串口
// 	_DEBUG("pcui_init USART1 OK\r\n");
// 	User_UI();
// #endif

// 放在main函数while循环中
// #if PCUI_FLAG
//         User_Event();
// #endif
