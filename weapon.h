//=============================================================================
//
// 武器処理 [weapon.h]
// Author : 
//
//=============================================================================
#pragma once

#include "main.h"
#include "renderer.h"
#include "sprite.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define BULLET_MAX		(100)		// バレットのMax数
#define BULLET_SPEED	(6.0f)		// バレットの移動スピード

struct WEAPON_TYPE 
{
	int type;
	BOOL hasDirectionSprites;
	BOOL destroyOnAnimationEnd;
	float width;
	float height;
	int textureDivideX;
	int textureDivideY;
	int animWait;
};

// バレット構造体
struct WEAPON
{
	BOOL				use;				// true:使っている  false:未使用
	int                 type;
	float				w, h, duration, elapsedTime; // 幅と高さと命の時間
	XMFLOAT3			pos;				// バレットの座標
	XMFLOAT3			rot;				// バレットの回転量
	XMFLOAT3			move;				// バレットの移動量
	int                 dir;
	int					countAnim;			// アニメーションカウント
	int					patternAnim;		// アニメーションパターンナンバー
	int					texNo;				// 何番目のテクスチャーを使用するのか

};

enum {
	WEAPON_TYPE_SWORD,
	WEAPON_TYPE_ENEMY_SWORD,
	WEAPON_TYPE_MAGIC_BULLET,
	WEAPON_TYPE_MAGIC_SWORD,
	WEAPON_TYPE_MAX
};

enum {
	WEAPON_DIR_UP,
	WEAPON_DIR_RIGHT,
	WEAPON_DIR_DOWN,
	WEAPON_DIR_LEFT
};

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitWeapon(void);
void UninitWeapon(void);
void UpdateWeapon(void);
void DrawWeapon(void);

WEAPON *GetWeapon(void);
void SetWeapon(XMFLOAT3 pos, XMFLOAT3 move, int weaponType, int weaponDirection);


