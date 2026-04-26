
#ifndef UTILS_H
#define UTILS_H

#include "Std_Types.h"

void delay_ms(uint32 ms);

#define REG32(ADDR)  ( *(volatile uint32 *)(ADDR) )

#endif //UTILS_H