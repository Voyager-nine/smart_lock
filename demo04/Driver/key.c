#include  "key.h"

uint8_t key_down,key_up,key_old,key_val;
uint8_t password_len = 0;
uint8_t password_buf[6] = {0};
uint8_t password[6] = {'1', '2', '3', '4', '5', '6'};
uint8_t refresh_lcd = 0;

/*
 * 矩阵按键 连接情况
 * C4 - PC5
 * C3 - PE7
 * C2 - PE9
 * C1 - PE11
 * R1 - PE13
 * R2 - PE15
 * R3 - PD9
 * R4 - PD11
 */

// ==============================================
// 矩阵按键初始化函数（行输出、列输入）
// ==============================================
void key_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    // 1. 开启所有用到的GPIO端口时钟（APB2总线）
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE); // C4
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE); // R3、R4
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE); // R1、R2、C1、C2、C3

    // 2. 初始化所有行引脚：推挽输出，50MHz速度
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    // 初始化PE端口的行引脚（R1、R2）
    GPIO_InitStructure.GPIO_Pin = KEY_R1_PIN | KEY_R2_PIN;
    GPIO_Init(KEY_R1_PORT, &GPIO_InitStructure);

    // 初始化PD端口的行引脚（R3、R4）
    GPIO_InitStructure.GPIO_Pin = KEY_R3_PIN | KEY_R4_PIN;
    GPIO_Init(KEY_R3_PORT, &GPIO_InitStructure);

    // 3. 初始化所有列引脚：内部上拉输入
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;

    // 初始化PE端口的列引脚（C1、C2、C3）
    GPIO_InitStructure.GPIO_Pin = KEY_C1_PIN | KEY_C2_PIN | KEY_C3_PIN;
    GPIO_Init(KEY_C1_PORT, &GPIO_InitStructure);

    // 初始化PC端口的列引脚（C4）
    GPIO_InitStructure.GPIO_Pin = KEY_C4_PIN;
    GPIO_Init(KEY_C4_PORT, &GPIO_InitStructure);

    // 4. 【重要】初始化完成后，所有行默认拉高
    // 确保上电瞬间没有按键按下时，所有列都是高电平
    GPIO_SetBits(KEY_R1_PORT, KEY_R1_PIN | KEY_R2_PIN);
    GPIO_SetBits(KEY_R3_PORT, KEY_R3_PIN | KEY_R4_PIN);
}

unsigned char key_read()
{
    uint8_t temp = 0;
    //第一行
    GPIO_ResetBits(GPIOE, GPIO_Pin_13);
    GPIO_SetBits(GPIOE, GPIO_Pin_15);
    GPIO_SetBits(GPIOD, GPIO_Pin_9);
    GPIO_SetBits(GPIOD, GPIO_Pin_11);

    if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_11) == 0) temp = 16;
    if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_9) == 0) temp = 15;
    if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_7) == 0) temp = 14;
    if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_5) == 0) temp = 13;

    //第二行
    GPIO_SetBits(GPIOE, GPIO_Pin_13);
    GPIO_ResetBits(GPIOE, GPIO_Pin_15);
    GPIO_SetBits(GPIOD, GPIO_Pin_9);
    GPIO_SetBits(GPIOD, GPIO_Pin_11);

    if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_11) == 0) temp = 12;
    if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_9) == 0) temp = 11;
    if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_7) == 0) temp = 10;
    if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_5) == 0) temp = 9;

    //第三行
    GPIO_SetBits(GPIOE, GPIO_Pin_13);
    GPIO_SetBits(GPIOE, GPIO_Pin_15);
    GPIO_ResetBits(GPIOD, GPIO_Pin_9);
    GPIO_SetBits(GPIOD, GPIO_Pin_11);

    if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_11) == 0) temp = 8;
    if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_9) == 0) temp = 7;
    if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_7) == 0) temp = 6;
    if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_5) == 0) temp = 5;

    //第四行
    GPIO_SetBits(GPIOE, GPIO_Pin_13);
    GPIO_SetBits(GPIOE, GPIO_Pin_15);
    GPIO_SetBits(GPIOD, GPIO_Pin_9);
    GPIO_ResetBits(GPIOD, GPIO_Pin_11);

    if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_11) == 0) temp = 4;
    if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_9) == 0) temp = 3;
    if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_7) == 0) temp = 2;
    if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_5) == 0) temp = 1;

    return temp;
}

void key_task()
{
    uint8_t i = 0;

    key_val = key_read();
    key_down = key_val & (key_val ^ key_old);
    key_up = ~key_val & (key_val ^ key_old);
    key_old = key_val;

    if(key_down == 0)
    {
        return;
    }

    switch(key_down)
    {
        case 1:
        case 2:
        case 3:
            if(password_len < 6)
            {
                password_buf[password_len++] = '0' + key_down;
                refresh_lcd = 1;
            }
            break;

        case 4:
            password_len = 0;
            refresh_lcd = 1;
            break;

        case 5:
        case 6:
        case 7:
            if(password_len < 6)
            {
                password_buf[password_len++] = '0' + key_down - 1;
                refresh_lcd = 1;
            }
            break;

        case 8:
            break;

        case 9:
        case 10:
        case 11:
            if(password_len < 6)
            {
                password_buf[password_len++] = '0' + key_down - 2;
                refresh_lcd = 1;
            }
            break;

        case 12:
            break;

        case 13:
            password_len = 0;
            refresh_lcd = 1;
            break;

        case 14:
            if(password_len < 6)
            {
                password_buf[password_len++] = '0';
                refresh_lcd = 1;
            }
            break;

        case 15:
            if(password_len > 0)
            {
                password_len--;
                refresh_lcd = 1;
            }
            break;

        case 16:
            if(password_len == 6)
            {
                for(i = 0; i < 6; i++)
                {
                    if(password_buf[i] != password[i])
                    {
                        break;
                    }
                }

                if(i == 6)
                {
                    lock(0);
                    voice_request(VOICE_PROMPT_PASSWORD_CORRECT);
                    //voice_request(VOICE_PROMPT_UNLOCKED);
                }
                else
                {
                    voice_request(VOICE_PROMPT_PASSWORD_WRONG);
                }
            }
            password_len = 0;
            refresh_lcd = 1;
            break;

        default:
            break;
    }
}
