#include "lcd.h"

/* ================= Internal Delay Wrapper ================= */
static void LCD_Delay(uint32 ms)
{
    for (volatile int i=0;i<1000;i++);
}

/* ================= EN Pulse ================= */
static void LCD_EnablePulse(void)
{
    Gpio_WritePin(LCD_EN_PORT, LCD_EN_PIN, 1);
    LCD_Delay(1);
    Gpio_WritePin(LCD_EN_PORT, LCD_EN_PIN, 0);
    LCD_Delay(1);
}

/* ================= Send 4 bits ================= */
static void LCD_Write4Bits(uint8 data)
{
    Gpio_WritePin(LCD_D4_PORT, LCD_D4_PIN, (data >> 0) & 1);
    Gpio_WritePin(LCD_D5_PORT, LCD_D5_PIN, (data >> 1) & 1);
    Gpio_WritePin(LCD_D6_PORT, LCD_D6_PIN, (data >> 2) & 1);
    Gpio_WritePin(LCD_D7_PORT, LCD_D7_PIN, (data >> 3) & 1);

    LCD_EnablePulse();
}

/* ================= Send Byte ================= */
static void LCD_Write(uint8 value, uint8 mode)
{
    Gpio_WritePin(LCD_RS_PORT, LCD_RS_PIN, mode);

    LCD_Write4Bits(value >> 4);
    LCD_Write4Bits(value & 0x0F);
}

/* ================= Public: Command ================= */
void LCD_SendCommand(uint8 cmd)
{
    LCD_Write(cmd, 0);
    LCD_Delay(2);
}

/* ================= Public: Data ================= */
void LCD_SendData(uint8 data)
{
    LCD_Write(data, 1);
}

/* ================= Init ================= */
void LCD_Init(void)
{
    /* GPIO init */
    Gpio_Init(LCD_RS_PORT, LCD_RS_PIN, GPIO_OUTPUT, GPIO_PUSH_PULL);
    Gpio_Init(LCD_EN_PORT, LCD_EN_PIN, GPIO_OUTPUT, GPIO_PUSH_PULL);

    Gpio_Init(LCD_D4_PORT, LCD_D4_PIN, GPIO_OUTPUT, GPIO_PUSH_PULL);
    Gpio_Init(LCD_D5_PORT, LCD_D5_PIN, GPIO_OUTPUT, GPIO_PUSH_PULL);
    Gpio_Init(LCD_D6_PORT, LCD_D6_PIN, GPIO_OUTPUT, GPIO_PUSH_PULL);
    Gpio_Init(LCD_D7_PORT, LCD_D7_PIN, GPIO_OUTPUT, GPIO_PUSH_PULL);

    LCD_Delay(50);

    /* Init sequence */
    LCD_Write4Bits(0x03);
    LCD_Delay(5);

    LCD_Write4Bits(0x03);
    LCD_Delay(5);

    LCD_Write4Bits(0x03);
    LCD_Delay(1);

    LCD_Write4Bits(0x02);

    /* Config */
    LCD_SendCommand(LCD_FUNCTION_SET | LCD_4BIT_MODE | LCD_2LINE | LCD_5x8_DOTS);
    LCD_SendCommand(LCD_DISPLAY_CONTROL | LCD_DISPLAY_ON | LCD_CURSOR_OFF | LCD_BLINK_OFF);
    LCD_SendCommand(LCD_ENTRY_MODE | LCD_ENTRY_LEFT | LCD_ENTRY_SHIFT_OFF);

    LCD_Clear();
}

/* ================= Clear ================= */
void LCD_Clear(void)
{
    LCD_SendCommand(LCD_CLEAR_DISPLAY);
    LCD_Delay(2);
}

/* ================= Cursor ================= */
void LCD_SetCursor(uint8 row, uint8 col)
{
    uint8 addr = (row == 0) ? col : (0x40 + col);
    LCD_SendCommand(LCD_SET_DDRAM_ADDR | addr);
}

/* ================= Print Char ================= */
void LCD_PrintChar(char c)
{
    LCD_SendData(c);
}

/* ================= Print String ================= */
void LCD_Print(char *str)
{
    while (*str)
    {
        LCD_PrintChar(*str++);
    }
}

//helper
void LCD_PrintInt(uint16 value)
{
    char buf[6];
    int i = 0;

    if (value == 0)
    {
        LCD_PrintChar('0');
        return;
    }

    while (value > 0 && i < 5)
    {
        buf[i++] = (value % 10) + '0';
        value /= 10;
    }

    while (i > 0)
    {
        LCD_PrintChar(buf[--i]);
    }
}
void LCD_PrintFloat(float value, uint8 precision)
{
    uint16 int_part = (uint16)value;
    float frac = value - (float)int_part;

    LCD_PrintInt(int_part);

    LCD_PrintChar('.');

    while (precision--)
    {
        frac *= 10;
        uint8 digit = (uint8)frac;
        LCD_PrintChar(digit + '0');
        frac -= digit;
    }
}
void LCD_PrintCentered(uint8 row, char *str)
{
    uint8 len = 0;
    uint8 i = 0;
    uint8 start;

    while (str[len] != '\0' && len < 16)
        len++;

    if (len >= 16)
    {
        LCD_SetCursor(row, 0);
        LCD_Print(str);
        return;
    }

    start = (16 - len) / 2;

    LCD_SetCursor(row, 0);

    for (i = 0; i < start; i++)
        LCD_PrintChar(' ');

    LCD_Print(str);
}

