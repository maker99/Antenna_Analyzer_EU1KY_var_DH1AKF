/*
 *   (c) Yury Kuchura
 *   kuchura@gmail.com
 *
 *   This code can be used on terms of WTFPL Version 2 (http://www.wtfpl.net/).
 */

//Panoramic window frequency and bandspan setting window

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "LCD.h"
#include "touch.h"
#include "font.h"
#include "num_keypad.h"
#include "textbox.h"
#include "config.h"
#include "panfreq.h"
#include "panvswr2.h"

extern void Sleep(uint32_t);
static void DigitHitCb(const TEXTBOX_t* tb);
static void LeftHitCb(void);// WK
static void RightHitCb(void);// WK
static void CancelHitCb(void);
static void OKHitCb(void);
static void BandHitCb(const TEXTBOX_t* tb);
static void M10HitCb(void);
static void M1HitCb(void);
static void M01HitCb(void);
static void M001HitCb(void);// WK
static void P10HitCb(void);
static void P1HitCb(void);
static void P01HitCb(void);
static void P001HitCb(void);// WK
static void BSPrevHitCb(void);
static void BSNextHitCb(void);

static uint32_t PanrqExit = 0;
static uint8_t rqDel = 0;
static uint8_t CurPos;
static uint32_t _f1;// in Hz
static BANDSPAN _bs;
static bool update_allowed = false;

#define NUMKEYH 38
#define NUMKEYW 54
#define NUMKEYX0 6
#define NUMKEY0 50
#define NUMKEYX(col) (NUMKEYX0 + col * NUMKEYW + 4 * col)
#define NUMKEYY(row) (NUMKEY0 + row * NUMKEYH + 4 * row)

#define BANDKEYH 36
#define BANDKEYW 54
#define BANDKEYX0 190
#define BANDKEY0 100
#define BANDKEYX(col) (BANDKEYX0 + col * BANDKEYW + 4 * col)
#define BANDKEYY(row) (BANDKEY0 + row * BANDKEYH + 4 * row)

