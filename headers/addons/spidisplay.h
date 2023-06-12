/*
 */

#ifndef SPIDISPLAY_H_
#define SPIDISPLAY_H_

#include <string>
#include <hardware/spi.h>
#include "gpaddon.h"
#include "gamepad.h"
#include "storagemanager.h"
#include "st7789/st7789.h"

// i2c Display Module
#define SPIDisplayName "SPIDisplay"

// i2C OLED Display
class SPIDisplayAddon : public GPAddon
{
public:
	virtual bool available();
	virtual void setup();
	virtual void preprocess();
	virtual void process();
	virtual std::string name() { return SPIDisplayName; }
private:
	int32_t testcount;
#if 0
	int initDisplay(int typeOverride);
	bool isSH1106(int detectedDisplay);
	void clearScreen(int render); // DisplayModule
	void drawStickless(int startX, int startY, int buttonRadius, int buttonPadding);
	void drawWasdBox(int startX, int startY, int buttonRadius, int buttonPadding);
	void drawArcadeStick(int startX, int startY, int buttonRadius, int buttonPadding);
	void drawStatusBar(Gamepad*);
	void drawText(int startX, int startY, std::string text);
	void initMenu(char**);
	//Adding my stuff here, remember to sort before PR
	void drawDiamond(int cx, int cy, int size, uint8_t colour, uint8_t filled);
	void drawUDLR(int startX, int startY, int buttonRadius, int buttonPadding);
	void drawMAMEA(int startX, int startY, int buttonSize, int buttonPadding);
	void drawMAMEB(int startX, int startY, int buttonSize, int buttonPadding);
	void drawKeyboardAngled(int startX, int startY, int buttonRadius, int buttonPadding);
	void drawVewlix(int startX, int startY, int buttonRadius, int buttonPadding);
	void drawVewlix7(int startX, int startY, int buttonRadius, int buttonPadding);
	void drawSega2p(int startX, int startY, int buttonRadius, int buttonPadding);
	void drawNoir8(int startX, int startY, int buttonRadius, int buttonPadding);
	void drawCapcom(int startX, int startY, int buttonRadius, int buttonPadding);
	void drawCapcom6(int startX, int startY, int buttonRadius, int buttonPadding);
	void drawSticklessButtons(int startX, int startY, int buttonRadius, int buttonPadding);
	void drawWasdButtons(int startX, int startY, int buttonRadius, int buttonPadding);
	void drawArcadeButtons(int startX, int startY, int buttonRadius, int buttonPadding);
	void drawSplashScreen(int splashMode, uint8_t* splashChoice, int splashSpeed);
	void drawDancepadA(int startX, int startY, int buttonSize, int buttonPadding);
	void drawDancepadB(int startX, int startY, int buttonSize, int buttonPadding);
	void drawTwinStickA(int startX, int startY, int buttonSize, int buttonPadding);
	void drawTwinStickB(int startX, int startY, int buttonSize, int buttonPadding);
	void drawBlankA(int startX, int startY, int buttonSize, int buttonPadding);
	void drawBlankB(int startX, int startY, int buttonSize, int buttonPadding);
	void drawVLXA(int startX, int startY, int buttonRadius, int buttonPadding);
	void drawVLXB(int startX, int startY, int buttonRadius, int buttonPadding);
	void drawButtonLayoutLeft(ButtonLayoutParamsLeft& options);
	void drawButtonLayoutRight(ButtonLayoutParamsRight& options);
	void drawFightboard(int startX, int startY, int buttonRadius, int buttonPadding);
	void drawFightboardMirrored(int startX, int startY, int buttonRadius, int buttonPadding);
	void drawFightboardStick(int startX, int startY, int buttonRadius, int buttonPadding);
	void drawFightboardStickMirrored(int startX, int startY, int buttonRadius, int buttonPadding);
	bool pressedUp();
	bool pressedDown();
	bool pressedLeft();
	bool pressedRight();
	const DisplayOptions& getDisplayOptions();
	bool isDisplayPowerOff();
	void setDisplayPower(uint8_t status);
	uint32_t displaySaverTimeout = 0;
	int32_t displaySaverTimer;
	uint8_t displayIsPowerOn = 1;
	uint32_t prevMillis;
	uint8_t ucBackBuffer[1024];
	OBDISP obd;
	std::string statusBar;
	Gamepad* gamepad;
	Gamepad* pGamepad;
	bool configMode;

	enum DisplayMode {
		CONFIG_INSTRUCTION,
		BUTTONS,
		SPLASH
	};

	DisplayMode getDisplayMode();
	DisplayMode prevDisplayMode;
	uint16_t prevButtonState;
#endif
};

#endif
