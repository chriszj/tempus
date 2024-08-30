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
#define TILES_PER_LAYER_MAX		(1000)		// �G�l�~�[��Max�l��
#define TILESET_MAX     (10)
#define MAP_LAYER_MAX   (10)
#define MAP_SCALE       (2)


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
	int tileWidth;
	int tileHeight;
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
	BOOL		use = false;	// true:�g���Ă���  false:���g�p
	float		w, h;			// ���ƍ���
	float		countAnim;		// �A�j���[�V�����J�E���g
	int			patternAnim;	// �A�j���[�V�����p�^�[���i���o�[
	int			texNo;			// �e�N�X�`���ԍ�

	float         textureU;
	float         textureV;
	float         textureWidth;
	float         textureHeigt;

	XMFLOAT3	move;			// �ړ����x


	float		tileTime;			// ���`��ԗp
	int			tblNo;			// �s���f�[�^�̃e�[�u���ԍ�
	int			tblMax;			// ���̃e�[�u���̃f�[�^��

	//INTERPOLATION_DATA* tbl_adr;			// �A�j���f�[�^�̃e�[�u���擪�A�h���X
	//int					tbl_size;			// �o�^�����e�[�u���̃��R�[�h����
	//float				move_time;			// ���s����
};

struct MAPLAYER
{
	//<layer id="1" name="Layer0" width="30" height="20" locked="1">
	int id = -1;

	const char* name;

	int width;

	int height;

	const char* rawData;

	TILE tiles[TILES_PER_LAYER_MAX];

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


