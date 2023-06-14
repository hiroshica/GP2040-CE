/* -*- mode: c++; coding: utf-8-dos; tab-width: 4; -*- */
/* ================================================================================================== */
/*!
 * @file drawfont.cpp
 * @brief drawfont.cpp
 *
 * bitmapフォントデータ描画&表示callbackルーチン
 * 19/11/2021 : 汎用化に向けてリビルド開始
 *
 * @author hiromitsu-s
 * @date 2010-05-11 15:40:11
 */
/* ================================================================================================== */
#define __SsFontDraw_CPP__

/* #INCLUDE ========================================================================================= */

#include "font_draw/font_draw.h"
#include "font_draw/UTF8SJIS.h"

/* extern font data */
extern uint8_t font8[];
extern uint8_t font10[];
extern uint8_t systemfont[];

namespace SsLib
{
	namespace Ut
	{
		/* #DEFINE ========================================================================================== */
		/* #PROTO  ========================================================================================== */
		uint32_t SsSjis2Towns(uint32_t i);
#define CONVBUF_SIZE (40 * 2)
		uint8_t convertbuffer[CONVBUF_SIZE + 8];
		/* #STATIC ========================================================================================== */
		/********************************************************/
		/*    sjis --> Towns font code                          */
		/********************************************************/
		uint32_t SsSjis2Towns(uint32_t i)
		{
			uint32_t high, low;
			high = (i & 0xff00) >> 8;
			low = i & 0x00ff;
			/**** begin ****/
			if (low > 0x007f && low < 0xff)
			{
				i -= 0x0001;
			}
			i -= (high - 0x81) * 0x0044;
			return i;
		}
		/**
		 *
		 */
		SsFontDraw::STRING_FUNC SsFontDraw::mStringFuncTable[eDRAWFONT_MAX] = {
			&SsFontDraw::string8,
			&SsFontDraw::string10,
			&SsFontDraw::stringT12,
			&SsFontDraw::stringT12,
		};
		/**
		 *
		 */
		SsFontDraw::FONT_FUNC SsFontDraw::mFontFuncTable[eDRAWFONT_MAX] = {
			&SsFontDraw::putc8,
			&SsFontDraw::putc10,
			&SsFontDraw::putcT12,
			&SsFontDraw::putcT12,
		};
		/**
		 *
		 */
		SsFontDraw::DRAW_FUNC SsFontDraw::mDrawFuncTable[eDRAWMODE_MAX] = {
			&SsFontDraw::drawDotUserCallback,
			&SsFontDraw::drawDotUserCallback,
			&SsFontDraw::drawDotUserCallback,
		};
		/**
		 *
		 */
		static tFontRecord sDefaultFontTable[eDRAWFONT_MAX] = {
			{false, 8, 8, 0, 0, font8},
			{false, 10, 10, 0, 0, font10},
			{true, 12, 12, 0, 0, systemfont},
			{true, 12, 12, 0, 0, systemfont},
		};
		/**
		 *
		 */
		static tSsColor sDefaultColor[] = {
			{0.00f, 0.00f, 0.00f, 1.00f}, /* 0x00 */
			{1.00f, 0.00f, 0.00f, 1.00f}, /* 0x01 */
			{0.00f, 1.00f, 0.00f, 1.00f}, /* 0x02 */
			{1.00f, 1.00f, 0.00f, 1.00f}, /* 0x03 */
			{0.00f, 0.00f, 1.00f, 1.00f}, /* 0x04 */
			{1.00f, 0.00f, 1.00f, 1.00f}, /* 0x05 */
			{0.00f, 1.00f, 1.00f, 1.00f}, /* 0x06 */
			{1.00f, 1.00f, 1.00f, 1.00f}, /* 0x07 */

			{0.25f, 0.25f, 0.25f, 1.00f}, /* 0x08 */
			{0.25f, 0.00f, 0.00f, 1.00f}, /* 0x09 */
			{0.00f, 0.25f, 0.00f, 1.00f}, /* 0x0a */
			{0.25f, 0.25f, 0.00f, 1.00f}, /* 0x0b */
			{0.00f, 0.00f, 0.25f, 1.00f}, /* 0x0c */
			{0.25f, 0.00f, 0.25f, 1.00f}, /* 0x0d */
			{0.00f, 0.25f, 0.25f, 1.00f}, /* 0x0e */
			{1.00f, 1.00f, 1.00f, 1.00f}, /* 0x0f */
		};

