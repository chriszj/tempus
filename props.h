//=============================================================================
//
// ������� [enemy.h]
// Author : 
//
//=============================================================================
#pragma once

#include "main.h"
#include "renderer.h"
#include "debugproc.h"
#include "sprite.h"
#include "timemachine.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define PROPS_MAX		(100)		// �G�l�~�[��Max�l��

//*****************************************************************************
// �\���̒�`
//*****************************************************************************

struct PROP
{
	int id = -1;

	TIMESTATE timeState;

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
	BOOL        active;

};


//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT InitProps(void);
void UninitProps(void);
void UpdateProps(void);
void DrawProps(void);

PROP* GetProps(void);

int GetPropsCount(void);




