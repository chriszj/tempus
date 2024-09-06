//=============================================================================
//
// �v���C���[���� [player.cpp]
// Author : 
//
//=============================================================================
#include "player.h"
#include "input.h"
#include "bg.h"
#include "field.h"
#include "bullet.h"
#include "item.h"
#include "enemy.h"
#include "collision.h"
#include "score.h"
#include "file.h"
#include "timemachine.h"
#include "gui.h"
#include <string>
#include <iostream>

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define TEXTURE_WIDTH				(200/2)	// �L�����T�C�Y
#define TEXTURE_HEIGHT				(200/2)	// 
#define TEXTURE_MAX					(2)		// �e�N�X�`���̐�

#define TEXTURE_PATTERN_DIVIDE_X	(4)		// �A�j���p�^�[���̃e�N�X�`�����������iX)
#define TEXTURE_PATTERN_DIVIDE_Y	(8)		// �A�j���p�^�[���̃e�N�X�`�����������iY)
#define ANIM_PATTERN_NUM			(TEXTURE_PATTERN_DIVIDE_X*TEXTURE_PATTERN_DIVIDE_Y)	// �A�j���[�V�����p�^�[����
#define ANIM_WAIT					(6)		// �A�j���[�V�����̐؂�ւ��Wait�l

// �v���C���[�̉�ʓ��z�u���W
#define PLAYER_DISP_X				(SCREEN_WIDTH/2)
#define PLAYER_DISP_Y				(SCREEN_HEIGHT/2 + TEXTURE_HEIGHT)

// �W�����v����
#define	PLAYER_JUMP_CNT_MAX			(30)		// 30�t���[���Œ��n����
#define	PLAYER_JUMP_Y_MAX			(300.0f)	// �W�����v�̍���

#define PLAYER_COLLIDER_WIDTH       (50.0f)
#define PLAYER_COLLIDER_HEIGHT      (50.0f)
#define PLAYER_COLLIDER_OFFSETX     (-4.0f)
#define PLAYER_COLLIDER_OFFSETY     (12.0f)


//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
void DrawPlayerOffset(int no);
void PushToTimeState(TIMESTATE* timeState, PLAYER* player);
void PullFromTimeState(TIMESTATE* timeState, PLAYER* player);


//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static ID3D11Buffer				*g_VertexBuffer = NULL;				// ���_���
static ID3D11ShaderResourceView	*g_Texture[TEXTURE_MAX] = { NULL };	// �e�N�X�`�����

static char *g_TexturName[TEXTURE_MAX] = {
	"data/TEXTURE/character_walk2.png",
	"data/TEXTURE/shadow000.jpg",
};

static BOOL		g_Load = FALSE;				// ���������s�������̃t���O
static PLAYER	g_Player[PLAYER_MAX];		// �v���C���[�\����
static int		g_PlayerCount = PLAYER_MAX;	// �����Ă�v���C���[�̐�

static int      g_jumpCnt = 0;
static int		g_jump[PLAYER_JUMP_CNT_MAX] =
{
	-15, -14, -13, -12, -11, -10, -9, -8, -7, -6, -5,-4,-3,-2,-1,
	  1,   2,   3,   4,   5,   6,  7,  8,  9, 10, 11,12,13,14,15
};

static BMPTEXT* g_Score;

