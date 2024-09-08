//=============================================================================
//
// ���폈�� [weapon.h]
// Author : 
//
//=============================================================================
#pragma once

#include "main.h"
#include "renderer.h"
#include "sprite.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define BULLET_MAX		(100)		// �o���b�g��Max��
#define BULLET_SPEED	(6.0f)		// �o���b�g�̈ړ��X�s�[�h

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

// �o���b�g�\����
struct WEAPON
{
	BOOL				use;				// true:�g���Ă���  false:���g�p
	int                 type;
	float				w, h, duration, elapsedTime; // ���ƍ����Ɩ��̎���
	XMFLOAT3			pos;				// �o���b�g�̍��W
	XMFLOAT3			rot;				// �o���b�g�̉�]��
	XMFLOAT3			move;				// �o���b�g�̈ړ���
	int                 dir;
	int					countAnim;			// �A�j���[�V�����J�E���g
	int					patternAnim;		// �A�j���[�V�����p�^�[���i���o�[
	int					texNo;				// ���Ԗڂ̃e�N�X�`���[���g�p����̂�

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
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT InitWeapon(void);
void UninitWeapon(void);
void UpdateWeapon(void);
void DrawWeapon(void);

WEAPON *GetWeapon(void);
void SetWeapon(XMFLOAT3 pos, XMFLOAT3 move, int weaponType, int weaponDirection);


