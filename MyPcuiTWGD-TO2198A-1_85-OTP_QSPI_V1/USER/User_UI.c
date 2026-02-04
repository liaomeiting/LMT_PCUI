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

#define MAX_PARAMS 16

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
    int x = 50, y = 50, x_spacing = 150, y_spacing = 70, box_width = 130, box_height = 50, font_size = 14;
	selectShowPage(2);
	clearWidget();
	creatButtonPage4("PowerOn", x, y, box_height, box_width, font_size);
	creatButtonPage4("PowerOFF", x+(x_spacing*1), y, box_height, box_width, font_size);
	creatButtonPage4("KEY_DOWN", x+(x_spacing*2), y, box_height, box_width, font_size);
	creatButtonPage4("KEY_UP", x+(x_spacing*3), y, box_height, box_width, font_size);
	creatButtonPage4("KEY_Enter", x+(x_spacing*4), y, box_height, box_width, font_size);

	creatButtonPage4("Auto_VCOM", x, y+(y_spacing*1), box_height, box_width, font_size);
	creatButtonPage4("Send_Cmd", x+(x_spacing*1), y+(y_spacing*1), box_height, box_width, font_size);
	creatButtonPage4("Read111", x+(x_spacing*2), y+(y_spacing*1), box_height, box_width, font_size);
	creatButtonPage4("Sleep In", x+(x_spacing*3), y+(y_spacing*1), box_height, box_width, font_size);
	creatButtonPage4("Sleep Out", x+(x_spacing*4), y+(y_spacing*1), box_height, box_width, font_size);

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

    setLineEdit_ParameterPage4(x,y+450, 150, font_size,0xffffff,1,"Read_Reg","0x01");
	creatButtonPage4("Read", x+355, 46+450, 45, 100, font_size);
    setLineEdit_ParameterPage4(50,550, 150, font_size,0xffffff,1,"result","");
}
int User_Event(void)
{
	static char pcui_cmd[512];
	static char get_line_str[2048];
	int r, g, b;
    int result,Reg;
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
				PowerOn();
			}
		}
		else if (pcui_Scan(pcui_cmd, "button:PowerOFF") >= 0)
		{
			if (display_on == 1)
			{
				PowerOFF();
			}
		}
		else if (pcui_Scan(pcui_cmd, "button:KEY_DOWN") >= 0)
		{
			KEYDOWN();
		}
		else if (pcui_Scan(pcui_cmd, "button:KEY_UP") >= 0)
		{
			KEYUP();
		}
		else if (pcui_Scan(pcui_cmd, "button:KEY_Enter") >= 0)
		{
			KEY_Enter();
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
				// SPI
				Img_Full(r, g, b);
			}
			else
			{
				setDialog("当前未点亮!",0);
			}
        }
		else if (pcui_Scan(pcui_cmd, "button:OK") >= 0)
		{
            getLineEdit_ContentPage4("Send_Cmd",get_line_str);
			parse_and_execute_spi_writes(get_line_str);
		}
		else if (pcui_Scan(pcui_cmd, "button:Read") >= 0)
		{
            getLineEdit_ContentPage4("Read_Reg",get_line_str);
			if (sscanf(get_line_str, "0x%02x", &Reg) == 1) 
			{
				result = SPI_Read(Reg); // 格式正确，调用 SPI_Read
				sprintf(get_line_str,"%s:0x%02x",get_line_str,result);
				setLineEdit_ParameterPage4(50,550, 150, 14,0xffffff,1,"result",get_line_str);
			}	
			else
			{
				setDialog("格式不符合!",0);
			}
		}
		else if (pcui_Scan(pcui_cmd, "button:Change2") >= 0)
		{
			if(display_on)
			{
				getComboBox("SwitchFrame2",get_line_str);
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

// 辅助函数：解析十六进制字符串（支持 "0xFE"、"FE" 或 "0xFE"）
int parse_hex(const char *str) {
    char buf[16] = {0};
    strncpy(buf, str, sizeof(buf) - 1);
    remove_all_spaces(buf); // 确保参数中没有空格

    uint8_t value;
    if (sscanf(buf, "0x%02hhX", &value) == 1 || sscanf(buf, "%02hhX", &value) == 1) {
        return value;
    }
    return -1;
}

void parse_and_execute_spi_writes(const char *input) {
    const char *current = input;
    char line[256] = {0};

    while (*current) {
        // 读取一行
        size_t line_len = 0;
        while (*current && *current != '\n') {
            if (line_len < sizeof(line) - 1) {
                line[line_len++] = *current;
            }
            current++;
        }
        line[line_len] = '\0';

        // 去除行中的所有空格（包括中间的空格）
        remove_all_spaces(line);

        // 跳过空行和注释行
        if (strlen(line) == 0 || strstr(line, "//") == line) {
            if (*current == '\n') current++;
            continue;
        }

        // 查找 '(' 和 ')'，提取中间的内容
        char *start = strchr(line, '(');
        char *end = strchr(line, ')');

        if (start && end && start < end) {
            start++; // 跳过 '('
            *end = '\0'; // 临时截断字符串，方便分割

            // 分割参数（以逗号分隔）
            char *params[MAX_PARAMS] = {0};
            int param_count = 0;
            char *token = strtok(start, ",");

            while (token && param_count < MAX_PARAMS) {
                params[param_count++] = token;
                token = strtok(NULL, ",");
            }

            // 恢复原始字符串（可选）
            *end = ')';

            // 解析参数并调用 SPI_Write
            if (param_count > 0) {
                uint8_t reg = parse_hex(params[0]); // 第一个参数是寄存器地址
                if (reg == (uint8_t)-1) {
                    _DEBUG("Error: Invalid register address -> %s\n", params[0]);
                    continue;
                }

                // 其余参数是数据，逐个调用 SPI_Write
                for (int i = 1; i < param_count; i++) {
                    uint8_t data = parse_hex(params[i]);
                    if (data == (uint8_t)-1) {
                        _DEBUG("Error: Invalid data -> %s\n", params[i]);
                        continue;
                    }
                    SPI_Write(reg, data); // 真正调用 SPI_Write
                    _DEBUG("0x%02x,0x%02x\r\n", reg, data);
                }
            }
        } else {
            // 如果没有括号，尝试直接解析（如 "0xFE,0xD0"）
            char *params[MAX_PARAMS] = {0};
            int param_count = 0;
            char *token = strtok(line, ",");

            while (token && param_count < MAX_PARAMS) {
                params[param_count++] = token;
                token = strtok(NULL, ",");
            }

            if (param_count > 0) {
                uint8_t reg = parse_hex(params[0]);
                if (reg == (uint8_t)-1) {
                    _DEBUG("Error: Invalid register address -> %s\n", params[0]);
                    continue;
                }

                for (int i = 1; i < param_count; i++) {
                    uint8_t data = parse_hex(params[i]);
                    if (data == (uint8_t)-1) {
                        _DEBUG("Error: Invalid data -> %s\n", params[i]);
                        continue;
                    }
                    SPI_Write(reg, data);
                }
            }
        }

        // 移动到下一行
        if (*current == '\n') current++;
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
