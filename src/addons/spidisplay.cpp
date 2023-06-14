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

#define LGFX_USE_V1
#include <LovyanGFX.hpp>

#define TFT_SCLK PICO_DEFAULT_SPI_SCK_PIN
#define TFT_MOSI PICO_DEFAULT_SPI_TX_PIN
#define TFT_MISO PICO_DEFAULT_SPI_RX_PIN
#define TFT_CS PICO_DEFAULT_SPI_CSN_PIN
#define TFT_DC (20)
#define TFT_RST (21)

namespace lgfx
{
    struct Panel_ADA5206 : public Panel_ST7789
    {
        // 変え点がおかしいのと位置がおかしいのを補正したい
        virtual uint8_t getMadCtl(uint8_t r) const
        {
            // clang-format off
      static constexpr uint8_t madctl_table[] =
      {
        0x00, //0°   左回り
        0xa0, //90°
        0xc0, //180°
        0x60, //270°
        0x40, //0°   文字反転
        0x20, //90°
        0x80, //180°
        0xe0, //270°
      };
            // clang-format on
            return madctl_table[r];
        }
    };
};

class LGFX : public lgfx::LGFX_Device
{
    lgfx::Panel_ADA5206 _panel_instance;
    lgfx::Bus_SPI _bus_instance;

public:
    LGFX(void)
    {
        {                                      // バス制御の設定を行います。
            auto cfg = _bus_instance.config(); // バス設定用の構造体を取得します。

            cfg.spi_host = 0;          // 使用するSPIを選択
            cfg.spi_mode = 0;          // SPI通信モードを設定 (0 ~ 3)
            cfg.freq_write = 80000000; // 送信時のSPIクロック (最大80MHz, 80MHzを整数で割った値に丸められます)
            cfg.freq_read = 80000000;  // 受信時のSPIクロック
            cfg.pin_sclk = TFT_SCLK;   // SPIのSCLKピン番号を設定
            cfg.pin_mosi = TFT_MOSI;   // SPIのMOSIピン番号を設定
            cfg.pin_miso = TFT_MISO;   // SPIのMISOピン番号を設定 (-1 = disable)
            cfg.pin_dc = TFT_DC;       // SPIのD/Cピン番号を設定  (-1 = disable)

            _bus_instance.config(cfg);              // 設定値をバスに反映します。
            _panel_instance.setBus(&_bus_instance); // バスをパネルにセットします。
        }
        {                                        // 表示パネル制御の設定を行います。
            auto cfg = _panel_instance.config(); // 表示パネル設定用の構造体を取得します。
            cfg.pin_cs = TFT_CS;                 // CSが接続されているピン番号   (-1 = disable)
            cfg.pin_rst = TFT_RST;               // RSTが接続されているピン番号  (-1 = disable)
            cfg.pin_busy = -1;                   // BUSYが接続されているピン番号 (-1 = disable)

            cfg.panel_width = 240;  // 実際に表示可能な幅
            cfg.panel_height = 300; // 実際に表示可能な高さ
            cfg.bus_shared = true;  // SDとバスを共有する場合trueに設定
            cfg.offset_x = 0;       // パネルのX方向オフセット量
            cfg.offset_y = 0;       // パネルのY方向オフセット量

            cfg.offset_x = 0; // パネルのX方向オフセット量
            cfg.offset_y = 0; // パネルのY方向オフセット量
            cfg.offset_rotation = 2;
            cfg.invert = true; // パネルの明暗が反転してしまう場合 trueに設定

            _panel_instance.config(cfg);
        }
        setPanel(&_panel_instance); // 使用するパネルをセットします。
    }
};
static LGFX lcd;

//-----------------------------------------------------------------------------------------------
bool SPIDisplayAddon::available()
{
    return true;
}
//-----------------------------------------------------------------------------------------------
void SPIDisplayAddon::setup()
{
    lcd.init();

#if 0
    // 背景にグラデーションを描画する
    lcd.startWrite();
    lcd.setAddrWindow(0, 0, lcd.width(), lcd.height());
    for (int y = 0; y < lcd.height(); ++y)
    {
        for (int x = 0; x < lcd.width(); ++x)
        {
            lcd.writeColor(lcd.color888(x >> 1, (x + y) >> 2, y >> 1), 1);
        }
    }
    lcd.endWrite();
#endif
    lcd.startWrite();
    lcd.setAddrWindow(0, 0, lcd.width(), lcd.height());
    lcd.endWrite();
}
//-----------------------------------------------------------------------------------------------
void SPIDisplayAddon::preprocess()
{
}
//-----------------------------------------------------------------------------------------------
static int32_t testcount = 0;
void SPIDisplayAddon::process()
{
    lcd.startWrite();

    //lcd.fillScreen(TFT_BLACK);
    lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    lcd.setTextSize(1.0f, 1.0f);
    lcd.setFont(&lgfx::fonts::lgfxJapanGothic_16);

    lcd.setCursor(16, 16);
    lcd.print("Hello\n");
    lcd.print("こんにちは\n");
    lcd.print("Hello\n");
    lcd.print("こんにちは\n");

    lcd.endWrite();
}
