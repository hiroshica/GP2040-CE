#ifndef __CONFIGURATION_H__
#define __CONFIGURATION_H__

#include "Observers/UsbGamepadSaturnControllerObserver.h"

// Can be either USB_TYPE_KEYBOARD or USB_TYPE_GAMEPAD
// #include "configuration_constants.h"
// #define PLAYER_1_USB_TYPE     USB_TYPE_GAMEPAD

// When true: Forces USB to always be connected for all players
// When false: USB will only connect when player 1 controller is detected and will dynamically
//             update USB descriptor when player 2 is connected/disconnected.
#define USB_ALWAYS_CONNECTED false
// Delay in seconds between controller disconnected and when USB is disconnected or updates.
#define DISCONNECT_DELAY_S 10.0
// Special case delay when only player 1 was connected and disconnects.
#define SINGLE_PLAYER_DISCONNECT_DELAY_S 1.0

#define SATURN_CONTROLLER_1_PIN_1 2
#define SATURN_CONTROLLER_1_PIN_2 3
#define SATURN_CONTROLLER_1_PIN_3 4
#define SATURN_CONTROLLER_1_PIN_4 5
#define SATURN_CONTROLLER_1_PIN_5 6
#define SATURN_CONTROLLER_1_PIN_6 7
#define SATURN_CONTROLLER_1_PIN_7 8

// Configuration specifically for Retropie - If all else fails, this will work with sdl2 driver in
// "fallback configuration" if it can't find a configuration for some reason. If you want XYZ and
// mode buttons to work, make sure you have the setting enabled to use 6 button controller in
// genesis settings (which is usually set to 3 button by default).
#define GAMEPAD_MAPPING                                               \
  {                                                                   \
    UsbGamepadSaturnControllerObserver::ButtonMap::MAP_KEY_A,         \
        UsbGamepadSaturnControllerObserver::ButtonMap::MAP_KEY_B,     \
        UsbGamepadSaturnControllerObserver::ButtonMap::MAP_KEY_C,     \
        UsbGamepadSaturnControllerObserver::ButtonMap::MAP_KEY_X,     \
        UsbGamepadSaturnControllerObserver::ButtonMap::MAP_KEY_Y,     \
        UsbGamepadSaturnControllerObserver::ButtonMap::MAP_KEY_Z,     \
        UsbGamepadSaturnControllerObserver::ButtonMap::MAP_KEY_L,     \
        UsbGamepadSaturnControllerObserver::ButtonMap::MAP_KEY_R,     \
        UsbGamepadSaturnControllerObserver::ButtonMap::MAP_KEY_START, \
        UsbGamepadSaturnControllerObserver::ButtonMap::MAP_LANALOG_Y, \
        UsbGamepadSaturnControllerObserver::ButtonMap::MAP_LANALOG_X, \
        UsbGamepadSaturnControllerObserver::ButtonMap::MAP_RANALOG_Y, \
        UsbGamepadSaturnControllerObserver::ButtonMap::MAP_RANALOG_X, \
        UsbGamepadSaturnControllerObserver::ButtonMap::MAP_LTRIGER,   \
        UsbGamepadSaturnControllerObserver::ButtonMap::MAP_RTRIGER,   \
        UsbGamepadSaturnControllerObserver::ButtonMap::MAP_KEY_UP,    \
        UsbGamepadSaturnControllerObserver::ButtonMap::MAP_KEY_DOWN,  \
        UsbGamepadSaturnControllerObserver::ButtonMap::MAP_KEY_LEFT,  \
        UsbGamepadSaturnControllerObserver::ButtonMap::MAP_KEY_RIGHT  \
  }

#define RAWKEY_UP (1 << 0)
#define RAWKEY_DOWN (1 << 1)
#define RAWKEY_LEFT (1 << 2)
#define RAWKEY_RIGHT (1 << 3)
#define RAWKEY_A (1 << (UsbGamepadSaturnControllerObserver::ButtonMap::MAP_KEY_A + 4))
#define RAWKEY_B (1 << (UsbGamepadSaturnControllerObserver::ButtonMap::MAP_KEY_B + 4))
#define RAWKEY_C (1 << (UsbGamepadSaturnControllerObserver::ButtonMap::MAP_KEY_C + 4))
#define RAWKEY_X (1 << (UsbGamepadSaturnControllerObserver::ButtonMap::MAP_KEY_X + 4))
#define RAWKEY_Y (1 << (UsbGamepadSaturnControllerObserver::ButtonMap::MAP_KEY_Y + 4))
#define RAWKEY_Z (1 << (UsbGamepadSaturnControllerObserver::ButtonMap::MAP_KEY_Z + 4))
#define RAWKEY_L (1 << (UsbGamepadSaturnControllerObserver::ButtonMap::MAP_KEY_L + 4))
#define RAWKEY_R (1 << (UsbGamepadSaturnControllerObserver::ButtonMap::MAP_KEY_R + 4))
#define RAWKEY_ST (1 << (UsbGamepadSaturnControllerObserver::ButtonMap::MAP_KEY_START + 4))

#endif // __CONFIGURATION_H__
