/*头文件引用区*/
#include "debug.h"  //必要系统头文件
#include "lcd.h"    //屏幕头文件
#include "pic.h"    //图片头文件
#include "timer.h"  //定时器头文件
#include "key.h"    //矩阵按键头文件
#include "uart.h"   //串口头文件
#include "audio.h"  //语音模块头文件
#include "string.h" //数组函数相关头文件
#include "as608.h"
#include "esp8266.h"
/*函数声明区*/
void TIM3_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));//定时器3快速中断
void USART3_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));//串口2快速中断
u8  string_chek(u8* string1,u8* string2,u8 len);//数组检测函数，一致返回1，否则0
void key_clear();//键盘暂存区清空函数
void string_copy(u8* string1,u8* string2,u8 len);
void USART2_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));//串口2快速中断
u8 rfid_chek();
void UART7_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));//串口7快速中断
void UART6_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));//串口7快速中断
void Ps_Wait();
u32 json(u8* string, u8* params, u8 len);
/*变量声明区*/
unsigned long int uwtick;             //系统计时变量
u8 key_val,key_old,key_down,key_up;  //按键相关变量
u8 password[6]={1,2,3,4,5,6};        //开锁密码
u8 password_cmd[6]={2,7,7,5,1,6};    //管理员密码
u8 key_temp[7]={10,10,10,10,10,10};  //按键暂存区
u8 key_index;                        //按键索引，已经输入的密码位数
u8 key_index_old;                    //同上
u16 time5000ms;                      //5秒钟计时变量，用来自动锁门
u8 lock_flag=1;                        //锁的标志位
u8 password_error;                   //密码错误次数
u16 time15s=15;                      //密码错误锁定倒计时15秒
u16 time1000ms;                      //密码错误锁定倒计时1秒
u8 show_flag;                        //显示汉字标志位
u8 show_flag_old;                    //显示汉字标志位
u8 mode;
//模式，0主页，1修改密码没有解锁管理员权限，2修改密码模式解锁了管理员权限
//3录入卡片，没有解锁管理员权限  4录入卡片，解锁了管理员权限
//5录入指纹，没有解锁管理员权限  6录入指纹，解锁了管理员权限
u8 rfid_index;                       //刷卡模块接收索引
u8 rfid_temp[4]={0};                 //卡号暂存数组
u8 rfid[4][4]={0};                   //卡片存储
u8 rfid_password_index;              //卡片存储索引
u8 uart7_rec_string[20]={0};
u8 uart7_rec_index;
u8 uart7_rec_tick;
u8 ps_wait_flag;
u8 as608_proc_falg;
u8 as608_proc_falg_old;
u8 as608_store_index=1;
u8 uart6_rec_string[256]={0};
u8 uart6_rec_tick;
u8 uart6_rec_index;
/*按键处理函数*/
void key_proc()
{
    /*三行消抖*/
  key_val=key_read();
  key_down=key_val&(key_val^key_old);
  key_up=~key_val&(key_val^key_old);
  key_old=key_val;

  if(password_error==3)return;


  /*按键按下*/
  if(key_down)//如果按下
  {
    audio_play(1);//播放按键音效
  }

  switch(key_down)
  {
      case 1:
      key_temp[key_index]=1;  //按键赋值给按键咱暂存区
      key_index++;            //索引值自增
      break;
      case 2:
      key_temp[key_index]=2;
      key_index++;
      break;
      case 3:
      key_temp[key_index]=3;
      key_index++;
      break;
      case 4:
      if(mode==0)
      {
          mode=1;
          audio_play(6);
          password_error=0;
          key_clear();
      }
      break;
      case 5:
      key_temp[key_index]=4;
      key_index++;
      break;
      case 6:
      key_temp[key_index]=5;
      key_index++;
      break;
      case 7:
      key_temp[key_index]=6;
      key_index++;
      break;
      case 8:
          if(mode==0)
          {
              mode=5;
              audio_play(18);
              password_error=0;
              key_clear();
          }
      break;
      case 9:
      key_temp[key_index]=7;
      key_index++;
      break;
      case 10:
      key_temp[key_index]=8;
      key_index++;
      break;
      case 11:
      key_temp[key_index]=9;
      key_index++;
      break;
      case 12:
       if(mode==0)
       {
           mode=3;
           audio_play(13);
           key_clear();
       }
      break;
      case 13:
      key_clear();
      if(mode==6)
      {
         PS_Empty();
         mode=0;
      }
      break;
      case 14:
      key_temp[key_index]=0;
      key_index++;
      break;
      case 15:
      if(key_index)
      {
      key_index--;
      key_temp[key_index]=10;
      }
      break;
      case 16://确认按键
          switch(mode)
          {
              case 0://主页
                  if( string_chek(key_temp,password,6) )
                      {
                       lock_flag=0;//开门
                       show_flag=1;//显示汉字们已开启
                       audio_play(3);//播放语音
                       key_clear();//清空状态
                       password_error=0;
                      }
                  else
                      {
                       audio_play(4);//播放语音
                       key_clear();
                       if(++password_error==3)
                       {
                          audio_play(5);
                          show_flag=2;
                       }
                      }
            break;//主页0的break
            case 1://修改密码没解锁管理员权限
                if( string_chek(key_temp,password_cmd,6) )
                {
                    mode=2;//修改密码 解锁了管理员权限
                    audio_play(7);//语音播放
                    key_clear();
                }
                else {
                    audio_play(9);//播放语音
                    if(++password_error==3)
                       {
                         audio_play(10);
                         show_flag=2;
                       }
                }
            break;//mode1
            case 2://修改开锁密码，解锁了管理员权限
                string_copy(key_temp,password,6);
                audio_play(8);
                mode=0;
                key_clear();
            break;
            case 3:
                if( string_chek(key_temp,password_cmd,6) )
                {
                    mode=4;//录入卡片 解锁了管理员权限
                    audio_play(14);//语音播放
                    key_clear();
                }
                else {
                    audio_play(9);//播放语音
                    if(++password_error==3)
                       {
                         audio_play(10);
                         show_flag=2;
                       }
                }
            break;
            case 5:
                if( string_chek(key_temp,password_cmd,6) )
                {
                    mode=6;//录入指纹 解锁了管理员权限
                    audio_play(19);//语音播放
                    key_clear();
                }
                else {
                    audio_play(9);//播放语音
                    if(++password_error==3)
                       {
                         audio_play(10);
                         show_flag=2;
                       }
                }

            break;
          }
      break;//确认按键的Brak
  }
}
void lock_proc()  //门锁处理函数
{
  lock(lock_flag);  //开关舵机
}

