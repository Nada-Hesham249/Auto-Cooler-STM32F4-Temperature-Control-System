#ifndef LCD_H
#define LCD_H

#include "Std_Types.h"
#include "Gpio.h"
#include "Timer.h"

/* ================= LCD Commands ================= */
#define LCD_CLEAR_DISPLAY      0x01
#define LCD_RETURN_HOME        0x02
#define LCD_ENTRY_MODE         0x04
#define LCD_DISPLAY_CONTROL    0x08
#define LCD_FUNCTION_SET       0x20
#define LCD_SET_DDRAM_ADDR     0x80

/* ================= Entry Mode ================= */
#define LCD_ENTRY_LEFT         0x02
#define LCD_ENTRY_SHIFT_OFF    0x00

/* ================= Display Control ================= */
#define LCD_DISPLAY_ON         0x04
#define LCD_DISPLAY_OFF        0x00
#define LCD_CURSOR_OFF         0x00
#define LCD_BLINK_OFF          0x00

/* ================= Function Set ================= */
#define LCD_4BIT_MODE          0x00
#define LCD_2LINE              0x08
#define LCD_5x8_DOTS           0x00

/* ================= Pins (editable) ================= */
#define LCD_RS_PORT GPIO_D
#define LCD_RS_PIN  0

#define LCD_EN_PORT GPIO_D
#define LCD_EN_PIN  1

#define LCD_D4_PORT GPIO_D
#define LCD_D4_PIN  2

#define LCD_D5_PORT GPIO_D
#define LCD_D5_PIN  3

#define LCD_D6_PORT GPIO_D
#define LCD_D6_PIN  4

#define LCD_D7_PORT GPIO_D
#define LCD_D7_PIN  5

/* ================= API ================= */
void LCD_Init(void);
void LCD_Clear(void);
void LCD_SetCursor(uint8 row, uint8 col);
void LCD_SendCommand(uint8 cmd);
void LCD_SendData(uint8 data);
void LCD_Print(char *str);
void LCD_PrintChar(char c);
//helper
void LCD_PrintInt(uint16 value);
void LCD_PrintFloat(float value, uint8 precision);
void LCD_PrintCentered(uint8 row, char *str);

#endif
