#ifndef ADC_PRIVATE_H
#define ADC_PRIVATE_H

#include "Std_Types.h"

/* ADC1 register map (STM32F401) */
typedef struct {
    volatile uint32 SR;
    volatile uint32 CR1;
    volatile uint32 CR2;
    volatile uint32 SMPR1;
    volatile uint32 SMPR2;
    volatile uint32 JOFR1;
    volatile uint32 JOFR2;
    volatile uint32 JOFR3;
    volatile uint32 JOFR4;
    volatile uint32 HTR;
    volatile uint32 LTR;
    volatile uint32 SQR1;
    volatile uint32 SQR2;
    volatile uint32 SQR3;
    volatile uint32 JSQR;
    volatile uint32 JDR1;
    volatile uint32 JDR2;
    volatile uint32 JDR3;
    volatile uint32 JDR4;
    volatile uint32 DR;
} AdcType;

typedef enum {
    ADC_ASYNC_STATE_IDLE       = 0U,
    ADC_ASYNC_STATE_SINGLE     = 1U,
    ADC_ASYNC_STATE_SCAN_GROUP = 2U
} Adc_AsyncStateType;

#define ADC1_BASE_ADDR          0x40012000UL

/*  ADC COMMON REGISTER */
#define ADC_COMMON_BASE_ADDR    0x40012300UL
#define ADC_CCR (*(volatile uint32 *)(ADC_COMMON_BASE_ADDR + 0x04UL))


/* SR bits */
#define SR_AWD           0U
#define SR_EOC           1U
#define SR_JEOC          2U
#define SR_JSTRT         3U
#define SR_STRT          4U
#define SR_OVR           5U

/* CR1 bits */
#define CR1_SCAN         8U
#define CR1_JEOCIE       7U
#define CR1_EOCIE        5U
#define CR1_RES          24U

/* CR2 bits */
#define CR2_ADON         0U
#define CR2_CONT         1U
#define CR2_DMA          8U
#define CR2_EOCS        10U
#define CR2_SWSTART     30U

/* Sample time */
#define SMPR_3_CYCLES    0x00U
#define SMPR_15_CYCLES   0x01U
#define SMPR_28_CYCLES   0x02U
#define SMPR_56_CYCLES   0x03U
#define SMPR_84_CYCLES   0x04U
#define SMPR_112_CYCLES  0x05U
#define SMPR_144_CYCLES  0x06U
#define SMPR_480_CYCLES  0x07U

/* CCR bits */
#define CCR_ADCPRE      16U

#endif
// #endif /* ADC_PRIVATE_H */