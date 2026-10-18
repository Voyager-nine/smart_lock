"""Host simulation of the real AS608 driver and fingerprint business logic.

Run: python tests/test_as608.py [path-to-native-gcc]
GPIO, UART and time are simulated; no hardware is required.
"""
from pathlib import Path
import subprocess
import sys

root = Path(__file__).resolve().parents[1]
gcc = sys.argv[1] if len(sys.argv) > 1 else r"D:\mingw64\bin\gcc.exe"
header = (root / "Driver/as608.h").read_text(encoding="utf-8-sig").replace('#include "mydefine.h"', "")
driver = (root / "Driver/as608.c").read_text(encoding="utf-8-sig").replace('#include "as608.h"', "")
key = (root / "Driver/key.c").read_text(encoding="utf-8-sig").replace('#include  "key.h"', "")
# Replace only matrix GPIO scanning with a deterministic key event source.
start = key.index("void key_init(void)")
end = key.index("void key_task()", start)
key = key[:start] + "uint8_t key_read(void) { return test_key; }\n" + key[end:]
voices = "\n".join(line for line in (root / "Driver/voice.h").read_text().splitlines() if line.startswith("#define VOICE_"))

mock = r'''
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#define GPIOA 1
#define GPIO_Pin_1 2
#define GPIO_Mode_IPD 3
#define GPIO_Speed_50MHz 4
#define RCC_APB2Periph_GPIOA 5
#define UART7 7
#define USART_IT_RXNE 8
#define USART_FLAG_TXE 9
#define ENABLE 1
#define DISABLE 0
#define RESET 0
#define UART2_FRAME_LEN 12
#define WHITE 0
#define BLUE 1
typedef struct {int GPIO_Pin, GPIO_Mode, GPIO_Speed;} GPIO_InitTypeDef;
static uint32_t tick;
static uint8_t touch, test_key, tx[20], tx_count, played;
static unsigned voice_count, text_count;
volatile uint8_t lock_flag = 1, password_lock_flag;
uint32_t GetTick(void) {return tick;}
void RCC_APB2PeriphClockCmd(int a,int b) {(void)a;(void)b;}
void GPIO_Init(int a,GPIO_InitTypeDef *b) {(void)a;(void)b;}
int GPIO_ReadInputDataBit(int a,int b) {(void)a;(void)b;return touch;}
void Uart7_Init(void) {}
void USART_ITConfig(int a,int b,int c) {(void)a;(void)b;(void)c;}
int USART_GetFlagStatus(int a,int b) {(void)a;(void)b;return 1;}
void USART_SendData(int a,uint8_t b) {(void)a; assert(tx_count < 20); tx[tx_count++] = b;}
void voice_play(uint8_t n) {played=n;voice_count++;}
void uart2_discard_frame(void) {}
uint8_t uart2_read_frame(uint8_t *p) {(void)p;return 0;}
void LCD_Fill(int a,int b,int c,int d,int e) {(void)a;(void)b;(void)c;(void)d;(void)e;}
void LCD_ShowString(int a,int b,const uint8_t *c,int d,int e,int f,int g)
{(void)a;(void)b;(void)d;(void)e;(void)f;(void)g;if(c[0]) text_count++;}
'''

