#ifndef DRIVER_TIMER_H_
#define DRIVER_TIMER_H_

#include "mydefine.h"

void Millis_Init(void);
uint32_t GetTick(void);
void TIM2_PWM_Init(void);
void lock(uint8_t enable);
#endif
