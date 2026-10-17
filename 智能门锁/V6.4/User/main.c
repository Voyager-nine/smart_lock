/*头文件引用区*/
#include "debug.h"//系统头文件，内含ch32vxx.h头文件
#include "stdio.h"//c语言标准输入输出库
#include "iic.h"//IIC头文件，控制EEPROM
#include "lock.h"//控制门锁开关，舵机代码
#include "Uart.h"//串口头文件
#include "Timer.h"//定时器
#include "trumpt.h"//喇叭，用于播放语音
#include "Key.h"//矩阵按键
#include "As608.h"//指纹


/*函数声明区*/
void USART3_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));//串口3快速中断
void TIM2_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));//定时器2快速中断
void UART7_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));//串口7快速中断
void UART6_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));//串口6快速中断
void UART8_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));//串口8快速中断
void Sys_Init();//系统初始化函数
u8 string_chek(u8* string1,u8* string2,u8 len);//识别两个数组前len位是不是一样，可以用string库中的memcmp实现，此处自己定义
void string_copy(u8* string1,u8* string2,u8 len);//将string1前len位复制给string2，可用标准库memcpy实现，此处自己定义
void Clear_All();//清空屏幕
/*变量声明区*/
/*数组区*/
u8 RFID1[]={0,0,0,0,0};//RFID卡密码1
u8 RFID2[]={0,0,0,0,0};//RFID卡密码2
u8 RFID3[]={0,0,0,0,0};//RFID卡密码3
u8 RFID4[]={0,0,0,0,0};//RFID卡密码4
u8 RFID5[]={0,0,0,0,0};//RFID卡密码5
u8 RFID_temp[]={0,0,0,0,0,0};//RFID卡临时存储数组
u8 screen_string[30]={0};//串口屏控制数组
u8 mima[6]={1,2,3,4,5,6};//门锁密码储存数组
u8 cmd_mima[6]={2,7,7,5,1,6};//管理员密码储存数组
u8 key_temp[7]={0,0,0,0,0,0,0};//按键临时储存数组
u8 Rc522_Clear[4]={0xff,0xff,0xff,0xff};//清楚RFID卡使用，全部写入0XFF
u8 Uart8_Rx_State;//串口8接收状态变量
u8   Uart8_Rx_Buf[20]={0};//串口8接收临时储存数组
/*定时区*/
u16 time1000ms;//定时1秒
u16 time5000ms;//定时5秒，用于开门一定时间后自动关门
u8 Show_Slow;//减速变量
u8 Sound_Slow;//减速变量
u8 Key_Slow;//减速变量，推荐10ms
u8 Key_Val,Key_Down,Key_Up,Key_Old;//按键消抖，提取上升沿下降沿
u8 Fig_Slowdown;//指纹减速变量
u8 time_s=20;//密码3次输错锁定时间
u8 time_m=0;//密码3次输错锁定时间
u8 time_h=0;//密码3次输错锁定时间
/*状态标志位*/
u8 Uart_Rx_State;//串口接收
u8 key_state;//按键状态
u8 Show_Mode;//0主界面  1修改密码  2添加卡片 3录入指纹
u8 lock_flag;//门锁标志位，1门开，0门关
u8 lock_falg_old=1;//门锁上一次状态，有变化才做出反应
u8 yuyin_flag;//语音模块标志位
u8 cmd_lock=1;//管理员锁，1开，无法修改密码等，输入管理员密码解锁
u8 show_flag;//显示标志位
u8 mima_error;//密码错误3次标志位
u8 Rc522_flag;//RFID卡模式，0寻卡模式 1写卡模式 2删卡模式
u8 Key_True;//真正作用Key值，因为矩阵键盘和串口屏同步同功能，两个key都传给Key_True再做处理
u8 cmd_error;//管理员密码错误3次标志位
u8 Fig_Num;//指纹编号
u8 start;
u8 RFID_num=0;//RFID卡编号，
u8 Usart3_State;//串口3接收
/*外部变量*/
extern u16 Usart2_Rc_Buff[20];

