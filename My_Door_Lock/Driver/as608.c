/*
 * as608.c
 *
 *  Created on: 2026Äê10ÔÂ18ÈÕ
 *      Author: Voyager
 */

#include "as608.h"

void as608_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    Uart7_Init();

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

void PS_GetImage(void)
{
    u8 command[] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF,
                    0x01, 0x00, 0x03, 0x01, 0x00, 0x05};

    uart7_send_string(command, sizeof(command));
}

void PS_GenChar(u8 buffer)
{
    u8 command[] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF,
                    0x01, 0x00, 0x04, 0x02, 0x00, 0x00, 0x00};
    u16 sum;

    command[10] = buffer;
    sum = command[6] + command[7] + command[8] + command[9] + command[10];
    command[11] = (u8)(sum >> 8);
    command[12] = (u8)sum;

    uart7_send_string(command, sizeof(command));
}
