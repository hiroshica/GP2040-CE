#include "addons/saturninterface.h"
#include "storagemanager.h"
#include "helper.h"
#include "config.pb.h"
#include "SaturnReader/SaturnController.h"

// GPIO PIN number
#define SATURN_CONTROLLER_1_PIN_1 2
#define SATURN_CONTROLLER_1_PIN_2 3
#define SATURN_CONTROLLER_1_PIN_3 4
#define SATURN_CONTROLLER_1_PIN_4 5
#define SATURN_CONTROLLER_1_PIN_5 6
#define SATURN_CONTROLLER_1_PIN_6 7
#define SATURN_CONTROLLER_1_PIN_7 8

SaturnController *m_SaturnController;

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
    m_SaturnController = new SaturnController(0,
                                              SATURN_CONTROLLER_1_PIN_1, // D3
                                              SATURN_CONTROLLER_1_PIN_2, // D2
                                              SATURN_CONTROLLER_1_PIN_3, // D1
                                              SATURN_CONTROLLER_1_PIN_4, // D0
                                              SATURN_CONTROLLER_1_PIN_5, // TH
                                              SATURN_CONTROLLER_1_PIN_6, // TR
                                              SATURN_CONTROLLER_1_PIN_7  // TL
    );
    m_SaturnController->pinInit();
}
//-------------------------------------------------------------------------------------------------------------------
void SaturnInterfaceInput::process()
{
    m_SaturnController->Update(true);
    Gamepad *gamepad = Storage::getInstance().GetGamepad();
    for (int iI = 0; iI < KEY_DIGITALMAX; iI++)
    {
        uint32_t keycheck[KEY_DIGITALMAX][3] = {
            // readdata index, button assign bit
            {4, SATURN_READ_KEY_A, GAMEPAD_MASK_B1},
            {4, SATURN_READ_KEY_B, GAMEPAD_MASK_B2},
            {4, SATURN_READ_KEY_C, GAMEPAD_MASK_L2},
            {4, SATURN_READ_KEY_X, GAMEPAD_MASK_B3},
            {4, SATURN_READ_KEY_Y, GAMEPAD_MASK_B4},
            {4, SATURN_READ_KEY_Z, GAMEPAD_MASK_R2},
            {4, SATURN_READ_KEY_L, GAMEPAD_MASK_L1},
            {4, SATURN_READ_KEY_R, GAMEPAD_MASK_R1},
            {5, SATURN_READ_KEY_START, GAMEPAD_MASK_A1},
        };

        uint32_t getstate = m_SaturnController->keyStatus[keycheck[iI][0]] & keycheck[iI][1];
        if (getstate)
        {
            gamepad->state.buttons |= keycheck[iI][2];
        }
    }
    SaturnController::SaturnID getID = m_SaturnController->m_GetId;
    if (getID == SaturnController::SaturnID::SaturnIDDigitalPad || getID == SaturnController::SaturnID::SaturnIDMulCon || getID == SaturnController::SaturnID::SaturnIDMulConDigital)
    {
        // ミッションスティックが勝手に作り出しているものを無視するためにIDでデジタル方向データを取得
        gamepad->state.dpad |= (m_SaturnController->keyStatus[5] & SATURN_READ_KEY_UP) ? GAMEPAD_MASK_UP : 0;
        gamepad->state.dpad |= (m_SaturnController->keyStatus[5] & SATURN_READ_KEY_DOWN) ? GAMEPAD_MASK_DOWN : 0;
        gamepad->state.dpad |= (m_SaturnController->keyStatus[5] & SATURN_READ_KEY_LEFT) ? GAMEPAD_MASK_LEFT : 0;
        gamepad->state.dpad |= (m_SaturnController->keyStatus[5] & SATURN_READ_KEY_RIGHT) ? GAMEPAD_MASK_RIGHT : 0;
    }
}
//-------------------------------------------------------------------------------------------------------------------
void SaturnInterfaceInput::preprocess()
{
}