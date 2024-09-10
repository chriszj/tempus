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
#include "collision.h"
#include "player.h"
#include "timemachine.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define ENEMY_MAX		(170)		// �G�l�~�[��Max�l��


//*****************************************************************************
// �\���̒�`
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
	int         currentAnimState;
	int			dir;
	BOOL		moving;
	int			texNo;			// �e�N�X�`���ԍ�
	XMFLOAT3	move;			// �ړ����x

	PLAYER* target;

	int         roamingCmdX;
	int         roamingCmdY;
	float       countForNextCmd;
	int         nextCmdWait;

	float		time;			// ���`��ԗp
	int			tblNo;			// �s���f�[�^�̃e�[�u���ԍ�
	int			tblMax;			// ���̃e�[�u���̃f�[�^��

	float       invincibilityTime;
	int         maxInvincibilityTime;

	//INTERPOLATION_DATA* tbl_adr;			// �A�j���f�[�^�̃e�[�u���擪�A�h���X
	//int					tbl_size;			// �o�^�����e�[�u���̃��R�[�h����
	//float				move_time;			// ���s����
};


//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT InitEnemy(void);
void UninitEnemy(void);
void UpdateEnemy(void);
void DrawEnemy(void);

ENEMY* GetEnemy(void);

int GetEnemyCount(void);

void AdjustEnemyHP(ENEMY* enemy, int ammount);



