#include "timer.h"

// 定义舵机的PWM周期和占空比
#define PWM_PERIOD 20000  // PWM周期为20ms (20000us)
#define PWM_MIN 500       // 最小脉宽为0.5ms (500us)
#define PWM_MAX 2500      // 最大脉宽为2.5ms (2500us)
#define LOCK_OPEN_TIME_MS 5000
#define PASSWORD_LOCK_TIME_MS 10000


static volatile uint32_t tick_ms = 0;
volatile uint8_t lock_flag = 1;
volatile uint8_t password_lock_flag = 0;
static volatile uint16_t password_lock_ms = 0;
static volatile uint16_t lock_open_ms = 0;
static volatile uint8_t lock_auto_close_voice_flag = 0;

void Millis_Init(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    TIM_TimeBaseStructure.TIM_Period = 1000 - 1;
    TIM_TimeBaseStructure.TIM_Prescaler = 96 - 1;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

    TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    TIM_Cmd(TIM3, ENABLE);
    __enable_irq();
}

uint32_t GetTick(void)
{
    return tick_ms;
}

void TIM3_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM3_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
    {
        tick_ms++;
        // 密码连续错误后的输入锁定计时
        if(password_lock_flag)
        {
            if(++password_lock_ms >= PASSWORD_LOCK_TIME_MS)
            {
                password_lock_ms = 0;
                password_lock_flag = 0;
            }
        }
        else
        {
            password_lock_ms = 0;
        }

        if(lock_flag == 0)
        {
            if(++lock_open_ms >= LOCK_OPEN_TIME_MS)
            {
                lock_open_ms = 0;
                lock_flag = 1;
                lock_auto_close_voice_flag = 1;
            }
        }
        else
        {
            lock_open_ms = 0;
        }
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
    }
}
// 初始化TIM2以生成PWM信号
void TIM2_PWM_Init(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);  // 使能TIM2时钟

    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;

    // 配置TIM2的基本参数
    TIM_TimeBaseStructure.TIM_Period = PWM_PERIOD - 1;  // 设置自动重装值
    TIM_TimeBaseStructure.TIM_Prescaler = 96 - 1;       // 设置预分频器，96MHz/96 = 1MHz (1us)
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

    // 配置TIM2的PWM模式
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = PWM_MIN;  // 初始占空比为最小值
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

    // 配置TIM2通道1
    TIM_OC1Init(TIM2, &TIM_OCInitStructure);  // 使用TIM2的通道1
    TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Enable);  // 使能TIM2通道1的预装载寄存器


    TIM_ARRPreloadConfig(TIM2, ENABLE);               // 使能TIM2的自动重装寄存器预装载

    TIM_Cmd(TIM2, ENABLE);  // 使能TIM2



    // 初始化GPIO以输出PWM信号
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);  // 使能GPIOA时钟
    GPIO_InitTypeDef GPIO_InitStructure;
    // 配置PA0为复用推挽输出（TIM2通道1）
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

void lock(uint8_t enable)
{
    //上锁
    if(enable)
        TIM_SetCompare1(TIM2,PWM_MIN);
    //开锁
    else
        TIM_SetCompare1(TIM2,1500);
}

void lock_proc(void)
{
    lock(lock_flag);

    if(lock_auto_close_voice_flag)
    {
        lock_auto_close_voice_flag = 0;
        if(lock_flag == 1)
        {
            voice_play(VOICE_TRACK_LOCKED);
        }
    }
}

