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



// #include "lcd.h"
// #include "Timer.h"
//
// /* ===== simple delay ===== */
// static void delay(volatile uint32 t)
// {
//     while(t--) {
//         __asm__("nop");
//     }
// }
//
// /* ===== EN pulse ===== */
// static void LCD_Enable(void)
// {
//     Gpio_WritePin(LCD_EN_PORT, LCD_EN_PIN, 1);
//     delay(3000);
//     Gpio_WritePin(LCD_EN_PORT, LCD_EN_PIN, 0);
//     delay(3000);
// }
//
// /* ===== send 4 bits ===== */
// static void LCD_Send4Bits(uint8 data)
// {
//     Gpio_WritePin(LCD_D4_PORT, LCD_D4_PIN, (data >> 0) & 1);
//     Gpio_WritePin(LCD_D5_PORT, LCD_D5_PIN, (data >> 1) & 1);
//     Gpio_WritePin(LCD_D6_PORT, LCD_D6_PIN, (data >> 2) & 1);
//     Gpio_WritePin(LCD_D7_PORT, LCD_D7_PIN, (data >> 3) & 1);
//
//     LCD_Enable();
// }
//
// /* ===== send command ===== */
// static void LCD_Cmd(uint8 cmd)
// {
//     Gpio_WritePin(LCD_RS_PORT, LCD_RS_PIN, 0);
//
//     LCD_Send4Bits(cmd >> 4);
//     LCD_Send4Bits(cmd & 0x0F);
//
//     delay(5000);
// }
//
// /* ===== send data ===== */
// void LCD_PrintChar(char c)
// {
//     Gpio_WritePin(LCD_RS_PORT, LCD_RS_PIN, 1);
//
//     LCD_Send4Bits(c >> 4);
//     LCD_Send4Bits(c & 0x0F);
//
//     delay(3000);
// }
//
// /* ===== init ===== */
// void LCD_Init(void)
// {
//     Gpio_Init(LCD_RS_PORT, LCD_RS_PIN, GPIO_OUTPUT, GPIO_PUSH_PULL);
//     Gpio_Init(LCD_EN_PORT, LCD_EN_PIN, GPIO_OUTPUT, GPIO_PUSH_PULL);
//
//     Gpio_Init(LCD_D4_PORT, LCD_D4_PIN, GPIO_OUTPUT, GPIO_PUSH_PULL);
//     Gpio_Init(LCD_D5_PORT, LCD_D5_PIN, GPIO_OUTPUT, GPIO_PUSH_PULL);
//     Gpio_Init(LCD_D6_PORT, LCD_D6_PIN, GPIO_OUTPUT, GPIO_PUSH_PULL);
//     Gpio_Init(LCD_D7_PORT, LCD_D7_PIN, GPIO_OUTPUT, GPIO_PUSH_PULL);
//
//     delay(50000);
//
//     /* init sequence */
//     LCD_Send4Bits(0x03);
//     delay(5000);
//     LCD_Send4Bits(0x03);
//     delay(5000);
//     LCD_Send4Bits(0x03);
//     LCD_Send4Bits(0x02);
//
//     LCD_Cmd(0x28); // 4-bit, 2 lines
//     LCD_Cmd(0x0C); // display ON
//     LCD_Cmd(0x06); // entry mode
//     LCD_Cmd(0x01); // clear
// }
//
// /* ===== clear ===== */
// void LCD_Clear(void)
// {
//     LCD_Cmd(0x01);
//     delay(5000);
// }
//
// /* ===== cursor ===== */
// void LCD_SetCursor(uint8 row, uint8 col)
// {
//     uint8 addr = (row == 0) ? 0x80 + col : 0xC0 + col;
//     LCD_Cmd(addr);
// }
//
// /* ===== print string ===== */
// void LCD_Print(char *str)
// {
//     while(*str)
//     {
//         LCD_PrintChar(*str++);
//     }
// }
//

