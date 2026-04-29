#include "StateMachine.h"

/* ===== Thresholds ===== */
#define T_COOLING_ENTER   250U   // 25°C
#define T_OVERHEAT_ENTER  400U   // 40°C

static State_t StateMachine_CurrentState = STATE_IDLE;

/* ===== Fan mapping ===== */
static uint8 GetFanSpeed(uint16 temp)
{
    if (temp < 250U)        // < 25°C
        return 0U;

    else if (temp < 300U)   // 25-30°C
        return 33U;

    else if (temp <350U)   // 30-35°C
        return 66U;

    else
        return 100U;    //more than 35
}

/* ================= INIT ================= */
void StateMachine_Init(void)
{
    StateMachine_CurrentState = STATE_IDLE;
}

/* ================= UPDATE ================= */
void StateMachine_Update(uint16 temp, SM_Output_t *output)
{
    if (output == 0)
        return;

    State_t next_state = StateMachine_CurrentState;

    switch (StateMachine_CurrentState)
    {
        /* ============ IDLE ============ */
        case STATE_IDLE:

            output->fan_speed = 0U;
            output->overheat_flag = 0U;
            output->state = STATE_IDLE;

            if (temp >= T_OVERHEAT_ENTER)
            {
                next_state = STATE_OVERHEAT;
            }
            else if (temp >= T_COOLING_ENTER)
            {
                next_state = STATE_COOLING;
            }
            break;

        /* ============ COOLING ============ */
        case STATE_COOLING:

            output->overheat_flag = 0U;
            output->state = STATE_COOLING;

            /* Fan control */
            output->fan_speed = GetFanSpeed(temp);

            if (temp < T_COOLING_ENTER)
            {
                next_state = STATE_IDLE;
            }
            else if (temp >= T_OVERHEAT_ENTER)
            {
                next_state = STATE_OVERHEAT;
            }
            break;

        /* ============ OVERHEAT ============ */
        case STATE_OVERHEAT:

            output->fan_speed = 100U;
            output->overheat_flag = 1U;
            output->state = STATE_OVERHEAT;

            if (temp < T_OVERHEAT_ENTER)
            {
                next_state = STATE_COOLING;
            }
            break;

        default:
            next_state = STATE_IDLE;
            break;
    }

    StateMachine_CurrentState = next_state;
}