//=============================================================================
// ����������
//=============================================================================
HRESULT InitPlayer(void)
{
	ID3D11Device* pDevice = GetDevice();

	//�e�N�X�`������
	for (int i = 0; i < TEXTURE_MAX; i++)
	{
		g_Texture[i] = NULL;
		D3DX11CreateShaderResourceViewFromFile(GetDevice(),
			g_TexturName[i],
			NULL,
			NULL,
			&g_Texture[i],
			NULL);
	}


	// ���_�o�b�t�@����
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX_3D) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	GetDevice()->CreateBuffer(&bd, NULL, &g_VertexBuffer);

	g_PlayerCount = 0;						// �����Ă�v���C���[�̐�

	XMFLOAT3 startPosition = XMFLOAT3(400.0f, 400.0f, 0.0f);

	MAPOBJECT* fieldPositions = GetMapObjectByNameFromLayer(MAPOBJLAYER_LOCATIONS, MAPOBJTYPE_PLAYERSTART);

	if (fieldPositions != NULL)
	{
		startPosition = XMFLOAT3((fieldPositions[0].x) - (TEXTURE_WIDTH / 4), (fieldPositions[0].y)-(TEXTURE_HEIGHT/2), 0.0f);
	}

	// �v���C���[�\���̂̏�����
	for (int i = 0; i < PLAYER_MAX; i++)
	{
		g_PlayerCount++;

		g_Player[i].use = TRUE;
		g_Player[i].pos = startPosition;	// ���S�_����\��
		g_Player[i].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Player[i].w = TEXTURE_WIDTH;
		g_Player[i].h = TEXTURE_HEIGHT;
		g_Player[i].collider = COLLIDER2DBOX(PLAYER_COLLIDER_OFFSETX, PLAYER_COLLIDER_OFFSETY, PLAYER_COLLIDER_WIDTH, PLAYER_COLLIDER_HEIGHT);
		g_Player[i].texNo = 0;

		g_Player[i].countAnim = 0;
		g_Player[i].patternAnim = 0;

		g_Player[i].move = XMFLOAT3(4.0f, 0.0f, 0.0f);		// �ړ���

		g_Player[i].dir = CHAR_DIR_DOWN;					// �������ɂ��Ƃ���
		g_Player[i].moving = FALSE;							// �ړ����t���O
		g_Player[i].patternAnim = g_Player[i].dir * TEXTURE_PATTERN_DIVIDE_X;

		// �W�����v�̏�����
		g_Player[i].jump = FALSE;
		g_Player[i].jumpCnt = 0;
		g_Player[i].jumpY = 0.0f;
		g_Player[i].jumpYMax = PLAYER_JUMP_Y_MAX;

		// ���g�p
		g_Player[i].dash = FALSE;
		for (int j = 0; j < PLAYER_OFFSET_CNT; j++)
		{
			g_Player[i].offset[j] = g_Player[i].pos;
		}

		// �ŏ��̃^�C���X�e�[�g��o�^����
		g_Player[i].timeState.status = WRITABLE;
		PushToTimeState(&g_Player[i].timeState,&g_Player[i]);

		RegisterObjectTimeState(&g_Player[i].timeState);
	}

	g_Score = GetUnusedText();
	g_Score->x = SCREEN_WIDTH / 2;
	g_Score->y = 25;
	g_Score->scale = 1;

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// �I������
//=============================================================================
void UninitPlayer(void)
{

	if (g_Load == FALSE) return;

	for (int i = 0; i < PLAYER_MAX; i++)
	{
		g_PlayerCount++;

		UnregisterObjectTimeState(&g_Player[i].timeState);

	}

	if (g_VertexBuffer)
	{
		g_VertexBuffer->Release();
		g_VertexBuffer = NULL;
	}

	for (int i = 0; i < TEXTURE_MAX; i++)
	{
		if (g_Texture[i])
		{
			g_Texture[i]->Release();
			g_Texture[i] = NULL;
		}
	}

	g_Score->use = FALSE;
	g_Score = NULL;

	g_Load = FALSE;
}

