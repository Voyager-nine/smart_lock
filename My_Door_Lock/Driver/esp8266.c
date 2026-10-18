#include "esp8266.h"

#include <string.h>

extern uint8_t password[6];

u8 string1[]="AT+RST\r\n";
u8 string2[]="AT+CWMODE=1\r\n";
u8 string3[]="AT+CWMODE=1\r\n";
u8 string4[]="AT+CWJAP=\"Kk\",\"12241224\"\r\n";
u8 string5[]="AT+MQTTUSERCFG=0,1,\"CH32\",\"ouBbCsG398\",\"version=2018-10-31&res=products%2FouBbCsG398%2Fdevices%2FCH32&et=1792508400&method=md5&sign=a5C7YI8l%2F2Yauyfgo8Zh7w%3D%3D\",0,0,\"\"\r\n";
u8 string6[]="AT+MQTTCONN=0,\"mqtts.heclouds.com\",1883,1\r\n";
u8 string7[]="AT+MQTTSUB=0,\"$sys/ouBbCsG398/CH32/thing/property/set\",1\r\n";
u8 string8[]="AT+MQTTPUB=0,\"$sys/ouBbCsG398/CH32/thing/property/post\",\"{\\\"id\\\":\\\"123\\\"\\,\\\"params\\\":{\\\"password\\\":{\\\"value\\\":67}}}\",0,0\r\n";

static uint8_t onenet_step;
static uint32_t onenet_started;

#define ONENET_JSON_CAPACITY 256

static char onenet_json[ONENET_JSON_CAPACITY];
static uint16_t onenet_json_len;
static uint8_t onenet_json_depth;
static uint8_t onenet_json_string;
static uint8_t onenet_json_escape;

static const char *onenet_skip_space(const char *text)
{
    while(*text == ' ' || *text == '\t' || *text == '\r' || *text == '\n')
        text++;
    return text;
}

static uint8_t onenet_read_password(const char *json, uint8_t *value)
{
    const char *p;
    uint8_t count = 0;

    p = strstr(json, "\"password\"");
    if(p == 0)
        return 0;
    p = strchr(p, ':');
    if(p == 0)
        return 0;
    p = onenet_skip_space(p + 1);

    /* OneNET may wrap a property in an object: {"value": "123456"}. */
    if(*p == '{')
    {
        p = strstr(p, "\"value\"");
        if(p == 0)
            return 0;
        p = strchr(p, ':');
        if(p == 0)
            return 0;
        p = onenet_skip_space(p + 1);
    }

    if(*p == '"')
    {
        p++;
        while(*p >= '0' && *p <= '9' && count < 6)
            value[count++] = (uint8_t)*p++;
    }
    else
    {
        while(*p >= '0' && *p <= '9' && count < 6)
            value[count++] = (uint8_t)*p++;
    }
    return count;
}

static uint8_t onenet_read_lock(const char *json, uint8_t *value,
                                uint8_t *is_boolean)
{
    const char *p;

    p = strstr(json, "\"lock\"");
    if(p == 0)
        return 0;
    p = strchr(p, ':');
    if(p == 0)
        return 0;
    p = onenet_skip_space(p + 1);

    *is_boolean = 0;

    /* Accept both lock:1 and lock:{"value":1}. */
    if(*p == '{')
    {
        p = strstr(p, "\"value\"");
        if(p == 0)
            return 0;
        p = strchr(p, ':');
        if(p == 0)
            return 0;
        p = onenet_skip_space(p + 1);
    }

    if(strncmp(p, "true", 4) == 0)
    {
        *value = 1;
        *is_boolean = 1;
        return 1;
    }
    if(strncmp(p, "false", 5) == 0)
    {
        *value = 0;
        *is_boolean = 1;
        return 1;
    }
    if(*p < '0' || *p > '9')
        return 0;
    *value = (uint8_t)(*p - '0');
    return 1;
}