tests = r'''
static void expect(uint8_t cmd)
{
    uint16_t sum=0; unsigned i;
    assert(tx_count >= 12 && tx[9] == cmd);
    assert(tx[8]+9 == tx_count);
    for(i=6;i+2<tx_count;i++) sum+=tx[i];
    assert(sum == ((uint16_t)tx[tx_count-2]<<8 | tx[tx_count-1]));
    tx_count=0;
}
static void feed(const uint8_t *payload, unsigned n, int bad)
{
    uint8_t p[44]={0xEF,1,0xFF,0xFF,0xFF,0xFF,7,0};
    uint16_t sum=0; unsigned i, len=n+11;
    p[8]=n+2; memcpy(p+9,payload,n);
    for(i=6;i<len-2;i++) sum+=p[i];
    p[len-2]=sum>>8; p[len-1]=sum;
    if(bad==1) p[len-1]^=1;
    if(bad==2) p[2]=0;
    for(i=0;i<len;i++) AS608_receive_byte(p[i]);
}
static void ack_code(uint8_t code) { feed(&code,1,0); AS608_proc(); }
static void parameters(void)
{
    uint8_t p[17]={0}; p[5]=1;p[6]=44; // capacity 300
    expect(0x0F); feed(p,17,0);AS608_proc();
}
static void reset(void)
{
    tick+=10000; touch=0;tx_count=0;AS608_init();
    mode_flag=MODE_UNLOCK;admin_verified_flag=0;finger_operation=0;
    finger_pending=0;password_lock_flag=0;lock_flag=1;
}
static void enroll_to_char(void)
{
    uint8_t p[33];
    assert(AS608_start_enroll()); parameters();expect(0x1F);
    memset(p,255,sizeof(p));p[0]=0;feed(p,33,0);AS608_proc();
    assert(tx[10]==1); expect(0x1F);
    memset(p,0,sizeof(p));p[1]=1;feed(p,33,0);AS608_proc();
    assert(AS608_get_id()==257);assert(AS608_get_event()==AS608_EVENT_PLACE);
    tick+=150;AS608_proc();expect(1);ack_code(0);expect(2);
    ack_code(0);assert(AS608_get_event()==AS608_EVENT_REMOVE);
}
static void press(uint8_t n) {test_key=n;key_old=0;key_task();test_key=0;key_task();}
int main(void)
{
    uint8_t full[33], match[]={0,0,42,0,80}, short_ok=0;
    // No finger and a transient no-image reply are normal waiting states.
    reset();assert(AS608_start_enroll());parameters();expect(0x1F);
    memset(full,0,33);feed(full,33,0);AS608_proc();
    assert(AS608_get_event()==AS608_EVENT_PLACE);
    tick+=150;AS608_proc();expect(1);ack_code(2);
    assert(AS608_get_event()==AS608_EVENT_NONE && AS608_busy());
    tick+=149;AS608_proc();assert(tx_count==0);
    tick+=1;AS608_proc();expect(1);ack_code(2);
    reset();enroll_to_char();
    tick+=150;AS608_proc();expect(1);ack_code(0); // still touching
    assert(AS608_get_event()==AS608_EVENT_NONE);
    touch=0;tick+=150;AS608_proc();expect(1);ack_code(2);
    assert(AS608_get_event()==AS608_EVENT_AGAIN);
    tick+=150;AS608_proc();expect(1);ack_code(0);expect(2);
    ack_code(0);expect(5);ack_code(0);
    assert(tx[10]==1 && tx[11]==1 && tx[12]==1);expect(6);
    ack_code(0);assert(AS608_get_event()==AS608_EVENT_SAVED);
    tick+=2000;AS608_proc();assert(AS608_busy());
    AS608_proc();expect(1);ack_code(0); // held finger cannot re-trigger
    tick+=150;AS608_proc();expect(1);ack_code(2);
    assert(!AS608_busy());

    assert(AS608_start_verify());parameters();expect(1);ack_code(0);expect(2);
    ack_code(0);assert(tx[13]==1 && tx[14]==44);expect(4);
    feed(match,5,0);AS608_proc();assert(AS608_get_event()==AS608_EVENT_MATCH);
    assert(AS608_get_id()==42);

    reset();assert(AS608_start_verify());parameters();expect(1);ack_code(0);
    expect(2);ack_code(0);expect(4);
    match[0]=9;feed(match,5,0);AS608_proc();match[0]=0;
    assert(AS608_get_event()==AS608_EVENT_NOT_MATCH);

    reset();assert(AS608_start_enroll());parameters();expect(0x1F);
    memset(full,255,33);full[0]=0;feed(full,33,0);AS608_proc();expect(0x1F);
    feed(full,33,0);AS608_proc();assert(AS608_get_event()==AS608_EVENT_FULL);
    assert(tx_count==0); // no StoreChar on a full library

    reset();assert(AS608_start_enroll());parameters();expect(0x1F);ack_code(0x1A);
    assert(AS608_get_event()==AS608_EVENT_ERROR);assert(tx_count==0);
    reset();assert(AS608_start_verify());expect(0x0F);
    feed(&short_ok,1,1);AS608_proc();assert(waiting); // bad checksum
    feed(&short_ok,1,2);AS608_proc();assert(waiting); // wrong address
    tick+=2000;AS608_proc();assert(AS608_get_error()==0xF0);
    reset();assert(AS608_start_verify());expect(0x0F);ack_code(0);
    assert(AS608_get_error()==0xF1); // success packet with wrong payload length
    reset();assert(AS608_start_verify());expect(0x0F);AS608_cancel();
    feed(&short_ok,1,0);AS608_proc();assert(AS608_get_event()==AS608_EVENT_NONE);
    assert(!AS608_start_verify());tick+=2000;AS608_proc();assert(!AS608_busy());

    // Key mode toggling preserves administrator errors.
    reset();admin_error_count=2;press(8);assert(mode_flag==3);
    press(4);assert(mode_flag==3);press(12);assert(mode_flag==3);
    press(8);assert(mode_flag==0 && admin_error_count==2);
    press(8);
    // Waiting or touching the sensor cannot authorize or start enrollment.
    voice_count=text_count=0;
    fingerprint_proc();tick+=3000;fingerprint_proc();
    assert(!admin_verified_flag && !finger_pending && !finger_operation);
    assert(tx_count==0 && voice_count==0 && text_count==0 && lock_flag==1);
    password_len=3;press(16);assert(password_len==3);
    fingerprint_proc();assert(!admin_verified_flag && tx_count==0);
    admin_error_count=0;password_len=6;memset(password_buf,'1',6);press(16);
    fingerprint_proc();assert(!admin_verified_flag && tx_count==0 && admin_error_count==1);
    password_len=6;memcpy(password_buf,admin_password,6);
    fingerprint_proc();assert(!admin_verified_flag && tx_count==0);
    press(16);
    assert(admin_verified_flag && finger_pending && admin_error_count==0);
    assert(played==VOICE_TRACK_FINGER_ADMIN);
    voice_count=text_count=0;
    fingerprint_proc();assert(finger_operation==2);expect(0x0F);
    // Missing UART response is a communication error, never a failed match.
    tick+=2000;fingerprint_proc();
    assert(AS608_get_error()==0xF0 && !finger_operation);
    assert(voice_count==0 && text_count==0 && lock_flag==1);
    press(8);assert(mode_flag==0 && !finger_operation);

    // Enrollment voice follows actual capture; no fingerprint LCD calls.
    reset();mode_flag=MODE_ADD_FINGER;admin_verified_flag=1;
    finger_operation=2;voice_count=text_count=0;
    enroll_to_char();event=AS608_EVENT_REMOVE;fingerprint_proc();
    assert(played==VOICE_TRACK_FINGER_ENROLL && voice_count==1 && text_count==0);
    tick+=150;AS608_proc();expect(1);ack_code(2);
    fingerprint_proc();assert(voice_count==1 && text_count==0);
    tick+=150;AS608_proc();expect(1);ack_code(0);expect(2);
    ack_code(0);expect(5);ack_code(0);expect(6);ack_code(0);
    fingerprint_proc();assert(played==VOICE_TRACK_FINGER_SAVED && mode_flag==MODE_UNLOCK);
    assert(voice_count==2 && text_count==0);

    reset();assert(AS608_start_verify());parameters();expect(1);ack_code(0);
    expect(2);ack_code(0);expect(4);match[0]=9;feed(match,5,0);match[0]=0;
    finger_operation=1;voice_count=0;fingerprint_proc();
    assert(played==VOICE_TRACK_FINGER_WRONG && voice_count==1 && lock_flag==1);

    // A successful response cannot open after mode change or lockout.
    reset();assert(AS608_start_verify());expect(0x0F);
    finger_operation=1;mode_flag=MODE_CHANGE_PASSWORD;
    fingerprint_proc();assert(lock_flag==1 && !finger_operation);
    reset();assert(AS608_start_verify());parameters();expect(1);ack_code(0);
    expect(2);ack_code(0);expect(4);feed(match,5,0);
    finger_operation=1;password_lock_flag=1;fingerprint_proc();assert(lock_flag==1);

    // A normal match opens and consumes the event exactly once.
    reset();assert(AS608_start_verify());parameters();expect(1);ack_code(0);
    expect(2);ack_code(0);expect(4);feed(match,5,0);finger_operation=1;
    fingerprint_proc();assert(lock_flag==0 && played==VOICE_TRACK_FINGER_CORRECT);
    lock_flag=1;fingerprint_proc();assert(lock_flag==1);
    tick+=2000;fingerprint_proc();fingerprint_proc();expect(1);
    // Release polling timeout must not authorize a new search on a held finger.
    tick+=2000;fingerprint_proc();assert(lock_flag==1 && require_release);
    tick+=2000;fingerprint_proc();fingerprint_proc();expect(1);
    ack_code(0);tick+=150;fingerprint_proc();expect(1);
    assert(lock_flag==1);ack_code(2);fingerprint_proc();expect(0x0F);

    // Normal unlock polls indefinitely with no finger, including PA1 stuck low.
    reset();voice_count=text_count=0;fingerprint_proc();parameters();expect(1);
    for(unsigned n=0;n<100;n++)
    {
        ack_code(2);tick+=150;fingerprint_proc();expect(1);
        assert(finger_operation==1 && voice_count==0 && lock_flag==1);
    }
    // Entering administrator mode cancels an in-flight polling operation.
    press(8);fingerprint_proc();tick+=2000;fingerprint_proc();
    assert(mode_flag==MODE_ADD_FINGER && !admin_verified_flag && tx_count==0);
    puts("PASS: enrollment, occupied slots, full library, match/mismatch, framing,");
    puts("timeouts, cancel, administrator keys, mode changes, lockout, single open");
    return 0;
}
'''

directory = root / "tests" / ".build_as608"
directory.mkdir(exist_ok=True)
source = directory / "test.c"
exe = directory / "test.exe"
try:
    source.write_text("\n".join([mock, voices, header, driver, key, tests]), encoding="utf-8")
    subprocess.run([gcc, "-std=c99", "-Wall", "-Wextra", "-Werror", str(source), "-o", str(exe)], check=True)
    subprocess.run([str(exe)], check=True)
finally:
    source.unlink(missing_ok=True)
    exe.unlink(missing_ok=True)
