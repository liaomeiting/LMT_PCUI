#ifndef _USER_UI_H_
#define _USER_UI_H_

#include "math.h"
#include "sys.h"
#include "S10.h"
#include "Bus.h"
#include "sd_file.h"
#include "ssd2828.h"
#include "in_img.h"
#include "showtext.h"
#include "cfl.h"
#include "stm_spi.h"
#include "qspi_lcd.h"
#include "stdarg.h"
#include "pcui.h"
#include "in_img2.h"
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define PCUI_FLAG 1
#define SIGNAL 1 //1: SPI; 2:QSPI; 3:MIPI
#define PASS_ARRAY(...) __VA_ARGS__
typedef struct
{
	void (*poweron)(void);
	void (*poweroff)(void);
	void (*keydown)(void);
	void (*keyup)(void);
	void (*keyenter)(void);
	void (*sleepin)(void);
	void (*sleepout)(void);
	void (*autovcom)(void);
}
PcuiConfigStruct;


int pcui_setCmd(const char *format, ...);
void PowerOn(void);
void PowerOFF(void);
void KEYDOWN(void);
void KEYUP(void);
void KEY_Enter(void);
void parse_and_execute_commands(const char *input);
void User_UI(void);
int User_Event(void);
int userUI_Online(void);
int MysetLineEdit_ParameterPage4(int x1, int y1, int width, int height,int size, int bgcolor, int type, char *name, char *content);
void SPI_WriteParams(unsigned char  DT, unsigned char *params, int count);
#endif
