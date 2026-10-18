#include <assert.h>
#include <stdint.h>
#include <string.h>

#define DRIVER_AS608_H_
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef struct { int GPIO_Pin; int GPIO_Mode; } GPIO_InitTypeDef;

#define GPIO_Pin_1 1
#define GPIO_Mode_IPD 2
#define GPIOA 1
#define UART7 7
#define USART_IT_RXNE 1
#define ENABLE 1
#define DISABLE 0
#define RCC_APB2Periph_GPIOA 1
#define LCD_PROMPT_FINGER_ADMIN 19
#define LCD_PROMPT_FINGER_ENROLL 20
#define LCD_PROMPT_FINGER_SAVED 21
#define LCD_PROMPT_FINGER_CORRECT 16
#define LCD_PROMPT_FINGER_WRONG 17
#define LCD_PROMPT_FINGER_ERROR 28
#define LCD_PROMPT_FINGER_TIMEOUT 29
#define LCD_PROMPT_FINGER_FAILED 30
#define LCD_PROMPT_FINGER_FULL 27
#define LCD_PROMPT_EXIT 24

volatile u8 uart7_rec_string[44];
volatile u8 uart7_rec_index, uart7_rec_tick, uart7_rec_ready;
volatile u8 lock_flag = 1, password_lock_flag;
u8 mode_flag, key_down;
static u8 authorized, touch, prompt;
static u8 command[20], command_len;
static u32 now_ms;

void Uart7_Init(void) {}
void RCC_APB2PeriphClockCmd(int clock, int enabled) { (void)clock; (void)enabled; }
void GPIO_Init(int port, GPIO_InitTypeDef *config) { (void)port; (void)config; }
u8 GPIO_ReadInputDataBit(int port, int pin) { (void)port; (void)pin; return touch; }
void USART_ITConfig(int port, int interrupt, int enabled)
{ (void)port; (void)interrupt; (void)enabled; }
void uart7_discard_frame(void)
{ uart7_rec_index = uart7_rec_tick = uart7_rec_ready = 0; }
void uart7_send_string(u8 *bytes, u8 len)
{
    u16 sum = 0;
    u8 i;
    assert(len <= sizeof(command));
    memcpy(command, bytes, len);
    command_len = len;
    for(i = 6; i < len - 2; i++) sum += bytes[i];
    assert(bytes[len - 2] == (u8)(sum >> 8));
    assert(bytes[len - 1] == (u8)sum);
}
void usart1_send_string(const volatile u8 *bytes, u8 len)
{ (void)bytes; (void)len; }
u32 GetTick(void) { return now_ms; }
u8 key_prompt(void) { return authorized ? LCD_PROMPT_FINGER_ADMIN : 0; }
void lcd_prompt(u8 value) { prompt = value; }

#include "../My_Door_Lock/Driver/as608.c"

static void reply(u8 code, const u8 *data, u8 data_len)
{
    u8 i, len = data_len + 12;
    u16 sum = 0;
    assert(len <= sizeof(uart7_rec_string));
    uart7_rec_string[0] = 0xEF;
    uart7_rec_string[1] = 0x01;
    for(i = 2; i < 6; i++) uart7_rec_string[i] = 0xFF;
    uart7_rec_string[6] = 0x07;
    uart7_rec_string[7] = 0;
    uart7_rec_string[8] = data_len + 3;
    uart7_rec_string[9] = code;
    for(i = 0; i < data_len; i++) uart7_rec_string[10 + i] = data[i];
    for(i = 6; i < len - 2; i++) sum += uart7_rec_string[i];
    uart7_rec_string[len - 2] = (u8)(sum >> 8);
    uart7_rec_string[len - 1] = (u8)sum;
    uart7_rec_index = len;
    uart7_rec_ready = 1;
    now_ms += 20;
    as608_task();
}

static void ack(void) { reply(0, 0, 0); }
static void lift(void) { touch = 0; now_ms += 10; as608_task(); }
static void press(void) { touch = 1; now_ms += 10; as608_task(); }

int main(void)
{
    u8 index[32];
    u8 match[4] = {0, 5, 0, 51};

    memset(index, 0xFF, sizeof(index));
    index[0] &= (u8)~(1u << 5);
    mode_flag = 3;
    authorized = 1;
    press();
    assert(command[9] == 0x1F && command[10] == 0);
    reply(0, index, 32);
    assert(command[9] == 0x01);
    ack();
    assert(command[9] == 0x02 && command[10] == 1);
    ack();
    lift();
    press();
    assert(command[9] == 0x01);
    ack();
    assert(command[9] == 0x02 && command[10] == 2);
    ack();
    assert(command[9] == 0x05);
    ack();
    assert(command[9] == 0x06 && command[11] == 0 && command[12] == 5);
    ack();
    assert(mode_flag == 0 && prompt == LCD_PROMPT_FINGER_SAVED);

    lift();
    press();
    assert(command[9] == 0x01);
    ack();
    assert(command[9] == 0x02 && command[10] == 2);
    ack();
    assert(command[9] == 0x04 && command_len == 17);
    reply(0, match, 4);
    assert(lock_flag == 0 && prompt == LCD_PROMPT_FINGER_CORRECT);

    lock_flag = 1;
    lift();
    press();
    ack();
    ack();
    mode_flag = 3;
    reply(0, match, 4);
    assert(lock_flag == 1);

    mode_flag = 0;
    lift();
    press();
    ack();
    ack();
    reply(9, 0, 0);
    assert(lock_flag == 1 && prompt == LCD_PROMPT_FINGER_WRONG);

    mode_flag = 3;
    lift();
    press();
    memset(index, 0xFF, sizeof(index));
    reply(0, index, 32);
    assert(command[9] == 0x1F && command[10] == 1);
    index[0] &= (u8)~1u;
    reply(0, index, 32);
    assert(command[9] == 0x01 && fp_page == 256);

    now_ms += 2001;
    as608_task();
    assert(prompt == LCD_PROMPT_FINGER_TIMEOUT);

    lift();
    key_down = 13;
    as608_task();
    assert(command[9] == 0x0D);
    key_down = 0;
    ack();
    assert(mode_flag == 0 && prompt == LCD_PROMPT_EXIT);
    return 0;
}