static const TEXTBOX_t tb_pan[] = {
    (TEXTBOX_t){ .x0 = NUMKEYX(0), .y0 = NUMKEYY(0), .text = "1", .font = FONT_FRANBIG, .width = NUMKEYW, .height = NUMKEYH, .center = 1,
                 .border = 1, .fgcolor = LCD_WHITE, .bgcolor = LCD_BLACK, .cb = (void(*)(void))DigitHitCb, .cbparam = 1, .next = (void*)&tb_pan[1] },
    (TEXTBOX_t){ .x0 = NUMKEYX(1), .y0 = NUMKEYY(0), .text = "2", .font = FONT_FRANBIG, .width = NUMKEYW, .height = NUMKEYH, .center = 1,
                 .border = 1, .fgcolor = LCD_WHITE, .bgcolor = LCD_BLACK, .cb = (void(*)(void))DigitHitCb, .cbparam = 1, .next = (void*)&tb_pan[2] },
    (TEXTBOX_t){ .x0 = NUMKEYX(2), .y0 = NUMKEYY(0), .text = "3", .font = FONT_FRANBIG, .width = NUMKEYW, .height = NUMKEYH, .center = 1,
                 .border = 1, .fgcolor = LCD_WHITE, .bgcolor = LCD_BLACK, .cb = (void(*)(void))DigitHitCb, .cbparam = 1, .next = (void*)&tb_pan[3] },
    (TEXTBOX_t){ .x0 = NUMKEYX(0), .y0 = NUMKEYY(1), .text = "4", .font = FONT_FRANBIG, .width = NUMKEYW, .height = NUMKEYH, .center = 1,
                 .border = 1, .fgcolor = LCD_WHITE, .bgcolor = LCD_BLACK, .cb = (void(*)(void))DigitHitCb, .cbparam = 1, .next = (void*)&tb_pan[4] },
    (TEXTBOX_t){ .x0 = NUMKEYX(1), .y0 = NUMKEYY(1), .text = "5", .font = FONT_FRANBIG, .width = NUMKEYW, .height = NUMKEYH, .center = 1,
                 .border = 1, .fgcolor = LCD_WHITE, .bgcolor = LCD_BLACK, .cb = (void(*)(void))DigitHitCb, .cbparam = 1, .next = (void*)&tb_pan[5] },
    (TEXTBOX_t){ .x0 = NUMKEYX(2), .y0 = NUMKEYY(1), .text = "6", .font = FONT_FRANBIG, .width = NUMKEYW, .height = NUMKEYH, .center = 1,
                 .border = 1, .fgcolor = LCD_WHITE, .bgcolor = LCD_BLACK, .cb = (void(*)(void))DigitHitCb, .cbparam = 1, .next = (void*)&tb_pan[6] },
    (TEXTBOX_t){ .x0 = NUMKEYX(0), .y0 = NUMKEYY(2), .text = "7", .font = FONT_FRANBIG, .width = NUMKEYW, .height = NUMKEYH, .center = 1,
                 .border = 1, .fgcolor = LCD_WHITE, .bgcolor = LCD_BLACK, .cb = (void(*)(void))DigitHitCb, .cbparam = 1, .next = (void*)&tb_pan[7] },
    (TEXTBOX_t){ .x0 = NUMKEYX(1), .y0 = NUMKEYY(2), .text = "8", .font = FONT_FRANBIG, .width = NUMKEYW, .height = NUMKEYH, .center = 1,
                 .border = 1, .fgcolor = LCD_WHITE, .bgcolor = LCD_BLACK, .cb = (void(*)(void))DigitHitCb, .cbparam = 1, .next = (void*)&tb_pan[8] },
    (TEXTBOX_t){ .x0 = NUMKEYX(2), .y0 = NUMKEYY(2), .text = "9", .font = FONT_FRANBIG, .width = NUMKEYW, .height = NUMKEYH, .center = 1,
                 .border = 1, .fgcolor = LCD_WHITE, .bgcolor = LCD_BLACK, .cb = (void(*)(void))DigitHitCb, .cbparam = 1, .next = (void*)&tb_pan[9] },
    (TEXTBOX_t){ .x0 = NUMKEYX(1), .y0 = NUMKEYY(3), .text = "0", .font = FONT_FRANBIG, .width = NUMKEYW, .height = NUMKEYH, .center = 1,
                 .border = 1, .fgcolor = LCD_WHITE, .bgcolor = LCD_BLACK, .cb = (void(*)(void))DigitHitCb, .cbparam = 1, .next = (void*)&tb_pan[10] },
    (TEXTBOX_t){ .x0 = NUMKEYX(0), .y0 = NUMKEYY(3), .text = "<", .font = FONT_FRANBIG, .width = NUMKEYW, .height = NUMKEYH, .center = 1,
                 .border = 1, .fgcolor = LCD_WHITE, .bgcolor = LCD_BLACK, .cb = LeftHitCb, .cbparam = 0, .next = (void*)&tb_pan[11] },
    (TEXTBOX_t){ .x0 = NUMKEYX(2), .y0 = NUMKEYY(3), .text = ">", .font = FONT_FRANBIG, .width = NUMKEYW, .height = NUMKEYH, .center = 1,               // WK
                 .border = 1, .fgcolor = LCD_WHITE, .bgcolor = LCD_BLACK, .cb = RightHitCb, .cbparam = 0, .next = (void*)&tb_pan[12] },

    (TEXTBOX_t){ .x0 = BANDKEYX(0), .y0 = BANDKEYY(0), .text = "160", .font = FONT_FRANBIG, .width = BANDKEYW, .height = BANDKEYH, .center = 1,
                 .border = 1, .fgcolor = LCD_WHITE, .bgcolor = LCD_RGB(0, 0, 128), .cb = (void(*)(void))BandHitCb, .cbparam = 1, .next = (void*)&tb_pan[13] },
    (TEXTBOX_t){ .x0 = BANDKEYX(1), .y0 = BANDKEYY(0), .text = "80", .font = FONT_FRANBIG, .width = BANDKEYW, .height = BANDKEYH, .center = 1,
                 .border = 1, .fgcolor = LCD_WHITE, .bgcolor = LCD_RGB(0, 0, 128), .cb = (void(*)(void))BandHitCb, .cbparam = 1, .next = (void*)&tb_pan[14] },
    (TEXTBOX_t){ .x0 = BANDKEYX(2), .y0 = BANDKEYY(0), .text = "60", .font = FONT_FRANBIG, .width = BANDKEYW, .height = BANDKEYH, .center = 1,
                 .border = 1, .fgcolor = LCD_WHITE, .bgcolor = LCD_RGB(0, 0, 128), .cb = (void(*)(void))BandHitCb, .cbparam = 1, .next = (void*)&tb_pan[15] },
    (TEXTBOX_t){ .x0 = BANDKEYX(3), .y0 = BANDKEYY(0), .text = "40", .font = FONT_FRANBIG, .width = BANDKEYW, .height = BANDKEYH, .center = 1,
                 .border = 1, .fgcolor = LCD_WHITE, .bgcolor = LCD_RGB(0, 0, 128), .cb = (void(*)(void))BandHitCb, .cbparam = 1, .next = (void*)&tb_pan[16] },
    (TEXTBOX_t){ .x0 = BANDKEYX(4), .y0 = BANDKEYY(0), .text = "30", .font = FONT_FRANBIG, .width = BANDKEYW, .height = BANDKEYH, .center = 1,
                 .border = 1, .fgcolor = LCD_WHITE, .bgcolor = LCD_RGB(0, 0, 128), .cb = (void(*)(void))BandHitCb, .cbparam = 1, .next = (void*)&tb_pan[17] },

    (TEXTBOX_t){ .x0 = BANDKEYX(0), .y0 = BANDKEYY(1), .text = "20", .font = FONT_FRANBIG, .width = BANDKEYW, .height = BANDKEYH, .center = 1,
                 .border = 1, .fgcolor = LCD_WHITE, .bgcolor = LCD_RGB(0, 0, 128), .cb = (void(*)(void))BandHitCb, .cbparam = 1, .next = (void*)&tb_pan[18] },
    (TEXTBOX_t){ .x0 = BANDKEYX(1), .y0 = BANDKEYY(1), .text = "17", .font = FONT_FRANBIG, .width = BANDKEYW, .height = BANDKEYH, .center = 1,
                 .border = 1, .fgcolor = LCD_WHITE, .bgcolor = LCD_RGB(0, 0, 128), .cb = (void(*)(void))BandHitCb, .cbparam = 1, .next = (void*)&tb_pan[19] },
    (TEXTBOX_t){ .x0 = BANDKEYX(2), .y0 = BANDKEYY(1), .text = "15", .font = FONT_FRANBIG, .width = BANDKEYW, .height = BANDKEYH, .center = 1,
                 .border = 1, .fgcolor = LCD_WHITE, .bgcolor = LCD_RGB(0, 0, 128), .cb = (void(*)(void))BandHitCb, .cbparam = 1, .next = (void*)&tb_pan[20] },
    (TEXTBOX_t){ .x0 = BANDKEYX(3), .y0 = BANDKEYY(1), .text = "12", .font = FONT_FRANBIG, .width = BANDKEYW, .height = BANDKEYH, .center = 1,
                 .border = 1, .fgcolor = LCD_WHITE, .bgcolor = LCD_RGB(0, 0, 128), .cb = (void(*)(void))BandHitCb, .cbparam = 1, .next = (void*)&tb_pan[21] },
    (TEXTBOX_t){ .x0 = BANDKEYX(4), .y0 = BANDKEYY(1), .text = "10", .font = FONT_FRANBIG, .width = BANDKEYW, .height = BANDKEYH, .center = 1,
                 .border = 1, .fgcolor = LCD_WHITE, .bgcolor = LCD_RGB(0, 0, 128), .cb = (void(*)(void))BandHitCb, .cbparam = 1, .next = (void*)&tb_pan[22] },

    (TEXTBOX_t){ .x0 = BANDKEYX(0), .y0 = BANDKEYY(2), .text = "6", .font = FONT_FRANBIG, .width = BANDKEYW, .height = BANDKEYH, .center = 1,
                 .border = 1, .fgcolor = LCD_WHITE, .bgcolor = LCD_RGB(0, 0, 128), .cb = (void(*)(void))BandHitCb, .cbparam = 1, .next = (void*)&tb_pan[23] },
    (TEXTBOX_t){ .x0 = BANDKEYX(1), .y0 = BANDKEYY(2), .text = "4", .font = FONT_FRANBIG, .width = BANDKEYW, .height = BANDKEYH, .center = 1,
                 .border = 1, .fgcolor = LCD_WHITE, .bgcolor = LCD_RGB(0, 0, 128), .cb = (void(*)(void))BandHitCb, .cbparam = 1, .next = (void*)&tb_pan[24] },
    (TEXTBOX_t){ .x0 = BANDKEYX(2), .y0 = BANDKEYY(2), .text = "2", .font = FONT_FRANBIG, .width = BANDKEYW, .height = BANDKEYH, .center = 1,
                 .border = 1, .fgcolor = LCD_WHITE, .bgcolor = LCD_RGB(0, 0, 128), .cb = (void(*)(void))BandHitCb, .cbparam = 1, .next = (void*)&tb_pan[25] },
    (TEXTBOX_t){ .x0 = BANDKEYX(3), .y0 = BANDKEYY(2), .text = "1.25m", .font = FONT_FRAN, .width = BANDKEYW, .height = BANDKEYH, .center = 1,
                 .border = 1, .fgcolor = LCD_WHITE, .bgcolor = LCD_RGB(0, 0, 128), .cb = (void(*)(void))BandHitCb, .cbparam = 1, .next = (void*)&tb_pan[26] },
    (TEXTBOX_t){ .x0 = BANDKEYX(4), .y0 = BANDKEYY(2), .text = "70cm", .font = FONT_FRAN, .width = BANDKEYW, .height = BANDKEYH, .center = 1,
                 .border = 1, .fgcolor = LCD_WHITE, .bgcolor = LCD_RGB(0, 0, 100), .cb = (void(*)(void))BandHitCb, .cbparam = 1, .next = (void*)&tb_pan[27] },

    (TEXTBOX_t){ .x0 = 386, .y0 =2, .text = "OK", .font = FONT_FRANBIG, .border = 1, .center = 1, .width = 90, .height = 38,
                 .fgcolor = LCD_YELLOW, .bgcolor = LCD_RGB(0,128,0), .cb = OKHitCb, .next = (void*)&tb_pan[28] },
    (TEXTBOX_t){ .x0 = 6, .y0 = 2, .text = "Cancel", .font = FONT_FRANBIG, .border = 1, .center = 1, .width = 90, .height = 38,
                 .fgcolor = LCD_BLACK, .bgcolor = LCD_YELLOW, .cb = CancelHitCb, .next = (void*)&tb_pan[29] },

    (TEXTBOX_t){ .x0 = 5, .y0 = 234, .text = "-10 M", .font = FONT_FRAN, .width = 46, .height = 36, .center = 1,
                 .border = 1, .fgcolor = LCD_WHITE, .bgcolor = LCD_BLACK, .cb = (void(*)(void))M10HitCb, .cbparam = 1, .next = (void*)&tb_pan[30] },
    (TEXTBOX_t){ .x0 = 55, .y0 = 234, .text = "-1 M", .font = FONT_FRAN, .width = 46, .height = 36, .center = 1,
                 .border = 1, .fgcolor = LCD_WHITE, .bgcolor = LCD_BLACK, .cb = (void(*)(void))M1HitCb, .cbparam = 1, .next = (void*)&tb_pan[31] },
    (TEXTBOX_t){ .x0 = 105, .y0 = 234, .text = "-100 k", .font = FONT_FRAN, .width = 46, .height = 36, .center = 1,
                 .border = 1, .fgcolor = LCD_WHITE, .bgcolor = LCD_BLACK, .cb = (void(*)(void))M01HitCb, .cbparam = 1, .next = (void*)&tb_pan[32] },
(TEXTBOX_t){ .x0 = 155, .y0 = 234, .text = "-10 k", .font = FONT_FRAN, .width = 46, .height = 36, .center = 1,
                 .border = 1, .fgcolor = LCD_WHITE, .bgcolor = LCD_BLACK, .cb = (void(*)(void))M001HitCb, .cbparam = 1, .next = (void*)&tb_pan[33] },// WK
(TEXTBOX_t){ .x0 = 270, .y0 = 234, .text = "+10 k", .font = FONT_FRAN, .width = 46, .height = 36, .center = 1,
                 .border = 1, .fgcolor = LCD_WHITE, .bgcolor = LCD_BLACK, .cb = (void(*)(void))P001HitCb, .cbparam = 1, .next = (void*)&tb_pan[34] },

    (TEXTBOX_t){ .x0 = 320, .y0 = 234, .text = "+100 k", .font = FONT_FRAN, .width = 46, .height = 36, .center = 1,
                 .border = 1, .fgcolor = LCD_WHITE, .bgcolor = LCD_BLACK, .cb = (void(*)(void))P01HitCb, .next = (void*)&tb_pan[35] },
    (TEXTBOX_t){ .x0 = 370, .y0 = 234, .text = "+1 M", .font = FONT_FRAN, .width = 46, .height = 36, .center = 1,
                 .border = 1, .fgcolor = LCD_WHITE, .bgcolor = LCD_BLACK, .cb = (void(*)(void))P1HitCb, .next = (void*)&tb_pan[36] },
    (TEXTBOX_t){ .x0 = 420, .y0 = 234, .text = "+10 M", .font = FONT_FRAN, .width = 46, .height = 36, .center = 1,
                 .border = 1, .fgcolor = LCD_WHITE, .bgcolor = LCD_BLACK, .cb = (void(*)(void))P10HitCb, .next = (void*)&tb_pan[37]},

    (TEXTBOX_t){ .x0 = 190, .y0 = 50, .text = "<<", .font = FONT_FRANBIG, .width = 50, .height = 36, .center = 1,
                 .border = 1, .fgcolor = LCD_WHITE, .bgcolor = LCD_RGB(0, 0, 128), .cb = (void(*)(void))BSPrevHitCb, .next = (void*)&tb_pan[38] },
    (TEXTBOX_t){ .x0 = 424, .y0 = 50, .text = ">>", .font = FONT_FRANBIG, .width = 50, .height = 36, .center = 1,
                 .border = 1, .fgcolor = LCD_WHITE, .bgcolor = LCD_RGB(0, 0, 128), .cb = (void(*)(void))BSNextHitCb },
};

