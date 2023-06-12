#include "spidisplay.h"
#include "GamepadState.h"
#include "enums.h"
#include "helper.h"
#include "storagemanager.h"
#include "pico/stdlib.h"
#include "bitmaps.h"
#include "ps4_driver.h"
#include "helper.h"
#include "config.pb.h"

// #define lcd_width (320)
// #define lcd_height (240)
#define lcd_width (240)
#define lcd_height (320)

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

//-----------------------------------------------------------------------------------------------
bool SPIDisplayAddon::available()
{
    return true;
}
//-----------------------------------------------------------------------------------------------
void SPIDisplayAddon::setup()
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
//-----------------------------------------------------------------------------------------------
void SPIDisplayAddon::preprocess()
{
}
//-----------------------------------------------------------------------------------------------
void SPIDisplayAddon::process()
{
}
