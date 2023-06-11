#include "addons/saturninterface.h"
#include "storagemanager.h"
#include "helper.h"
#include "config.pb.h"

//-------------------------------------------------------------------------------------------------------------------
// このinput deviceが存在するか
//-------------------------------------------------------------------------------------------------------------------
bool SaturnInterfaceInput::available()
{
    return true;
}
//-------------------------------------------------------------------------------------------------------------------
void SaturnInterfaceInput::setup()
{
}
//-------------------------------------------------------------------------------------------------------------------
void SaturnInterfaceInput::process()
{
    Gamepad *gamepad = Storage::getInstance().GetGamepad();
    // gamepad->stateの中を置き換えれば本体につたわるみたい
#if 0
    switch ((testwork / 200) % 4)
    {
    case 0:
        gamepad->state.buttons |= GAMEPAD_MASK_B1;
        break;
    case 1:
        gamepad->state.buttons |= GAMEPAD_MASK_B2;
        break;
    case 2:
        gamepad->state.buttons |= GAMEPAD_MASK_B3;
        break;
    case 3:
        gamepad->state.buttons |= GAMEPAD_MASK_B4;
        break;
    }
#endif
    testwork++;
}
//-------------------------------------------------------------------------------------------------------------------
void SaturnInterfaceInput::preprocess()
{
}