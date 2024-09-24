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
#include "timemachine.h"
#include "collision.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define INTERACTABLES_MAX		(150)		// �G�l�~�[��Max�l��

enum {
	INTERACTABLES_DOOR,
	INTERACTABLES_MASTER_DOOR,
	INTERACTABLES_PIT,
	INTERACTABLES_PIT_2,
	INTERACTABLES_TYPES_MAX
};

enum { 
	INTERACTION_MODE_NORMAL,
	INTERACTION_MODE_HOSTILE
};

//*****************************************************************************
// �\���̒�`
//*****************************************************************************
struct INTERACTABLETYPES {

	int id;
	BOOL isAnimReverse;
	int normalModeMinFrame = -1;
	int normalModeMaxFrame = -1;
	int hostileModeMinFrame = -1;
	int hostileModeMaxFrame = -1;
};


struct INTERACTABLE
{
	int id = -1;
	int type = -1;

	TIMESTATE timeState;

	XMFLOAT3	pos;			// �|���S���̍��W
	XMFLOAT3	rot;			// �|���S���̉�]��
	XMFLOAT3	scl;			// �|���S���̊g��k��
	BOOL		use;			// true:�g���Ă���  false:���g�p
	float		w, h;			// ���ƍ���

	COLLIDER2DBOX collider;

	float		countAnim;		// �A�j���[�V�����J�E���g
	int			patternAnim;	// �A�j���[�V�����p�^�[���i���o�[
	int         animDivideX;
	int         animDivideY;
	int         patternAnimNum;
	int			normalModeMinFrame = -1;
	int			normalModeMaxFrame = -1;
	int			hostileModeMinFrame = -1;
	int			hostileModeMaxFrame = -1;
	int			texNo;			// �e�N�X�`���ԍ�
	XMFLOAT3	move;			// �ړ����x
	BOOL        active;
	int         interactionMode;

	//INTERPOLATION_DATA* tbl_adr;			// �A�j���f�[�^�̃e�[�u���擪�A�h���X
	//int					tbl_size;			// �o�^�����e�[�u���̃��R�[�h����
	//float				move_time;			// ���s����
};


//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT InitInteractables(void);
void UninitInteractables(void);
void UpdateInteractables(void);
void DrawInteractables(void);

INTERACTABLE* GetInteractables(void);

int GetInteractablesCount(void);
void SetInteractable(INTERACTABLE* interactable, BOOL active);



