/* -*- mode: c++; coding: utf-8-dos; tab-width: 4; -*- */
/****************************************************************/
/*    CDIB BMP用フォント書き込みルーチン                        */
/****************************************************************/
#ifndef __SSDRAWFONT_H__
#define __SSDRAWFONT_H__

/********************************/
/*    include                   */
/********************************/
#include <base/system/platform.h>
#include <base/system/basic_types.h>

namespace SsLib { namespace Ut {
/********************************/
/*    define                    */
/********************************/
/**
 * 外部用定義
 */
enum {
	eDRAWFONT_FONT8,
	eDRAWFONT_FONT10,
	eDRAWFONT_FONT12,
	eDRAWFONT_SYSTEM,

	eDRAWFONT_MAX
};

/**
 * 初期化用パラメータ
 */
struct tFontRecord {
	bool       mHalfCalcMode;	// CharLocateで半分で計算するかどうか
	int32_t    mFontX;			/* X */
	int32_t    mFontY;			/* Y */
	int32_t    mSpaceX;			/* X */
	int32_t    mSpaceY;			/* Y */

	uint8_t    *mFont;
};
#if 0
//動いてから実装し直す
struct tDrawFontParam {
	void          *mWork;
	float         mWidth;
	float         mHeight;
	bool          mIsAlpha;			/* RGBAフレームバッファかどうか(default:off) */
	int32_t       mFontMode;
	int32_t       mForegroundColor;
	int32_t       mBackgroundColor;
	tFontRecord   mFont[eDRAWFONT_MAX];
};
#endif

/**
 * 内部パラメータ
 */
#define    kWIDTH_ALIGN(w)    (((w) + 3)&~3)
static const int32_t    kFRAMEBUF_RGB  = (3);
static const int32_t    kFRAMEBUF_RGBA = (4);
static const int32_t    kPALETTE_MAX   = (0x100);

static const int32_t    kHALF_ASCII       = (0x80);
static const int32_t    kTOWNS_ASCIILIMIT = (0x00000100);

enum eDRAWMODE{
	eDRAWMODE_RGB,
	eDRAWMODE_RGBA,
	eDRAWMODE_USERCALLBACK,

	eDRAWMODE_MAX
};

typedef struct _DrawContext{
	tSsColor *fgcolor;
	tSsColor *bgcolor;
	float    scale;
	float    width;
	float    height;
} DrawContext;

typedef void (*DRAW_CALLBACK)(uint16_t *dotdata, int32_t bitmapx, int32_t bitmapy, tSsColor *fgcolor, tSsColor *bgcolor, int32_t inx, int32_t iny,float scale, float width, float height);
//typedef void (*DRAWIN_CALLBACK)();
typedef void (*DRAWEND_CALLBACK)(int32_t scaley);

typedef struct _DrawCallRecord{
	//DRAWIN_CALLBACK m_DrawIn;
	DRAW_CALLBACK   m_DrawCall;
	DRAWEND_CALLBACK m_DrawEnd;
} DrawCallRecord;

/********************************/
/*    structs                   */
/********************************/
/**
 * SsFontDraw
 */
class SsFontDraw{
private:
	int32_t      mXAdf, mYAdf;          /* 描画フォントサイズ */
	int32_t      mNowLocateX,   mNowLocateY;   /* 今の位置 */
	int32_t      mStockLocateX, mStockLocateY; /* 描画位置(dot指定) */
	int32_t      mTab;                  /* \tの時の展開スペース */
	int32_t      mBGColor, mFGColor;

	float       mWidth, mHeight;       /* フレームバッファのX/Y */
	float       m_Scale = 1.0f;

	uint8_t     *mFrameBuffer;				/* 描画先 */

	int32_t      mFontMode;
	tSsColor     mTextColor[kPALETTE_MAX];
	/* font table */
	tFontRecord  *mNowFont;
	tFontRecord  mFont[eDRAWFONT_MAX];

	/* draw */
	eDRAWMODE    mDrawMode;
	DrawCallRecord *m_DrawCallRecord;
	bool         mIsDebugDraw;
	bool         mIsForceUpdate;

public:
	/* 初期化・最終解放メソッド */
	virtual bool Init(void);
private:
	bool init(void);
	void remove(void);
public:
	bool Create(uint16_t inwidth,uint16_t inheight, eDRAWMODE drawmode, DrawCallRecord *inDrawCallRecord);
	bool Destroy(void);
	/* 専用命令 */
	void    ClearFrameBuffer(int32_t index = -1); /* 指定されたcolor indexでframe bufferを初期化(-1:mBGColor) */
	int32_t DrawString(const char *mesg);	 /* 文字列を描画する */
	int32_t GetLatestY(){
		return mNowLocateY;
	}
	int32_t GetLatestX(){
		return mNowLocateX;
	}

	bool SetColor(int32_t index, tSsColor &color); /* 指定されたcolor indexにカラーをセットする */
	bool ChangeForegroundColor(int32_t index);	   /* 文字色を指定されたcolor indexに変更 */
	bool ChangeBackgroundColor(int32_t index);	   /* 抜き色を指定されたcolor indexに変更 */
	
	void ChangeLocate(int32_t x, int32_t y); /* 描画位置の変更（dot単位） */
	void ChangeCharLocate(int32_t x, int32_t y); /* 描画位置の変更(半角単位) */
	void ChangeFullCharLocate(int32_t x, int32_t y); /* 描画位置の変更(全角単位) */
	void ChangeFont(int32_t font);			 /* 指定された文字フォントに変更 */
	void ChangeScale(float inscale){ m_Scale = inscale > 4 ? 4: inscale; }
	void GetLocate(int32_t *getX,int32_t *getY){
		*getX = mNowLocateX;
		*getY = mNowLocateY;
	}

private:
	typedef int32_t (SsFontDraw::*STRING_FUNC)(uint8_t **inp);
	typedef int32_t (SsFontDraw::*FONT_FUNC)(tFontRecord *fontrecord, int32_t cno);
	typedef void (SsFontDraw::*DRAW_FUNC)(uint16_t *dotdata, int32_t bitmapx, int32_t bitmapy, tSsColor *fgcolor, tSsColor *bgcolor, int32_t inx, int32_t iny,float scale, float width, float height);

	static STRING_FUNC mStringFuncTable[eDRAWFONT_MAX];
	static FONT_FUNC mFontFuncTable[eDRAWFONT_MAX];
	static DRAW_FUNC mDrawFuncTable[eDRAWMODE_MAX];

	int32_t string8(uint8_t **inp);
	int32_t string10(uint8_t **inp);
	int32_t stringT12(uint8_t **inp);


	int32_t putc8(tFontRecord *fontrecord, int32_t cno);
	int32_t putc10(tFontRecord *fontrecord, int32_t cno);
	int32_t putcT12(tFontRecord *fontrecord, int32_t cno);

	void    initLocate(void);
	void    resetLocate(void);
	bool    setLocate(int32_t x, int32_t y);
	bool    addLocateCharX(int32_t addoffset);

	void getNowXYPosition(int32_t charnum, int32_t *x, int32_t *y);

	void drawDotUserCallback(uint16_t *dotdata, int32_t bitmapx, int32_t bitmapy, tSsColor *fgcolor, tSsColor *bgcolor, int32_t inx, int32_t iny,float scale, float width, float height);
};


/********************************/
/*    global works              */
/********************************/
/********************************/
/*    prototypes                */
/********************************/



/********************************/
}} // Debug / SsLib
#endif /* __SSDRAWFONT_H__ */
/**************** end of file ****************/