static void onenet_process_property(const char *json)
{
    uint8_t remote_password[6];
    uint8_t remote_password_len;
    uint8_t lock_value;
    uint8_t lock_is_boolean;
    uint8_t i;

    if(strstr(json, "\"params\"") == 0)
        return;

    /* A cloud command must contain the correct six-digit password. */
    if(strstr(json, "\"password\"") == 0)
        return;

    remote_password_len = onenet_read_password(json, remote_password);
    if(remote_password_len != 6)
        return;

    for(i = 0; i < 6; i++)
    {
        if(remote_password[i] != password[i])
        {
            lcd_prompt(LCD_PROMPT_PASSWORD_WRONG);
            return;
        }
    }

    if(onenet_read_lock(json, &lock_value, &lock_is_boolean) == 0)
        return;

    /* Boolean property: true locks, false unlocks. */
    if(lock_is_boolean == 0)
        return;
    if(lock_value == 0)
    {
        lock_flag = 0;
        lcd_prompt(LCD_PROMPT_PASSWORD_CORRECT);
    }
    else
    {
        lock_flag = 1;
        lcd_prompt(LCD_PROMPT_LOCKED);
    }

    password_len = 0;
    refresh_lcd = 1;
}

static void onenet_receive_bytes(const u8 *data, u8 len)
{
    u8 i;

    for(i = 0; i < len; i++)
    {
        if(onenet_json_depth == 0)
        {
            if(data[i] != '{')
                continue;
            onenet_json_len = 0;
            onenet_json_string = 0;
            onenet_json_escape = 0;
            onenet_json_depth = 1;
            onenet_json[onenet_json_len++] = '{';
        }
        else
        {
            if(onenet_json_len >= ONENET_JSON_CAPACITY - 1)
            {
                onenet_json_len = 0;
                onenet_json_depth = 0;
                onenet_json_string = 0;
                onenet_json_escape = 0;
                continue;
            }

            onenet_json[onenet_json_len++] = (char)data[i];

            if(onenet_json_escape)
                onenet_json_escape = 0;
            else if(onenet_json_string && data[i] == '\\')
                onenet_json_escape = 1;
            else if(data[i] == '"')
                onenet_json_string = !onenet_json_string;
            else if(onenet_json_string == 0 && data[i] == '{')
                onenet_json_depth++;
            else if(onenet_json_string == 0 && data[i] == '}')
            {
                onenet_json_depth--;
                if(onenet_json_depth == 0)
                {
                    onenet_json[onenet_json_len] = 0;
                    onenet_process_property(onenet_json);
                    onenet_json_len = 0;
                }
            }
        }
    }
}

void esp8266_init(void)
{
    Uart6_Init();
}


void onenet_init(void)
{
    uart6_send_string(string1,sizeof(string1)-1);
    Delay_Ms(1000);
    uart6_send_string(string2,sizeof(string2)-1);
    Delay_Ms(1000);
    uart6_send_string(string3,sizeof(string3)-1);
    Delay_Ms(1000);
    uart6_send_string(string4,sizeof(string4)-1);
    Delay_Ms(1000);
    Delay_Ms(1000);
    uart6_send_string(string5,sizeof(string5)-1);
    Delay_Ms(1000);
    Delay_Ms(1000);
    uart6_send_string(string6,sizeof(string6)-1);
    Delay_Ms(1000);
    Delay_Ms(1000);
    uart6_send_string(string7,sizeof(string7)-1);
    Delay_Ms(1000);
    Delay_Ms(1000);
    uart6_send_string(string8,sizeof(string8)-1);
    Delay_Ms(1000);
    Delay_Ms(1000);






}

void onenet_task(void)
{
    u8 reply[32];
    u8 length = uart6_read_bytes(reply, sizeof(reply));

    if(length)
    {
        usart1_send_string(reply, length);
        onenet_receive_bytes(reply, length);
    }

    if(onenet_step == 1 && (uint32_t)(GetTick() - onenet_started) >= 1000)
    {
        uart6_send_string(string2, sizeof(string2) - 1);
        onenet_step = 0;
    }
}
