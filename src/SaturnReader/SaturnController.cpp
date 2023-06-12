#include "SaturnReader/SaturnController.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

extern bool drawupdatef;

SaturnController::SaturnController(uint8_t id,
								   uint8_t d3,
								   uint8_t d2,
								   uint8_t d1,
								   uint8_t d0,
								   uint8_t th,
								   uint8_t tr,
								   uint8_t tl) : id(id),
												 inputPins{tl, d3, d2, d1, d0},
												 inputMasks{1UL << tl, 1UL << d3, 1UL << d2, 1UL << d1, 1UL << d0},
												 allInputsMask(1UL << tl | 1UL << d3 | 1UL << d2 | 1UL << d1 | 1UL << d0),
												 selectPin{th, tr},
												 allOutputsMask(1UL << th | 1UL << tr),
												 keyStatus(),
												 controllerSensed(false),
												 connectingCount(0)
{
}

void SaturnController::pinInit()
{
	// Initialize all GPIO
	uint32_t allGpioMask = allInputsMask | allOutputsMask;
	gpio_init_mask(allGpioMask);
	gpio_set_dir_masked(allGpioMask, allOutputsMask);
	// Set all inputs as pull up
	const uint8_t *pPin = inputPins;
	for (uint32_t i = INPUT_PIN_COUNT; i > 0; --i, ++pPin)
	{
		// Assume external pull resistors
		gpio_pull_up(*pPin);
	}
	m_GetId = SaturnID::SaturnIDNone;
	THR_SET(true, false);
}

void SaturnController::Update(int32_t onoffsetf)
{
	uint32_t tmp1, tmp2;

	g_mouse_detected = 0;

	THR_SET(true, true);
	delay_us(4);
	tmp1 = getDat();
	THR_SET(true, false);
	delay_us(4);
	uint8_t mm_check = waitTL(200, 0);

	tmp2 = getDat();

	if ((tmp1 & 0x07) == 0x4)
	{
		// digital pad
		m_GetId = SaturnID::SaturnIDDigitalPad;
		m_ErrorIDCount = 0;
		idleMouse();
		saturnReadPad();
	}
	else if (((tmp1 & 0x0f) == 0x01) && ((tmp2 & 0x0f) == 0x01))
	{
		m_ErrorIDCount = 0;
		idleMouse();
		AnalogRetcode ret = saturnReadAnalog();
		if (AnalogRetcode::Analog_Mission == ret)
		{
			m_GetId = SaturnID::SaturnIDAnalog;
		}
		else if (AnalogRetcode::Analog_Mulcon == ret)
		{
			m_GetId = SaturnID::SaturnIDMulCon;
		}
		else if (AnalogRetcode::Digital_MulCon == ret)
		{
			m_GetId = SaturnID::SaturnIDMulConDigital;
		}
	}
	else
	{
		// unknown ID
		// default idle
		idleJoystick();
		idleMouse();
		m_GetId = SaturnID::SaturnIDNone;
	}
	processSignals(onoffsetf);
	THR_SET(true, false);
}

void SaturnController::processSignals(int32_t onoffsetf)
{
	// 以前はUSBがつながっていなかったらPAD Dataを作成していなかったが
	// 1ms単位で常に作るようにする
	{
		// Get next button state
		uint32_t active_size;
		active_size = saturnBuildReport(keyStatus);
#if 0
		// 以下はdirect dataの内訳
		if (m_GetId > 0)
		{
			if (m_GetId == SaturnID::SaturnIDDigitalPad || m_GetId == SaturnID::SaturnIDMulCon || m_GetId == SaturnID::SaturnIDMulConDigital)
			{
				// ミッションスティックが勝手に作り出しているので修正
				updateKey(ISaturnControllerObserver::KEY_UP, readdata[5] & SATURN_READ_KEY_UP);
				updateKey(ISaturnControllerObserver::KEY_DOWN, readdata[5] & SATURN_READ_KEY_DOWN);
				updateKey(ISaturnControllerObserver::KEY_LEFT, readdata[5] & SATURN_READ_KEY_LEFT);
				updateKey(ISaturnControllerObserver::KEY_RIGHT, readdata[5] & SATURN_READ_KEY_RIGHT);
			}
			updateKey(ISaturnControllerObserver::KEY_LANALOG_X, readdata[0]);
			updateKey(ISaturnControllerObserver::KEY_LANALOG_Y, readdata[1]);
			updateKey(ISaturnControllerObserver::KEY_LTRIGER, readdata[2]);
			updateKey(ISaturnControllerObserver::KEY_RTRIGER, readdata[3]);

			for (int iI = 0; iI < ISaturnControllerObserver::Key::KEY_DIGITALMAX; iI++)
			{
				uint32_t keycheck[ISaturnControllerObserver::Key::KEY_DIGITALMAX][2] = {
					// readdata index, button assign bit
					{4, SATURN_READ_KEY_A},
					{4, SATURN_READ_KEY_B},
					{4, SATURN_READ_KEY_C},
					{4, SATURN_READ_KEY_X},
					{4, SATURN_READ_KEY_Y},
					{4, SATURN_READ_KEY_Z},
					{4, SATURN_READ_KEY_L},
					{4, SATURN_READ_KEY_R},
					{5, SATURN_READ_KEY_START},
				};
				bool getstate = readdata[keycheck[iI][0]] & keycheck[iI][1];
				// 連射変更ないキー保存
				updateKeyRaw((ISaturnControllerObserver::Key)iI, getstate);
				// 連射キー作成
				getstate = m_RapidFire[iI].update(getstate);
				updateKey((ISaturnControllerObserver::Key)iI, getstate);
			}
		}
#endif
	}
}

