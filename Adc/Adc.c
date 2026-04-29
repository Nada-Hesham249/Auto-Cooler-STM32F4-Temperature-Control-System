#include "adc.h"
#include "stddef.h"
#include "Adc_Private.h"
#include "Bit_Operations.h"
#include "Nvic.h"

#define ADC1              ((AdcType *)ADC1_BASE_ADDR)
#define ADC_IRQ_NUMBER    18U


/* ================= ADC ASYNC STATE ================= */
static volatile Adc_AsyncStateType Adc_CurrentAsyncState = ADC_ASYNC_STATE_IDLE;

/*  state for async scan-group read  */
static uint16* Adc_AsyncBuf = NULL;
static uint8 Adc_AsyncTotal = 0;
static uint8 Adc_AsyncIndex = 0;
static AdcMultiChannelCallback Adc_AsyncCallback = NULL;

/* Single channel async callback */
static AdcSingleChannelCallback Adc_SingleCallback = NULL;


/* ================= HELPER FUNCTIONS ================= */

static void Adc_SetSampleTime(uint8 Channel)
{
    if (Channel < 10)
    {
        ADC1->SMPR2 &= ~(0x07UL << (Channel * 3));
        ADC1->SMPR2 |= ((uint32) SMPR_84_CYCLES  << (Channel * 3));
    }
    else
    {
        ADC1->SMPR1 &= ~(0x07UL << ((Channel - 10) * 3));
        ADC1->SMPR1 |= (SMPR_84_CYCLES  << ((Channel - 10) * 3));
    }
}


/* Set channel order in scan sequence */
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
    /* ADC prescaler */
    ADC_CCR &= ~(0x03UL << CCR_ADCPRE);

    /* Set resolution */
    ADC1->CR1 &= ~(0x03UL <<  CR1_RES);
    ADC1->CR1 |= ((uint32)Resolution << CR1_RES);

    /* Enable ADC */
    SET_BIT(ADC1->CR2, CR2_ADON);
}

void Adc_StartConversion(void)
{
    ADC1->SR = 0;
    SET_BIT(ADC1->CR2, CR2_SWSTART);
}

void Adc_StopConversion(void)
{
    CLEAR_BIT(ADC1->CR2, CR2_ADON);
}


/* ================= SINGLE CHANNEL ================= */

void Adc_ConfigSingleChannel_OneShot(uint8 Channel)
{
    CLEAR_BIT(ADC1->CR1, CR1_SCAN);
    CLEAR_BIT(ADC1->CR2, CR2_CONT);

    SET_BIT(ADC1->CR2, CR2_EOCS);  // Interrupt after each conversion

    ADC1->SQR1 &= ~(0x0FUL << 20);
    ADC1->SQR3 = Channel;

    Adc_SetSampleTime(Channel);
}
uint16 Adc_ReadSingleChannel(void)
{
    while (!READ_BIT(ADC1->SR, SR_EOC)){}

    return (uint16)(ADC1->DR & 0xFFFFU);
}


/* Enable async read for single conversion */
void Adc_ReadSingleChannelAsync(AdcSingleChannelCallback Callback)
{
    Adc_AsyncBuf = 0;
    Adc_AsyncTotal = 0;
    Adc_AsyncIndex = 0;

    Adc_SingleCallback = Callback;  //registeration of callback func
    Adc_CurrentAsyncState = ADC_ASYNC_STATE_SINGLE;

    SET_BIT(ADC1->CR1, CR1_EOCIE);   // Enable EOC interrupt
    Nvic_EnableIrq(ADC_IRQ_NUMBER);
}


/* ================= SCAN MODE ================= */

void Adc_ConfigScanGroup_Continuous(uint8* Channels, uint8 NumChannels)
{
    SET_BIT(ADC1->CR1, CR1_SCAN);
    SET_BIT(ADC1->CR2, CR2_EOCS);  // Interrupt per channel

    ADC1->SQR1 &= ~(0x0FUL << 20);
    ADC1->SQR1 |= ((uint32)(NumChannels - 1) << 20);

    ADC1->SQR3 = 0;
    ADC1->SQR2 = 0;

    for (uint8 i = 0; i < NumChannels; i++)
    {
        Adc_SetSequence(i, Channels[i]);
        Adc_SetSampleTime(Channels[i]);
    }
}


/* Polling */
void Adc_ScanChannelGroup(uint16* Results, uint8 NumChannels)
{
    for (uint8 i = 0; i < NumChannels; i++)
    {
        while (!READ_BIT(ADC1->SR, SR_EOC)){}

        Results[i] = (uint16)(ADC1->DR & 0xFFFFU);
    }
}

/* Async scan using interrupt */
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
    SET_BIT(ADC1->CR2, CR2_ADON);
}


/* ================= ISR ================= */

void ADC_IRQHandler(void)
{
    /* Check End Of Conversion flag */
    if (READ_BIT(ADC1->SR, SR_EOC))
    {
        /* ================= Scan Group Mode ================= */
        if (Adc_CurrentAsyncState == ADC_ASYNC_STATE_SCAN_GROUP)
        {
            /* Store converted value in buffer */
            Adc_AsyncBuf[Adc_AsyncIndex++] = (uint16)(ADC1->DR & 0xFFFFU);

            /* Check if all channels are read */
            if (Adc_AsyncIndex >= Adc_AsyncTotal)
            {
                /* Stop ADC after finishing all conversions */
                CLEAR_BIT(ADC1->CR2, CR2_ADON);

                /* Call user callback with results */
                if (Adc_AsyncCallback)
                {
                    Adc_AsyncCallback(Adc_AsyncBuf, Adc_AsyncTotal);
                }
            }
        }

        /* ================= Single Channel Mode ================= */
        else if (Adc_CurrentAsyncState == ADC_ASYNC_STATE_SINGLE)
        {
            /* Read conversion result */
            uint16 result = (uint16)(ADC1->DR & 0xFFFFU);

            /* Call user callback with the result */
            if (Adc_SingleCallback)
            {
                Adc_SingleCallback(result);
            }

            /* Restart conversion manually */
             Adc_StartConversion();

        }
    }
}