/*按键处理函数***********************************************************/
void Key_Proc()
{
    if(Key_Slow)return;
    Key_Slow=1;

    if(mima_error==3)return;//如果密码输入错误3次或者管理员密码输入错误3次将锁定，不对按键响应
    if(cmd_error==3)return;

    /*读取矩阵按键值，3行消抖*/
    Key_Val=Key_Read();
    Key_Down=Key_Val&(Key_Val^Key_Old);
    Key_Up=~Key_Val&(Key_Val^Key_Old);
    Key_Old=Key_Val;

    /*将下降沿赋值给Key_True,串口屏在中断里直接赋值给Key_True*/
    if(Key_Down)Key_True=Key_Down;

    /*响应按键操作*/
    if(Key_True)
    {
       switch(Key_True)
       {
        case 0:

        break;
        /*将前几次按键值存储在数组里*/
        case 1:
        key_temp[key_state]=1;
        key_state++;
        break;
        case 2:
        key_temp[key_state]=2;
        key_state++;
        break;
        case 3:
        key_temp[key_state]=3;
        key_state++;
        break;
        case 4:
        key_temp[key_state]=4;
        key_state++;
        break;
        case 5:
        key_temp[key_state]=5;
        key_state++;
        break;
        case 6:
        key_temp[key_state]=6;
        key_state++;
        break;
        case 7:
        key_temp[key_state]=7;
        key_state++;
        break;
        case 8:
        key_temp[key_state]=8;
        key_state++;
        break;
        case 9:
        key_temp[key_state]=9;
        key_state++;
        break;

        /*按下10，密码输入回退一位*/
        case 10:
        key_temp[key_state-1]=0;
        if(key_state)key_state--;
        break;

        /*按下11，清空当前输入*/
        case 11:
        key_temp[key_state]=0;
        key_state++;
        break;

        /*按下12，确定按键*/
        case 12:
             if(Show_Mode==0)//主页模式下
             {
                  if(string_chek(mima,key_temp,6))//如果键盘输入和密码一样
                  {
                     yuyin_flag=2;//播报语音2
                     lock_flag=1;//门打开
                     mima_error=0;//密码错误次数清零
                  }
                  else
                  {
                      /*按键临时数组归0*/
                    key_temp[0]=0; key_temp[1]=0; key_temp[2]=0; key_temp[3]=0; key_temp[4]=0; key_temp[5]=0;
                    /*按键状态归0*/
                    key_state=0;
                    yuyin_flag=4;//播放语音4*/
                    mima_error++;//密码错误次数+1
                    if(mima_error==3)//如果密码输入错误次数等于3
                    {    time_s=20;//锁定20秒
                         show_flag=20;//显示20
                         yuyin_flag=5;//播放语音5

                         sprintf(screen_string,"page 2");//串口屏跳转到page 2
                         screen_printf(screen_string);
                    }
                  }
             }

             if(Show_Mode==1)//修改密码界面
             {
                 if(cmd_lock==1)//管理员密码没解锁
                 {
                         if(string_chek(cmd_mima,key_temp,6))//检查管理员密码是否正确
                         {
                           key_state=0;//按键状态清零
                           /*键盘临时数组清零*/
                           key_temp[0]=0; key_temp[1]=0; key_temp[2]=0; key_temp[3]=0; key_temp[4]=0; key_temp[5]=0;
                           yuyin_flag=10;//播放语音10
                           show_flag=19;//显示19
                           cmd_lock=0;//管理员解锁
                         }
                         else
                         {
                           /*按键临时数组清零*/
                           key_temp[0]=0; key_temp[1]=0; key_temp[2]=0; key_temp[3]=0; key_temp[4]=0; key_temp[5]=0;
                           /*按键状态清零*/
                           key_state=0;
                           yuyin_flag=14;//播放语音14
                           cmd_error++;//管理员密码错误次数+1

                           if(cmd_error==3)//如果管理员密码错误3次
                           {
                               show_flag=20;//显示20
                               yuyin_flag=5;//播放语音5
                               sprintf(screen_string,"page 2");//串口屏跳转到界面2
                               screen_printf(screen_string);
                           }
                         }
                 }
                 else
                 {
                     string_copy(mima, key_temp, 6);//修改密码成功，将按键临时数组内容赋值给密码数组
                     key_state=0;
                     key_temp[0]=0; key_temp[1]=0; key_temp[2]=0; key_temp[3]=0; key_temp[4]=0; key_temp[5]=0;
                     yuyin_flag=11;
                     show_flag=14;
                     Show_Mode=0;
                     cmd_lock=1;//管理员锁重新上锁
                     EEPROM_Write(mima,1,6);//将新密码写入eeprom
                 }
             }

             if(Show_Mode==2)//添加卡片
             {
                 if(cmd_lock==1)
                 {
                         if(string_chek(cmd_mima,key_temp,6))//检查管理员密码
                         {
                           key_state=0;
                           key_temp[0]=0; key_temp[1]=0; key_temp[2]=0; key_temp[3]=0; key_temp[4]=0; key_temp[5]=0;
                           yuyin_flag=18;
                           show_flag=23;
                           cmd_lock=0;
                         }
                         else
                         {
                           key_temp[0]=0; key_temp[1]=0; key_temp[2]=0; key_temp[3]=0; key_temp[4]=0; key_temp[5]=0;
                           key_state=0;
                           yuyin_flag=14;
                           cmd_error++;
                           if(cmd_error==3)
                           {
                               show_flag=20;
                               yuyin_flag=5;
                               sprintf(screen_string,"page 2");
                               screen_printf(screen_string);
                           }
                         }
                 }

             }
             if(Show_Mode==3)//添加指纹
                   {
                       if(cmd_lock==1)
                       {
                               if(string_chek(cmd_mima,key_temp,6))//检查管理员密码
                               {
                                 key_state=0;
                                 key_temp[0]=0; key_temp[1]=0; key_temp[2]=0; key_temp[3]=0; key_temp[4]=0; key_temp[5]=0;
                                 yuyin_flag=7;
                                 show_flag=22;
                                 cmd_lock=0;
                               }
                               else
                               {
                                 key_temp[0]=0; key_temp[1]=0; key_temp[2]=0; key_temp[3]=0; key_temp[4]=0; key_temp[5]=0;
                                 key_state=0;
                                 yuyin_flag=14;
                                 cmd_error++;
                                 if(cmd_error==3)
                                 {
                                     show_flag=20;
                                     yuyin_flag=5;
                                     sprintf(screen_string,"page 2");
                                     screen_printf(screen_string);
                                 }
                               }
                       }
                   }
        break;
        /*修改密码*/
        case 13:
               if(Show_Mode)return;
               Show_Mode=1;
               show_flag=15;
               yuyin_flag=9;
               key_state=0;
               key_temp[0]=0; key_temp[1]=0; key_temp[2]=0; key_temp[3]=0; key_temp[4]=0; key_temp[5]=0;
        break;
        /*录入指纹*/
        case 14:
            if(Show_Mode)return;
            Show_Mode=3;
            show_flag=15;
            yuyin_flag=6;
            key_state=0;
            key_temp[0]=0; key_temp[1]=0; key_temp[2]=0; key_temp[3]=0; key_temp[4]=0; key_temp[5]=0;

        break;
        /*添加卡片*/
        case 15:
            if(Show_Mode)return;
            Show_Mode=2;
            yuyin_flag=17;
            show_flag=15;
            key_state=0;
            key_temp[0]=0; key_temp[1]=0; key_temp[2]=0; key_temp[3]=0; key_temp[4]=0; key_temp[5]=0;
        break;
        /*回到主页*/
        case 16:
             cmd_lock=1;
             Show_Mode=0;
             show_flag=14;
             key_state=0;
             key_temp[0]=0; key_temp[1]=0; key_temp[2]=0; key_temp[3]=0; key_temp[4]=0; key_temp[5]=0;
        break;
        case 17:
             lock_flag=1;

        break;
       }
       if(key_state==7)key_state=6;//防止按键一直输入，溢出
       Key_True=0;
    }
}