static bool IsValidRange(void)
{
    uint32_t fstart, fend;
    if (CFG_GetParam(CFG_PARAM_PAN_CENTER_F))
    {
        fstart = _f1 - 500*BSVALUES[_bs];
        fend = _f1 + 500*BSVALUES[_bs];
    }
    else
    {
        fstart = _f1;
        fend = _f1 + 500*BSVALUES[_bs];
    }
    return ((fstart < fend) && (fstart >= BAND_FMIN) && (fend <= CFG_GetParam(CFG_PARAM_BAND_FMAX)));
}

static void Show_F(void)
{
    uint32_t color = IsValidRange() ? LCD_WHITE : LCD_RED;
    uint32_t dp = (_f1 % 1000000)/1000 ;// WK
    uint32_t mhz = _f1 / 1000000;
    uint16_t x;

    LCD_FillRect(LCD_MakePoint(160, 2), LCD_MakePoint(319, 43), LCD_BLACK);
    LCD_FillRect(LCD_MakePoint(250, 53), LCD_MakePoint(410, FONT_GetHeight(FONT_FRANBIG) + 53), LCD_BLACK);
    //if(CurPos>5) CurPos=5;
    if (CurPos>6) CurPos=6;
    x=160+(6-CurPos)*17;
    if(CurPos<3) x+=6;
    LCD_FillRect((LCDPoint){x,34},(LCDPoint){x+16,38}, LCD_WHITE);// Set the cursor

    FONT_Print(FONT_FRANBIG, color, LCD_BLACK, 160, 2, "%04u.%03u MHz", mhz, dp);
    if (CFG_GetParam(CFG_PARAM_PAN_CENTER_F))
    {
        FONT_Print(FONT_FRANBIG, color, LCD_RGB(0, 0, 128), 260, 53, " +/- %s ", BSSTR_HALF[_bs]);
    }
    else
    {
        FONT_Print(FONT_FRANBIG, color, LCD_RGB(0, 0, 128), 260, 53, " +%s ", BSSTR[_bs]);
    }
}

