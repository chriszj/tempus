//=============================================================================
//
// 小道具処理 [enemy.h]
// Author : 
//
//=============================================================================
#pragma once

#include "main.h"
#include "renderer.h"
#include "debugproc.h"
#include "sprite.h"
#include "timemachine.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define PROPS_MAX		(100)		// エネミーのMax人数

//*****************************************************************************
// 構造体定義
//*****************************************************************************

struct PROP
{
	int id = -1;

	TIMESTATE timeState;

	XMFLOAT3	pos;			// ポリゴンの座標
	XMFLOAT3	rot;			// ポリゴンの回転量
	XMFLOAT3	scl;			// ポリゴンの拡大縮小
	BOOL		use;			// true:使っている  false:未使用
	float		w, h;			// 幅と高さ
	float		countAnim;		// アニメーションカウント
	int			patternAnim;	// アニメーションパターンナンバー
	int         animDivideX;
	int         animDivideY;
	int         patternAnimNum;
	int			texNo;			// テクスチャ番号
	XMFLOAT3	move;			// 移動速度
	BOOL        active;

};


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitProps(void);
void UninitProps(void);
void UpdateProps(void);
void DrawProps(void);

PROP* GetProps(void);

int GetPropsCount(void);




