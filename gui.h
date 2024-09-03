//=============================================================================
//
// スコア処理 [score.h]
// Author : 
//
//=============================================================================
#pragma once
#include "debugproc.h"


//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define FONTCHAR_MAX		(20000)		// スコアの最大値
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

struct BMPTEXT {

	BOOL use = false;

	float x, y;

	char richText[TEXTCHAR_MAX] = "";

	BMPCHAR* textChars[TEXTCHAR_MAX];

};

struct BMPPAGE {

	int id = -1;
	char file[128] = "";

};

struct BMPFONT {

	//  <common lineHeight="32" base="26" scaleW="2048" scaleH="2048" pages="2" packed="0" alphaChnl="1" redChnl="0" greenChnl="0" blueChnl="0"/>
	int lineHeight, base, scaleW, scaleH;

	BMPCHAR chars[FONTCHAR_MAX];
};


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitGUI(void);
void UninitGUI(void);
void UpdateGUI(void);
void DrawGUI(void);

BMPTEXT* AddScore(const char* text);
int GetScore(void);
void SetScore(int score);


