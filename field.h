//=============================================================================
//
// �G�l�~�[���� [fieldf.h]
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
// �}�N����`
//*****************************************************************************
#define TILES_PER_LAYER_MAX		(15000)		// �G�l�~�[��Max�l��
#define TILESET_MAX				(15)
#define MAP_LAYER_MAX			(10)
#define MAP_SCALE				(1.5)
#define MAP_DRAW_DEBUG			true
#define MAP_DRAW_DEBUG_WALLS    false
#define MAP_DEBUG_KEY			"Debug"
#define MAP_OBJGROUPS_MAX		(10)
#define MAP_OBJGRP_OBJ_MAX		(200)

#define MAPOBJTYPE_PLAYERSTART	"PlayerStartPoint"
#define MAPOBJLAYER_WALL			"Wall"
#define MAPOBJLAYER_LOCATIONS		"Locations"
#define MAPOBJLAYER_ENEMY		    "Enemy"

//*****************************************************************************
// �\���̒�`
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

	XMFLOAT3	pos;			// �|���S���̍��W
	XMFLOAT3	rot;			// �|���S���̉�]��
	XMFLOAT3	scl;			// �|���S���̊g��k��
	BOOL		use = FALSE;	// true:�g���Ă���  false:���g�p
	float		w, h;			// ���ƍ���
	float		countAnim;		// �A�j���[�V�����J�E���g
	int			patternAnim;	// �A�j���[�V�����p�^�[���i���o�[
	int			texNo;			// �e�N�X�`���ԍ�

	float         textureU;
	float         textureV;
	float         textureWidth;
	float         textureHeigt;

	XMFLOAT3	move;			// �ړ����x

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
	char name[128] = "";
	char objectType[128] = "";

	float x, y, width, height;

};

struct FIELDOBJECTGROUP
{
	int id;
	char name[128] = "";
	char objectGroupClass[128] = "";

	MAPOBJECT fObjects[MAP_OBJGRP_OBJ_MAX];

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
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT InitField(void);
void UninitField(void);
void UpdateField(void);
void DrawField(int layer);

TILE* GetField(void);

int GetFieldCount(void);

TILESET* GetTilesetFromTileID(int tileId);

MAPOBJECT** GetMapObjectsByClass(const char* objectType);
MAPOBJECT* GetMapObjectsFromLayer(const char* objectLayer);
MAPOBJECT* GetMapObjectByNameFromLayer(const char* objectLayer, const char* objectName);

