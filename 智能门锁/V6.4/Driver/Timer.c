#include "debug.h"

void Tim2_Init(u16 arr,u16 psc)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    NVIC_InitTypeDef NVIC_InitStructure;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    TIM_TimeBaseInitStruct.TIM_ClockDivision=0;//时钟分割
    TIM_TimeBaseInitStruct.TIM_CounterMode=TIM_CounterMode_Up;//设置为向上计数
    TIM_TimeBaseInitStruct.TIM_Period=arr;//自动重装载值，5000为500ms
    TIM_TimeBaseInitStruct.TIM_Prescaler=psc;//设置分频值

    TIM_TimeBaseInit(TIM2,&TIM_TimeBaseInitStruct);
    TIM_ITConfig(TIM2, TIM_IT_Update|TIM_IT_Trigger, ENABLE);



    NVIC_InitStructure.NVIC_IRQChannel=TIM2_IRQn;
//    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;
//    NVIC_InitStructure.NVIC_IRQChannelSubPriority=3;
    NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;

    NVIC_Init(&NVIC_InitStructure);

    TIM_Cmd(TIM2, ENABLE);

}