/*声音处理函数**************************************************************/
//1门已上锁，请刷卡或输入密码解锁
//2门已开启，欢迎回家
//3欢迎使用乡村智居系统
//4密码错误，请重新输入
//5连续三次密码错误，已锁定，请等待解锁后重试一下
//6录入指纹，请输入管理员密码
//7管理员密码验证成功，请将手指平放在传感器上
//8指纹录入成功
//9修改开锁密码，请输入管理员密码
//10管理员密码验证成功，请输入新密码
//11新密码设置成功
//12指纹验证成功，门已开启
//13指纹验证失败，请重试
//14管理员密码错误，请重新输入
//15刷卡成功，门已开启
//16刷卡失败，请重试
//17添加卡片，请输入管理员密码
//18管理员密码验证成功，请将卡片放置在传感器上
//19卡片录入成功
void Sound_Proc()
{
    if(Sound_Slow)return;
    Sound_Slow=1;
   if(start==0)//上电开始只执行一次，锁门并播放语音
   {
    Lock(0);
    start=1;
   }

    if(yuyin_flag!=0)
    {
        trumpet(yuyin_flag);
        switch(yuyin_flag)
        {
        case 16:
            Delay_Ms(1500);//防止刷卡时连续播报刷卡失败
        break;
        }
        yuyin_flag=0;
    }
}


