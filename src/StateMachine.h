#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include "Std_Types.h"

typedef enum
{
    STATE_IDLE = 0,
    STATE_COOLING,
    STATE_OVERHEAT
} State_t;

typedef struct
{
    uint8 fan_speed;
    uint8 overheat_flag;
    State_t state;
} SM_Output_t;

void StateMachine_Init(void);
void StateMachine_Update(uint16 temperature_tenths_c, SM_Output_t *output);

#endif