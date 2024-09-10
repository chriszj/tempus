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
#include "collision.h"
#include "player.h"
#include "timemachine.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define ENEMY_MAX		(170)		// エネミーのMax人数


//*****************************************************************************
// 構造体定義
//*****************************************************************************

enum {
	ENEMY_DIR_UP,
	ENEMY_DIR_RIGHT,
	ENEMY_DIR_DOWN,
	ENEMY_DIR_LEFT
};

enum {
	ENEMY_TYPE_SKELETON,
	ENEMY_TYPE_SKELETON_WARRIOR,
	ENEMY_TYPE_MIMIC,
	ENEMY_TYPE_ROGUE_SKELETON,
	ENEMY_TYPE_MAX
};

struct ENEMYTYPE {

	int id;

	ANIM_DATA animStates[CHAR_ANIM_MAX];

	int maxHP;
};

struct ENEMY
{
	TIMESTATE   timeState;

	int         type;
	int         hp;
	BOOL        alive;
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
	int         currentAnimState;
	int			dir;
	BOOL		moving;
	int			texNo;			// テクスチャ番号
	XMFLOAT3	move;			// 移動速度

	PLAYER* target;

	int         roamingCmdX;
	int         roamingCmdY;
	float       countForNextCmd;
	int         nextCmdWait;

	float		time;			// 線形補間用
	int			tblNo;			// 行動データのテーブル番号
	int			tblMax;			// そのテーブルのデータ数

	float       invincibilityTime;
	int         maxInvincibilityTime;

	//INTERPOLATION_DATA* tbl_adr;			// アニメデータのテーブル先頭アドレス
	//int					tbl_size;			// 登録したテーブルのレコード総数
	//float				move_time;			// 実行時間
};


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitEnemy(void);
void UninitEnemy(void);
void UpdateEnemy(void);
void DrawEnemy(void);

ENEMY* GetEnemy(void);

int GetEnemyCount(void);

void AdjustEnemyHP(ENEMY* enemy, int ammount);



