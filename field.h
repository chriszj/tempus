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
#define TILESET_MAX				(10)
#define MAP_LAYER_MAX			(10)
#define MAP_SCALE				(2)
#define MAP_DRAW_DEBUG			true
#define MAP_DEBUG_KEY			"Debug"
#define MAP_OBJGROUPS_MAX		(10)
#define MAP_OBJGRP_OBJ_MAX		(200)

#define FOBJTYPE_PLAYERSTART	"PlayerStartPoint"
#define FOBJGROUP_WALL			"Wall"
#define FOBJGROUP_LOCATIONS		"Locations"

//*****************************************************************************
// 構造体定義
//*****************************************************************************

struct TILESET
{
	int id = -1;

	int firstGID = -1;
	char source[128] = "";

	// <tileset version="1.10" tiledversion="1.11.0" name="Grass" tileWidth="32" tileHeight="32" tileCount="64" columns="8">
	char name[128] = "";
	float tileWidth;
	float tileHeight;
	int tileCount;
	int columns;

	char textureSource[128] = "";
	int textureW, textureH;

};

struct TILE
{
	int id;

	XMFLOAT3	pos;			// ポリゴンの座標
	XMFLOAT3	rot;			// ポリゴンの回転量
	XMFLOAT3	scl;			// ポリゴンの拡大縮小
	BOOL		use = false;	// true:使っている  false:未使用
	float		w, h;			// 幅と高さ
	float		countAnim;		// アニメーションカウント
	int			patternAnim;	// アニメーションパターンナンバー
	int			texNo;			// テクスチャ番号

	float         textureU;
	float         textureV;
	float         textureWidth;
	float         textureHeigt;

	XMFLOAT3	move;			// 移動速度


	float		tileTime;			// 線形補間用
	int			tblNo;			// 行動データのテーブル番号
	int			tblMax;			// そのテーブルのデータ数

	//INTERPOLATION_DATA* tbl_adr;			// アニメデータのテーブル先頭アドレス
	//int					tbl_size;			// 登録したテーブルのレコード総数
	//float				move_time;			// 実行時間
};

struct TILELAYER
{

	int id = -1;

	char name[128] = "";

	char layerClass[128] = "";

	float width;

	float height;

	TILE tiles[TILES_PER_LAYER_MAX];

	void Reset(void){

		id = -1;
		memset(name, 0, sizeof(name));
		memset(layerClass, 0, sizeof(layerClass));
		width = 0;
		height = 0;

		for (int t = 0; t < TILES_PER_LAYER_MAX; t++)
		{
			tiles[t] = {};
		}

	}

};

struct FIELDOBJECT
{
	int id;
	char name[128] = "";
	char objectType[128] = "";

	float x, y, width, height;

};

struct FIELDOBJECTGROUP
{
	int id;
	char name[128] = "";
	char objectGroupClass[128] = "";

	FIELDOBJECT fObjects[MAP_OBJGRP_OBJ_MAX];

	void Reset(void) {

		id = -1;
		memset(name, 0, sizeof(name));
		memset(objectGroupClass, 0, sizeof(objectGroupClass));

		for (int t = 0; t < MAP_OBJGRP_OBJ_MAX; t++)
		{
			fObjects[t] = {};
		}

	}
};


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitField(void);
void UninitField(void);
void UpdateField(void);
void DrawField(int layer);

TILE* GetField(void);

int GetFieldCount(void);

TILESET* GetTilesetFromTileID(int tileId);

FIELDOBJECT** GetFieldObjectsByClass(const char* objectType);
FIELDOBJECT* GetFieldObjectsFromGroup(const char* objectGroup);
FIELDOBJECT* GetFieldObjectByNameFromGroup(const char* objectGroup, const char* objectName);

