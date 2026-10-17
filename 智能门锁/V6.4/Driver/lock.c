#include "debug.h"

/*舵机引脚初始化*/
void Lock_Init()
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOE, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;

    GPIO_Init(GPIOC, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;

    GPIO_Init(GPIOE, &GPIO_InitStructure);

}

/*舵机控制*/
void Lock(unsigned char flag)
{
    int time1,time2,n=8;
    if(flag)time1=800;else time1=1700;
    if(flag)time2=1500;else time2=500;

    while(n--)
    {
       GPIO_ResetBits(GPIOC, GPIO_Pin_13);
       GPIO_ResetBits(GPIOE, GPIO_Pin_4);
       Delay_Ms(10);

       GPIO_SetBits(GPIOC, GPIO_Pin_13);
       Delay_Us(time1);
       GPIO_ResetBits(GPIOC, GPIO_Pin_13);


       GPIO_SetBits(GPIOE, GPIO_Pin_4);
       Delay_Us(time2);

       GPIO_ResetBits(GPIOE, GPIO_Pin_4);
       GPIO_ResetBits(GPIOC, GPIO_Pin_13);
       Delay_Ms(10);
    }
}
