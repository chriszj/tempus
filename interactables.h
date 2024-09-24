//=============================================================================
//
// エネミー処理 [enemy.h]
// Author : 
//
//=============================================================================
#pragma once

#include "main.h"
#include "renderer.h"
#include "debugproc.h"
#include "sprite.h"
#include "timemachine.h"
#include "collision.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define INTERACTABLES_MAX		(150)		// エネミーのMax人数

enum {
	INTERACTABLES_DOOR,
	INTERACTABLES_MASTER_DOOR,
	INTERACTABLES_PIT,
	INTERACTABLES_PIT_2,
	INTERACTABLES_TYPES_MAX
};

enum { 
	INTERACTION_MODE_NORMAL,
	INTERACTION_MODE_HOSTILE
};

//*****************************************************************************
// 構造体定義
//*****************************************************************************
struct INTERACTABLETYPES {

	int id;
	BOOL isAnimReverse;
	int normalModeMinFrame = -1;
	int normalModeMaxFrame = -1;
	int hostileModeMinFrame = -1;
	int hostileModeMaxFrame = -1;
};


struct INTERACTABLE
{
	int id = -1;
	int type = -1;

	TIMESTATE timeState;

	XMFLOAT3	pos;			// ポリゴンの座標
	XMFLOAT3	rot;			// ポリゴンの回転量
	XMFLOAT3	scl;			// ポリゴンの拡大縮小
	BOOL		use;			// true:使っている  false:未使用
	float		w, h;			// 幅と高さ

	COLLIDER2DBOX collider;

	float		countAnim;		// アニメーションカウント
	int			patternAnim;	// アニメーションパターンナンバー
	int         animDivideX;
	int         animDivideY;
	int         patternAnimNum;
	int			normalModeMinFrame = -1;
	int			normalModeMaxFrame = -1;
	int			hostileModeMinFrame = -1;
	int			hostileModeMaxFrame = -1;
	int			texNo;			// テクスチャ番号
	XMFLOAT3	move;			// 移動速度
	BOOL        active;
	int         interactionMode;

	//INTERPOLATION_DATA* tbl_adr;			// アニメデータのテーブル先頭アドレス
	//int					tbl_size;			// 登録したテーブルのレコード総数
	//float				move_time;			// 実行時間
};


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitInteractables(void);
void UninitInteractables(void);
void UpdateInteractables(void);
void DrawInteractables(void);

INTERACTABLE* GetInteractables(void);

int GetInteractablesCount(void);
void SetInteractable(INTERACTABLE* interactable, BOOL active);



