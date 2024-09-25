//=============================================================================
//
// �G�l�~�[���� [enemy.h]
// Author : 
//
//=============================================================================
#pragma once

#include "main.h"
#include "renderer.h"
#include "debugproc.h"
#include "sprite.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define ITEM_MAX		(200)		// �G�l�~�[��Max�l��

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
// �\���̒�`
//*****************************************************************************

struct ITEM
{
	int id = -1;

	XMFLOAT3	pos;			// �|���S���̍��W
	XMFLOAT3	rot;			// �|���S���̉�]��
	XMFLOAT3	scl;			// �|���S���̊g��k��
	BOOL		use;			// true:�g���Ă���  false:���g�p
	float		w, h;			// ���ƍ���
	float		countAnim;		// �A�j���[�V�����J�E���g
	int			patternAnim;	// �A�j���[�V�����p�^�[���i���o�[
	int         animDivideX;
	int         animDivideY;
	int         patternAnimNum;
	int			texNo;			// �e�N�X�`���ԍ�
	XMFLOAT3	move;			// �ړ����x

	//INTERPOLATION_DATA* tbl_adr;			// �A�j���f�[�^�̃e�[�u���擪�A�h���X
	//int					tbl_size;			// �o�^�����e�[�u���̃��R�[�h����
	//float				move_time;			// ���s����
};


//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT InitItem(void);
void UninitItem(void);
void UpdateItem(void);
void DrawItem(void);

ITEM* GetItem(void);

int GetItemCount(void);
void SetItem(XMFLOAT3 pos, int itemType);