		/* #SRC    ========================================================================================== */
		/* ---------------------------------------------------------------------------
		 * init
		 ----------------------------------------------------------------------------*/
		/*!
		 *
		 */
		bool SsFontDraw::Init(void)
		{
			return true;
		}
		/* ---------------------------------------------------------------------------
		 * init
		 ----------------------------------------------------------------------------*/
		/*!
		 *
		 */
		bool SsFontDraw::init(void)
		{
			int32_t iI, iJ;

			mWidth = 512.0f;
			mHeight = 512.0f;
			initLocate();
			mTab = 4;

			/* setup default font */
			for (iI = 0; iI < eDRAWFONT_MAX; ++iI)
			{
				mFont[iI] = sDefaultFontTable[iI];
			}
			mFontMode = eDRAWFONT_SYSTEM;
			mNowFont = &mFont[mFontMode];
			/**
			 * default paletteの設定
			 */
			for (iI = 0; iI < 0x10; ++iI)
			{
				int32_t index = iI * 0x10;
				for (iJ = 0; iJ < 0x10; ++iJ)
				{
					SetColor(index + iJ, sDefaultColor[iJ]);
				}
			}
			///* palette #0だけ抜き色にする */
			// mTextColor[0x00].mA = 0.0f;

			/* 初期パレット指定 */
			mFGColor = 0xff;
			mBGColor = 0x00;

			mDrawMode = eDRAWMODE_RGBA;
			return true;
		}
		/* ---------------------------------------------------------------------------
		 * remove
		 ----------------------------------------------------------------------------*/
		/*!
		 *
		 */
		void SsFontDraw::remove(void)
		{
			init();
		}
		/******************************************************************************************/
		/* ---------------------------------------------------------------------------
		 * インスタンスの生成
		 ----------------------------------------------------------------------------*/
		/*!
		 *
		 */
		bool SsFontDraw::Create(uint16_t inwidth, uint16_t inheight, eDRAWMODE drawmode, DrawCallRecord *inDrawCallRecord)
		{
			int32_t iI;
			init();
			/* setup mode */
			mWidth = inwidth;
			mHeight = inheight;
			mDrawMode = drawmode;
			m_DrawCallRecord = inDrawCallRecord;
			ClearFrameBuffer();
			return true;
		}
		/* ---------------------------------------------------------------------------
		 * インスタンスの解放（本当の解放はDestroyExec())
		 ----------------------------------------------------------------------------*/
		/*!
		 *
		 */
		bool SsFontDraw::Destroy(void)
		{
			return true;
		}
		/******************************************************************************************/
		/* ---------------------------------------------------------------------------
		 * clear frame buffer
		 ----------------------------------------------------------------------------*/
		/*!
		 *
		 */
		void SsFontDraw::ClearFrameBuffer(int32_t index)
		{
			// ここは後で考える
#if 0
	int32_t width  = (int32_t)mWidth;
	int32_t height = (int32_t)mHeight;
	int32_t x, y;
	int32_t rgbmode = mIsAlpha ? kFRAMEBUF_RGBA : kFRAMEBUF_RGB;
	
	if (-1 == index) {
		index = mBGColor;
	}
	tSsColor  *getColor = &mTextColor[index];
	uint8_t fillColor[4];
	fillColor[0] = (uint8_t)((float)0x000000ff * getColor->mR);
	fillColor[1] = (uint8_t)((float)0x000000ff * getColor->mG);
	fillColor[2] = (uint8_t)((float)0x000000ff * getColor->mB);
	fillColor[3] = (uint8_t)((float)0x000000ff * getColor->mA);
	if (false == mIsAlpha) {
		for (y = 0; y < height; ++y) {
			uint8_t *buf = mFrameBuffer + (kWIDTH_ALIGN((int32_t)mWidth * kFRAMEBUF_RGB) * y);
#if 1
			for (x = 0; x < width; ++x) {
				*(buf + 0) = fillColor[0];
				*(buf + 1) = fillColor[1];
				*(buf + 2) = fillColor[2];
				buf += kFRAMEBUF_RGB;
			}
#else
			*(buf + 0) = fillColor[0];
			*(buf + 1) = fillColor[1];
			*(buf + 2) = fillColor[2];
			memmove(buf + kFRAMEBUF_RGB, buf, (width - 1) * kFRAMEBUF_RGB);
#endif
		}
	}
	else{
		for (y = 0; y < height; ++y) {
			uint8_t *buf = mFrameBuffer + (kWIDTH_ALIGN((int32_t)mWidth * kFRAMEBUF_RGBA) * y);
#if 1
			for (x = 0; x < width; ++x) {
				*(buf + 0) = fillColor[0];
				*(buf + 1) = fillColor[1];
				*(buf + 2) = fillColor[2];
				*(buf + 3) = fillColor[3];
				buf += kFRAMEBUF_RGBA;
			}
#else
			*(buf + 0) = fillColor[0];
			*(buf + 1) = fillColor[1];
			*(buf + 2) = fillColor[2];
			*(buf + 3) = fillColor[3];
			memmove(buf + kFRAMEBUF_RGBA, buf, (width - 1) * kFRAMEBUF_RGBA);
#endif
		}
	}
	/* locateも一緒に初期化する */
#endif

			resetLocate();
		}
		/* ---------------------------------------------------------------------------
		 * draw string
		 ----------------------------------------------------------------------------*/
		/*!
		 *
		 */
		int32_t SsFontDraw::DrawString(const char *mesg)
		{
			int32_t iI, gsf, cno;
			uint8_t *p;
			int32_t addoffset;
			int32_t FontY = (mNowFont->mFontY + mNowFont->mSpaceY) * m_Scale;

#if 0
	// debug 文字描画(多分タウンズ漢字フォント)
	for (iI = 0; iI < 42 * 36; ++iI) {
		cno = 0x140 + mDebugCount;
		mDebugCount++;
		addoffset = (this->*mFontFuncTable[mFontMode])(mNowFont->mFont, cno);
		addLocateCharX(addoffset);
	}
	return iI;
#endif
			// ここでUtf8->Sjisをする
			UTF8ToSJIS((uint8_t *)mesg, convertbuffer, CONVBUF_SIZE);
			// ここでUtf8->Sjisをする
			/**** Begin ****/
			gsf = 0;
			m_DrawCallRecord->m_DrawEnd(FontY);
			for (iI = 0, p = (uint8_t *)convertbuffer; *p; ++iI)
			{
				switch (*p)
				{
				case '\r':
					setLocate(mStockLocateX, mNowLocateY);
					m_DrawCallRecord->m_DrawEnd(FontY);
					break;
				case '\n':
					setLocate(mStockLocateX, mNowLocateY + FontY); /* 改行 */
					m_DrawCallRecord->m_DrawEnd(FontY);
					break;
				case '\t':
					addoffset = (mTab - ((int32_t)(mNowLocateX / FontY) % mTab));
					setLocate(mNowLocateX + addoffset, mNowLocateY);
					break;
#if 0
			/**
			 * !!ここのコマンド群はbasic_stringsにもかかわるので後ほど
			 */
			/**** set TAB colum size ****/
		case 0x01:    *p++;    tclm = *p;    break;
			/**** set Foreground color number ****/
		case 0x02:    *p++;    fgcl = *p;    break;
			/**** set Background color number ****/
		case 0x03:    *p++;    bgcl = *p;    break;
			/**** set Grad. color number ****/
		case 0x04:    *p++;    gscl = *p;    break;
			/**** set ATTRIBUTE flag ****/
		case 0x05:
			*p++;    mask = ((*p)<<8)&0xff00;    *p++;    mask |= (*p)&0x00ff;    txtattr |= mask;    break;
			/**** reset ATTRIBUTE flag ****/
		case 0x06:
			*p++;    mask = ((*p)<<8)&0xff00;    *p++;    mask |= (*p)&0x00ff;    mask = mask^0xffff;    txtattr &= mask;    break;
#endif
				default:
					cno = (this->*mStringFuncTable[mFontMode])(&p); /* 内部の関数で2byte codeの時は(P+1)される */
					addoffset = (this->*mFontFuncTable[mFontMode])(mNowFont, cno);
					if (addLocateCharX(addoffset))
					{
						m_DrawCallRecord->m_DrawEnd(FontY);
					}
					break;
				}
				p++;
			}
			mIsForceUpdate = true;
			return iI;
		}
		/* ---------------------------------------------------------------------------
		 * set color
		 ----------------------------------------------------------------------------*/
		/*!
		 *
		 */
		bool SsFontDraw::SetColor(int32_t index, tSsColor &color)
		{
			mTextColor[index] = color;
			return true;
		}
		/* ---------------------------------------------------------------------------
		 * change foreground color
		 ----------------------------------------------------------------------------*/
		/*!
		 *
		 */
		bool SsFontDraw::ChangeForegroundColor(int32_t index)
		{
			mFGColor = index;
			return true;
		}
		/* ---------------------------------------------------------------------------
		 * change background color
		 ----------------------------------------------------------------------------*/
		/*!
		 *
		 */
		bool SsFontDraw::ChangeBackgroundColor(int32_t index)
		{
			mBGColor = index;
			return true;
		}
		/* ---------------------------------------------------------------------------
		 * change locate
		 ----------------------------------------------------------------------------*/
		/*!
		 *
		 */
		void SsFontDraw::ChangeLocate(int32_t x, int32_t y)
		{
			mStockLocateX = x;
			mStockLocateY = y;
			setLocate(mStockLocateX, mStockLocateY);
		}
		/* ---------------------------------------------------------------------------
		 * change chara locate
		 ----------------------------------------------------------------------------*/
		/*!
		 *
		 */
		void SsFontDraw::ChangeCharLocate(int32_t x, int32_t y)
		{
			int locateX = mNowFont->mFontX;
			if (mNowFont->mHalfCalcMode)
			{
				locateX >>= 1;
			}
			mStockLocateX = (int32_t)(x * locateX * m_Scale);
			mStockLocateY = (int32_t)(y * mNowFont->mFontY * m_Scale);
			setLocate(mStockLocateX, mStockLocateY);
		}
		/* ---------------------------------------------------------------------------
		 * change chara locate
		 ----------------------------------------------------------------------------*/
		/*!
		 *
		 */
		void SsFontDraw::ChangeFullCharLocate(int32_t x, int32_t y)
		{
			mStockLocateX = (int32_t)(x * mNowFont->mFontX * m_Scale);
			mStockLocateY = (int32_t)(y * mNowFont->mFontY * m_Scale);
			setLocate(mStockLocateX, mStockLocateY);
		}
		/* ---------------------------------------------------------------------------
		 * change font
		 ----------------------------------------------------------------------------*/
		/*!
		 *
		 */
		void SsFontDraw::ChangeFont(int32_t font)
		{
			mFontMode = font;
			mNowFont = &mFont[mFontMode];
			setLocate(mNowLocateX, mNowLocateY);
		}

