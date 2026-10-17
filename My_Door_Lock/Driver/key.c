#include  "key.h"

uint8_t key_down,key_up,key_old,key_val;
uint8_t password_len = 0;
uint8_t password_buf[6] = {0};
uint8_t password[6] = {'1', '2', '3', '4', '5', '6'};
uint8_t admin_password[6] = {'2', '2', '2', '3', '3', '3'};
uint8_t refresh_lcd = 0;
static uint8_t password_error_count = 0;
static uint8_t admin_error_count = 0;

uint8_t mode_flag = 0; // 0:开锁 1:修改密码 2:添加卡片
static uint8_t admin_verified_flag = 0; // 0:验证管理员密码 1:执行管理操作

#define MODE_UNLOCK 0
#define MODE_CHANGE_PASSWORD 1
#define MODE_ADD_CARD 2
#define CARD_MAX_COUNT 10
#define CARD_ID_LEN 6

static uint8_t card_ids[CARD_MAX_COUNT][CARD_ID_LEN];
static uint8_t card_count = 0;

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

    if(password_lock_flag || key_down == 0)
    {
        return;
    }

    // 管理员验证后等待刷卡，此时按键不改变录卡状态
    if(mode_flag == MODE_ADD_CARD && admin_verified_flag)
        return;

    // 不足6位时忽略确认，保留已输入密码
    if(key_down == 16 && password_len != 6)
    {
        return;
    }

    if(key_down != 16 && key_down != 4 && key_down != 12)
    {
        voice_play(VOICE_TRACK_KEY);
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
            if(mode_flag == MODE_UNLOCK)
            {
                mode_flag = MODE_CHANGE_PASSWORD;
                admin_verified_flag = 0;
                password_len = 0;
                refresh_lcd = 1;
                voice_play(VOICE_TRACK_ADMIN_ENTER);
            }
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
            if(mode_flag == MODE_UNLOCK)
            {
                mode_flag = MODE_ADD_CARD;
                admin_verified_flag = 0;
                password_len = 0;
                refresh_lcd = 1;
                uart2_discard_frame();
                voice_play(VOICE_TRACK_CARD_ENTER);
            }
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
            if(mode_flag == MODE_CHANGE_PASSWORD && admin_verified_flag == 1)
            {
                for(i = 0; i < 6; i++)
                {
                    password[i] = password_buf[i];
                }
                mode_flag = MODE_UNLOCK;
                admin_verified_flag = 0;
                voice_play(VOICE_TRACK_PASSWORD_SAVED);
            }
            else
            {
                for(i = 0; i < 6; i++)
                {
                    if(password_buf[i] != (mode_flag != MODE_UNLOCK ?
                                           admin_password[i] : password[i]))
                        break;
                }

                if(i == 6)
                {
                    // 任一密码验证成功后，错误次数重新累计
                    password_error_count = 0;
                    admin_error_count = 0;
                    if(mode_flag != MODE_UNLOCK)
                    {
                        admin_verified_flag = 1;
                        if(mode_flag == MODE_ADD_CARD)
                        {
                            uart2_discard_frame();
                            voice_play(VOICE_TRACK_CARD_ADMIN_CORRECT);
                        }
                        else
                            voice_play(VOICE_TRACK_ADMIN_CORRECT);
                    }
                    else
                    {
                        lock_flag = 0;
                        voice_play(VOICE_TRACK_PASSWORD_CORRECT);
                    }
                }
                else
                {
                    if(mode_flag != MODE_UNLOCK)
                    {
                        if(++admin_error_count >= 3)
                        {
                            admin_error_count = 0;
                            admin_verified_flag = 0;
                            password_lock_flag = 1;
                            voice_play(VOICE_TRACK_ADMIN_LOCKED);
                        }
                        else
                            voice_play(VOICE_TRACK_ADMIN_WRONG);
                    }
                    else
                    {
                        if(++password_error_count >= 3)
                        {
                            password_error_count = 0;
                            password_lock_flag = 1;
                            voice_play(VOICE_TRACK_PASSWORD_LOCKED);
                        }
                        else
                            voice_play(VOICE_TRACK_PASSWORD_WRONG);
                    }
                }
            }
            password_len = 0;
            refresh_lcd = 1;
            break;

        default:
            break;
    }
}

void card_proc(void)
{
    uint8_t frame[UART2_FRAME_LEN];
    uint8_t checksum = 0;
    uint8_t expected_checksum;
    uint8_t i, j;

    if(uart2_read_frame(frame) == 0)
        return;

    // 锁定期间及管理员密码验证阶段，刷卡操作无效
    if(password_lock_flag || (mode_flag != MODE_UNLOCK &&
       (mode_flag != MODE_ADD_CARD || admin_verified_flag == 0)))
        return;

    for(i = 0; i < UART2_FRAME_LEN - 1; i++)
        checksum ^= frame[i];
    expected_checksum = (uint8_t)(checksum ^ 0xFF);

    if(frame[0] != 0x04 || frame[1] != UART2_FRAME_LEN ||
       frame[2] != 0x02 || frame[3] != 0x30 ||
       frame[11] != expected_checksum)
        return;

    if(frame[4] != 0x00)
    {
        voice_play(VOICE_TRACK_CARD_WRONG);
        return;
    }

    // 卡类型2字节和卡ID4字节一并保存，避免不同卡类型ID相同
    for(i = 0; i < card_count; i++)
    {
        for(j = 0; j < CARD_ID_LEN; j++)
        {
            if(card_ids[i][j] != frame[5 + j])
                break;
        }
        if(j == CARD_ID_LEN)
            break;
    }

    if(mode_flag == MODE_ADD_CARD)
    {
        if(i == card_count && card_count < CARD_MAX_COUNT)
        {
            for(j = 0; j < CARD_ID_LEN; j++)
                card_ids[card_count][j] = frame[5 + j];
            card_count++;
        }

        if(i < card_count)
        {
            mode_flag = MODE_UNLOCK;
            admin_verified_flag = 0;
            voice_play(VOICE_TRACK_CARD_SAVED);
        }
        else
            voice_play(VOICE_TRACK_CARD_WRONG);
        return;
    }

    if(i < card_count)
    {
        lock_flag = 0;
        password_len = 0;
        refresh_lcd = 1;
        voice_play(VOICE_TRACK_CARD_CORRECT);
    }
    else
        voice_play(VOICE_TRACK_CARD_WRONG);
}