static void BSPrevHitCb(void)
{
    if (_bs == BS2)// ** WK **
    {
        _bs = BS500M;// DL8MBY
        if (!IsValidRange())
            _bs = BS200;// DL8MBY
    }
    else
        _bs -= 1;
    Show_F();
}

static void BSNextHitCb(void)
{
    if (_bs == BS500M)//DL8MBY
        _bs = BS2;// ** WK **
    else
    {
        _bs += 1;
        if (!IsValidRange())
            _bs -= 1;
    }
    Show_F();
}

static void F0_Decr(uint32_t khz)
{
int32_t Save_f1=_f1;
    if (CFG_GetParam(CFG_PARAM_PAN_CENTER_F))
    {
        if (_f1 >= (BAND_FMIN  + 1000*khz + 500*BSVALUES[_bs]))
            _f1 -= 1000*khz;
    }
    else
    {
        if (_f1 >= (BAND_FMIN  + 1000*khz))
            _f1 -= 1000*khz;
    }
    if (!IsValidRange()){
        _f1=Save_f1;// no reaction, if out of range
    }
    Show_F();
}

static void F0_Incr(uint32_t khz)
{

int32_t Save_f1=_f1;
    if (CFG_GetParam(CFG_PARAM_PAN_CENTER_F))
    {
        if (_f1 <= (CFG_GetParam(CFG_PARAM_BAND_FMAX) - 1000*khz - 500*BSVALUES[_bs]))
            _f1 += 1000*khz;
    }
    else
    {
        if (_f1 <= (CFG_GetParam(CFG_PARAM_BAND_FMAX)  - 1000*khz))
            _f1 += 1000*khz;
    }
    if (!IsValidRange()){
        _f1=Save_f1;// no reaction, if out of range
    }
    Show_F();
}