void esp8266_proc()
{
  if(uart6_rec_index==0)return;
  if(uart6_rec_tick>10)
  {
     usart1_send_string(uart6_rec_string,uart6_rec_index);
     lock_flag=json(uart6_rec_string,"lock",uart6_rec_index);
     uart6_rec_index=0;
  }

}

void as608_proc()
{
  as608_proc_falg=GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1);
  if(as608_proc_falg==as608_proc_falg_old)return;
  as608_proc_falg_old=as608_proc_falg;
  if(as608_proc_falg==0)return;


  if(mode==6)
  {
    PS_GetImage();//从指纹传感器获取图像到暂存区
    Ps_Wait();//等待接收到数据包
    PS_GenChar(1);
    Ps_Wait();//等待接收到数据包
    PS_GetImage();//从指纹传感器获取图像到暂存区
    Ps_Wait();//等待接收到数据包
    PS_GenChar(2);
    Ps_Wait();//等待接收到数据包
    PS_RegModel();
    Ps_Wait();//等待接收到数据包
    PS_StoreChar(as608_store_index);
    as608_store_index++;
    Ps_Wait();//等待接收到数据包
    audio_play(21);
    mode=0;
  }

  if(mode==0)
  {
      PS_GetImage();//从指纹传感器获取图像到暂存区
      Ps_Wait();//等待接收到数据包
      PS_GenChar(2);
      Ps_Wait();//等待接收到数据包
      PS_Search();
      Ps_Wait();//等待接收到数据包

      if(uart7_rec_string[13]>50)//匹配
      {
          audio_play(16);
          lock_flag=0;//开门
          show_flag=1;//显示汉字们已开启
      }
      else
      {
          audio_play(17);
      }
  }
}

/*屏幕处理函数*/
void lcd_proc()
{
    /*黄色密码输入区，*显示*/
    if(key_index!=key_index_old)   //如果暂存区位数发生了变化
    {
         u8 i=key_index;
         LCD_Fill(16,45,112,66,YELLOW);//填充黄色背景
         if(key_index==7)key_index=6;  //限位
         while(i--)//循环显示*
         {
             if(i<6)
             LCD_ShowChar(20+16*i,45,'*',RED,YELLOW,16,0);
         }
         key_index_old=key_index; //更新索引值
    }

   /*屏幕顶部白色字符显示区*/
   if(show_flag!=show_flag_old)
   {
       LCD_Fill(0,0,128,32,WHITE);//填充白色
      show_flag_old=show_flag;
   }

     switch(show_flag)
        {
        case 0://门已上锁
        lcd_show_chinese(0,0,"门已上锁，请输入密码",RED,WHITE,16,0);
        break;
        case 1:
        lcd_show_chinese(0,0,"门已开启，欢迎回家",RED,WHITE,16,0);
        break;
        case 2://锁定倒计时
        LCD_ShowIntNum(50, 16,time15s,2, RED,WHITE,16);
        break;
        }
}

