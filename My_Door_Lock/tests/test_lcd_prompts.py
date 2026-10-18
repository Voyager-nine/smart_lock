"""Check actual prompt glyph coverage, layout and restoration without hardware."""
from pathlib import Path
import re
import subprocess

root = Path(__file__).resolve().parents[1]
lcd = (root / 'Driver/lcd.c').read_bytes().decode('gbk')
font = (root / 'Driver/font_ascii_16x8.h').read_bytes().decode('gbk')
header = (root / 'Driver/lcd.h').read_bytes().decode('gbk')
glyphs = font[font.index('const typFNT_GB16 tfont16[]'):]
glyphs = glyphs[:glyphs.index('};')]
assert not re.search(r'(?m)^DB\s', font)
for values in re.findall(r'"((?:\\x[0-9A-F]{2})+)"', lcd):
    text = bytes(int(v, 16) for v in re.findall(r'\\x([0-9A-F]{2})', values)).decode('gbk')
    assert len(text) <= 8
    for char in text:
        assert '"' + char + '"' in glyphs, repr(char)
macros = '\n'.join(line for line in header.splitlines() if line.startswith('#define LCD_PROMPT_'))
mock = r'''
#include <stdint.h>
#include <assert.h>
typedef uint8_t u8;
#define LCD_W 128
#define LCD_H 128
#define WHITE 0
#define RED 1
#define BLUE 2
#define LGRAY 3
static uint32_t tick;
static uint8_t seconds, default_prompt=22, refresh_lcd, password_len;
static unsigned glyph_count, password_count, backgrounds;
static uint8_t gImage_lock_disp[8];
uint32_t GetTick(void) {return tick;}
uint8_t key_prompt(void) {return default_prompt;}
uint8_t password_lock_seconds(void) {return seconds;}
void LCD_Fill(int x,int y,int right,int bottom,int color)
{(void)x;(void)right;(void)color;assert((y==0 && (bottom==24 || bottom==128)) ||
 (y==30 && bottom==57) || (y==48 && bottom==80));}
void LCD_ShowChinese16x16(int x,int y,u8 *s,int fc,int bc,int size,int mode)
{assert(x>=0 && x+16<=128 && y==4 && y+size<30);assert(s[0]>=0x80);
 (void)fc;(void)bc;(void)mode;glyph_count++;}
void LCD_ShowIntNum(int x,int y,int num,int digits,int fc,int bc,int size)
{assert(x==(128-digits*16)/2 && y==48 && num==seconds && fc==RED && size==32);(void)bc;}
void LCD_ShowPicture(int x,int y,int w,int h,const u8 *p)
{(void)x;(void)y;(void)w;(void)h;(void)p;backgrounds++;}
void LCD_ShowChar(int x,int y,int c,int fc,int bc,int size,int mode)
{assert(x>=24 && y==36 && c=='*');(void)fc;(void)bc;(void)size;(void)mode;password_count++;}
'''
tests = r'''
int main(void) {
 lcd_task();assert(glyph_count==5);
 password_len=3;refresh_lcd=1;lcd_task();assert(glyph_count==5 && password_count==3);
 lcd_prompt(LCD_PROMPT_PASSWORD_WRONG);lcd_task();assert(glyph_count==9);
 tick=2000;lcd_task();assert(glyph_count==14);
 default_prompt=LCD_PROMPT_ADMIN_ENTER;lcd_task();assert(glyph_count==21);
 seconds=10;lcd_task();assert(glyph_count==26);seconds=9;lcd_task();assert(glyph_count==26);
 seconds=0;lcd_task();assert(backgrounds==1 && glyph_count==33);
 lcd_prompt(LCD_PROMPT_EXIT);lcd_task();assert(glyph_count==36);
 default_prompt=LCD_PROMPT_INPUT_PASSWORD;tick+=2000;lcd_task();assert(glyph_count==41);
 return 0;
}
'''
directory = root / 'tests/.build_as608'
directory.mkdir(exist_ok=True)
source, exe = directory / 'lcd_test.c', directory / 'lcd_test.exe'
try:
    source.write_text('\n'.join([mock, macros, lcd[lcd.index('static const char *const lcd_prompt_text[]'):], tests]), encoding='ascii')
    subprocess.run([r'D:\mingw64\bin\gcc.exe', '-std=c99', '-Wall', '-Wextra', '-Werror', str(source), '-o', str(exe)], check=True)
    subprocess.run([str(exe)], check=True)
finally:
    source.unlink(missing_ok=True)
    exe.unlink(missing_ok=True)
print('PASS: GBK glyph coverage, top layout, password isolation, prompt expiry and lockout restoration')