		/************************************************************************************************************************************************************************************/
		/************************************************************************************************************************************************************************************
		 * string?は文字コードとデータ位置の変換
		 ************************************************************************************************************************************************************************************/
		/************************************************************************************************************************************************************************************/
		/* ---------------------------------------------------------------------------
		 * check string 8dot
		 ----------------------------------------------------------------------------*/
		/*!
		 *
		 */
		int32_t SsFontDraw::string8(uint8_t **inp)
		{
			int32_t cno;
			uint8_t *p = *inp;
			if (*p < (uint8_t)kHALF_ASCII)
			{
				cno = ((int32_t)(*p)) & 0x007f;
			}
			else
			{
				cno = ((int32_t)(*p++)) & 0x007f;
				cno *= 0x0100;
				cno += ((int32_t)(*p)) & 0x00ff;
			}
			*inp = p;
			return cno;
		}
		/* ---------------------------------------------------------------------------
		 * check string 10dot
		 ----------------------------------------------------------------------------*/
		/*!
		 *
		 */
		int32_t SsFontDraw::string10(uint8_t **inp)
		{
			int32_t cno;
			uint8_t *p = *inp;
			if (*p < (uint8_t)kHALF_ASCII)
			{
				cno = ((int32_t)(*p)) & 0x007f;
			}
			else
			{
				cno = ((int32_t)(*p++)) & 0x007f;
				cno *= 0x0100;
				cno += ((int32_t)(*p)) & 0x00ff;
			}
			*inp = p;
			return cno;
		}
		/* ---------------------------------------------------------------------------
		 * check string 12dot
		 ----------------------------------------------------------------------------*/
		/*!
		 *
		 */
		int32_t SsFontDraw::stringT12(uint8_t **inp)
		{
			int32_t cno;
			uint8_t *p = *inp;
			if (*p < (uint8_t)kHALF_ASCII)
			{
				cno = ((int32_t)(*p)) & 0x007f;
			}
			else
			{
				cno = ((int32_t)(*p++)) & 0x00ff;
				cno *= 0x0100;
				cno += ((int32_t)(*p)) & 0x00ff;
				cno = SsSjis2Towns(cno) & 0x7fff; // townsコードに変換
			}
			*inp = p;
			return cno;
		}
		/************************************************************************************************************************************************************************************/
		/************************************************************************************************************************************************************************************
		 * putc?は1bitフォントデータをカラー変換して描画位置に書き出す処理
		 ************************************************************************************************************************************************************************************/
		/************************************************************************************************************************************************************************************/
		/* ---------------------------------------------------------------------------
		 * put 8dot
		 ----------------------------------------------------------------------------*/
		/*!
		 *
		 */
		int32_t SsFontDraw::putc8(tFontRecord *fontrecord, int32_t cno)
		{
			int32_t charnum = 2;
			int32_t iI;
			uint16_t getcode[8];
			int32_t getx, gety;

			tSsColor *fgColor = &mTextColor[mFGColor];
			tSsColor *bgColor = &mTextColor[mBGColor];
			getNowXYPosition(charnum, &getx, &gety);
			for (iI = 0; iI < 8; ++iI)
			{
				getcode[iI] = ((uint16_t)fontrecord->mFont[(cno - 0x20) * 8 + iI]) << 8;
			}
			/** 1line draw call **/
			(this->*mDrawFuncTable[mDrawMode])(getcode, fontrecord->mFontX, fontrecord->mFontY, fgColor, bgColor, getx, gety, m_Scale, mWidth, mHeight);
			return charnum; /* 半角何文字分進んだかを返す */
		}
		/* ---------------------------------------------------------------------------
		 * put 10dot
		 ----------------------------------------------------------------------------*/
		/*!
		 *
		 */
		int32_t SsFontDraw::putc10(tFontRecord *fontrecord, int32_t cno)
		{
			int32_t charnum = 2;
			int32_t iI;
			uint16_t getcode[10];
			int32_t getx, gety;

			tSsColor *fgColor = &mTextColor[mFGColor];
			tSsColor *bgColor = &mTextColor[mBGColor];
			getNowXYPosition(charnum, &getx, &gety);

			for (iI = 0; iI < 10; ++iI)
			{
				getcode[10] = (((uint16_t)fontrecord->mFont[(cno - 0x20) * 32 + iI * 2 + 0]) << 8) | ((uint16_t)fontrecord->mFont[(cno - 0x20) * 32 + iI * 2 + 1]);
			}
			/** 1line draw call **/
			(this->*mDrawFuncTable[mDrawMode])(getcode, fontrecord->mFontX, fontrecord->mFontY, fgColor, bgColor, getx, gety, m_Scale, mWidth, mHeight);
			return charnum; /* 半角何文字分進んだかを返す */
		}
		/* ---------------------------------------------------------------------------
		 * put 12dot
		 ----------------------------------------------------------------------------*/
		/*!
		 *
		 */
		int32_t SsFontDraw::putcT12(tFontRecord *fontrecord, int32_t cno)
		{
			int32_t charnum = 1;
			int32_t iI;
			uint16_t getcode[12];
			int32_t getx, gety;

			tSsColor *fgColor = &mTextColor[mFGColor];
			tSsColor *bgColor = &mTextColor[mBGColor];
			getNowXYPosition(charnum, &getx, &gety);
			int32_t px = 6;
			int32_t py = 12;

			if (cno < kTOWNS_ASCIILIMIT)
			{
				/* ASCIIフォントエリア */
				for (iI = 0; iI < 12; ++iI)
				{
					getcode[iI] = ((uint16_t)fontrecord->mFont[(cno - 0x00) * 6 * 2 + iI]) << 8;
				}
			}
			else
			{
				charnum = 2;
				px = 12;
				/* 漢字コードエリア */
				for (iI = 0; iI < 12; ++iI)
				{
					getcode[iI] = (((uint16_t)fontrecord->mFont[((cno - 0x0140 + 0x80) * 12 * 2) + iI * 2 + 0]) << 8) | ((uint16_t)fontrecord->mFont[((cno - 0x0140 + 0x80) * 12 * 2) + iI * 2 + 1]);
				}
			}
			/** 1line draw call **/
			(this->*mDrawFuncTable[mDrawMode])(getcode, px, py, fgColor, bgColor, getx, gety, m_Scale, mWidth, mHeight);
			return charnum;
		}
		/******************************************************************************************/
		/* ---------------------------------------------------------------------------
		 * init locate
		 ----------------------------------------------------------------------------*/
		/*!
		 *
		 */
		void SsFontDraw::initLocate(void)
		{
			mStockLocateX = mStockLocateX = 0;
			mNowLocateX = mNowLocateY = 0;
		}
		/* ---------------------------------------------------------------------------
		 * reset locate
		 ----------------------------------------------------------------------------*/
		/*!
		 *
		 */
		void SsFontDraw::resetLocate(void)
		{
			mNowLocateX = mStockLocateX;
			mNowLocateY = mStockLocateY;
		}
		/* ---------------------------------------------------------------------------
		 * set locate
		 ----------------------------------------------------------------------------*/
		/*!
		 *
		 */
		bool SsFontDraw::setLocate(int32_t x, int32_t y)
		{
			bool update = false;
			mNowLocateX = x;
			mNowLocateY = y;

			int FontX = (mNowFont->mFontX * m_Scale);
			int FontY = (mNowFont->mFontY * m_Scale);

			if ((int32_t)mWidth < (mNowLocateX + FontX))
			{
				mNowLocateX = mStockLocateX;
				mNowLocateY += FontY;
				update = true;
			}
			if ((int32_t)mHeight < (mNowLocateY + FontY))
			{
				mNowLocateY = mStockLocateY;
			}
			return update;
		}
		/* ---------------------------------------------------------------------------
		 * add locate x (chara)
		 ----------------------------------------------------------------------------*/
		/*!
		 *
		 */
		bool SsFontDraw::addLocateCharX(int32_t addoffset)
		{
			int32_t calc = ((mNowFont->mFontX >> 1) * addoffset * m_Scale) + mNowFont->mSpaceX; /* addoffsetは半角単位 */
			return setLocate(mNowLocateX + calc, mNowLocateY);
		}
		/******************************************************************************************/
		/* ---------------------------------------------------------------------------
		 * get framebuffer offset
		 ----------------------------------------------------------------------------*/
		/*!
		 *
		 */
		void SsFontDraw::getNowXYPosition(int32_t charnum, int32_t *x, int32_t *y)
		{
			float calc = (float)(mNowFont->mFontX >> 1);
			calc *= m_Scale;
			calc *= charnum;
			int FontY = (mNowFont->mFontY + mNowFont->mSpaceY) * m_Scale;

			if ((int32_t)mWidth < (mNowLocateX + (int32_t)calc))
			{
				/* 横がはみ出すので改行する */
				setLocate(mStockLocateX, mNowLocateY + FontY); /* 改行 */
				m_DrawCallRecord->m_DrawEnd(FontY);
			}
			*x = mNowLocateX;
			*y = mNowLocateY;
		}
		/* ------------------------------------------------------------------------------------------------------------------------------------------------------*/
		/* ------------------------------------------------------------------------------------------------------------------------------------------------------*/
		/* ------------------------------------------------------------------------------------------------------------------------------------------------------*/
		/* ------------------------------------------------------------------------------------------------------------------------------------------------------*/
		/* ---------------------------------------------------------------------------
		 * draw dot (UserCallback)
		 ----------------------------------------------------------------------------*/
		/*!
		 *
		 */
		void SsFontDraw::drawDotUserCallback(uint16_t *dotdata, int32_t bitmapx, int32_t bitmapy, tSsColor *fgcolor, tSsColor *bgcolor, int32_t inx, int32_t iny, float scale, float width, float height)
		{
			m_DrawCallRecord->m_DrawCall(dotdata, bitmapx, bitmapy, fgcolor, bgcolor, inx, iny, scale, width, height);
		}
		/* #END    ========================================================================================== */
	}
} // Graphics / SsLib
/**************** end of file ****************/
