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

    lcd.startWrite();
    lcd.setAddrWindow(0, 0, lcd.width(), lcd.height());
    lcd.endWrite();

    clearScreen(1);

    gamepad = Storage::getInstance().GetGamepad();
    pGamepad = Storage::getInstance().GetProcessedGamepad();
    prevButtonState = 0;
    configMode = Storage::getInstance().GetConfigMode();
}
//-----------------------------------------------------------------------------------------------
void SPIDisplayAddon::preprocess()
{
}
//-----------------------------------------------------------------------------------------------
const DisplayOptions &SPIDisplayAddon::getDisplayOptions()
{
    bool configMode = Storage::getInstance().GetConfigMode();
    return configMode ? Storage::getInstance().getPreviewDisplayOptions() : Storage::getInstance().getDisplayOptions();
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
#define OFFSETX (8)
#define OFFSETY (24)
int color_table[] = {
    TFT_BLACK,
    TFT_WHITE,
};
//-----------------------------------------------------------------------------------------------
void SPIDisplayAddon::drawText(int x, int y, std::string text)
{
    lcd.startWrite();
    lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    lcd.setTextSize(1.6f, 1.6f);
    lcd.setCursor((x * 8 * lcd.getTextSizeX()) + OFFSETX, (y * 16 * lcd.getTextSizeX()) + OFFSETY);
    lcd.print(text.c_str());
    lcd.endWrite();
}
//-----------------------------------------------------------------------------------------------
void SPIDisplayAddon::drawPreciseEllipse(int x, int y, int32_t iRadiusX, int32_t iRadiusY, uint8_t ucColor, uint8_t bFilled)
{
    lcd.startWrite();
    lcd.setColor(color_table[ucColor]);

    if (bFilled)
    {
        lcd.fillEllipse((x + OFFSETX), (y + OFFSETY), iRadiusX, iRadiusY);
    }
    else
    {
        lcd.drawEllipse((x + OFFSETX), (y + OFFSETY), iRadiusX, iRadiusY);
    }

    lcd.endWrite();
}
//-----------------------------------------------------------------------------------------------
void SPIDisplayAddon::drawRectangle(int x1, int y1, int x2, int y2, uint8_t ucColor, uint8_t bFilled)
{
    int w = x2 - x1;
    int h = y2 - y1;
    lcd.startWrite();
    lcd.setColor(color_table[ucColor]);

    if (bFilled)
    {
        lcd.fillRect((x1 + OFFSETX), (y1 + OFFSETY), w, h);
    }
    else
    {
        lcd.drawRect((x1 + OFFSETX), (y1 + OFFSETY), w, h);
    }

    lcd.endWrite();
}
//-----------------------------------------------------------------------------------------------
void SPIDisplayAddon::drawSprite(uint8_t *pSprite, int cx, int cy, int iPitch, int x, int y, uint8_t iPriority)
{
    lcd.startWrite();

    lcd.endWrite();
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
void SPIDisplayAddon::process()
{
#if 0
    lcd.startWrite();

    // lcd.fillScreen(TFT_BLACK);
    lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    lcd.setTextSize(1.0f, 1.0f);
    lcd.setFont(&lgfx::fonts::lgfxJapanGothic_16);

    lcd.setCursor(16, 16);
    lcd.print("Hello\n");
    lcd.print("こんにちは\n");
    lcd.print("Hello\n");
    lcd.print("こんにちは\n");

    lcd.endWrite();
#endif
    if (!configMode && isDisplayPowerOff())
        return;

    DisplayMode nowMode = getDisplayMode();

    lcd.startWrite();
    if (clearClearDisplayMode != nowMode)
    {
        clearClearDisplayMode = nowMode;
        clearScreen(0);
    }

    switch (nowMode)
    {
    case SPIDisplayAddon::DisplayMode::CONFIG_INSTRUCTION:
        drawStatusBar(gamepad);
        drawText(0, 2, "[Web Config Mode]");
        drawText(0, 3, std::string("GP2040-CE : ") + std::string(GP2040VERSION));
        drawText(0, 4, std::string("[http://192.168.") + std::to_string(IPADDR_3) + std::string(".1]"));
        // drawText(0, 5, "Preview:");
        // drawText(5, 6, "B1 > Button");
        // drawText(5, 7, "B2 > Splash");
        break;
    case SPIDisplayAddon::DisplayMode::SPLASH:
        /*
        if (getDisplayOptions().splashMode == static_cast<SplashMode>(SPLASH_MODE_NONE)) {
            drawText(0, 4, " Splash NOT enabled.");
            break;
        }
        drawSplashScreen(getDisplayOptions().splashMode, (uint8_t*) Storage::getInstance().getDisplayOptions().splashImage.bytes, 90);
        */
        break;
    case SPIDisplayAddon::DisplayMode::BUTTONS:
        lcd.startWrite();
        lcd.fillRect(0, 0, lcd.width(), (lcd.height() / 3), TFT_BLACK);
        lcd.endWrite();
        drawStatusBar(gamepad);
        const DisplayOptions &options = getDisplayOptions();
        ButtonLayoutCustomOptions buttonLayoutCustomOptions = options.buttonLayoutCustomOptions;

        switch (options.buttonLayout)
        {
        case BUTTON_LAYOUT_STICK:
            drawArcadeStick(8, 28, 8, 2);
            break;
        case BUTTON_LAYOUT_STICKLESS:
            drawStickless(8, 20, 8, 2);
            break;
        case BUTTON_LAYOUT_BUTTONS_ANGLED:
            drawWasdBox(8, 28, 7, 3);
            break;
        case BUTTON_LAYOUT_BUTTONS_BASIC:
            drawUDLR(8, 28, 8, 2);
            break;
        case BUTTON_LAYOUT_KEYBOARD_ANGLED:
            drawKeyboardAngled(18, 28, 5, 2);
            break;
        case BUTTON_LAYOUT_KEYBOARDA:
            drawMAMEA(8, 28, 10, 1);
            break;
        case BUTTON_LAYOUT_DANCEPADA:
            drawDancepadA(39, 12, 15, 2);
            break;
        case BUTTON_LAYOUT_TWINSTICKA:
            drawTwinStickA(8, 28, 8, 2);
            break;
        case BUTTON_LAYOUT_BLANKA:
            drawBlankA(0, 0, 0, 0);
            break;
        case BUTTON_LAYOUT_VLXA:
            drawVLXA(7, 28, 7, 2);
            break;
        case BUTTON_LAYOUT_CUSTOMA:
            drawButtonLayoutLeft(buttonLayoutCustomOptions.paramsLeft);
            break;
        case BUTTON_LAYOUT_FIGHTBOARD_STICK:
            drawArcadeStick(18, 22, 8, 2);
            break;
        case BUTTON_LAYOUT_FIGHTBOARD_MIRRORED:
            drawFightboardMirrored(0, 22, 7, 2);
            break;
        }

        switch (options.buttonLayoutRight)
        {
        case BUTTON_LAYOUT_ARCADE:
            drawArcadeButtons(8, 28, 8, 2);
            break;
        case BUTTON_LAYOUT_STICKLESSB:
            drawSticklessButtons(8, 20, 8, 2);
            break;
        case BUTTON_LAYOUT_BUTTONS_ANGLEDB:
            drawWasdButtons(8, 28, 7, 3);
            break;
        case BUTTON_LAYOUT_VEWLIX:
            drawVewlix(8, 28, 8, 2);
            break;
        case BUTTON_LAYOUT_VEWLIX7:
            drawVewlix7(8, 28, 8, 2);
            break;
        case BUTTON_LAYOUT_CAPCOM:
            drawCapcom(6, 28, 8, 2);
            break;
        case BUTTON_LAYOUT_CAPCOM6:
            drawCapcom6(16, 28, 8, 2);
            break;
        case BUTTON_LAYOUT_SEGA2P:
            drawSega2p(8, 28, 8, 2);
            break;
        case BUTTON_LAYOUT_NOIR8:
            drawNoir8(8, 28, 8, 2);
            break;
        case BUTTON_LAYOUT_KEYBOARDB:
            drawMAMEB(68, 28, 10, 1);
            break;
        case BUTTON_LAYOUT_DANCEPADB:
            drawDancepadB(39, 12, 15, 2);
            break;
        case BUTTON_LAYOUT_TWINSTICKB:
            drawTwinStickB(100, 28, 8, 2);
            break;
        case BUTTON_LAYOUT_BLANKB:
            drawSticklessButtons(0, 0, 0, 0);
            break;
        case BUTTON_LAYOUT_VLXB:
            drawVLXB(6, 28, 7, 2);
            break;
        case BUTTON_LAYOUT_CUSTOMB:
            drawButtonLayoutRight(buttonLayoutCustomOptions.paramsRight);
            break;
        case BUTTON_LAYOUT_FIGHTBOARD:
            drawFightboard(8, 22, 7, 3);
            break;
        case BUTTON_LAYOUT_FIGHTBOARD_STICK_MIRRORED:
            drawArcadeStick(90, 22, 8, 2);
            break;
        }
        break;
    }
}
//-----------------------------------------------------------------------------------------------
void SPIDisplayAddon::clearScreen(int render)
{
    lcd.startWrite();
    lcd.fillScreen(TFT_BLACK);
    lcd.endWrite();
}
//-----------------------------------------------------------------------------------------------
bool SPIDisplayAddon::isDisplayPowerOff()
{
    return false;
}
//-----------------------------------------------------------------------------------------------
void SPIDisplayAddon::setDisplayPower(uint8_t status)
{
}
//-----------------------------------------------------------------------------------------------
SPIDisplayAddon::DisplayMode SPIDisplayAddon::getDisplayMode()
{
    if (configMode)
    {
        gamepad->read();
        uint16_t buttonState = gamepad->state.buttons;
        if (prevButtonState && !buttonState)
        { // has button been pressed (held and released)?
            switch (prevButtonState)
            {
            case (GAMEPAD_MASK_B1):
                prevDisplayMode =
                    prevDisplayMode == SPIDisplayAddon::DisplayMode::BUTTONS ? SPIDisplayAddon::DisplayMode::CONFIG_INSTRUCTION : SPIDisplayAddon::DisplayMode::BUTTONS;
                break;
            case (GAMEPAD_MASK_B2):
                prevDisplayMode =
                    prevDisplayMode == SPIDisplayAddon::DisplayMode::SPLASH ? SPIDisplayAddon::DisplayMode::CONFIG_INSTRUCTION : SPIDisplayAddon::DisplayMode::SPLASH;
                break;
            default:
                prevDisplayMode = SPIDisplayAddon::DisplayMode::CONFIG_INSTRUCTION;
            }
        }
        prevButtonState = buttonState;
        return prevDisplayMode;
    }
    else
    {
        if (Storage::getInstance().getDisplayOptions().splashMode != static_cast<SplashMode>(SPLASH_MODE_NONE))
        {
            int splashDuration = getDisplayOptions().splashDuration;
            if (splashDuration == 0 || getMillis() < splashDuration)
            {
                return SPIDisplayAddon::DisplayMode::SPLASH;
            }
        }
    }

    return SPIDisplayAddon::DisplayMode::BUTTONS;
}
//-----------------------------------------------------------------------------------------------
void SPIDisplayAddon::drawButtonLayoutLeft(ButtonLayoutParamsLeft &options)
{
    int32_t &startX = options.common.startX;
    int32_t &startY = options.common.startY;
    int32_t &buttonRadius = options.common.buttonRadius;
    int32_t &buttonPadding = options.common.buttonPadding;

    switch (options.layout)
    {
    case BUTTON_LAYOUT_STICK:
        drawArcadeStick(startX, startY, buttonRadius, buttonPadding);
        break;
    case BUTTON_LAYOUT_STICKLESS:
        drawStickless(startX, startY, buttonRadius, buttonPadding);
        break;
    case BUTTON_LAYOUT_BUTTONS_ANGLED:
        drawWasdBox(startX, startY, buttonRadius, buttonPadding);
        break;
    case BUTTON_LAYOUT_BUTTONS_BASIC:
        drawUDLR(startX, startY, buttonRadius, buttonPadding);
        break;
    case BUTTON_LAYOUT_KEYBOARD_ANGLED:
        drawKeyboardAngled(startX, startY, buttonRadius, buttonPadding);
        break;
    case BUTTON_LAYOUT_KEYBOARDA:
        drawMAMEA(startX, startY, buttonRadius, buttonPadding);
        break;
    case BUTTON_LAYOUT_DANCEPADA:
        drawDancepadA(startX, startY, buttonRadius, buttonPadding);
        break;
    case BUTTON_LAYOUT_TWINSTICKA:
        drawTwinStickA(startX, startY, buttonRadius, buttonPadding);
        break;
    case BUTTON_LAYOUT_BLANKA:
        drawBlankA(startX, startY, buttonRadius, buttonPadding);
        break;
    case BUTTON_LAYOUT_VLXA:
        drawVLXA(startX, startY, buttonRadius, buttonPadding);
        break;
    case BUTTON_LAYOUT_FIGHTBOARD_STICK:
        drawArcadeStick(startX, startY, buttonRadius, buttonPadding);
        break;
    case BUTTON_LAYOUT_FIGHTBOARD_MIRRORED:
        drawFightboardMirrored(startX, startY, buttonRadius, buttonPadding);
        break;
    }
}
//-----------------------------------------------------------------------------------------------
void SPIDisplayAddon::drawButtonLayoutRight(ButtonLayoutParamsRight &options)
{
    int32_t &startX = options.common.startX;
    int32_t &startY = options.common.startY;
    int32_t &buttonRadius = options.common.buttonRadius;
    int32_t &buttonPadding = options.common.buttonPadding;

    switch (options.layout)
    {
    case BUTTON_LAYOUT_ARCADE:
        drawArcadeButtons(startX, startY, buttonRadius, buttonPadding);
        break;
    case BUTTON_LAYOUT_STICKLESSB:
        drawSticklessButtons(startX, startY, buttonRadius, buttonPadding);
        break;
    case BUTTON_LAYOUT_BUTTONS_ANGLEDB:
        drawWasdButtons(startX, startY, buttonRadius, buttonPadding);
        break;
    case BUTTON_LAYOUT_VEWLIX:
        drawVewlix(startX, startY, buttonRadius, buttonPadding);
        break;
    case BUTTON_LAYOUT_VEWLIX7:
        drawVewlix7(startX, startY, buttonRadius, buttonPadding);
        break;
    case BUTTON_LAYOUT_CAPCOM:
        drawCapcom(startX, startY, buttonRadius, buttonPadding);
        break;
    case BUTTON_LAYOUT_CAPCOM6:
        drawCapcom6(startX, startY, buttonRadius, buttonPadding);
        break;
    case BUTTON_LAYOUT_SEGA2P:
        drawSega2p(startX, startY, buttonRadius, buttonPadding);
        break;
    case BUTTON_LAYOUT_NOIR8:
        drawNoir8(startX, startY, buttonRadius, buttonPadding);
        break;
    case BUTTON_LAYOUT_KEYBOARDB:
        drawMAMEB(startX, startY, buttonRadius, buttonPadding);
        break;
    case BUTTON_LAYOUT_DANCEPADB:
        drawDancepadB(startX, startY, buttonRadius, buttonPadding);
        break;
    case BUTTON_LAYOUT_TWINSTICKB:
        drawTwinStickB(startX, startY, buttonRadius, buttonPadding);
        break;
    case BUTTON_LAYOUT_BLANKB:
        drawSticklessButtons(startX, startY, buttonRadius, buttonPadding);
        break;
    case BUTTON_LAYOUT_VLXB:
        drawVLXB(startX, startY, buttonRadius, buttonPadding);
        break;
    case BUTTON_LAYOUT_FIGHTBOARD:
        drawFightboard(startX, startY, buttonRadius, buttonPadding);
        break;
    case BUTTON_LAYOUT_FIGHTBOARD_STICK_MIRRORED:
        drawArcadeStick(startX, startY, buttonRadius, buttonPadding);
        break;
    }
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// ここから下が実際の描画？
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
void SPIDisplayAddon::drawDiamond(int cx, int cy, int size, uint8_t colour, uint8_t filled)
{
    lcd.startWrite();
    lcd.setColor(color_table[colour]);
    if (filled)
    {
        int i;
        for (i = 0; i < size; i++)
        {
            lcd.drawLine(cx - i, cy - size + i, cx + i, cy - size + i);
            lcd.drawLine(cx - i, cy + size - i, cx + i, cy + size - i);
        }
        lcd.drawLine(cx - size, cy, cx + size, cy); // Fill in the middle
    }
    lcd.drawLine(cx - size, cy, cx, cy - size);
    lcd.drawLine(cx, cy - size, cx + size, cy);
    lcd.drawLine(cx + size, cy, cx, cy + size);
    lcd.drawLine(cx, cy + size, cx - size, cy);
    lcd.endWrite();
}
//-----------------------------------------------------------------------------------------------
void SPIDisplayAddon::drawStickless(int startX, int startY, int buttonRadius, int buttonPadding)
{

    const int buttonMargin = buttonPadding + (buttonRadius * 2);

    drawPreciseEllipse(startX, startY, buttonRadius, buttonRadius, 1, pressedLeft());
    drawPreciseEllipse(startX + buttonMargin, startY, buttonRadius, buttonRadius, 1, pressedDown());
    drawPreciseEllipse(startX + (buttonMargin * 1.875), startY + (buttonMargin / 2), buttonRadius, buttonRadius, 1, pressedRight());
    drawPreciseEllipse(startX + (buttonMargin * 2.25), startY + buttonMargin * 1.875, buttonRadius, buttonRadius, 1, pressedUp());
}
//-----------------------------------------------------------------------------------------------
void SPIDisplayAddon::drawWasdBox(int startX, int startY, int buttonRadius, int buttonPadding)
{
    const int buttonMargin = buttonPadding + (buttonRadius * 2);

    // WASD
    drawPreciseEllipse(startX, startY + buttonMargin * 0.5, buttonRadius, buttonRadius, 1, pressedLeft());
    drawPreciseEllipse(startX + buttonMargin, startY + buttonMargin * 0.875, buttonRadius, buttonRadius, 1, pressedDown());
    drawPreciseEllipse(startX + buttonMargin * 1.5, startY - buttonMargin * 0.125, buttonRadius, buttonRadius, 1, pressedUp());
    drawPreciseEllipse(startX + (buttonMargin * 2), startY + buttonMargin * 1.25, buttonRadius, buttonRadius, 1, pressedRight());
}

void SPIDisplayAddon::drawUDLR(int startX, int startY, int buttonRadius, int buttonPadding)
{
    const int buttonMargin = buttonPadding + (buttonRadius * 2);

    // UDLR
    drawPreciseEllipse(startX, startY + buttonMargin / 2, buttonRadius, buttonRadius, 1, pressedLeft());
    drawPreciseEllipse(startX + (buttonMargin * 0.875), startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pressedUp());
    drawPreciseEllipse(startX + (buttonMargin * 0.875), startY + buttonMargin * 1.25, buttonRadius, buttonRadius, 1, pressedDown());
    drawPreciseEllipse(startX + (buttonMargin * 1.625), startY + buttonMargin / 2, buttonRadius, buttonRadius, 1, pressedRight());
}

void SPIDisplayAddon::drawArcadeStick(int startX, int startY, int buttonRadius, int buttonPadding)
{
    const int buttonMargin = buttonPadding + (buttonRadius * 2);

    // Stick
    drawPreciseEllipse(startX + (buttonMargin / 2), startY + (buttonMargin / 2), buttonRadius * 1.25, buttonRadius * 1.25, 1, 0);

    if (pressedUp())
    {
        if (pressedLeft())
        {
            drawPreciseEllipse(startX + (buttonMargin / 5), startY + (buttonMargin / 5), buttonRadius, buttonRadius, 1, 1);
        }
        else if (pressedRight())
        {
            drawPreciseEllipse(startX + (buttonMargin * 0.875), startY + (buttonMargin / 5), buttonRadius, buttonRadius, 1, 1);
        }
        else
        {
            drawPreciseEllipse(startX + (buttonMargin / 2), startY, buttonRadius, buttonRadius, 1, 1);
        }
    }
    else if (pressedDown())
    {
        if (pressedLeft())
        {
            drawPreciseEllipse(startX + (buttonMargin / 5), startY + (buttonMargin * 0.875), buttonRadius, buttonRadius, 1, 1);
        }
        else if (pressedRight())
        {
            drawPreciseEllipse(startX + (buttonMargin * 0.875), startY + (buttonMargin * 0.875), buttonRadius, buttonRadius, 1, 1);
        }
        else
        {
            drawPreciseEllipse(startX + buttonMargin / 2, startY + buttonMargin, buttonRadius, buttonRadius, 1, 1);
        }
    }
    else if (pressedLeft())
    {
        drawPreciseEllipse(startX, startY + buttonMargin / 2, buttonRadius, buttonRadius, 1, 1);
    }
    else if (pressedRight())
    {
        drawPreciseEllipse(startX + buttonMargin, startY + buttonMargin / 2, buttonRadius, buttonRadius, 1, 1);
    }
    else
    {
        drawPreciseEllipse(startX + buttonMargin / 2, startY + buttonMargin / 2, buttonRadius, buttonRadius, 1, 1);
    }
}

void SPIDisplayAddon::drawVLXA(int startX, int startY, int buttonRadius, int buttonPadding)
{
    const int buttonMargin = buttonPadding + (buttonRadius * 2);

    // Stick
    drawPreciseEllipse(startX + (buttonMargin / 2), startY + (buttonMargin / 2), buttonRadius * 1.25, buttonRadius * 1.25, 1, 0);

    if (pressedUp())
    {
        if (pressedLeft())
        {
            drawPreciseEllipse(startX + (buttonMargin / 5), startY + (buttonMargin / 5), buttonRadius, buttonRadius, 1, 1);
        }
        else if (pressedRight())
        {
            drawPreciseEllipse(startX + (buttonMargin * 0.875), startY + (buttonMargin / 5), buttonRadius, buttonRadius, 1, 1);
        }
        else
        {
            drawPreciseEllipse(startX + (buttonMargin / 2), startY, buttonRadius, buttonRadius, 1, 1);
        }
    }
    else if (pressedDown())
    {
        if (pressedLeft())
        {
            drawPreciseEllipse(startX + (buttonMargin / 5), startY + (buttonMargin * 0.875), buttonRadius, buttonRadius, 1, 1);
        }
        else if (pressedRight())
        {
            drawPreciseEllipse(startX + (buttonMargin * 0.875), startY + (buttonMargin * 0.875), buttonRadius, buttonRadius, 1, 1);
        }
        else
        {
            drawPreciseEllipse(startX + buttonMargin / 2, startY + buttonMargin, buttonRadius, buttonRadius, 1, 1);
        }
    }
    else if (pressedLeft())
    {
        drawPreciseEllipse(startX, startY + buttonMargin / 2, buttonRadius, buttonRadius, 1, 1);
    }
    else if (pressedRight())
    {
        drawPreciseEllipse(startX + buttonMargin, startY + buttonMargin / 2, buttonRadius, buttonRadius, 1, 1);
    }
    else
    {
        drawPreciseEllipse(startX + buttonMargin / 2, startY + buttonMargin / 2, buttonRadius, buttonRadius, 1, 1);
    }
}

void SPIDisplayAddon::drawFightboardMirrored(int startX, int startY, int buttonRadius, int buttonPadding)
{
    const int buttonMargin = buttonPadding + (buttonRadius * 2);
    const int leftMargin = startX + buttonPadding + buttonRadius;

    drawPreciseEllipse(leftMargin, startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedL1());
    drawPreciseEllipse(leftMargin + buttonMargin, startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedR1());
    drawPreciseEllipse(leftMargin + (buttonMargin * 2), startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedB4());
    drawPreciseEllipse(leftMargin + (buttonMargin * 3), startY * 1.25, buttonRadius, buttonRadius, 1, pGamepad->pressedB3());

    drawPreciseEllipse(leftMargin, startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedL2());
    drawPreciseEllipse(leftMargin + buttonMargin, startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedR2());
    drawPreciseEllipse(leftMargin + (buttonMargin * 2), startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedB2());
    drawPreciseEllipse(leftMargin + (buttonMargin * 3), startY + buttonMargin * 1.25, buttonRadius, buttonRadius, 1, pGamepad->pressedB1());

    // Extra buttons
    drawPreciseEllipse(startX + buttonMargin * 0.5, startY + (buttonMargin * 1.5), 3, 3, 1, pGamepad->pressedL3());
    drawPreciseEllipse(startX + buttonMargin * 1.0625, startY + (buttonMargin * 1.5), 3, 3, 1, pGamepad->pressedS1());
    drawPreciseEllipse(startX + buttonMargin * 1.625, startY + (buttonMargin * 1.5), 3, 3, 1, pGamepad->pressedA1());
    drawPreciseEllipse(startX + buttonMargin * 2.125 + 0.0625, startY + (buttonMargin * 1.5), 3, 3, 1, pGamepad->pressedS2());
    drawPreciseEllipse(startX + buttonMargin * 2.75, startY + (buttonMargin * 1.5), 3, 3, 1, pGamepad->pressedR3());
}

void SPIDisplayAddon::drawTwinStickA(int startX, int startY, int buttonRadius, int buttonPadding)
{
    const int buttonMargin = buttonPadding + (buttonRadius * 2);

    // Stick
    drawPreciseEllipse(startX + (buttonMargin / 2), startY + (buttonMargin / 2), buttonRadius * 1.25, buttonRadius * 1.25, 1, 0);

    if (pressedUp())
    {
        if (pressedLeft())
        {
            drawPreciseEllipse(startX + (buttonMargin / 5), startY + (buttonMargin / 5), buttonRadius, buttonRadius, 1, 1);
        }
        else if (pressedRight())
        {
            drawPreciseEllipse(startX + (buttonMargin * 0.875), startY + (buttonMargin / 5), buttonRadius, buttonRadius, 1, 1);
        }
        else
        {
            drawPreciseEllipse(startX + (buttonMargin / 2), startY, buttonRadius, buttonRadius, 1, 1);
        }
    }
    else if (pressedDown())
    {
        if (pressedLeft())
        {
            drawPreciseEllipse(startX + (buttonMargin / 5), startY + (buttonMargin * 0.875), buttonRadius, buttonRadius, 1, 1);
        }
        else if (pressedRight())
        {
            drawPreciseEllipse(startX + (buttonMargin * 0.875), startY + (buttonMargin * 0.875), buttonRadius, buttonRadius, 1, 1);
        }
        else
        {
            drawPreciseEllipse(startX + buttonMargin / 2, startY + buttonMargin, buttonRadius, buttonRadius, 1, 1);
        }
    }
    else if (pressedLeft())
    {
        drawPreciseEllipse(startX, startY + buttonMargin / 2, buttonRadius, buttonRadius, 1, 1);
    }
    else if (pressedRight())
    {
        drawPreciseEllipse(startX + buttonMargin, startY + buttonMargin / 2, buttonRadius, buttonRadius, 1, 1);
    }
    else
    {
        drawPreciseEllipse(startX + buttonMargin / 2, startY + buttonMargin / 2, buttonRadius, buttonRadius, 1, 1);
    }
}

void SPIDisplayAddon::drawTwinStickB(int startX, int startY, int buttonRadius, int buttonPadding)
{
    const int buttonMargin = buttonPadding + (buttonRadius * 2);

    // Stick
    drawPreciseEllipse(startX + (buttonMargin / 2), startY + (buttonMargin / 2), buttonRadius * 1.25, buttonRadius * 1.25, 1, 0);

    if (pGamepad->pressedB4())
    {
        if (pGamepad->pressedB3())
        {
            drawPreciseEllipse(startX + (buttonMargin / 5), startY + (buttonMargin / 5), buttonRadius, buttonRadius, 1, 1);
        }
        else if (pGamepad->pressedB2())
        {
            drawPreciseEllipse(startX + (buttonMargin * 0.875), startY + (buttonMargin / 5), buttonRadius, buttonRadius, 1, 1);
        }
        else
        {
            drawPreciseEllipse(startX + (buttonMargin / 2), startY, buttonRadius, buttonRadius, 1, 1);
        }
    }
    else if (pGamepad->pressedB1())
    {
        if (pGamepad->pressedB3())
        {
            drawPreciseEllipse(startX + (buttonMargin / 5), startY + (buttonMargin * 0.875), buttonRadius, buttonRadius, 1, 1);
        }
        else if (pGamepad->pressedB2())
        {
            drawPreciseEllipse(startX + (buttonMargin * 0.875), startY + (buttonMargin * 0.875), buttonRadius, buttonRadius, 1, 1);
        }
        else
        {
            drawPreciseEllipse(startX + buttonMargin / 2, startY + buttonMargin, buttonRadius, buttonRadius, 1, 1);
        }
    }
    else if (pGamepad->pressedB3())
    {
        drawPreciseEllipse(startX, startY + buttonMargin / 2, buttonRadius, buttonRadius, 1, 1);
    }
    else if (pGamepad->pressedB2())
    {
        drawPreciseEllipse(startX + buttonMargin, startY + buttonMargin / 2, buttonRadius, buttonRadius, 1, 1);
    }
    else
    {
        drawPreciseEllipse(startX + buttonMargin / 2, startY + buttonMargin / 2, buttonRadius, buttonRadius, 1, 1);
    }
}

void SPIDisplayAddon::drawMAMEA(int startX, int startY, int buttonSize, int buttonPadding)
{
    const int buttonMargin = buttonPadding + buttonSize;

    // MAME
    drawRectangle(startX, startY + buttonMargin, startX + buttonSize, startY + buttonSize + buttonMargin, 1, pressedLeft());
    drawRectangle(startX + buttonMargin, startY + buttonMargin, startX + buttonSize + buttonMargin, startY + buttonSize + buttonMargin, 1, pressedDown());
    drawRectangle(startX + buttonMargin, startY, startX + buttonSize + buttonMargin, startY + buttonSize, 1, pressedUp());
    drawRectangle(startX + buttonMargin * 2, startY + buttonMargin, startX + buttonSize + buttonMargin * 2, startY + buttonSize + buttonMargin, 1, pressedRight());
}

void SPIDisplayAddon::drawMAMEB(int startX, int startY, int buttonSize, int buttonPadding)
{
    const int buttonMargin = buttonPadding + buttonSize;

    // 6-button MAME Style
    drawRectangle(startX, startY, startX + buttonSize, startY + buttonSize, 1, pGamepad->pressedB3());
    drawRectangle(startX + buttonMargin, startY, startX + buttonSize + buttonMargin, startY + buttonSize, 1, pGamepad->pressedB4());
    drawRectangle(startX + buttonMargin * 2, startY, startX + buttonSize + buttonMargin * 2, startY + buttonSize, 1, pGamepad->pressedR1());

    drawRectangle(startX, startY + buttonMargin, startX + buttonSize, startY + buttonMargin + buttonSize, 1, pGamepad->pressedB1());
    drawRectangle(startX + buttonMargin, startY + buttonMargin, startX + buttonSize + buttonMargin, startY + buttonMargin + buttonSize, 1, pGamepad->pressedB2());
    drawRectangle(startX + buttonMargin * 2, startY + buttonMargin, startX + buttonSize + buttonMargin * 2, startY + buttonMargin + buttonSize, 1, pGamepad->pressedR2());
}

void SPIDisplayAddon::drawKeyboardAngled(int startX, int startY, int buttonRadius, int buttonPadding)
{
    const int buttonMargin = buttonPadding + (buttonRadius * 2);

    // MixBox
    drawDiamond(startX, startY, buttonRadius, 1, pressedLeft());
    drawDiamond(startX + buttonMargin / 2, startY + buttonMargin / 2, buttonRadius, 1, pressedDown());
    drawDiamond(startX + buttonMargin, startY, buttonRadius, 1, pressedUp());
    drawDiamond(startX + buttonMargin, startY + buttonMargin, buttonRadius, 1, pressedRight());
}

void SPIDisplayAddon::drawVewlix(int startX, int startY, int buttonRadius, int buttonPadding)
{
    const int buttonMargin = buttonPadding + (buttonRadius * 2);

    // 8-button Vewlix
    drawPreciseEllipse(startX + (buttonMargin * 2.75), startY + (buttonMargin * 0.2), buttonRadius, buttonRadius, 1, pGamepad->pressedB3());
    drawPreciseEllipse(startX + (buttonMargin * 3.75), startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedB4());
    drawPreciseEllipse(startX + (buttonMargin * 4.75), startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedR1());
    drawPreciseEllipse(startX + (buttonMargin * 5.75), startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedL1());

    drawPreciseEllipse(startX + (buttonMargin * 2.75) - (buttonMargin / 3), startY + buttonMargin + (buttonMargin * 0.2), buttonRadius, buttonRadius, 1, pGamepad->pressedB1());
    drawPreciseEllipse(startX + (buttonMargin * 3.75) - (buttonMargin / 3), startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedB2());
    drawPreciseEllipse(startX + (buttonMargin * 4.75) - (buttonMargin / 3), startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedR2());
    drawPreciseEllipse(startX + (buttonMargin * 5.75) - (buttonMargin / 3), startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedL2());
}

void SPIDisplayAddon::drawVLXB(int startX, int startY, int buttonRadius, int buttonPadding)
{
    const int buttonMargin = buttonPadding + (buttonRadius * 2);

    // 9-button Hori VLX
    drawPreciseEllipse(startX + (buttonMargin * 2.75), startY + (buttonMargin * 0.2), buttonRadius, buttonRadius, 1, pGamepad->pressedB3());
    drawPreciseEllipse(startX + (buttonMargin * 3.75), startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedB4());
    drawPreciseEllipse(startX + (buttonMargin * 4.75), startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedR1());
    drawPreciseEllipse(startX + (buttonMargin * 5.75), startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedL1());

    drawPreciseEllipse(startX + (buttonMargin * 2.75) - (buttonMargin / 3), startY + buttonMargin + (buttonMargin * 0.2), buttonRadius, buttonRadius, 1, pGamepad->pressedB1());
    drawPreciseEllipse(startX + (buttonMargin * 3.75) - (buttonMargin / 3), startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedB2());
    drawPreciseEllipse(startX + (buttonMargin * 4.75) - (buttonMargin / 3), startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedR2());
    drawPreciseEllipse(startX + (buttonMargin * 5.75) - (buttonMargin / 3), startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedL2());

    drawPreciseEllipse(startX + (buttonMargin * 7.4) - (buttonMargin / 3.5), startY + buttonMargin - (buttonMargin / 1.5), buttonRadius * .8, buttonRadius * .8, 1, pGamepad->pressedS2());
}

void SPIDisplayAddon::drawFightboard(int startX, int startY, int buttonRadius, int buttonPadding)
{
    const int buttonMargin = buttonPadding + (buttonRadius * 2);

    drawPreciseEllipse((startX + buttonMargin * 3.625), startY * 1.25, buttonRadius, buttonRadius, 1, pGamepad->pressedB3());
    drawPreciseEllipse((startX + buttonMargin * 4.625), startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedB4());
    drawPreciseEllipse((startX + buttonMargin * 5.625), startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedR1());
    drawPreciseEllipse((startX + buttonMargin * 6.625), startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedL1());

    drawPreciseEllipse((startX + buttonMargin * 3.625), startY + buttonMargin * 1.25, buttonRadius, buttonRadius, 1, pGamepad->pressedB1());
    drawPreciseEllipse((startX + buttonMargin * 4.625), startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedB2());
    drawPreciseEllipse((startX + buttonMargin * 5.625), startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedR2());
    drawPreciseEllipse((startX + buttonMargin * 6.625), startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedL2());

    // Extra buttons
    drawPreciseEllipse(startX + buttonMargin * 4.5, startY + (buttonMargin * 1.5), 3, 3, 1, pGamepad->pressedL3());
    drawPreciseEllipse(startX + buttonMargin * 5.0625, startY + (buttonMargin * 1.5), 3, 3, 1, pGamepad->pressedS1());
    drawPreciseEllipse(startX + buttonMargin * 5.625, startY + (buttonMargin * 1.5), 3, 3, 1, pGamepad->pressedA1());
    drawPreciseEllipse(startX + buttonMargin * 6.125 + 0.0625, startY + (buttonMargin * 1.5), 3, 3, 1, pGamepad->pressedS2());
    drawPreciseEllipse(startX + buttonMargin * 6.75, startY + (buttonMargin * 1.5), 3, 3, 1, pGamepad->pressedR3());
}

void SPIDisplayAddon::drawVewlix7(int startX, int startY, int buttonRadius, int buttonPadding)
{
    const int buttonMargin = buttonPadding + (buttonRadius * 2);

    // 8-button Vewlix
    drawPreciseEllipse(startX + (buttonMargin * 2.75), startY + (buttonMargin * 0.2), buttonRadius, buttonRadius, 1, pGamepad->pressedB3());
    drawPreciseEllipse(startX + (buttonMargin * 3.75), startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedB4());
    drawPreciseEllipse(startX + (buttonMargin * 4.75), startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedR1());
    drawPreciseEllipse(startX + (buttonMargin * 5.75), startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedL1());

    drawPreciseEllipse(startX + (buttonMargin * 2.75) - (buttonMargin / 3), startY + buttonMargin + (buttonMargin * 0.2), buttonRadius, buttonRadius, 1, pGamepad->pressedB1());
    drawPreciseEllipse(startX + (buttonMargin * 3.75) - (buttonMargin / 3), startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedB2());
    drawPreciseEllipse(startX + (buttonMargin * 4.75) - (buttonMargin / 3), startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedR2());
    // drawPreciseEllipse(startX + (buttonMargin * 5.75) - (buttonMargin / 3), startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, gamepad->pressedL2());
}

void SPIDisplayAddon::drawSega2p(int startX, int startY, int buttonRadius, int buttonPadding)
{
    const int buttonMargin = buttonPadding + (buttonRadius * 2);

    // 8-button Sega2P
    drawPreciseEllipse(startX + (buttonMargin * 2.75), startY + (buttonMargin / 3), buttonRadius, buttonRadius, 1, pGamepad->pressedB3());
    drawPreciseEllipse(startX + (buttonMargin * 3.75), startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedB4());
    drawPreciseEllipse(startX + (buttonMargin * 4.75), startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedR1());
    drawPreciseEllipse(startX + (buttonMargin * 5.75), startY, buttonRadius, buttonRadius, 1, pGamepad->pressedL1());

    drawPreciseEllipse(startX + (buttonMargin * 2.75), startY + buttonMargin + (buttonMargin / 3), buttonRadius, buttonRadius, 1, pGamepad->pressedB1());
    drawPreciseEllipse(startX + (buttonMargin * 3.75), startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedB2());
    drawPreciseEllipse(startX + (buttonMargin * 4.75), startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedR2());
    drawPreciseEllipse(startX + (buttonMargin * 5.75), startY + buttonMargin, buttonRadius, buttonRadius, 1, pGamepad->pressedL2());
}

void SPIDisplayAddon::drawNoir8(int startX, int startY, int buttonRadius, int buttonPadding)
{
    const int buttonMargin = buttonPadding + (buttonRadius * 2);

    // 8-button Noir8
    drawPreciseEllipse(startX + (buttonMargin * 2.75), startY + (buttonMargin / 3.5), buttonRadius, buttonRadius, 1, pGamepad->pressedB3());
    drawPreciseEllipse(startX + (buttonMargin * 3.75), startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedB4());
    drawPreciseEllipse(startX + (buttonMargin * 4.75), startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedR1());
    drawPreciseEllipse(startX + (buttonMargin * 5.75), startY, buttonRadius, buttonRadius, 1, pGamepad->pressedL1());

    drawPreciseEllipse(startX + (buttonMargin * 2.75), startY + buttonMargin + (buttonMargin / 3.5), buttonRadius, buttonRadius, 1, pGamepad->pressedB1());
    drawPreciseEllipse(startX + (buttonMargin * 3.75), startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedB2());
    drawPreciseEllipse(startX + (buttonMargin * 4.75), startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedR2());
    drawPreciseEllipse(startX + (buttonMargin * 5.75), startY + buttonMargin, buttonRadius, buttonRadius, 1, pGamepad->pressedL2());
}

void SPIDisplayAddon::drawCapcom(int startX, int startY, int buttonRadius, int buttonPadding)
{
    const int buttonMargin = buttonPadding + (buttonRadius * 2);

    // 8-button Capcom
    drawPreciseEllipse(startX + buttonMargin * 3.25, startY, buttonRadius, buttonRadius, 1, pGamepad->pressedB3());
    drawPreciseEllipse(startX + buttonMargin * 4.25, startY, buttonRadius, buttonRadius, 1, pGamepad->pressedB4());
    drawPreciseEllipse(startX + buttonMargin * 5.25, startY, buttonRadius, buttonRadius, 1, pGamepad->pressedR1());
    drawPreciseEllipse(startX + buttonMargin * 6.25, startY, buttonRadius, buttonRadius, 1, pGamepad->pressedL1());

    drawPreciseEllipse(startX + buttonMargin * 3.25, startY + buttonMargin, buttonRadius, buttonRadius, 1, pGamepad->pressedB1());
    drawPreciseEllipse(startX + buttonMargin * 4.25, startY + buttonMargin, buttonRadius, buttonRadius, 1, pGamepad->pressedB2());
    drawPreciseEllipse(startX + buttonMargin * 5.25, startY + buttonMargin, buttonRadius, buttonRadius, 1, pGamepad->pressedR2());
    drawPreciseEllipse(startX + buttonMargin * 6.25, startY + buttonMargin, buttonRadius, buttonRadius, 1, pGamepad->pressedL2());
}

void SPIDisplayAddon::drawCapcom6(int startX, int startY, int buttonRadius, int buttonPadding)
{
    const int buttonMargin = buttonPadding + (buttonRadius * 2);

    // 6-button Capcom
    drawPreciseEllipse(startX + buttonMargin * 3.25, startY, buttonRadius, buttonRadius, 1, pGamepad->pressedB3());
    drawPreciseEllipse(startX + buttonMargin * 4.25, startY, buttonRadius, buttonRadius, 1, pGamepad->pressedB4());
    drawPreciseEllipse(startX + buttonMargin * 5.25, startY, buttonRadius, buttonRadius, 1, pGamepad->pressedR1());

    drawPreciseEllipse(startX + buttonMargin * 3.25, startY + buttonMargin, buttonRadius, buttonRadius, 1, pGamepad->pressedB1());
    drawPreciseEllipse(startX + buttonMargin * 4.25, startY + buttonMargin, buttonRadius, buttonRadius, 1, pGamepad->pressedB2());
    drawPreciseEllipse(startX + buttonMargin * 5.25, startY + buttonMargin, buttonRadius, buttonRadius, 1, pGamepad->pressedR2());
}

void SPIDisplayAddon::drawSticklessButtons(int startX, int startY, int buttonRadius, int buttonPadding)
{
    const int buttonMargin = buttonPadding + (buttonRadius * 2);

    // 8-button
    drawPreciseEllipse(startX + (buttonMargin * 2.75), startY, buttonRadius, buttonRadius, 1, pGamepad->pressedB3());
    drawPreciseEllipse(startX + (buttonMargin * 3.75), startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedB4());
    drawPreciseEllipse(startX + (buttonMargin * 4.75), startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedR1());
    drawPreciseEllipse(startX + (buttonMargin * 5.75), startY, buttonRadius, buttonRadius, 1, pGamepad->pressedL1());

    drawPreciseEllipse(startX + (buttonMargin * 2.75), startY + buttonMargin, buttonRadius, buttonRadius, 1, pGamepad->pressedB1());
    drawPreciseEllipse(startX + (buttonMargin * 3.75), startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedB2());
    drawPreciseEllipse(startX + (buttonMargin * 4.75), startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedR2());
    drawPreciseEllipse(startX + (buttonMargin * 5.75), startY + buttonMargin, buttonRadius, buttonRadius, 1, pGamepad->pressedL2());
}

void SPIDisplayAddon::drawWasdButtons(int startX, int startY, int buttonRadius, int buttonPadding)
{
    const int buttonMargin = buttonPadding + (buttonRadius * 2);

    // 8-button
    drawPreciseEllipse(startX + buttonMargin * 3.625, startY, buttonRadius, buttonRadius, 1, pGamepad->pressedB3());
    drawPreciseEllipse(startX + buttonMargin * 4.625, startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedB4());
    drawPreciseEllipse(startX + buttonMargin * 5.625, startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedR1());
    drawPreciseEllipse(startX + buttonMargin * 6.625, startY, buttonRadius, buttonRadius, 1, pGamepad->pressedL1());

    drawPreciseEllipse(startX + buttonMargin * 3.25, startY + buttonMargin, buttonRadius, buttonRadius, 1, pGamepad->pressedB1());
    drawPreciseEllipse(startX + buttonMargin * 4.25, startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedB2());
    drawPreciseEllipse(startX + buttonMargin * 5.25, startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedR2());
    drawPreciseEllipse(startX + buttonMargin * 6.25, startY + buttonMargin, buttonRadius, buttonRadius, 1, pGamepad->pressedL2());
}

void SPIDisplayAddon::drawArcadeButtons(int startX, int startY, int buttonRadius, int buttonPadding)
{
    const int buttonMargin = buttonPadding + (buttonRadius * 2);

    // 8-button
    drawPreciseEllipse(startX + buttonMargin * 3.125, startY, buttonRadius, buttonRadius, 1, pGamepad->pressedB3());
    drawPreciseEllipse(startX + buttonMargin * 4.125, startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedB4());
    drawPreciseEllipse(startX + buttonMargin * 5.125, startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedR1());
    drawPreciseEllipse(startX + buttonMargin * 6.125, startY, buttonRadius, buttonRadius, 1, pGamepad->pressedL1());

    drawPreciseEllipse(startX + buttonMargin * 2.875, startY + buttonMargin, buttonRadius, buttonRadius, 1, pGamepad->pressedB1());
    drawPreciseEllipse(startX + buttonMargin * 3.875, startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedB2());
    drawPreciseEllipse(startX + buttonMargin * 4.875, startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedR2());
    drawPreciseEllipse(startX + buttonMargin * 5.875, startY + buttonMargin, buttonRadius, buttonRadius, 1, pGamepad->pressedL2());
}

// I pulled this out of my PR, brought it back because of recent talks re: SOCD and rhythm games
// Enjoy!

void SPIDisplayAddon::drawDancepadA(int startX, int startY, int buttonSize, int buttonPadding)
{
    const int buttonMargin = buttonPadding + buttonSize;

    drawRectangle(startX, startY + buttonMargin, startX + buttonSize, startY + buttonSize + buttonMargin, 1, pressedLeft());
    drawRectangle(startX + buttonMargin, startY + buttonMargin * 2, startX + buttonSize + buttonMargin, startY + buttonSize + buttonMargin * 2, 1, pressedDown());
    drawRectangle(startX + buttonMargin, startY, startX + buttonSize + buttonMargin, startY + buttonSize, 1, pressedUp());
    drawRectangle(startX + buttonMargin * 2, startY + buttonMargin, startX + buttonSize + buttonMargin * 2, startY + buttonSize + buttonMargin, 1, pressedRight());
}

void SPIDisplayAddon::drawDancepadB(int startX, int startY, int buttonSize, int buttonPadding)
{
    const int buttonMargin = buttonPadding + buttonSize;

    drawRectangle(startX, startY, startX + buttonSize, startY + buttonSize, 1, pGamepad->pressedB2());                                                                             // Up/Left
    drawRectangle(startX, startY + buttonMargin * 2, startX + buttonSize, startY + buttonSize + buttonMargin * 2, 1, pGamepad->pressedB4());                                       // Down/Left
    drawRectangle(startX + buttonMargin * 2, startY, startX + buttonSize + buttonMargin * 2, startY + buttonSize, 1, pGamepad->pressedB1());                                       // Up/Right
    drawRectangle(startX + buttonMargin * 2, startY + buttonMargin * 2, startX + buttonSize + buttonMargin * 2, startY + buttonSize + buttonMargin * 2, 1, pGamepad->pressedB3()); // Down/Right
}

void SPIDisplayAddon::drawBlankA(int startX, int startY, int buttonSize, int buttonPadding)
{
}

void SPIDisplayAddon::drawBlankB(int startX, int startY, int buttonSize, int buttonPadding)
{
}

void SPIDisplayAddon::drawSplashScreen(int splashMode, uint8_t *splashChoice, int splashSpeed)
{
    int mils = getMillis();
    switch (splashMode)
    {
    case SPLASH_MODE_STATIC: // Default, display static or custom image
        drawSprite(splashChoice, 128, 64, 16, 0, 0, 1);
        break;
    case SPLASH_MODE_CLOSEIN: // Close-in. Animate the GP2040 logo
        drawSprite((uint8_t *)bootLogoTop, 43, 39, 6, 43, std::min<int>((mils / splashSpeed) - 39, 0), 1);
        drawSprite((uint8_t *)bootLogoBottom, 80, 21, 10, 24, std::max<int>(64 - (mils / (splashSpeed * 2)), 44), 1);
        break;
    case SPLASH_MODE_CLOSEINCUSTOM: // Close-in on custom image or delayed close-in if custom image does not exist
        drawSprite(splashChoice, 128, 64, 16, 0, 0, 1);
        if (mils > 2500)
        {
            int milss = mils - 2500;
            drawRectangle(0, 0, 127, 1 + (milss / splashSpeed), 0, 1);
            drawRectangle(0, 63, 127, 62 - (milss / (splashSpeed * 2)), 0, 1);
            drawSprite((uint8_t *)bootLogoTop, 43, 39, 6, 43, std::min<int>((milss / splashSpeed) - 39, 0), 1);
            drawSprite((uint8_t *)bootLogoBottom, 80, 21, 10, 24, std::max<int>(64 - (milss / (splashSpeed * 2)), 44), 1);
        }
        break;
    }
}

void SPIDisplayAddon::drawStatusBar(Gamepad *gamepad)
{
    const DisplayOptions &options = getDisplayOptions();
    const TurboOptions &turboOptions = Storage::getInstance().getAddonOptions().turboOptions;

    // Limit to 21 chars with 6x8 font for now
    statusBar.clear();

    switch (gamepad->getOptions().inputMode)
    {
    case INPUT_MODE_HID:
        statusBar += "DINPUT";
        break;
    case INPUT_MODE_SWITCH:
        statusBar += "SWITCH";
        break;
    case INPUT_MODE_XINPUT:
        statusBar += "XINPUT";
        break;
    case INPUT_MODE_PS4:
        if (PS4Data::getInstance().authsent == true)
        {
            statusBar += "PS4:AS";
        }
        else
        {
            statusBar += "PS4   ";
        }
        break;
    case INPUT_MODE_KEYBOARD:
        statusBar += "HID-KB";
        break;
    case INPUT_MODE_CONFIG:
        statusBar += "CONFIG";
        break;
    }

    if (turboOptions.enabled && isValidPin(turboOptions.buttonPin))
    {
        statusBar += " T";
        if (turboOptions.shotCount < 10) // padding
            statusBar += "0";
        statusBar += std::to_string(turboOptions.shotCount);
    }
    else
    {
        statusBar += "    "; // no turbo, don't show Txx setting
    }
    switch (gamepad->getOptions().dpadMode)
    {

    case DPAD_MODE_DIGITAL:
        statusBar += " DP";
        break;
    case DPAD_MODE_LEFT_ANALOG:
        statusBar += " LS";
        break;
    case DPAD_MODE_RIGHT_ANALOG:
        statusBar += " RS";
        break;
    }

    switch (Gamepad::resolveSOCDMode(gamepad->getOptions()))
    {
    case SOCD_MODE_NEUTRAL:
        statusBar += " SOCD-N";
        break;
    case SOCD_MODE_UP_PRIORITY:
        statusBar += " SOCD-U";
        break;
    case SOCD_MODE_SECOND_INPUT_PRIORITY:
        statusBar += " SOCD-L";
        break;
    case SOCD_MODE_FIRST_INPUT_PRIORITY:
        statusBar += " SOCD-F";
        break;
    case SOCD_MODE_BYPASS:
        statusBar += " SOCD-X";
        break;
    }
    drawText(0, 0, statusBar);
}

bool SPIDisplayAddon::pressedUp()
{
    switch (gamepad->getOptions().dpadMode)
    {
    case DPAD_MODE_DIGITAL:
        return pGamepad->pressedUp();
    case DPAD_MODE_LEFT_ANALOG:
        return pGamepad->state.ly == GAMEPAD_JOYSTICK_MIN;
    case DPAD_MODE_RIGHT_ANALOG:
        return pGamepad->state.ry == GAMEPAD_JOYSTICK_MIN;
    }

    return false;
}

bool SPIDisplayAddon::pressedDown()
{
    switch (gamepad->getOptions().dpadMode)
    {
    case DPAD_MODE_DIGITAL:
        return pGamepad->pressedDown();
    case DPAD_MODE_LEFT_ANALOG:
        return pGamepad->state.ly == GAMEPAD_JOYSTICK_MAX;
    case DPAD_MODE_RIGHT_ANALOG:
        return pGamepad->state.ry == GAMEPAD_JOYSTICK_MAX;
    }

    return false;
}

bool SPIDisplayAddon::pressedLeft()
{
    switch (gamepad->getOptions().dpadMode)
    {
    case DPAD_MODE_DIGITAL:
        return pGamepad->pressedLeft();
    case DPAD_MODE_LEFT_ANALOG:
        return pGamepad->state.lx == GAMEPAD_JOYSTICK_MIN;
    case DPAD_MODE_RIGHT_ANALOG:
        return pGamepad->state.rx == GAMEPAD_JOYSTICK_MIN;
    }

    return false;
}

bool SPIDisplayAddon::pressedRight()
{
    switch (gamepad->getOptions().dpadMode)
    {
    case DPAD_MODE_DIGITAL:
        return pGamepad->pressedRight();
    case DPAD_MODE_LEFT_ANALOG:
        return pGamepad->state.lx == GAMEPAD_JOYSTICK_MAX;
    case DPAD_MODE_RIGHT_ANALOG:
        return pGamepad->state.rx == GAMEPAD_JOYSTICK_MAX;
    }

    return false;
}
