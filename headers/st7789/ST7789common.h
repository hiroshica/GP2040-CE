#ifndef __ST7789COMMON_H__
#define __ST7789COMMON_H__

#include "common.h"
#include "st7789.h"

#include "font_draw/font_draw.h"
#include "Button.h"

/*-------------------------------------------------------------------------------------------------------*/
// #define lcd_width (320)
// #define lcd_height (240)
#define lcd_width (240)
#define lcd_height (320)

#define HL_SWAP(a) ((a >> 8) & 0xff) | ((a & 0xff) << 8)

#define BUF_X lcd_width
#define BUF_Y (16)
#define BUF_SCALE (4)

/*-------------------------------------------------------------------------------------------------------*/

void ST7789Start();
void ST7789_blit_buffer(uint16_t *buffer, uint32_t bufferLen, int x, int y, int width, int height);
void ST7789_CLS(uint16_t setcol);
void ST7789_drawend(int32_t scaley);
void ST7789_draw(uint16_t *dotdata, int32_t bitmapx, int32_t bitmapy, tSsColor *fgcolor, tSsColor *bgcolor, int32_t inx, int32_t iny, float scale, float width, float height);

/*-------------------------------------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
    /*-------------------------------------------------------------------------------------------------------*/
    /*-------------------------------------------------------------------------------------------------------*/
    extern void DrawMenu(char *mesg);
    extern void CLSCall();

    extern void DrawMenu(char *mesg);

    extern char mesgbuf[];
    extern SsLib::Ut::SsFontDraw *fontdraw;

    /*-------------------------------------------------------------------------------------------------------*/
#ifdef __cplusplus
}
#endif /* __cplusplus */
/*-------------------------------------------------------------------------------------------------------*/

#endif // __ST7789COMMON_H__
