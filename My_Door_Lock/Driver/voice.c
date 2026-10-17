#include "voice.h"

#define VOICE_FRAME_HEAD 0x7E
#define VOICE_FRAME_LEN  0x05
#define VOICE_CMD_TRACK  0x41
#define VOICE_FRAME_TAIL 0xEF

void voice_init(void)
{
    Usart3_Init();
}

void voice_play(uint8_t num)
{
    uint8_t frame[7];
    uint8_t i;

    if(num < 1 || num > VOICE_TRACK_COUNT)
    {
        return;
    }

    frame[0] = VOICE_FRAME_HEAD;
    frame[1] = VOICE_FRAME_LEN;
    frame[2] = VOICE_CMD_TRACK;
    frame[3] = 0x00;
    frame[4] = num;
    frame[5] = frame[1] ^ frame[2] ^ frame[3] ^ frame[4];
    frame[6] = VOICE_FRAME_TAIL;

    for(i = 0; i < sizeof(frame); i++)
    {
        while(USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
        USART_SendData(USART3, frame[i]);
    }
}