static void M10HitCb(void)
{
    F0_Decr(10000);
}

static void M1HitCb(void)
{
    F0_Decr(1000);
}

static void M01HitCb(void)
{
    F0_Decr(100);
}

static void M001HitCb(void)// WK
{
    F0_Decr(10);
}
static void P10HitCb(void)
{
    F0_Incr(10000);
}

static void P1HitCb(void)
{
    F0_Incr(1000);
}

static void P01HitCb(void)
{
    F0_Incr(100);
}

static void P001HitCb(void)// WK
{
    F0_Incr(10);
}

static void OKHitCb(void)
{
    PanrqExit = 1;
    if (IsValidRange())
        update_allowed = true;
}

static void CancelHitCb(void)
{
    rqDel = 1;// WK
    PanrqExit = 1;
}

static void LeftHitCb(void) // WK
{
    //if(CurPos<5)
    if(CurPos<6)
        CurPos++;
   Show_F();
}
static void RightHitCb(void) // WK
{
   if(CurPos>0)
        CurPos--;
   Show_F();
}
static uint32_t digit;
static uint32_t rest;

static void DigitHitCb(const TEXTBOX_t* tb)
{
uint32_t k;
uint8_t i;
int fkhz=_f1/1000;
uint32_t Save_f1=_f1;
    if (_f1 <= CFG_GetParam(CFG_PARAM_BAND_FMAX) )
    {
        digit = tb->text[0] - '0';
        k=1;
        for(i=0; i<CurPos; i++){
            k=10*k;
        }
        rest = fkhz%(k);
        fkhz = ((fkhz/(10*k))*10 +digit)*k+rest;
        _f1=fkhz*1000;
        if (IsValidRange()){
            if(CurPos>0) CurPos--;
        }
        else{
            _f1=Save_f1;// no reaction, if out of range
        }
    }
    Show_F();
}