/*系统计时定时器3中断服务函数*/
void TIM3_IRQHandler(void)//每1毫秒
{
    if(TIM_GetITStatus(TIM3, TIM_IT_Update)!=RESET)
    {
      uwtick++;//系统计时变量自增
      uart7_rec_tick++;
      uart6_rec_tick++;
      if(lock_flag==0)//门开
      {
        if(++time5000ms==5000)//计时5秒
        {
            time5000ms=0;
            lock_flag=1;//关门
            show_flag=0;//显示汉字关门
            audio_play(2);//语音播放
        }
      }

      if(password_error==3)//如果密码错误3次
      {
          if(++time1000ms==1000)//一秒
          {
              time1000ms=0;
              if(--time15s==0)//15秒倒计时
              {
               time15s=15;
               password_error=0;//密码错误次数归零
               show_flag=0;
              }
          }
      }


    }
    TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
}
/*任务调度器相关*/
typedef struct
{
    void (*task_func)(void);//任务函数
    unsigned long int rate_ms;  //任务执行周期
    unsigned long int last_run; //任务上次运行的时间
}task_t;

task_t scheduler_task[]={
        {lcd_proc,100,0},//屏幕处理函数，100毫秒执行一次，0秒开始执行
        {key_proc,10,0},//按键处理函数，10毫秒执行一次，0秒开始执行
        {lock_proc,30,0},//门锁处理函数（舵机开关）
        {as608_proc,20,0},
        {esp8266_proc,2,0},
};
unsigned char task_num;  //任务数量变量
void scheduler_init()
{
    task_num=sizeof(scheduler_task)/sizeof(task_t);  //计算任务数量
}
void scheduler_run()
{
    unsigned char i;
    for(i=0;i<task_num;i++)
    {
        unsigned long int now_time=uwtick;  //更新当前系统时间
        if(now_time> (scheduler_task[i].last_run+scheduler_task[i].rate_ms) )
        {
            scheduler_task[i].last_run=now_time;//记录本次运行时间
            scheduler_task[i].task_func();//运行任务
        }
    }
}

/*主函数*/
int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//中断优先级分组
	SystemCoreClockUpdate();//时钟配置
	USART_Printf_Init(115200);//串口1初始化
	Delay_Init();//延时函数初始化
	TIM2_PWM_Init();//舵机初始化（TIM2输出PWM）
    lock(1);//初始默认上锁
    LCD_Init();//屏幕初始化
    LCD_Fill(0,0,127,127,WHITE);//整个屏幕填充白色
    lcd_show_chinese(20,50,"正在启动",RED,WHITE,16,0);//开机显示
    unsigned char i=0;//循环次数
//    while(i<128)//循环划线实现进度条的效果
//    {
//    LCD_DrawLine(i,100,i,128,RED);
//    Delay_Ms(10);
//    i++;
//    }

    lcd_show_chinese(20,50,"启动成功",RED,WHITE,16,0);//开机成功
    Delay_Ms(500);
    LCD_ShowPicture(0,0,128,128,gImage_1);//显示logo
    //Delay_Ms(1000);
    LCD_ShowPicture(0,0,128,128,gImage_2);//显示主页
    lcd_show_chinese(0,0,"门已上锁，请输入密码",RED,WHITE,16,0);//开机成功
    LCD_Fill(16,45,112,66,YELLOW);//填充黄色背景
    key_init();//矩阵按键初始化
    Tim3_Init(1000,96-1);//系统计时定时器初始化
    scheduler_init();//调度器初始化
    audio_init();
    audio_yinliang(20);
    Delay_Ms(10);
    audio_play(2);
    Usart2_Init();
    as608_init();
    esp8266_init();
    Delay_Ms(10);
    onenet_init();

	while(1)
    {
	    scheduler_run();//调度器运行
	}
}


/*串口6中断*/
void UART6_IRQHandler(void)
{
    u8 temp=0;
    if(USART_GetITStatus(UART6, USART_IT_RXNE) != RESET)
       {
        uart6_rec_tick=0;
        temp=USART_ReceiveData(UART6);
        uart6_rec_string[uart6_rec_index]=temp;
        uart6_rec_index++;
       }
    USART_ClearITPendingBit(UART6, USART_IT_RXNE);
}



/*串口7中断*/
void UART7_IRQHandler(void)
{
    u8 temp=0;
    if(USART_GetITStatus(UART7, USART_IT_RXNE) != RESET)
       {
        ps_wait_flag=0;
        if(uart7_rec_tick>10)uart7_rec_index=0;
        temp=USART_ReceiveData(UART7);
        uart7_rec_string[uart7_rec_index]=temp;
        uart7_rec_index++;
        uart7_rec_tick=0;
       }
    USART_ClearITPendingBit(UART7, USART_IT_RXNE);
}