/*信息处理函数***************************************************************/
void Show_Proc()
{
    if(Show_Slow)return;
    Show_Slow=1;
    /*信息获取区*/

    /*信息显示区*/

    /*串口屏图片显示，show_flag对应*/
    /*14请输入密码
     *15请输入管理员密码
     *16管理员密码错误
     *17密码错误
     *18门已开启
     *19请输入新密码
     *20已锁定
     *21请输入指纹编号
     *22指纹录入中
     *23卡片添加中
     */

//    前期调试指纹和eeprom时使用
//    sprintf(screen_string,"n0.val=%d",Fig_Num);
//    screen_printf(screen_string);


if(show_flag!=0)//串口屏状态显示区
{
    sprintf(screen_string,"p5.pic=%d",show_flag);
    screen_printf(screen_string);
    show_flag=0;
}

if( (mima_error==3) ||  (cmd_error==3)  )//密码错误3次串口屏显示倒计时
{
    sprintf(screen_string,"n1.val=%d",time_s);
     screen_printf(screen_string);
}

/*锁门或者开门*/
if(lock_flag&(lock_flag^lock_falg_old))
      {
        Lock(1);
        key_state=0;
        key_temp[0]=0; key_temp[1]=0; key_temp[2]=0; key_temp[3]=0; key_temp[4]=0; key_temp[5]=0;
        time5000ms=0;
        show_flag=18;
      }
      if(~lock_flag&(lock_flag^lock_falg_old))
      {
          Lock(0);
          yuyin_flag=1;
          show_flag=14;
      }
      lock_falg_old=lock_flag;

/*主页模式*/
if(Show_Mode==0)
{
   sprintf(screen_string,"j3.val=%d",key_state*15+3);//密码输入框的*个数
   screen_printf(screen_string);
}

/*修改密码*/
if(Show_Mode==1)
{
    sprintf(screen_string,"j3.val=%d",key_state*15+3);
    screen_printf(screen_string);
}

/*添加卡片*/
if(Show_Mode==2)
{
    sprintf(screen_string,"j3.val=%d",key_state*15+3);
    screen_printf(screen_string);
}

/*录入指纹*/
if(Show_Mode==3)
{
    sprintf(screen_string,"j3.val=%d",key_state*15+3);//密码输入框的*个数
    screen_printf(screen_string);

    if(cmd_lock)return;//管理员密码上锁
    if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1)==1)//判断指纹模块上是否有手指
    {
        input_fingerprint(Fig_Num);
        PS_GetImage();
        PS_GetImage();
        PS_GetImage();
        Contrast_fingerprint();

        Fig_Num++;//指纹编号+1
        EEPROM_Write(&Fig_Num, 110, 1);//存储指纹编号到eeprom
        //yuyin_flag=8;
        trumpet(8);//播放语音8
        show_flag=14;
        Show_Mode=0;
        cmd_lock=1;
        Usart2_Rc_Buff[1]=2;
        while(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1)==1);//等待手指移开
    }
}

}
/*中断服务函数*/
void TIM2_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM2, TIM_IT_Update)!=RESET)
    {
        if(++Key_Slow==10)Key_Slow=0;//控制按键处理函数打开频率
        if(++Show_Slow==50)Show_Slow=0;//控制显示函数刷新频率
        if(++Sound_Slow==50)Sound_Slow=0;//控制声音处理函数刷频频率
        if(++Fig_Slowdown==40)Fig_Slowdown=0;//指纹识别函数减速变量

        if(  (mima_error==3)  ||  (cmd_error==3))//密码错误3次
        {
         if(++time1000ms==1000)//倒计时
         {
                 time1000ms=0;
                 if(time_s--==0)
                 {
                     sprintf(screen_string,"page 1");
                     screen_printf(screen_string);
                     sprintf(screen_string,"page 1");
                     screen_printf(screen_string);
                      show_flag=14;
                      Show_Mode=0;
                      time_s=20;
                      mima_error=0;
                      cmd_error=0;
                 }
             }
        }
        if(++time5000ms==5000)//控制开门后5秒自动关门
        {
            lock_flag=0;
            if (time5000ms>5000)time5000ms=5001;
        }
    }
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
}

