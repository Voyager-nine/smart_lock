#include "debug.h"

/*4*4矩阵键盘
//C1 PA5PE7
//C2 PE7
//C3 PE9
//C4 PE11

//R1 PE13
//R2 PE15
//R3 PD9
//R4 PD11

/* 独立按键 */
//PC6
//PC7
void Key_Init()
{

    GPIO_InitTypeDef  GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE|RCC_APB2Periph_GPIOD|RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin=GPIO_Pin_7|GPIO_Pin_9|GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin=GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin=GPIO_Pin_13|GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin=GPIO_Pin_9|GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
    GPIO_Init(GPIOD, &GPIO_InitStructure);


    GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin=GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin=GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_ResetBits(GPIOA,GPIO_Pin_10);


}

uint8_t Key_Read()
{
    uint8_t temp=0x00;

    // 扫描C4(PA5)列：S13, S9, S5, S1
    GPIO_ResetBits(GPIOA,GPIO_Pin_5);GPIO_SetBits(GPIOE, GPIO_Pin_7);GPIO_SetBits(GPIOE,GPIO_Pin_9);GPIO_SetBits(GPIOE, GPIO_Pin_11);
    if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_13)==0)temp=13;  // R1 -> S13
    if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_15)==0)temp=9;   // R2 -> S9
    if(GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_9)==0)temp=5;    // R3 -> S5
    if(GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_11)==0)temp=1;   // R4 -> S1

    // 扫描C3(PE7)列：S14, S10, S6, S2
    GPIO_ResetBits(GPIOE,GPIO_Pin_7);GPIO_SetBits(GPIOA, GPIO_Pin_5);GPIO_SetBits(GPIOE,GPIO_Pin_9);GPIO_SetBits(GPIOE, GPIO_Pin_11);
    if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_13)==0)temp=14;  // R1 -> S14
    if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_15)==0)temp=10;  // R2 -> S10
    if(GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_9)==0)temp=6;    // R3 -> S6
    if(GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_11)==0)temp=2;   // R4 -> S2

    // 扫描C2(PE9)列：S15, S11, S7, S3
    GPIO_ResetBits(GPIOE,GPIO_Pin_9);GPIO_SetBits(GPIOE, GPIO_Pin_7);GPIO_SetBits(GPIOA,GPIO_Pin_5);GPIO_SetBits(GPIOE, GPIO_Pin_11);
    if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_13)==0)temp=15;  // R1 -> S15
    if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_15)==0)temp=11;  // R2 -> S11
    if(GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_9)==0)temp=7;    // R3 -> S7
    if(GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_11)==0)temp=3;   // R4 -> S3

    // 扫描C1(PE11)列：S16, S12, S8, S4
    GPIO_ResetBits(GPIOE,GPIO_Pin_11);GPIO_SetBits(GPIOE, GPIO_Pin_7);GPIO_SetBits(GPIOE,GPIO_Pin_9);GPIO_SetBits(GPIOA, GPIO_Pin_5);
    if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_13)==0)temp=16;  // R1 -> S16
    if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_15)==0)temp=12;  // R2 -> S12
    if(GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_9)==0)temp=8;    // R3 -> S8
    if(GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_11)==0)temp=4;   // R4 -> S4

    // 独立按键PA8
    if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_8)==0)temp=17;
    return temp;
}

