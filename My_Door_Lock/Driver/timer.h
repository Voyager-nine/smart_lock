#ifndef DRIVER_TIMER_H_
#define DRIVER_TIMER_H_

#include "mydefine.h"

extern volatile uint8_t lock_flag;
extern volatile uint8_t password_lock_flag;

void Millis_Init(void);
uint32_t GetTick(void);
void TIM2_PWM_Init(void);
void lock(uint8_t enable);
void lock_proc(void);
#endif
