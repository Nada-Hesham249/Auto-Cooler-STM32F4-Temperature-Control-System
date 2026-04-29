
#include "Lcd.h"
#include "Display.h"

void Display_Update(uint16 temp, uint8 fan, uint8 overheat)
{
    /* -------- Line 1: Temperature -------- */
    LCD_SetCursor(0, 0);
    LCD_Print("Temp: ");

    LCD_PrintInt(temp / 10);
    LCD_PrintChar('.');
    LCD_PrintInt(temp % 10);
    LCD_Print(" C   ");

    /* -------- Line 2 -------- */
    LCD_SetCursor(1, 0);

    if (overheat)
    {
        LCD_PrintCentered(1, "OVERHEAT");    }
    else
    {
        LCD_Print("Fan: ");
        LCD_PrintInt(fan);
        LCD_Print("%   ");
    }
}

