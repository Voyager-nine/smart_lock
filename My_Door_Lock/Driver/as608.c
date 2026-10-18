/*
 * as608.c
 *
 *  Created on: 2026年10月18日
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

void PS_RegModel(void)
{
    u8 command[] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF,
                    0x01, 0x00, 0x03, 0x05, 0x00, 0x09};

    uart7_send_string(command, sizeof(command));
}

void PS_StoreChar(u16 page)
{
    u8 command[] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF,
                    0x01, 0x00, 0x06, 0x06, 0x01, 0x00, 0x00, 0x00, 0x00};
    u16 sum;

    command[11] = (u8)(page >> 8);
    command[12] = (u8)page;
    sum = command[6] + command[7] + command[8] + command[9] +
          command[10] + command[11] + command[12];
    command[13] = (u8)(sum >> 8);
    command[14] = (u8)sum;
    uart7_send_string(command, sizeof(command));
}

void PS_Search(void)
{
    u8 command[] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF,
                    0x01, 0x00, 0x08, 0x04, 0x02, 0x00, 0x00,
                    0x01, 0x2C, 0x00, 0x3C};

    uart7_send_string(command, sizeof(command));
}

void PS_ReadIndexTable(u8 page)
{
    u8 command[] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF,
                    0x01, 0x00, 0x04, 0x1F, 0x00, 0x00, 0x00};

    command[10] = page;
    command[12] = 0x24 + page;
    uart7_send_string(command, sizeof(command));
}

void PS_Empty(void)
{
    u8 command[] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF,
                    0x01, 0x00, 0x03, 0x0D, 0x00, 0x11};

    uart7_send_string(command, sizeof(command));
}

#define FP_ENROLL_MODE 3
#define FP_REPLY_TIMEOUT_MS 2000
#define FP_SECOND_TOUCH_TIMEOUT_MS 15000
#define FP_MIN_SCORE 50
#define FP_LAST_PAGE 299

typedef enum {
    FP_IDLE,
    FP_INDEX,
    FP_IMAGE_1,
    FP_CHAR_1,
    FP_RELEASE,
    FP_SECOND_TOUCH,
    FP_IMAGE_2,
    FP_CHAR_2,
    FP_MODEL,
    FP_STORE,
    FP_SEARCH,
    FP_EMPTY
} fp_state_t;

static fp_state_t fp_state;
static u8 fp_enrolling;
static u8 fp_touch_old;
static u16 fp_page;
static u8 fp_index_page;
static u32 fp_started;
extern uint8_t key_down;

static void fp_wait(fp_state_t state)
{
    fp_state = state;
    fp_started = GetTick();
}

static void fp_fail(u8 prompt)
{
    fp_state = FP_IDLE;
    lcd_prompt(prompt);
}

static void fp_clear_rx(void)
{
    uart7_discard_frame();
}

static u8 fp_reply_valid(u8 length)
{
    u8 i;
    u16 sum = 0;
    u16 payload_length;

    if(length < 12 || uart7_rec_string[0] != 0xEF || uart7_rec_string[1] != 0x01 ||
       uart7_rec_string[2] != 0xFF || uart7_rec_string[3] != 0xFF ||
       uart7_rec_string[4] != 0xFF || uart7_rec_string[5] != 0xFF ||
       uart7_rec_string[6] != 0x07)
        return 0;

    payload_length = ((u16)uart7_rec_string[7] << 8) | uart7_rec_string[8];
    if(length != payload_length + 9)
        return 0;

    for(i = 6; i < length - 2; i++)
        sum += uart7_rec_string[i];
    return uart7_rec_string[length - 2] == (u8)(sum >> 8) &&
           uart7_rec_string[length - 1] == (u8)sum;
}

void as608_task(void)
{
    u8 touch = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1);
    u8 touch_down = touch && !fp_touch_old;
    u8 length;
    u8 confirmation;
    u16 page;
    u8 bit;

    fp_touch_old = touch;

    if(fp_state != FP_IDLE &&
       ((fp_enrolling &&
         (mode_flag != FP_ENROLL_MODE || key_prompt() != LCD_PROMPT_FINGER_ADMIN)) ||
        (!fp_enrolling && (mode_flag != 0 || password_lock_flag))))
    {
        fp_state = FP_IDLE;
        fp_clear_rx();
        return;
    }

    if(uart7_rec_ready)
    {
        length = uart7_rec_index;
        usart1_send_string(uart7_rec_string, length);
        confirmation = uart7_rec_string[9];
        if(fp_state != FP_IDLE && !fp_reply_valid(length))
            fp_fail(LCD_PROMPT_FINGER_ERROR);
        else if(fp_state == FP_INDEX)
        {
            if(confirmation || length != 44)
                fp_fail(LCD_PROMPT_FINGER_ERROR);
            else
            {
                for(page = fp_index_page * 256; page <= FP_LAST_PAGE &&
                    page < (u16)(fp_index_page + 1) * 256; page++)
                {
                    bit = page % 256;
                    if(!(uart7_rec_string[10 + bit / 8] & (1u << (bit % 8))))
                        break;
                }
                if(page <= FP_LAST_PAGE && page < (u16)(fp_index_page + 1) * 256)
                {
                    fp_page = page;
                    fp_wait(FP_IMAGE_1);
                }
                else if(fp_index_page == 0)
                {
                    fp_index_page = 1;
                    fp_wait(FP_INDEX);
                }
                else
                    fp_fail(LCD_PROMPT_FINGER_FULL);
            }
        }
        else if(fp_state == FP_SEARCH)
        {
            if(confirmation == 0 && length == 16 &&
               ((((u16)uart7_rec_string[12] << 8) | uart7_rec_string[13]) > FP_MIN_SCORE))
            {
                lock_flag = 0;
                lcd_prompt(LCD_PROMPT_FINGER_CORRECT);
            }
            else
                lcd_prompt(confirmation == 9 || (confirmation == 0 && length == 16) ?
                           LCD_PROMPT_FINGER_WRONG : LCD_PROMPT_FINGER_ERROR);
            fp_state = FP_IDLE;
        }
        else if(fp_state != FP_IDLE && (confirmation != 0 || length != 12))
            fp_fail(fp_enrolling ? LCD_PROMPT_FINGER_FAILED : LCD_PROMPT_FINGER_WRONG);
        else
        {
            switch(fp_state)
            {
                case FP_IMAGE_1: fp_wait(FP_CHAR_1); break;
                case FP_CHAR_1:
                    if(fp_enrolling)
                    {
                        fp_state = FP_RELEASE;
                        fp_started = GetTick();
                        lcd_prompt(LCD_PROMPT_FINGER_ENROLL);
                    }
                    else
                        fp_wait(FP_SEARCH);
                    break;
                case FP_IMAGE_2: fp_wait(FP_CHAR_2); break;
                case FP_CHAR_2: fp_wait(FP_MODEL); break;
                case FP_MODEL: fp_wait(FP_STORE); break;
                case FP_STORE:
                    fp_state = FP_IDLE;
                    mode_flag = 0;
                    lcd_prompt(LCD_PROMPT_FINGER_SAVED);
                    break;
                case FP_EMPTY:
                    fp_state = FP_IDLE;
                    mode_flag = 0;
                    lcd_prompt(LCD_PROMPT_EXIT);
                    break;
                default: break;
            }
        }

        fp_clear_rx();

        switch(fp_state)
        {
            case FP_INDEX: PS_ReadIndexTable(fp_index_page); break;
            case FP_IMAGE_1: PS_GetImage(); break;
            case FP_CHAR_1: PS_GenChar(fp_enrolling ? 1 : 2); break;
            case FP_IMAGE_2: PS_GetImage(); break;
            case FP_CHAR_2: PS_GenChar(2); break;
            case FP_MODEL: PS_RegModel(); break;
            case FP_STORE: PS_StoreChar(fp_page); break;
            case FP_SEARCH: PS_Search(); break;
            case FP_EMPTY: PS_Empty(); break;
            default: break;
        }
        return;
    }

    if(fp_state == FP_RELEASE && !touch)
    {
        fp_state = FP_SECOND_TOUCH;
        fp_started = GetTick();
    }
    else if(fp_state == FP_SECOND_TOUCH && touch_down)
    {
        fp_wait(FP_IMAGE_2);
        PS_GetImage();
    }
    else if(fp_state != FP_IDLE &&
            (u32)(GetTick() - fp_started) >=
            ((fp_state == FP_SECOND_TOUCH || fp_state == FP_RELEASE) ?
             FP_SECOND_TOUCH_TIMEOUT_MS : FP_REPLY_TIMEOUT_MS))
        fp_fail(LCD_PROMPT_FINGER_TIMEOUT);

    if(fp_state == FP_IDLE && mode_flag == FP_ENROLL_MODE &&
       key_prompt() == LCD_PROMPT_FINGER_ADMIN && key_down == 13 &&
       !password_lock_flag)
    {
        fp_enrolling = 1;
        fp_wait(FP_EMPTY);
        PS_Empty();
    }
    else if(fp_state == FP_IDLE && touch_down && !uart7_rec_ready && !password_lock_flag)
    {
        fp_enrolling = mode_flag == FP_ENROLL_MODE &&
                       key_prompt() == LCD_PROMPT_FINGER_ADMIN;
        if(fp_enrolling)
        {
            lcd_prompt(LCD_PROMPT_FINGER_ENROLL);
            fp_index_page = 0;
            fp_wait(FP_INDEX);
            PS_ReadIndexTable(fp_index_page);
        }
        else if(mode_flag == 0 && lock_flag)
        {
            fp_wait(FP_IMAGE_1);
            PS_GetImage();
        }
    }
}
