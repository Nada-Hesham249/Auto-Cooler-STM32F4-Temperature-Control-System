#include "Nvic.h"

typedef struct {
    volatile uint32 ISER[8];
    uint32 RESERVED0[24];

    volatile uint32 ICER[8];
    uint32 RESERVED1[24];

    volatile uint32 ISPR[8];
    uint32 RESERVED2[24];

    volatile uint32 ICPR[8];
    uint32 RESERVED3[24];

    volatile uint32 IABR[8];
    uint32 RESERVED4[56];

    volatile uint32 IPR[60];
} Nvic_Type;

#define NVIC ((Nvic_Type*)0xE000E100)

/* ================= ENABLE IRQ ================= */
void Nvic_EnableIrq(uint8 irqNumber)
{
    NVIC->ISER[irqNumber / 32U] |= (1UL << (irqNumber % 32U));}

/* ================= DISABLE IRQ ================= */
void Nvic_DisableIrq(uint8 irqNumber)
{
    NVIC->ICER[irqNumber / 32U] = (1UL << (irqNumber % 32U));
}

/* ================= SET PRIORITY ================= */
void Nvic_SetPriority(uint8 irqNumber, uint8 priority)
{
    uint32 regIndex = irqNumber / 4U;        /* 4 IRQs per 32-bit register */
    uint32 section   = irqNumber % 4U;       /* which byte inside register */

    /* clear previous value */
    NVIC->IPR[regIndex] &= ~(0xFFUL << (section * 8U));

    /* set new priority (STM32 uses upper 4 bits) */
    NVIC->IPR[regIndex] |= ((uint32)(priority << 4U) << (section * 8U));
}
