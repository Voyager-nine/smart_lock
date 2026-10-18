#ifndef DRIVER_UART_H_
#define DRIVER_UART_H_

#include "mydefine.h"

#define UART2_BAUDRATE 115200
#define UART2_TX_PORT GPIOA
#define UART2_TX_PIN GPIO_Pin_2
#define UART2_RX_PORT GPIOA
#define UART2_RX_PIN GPIO_Pin_3
#define UART2_FRAME_LEN 12

#define UART3_BAUDRATE 9600
#define UART3_TX_PORT GPIOB
#define UART3_TX_PIN GPIO_Pin_10
#define UART3_RX_PORT GPIOB
#define UART3_RX_PIN GPIO_Pin_11

#define UART7_BAUDRATE 115200
#define UART7_FRAME_CAPACITY 44
#define UART7_TX_PORT GPIOC
#define UART7_TX_PIN GPIO_Pin_2
#define UART7_RX_PORT GPIOC
#define UART7_RX_PIN GPIO_Pin_3

extern volatile uint8_t uart3_rx_data;
extern volatile uint8_t uart3_rx_flag;
extern volatile uint8_t uart7_rx_data;
extern volatile uint8_t uart7_rx_flag;
extern volatile u8 uart7_rec_string[UART7_FRAME_CAPACITY];
extern volatile u8 uart7_rec_index;
extern volatile u8 uart7_rec_tick;
extern volatile u8 uart7_rec_ready;

void uart7_receive_byte(u8 data);
void uart7_timeout_1ms(void);
void uart7_discard_frame(void);
u8 uart7_read_frame(u8 frame[20]);

void Usart2_Init(void);
void uart2_receive_byte(uint8_t data);
uint8_t uart2_read_frame(uint8_t frame[UART2_FRAME_LEN]);
void uart2_discard_frame(void);
void Usart3_Init(void);
void Uart7_Init(void);
void uart7_send_string(uint8_t* string, uint8_t len);
void usart1_send_string(const volatile u8* string, u8 len);

#endif /* DRIVER_UART_H_ */
