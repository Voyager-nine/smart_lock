/*
 * key.h
 *
 *  Created on: 2026年4月18日
 *      Author: Voyager
 */

#ifndef DRIVER_KEY_H_
#define DRIVER_KEY_H_

#include "mydefine.h"

// ==============================================
// 矩阵按键引脚宏定义（改引脚只需要改这里！）
// ==============================================
// 行引脚（R1-R4）：推挽输出，用于逐行拉低
#define KEY_R1_PIN  GPIO_Pin_13
#define KEY_R1_PORT GPIOE
#define KEY_R2_PIN  GPIO_Pin_15
#define KEY_R2_PORT GPIOE
#define KEY_R3_PIN  GPIO_Pin_9
#define KEY_R3_PORT GPIOD
#define KEY_R4_PIN  GPIO_Pin_11
#define KEY_R4_PORT GPIOD

// 列引脚（C1-C4）：上拉输入，用于检测电平
#define KEY_C1_PIN  GPIO_Pin_11
#define KEY_C1_PORT GPIOE
#define KEY_C2_PIN  GPIO_Pin_9
#define KEY_C2_PORT GPIOE
#define KEY_C3_PIN  GPIO_Pin_7
#define KEY_C3_PORT GPIOE
#define KEY_C4_PIN  GPIO_Pin_5
#define KEY_C4_PORT GPIOC

void key_init(void);
void key_task();
uint8_t key_prompt(void);
#endif /* DRIVER_KEY_H_ */