void SaturnController::THR_SET(bool thflag, bool trflag)
{
	gpio_put(selectPin[SaturnController::SelectPin::eTH_PIN], thflag);
	gpio_put(selectPin[SaturnController::SelectPin::eTR_PIN], trflag);
}

void SaturnController::TR_HIGH()
{
	gpio_put(selectPin[SaturnController::SelectPin::eTR_PIN], true);
}
void SaturnController::TR_LOW()
{
	gpio_put(selectPin[SaturnController::SelectPin::eTR_PIN], false);
}
void SaturnController::TH_HIGH()
{
	gpio_put(selectPin[SaturnController::SelectPin::eTH_PIN], true);
}
void SaturnController::TH_LOW()
{
	gpio_put(selectPin[SaturnController::SelectPin::eTH_PIN], false);
}

uint32_t SaturnController::getDat()
{
	// uint32_t inputs = gpio_get_all();
	uint32_t retcode = 0;
	for (uint32_t iI = 0; iI < INPUT_PIN_COUNT; iI++)
	{
		uint32_t index = INPUT_PIN_COUNT - iI - 1;
		// if((inputs&inputMasks[INPUT_PIN_COUNT-iI-1]))
		if (gpio_get(inputPins[index]))
		{
			retcode |= 1UL << iI;
		}
	}
	return retcode;
}

void SaturnController::delay_us(uint32_t inwait)
{
	absolute_time_t nextTime;
	update_us_since_boot(&nextTime, time_us_64());
	update_us_since_boot(&nextTime, to_us_since_boot(nextTime) + inwait);
	busy_wait_until(nextTime);
}

uint8_t SaturnController::waitTL(uint32_t inwait, uint8_t state)
{
	uint8_t t_out = inwait;
	if (state)
	{
		// 1 =　立ち下り検知
		while (!(getDat() & 0x10))
		{
			delay_us(1);
			t_out--;
			if (!t_out)
				return -1;
		}
	}
	else
	{
		// 0 =　立ち上がり検知
		while ((getDat() & 0x10))
		{
			delay_us(1);
			t_out--;
			if (!t_out)
				return -1;
		}
	}
	return 0;
}

void SaturnController::idleJoystick(void)
{
	unsigned char *joy_report = last_built_report[JOYSTICK_REPORT_IDX];
	joy_report[0] = 0;
	joy_report[1] = 0;
	joy_report[2] = 0;
	joy_report[3] = 0;
	joy_report[4] = 0;
	joy_report[5] = 0;
}

void SaturnController::idleMouse(void)
{
	unsigned char *mouse_report = last_built_report[MOUSE_REPORT_IDX];
	memset(mouse_report, 0, MAX_REPORT_SIZE);
}

