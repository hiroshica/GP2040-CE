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

#define ANALOG_CENTER 0.5f    // 0.5f is center
#define ANALOG_DEADZONE 0.05f // move to config (future release)

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
    uIntervalMS = 0;
    nextTimer = getMillis() + uIntervalMS;
}
//-------------------------------------------------------------------------------------------------------------------
void SaturnInterfaceInput::process()
{
    //if (nextTimer < getMillis())
    {
        m_SaturnController->Update(true);
        nextTimer = getMillis() + uIntervalMS;
    }

    SaturnController::SaturnID getID = m_SaturnController->m_GetId;
    Gamepad *gamepad = Storage::getInstance().GetGamepad();
    if (getID > 0)
    {
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

        gamepad->state.lx = ((uint16_t)m_SaturnController->keyStatus[0]) << 8;
        gamepad->state.ly = ((uint16_t)m_SaturnController->keyStatus[1]) << 8;
        gamepad->state.lt = m_SaturnController->keyStatus[2];
        gamepad->state.rt = m_SaturnController->keyStatus[3];

        if (getID == SaturnController::SaturnID::SaturnIDDigitalPad || getID == SaturnController::SaturnID::SaturnIDMulCon || getID == SaturnController::SaturnID::SaturnIDMulConDigital)
        {
            // ミッションスティックが勝手に作り出しているものを無視するためにIDでデジタル方向データを取得
            gamepad->state.dpad |= (m_SaturnController->keyStatus[5] & SATURN_READ_KEY_UP) ? GAMEPAD_MASK_UP : 0;
            gamepad->state.dpad |= (m_SaturnController->keyStatus[5] & SATURN_READ_KEY_DOWN) ? GAMEPAD_MASK_DOWN : 0;
            gamepad->state.dpad |= (m_SaturnController->keyStatus[5] & SATURN_READ_KEY_LEFT) ? GAMEPAD_MASK_LEFT : 0;
            gamepad->state.dpad |= (m_SaturnController->keyStatus[5] & SATURN_READ_KEY_RIGHT) ? GAMEPAD_MASK_RIGHT : 0;
        }
    }
}
//-------------------------------------------------------------------------------------------------------------------
void SaturnInterfaceInput::preprocess()
{
}