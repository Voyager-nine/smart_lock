/*
 * voice.h
 *
 *  Created on: 2026-05-23
 *      Author: Voyager
 */

#ifndef DRIVER_VOICE_H_
#define DRIVER_VOICE_H_

#include "debug.h"

#define VOICE_UART              USART3
#define VOICE_UART_BAUDRATE     9600

#define VOICE_TX_PORT           GPIOB
#define VOICE_TX_PIN            GPIO_Pin_10
#define VOICE_RX_PORT           GPIOB
#define VOICE_RX_PIN            GPIO_Pin_11
#define VOICE_BUSY_PORT         GPIOB
#define VOICE_BUSY_PIN          GPIO_Pin_12

#define VOICE_TRACK_LOCKED_INPUT       1
#define VOICE_TRACK_PASSWORD_WRONG     2
#define VOICE_TRACK_PASSWORD_CORRECT   3
#define VOICE_TRACK_UNLOCKED           4
#define VOICE_TRACK_INPUT_PASSWORD     5

typedef enum
{
    VOICE_PROMPT_NONE = 0,
    VOICE_PROMPT_LOCKED_INPUT,
    VOICE_PROMPT_PASSWORD_WRONG,
    VOICE_PROMPT_PASSWORD_CORRECT,
    VOICE_PROMPT_UNLOCKED,
    VOICE_PROMPT_INPUT_PASSWORD
} voice_prompt_t;

typedef enum
{
    VOICE_EQ_NORMAL = 0,
    VOICE_EQ_POP,
    VOICE_EQ_ROCK,
    VOICE_EQ_JAZZ,
    VOICE_EQ_CLASSIC,
    VOICE_EQ_BASS
} voice_eq_t;

typedef enum
{
    VOICE_LOOP_ALL = 0,
    VOICE_LOOP_FOLDER,
    VOICE_LOOP_SINGLE,
    VOICE_LOOP_RANDOM,
    VOICE_LOOP_NONE
} voice_loop_mode_t;

void voice_init(void);
void voice_task(void);
void voice_request(voice_prompt_t prompt);
void voice_play(void);
void voice_pause(void);
void voice_next(void);
void voice_previous(void);
void voice_volume_up(void);
void voice_volume_down(void);
void voice_play_track(uint16_t track);
uint8_t voice_play_track_blocking(uint16_t track, uint32_t timeout_ms);
void voice_play_folder_track(uint8_t folder, uint8_t track);
void voice_insert_track(uint16_t track);
void voice_combo_play_tracks(const uint16_t *tracks, uint8_t count);
void voice_set_volume(uint8_t volume);
void voice_set_eq(voice_eq_t eq);
void voice_set_loop_mode(voice_loop_mode_t mode);
void voice_stop(void);
void voice_reset(void);
uint8_t voice_is_busy(void);
uint8_t voice_wait_idle(uint32_t timeout_ms);

#endif /* DRIVER_VOICE_H_ */