void SaturnController::permuteButtons(void)
{
	uint32_t buttons_in, buttons_out;
	uint8_t *joy_report = last_built_report[JOYSTICK_REPORT_IDX];
	int i;

	/* Saturn		:   A  B  C  X  Y  Z  S  L  R */
	uint8_t sls[9] = {1, 2, 5, 0, 3, 4, 9, 6, 7};
	uint8_t sls_alt[9] = {1, 2, 5, 0, 3, 4, 8, 6, 7};
	uint8_t vip[9] = {0, 1, 2, 3, 4, 5, 8, 6, 7};
	uint8_t *permuter;

	buttons_in = joy_report[4];
	buttons_in |= joy_report[5] << 8;

	/* Only run once. Hold A or B at power-up to select mappings. */
	if (current_mapping == MAPPING_UNDEFINED)
	{

		current_mapping = MAPPING_SLS; // default.

		if (buttons_in & 0x01) // A
			current_mapping = MAPPING_SLS_ALT;
		if (buttons_in & 0x02) // B
			current_mapping = MAPPING_VIP;
		if (buttons_in & 0x04) // C
			current_mapping = MAPPING_IDENTITY;
	}

	switch (current_mapping)
	{
	default:
	case MAPPING_IDENTITY:
		return;

	case MAPPING_SLS:
		permuter = sls;
		break;

	case MAPPING_SLS_ALT:
		permuter = sls_alt;
		break;

	case MAPPING_VIP:
		permuter = vip;
		break;
	}

	// Let the analog pad D-Pad buttons
	// pass through by masking only those
	// handled here.
	buttons_out = buttons_in;
	buttons_out &= ~0x1FF;

	for (i = 0; i < 9; i++)
	{
		if (buttons_in & (1 << i))
		{
			buttons_out |= (1 << permuter[i]);
		}
	}

	joy_report[4] = buttons_out;
	joy_report[5] = buttons_out >> 8;
}

SaturnController::AnalogRetcode SaturnController::saturnReadAnalog(void)
{
	AnalogRetcode retcode = AnalogRetcode::Analog_Mission;
	uint32_t dat[32];
	int32_t i;
	uint8_t tr = 0;
	uint8_t r;
	int32_t nibbles = 14;
	uint8_t *joy_report = last_built_report[JOYSTICK_REPORT_IDX];

	delay_us(2);
	TH_LOW();
	delay_us(2);

	for (i = 0; i < nibbles; i++)
	{
		if (tr)
		{
			TR_HIGH();
			delay_us(2);
			r = waitTL(200, 1);
			if (r)
			{
				retcode = AnalogRetcode::Error;
				goto Error;
			}
		}
		else
		{
			TR_LOW();
			delay_us(2);
			r = waitTL(200, 0);
			if (r)
			{
				retcode = AnalogRetcode::Error;
				goto Error;
			}
		}

		delay_us(2);

		dumpdata[i] = dat[i] = getDat();

		tr ^= 1;

		if (i >= 1)
		{
			if (dat[1] == 0x12)
			{
				nibbles = 8;
			}
		}
	}

	if (dat[1] == 0x15)
	{
		retcode = AnalogRetcode::Analog_Mission;
	}
	else if (dat[1] == 0x12)
	{
		retcode = AnalogRetcode::Digital_MulCon;
	}
	else if (dat[1] == 0x16)
	{
		retcode = AnalogRetcode::Analog_Mulcon;
	}

	idleJoystick();

	if (!(dat[3] & 0x04)) // A
		joy_report[4] |= SATURN_READ_KEY_A;
	if (!(dat[3] & 0x01)) // B
		joy_report[4] |= SATURN_READ_KEY_B;
	if (!(dat[3] & 0x02)) // C
		joy_report[4] |= SATURN_READ_KEY_C;

	if (!(dat[4] & 0x04)) // X
		joy_report[4] |= SATURN_READ_KEY_X;
	if (!(dat[4] & 0x02)) // Y
		joy_report[4] |= SATURN_READ_KEY_Y;
	if (!(dat[4] & 0x01)) // Z
		joy_report[4] |= SATURN_READ_KEY_Z;

	if (!(dat[3] & 0x08)) // Start
		joy_report[5] |= SATURN_READ_KEY_START;

	if (!(dat[5] & 0x08)) // L
		joy_report[4] |= SATURN_READ_KEY_L;
	if (!(dat[4] & 0x08)) // R
		joy_report[4] |= SATURN_READ_KEY_R;

	if (retcode == AnalogRetcode::Digital_MulCon)
	{
		// switch is in the "+" position
		if (!(dat[2] & 0x08)) // right
		{
			// joy_report[0] = 0x7f;
			joy_report[5] |= SATURN_READ_KEY_RIGHT;
		}
		if (!(dat[2] & 0x04)) // left
		{
			// joy_report[0] = 0x80;
			joy_report[5] |= SATURN_READ_KEY_LEFT;
		}
		if (!(dat[2] & 0x02)) // down
		{
			// joy_report[1] = 0x7f;
			joy_report[5] |= SATURN_READ_KEY_DOWN;
		}
		if (!(dat[2] & 0x01)) // Up
		{
			// joy_report[1] = 0x80;
			joy_report[5] |= SATURN_READ_KEY_UP;
		}
	}
	else
	{
		if (!(dat[2] & 0x08)) // Right
			joy_report[5] |= SATURN_READ_KEY_RIGHT;
		if (!(dat[2] & 0x04)) // Left
			joy_report[5] |= SATURN_READ_KEY_LEFT;
		if (!(dat[2] & 0x02)) // Down
			joy_report[5] |= SATURN_READ_KEY_DOWN;
		if (!(dat[2] & 0x01)) // Up
			joy_report[5] |= SATURN_READ_KEY_UP;

		// switch is in the "o" position
		joy_report[0] = ConvAnaglog(((dat[7] & 0xf) | (dat[6] << 4)));
		joy_report[1] = ConvAnaglog(((dat[9] & 0xf) | (dat[8] << 4)));

		joy_report[2] = ConvAnaglog(((dat[13] & 0xf) | (dat[12] << 4)));
		joy_report[3] = ConvAnaglog(((dat[11] & 0xf) | (dat[10] << 4)));
	}

Error:
{
	TR_HIGH();
	delay_us(4);
	TH_HIGH();
	delay_us(4);
}
	return retcode;
}
uint8_t SaturnController::ConvAnaglog(uint8_t indata)
{
#if 0
	uint8_t retdata = 0;
	if (indata <= (uint8_t)0x80)
	{
		retdata = -(0x80 - indata);
	}
	else
	{
		retdata = indata - 0x80;
	}
	return retdata;
#else
	return indata;
#endif
}

