#ifndef __SATURN_CONTROLLER_H__
#define __SATURN_CONTROLLER_H__

#include <stdint.h>
#include "pico/stdlib.h"
// #include "Debug.h"

class SaturnController
{
public:
  // saturen IDs
  enum SaturnID
  {
    SaturnIDError = -2,
    SaturnIDNone = -1,
    SaturnIDDigitalPad = 1,
    SaturnIDAnalog,
    SaturnIDMulCon,
    SaturnIDMulConDigital,
    SaturnIDMouse,
    SaturnIDMax
  };

protected:
  //! All of the available input pins
  enum InputPinSelect
  {
    INPUT_PIN_0 = 0, // d0
    INPUT_PIN_1,     // d1
    INPUT_PIN_2,     // d2
    INPUT_PIN_3,     // d3
    INPUT_PIN_4,     // TL
    INPUT_PIN_COUNT
  };
  enum SelectPin
  {
    eTH_PIN = 0,
    eTR_PIN
  };

// saturn read status
#define MAPPING_UNDEFINED 0
#define MAPPING_SLS 1
#define MAPPING_SLS_ALT 2 // Saturn start -> PS3 select
#define MAPPING_VIP 3
#define MAPPING_TEST 4
#define MAPPING_IDENTITY 5

#define MAX_REPORT_SIZE 6
#define NUM_REPORTS 2

#define JOYSTICK_REPORT_IDX 0
#define JOYSTICK_REPORT_SIZE 6
#define MOUSE_REPORT_IDX 1
#define MOUSE_REPORT_SIZE 3
// saturn read status

// data read size
#define MAX_REPORT_SIZE 6
#define NUM_REPORTS 2

// keyStatus index
// 0 : X
// 1 : Y
// 2 : LT
// 3 : RT
// 4
#define SATURN_READ_KEY_A (1UL << 0)
#define SATURN_READ_KEY_B (1UL << 1)
#define SATURN_READ_KEY_C (1UL << 2)
#define SATURN_READ_KEY_X (1UL << 3)
#define SATURN_READ_KEY_Y (1UL << 4)
#define SATURN_READ_KEY_Z (1UL << 5)
#define SATURN_READ_KEY_L (1UL << 6)
#define SATURN_READ_KEY_R (1UL << 7)
// 5
#define SATURN_READ_KEY_START (1UL << 0)
#define SATURN_READ_KEY_UP (1UL << 7)
#define SATURN_READ_KEY_DOWN (1UL << 6)
#define SATURN_READ_KEY_LEFT (1UL << 5)
#define SATURN_READ_KEY_RIGHT (1UL << 4)

#define KEY_DIGITALMAX  (3+3+2+1)

  enum AnalogRetcode
  {
    Error = -1,
    Analog_Mission = 0,
    Analog_Mulcon,
    Digital_MulCon,
  };

public:
  //! SaturnController constructor
  //! @param[in] id  The controller ID for this controller
  //! @param[in] pin1  GPIO  (in) d3
  //! @param[in] pin2  GPIO  (in) d2
  //! @param[in] pin3  GPIO  (in) d1
  //! @param[in] pin4  GPIO  (in) d0
  //! @param[in] pin6  GPIO  (in) tl
  //! @param[in] pin7  GPIO (out) th
  //! @param[in] pin9  GPIO (out) tr
  //! @param[in] pObserver  The observer to update whenever keys are pressed and released
  SaturnController(uint8_t id,
                   uint8_t d3,
                   uint8_t d2,
                   uint8_t d1,
                   uint8_t d0,
                   uint8_t th,
                   uint8_t tr,
                   uint8_t tl);
  //! Initializes all of my pins
  void pinInit();

  void Update(int32_t onoffsetf);
  // TRH_SelectMode nextSignal(uint8_t count, uint32_t inputs);
protected:
private:
  //! Default constructor not defined or used
  SaturnController();
  void processSignals(int32_t onoffsetf); // Saturnを読み込んだらUSBの確認を行う

  void THR_SET(bool thflag, bool trflag);
  void TR_HIGH();
  void TR_LOW();
  void TH_HIGH();
  void TH_LOW();
  uint32_t getDat();

  void delay_us(uint32_t inwait);
  uint8_t waitTL(uint32_t inwait, uint8_t state);
  void idleJoystick(void);
  void idleMouse(void);
  void permuteButtons();

  SaturnController::AnalogRetcode saturnReadAnalog(void);
  void saturnReadPad();

  uint8_t saturnBuildReport(uint8_t *reportBuffer);
  uint8_t ConvAnaglog(uint8_t indata);

public:
  // static const uint8_t NUM_COUNTS = 2;

private:
#if 0
  //! Set to true to invert inputs
  //! @note This assumes external pull-down resistor used when false or pull-up when true
  static const bool INVERT_INPUTS = false;
  //! Set to true to invert outputs
  static const bool INVERT_OUTPUTS = true; // An NPN is used which inverts the output
  //! Number of connecting cycles to make before becomming connected
  static const uint8_t MAX_CONNECTING_COUNT = 25;
#endif

  // hardware works (pico side)
  //! The ID for this controller
  const uint8_t id;
  //! GPIO indicies for input pins
  const uint8_t inputPins[INPUT_PIN_COUNT];
  //! GPIO index for select pin (7)
  const uint8_t selectPin[2];
  //! GPIO masks for input pins
  const uint32_t inputMasks[INPUT_PIN_COUNT];
  //! Mask which represents all inputs
  const uint32_t allInputsMask;
  //! Mask which represents all outputs
  const uint32_t allOutputsMask;
  //! The current controller (USB)state
  // State controllerState;
  //! Set to true if controller is sensed; false otherwise
  bool controllerSensed;
  //! Set to true if controller is six button; false otherwise
  bool isSixButton;
  //! The count kept while connecting
  uint8_t connectingCount;
  static const uint8_t currentPort = 1;

  // hardware work (saturn side)
  uint8_t dumpdata[0x20];
#define ERRORCOUNT_WAIT (60)
  uint32_t m_ErrorIDCount;

  uint8_t last_built_report[NUM_REPORTS][MAX_REPORT_SIZE]; // 　最後に取得したデータ保存エリア
  uint8_t last_sent_report[NUM_REPORTS][MAX_REPORT_SIZE];  //  最後に送り出したデータ
  uint8_t report_sizes[NUM_REPORTS] = {JOYSTICK_REPORT_SIZE, MOUSE_REPORT_SIZE};
  uint8_t g_mouse_detected = 0;
  uint8_t g_mouse_mode = 0;
  uint8_t current_mapping = MAPPING_UNDEFINED;

public:
  // Get ID
  SaturnID m_GetId;
  SaturnID m_OldGetId;
  //! Current status of all keys (true when pressed)
  uint8_t keyStatus[MAX_REPORT_SIZE]; // direct data
};

#endif // __SATURN_CONTROLLER_H__
