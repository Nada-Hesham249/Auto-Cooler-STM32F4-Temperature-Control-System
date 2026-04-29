#include "App.h"
#include "Rcc.h"
#include "Adc.h"
#include "Gpio.h"
#include "Lcd.h"
#include "Pwm.h"
#include "StateMachine.h"
#include "Display.h"
#include "Nvic.h"

#define APP_LM35_ADC_CHANNEL    0U
#define APP_ALARM_PORT          GPIO_A
#define APP_ALARM_PIN           5U
#define APP_PWM_PORT            GPIO_A
#define APP_PWM_PIN             6U
#define ADC_IRQ_NUMBER          18U

static volatile uint16 App_LatestAdcValue = 0U;
static volatile uint8 App_NewSampleFlag = 0U;
static uint16 App_AdcBuffer[1] = {0U};

/* ================== Callback ================== */
void App_AdcCallback(uint16 *results, uint8 numChannels)
{
    if ((results != 0) && (numChannels > 0U))
    {
        App_LatestAdcValue = results[0];
        App_NewSampleFlag = 1U;

        Adc_StartConversion();

    }
}

/* ================== Init ================== */
void App_Init(void)
{
    StateMachine_Init();

    uint8 adc_channels[1];

    Rcc_Init();
    Rcc_Enable(RCC_GPIOA);
    Rcc_Enable(RCC_GPIOD);
    Rcc_Enable(RCC_TIM3);
    Rcc_Enable(RCC_ADC1);

    Gpio_Init(APP_ALARM_PORT, APP_ALARM_PIN, GPIO_OUTPUT, GPIO_PUSH_PULL);
    Gpio_Init(APP_PWM_PORT, APP_PWM_PIN, GPIO_AF, GPIO_PUSH_PULL);
    Gpio_SetAF(APP_PWM_PORT, APP_PWM_PIN, GPIO_AF2);
    Gpio_Init(GPIO_A , APP_LM35_ADC_CHANNEL, GPIO_ANALOG, GPIO_PUSH_PULL);

    LCD_Init();
    LCD_Clear();
    LCD_SetCursor(0, 0U);
    LCD_Print("SYSTEM INIT     ");
    LCD_SetCursor(1, 0U);
    LCD_Print("                ");

    Pwm_Init(TIMER3, PWM_CHANNEL_1, 1599U, 99U);
    // Pwm_Init(TIMER3, PWM_CHANNEL_1, 15U, 99U);
    Pwm_Start(TIMER3, PWM_CHANNEL_1);

    Adc_Init(ADC_RES_12BIT);
    adc_channels[0] = APP_LM35_ADC_CHANNEL;

    Adc_ConfigScanGroup_Continuous(adc_channels, 1U);
    Adc_ScanChannelGroupAsync(App_AdcBuffer, 1U, App_AdcCallback);

    Adc_StartConversion();
}
void App_Update(void)
{
    static uint16 prev_temp = 0xFFFF;
    static uint8  prev_fan  = 0xFF;
    static uint8  prev_flag = 0xFF;

    uint16 adc_value;
    SM_Output_t current_output;
    uint8 display_needs_update = 0U;

    /* ================= Check new ADC sample ================= */
    if (App_NewSampleFlag)
    {
        /* ===== Critical Section: protect shared ADC data ===== */
        Nvic_DisableIrq(ADC_IRQ_NUMBER);
        adc_value = App_LatestAdcValue;
        App_NewSampleFlag = 0U;
        Nvic_EnableIrq(ADC_IRQ_NUMBER);

        /* ===== Convert ADC to temperature (fixed-point) ===== */
        uint16 temperature_tenths_c =
            (uint16)((adc_value * 3300U) / 4096U);

        /* ===== State Machine processing ===== */
        StateMachine_Update(temperature_tenths_c, &current_output);

        /* ================= PWM update ================= */
        if (current_output.fan_speed != prev_fan)
        {
            Pwm_SetDutyPercent(TIMER3,
                               PWM_CHANNEL_1,
                               current_output.fan_speed);

            prev_fan = current_output.fan_speed;
            display_needs_update = 1U;
        }

        /* ================= Alarm GPIO update ================= */
        if (current_output.overheat_flag != prev_flag)
        {
            Gpio_WritePin(APP_ALARM_PORT,
                          APP_ALARM_PIN,
                          current_output.overheat_flag);

            prev_flag = current_output.overheat_flag;
            display_needs_update = 1U;
        }

        /* ================= Temperature change check ================= */
        if (temperature_tenths_c != prev_temp)
        {
            prev_temp = temperature_tenths_c;
            display_needs_update = 1U;
        }

        /* ================= LCD update ================= */
        if (display_needs_update)
        {
            Display_Update(temperature_tenths_c,
                           current_output.fan_speed,
                           current_output.overheat_flag);
        }
    }
}