/*串口屏接收函数*/
void UART7_IRQHandler(void)
{
    u8 temp=0;
    if(USART_GetITStatus(UART7, USART_IT_RXNE) != RESET)
       {

        temp=USART_ReceiveData(UART7);

        switch(Uart_Rx_State)
        {
        /*串口屏自定义包头*/
        case 0:
            if(temp!=0x90)Uart_Rx_State=0;
            else Uart_Rx_State++;
        break;
        case 1:
            if(temp!=0x8e)Uart_Rx_State=0;
            else Uart_Rx_State++;
        break;
        /*串口屏键码*/
        case 2:
             Key_True=temp;
             Uart_Rx_State++;

        break;
        case 3:
            Uart_Rx_State=0;
        break;
        }
       }
    USART_ClearITPendingBit(UART7, USART_IT_RXNE);
}


void UART6_IRQHandler(void)
{
    u8 temp=0;
    if(USART_GetITStatus(UART6, USART_IT_RXNE) != RESET)
       {

        temp=USART_ReceiveData(UART6);


       }
    USART_ClearITPendingBit(UART6, USART_IT_RXNE);
}

/*指纹处理函数*/
void Fingerprint_Proc()
{
    if(Show_Mode!=0)return;//不在主页模式下直接退出
    if(Fig_Slowdown)return;//1ms减速
    Fig_Slowdown=1;

    if(lock_flag)return;//门已经打开的情况直接退出

    if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1)==1)//判断传感器上是否有手指
    {
         PS_GetImage();
         PS_GetImage();
         PS_GetImage();
         Contrast_fingerprint();

         if(Usart2_Rc_Buff[2]>50)//可信度大于50，认为匹配成功，该数据包解析在串口2接收中断
         {
             yuyin_flag=12;
             lock_flag=1;
         }
         else
         {
             Usart2_Rc_Buff[1]=2;
             //yuyin_flag=13;
             trumpet(13);
             while(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1)==1);//等待手指移开
         }
     }
}