static void BandHitCb(const TEXTBOX_t* tb)
{
    if (0 == strcmp(tb->text, "160"))
    {
        _f1 = 1800;
        _bs = BS200;
    }
    else if (0 == strcmp(tb->text, "80"))
    {
        _f1 = 3500;
        _bs = BS400;
    }
    else if (0 == strcmp(tb->text, "60"))
    {
        _f1 = 5200;
        _bs = BS400;
    }
    else if (0 == strcmp(tb->text, "40"))
    {
        _f1 = 7000;
        _bs = BS400;
    }
    else if (0 == strcmp(tb->text, "30"))
    {
        _f1 = 10100;
        _bs = BS100;
    }
    else if (0 == strcmp(tb->text, "20"))
    {
        _f1 = 14000;
        _bs = BS400;
    }
    else if (0 == strcmp(tb->text, "17"))
    {
        _f1 = 18000;
        _bs = BS200;
    }
    else if (0 == strcmp(tb->text, "15"))
    {
        _f1 = 21000;
        _bs = BS400;
    }
    else if (0 == strcmp(tb->text, "12"))
    {
        _f1 = 24890;
        _bs = BS400;
    }
    else if (0 == strcmp(tb->text, "10"))
    {
        _f1 = 28000;
        _bs = BS2M;
    }
    else if (0 == strcmp(tb->text, "6"))
    {
        _f1 = 50000;
        _bs = BS2M;
    }
    else if (0 == strcmp(tb->text, "4"))
    {
        _f1 = 70000;
        _bs = BS2M;
    }
    else if (0 == strcmp(tb->text, "2"))
    {
        _f1 = 144000;
        _bs = BS2M;
    }
    else if (0 == strcmp(tb->text, "1.25m"))// && (CFG_GetParam(CFG_PARAM_BAND_FMAX) >= 234000000ul))
    {// 222-225 MHz in USA
        _f1 = 214000;
        _bs = BS20M;
    }
    else if (0 == strcmp(tb->text, "70cm"))// && (CFG_GetParam(CFG_PARAM_BAND_FMAX) >= 445000000ul))
    {
        _f1 = 420000;
        _bs = BS20M;
    }
    else
        return;
    _f1*=1000;
    if (CFG_GetParam(CFG_PARAM_PAN_CENTER_F))
        _f1 += 500*BSVALUES[_bs];
    Show_F();
}

