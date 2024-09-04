//=============================================================================
//
// グラフィックユーザーインターフェース処理 [gui.h]
// Author : 
//
//=============================================================================
#pragma once
#include "debugproc.h"


//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define FONTCHAR_MAX		(20000)
#define FONTTEXTURE_MAX		(8)		// テクスチャの数
#define TEXTCHAR_MAX		(500)
#define TEXTOBJECTS_MAX     (50)

//*****************************************************************************
// 構造体定義
//*****************************************************************************

struct BMPCHAR
{
	//    <char id="32" x="303" y="2046" width="3" height="1" xoffset="-1" yoffset="31" xadvance="6" page="0" chnl="15" />

	int id = -1;
	int x, y, width, height, xoffset, yoffset, xadvance, page, chnl;

	float textureU, textureV, textureWidth, textureHeigt;

};

struct BMPPAGE {

	int id = -1;
	char file[128] = "";

};

struct BMPFONT {

	//  <common lineHeight="32" base="26" scaleW="2048" scaleH="2048" pages="2" packed="0" alphaChnl="1" redChnl="0" greenChnl="0" blueChnl="0"/>
	int lineHeight, base, scaleW, scaleH = -1;

	BMPPAGE pages[FONTTEXTURE_MAX];

	BMPCHAR chars[FONTCHAR_MAX];

	void Reset(void) {
		lineHeight = base = scaleW = scaleH = -1;
		memset(pages, 0, sizeof(pages));
		memset(chars, 0, sizeof(chars));
	}
};

struct BMPTEXT {

	BOOL use = FALSE;

	float x, y;

	float scale = 1.0f;

	wchar_t richText[TEXTCHAR_MAX] = L"";

	BMPCHAR* textChars[TEXTCHAR_MAX];

};


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitGUI(void);
void UninitGUI(void);
void UpdateGUI(void);
void DrawGUI(void);

BMPTEXT* GetUnusedText();
void SetText(BMPTEXT* bmpText, wchar_t* text);
void ClearGUI(void);
