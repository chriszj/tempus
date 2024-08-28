//=============================================================================
//
// エネミー処理 [fieldf.h]
// Author : 
//
//=============================================================================
#pragma once

#include "main.h"
#include "renderer.h"
#include "debugproc.h"
#include "sprite.h"
#include <iostream>

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TILES_PER_LAYER_MAX		(1000)		// エネミーのMax人数
#define TILESET_MAX     (10)
#define MAP_LAYER_MAX   (10)


//*****************************************************************************
// 構造体定義
//*****************************************************************************

struct TILESET
{
	int firstgid = -1;
	const char *source;

	// <tileset version="1.10" tiledversion="1.11.0" name="Grass" tilewidth="32" tileheight="32" tilecount="64" columns="8">
	const char *name;
	int tilewidth;
	int tileheight;
	int tilecount;
	int columns;

	const char *textureSource;
	int textureW, textureH;

};

struct TILE
{
	XMFLOAT3	pos;			// ポリゴンの座標
	XMFLOAT3	rot;			// ポリゴンの回転量
	XMFLOAT3	scl;			// ポリゴンの拡大縮小
	BOOL		use;			// true:使っている  false:未使用
	float		w, h;			// 幅と高さ
	float		countAnim;		// アニメーションカウント
	int			patternAnim;	// アニメーションパターンナンバー
	int			texNo;			// テクスチャ番号
	XMFLOAT3	move;			// 移動速度


	float		tileTime;			// 線形補間用
	int			tblNo;			// 行動データのテーブル番号
	int			tblMax;			// そのテーブルのデータ数

	//INTERPOLATION_DATA* tbl_adr;			// アニメデータのテーブル先頭アドレス
	//int					tbl_size;			// 登録したテーブルのレコード総数
	//float				move_time;			// 実行時間
};

struct MAPLAYER
{
	//<layer id="1" name="Layer0" width="30" height="20" locked="1">
	int id = -1;

	const char* name;

	int width;

	int height;

	const char* rawData;

	TILE* tiles[TILES_PER_LAYER_MAX];

};


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitField(void);
void UninitField(void);
void UpdateField(void);
void DrawField(void);

TILE* GetField(void);

int GetFieldCount(void);