void SaturnController::saturnReadPad()
{
	uint32_t a, b, c, d;
	uint8_t *joy_report = last_built_report[JOYSTICK_REPORT_IDX];
	/*
	 TH and TR already high from detecting, read this
	 nibble first! Otherwise the HORIPAD SS (HSS-11) does
	 not work! The Performance Super Pad 8 does though.
	*/
	//
	// d0 d1 d2 d3
	// 0  0  1  L
	TH_HIGH();
	TR_HIGH();
	delay_us(2);
	d = getDat();

	// d0 d1 d2 d3
	// Z  Y  X  R
	TH_LOW();
	TR_LOW();
	delay_us(2);
	a = getDat();

	// d0 d1 d2 d3
	// B  C  A  St
	TH_HIGH();
	TR_LOW();
	delay_us(2);
	b = getDat();

	// d0 d1 d2 d3
	// UP DN LT RT
	TH_LOW();
	TR_HIGH();
	delay_us(2);
	c = getDat();

	idleJoystick();

	dumpdata[0] = a;
	dumpdata[1] = b;
	dumpdata[2] = c;
	dumpdata[3] = d;

	if (!(c & 0x08)) // right
	{
		// joy_report[0] = 0x7f;
		joy_report[5] |= SATURN_READ_KEY_RIGHT;
	}
	if (!(c & 0x04)) // left
	{
		// joy_report[0] = 0x80;
		joy_report[5] |= SATURN_READ_KEY_LEFT;
	}
	if (!(c & 0x02)) // down
	{
		// joy_report[1] = 0x7f;
		joy_report[5] |= SATURN_READ_KEY_DOWN;
	}
	if (!(c & 0x01)) // Up
	{
		// joy_report[1] = 0x80;
		joy_report[5] |= SATURN_READ_KEY_UP;
	}

	if (!(b & 0x04)) // A
	{
		joy_report[4] |= SATURN_READ_KEY_A;
	}
	if (!(b & 0x01)) // B
	{
		joy_report[4] |= SATURN_READ_KEY_B;
	}
	if (!(b & 0x02)) // C
	{
		joy_report[4] |= SATURN_READ_KEY_C;
	}
	if (!(a & 0x04)) // X
	{
		joy_report[4] |= SATURN_READ_KEY_X;
	}
	if (!(a & 0x02)) // Y
	{
		joy_report[4] |= SATURN_READ_KEY_Y;
	}
	if (!(a & 0x01)) // Z
	{
		joy_report[4] |= SATURN_READ_KEY_Z;
	}

	if (!(b & 0x08)) // Start
	{
		joy_report[5] |= SATURN_READ_KEY_START;
	}
	if (!(d & 0x08)) // L
	{
		joy_report[4] |= SATURN_READ_KEY_L;
	}
	if (!(a & 0x08)) // R
	{
		joy_report[4] |= SATURN_READ_KEY_R;
	}
}

uint8_t SaturnController::saturnBuildReport(uint8_t *reportBuffer)
{
	uint8_t report_id;
	if (g_mouse_mode)
	{
		report_id = MOUSE_REPORT_IDX;
	}
	else
	{
		report_id = JOYSTICK_REPORT_IDX;
	}

	if (reportBuffer != NULL)
	{
		memcpy(reportBuffer, last_built_report[report_id], report_sizes[report_id]);
	}
	memcpy(last_sent_report[report_id], last_built_report[report_id],
		   report_sizes[report_id]);

	return report_sizes[report_id];
}
