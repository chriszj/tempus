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
#include "collision.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TILES_PER_LAYER_MAX		(15000)		// エネミーのMax人数
#define TILESET_CUSTOM_TILES_MAX (500)
#define TILESET_MAX				(15)
#define MAP_LAYER_MAX			(10)
#define MAP_SCALE				(1.5)
#define MAP_DRAW_DEBUG			true
#define MAP_DRAW_DEBUG_WALLS    false
#define MAP_DEBUG_KEY			"Debug"
#define MAP_OBJLAYERS_MAX		(10)
#define MAP_OBJECTS_PER_LAYER_MAX (200)

#define MAPOBJTYPE_PLAYERSTART	"PlayerStartPoint"
#define MAPOBJLAYER_WALL	    "Wall"
#define MAPOBJLAYER_LOCATIONS   "Locations"
#define MAPOBJLAYER_ENEMY	    "Enemy"
#define MAPOBJLAYER_ITEMS       "Items"
#define MAPOBJLAYER_ENEMIES     "Enemies"
#define MAPOBJLAYER_INTERACTABLES "Interactables"
#define MAPOBJLAYER_PROPS       "Props"

#define TILESET_ITEMS_NAME      "Items2"
#define TILESET_ENEMIES_NAME    "Enemies"
#define TILESET_INTERACTABLES_NAME "Interactables"
#define TILESET_PROPS_NAME "Props"

//*****************************************************************************
// 構造体定義
//*****************************************************************************

struct TILESETCUSTOMTILE {

	// id="1" x="0" y="0" width="13" height="40"
	int id = -1;
	float x, y, width, height;

	int animDivideX, animDivideY, patternAnimNum;

	char textureSource[128] = "";
	int textureW, textureH;

	COLLIDER2DBOX collider;

};

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

	TILESETCUSTOMTILE customTiles[TILESET_CUSTOM_TILES_MAX];


	void Reset(void) {

		id = -1;
		memset(source, 0, sizeof(source));
		
		firstGID = -1;
		tileWidth = 0;
		tileHeight = 0;
		tileCount = 0;
		columns = 0;

		memset(textureSource, 0, sizeof(textureSource));
		textureW = 0;
		textureH = 0;

		for (int t = 0; t < TILESET_CUSTOM_TILES_MAX; t++)
		{
			customTiles[t] = {};
		}

	}

};

struct TILE
{
	int id;

	XMFLOAT3	pos;			// ポリゴンの座標
	XMFLOAT3	rot;			// ポリゴンの回転量
	XMFLOAT3	scl;			// ポリゴンの拡大縮小
	BOOL		use = FALSE;	// true:使っている  false:未使用
	float		w, h;			// 幅と高さ
	float		countAnim;		// アニメーションカウント
	int			patternAnim;	// アニメーションパターンナンバー
	int			texNo;			// テクスチャ番号

	float         textureU;
	float         textureV;
	float         textureWidth;
	float         textureHeigt;

	XMFLOAT3	move;			// 移動速度

};

struct MAPTILELAYER
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

struct MAPOBJECT
{
	int id;
	int gid;
	char name[128] = "";
	char objectType[128] = "";

	float x, y, width, height;

	float textureWidth;
	float textureHeight;

};

struct FIELDOBJECTGROUP
{
	int id;
	char name[128] = "";
	char objectGroupClass[128] = "";

	MAPOBJECT fObjects[MAP_OBJECTS_PER_LAYER_MAX];

	void Reset(void) {

		id = -1;
		memset(name, 0, sizeof(name));
		memset(objectGroupClass, 0, sizeof(objectGroupClass));

		for (int t = 0; t < MAP_OBJECTS_PER_LAYER_MAX; t++)
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
TILESET* GetTilesetByName(const char* name);

MAPOBJECT** GetMapObjectsByClass(const char* objectType);
MAPOBJECT* GetMapObjectsFromLayer(const char* objectLayer);
MAPOBJECT* GetMapObjectByNameFromLayer(const char* objectLayer, const char* objectName);

