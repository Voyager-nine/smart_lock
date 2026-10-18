#include "uart.h"

static u8 uart7_rec_overflow;

void uart7_receive_byte(u8 data)
{
    // 完整包取走前保留缓冲区，忽略后续字节
    if(uart7_rec_ready)
        return;

    if(uart7_rec_index < sizeof(uart7_rec_string))
        uart7_rec_string[uart7_rec_index++] = data;
    else
        uart7_rec_overflow = 1;
    uart7_rec_tick = 10;
}

void uart7_timeout_1ms(void)
{
    if(uart7_rec_tick && --uart7_rec_tick == 0)
    {
        if(uart7_rec_overflow)
        {
            // 超长包整包丢弃
            uart7_rec_index = 0;
            uart7_rec_overflow = 0;
        }
        else if(uart7_rec_index)
            uart7_rec_ready = 1;
    }
}

// 返回包长度；frame 需至少20字节，尚未收完时返回0
u8 uart7_read_frame(u8 frame[20])
{
    u8 i, len;

    if(frame == 0 || uart7_rec_ready == 0)
        return 0;

    USART_ITConfig(UART7, USART_IT_RXNE, DISABLE);
    len = uart7_rec_index;
    for(i = 0; i < len; i++)
        frame[i] = uart7_rec_string[i];
    uart7_rec_index = 0;
    uart7_rec_ready = 0;
    USART_ITConfig(UART7, USART_IT_RXNE, ENABLE);
    return len;
}

void usart1_send_string(u8* string, u8 led)
{
    u8 i;

    if(string == 0 || led == 0)
        return;

    for(i = 0; i < led; i++)
    {
        while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
        USART_SendData(USART1, string[i]);
    }
    while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
}

void uart7_send_string(uint8_t* string, uint8_t len)
{
    uint8_t i;

    if(string == 0 || len == 0)
        return;

    for(i = 0; i < len; i++)
    {
        while(USART_GetFlagStatus(UART7, USART_FLAG_TXE) == RESET);
        USART_SendData(UART7, string[i]);
    }
    while(USART_GetFlagStatus(UART7, USART_FLAG_TC) == RESET);

    usart1_send_string(string,len);
}

static volatile uint8_t uart2_frame[UART2_FRAME_LEN];
static volatile uint8_t uart2_frame_ready = 0;
static uint8_t uart2_frame_index = 0;
static uint32_t uart2_last_byte_ms = 0;
volatile uint8_t uart3_rx_data = 0;
volatile uint8_t uart3_rx_flag = 0;
volatile uint8_t uart7_rx_data = 0;
volatile uint8_t uart7_rx_flag = 0;

void uart2_receive_byte(uint8_t data)
{
    uint32_t now = GetTick();

    if(password_lock_flag)
    {
        uart2_frame_index = 0;
        uart2_frame_ready = 0;
        return;
    }

    if(uart2_frame_ready)
        return;

    // 包内相邻字节间隔过长时丢弃残包
    if(uart2_frame_index && (uint32_t)(now - uart2_last_byte_ms) > 20)
        uart2_frame_index = 0;
    uart2_last_byte_ms = now;

    if(uart2_frame_index == 0 && data != 0x04)
        return;
    if(uart2_frame_index == 1 && data != UART2_FRAME_LEN)
    {
        uart2_frame_index = (data == 0x04) ? 1 : 0;
        return;
    }

    uart2_frame[uart2_frame_index++] = data;
    if(uart2_frame_index == UART2_FRAME_LEN)
    {
        uart2_frame_index = 0;
        uart2_frame_ready = 1;
    }
}

uint8_t uart2_read_frame(uint8_t frame[UART2_FRAME_LEN])
{
    uint8_t i;

    if(uart2_frame_ready == 0)
        return 0;

    for(i = 0; i < UART2_FRAME_LEN; i++)
        frame[i] = uart2_frame[i];
    uart2_frame_ready = 0;
    return 1;
}

void uart2_discard_frame(void)
{
    USART_ITConfig(USART2, USART_IT_RXNE, DISABLE);
    uart2_frame_ready = 0;
    uart2_frame_index = 0;
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
}

void Usart2_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    USART_InitTypeDef USART_InitStructure = {0};
    NVIC_InitTypeDef NVIC_InitStructure = {0};

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitStructure.GPIO_Pin = UART2_TX_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(UART2_TX_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = UART2_RX_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(UART2_RX_PORT, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = UART2_BAUDRATE;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART2, &USART_InitStructure);

    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USART_Cmd(USART2, ENABLE);
}

void Usart3_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    USART_InitTypeDef USART_InitStructure = {0};
    NVIC_InitTypeDef NVIC_InitStructure = {0};

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitStructure.GPIO_Pin = UART3_TX_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(UART3_TX_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = UART3_RX_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(UART3_RX_PORT, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = UART3_BAUDRATE;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART3, &USART_InitStructure);

    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USART_Cmd(USART3, ENABLE);
}

void Uart7_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    USART_InitTypeDef USART_InitStructure = {0};
    NVIC_InitTypeDef NVIC_InitStructure = {0};

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART7, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    GPIO_InitStructure.GPIO_Pin = UART7_TX_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(UART7_TX_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = UART7_RX_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(UART7_RX_PORT, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = UART7_BAUDRATE;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(UART7, &USART_InitStructure);

    USART_ITConfig(UART7, USART_IT_RXNE, ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel = UART7_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USART_Cmd(UART7, ENABLE);
}