bool PanFreqWindow(uint32_t *pFkhz, BANDSPAN *pBs)
{
    LCD_Push(); //Store current LCD bitmap
    LCD_FillAll(LCD_BLACK);
    PanrqExit = 0;
    rqDel = 0;// WK
    CurPos=3;
    _f1 = (*pFkhz)*1000;
    _bs = *pBs;
    update_allowed = false;
    bool isUpdated = false;

    TEXTBOX_CTX_t fctx;
    TEXTBOX_InitContext(&fctx);

    TEXTBOX_Append(&fctx, (TEXTBOX_t*)tb_pan); //Append the very first element of the list in the flash.
                                                      //It is enough since all the .next fields are filled at compile time.
    TEXTBOX_DrawContext(&fctx);

    Show_F();
    while(TOUCH_IsPressed());

    for(;;)
    {
        if (TEXTBOX_HitTest(&fctx))
        {
            Sleep(50);
        }
        if(rqDel == 1){
            LCD_Pop();
            *pFkhz = 0;
            return false;
        }
        if (PanrqExit)
        {
            if (update_allowed && ((_f1 != (*pFkhz)*1000) || (_bs != *pBs)))
            {
                *pFkhz = _f1/1000;
                *pBs = _bs;
                CFG_SetParam(CFG_PARAM_PAN_F1, _f1);
                CFG_SetParam(CFG_PARAM_PAN_SPAN, _bs);
                CFG_Flush();
                isUpdated = true;
            }
            break;
        }
        Sleep(10);
    }
    Sleep(200);// WK
    //while(TOUCH_IsPressed())
    //    Sleep(0);
    LCD_Pop(); //Restore last saved LCD bitmap

    return isUpdated;
}