//=============================================================================
// �X�V����
//=============================================================================
void UpdatePlayer(void)
{
	if (g_Load == FALSE) return;
	g_PlayerCount = 0;				// �����Ă�v���C���[�̐�

	for (int i = 0; i < PLAYER_MAX; i++)
	{
		// �����Ă�v���C���[��������������
		if (g_Player[i].use == TRUE)
		{

			g_PlayerCount++;		// �����Ă�v���C���[�̐�

			if (g_Player[i].timeState.status == READ_ONLY)
			{

				PullFromTimeState(&g_Player[i].timeState, &g_Player[i]);
				
			}
			else {
				

				// �A�j���[�V����  
				if (g_Player[i].moving == TRUE)
				{
					g_Player[i].countAnim += 1.0f;
					if (g_Player[i].countAnim > ANIM_WAIT)
					{
						g_Player[i].countAnim = 0.0f;
						// �p�^�[���̐؂�ւ�
						g_Player[i].patternAnim = (g_Player[i].dir * TEXTURE_PATTERN_DIVIDE_X) + ((g_Player[i].patternAnim + 1) % TEXTURE_PATTERN_DIVIDE_X);
					}
				}

				// �L�[���͂ňړ� 
				{
					float speed = PLAYER_WALK_SPEED;

					g_Player[i].moving = FALSE;
					g_Player[i].dash = FALSE;
					g_Player[i].move.x = 0;
					g_Player[i].move.y = 0;

					/*if (GetKeyboardPress(DIK_C) || IsButtonPressed(0, BUTTON_A))
					{
						speed *= 4;
						g_Player[i].dash = TRUE;
					}*/

					int directionMod = 0;

					if (GetKeyboardPress(DIK_S) || IsButtonPressed(0, BUTTON_DOWN))
					{
						g_Player[i].move.y += speed;
						directionMod++;
						g_Player[i].dir = CHAR_DIR_DOWN;
						g_Player[i].moving = TRUE;
					}
					else if (GetKeyboardPress(DIK_W) || IsButtonPressed(0, BUTTON_UP))
					{
						g_Player[i].move.y -= speed;
						directionMod--;
						g_Player[i].dir = CHAR_DIR_UP;
						g_Player[i].moving = TRUE;
					}

					if (GetKeyboardPress(DIK_D) || IsButtonPressed(0, BUTTON_RIGHT))
					{
						g_Player[i].move.x += speed;
						g_Player[i].dir = CHAR_DIR_RIGHT + directionMod;
						g_Player[i].moving = TRUE;
					}
					else if (GetKeyboardPress(DIK_A) || IsButtonPressed(0, BUTTON_LEFT))
					{
						g_Player[i].move.x -= speed;
						g_Player[i].dir = CHAR_DIR_LEFT - directionMod;
						g_Player[i].moving = TRUE;
					}

					

					// MAP�O�`�F�b�N
					BG* bg = GetBG();

					if (g_Player[i].pos.x < 0.0f)
					{
						g_Player[i].pos.x = 0.0f;
					}

					if (g_Player[i].pos.x > bg->w)
					{
						g_Player[i].pos.x = bg->w;
					}

					if (g_Player[i].pos.y < 0.0f)
					{
						g_Player[i].pos.y = 0.0f;
					}

					if (g_Player[i].pos.y > bg->h)
					{
						g_Player[i].pos.y = bg->h;
					}

					//�@�t�B�[���h�̓����蔻��
					MAPOBJECT* walls = GetMapObjectsFromLayer(MAPOBJLAYER_WALL);

					XMFLOAT3 newXPos = XMFLOAT3(g_Player[i].pos);
					XMFLOAT3 newYPos = XMFLOAT3(g_Player[i].pos);
					newXPos.x += g_Player[i].move.x;
					newYPos.y += g_Player[i].move.y;

					for (int w = 0; w < MAP_OBJECTS_PER_LAYER_MAX; w++)
					{
						XMFLOAT3 wallPos = XMFLOAT3(walls[w].x, walls[w].y, 0.0f);
						COLLIDER2DBOX wallCollider = COLLIDER2DBOX(0.0f, 0.0f, walls[w].width, walls[w].height);

						// X���̓����蔻��
						BOOL ansX = CollisionBB(newXPos, g_Player[i].collider, wallPos, wallCollider);

						if (ansX)
						{
							g_Player[i].move.x = 0;
							newXPos.x = g_Player[i].pos.x;
						}

						// Y���̓����蔻��
						BOOL ansY = CollisionBB(newYPos, g_Player[i].collider, wallPos, wallCollider);

						if (ansY)
						{
							g_Player[i].move.y = 0;
							newYPos.y = g_Player[i].pos.y;
						}


					}

					g_Player[i].pos.x = newXPos.x;
					g_Player[i].pos.y = newYPos.y;


					// �ړ����I�������G�l�~�[�Ƃ̓����蔻��
					{
						ENEMY* enemy = GetEnemy();

						// �G�l�~�[�̐��������蔻����s��
						for (int j = 0; j < ENEMY_MAX; j++)
						{
							// �����Ă�G�l�~�[�Ɠ����蔻�������
							if (enemy[j].use == TRUE)
							{
								BOOL ans = CollisionBB(g_Player[i].pos, g_Player[i].w/2, g_Player[i].h/2,
									enemy[j].pos, enemy[j].w/2, enemy[j].h/2);
								// �������Ă���H
								if (ans == TRUE)
								{
									// �����������̏���
									enemy[j].use = FALSE;
									AddScore(10);
								}
							}
						}
					}

					// �A�C�e���̓����蔻��
					{
						ITEM* item = GetItem();

						// �G�l�~�[�̐��������蔻����s��
						for (int itm = 0; itm < ITEM_MAX; itm++)
						{
							// �����Ă�G�l�~�[�Ɠ����蔻�������
							if (item[itm].use == TRUE)
							{
								BOOL ans = CollisionBB(g_Player[i].pos, g_Player[i].w / 2, g_Player[i].h / 2,
									item[itm].pos, item[itm].w / 2, item[itm].h / 2);
								// �������Ă���H
								if (ans == TRUE)
								{
									// �����������̏���
									switch (item[itm].id)
									{
									case KEY:
										g_Player[i].inventoryKeys++;
										break;
									case SOUL:
										g_Player[i].inventorySouls++;
										break;
									default:
										break;
									}

									item[itm].use = FALSE;
								}
							}
						}

					}

					// �o���b�g����
					/*if (GetKeyboardTrigger(DIK_SPACE))
					{
						XMFLOAT3 pos = g_Player[i].pos;
						pos.y += g_Player[i].jumpY;
						SetBullet(pos);
					}

					if (IsButtonTriggered(0, BUTTON_B))
					{
						XMFLOAT3 pos = g_Player[i].pos;
						pos.y += g_Player[i].jumpY;
						SetBullet(pos);
					}*/

					/// �^�C���X�e�[�g�̓o�^
					PushToTimeState(&g_Player[i].timeState, &g_Player[i]);


				}

			}

			// �n�`�Ƃ̓����蔻��p�ɍ��W�̃o�b�N�A�b�v������Ă���
			XMFLOAT3 pos_old = g_Player[i].pos;

			// ���g�p
			for (int j = PLAYER_OFFSET_CNT - 1; j > 0; j--)
			{
				g_Player[i].offset[j] = g_Player[i].offset[j - 1];
			}
			g_Player[i].offset[0] = pos_old;

			// MAP�O�`�F�b�N
			BG* bg = GetBG();

			// �v���C���[�̗����ʒu����MAP�̃X�N���[�����W���v�Z����
			bg->pos.x = g_Player[i].pos.x - PLAYER_DISP_X;
			if (bg->pos.x < 0) bg->pos.x = 0;
			if (bg->pos.x > bg->w - SCREEN_WIDTH) bg->pos.x = bg->w - SCREEN_WIDTH;

			bg->pos.y = g_Player[i].pos.y - PLAYER_DISP_Y;
			if (bg->pos.y < 0) bg->pos.y = 0;
			if (bg->pos.y > bg->h - SCREEN_HEIGHT) bg->pos.y = bg->h - SCREEN_HEIGHT;

		}
	}

	if (GetKeyboardTrigger(DIK_K) || IsButtonTriggered(0, BUTTON_X)) {
		
		DeactivateTimeMachine();
		
	}

	if (GetKeyboardPress(DIK_Q) || IsButtonPressed(0,BUTTON_L)) {

		RewindTimeMachine(60);

	}

	if (GetKeyboardPress(DIK_E) || IsButtonPressed(0, BUTTON_R)) {

		FastForwardTimeMachine(60);

	}


	// ������Z�[�u����
	if (GetKeyboardTrigger(DIK_S))
	{
		SaveData();
	}

	std::wstring wstr = std::to_wstring(GetScore());

	SetText(g_Score, (wchar_t*)wstr.c_str());


#ifdef _DEBUG	// �f�o�b�O����\������


#endif

}

