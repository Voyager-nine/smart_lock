#include "voice.h"

#define VOICE_FRAME_HEAD        0x7E
#define VOICE_FRAME_TAIL        0xEF

#define VOICE_CMD_PLAY          0x01
#define VOICE_CMD_PAUSE         0x02
#define VOICE_CMD_NEXT          0x03
#define VOICE_CMD_PREVIOUS      0x04
#define VOICE_CMD_VOLUME_UP     0x05
#define VOICE_CMD_VOLUME_DOWN   0x06
#define VOICE_CMD_RESET         0x09
#define VOICE_CMD_STOP          0x0E
#define VOICE_CMD_SET_VOLUME    0x31
#define VOICE_CMD_SET_EQ        0x32
#define VOICE_CMD_SET_LOOP      0x33
#define VOICE_CMD_PLAY_TRACK    0x41
#define VOICE_CMD_PLAY_FOLDER   0x42
#define VOICE_CMD_INSERT_TRACK  0x43

#define VOICE_COMMAND_GAP_MS    25
#define VOICE_COMBO_GAP_MS      5
#define VOICE_COMBO_MAX_COUNT   10
#define VOICE_QUEUE_SIZE        4
#define VOICE_BUSY_START_TIMEOUT_MS    200

static volatile voice_prompt_t voice_queue[VOICE_QUEUE_SIZE];
static volatile uint8_t voice_queue_head = 0;
static volatile uint8_t voice_queue_tail = 0;

static void voice_send_byte(uint8_t data)
{
    while(USART_GetFlagStatus(VOICE_UART, USART_FLAG_TXE) == RESET);
    USART_SendData(VOICE_UART, data);
}

static void voice_send_frame_with_gap(uint8_t cmd, const uint8_t *param, uint8_t param_len, uint8_t gap_ms)
{
    uint8_t i;
    uint8_t len = param_len + 3;
    uint8_t check = len ^ cmd;

    voice_send_byte(VOICE_FRAME_HEAD);
    voice_send_byte(len);
    voice_send_byte(cmd);

    for(i = 0; i < param_len; i++)
    {
        check ^= param[i];
        voice_send_byte(param[i]);
    }

    voice_send_byte(check);
    voice_send_byte(VOICE_FRAME_TAIL);

    if(gap_ms > 0)
    {
        Delay_Ms(gap_ms);
    }
}

static void voice_send_frame(uint8_t cmd, const uint8_t *param, uint8_t param_len)
{
    voice_send_frame_with_gap(cmd, param, param_len, VOICE_COMMAND_GAP_MS);
}

static void voice_send_command(uint8_t cmd)
{
    voice_send_frame(cmd, 0, 0);
}

static uint8_t voice_pop_prompt(voice_prompt_t *prompt)
{
    if(voice_queue_head == voice_queue_tail)
    {
        return 0;
    }

    *prompt = voice_queue[voice_queue_tail];
    voice_queue_tail = (voice_queue_tail + 1) % VOICE_QUEUE_SIZE;
    return 1;
}

void voice_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    USART_InitTypeDef USART_InitStructure = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

    /* PB10 USART3_TX -> BY8301 RX. */
    GPIO_InitStructure.GPIO_Pin = VOICE_TX_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(VOICE_TX_PORT, &GPIO_InitStructure);

    /* PB11 USART3_RX <- BY8301 TX. Reserved for response parsing. */
    GPIO_InitStructure.GPIO_Pin = VOICE_RX_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(VOICE_RX_PORT, &GPIO_InitStructure);

    /* BUSY is high during playback and low when stopped. */
    GPIO_InitStructure.GPIO_Pin = VOICE_BUSY_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_Init(VOICE_BUSY_PORT, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = VOICE_UART_BAUDRATE;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_Init(VOICE_UART, &USART_InitStructure);
    USART_Cmd(VOICE_UART, ENABLE);
}

void voice_task(void)
{
    voice_prompt_t prompt;

    if(voice_is_busy())
    {
        return;
    }

    if(voice_pop_prompt(&prompt) == 0)
    {
        return;
    }

    switch(prompt)
    {
        case VOICE_PROMPT_LOCKED_INPUT:
            voice_play_track(VOICE_TRACK_LOCKED_INPUT);
            break;

        case VOICE_PROMPT_PASSWORD_WRONG:
            voice_play_track(VOICE_TRACK_PASSWORD_WRONG);
            break;

        case VOICE_PROMPT_PASSWORD_CORRECT:
            voice_play_track(VOICE_TRACK_PASSWORD_CORRECT);
            break;

        case VOICE_PROMPT_UNLOCKED:
            voice_play_track(VOICE_TRACK_UNLOCKED);
            break;

        case VOICE_PROMPT_INPUT_PASSWORD:
            voice_play_track(VOICE_TRACK_INPUT_PASSWORD);
            break;

        default:
            break;
    }
}

