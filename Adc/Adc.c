#include "Adc.h"
#include "stddef.h"

#include "Adc_Private.h"
#include "Bit_Operations.h"
#include "Nvic.h"

#define ADC1              ((AdcType *)ADC1_BASE_ADDR)
#define ADC_IRQ_NUMBER    18U

static volatile Adc_AsyncStateType Adc_CurrentAsyncState = ADC_ASYNC_STATE_IDLE;

/*  state for async scan-group read  */
static uint16* Adc_AsyncBuf = NULL;
static uint8 Adc_AsyncTotal = 0;
static uint8 Adc_AsyncIndex = 0;
static AdcMultiChannelCallback Adc_AsyncCallback = NULL;

/*  state for async single-channel read  */
static AdcSingleChannelCallback Adc_SingleCallback = NULL;

/* ------------------------------------------------------------------ */
/*  Helper: set sample time for a channel (84 cycles)                 */
/* ------------------------------------------------------------------ */
static void Adc_SetSampleTime(uint8 Channel)
{
    if (Channel < 10)
    {
        ADC1->SMPR2 &= ~(0x07UL << (Channel * 3));
        ADC1->SMPR2 |= ((uint32)SMPR_84_CYCLES << (Channel * 3));
    }
    else
    {
        ADC1->SMPR1 &= ~(0x07UL << ((Channel - 10) * 3));
        ADC1->SMPR1 |= ((uint32)SMPR_84_CYCLES << ((Channel - 10) * 3));
    }
}

/* ------------------------------------------------------------------ */
/*  Helper: set a channel into the regular sequence at position index */
/* ------------------------------------------------------------------ */
static void Adc_SetSequence(uint8 Index, uint8 Channel)
{
    if (Index < 6)
    {
        ADC1->SQR3 &= ~(0x1FUL << (Index * 5));
        ADC1->SQR3 |=  ((uint32)Channel << (Index * 5));
    }
    else if (Index < 12)
    {
        ADC1->SQR2 &= ~(0x1FUL << ((Index - 6) * 5));
        ADC1->SQR2 |=  ((uint32)Channel << ((Index - 6) * 5));
    }
    else
    {
        ADC1->SQR1 &= ~(0x1FUL << ((Index - 12) * 5));
        ADC1->SQR1 |=  ((uint32)Channel << ((Index - 12) * 5));
    }
}

void Adc_Init(uint8 Resolution)
{
    // /* Enable ADC clock */
    // SET_BIT(RCC_APB2ENR, 8);

    /* ADC prescaler */
    ADC_CCR &= ~(0x03UL << CCR_ADCPRE);

    /* Set resolution */
    ADC1->CR1 &= ~(0x03UL << CR1_RES);
    ADC1->CR1 |= ((uint32)Resolution << CR1_RES);

    /* Enable ADC */
    SET_BIT(ADC1->CR2, CR2_ADON);
}

void Adc_StartConversion(void)
{
    ADC1->SR = 0;
    SET_BIT(ADC1->CR2, CR2_ADON);
    SET_BIT(ADC1->CR2, CR2_SWSTART);
}

void Adc_StopConversion(void)
{
    CLEAR_BIT(ADC1->CR2, CR2_ADON);
}

/* ================= SINGLE MODE ================= */

void Adc_ConfigSingleChannel_OneShot(uint8 Channel)
{
    CLEAR_BIT(ADC1->CR1, CR1_SCAN);
    CLEAR_BIT(ADC1->CR2, CR2_CONT);
    SET_BIT(ADC1->CR2, CR2_EOCS);

    ADC1->SQR1 &= ~(0x0FUL << 20);

    ADC1->SQR3 &= ~(0x1FUL << 0);
    ADC1->SQR3 |= (uint32)Channel;

    Adc_SetSampleTime(Channel);

    SET_BIT(ADC1->CR2, CR2_ADON);
}

uint16 Adc_ReadSingleChannel(void)
{
    while (!READ_BIT(ADC1->SR, SR_EOC)) {}

    return (uint16)(ADC1->DR & 0xFFFFU);
}

void Adc_ReadSingleChannelAsync(AdcSingleChannelCallback Callback)
{
    Adc_AsyncBuf = 0;
    Adc_AsyncTotal = 0;
    Adc_AsyncIndex = 0;

    Adc_SingleCallback = Callback;
    Adc_CurrentAsyncState = ADC_ASYNC_STATE_SINGLE;

    SET_BIT(ADC1->CR1, CR1_EOCIE);
    Nvic_EnableIrq(ADC_IRQ_NUMBER);
}

/* ================= SCAN MODE ================= */

void Adc_ConfigScanGroup_Continuous(uint8* Channels, uint8 NumChannels)
{
    uint8 i;

    SET_BIT(ADC1->CR1, CR1_SCAN);
    /* FIXED: enable continuous */
    SET_BIT(ADC1->CR2, CR2_CONT);
    SET_BIT(ADC1->CR2, CR2_EOCS);
    /* FIXED */
    ADC1->SQR1 &= ~(0x0FUL << 20);
    ADC1->SQR1 |= ((uint32)(NumChannels - 1) << 20);

    ADC1->SQR3 = 0;
    ADC1->SQR2 = 0;

    for (i = 0; i < NumChannels; i++)
    {
        Adc_SetSequence(i, Channels[i]);
        Adc_SetSampleTime(Channels[i]);
    }
}

void Adc_ScanChannelGroup(uint16* Results, uint8 NumChannels)
{
    uint8 i;
    for (i = 0; i < NumChannels; i++)
    {
        while (!READ_BIT(ADC1->SR, SR_EOC)) {}

        Results[i] = (uint16)(ADC1->DR & 0xFFFFU);
    }
}

void Adc_ScanChannelGroupAsync(uint16* Results, uint8 NumChannels,
                               AdcMultiChannelCallback Callback)
{
    Adc_AsyncBuf = Results;
    Adc_AsyncTotal = NumChannels;
    Adc_AsyncIndex = 0;
    Adc_AsyncCallback = Callback;
    Adc_CurrentAsyncState = ADC_ASYNC_STATE_SCAN_GROUP;

    SET_BIT(ADC1->CR1, CR1_EOCIE);
    Nvic_EnableIrq(ADC_IRQ_NUMBER);
}

/* ================= ISR ================= */

void ADC_IRQHandler(void)
{
    if (READ_BIT(ADC1->SR, SR_EOC))
    {
        if (Adc_CurrentAsyncState == ADC_ASYNC_STATE_SCAN_GROUP)
        {
            if (Adc_AsyncBuf != NULL && Adc_AsyncIndex < Adc_AsyncTotal)
            {
                Adc_AsyncBuf[Adc_AsyncIndex++] = (uint16)(ADC1->DR & 0xFFFFU);
                Adc_AsyncIndex++;
            }

            if (Adc_AsyncIndex >= Adc_AsyncTotal)
            {
                if (Adc_AsyncCallback != NULL)
                {
                    Adc_AsyncCallback(Adc_AsyncBuf, Adc_AsyncTotal);
                }

                Adc_AsyncIndex = 0;
            }
        }
        else if (Adc_CurrentAsyncState == ADC_ASYNC_STATE_SINGLE)
        {
            uint16 result = (uint16)(ADC1->DR & 0xFFFFU);

            if (Adc_SingleCallback != NULL)
            {
                Adc_SingleCallback(result);
            }
        }
        else
        {
            volatile uint16 dummy = ADC1->DR;
            (void)dummy;
        }
    }
}


