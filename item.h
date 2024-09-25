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

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define ITEM_MAX		(200)		// エネミーのMax人数

enum {
	ITEM_TYPE_KEY,
	ITEM_TYPE_SWORD,
	ITEM_TYPE_SOUL,
	ITEM_TYPE_MASTER_KEY,
	ITEM_TYPE_MASTER_SWORD,
	ITEM_TYPE_HEALTH_POTION,
	ITEM_TYPE_COIN,
	ITEM_TYPE_MONEY,
	ITEM_TYPE_MAX
};

//*****************************************************************************
// 構造体定義
//*****************************************************************************

struct ITEM
{
	int id = -1;

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

	//INTERPOLATION_DATA* tbl_adr;			// アニメデータのテーブル先頭アドレス
	//int					tbl_size;			// 登録したテーブルのレコード総数
	//float				move_time;			// 実行時間
};


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitItem(void);
void UninitItem(void);
void UpdateItem(void);
void DrawItem(void);

ITEM* GetItem(void);

int GetItemCount(void);
void SetItem(XMFLOAT3 pos, int itemType);