/*门禁卡*/
void USART3_IRQHandler(void)
{
    u8 temp;
    if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
    {
        temp=USART_ReceiveData(USART3);

        switch(Usart3_State)
        {

        /*包头及数据类型，数据长度等*/
        case 0:if(temp==0x04)Usart3_State++;else Usart3_State=0;
        break;
        case 1:if(temp==0x0C)Usart3_State++;else Usart3_State=0;
        break;
        case 2:if(temp==0x02)Usart3_State++;else Usart3_State=0;
        break;
        case 3:if(temp==0x30)Usart3_State++;else Usart3_State=0;
        break;
        case 4:if(temp==0x00)Usart3_State++;else Usart3_State=0;
        break;
        case 5:if(temp==0x04)Usart3_State++;else Usart3_State=0;
        break;
        case 6:if(temp==0x00)Usart3_State++;else Usart3_State=0;
        break;

        /*RFID卡号*/
        case 7:
               RFID_temp[0]=temp;
               Usart3_State++;
        break;
        case 8:
            RFID_temp[1]=temp;
            Usart3_State++;

        break;
        case 9:
            RFID_temp[2]=temp;
            Usart3_State++;

        break;
        case 10:
            RFID_temp[3]=temp;
             Usart3_State++;

        break;
        case 11:
            RFID_temp[4]=temp;
            Usart3_State=0;
            if(Show_Mode==2)//卡片录入模式
            {

                if(cmd_lock==0)//管理员锁解开状态
                {
                    /*将接收到的卡号添加到数组*/
                    RFID_num++;
                    switch(RFID_num)
                    {
                        case 1:
                            string_copy(RFID1, RFID_temp, 5);
                        break;
                        case 2:
                            string_copy(RFID2, RFID_temp, 5);
                        break;
                        case 3:
                            string_copy(RFID3, RFID_temp, 5);
                        break;
                        case 4:
                            string_copy(RFID4, RFID_temp, 5);
                        break;
                        case 5:
                            string_copy(RFID5, RFID_temp, 5);
                        break;
                    }

                    /*将当前卡的数量写入eeprom*/
                    EEPROM_Write(&RFID_num, 60, 1);

                    /*将卡号写入eeprom*/
                    switch(RFID_num)
                    {
                    case 1:
                        EEPROM_Write(RFID1, 10, 5);
                    break;
                    case 2:
                        EEPROM_Write(RFID2, 20, 5);
                    break;
                    case 3:
                        EEPROM_Write(RFID3, 30, 5);
                    break;
                    case 4:
                        EEPROM_Write(RFID4, 40, 5);
                    break;
                    case 5:
                        EEPROM_Write(RFID5, 50, 5);
                    break;
                    }
                    yuyin_flag=19;
                    Show_Mode=0;
                    show_flag=14;
                    cmd_lock=1;
                    return;
                }
            }

            if(Show_Mode==0)//主页状态下
            {
              switch(RFID_num)
              {
                  /*比对已经存在的卡号*/
                  case 5:
                      if(string_chek(RFID_temp, RFID5, 5)){yuyin_flag=15;lock_flag=1;return;}
                  case 4:
                      if(string_chek(RFID_temp, RFID4, 5)){yuyin_flag=15;lock_flag=1;return;}
                  case 3:
                      if(string_chek(RFID_temp, RFID3, 5)){yuyin_flag=15;lock_flag=1;return;}
                  case 2:
                      if(string_chek(RFID_temp, RFID2, 5)){yuyin_flag=15;lock_flag=1;return;}
                  case 1:
                      if(string_chek(RFID_temp, RFID1, 5)){yuyin_flag=15;lock_flag=1;return;}
                  /*未和已存在的卡号比对成功，播放刷卡失败语音*/
                  case 0:
                  {yuyin_flag=16;};
                  break;
              }
            }
        break;
        }
    }
    USART_ClearITPendingBit(USART3, USART_IT_RXNE);
}

