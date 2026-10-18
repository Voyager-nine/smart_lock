/*
 * mydefine.h
 *
 *  Created on: 2026-05-14
 *      Author: Voyager
 */

#ifndef DRIVER_MYDEFINE_H_
#define DRIVER_MYDEFINE_H_

#include "debug.h"
#include "timer.h"
#include "lcd.h"
#include "key.h"
#include "voice.h"
#include "uart.h"
#include "as608.h"

extern uint8_t refresh_lcd;
extern uint8_t password_len;
extern uint8_t password_buf[];
extern uint8_t mode_flag;
void card_proc(void);

#endif /* DRIVER_MYDEFINE_H_ */
