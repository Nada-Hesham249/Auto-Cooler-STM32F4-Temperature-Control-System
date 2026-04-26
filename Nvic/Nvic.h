#ifndef NVIC_H
#define NVIC_H

#include "Std_Types.h"


void Nvic_EnableIrq(uint8 irqNumber);
void Nvic_DisableIrq(uint8 irqNumber);
void Nvic_SetPriority(uint8 irqNumber, uint8 priority);

#endif