//串口2中断
void USART2_IRQHandler(void)
{
    u8 temp;
    if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
    {
        temp=USART_ReceiveData(USART2);
        switch(rfid_index)
                {
                    case 0:
                        if(temp==0x04)rfid_index++;
                    break;
                    case 1:
                        if(temp==0x0c)rfid_index++;
                        else rfid_index=0;
                    break;
                    case 2:
                        if(temp==0x02)rfid_index++;
                        else rfid_index=0;
                    break;
                    case 3:
                        if(temp==0x30)rfid_index++;
                        else rfid_index=0;
                    break;
                    case 4:
                        if(temp==0x00)rfid_index++;
                        else rfid_index=0;
                    break;
                    case 5:
                        if(temp==0x04)rfid_index++;
                        else rfid_index=0;
                    break;
                    case 6:
                        if(temp==0x00)rfid_index++;
                        else rfid_index=0;
                    break;
                    case 7:
                        rfid_temp[0]=temp;
                        rfid_index++;
                    break;
                    case 8:
                        rfid_temp[1]=temp;
                        rfid_index++;
                    break;
                    case 9:
                        rfid_temp[2]=temp;
                        rfid_index++;
                    break;
                    case 10:
                        rfid_temp[3]=temp;
                        rfid_index=0;
                        if(mode==0)
                        {
                            if( rfid_chek() )
                            {
                                audio_play(11);
                                lock_flag=0;
                            }
                            else
                            {
                                audio_play(12);


                            }
                        }
                        if(mode==4)
                        {
                            string_copy(rfid_temp,rfid[rfid_password_index],4);
                            audio_play(15);
                            mode=0;
                            rfid_password_index++;

                        }

                    break;
                }
    }
    USART_ClearITPendingBit(USART2, USART_IT_RXNE);
}

//串口3中断(发送中断+接收中断)
void USART3_IRQHandler(void)
{
    u8 temp;
    if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
    {
        temp=USART_ReceiveData(USART3);

    }
    USART_ClearITPendingBit(USART3, USART_IT_RXNE);
}
void key_clear()//键盘暂存区清空函数
{
    memset(key_temp,10,6);
    key_index=0;
}
u8  string_chek(u8* string1,u8* string2,u8 len)//检测两个数组前len位是否一致
{
    while(len--)
    {
        if(string1[len]==string2[len]);
        else return 0;
    }
    return 1;
}
//数组复制函数
void string_copy(u8* string1,u8* string2,u8 len)
{
    u8 i;
    for (i = 0; i < len; ++i)
    {
      string2[i]=string1[i];
    }
}

u8 rfid_chek()
{
    u8 i;
    for (i=0; i<rfid_password_index; ++i)
    {
       if( string_chek(rfid_temp,rfid[i],4) )return 1;
    }
    return 0;
}

void Ps_Wait()
{
  ps_wait_flag=1;
  do
  {
   Delay_Ms(200);
  }
   while(ps_wait_flag);
}


u32 json(u8* string, u8* params, u8 len)
{
    char* str = (char*)string;
    char* key = (char*)params;

    // 构造搜索模式："params":{
    char* params_start = strstr(str, "\"params\":{");
    if (!params_start) return 0;
    params_start += 9;  // 跳过"params":{ 共9个字符

    // 构造键值匹配模式
    char pattern[32];
    snprintf(pattern, sizeof(pattern), "\"%s\":", key);

    // 在params对象内搜索目标字段
    char* key_pos = strstr(params_start, pattern);
    if (!key_pos) return 0;

    // 定位数值起始位置
    char* val_start = key_pos + strlen(pattern);

    // 跳过可能的空格和制表符
    while (*val_start == ' ' || *val_start == '\t') val_start++;

    /* 新增布尔值解析 */
    // 检查true/false
    if (*val_start == 't') {
        if (strncmp(val_start, "true", 4) == 0) {
            // 验证后续字符合法性（逗号/右括号/空格）
            char next_char = val_start[4];
            if (next_char == ',' || next_char == '}' ||
                next_char == ' ' || next_char == '\t') {
                return 1;
            }
        }
    }
    else if (*val_start == 'f') {
        if (strncmp(val_start, "false", 5) == 0) {
            // 验证后续字符合法性
            char next_char = val_start[5];
            if (next_char == ',' || next_char == '}' ||
                next_char == ' ' || next_char == '\t') {
                return 0;
            }
        }
    }

    // 数字解析
    u32 result = 0;
    for (u8 i = 0; i < len; i++) {
        if (*val_start >= '0' && *val_start <= '9') {
            result = result * 10 + (*val_start - '0');
            val_start++;
        } else {
            break;
        }
    }

    return result;
}