void voice_request(voice_prompt_t prompt)
{
    uint8_t next_head;

    if(prompt == VOICE_PROMPT_NONE)
    {
        return;
    }

    next_head = (voice_queue_head + 1) % VOICE_QUEUE_SIZE;
    if(next_head == voice_queue_tail)
    {
        voice_queue_tail = (voice_queue_tail + 1) % VOICE_QUEUE_SIZE;
    }

    voice_queue[voice_queue_head] = prompt;
    voice_queue_head = next_head;
}

void voice_play(void)
{
    voice_send_command(VOICE_CMD_PLAY);
}

void voice_pause(void)
{
    voice_send_command(VOICE_CMD_PAUSE);
}

void voice_next(void)
{
    voice_send_command(VOICE_CMD_NEXT);
}

void voice_previous(void)
{
    voice_send_command(VOICE_CMD_PREVIOUS);
}

void voice_volume_up(void)
{
    voice_send_command(VOICE_CMD_VOLUME_UP);
}

void voice_volume_down(void)
{
    voice_send_command(VOICE_CMD_VOLUME_DOWN);
}

void voice_play_track(uint16_t track)
{
    uint8_t param[2];

    if(track == 0)
    {
        track = 1;
    }

    param[0] = (uint8_t)(track >> 8);
    param[1] = (uint8_t)(track & 0xFF);
    voice_send_frame(VOICE_CMD_PLAY_TRACK, param, 2);
}

uint8_t voice_play_track_blocking(uint16_t track, uint32_t timeout_ms)
{
    uint32_t wait_ms = 0;

    voice_play_track(track);

    while(voice_is_busy() == 0 && wait_ms < VOICE_BUSY_START_TIMEOUT_MS)
    {
        Delay_Ms(1);
        wait_ms++;
    }

    if(voice_is_busy() == 0)
    {
        return 0;
    }

    return voice_wait_idle(timeout_ms);
}

void voice_play_folder_track(uint8_t folder, uint8_t track)
{
    uint8_t param[2];

    param[0] = folder;
    param[1] = track;
    voice_send_frame(VOICE_CMD_PLAY_FOLDER, param, 2);
}

void voice_insert_track(uint16_t track)
{
    uint8_t param[2];

    if(track == 0)
    {
        track = 1;
    }

    param[0] = (uint8_t)(track >> 8);
    param[1] = (uint8_t)(track & 0xFF);
    voice_send_frame(VOICE_CMD_INSERT_TRACK, param, 2);
}

void voice_combo_play_tracks(const uint16_t *tracks, uint8_t count)
{
    uint8_t i;
    uint8_t param[2];
    uint16_t track;

    if(tracks == 0 || count == 0)
    {
        return;
    }

    if(count > VOICE_COMBO_MAX_COUNT)
    {
        count = VOICE_COMBO_MAX_COUNT;
    }

    for(i = 0; i < count; i++)
    {
        track = tracks[i];
        if(track == 0)
        {
            track = 1;
        }

        param[0] = (uint8_t)(track >> 8);
        param[1] = (uint8_t)(track & 0xFF);
        voice_send_frame_with_gap(VOICE_CMD_PLAY_TRACK, param, 2, VOICE_COMBO_GAP_MS);
    }
}

void voice_set_volume(uint8_t volume)
{
    if(volume > 30)
    {
        volume = 30;
    }

    voice_send_frame(VOICE_CMD_SET_VOLUME, &volume, 1);
}

void voice_set_eq(voice_eq_t eq)
{
    uint8_t param;

    if(eq > VOICE_EQ_BASS)
    {
        eq = VOICE_EQ_NORMAL;
    }

    param = (uint8_t)eq;
    voice_send_frame(VOICE_CMD_SET_EQ, &param, 1);
}

void voice_set_loop_mode(voice_loop_mode_t mode)
{
    uint8_t param;

    if(mode > VOICE_LOOP_NONE)
    {
        mode = VOICE_LOOP_NONE;
    }

    param = (uint8_t)mode;
    voice_send_frame(VOICE_CMD_SET_LOOP, &param, 1);
}

void voice_stop(void)
{
    voice_send_command(VOICE_CMD_STOP);
}

void voice_reset(void)
{
    voice_send_command(VOICE_CMD_RESET);
}

uint8_t voice_is_busy(void)
{
    return GPIO_ReadInputDataBit(VOICE_BUSY_PORT, VOICE_BUSY_PIN);
}

uint8_t voice_wait_idle(uint32_t timeout_ms)
{
    uint32_t wait_ms = 0;

    while(voice_is_busy())
    {
        if(timeout_ms > 0 && wait_ms >= timeout_ms)
        {
            return 0;
        }

        Delay_Ms(1);
        wait_ms++;
    }

    return 1;
}