//=============================================================================
// �`�揈��
//=============================================================================
void DrawPlayer(void)
{
	// ���_�o�b�t�@�ݒ�
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

	// �}�g���N�X�ݒ�
	SetWorldViewProjection2D();

	// �v���~�e�B�u�g�|���W�ݒ�
	GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// �}�e���A���ݒ�
	MATERIAL material;
	ZeroMemory(&material, sizeof(material));
	material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	SetMaterial(material);

	BG* bg = GetBG();

	for (int i = 0; i < PLAYER_MAX; i++)
	{
		if (g_Player[i].use == TRUE)		// ���̃v���C���[���g���Ă���H
		{									// Yes

			{	// �e�\��
				SetBlendState(BLEND_MODE_SUBTRACT);	// ���Z����

				// �e�N�X�`���ݒ�
				GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[1]);

				float px = g_Player[i].pos.x - bg->pos.x;	// �v���C���[�̕\���ʒuX
				float py = g_Player[i].pos.y - bg->pos.y;	// �v���C���[�̕\���ʒuY
				float pw = g_Player[i].w;		// �v���C���[�̕\����
				float ph = g_Player[i].h/4;		// �v���C���[�̕\������
				py += 35.0f;		// �����ɕ\��

				float tw = 1.0f;	// �e�N�X�`���̕�
				float th = 1.0f;	// �e�N�X�`���̍���
				float tx = 0.0f;	// �e�N�X�`���̍���X���W
				float ty = 0.0f;	// �e�N�X�`���̍���Y���W

				// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
				SetSpriteColor(g_VertexBuffer, px, py, pw, ph, tx, ty, tw, th,
					XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

				// �|���S���`��
				GetDeviceContext()->Draw(4, 0);

				SetBlendState(BLEND_MODE_ALPHABLEND);	// ���������������ɖ߂�

			}

			// �v���C���[�̕��g��`��
			if (g_Player[i].dash)
			{	// �_�b�V�����������g����
				DrawPlayerOffset(i);
			}

			// �e�N�X�`���ݒ�
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_Player[i].texNo]);

			//�v���C���[�̈ʒu��e�N�X�`���[���W�𔽉f
			float px = g_Player[i].pos.x - bg->pos.x;	// �v���C���[�̕\���ʒuX
			float py = g_Player[i].pos.y - bg->pos.y;	// �v���C���[�̕\���ʒuY
			float pw = g_Player[i].w;		// �v���C���[�̕\����
			float ph = g_Player[i].h;		// �v���C���[�̕\������

			py += g_Player[i].jumpY;		// �W�����v���̍����𑫂�

			// �A�j���[�V�����p
			float tw = 1.0f / TEXTURE_PATTERN_DIVIDE_X;	// �e�N�X�`���̕�
			float th = 1.0f / TEXTURE_PATTERN_DIVIDE_Y;	// �e�N�X�`���̍���
			float tx = (float)(g_Player[i].patternAnim % TEXTURE_PATTERN_DIVIDE_X) * tw;	// �e�N�X�`���̍���X���W
			float ty = (float)(g_Player[i].patternAnim / TEXTURE_PATTERN_DIVIDE_X) * th;	// �e�N�X�`���̍���Y���W

			//float tw = 1.0f;	// �e�N�X�`���̕�
			//float th = 1.0f;	// �e�N�X�`���̍���
			//float tx = 0.0f;	// �e�N�X�`���̍���X���W
			//float ty = 0.0f;	// �e�N�X�`���̍���Y���W

			// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
			SetSpriteColorRotation(g_VertexBuffer, px, py, pw, ph, tx, ty, tw, th,
				XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
				g_Player[i].rot.z);

			// �|���S���`��
			GetDeviceContext()->Draw(4, 0);

		}
	}


}