/*zigbee模块接收函数*/
void UART8_IRQHandler(void)
{
     u8 temp=6;
    if(USART_GetITStatus(UART8, USART_IT_RXNE) != RESET)
       {
        temp=USART_ReceiveData(UART8);

        switch(Uart8_Rx_State)
                   {
                        /*包头数据类型数据长度等*/
                       case 0:

                             if(temp==0x55)Uart8_Rx_State++;
                             else Uart8_Rx_State=0;
                       break;
                       case 1:
                             if(temp==0xFF)Uart8_Rx_State++;
                             else Uart8_Rx_State=0;
                       break;
                       case 2:
                             if(temp==0x01)Uart8_Rx_State++;
                             else Uart8_Rx_State=0;
                       break;
                       case 3:
                           Uart8_Rx_State++;
                       break;
                       case 4:
                           Uart8_Rx_State++;
                       break;
                       case 5:
                           Uart8_Rx_State++;
                       break;

                       /*用户自定义数据段，长度位10*/
                       case 6:
                            Uart8_Rx_Buf[0]=temp;
                            Uart8_Rx_State++;

                       break;
                       case 7:
                            Uart8_Rx_Buf[1]=temp;
                            Uart8_Rx_State++;
                            if(Uart8_Rx_Buf[0]==4)//为4表示是向门锁发送的数据
                            {
                                lock_flag=temp;//由次位控制大门开关
                            }
                       break;
                       case 8:
                            Uart8_Rx_Buf[2]=temp;
                            Uart8_Rx_State++;
                       break;
                       case 9:
                            Uart8_Rx_Buf[3]=temp;
                            Uart8_Rx_State++;
                       break;
                       case 10:
                            Uart8_Rx_Buf[4]=temp;
                            Uart8_Rx_State++;
                       break;
                       case 11:
                            Uart8_Rx_Buf[5]=temp;
                            Uart8_Rx_State++;
                       break;
                       case 12:
                            Uart8_Rx_Buf[6]=temp;
                            Uart8_Rx_State++;
                       break;
                       case 13:
                            Uart8_Rx_Buf[7]=temp;
                            Uart8_Rx_State++;
                       break;
                       case 14:
                            Uart8_Rx_Buf[8]=temp;
                            Uart8_Rx_State++;
                       break;
                       case 15:
                            Uart8_Rx_Buf[9]=temp;
                            Uart8_Rx_State++;
                       break;
                       case 16:
                           Uart8_Rx_State++;
                       break;
                       case 17:
                           Uart8_Rx_State++;
                           Uart8_Rx_State=0;
                       break;
                       case 18:
                           Uart8_Rx_State++;
                           Uart8_Rx_State=0;
                       break;

                   }
       }
    USART_ClearITPendingBit(UART8, USART_IT_RXNE);
}



/*主函数*/
void main()
{
    Sys_Init();//系统初始化
    //Clear_All();//清除RFID卡记录，并用当前指纹覆盖AS608全部指纹编号 -- 调用时需确保指纹传感器上有手指可供采集
    printf("SystemClk:%d\r\n",SystemCoreClock);
    while(1)
    {
        Fingerprint_Proc();//指纹处理函数
        Key_Proc();//按键处理函数
        Show_Proc();//显示处理函数
        Sound_Proc();//声音处理函数
    }
}

/*初始化函数*/
void Sys_Init()
{
       NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
       SystemCoreClockUpdate();
       Delay_Init();
       USART_Printf_Init(115200);
       Uart6_Init();
       Uart7_Init();
       Lock_Init();
       As608_Init();
       AT24C02_Init();
       EEPROM_Read(mima, 1, 6);//读取eeprom里存储的密码
       EEPROM_Read(&Fig_Num, 110, 1);//读取指纹编号
       Tim2_Init(100, 96-1);//1ms，这里因为板子烧了，速度慢了10倍，没找到原因，正常板子参数1为1000
       Usart2_Init();
       Uart8_Init();
       Key_Init();
       Usart3_Init();
}
/*数组比对函数*/
u8 string_chek(u8* string1,u8* string2,u8 len)
{
    u8 i=0;
    for(i=0;i<len;i++)
    {
       if(string1[i]!=string2[i]) return 0;
    }
    return 1;
}
/*数组复制函数*/
void string_copy(u8* string1,u8* string2,u8 len)
{
    u8 i=0;
        for(i=0;i<len;i++)
        {
           (string1[i]=string2[i]) ;
        }
}

/*清空RFID卡和指纹*/
void Clear_All()
{
    EEPROM_Write(Rc522_Clear,10,5);
    Delay_Ms(1000);
    EEPROM_Write(Rc522_Clear,20,5);
    Delay_Ms(1000);
    EEPROM_Write(Rc522_Clear,30,5);
    Delay_Ms(1000);
    EEPROM_Write(Rc522_Clear,40,5);
    Delay_Ms(1000);
    EEPROM_Write(Rc522_Clear,50,5);
    u8 temp=0;
    EEPROM_Write(&temp, 60, 1);
    u8 i=0;
    PS_GetImage();
    Delay_Ms(100);
    PS_GenChar(1);
    Delay_Ms(100);
    PS_GetImage();
    Delay_Ms(100);
    PS_GenChar(2);
    Delay_Ms(100);
    PS_RegModel();
    Delay_Ms(100);
    while(i<256)
    {
        PS_StoreChar(i);
        Delay_Ms(100);
        i++;
    }
}
