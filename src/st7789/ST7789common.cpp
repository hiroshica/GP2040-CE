
#include "pico/stdlib.h"
#include "ST7789common.h"
#include "SDCommon.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
    /*-------------------------------------------------------------------------------------------------------*/
    /*-------------------------------------------------------------------------------------------------------*/

    /*-------------------------------------------------------------------------------------------------------*/
    // lcd configuration
    const struct st7789_config lcd_config = {
        .spi = PICO_DEFAULT_SPI_INSTANCE,
        .gpio_din = PICO_DEFAULT_SPI_TX_PIN,
        .gpio_clk = PICO_DEFAULT_SPI_SCK_PIN,
        .gpio_cs = PICO_DEFAULT_SPI_CSN_PIN,
        .gpio_dc = 20,
        .gpio_rst = 21,
        .gpio_bl = -1, // -1 = Un Used
        .rotmode = 0xc0,
        //.rotmode = 0x00, // default コネクタサイドが上、文字が正像
        //.rotmode = 0x10, // コネクタサイドが上、文字が正像
        //.rotmode = 0x20, // コネクタサイドが左、文字が裏像
        //.rotmode = 0x30, // コネクタサイドが左、文字が裏像
        //.rotmode = 0x40, // コネクタサイドが上、文字が裏像
        //.rotmode = 0x50, // コネクタサイドが上、文字が裏像
        //.rotmode = 0x60, // コネクタサイドが左、文字が正像
        //.rotmode = 0x70, // コネクタサイドが左、文字が正像
        //.rotmode = 0x80, // コネクタサイドが下、文字が裏像
        //.rotmode = 0x90, // コネクタサイドが下、文字が裏像
        //.rotmode = 0xa0, // コネクタサイドが右、文字が正像
        //.rotmode = 0xb0, // コネクタサイドが右、文字が正像
        //.rotmode = 0xc0, // コネクタサイドが下、文字が正像
        //.rotmode = 0xd0, // コネクタサイドが下、文字が正像
        //.rotmode = 0xe0, // コネクタサイドが右、文字が裏像
        //.rotmode = 0xf0, // コネクタサイドが右、文字が裏像
        /*
            0x00 = default

            7: MY Y Draw flip ???
            6: MX X Draw flip ???
            5: MV X <> Y Swap
            4: ML Y Draw update
            3: no use
            2: MH X Draw update
            1: no use
            0: no use
        */
    };
    /*-------------------------------------------------------------------------------------------------------*/
    SsLib::Ut::SsFontDraw *fontdraw;
    uint16_t line_vram[BUF_X * BUF_Y * BUF_SCALE];
    int32_t line_vram_size = sizeof(line_vram);
    char mesgbuf[lcd_width];
    /*-------------------------------------------------------------------------------------------------------*/
    void DrawMenu(char *mesg)
    {
        fontdraw->DrawString((const char *)mesg);
    }

    void CLSCall()
    {
        ST7789_CLS(0x0000);
    }
/*-------------------------------------------------------------------------------------------------------*/
#ifdef __cplusplus
}
#endif /* __cplusplus */
/*-------------------------------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------------------------------*/
void ST7789Start()
{
    // initialize the lcd
    uint16_t rgbdata = 0x0000;
    // uint16_t rgbdata = 0x001f;  // 青
    // uint16_t rgbdata = 0x03e0;  // 緑
    // uint16_t rgbdata = 0x7a00; // 赤
    st7789_init(&lcd_config, lcd_width, lcd_height);
    st7789_fill(rgbdata);
    // initialize the lcd
}
/*-------------------------------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------------------------------*/
bool m_DrawStart = false;
int32_t m_DrawY = 0;
void ST7789_blit_buffer(uint16_t *buffer, uint32_t bufferLen, int x, int y, int width, int height)
{
    uint16_t w = x + width - 1;
    uint16_t h = y + height - 1;
    st7789_set_window(x, y, w, h);
    st7789_write((uint8_t *)buffer, bufferLen);
}
void ST7789_CLS(uint16_t setcol)
{
    st7789_fill(setcol);
}
void ST7789_drawend(int32_t transy)
{
    if (m_DrawStart)
    {
        int32_t blocksize = transy * lcd_width;
        ST7789_blit_buffer(line_vram, blocksize * sizeof(uint16_t), 0, m_DrawY, lcd_width, transy);
        m_DrawStart = false;
    }
}
void ST7789_draw(uint16_t *dotdata, int32_t bitmapx, int32_t bitmapy, tSsColor *fgcolor, tSsColor *bgcolor, int32_t inx, int32_t iny, float scale, float width, float height)
{
    if (!m_DrawStart)
    {
        m_DrawStart = true;
        m_DrawY = iny;
        memset(line_vram, 0, line_vram_size);
    }

    uint16_t R = (((uint16_t)fgcolor->mR * 0x1f)) << 11;
    uint16_t G = (((uint16_t)fgcolor->mG * 0x2f)) << 5;
    uint16_t B = (((uint16_t)fgcolor->mB * 0x1f)) << 0;
    uint16_t fgcol = R | G | B;
    R = (((uint16_t)bgcolor->mR * 0x1f)) << 11;
    G = (((uint16_t)bgcolor->mG * 0x2f)) << 5;
    B = (((uint16_t)bgcolor->mB * 0x1f)) << 0;
    uint16_t bgcol = R | G | B;
    fgcol = HL_SWAP(fgcol);
    bgcol = HL_SWAP(bgcol);

    int drawxstart = 0;
    int drawystart = 0;
    int drawxend = 0;
    int drawyend = 0;
    int32_t offsetX = inx;
    uint16_t dcol = 0;

    for (int32_t index = 0; index < bitmapy; ++index)
    {
        float yscale = ((float)(index + 1) * scale);
        drawyend = (int32_t)yscale;
        for (; drawystart < drawyend; drawystart++)
        {
            int32_t offsetY = (drawystart * lcd_width); // Vram offset Y
            uint16_t getdot = dotdata[index];
            drawxstart = 0;

            for (int32_t iF = 0; iF < bitmapx; ++iF)
            {
                float xscale = ((float)(iF + 1) * scale);
                drawxend = (int32_t)xscale;
                if (getdot & 0x8000)
                {
                    dcol = fgcol;
                }
                else
                {
                    dcol = bgcol;
                }
                for (; drawxstart < drawxend; drawxstart++)
                {
                    line_vram[offsetY + offsetX + drawxstart] = dcol;
                }
                getdot <<= 1;
            }
        }
    }
}