//=============================================================================
// Player�\���̂̐擪�A�h���X���擾
//=============================================================================
PLAYER* GetPlayer(void)
{
	return &g_Player[0];
}


// �����Ă�G�l�~�[�̐�
int GetPlayerCount(void)
{
	return g_PlayerCount;
}


//=============================================================================
// �v���C���[�̕��g��`��
//=============================================================================
void DrawPlayerOffset(int no)
{
	BG* bg = GetBG();
	float alpha = 0.0f;

	// �e�N�X�`���ݒ�
	GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_Player[no].texNo]);

	for (int j = PLAYER_OFFSET_CNT - 1; j >= 0; j--)
	{
		//�v���C���[�̈ʒu��e�N�X�`���[���W�𔽉f
		float px = g_Player[no].offset[j].x - bg->pos.x;	// �v���C���[�̕\���ʒuX
		float py = g_Player[no].offset[j].y - bg->pos.y;	// �v���C���[�̕\���ʒuY
		float pw = g_Player[no].w;		// �v���C���[�̕\����
		float ph = g_Player[no].h;		// �v���C���[�̕\������

		// �A�j���[�V�����p
		float tw = 1.0f / TEXTURE_PATTERN_DIVIDE_X;	// �e�N�X�`���̕�
		float th = 1.0f / TEXTURE_PATTERN_DIVIDE_Y;	// �e�N�X�`���̍���
		float tx = (float)(g_Player[no].patternAnim % TEXTURE_PATTERN_DIVIDE_X) * tw;	// �e�N�X�`���̍���X���W
		float ty = (float)(g_Player[no].patternAnim / TEXTURE_PATTERN_DIVIDE_X) * th;	// �e�N�X�`���̍���Y���W


		// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
		SetSpriteColorRotation(g_VertexBuffer, px, py, pw, ph, tx, ty, tw, th,
			XMFLOAT4(1.0f, 1.0f, 1.0f, alpha),
			g_Player[no].rot.z);

		alpha += (1.0f / PLAYER_OFFSET_CNT);

		// �|���S���`��
		GetDeviceContext()->Draw(4, 0);
	}
}

void PushToTimeState(TIMESTATE* timeState, PLAYER* player) 
{
	timeState->x = player->pos.x;
	timeState->y = player->pos.y;
	timeState->countAnim = player->countAnim;
	timeState->patternAnim = player->patternAnim;
}

void PullFromTimeState(TIMESTATE* timeState, PLAYER* player) 
{
	player->pos.x = timeState->x;
	player->pos.y = timeState->y;
	player->countAnim = timeState->countAnim;
	player->patternAnim = timeState->patternAnim;
	player->dash = TRUE;
}



