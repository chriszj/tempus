//=============================================================================
//
// プレイヤー処理 [player.h]
// Author : 
//
//=============================================================================
#pragma once

#include "main.h"
#include "renderer.h"
#include "debugproc.h"
#include "sprite.h"
#include "collision.h"
#include "timemachine.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define PLAYER_MAX			(1)		// プレイヤーのMax人数

#define	PLAYER_OFFSET_CNT	(16)	// 16分身
#define PLAYER_WALK_SPEED   (5)
#define PLAYER_DASH_SPEED   (5)

enum
{
	CHAR_DIR_UP,
	CHAR_DIR_UP_RIGHT,
	CHAR_DIR_RIGHT,
	CHAR_DIR_RIGHT_DOWN,
	CHAR_DIR_DOWN,
	CHAR_DIR_DOWN_LEFT,
	CHAR_DIR_LEFT,
	CHAR_DIR_LEFT_UP,

	CHAR_DIR_MAX
};

//*****************************************************************************
// 構造体定義
//*****************************************************************************

struct PLAYER
{
	TIMESTATE timeState;

	XMFLOAT3	pos;			// ポリゴンの座標
	XMFLOAT3	rot;			// ポリゴンの回転量
	BOOL		use;			// true:使っている  false:未使用
	float		w, h;			// 幅と高さ
	COLLIDER2DBOX collider;       // 当たり判定の改善のため
	float		countAnim;		// アニメーションカウント
	int			patternAnim;	// アニメーションパターンナンバー
	int         currentAnimState;
	int			texNo;			// テクスチャ番号
	
	int			dir;			// 向き（0:上 1:右 2:下 3:左）
	BOOL		moving;			// 移動中フラグ

	BOOL		dash;			// ダッシュ中フラグ
	XMFLOAT3	move;			// 移動速度
	XMFLOAT3	offset[PLAYER_OFFSET_CNT];		// 残像ポリゴンの座標

	BOOL		jump;			// ジャンプフラグ
	float		jumpY;			// ジャンプの高さ
	int			jumpCnt;		// ジャンプ中のカウント
	float		jumpYMax;		// 

	XMFLOAT3    lastCheckPointPos;

	int         inventoryKeys, usedInventoryKeys;
	int         inventorySouls;
	int			inventoryCoins;
	int			inventorySwords;
	int			inventoryMasterKeys, usedInventoryMasterKeys;
	int			lastSwitchOrderActivated = -1;

	float       invincibilityTime;
	int         maxInvincibilityTime;

	int         hp;
};


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitPlayer(void);
void UninitPlayer(void);
void UpdatePlayer(void);
void DrawPlayer(void);

PLAYER* GetPlayer(void);

int GetPlayerCount(void);

void AdjustPlayerHP(PLAYER* player, int ammount);

void MakePlayerFall(PLAYER* player, int fallDmg);

BOOL HasReachedGoal(void);


