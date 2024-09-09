//=============================================================================
//
// �o���b�g���� [bullet.cpp]
// Author : 
//
//=============================================================================
#include "weapon.h"
#include "enemy.h"
#include "collision.h"
#include "score.h"
#include "bg.h"
#include "effect.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define TEXTURE_WIDTH				(100/2)	// �L�����T�C�Y
#define TEXTURE_HEIGHT				(100/2)	// 
#define TEXTURE_MAX					(2)		// �e�N�X�`���̐�

#define TEXTURE_PATTERN_DIVIDE_X	(5)		// �A�j���p�^�[���̃e�N�X�`�����������iX)
#define TEXTURE_PATTERN_DIVIDE_Y	(4)		// �A�j���p�^�[���̃e�N�X�`�����������iY)
#define ANIM_PATTERN_NUM			(TEXTURE_PATTERN_DIVIDE_X*TEXTURE_PATTERN_DIVIDE_Y)	// �A�j���[�V�����p�^�[����
#define ANIM_WAIT					(4)		// �A�j���[�V�����̐؂�ւ��Wait�l


//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************


//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static ID3D11Buffer				*g_VertexBuffer = NULL;				// ���_���
static ID3D11ShaderResourceView	*g_Texture[TEXTURE_MAX] = { NULL };	// �e�N�X�`�����

static char *g_TexturName[] = {
	"data/TEXTURE/weapon.png",
	"data/TEXTURE/weapon2.png"
};

static BOOL		g_Load = FALSE;			// ���������s�������̃t���O
static WEAPON	g_Weapon[BULLET_MAX];	// �o���b�g�\����

static WEAPON_TYPE g_WeaponTypes[WEAPON_TYPE_MAX] =
{
	{WEAPON_TYPE_SWORD, TRUE, TRUE, 200, 200, 5, 4, 4},
	{WEAPON_TYPE_MAGIC_SWORD, TRUE, TRUE, 200, 200, 5, 4, 4}
};

//=============================================================================
// ����������
//=============================================================================
HRESULT InitWeapon(void)
{
	ID3D11Device *pDevice = GetDevice();

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


	// �o���b�g�\���̂̏�����
	for (int i = 0; i < BULLET_MAX; i++)
	{
		g_Weapon[i].use   = FALSE;			// ���g�p�i���˂���Ă��Ȃ��e�j
		g_Weapon[i].w     = TEXTURE_WIDTH;
		g_Weapon[i].h     = TEXTURE_HEIGHT;
		g_Weapon[i].duration = -1;
		g_Weapon[i].elapsedTime = 0;
		g_Weapon[i].pos   = XMFLOAT3(300, 300.0f, 0.0f);
		g_Weapon[i].rot   = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Weapon[i].texNo = 0;

		g_Weapon[i].countAnim = 0;
		g_Weapon[i].patternAnim = 0;

		g_Weapon[i].move = XMFLOAT3(0.0f, -BULLET_SPEED, 0.0f);	// �ړ��ʂ�������
	}
	
	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// �I������
//=============================================================================
void UninitWeapon(void)
{
	if (g_Load == FALSE) return;

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

}

//=============================================================================
// �X�V����
//=============================================================================
void UpdateWeapon(void)
{
	if (g_Load == FALSE) return;
	int bulletCount = 0;				// ���������o���b�g�̐�

	for (int i = 0; i < BULLET_MAX; i++)
	{
		if (g_Weapon[i].use == TRUE)	// ���̃o���b�g���g���Ă���H
		{								// Yes
			// �A�j���[�V����  
			g_Weapon[i].countAnim++;
			if ((g_Weapon[i].countAnim % g_WeaponTypes[g_Weapon[i].type].animWait) == 0)
			{

				// �p�^�[���̐؂�ւ�
				int animStateIndex = 0;
				int frameCountX = g_WeaponTypes[g_Weapon[i].type].textureDivideX;

				if (g_WeaponTypes[g_Weapon[i].type].hasDirectionSprites)
				{
					animStateIndex += g_Weapon[i].dir * frameCountX;
				}

				// �p�^�[���̐؂�ւ�
				g_Weapon[i].patternAnim = animStateIndex + (g_Weapon[i].patternAnim + 1) % frameCountX;

				if (g_WeaponTypes[g_Weapon[i].type].destroyOnAnimationEnd)
				{
					int lastFrame = animStateIndex + frameCountX;
					if (g_Weapon[i].patternAnim + 1 >= lastFrame)
						g_Weapon[i].use = FALSE;
				}

			}

			// �o���b�g�̈ړ�����
			XMVECTOR pos  = XMLoadFloat3(&g_Weapon[i].pos);
			XMVECTOR move = XMLoadFloat3(&g_Weapon[i].move);
			pos += move;
			XMStoreFloat3(&g_Weapon[i].pos, pos);

			// ��ʊO�܂Ői�񂾁H
			BG* bg = GetBG();
			if (g_Weapon[i].pos.y < (-g_Weapon[i].h/2))		// �����̑傫�����l�����ĉ�ʊO�����肵�Ă���
			{
				g_Weapon[i].use = false;
			}
			if (g_Weapon[i].pos.y > (bg->h + g_Weapon[i].h/2))	// �����̑傫�����l�����ĉ�ʊO�����肵�Ă���
			{
				g_Weapon[i].use = false;
			}

			// �����蔻�菈��
			{
				ENEMY* enemy = GetEnemy();

				// �G�l�~�[�̐��������蔻����s��
				for (int j = 0; j < ENEMY_MAX; j++)
				{
					// �����Ă�G�l�~�[�Ɠ����蔻�������
					if (enemy[j].use == TRUE)
					{
						XMFLOAT3 hitBoxPos = g_Weapon[i].pos;
						float hitboxW = g_Weapon[i].w * 0.2f;
						float hitboxH = g_Weapon[i].h * 0.2f;

						hitboxH /= 2;
						hitboxW /= 2;
						
						switch (g_Weapon[i].dir)
						{
							case WEAPON_DIR_UP:
								//hitboxH /= 2;
								hitboxW *= 0.7f;
								hitBoxPos.y -= hitboxH;
								break;
							case WEAPON_DIR_RIGHT:
								//hitboxW /= 2;
								hitboxH *= 0.7f;
								hitBoxPos.x += hitboxW;
								break;
							case WEAPON_DIR_DOWN:
								//hitboxH /= 2;
								hitboxW *= 0.7f;
								hitBoxPos.y += hitboxH;
								break;
							case WEAPON_DIR_LEFT:
								//hitboxW /= 2;
								hitboxH *= 0.7f;
								hitBoxPos.x -= hitboxW;
								break;
						
						}


						BOOL ans = CollisionBB(hitBoxPos, hitboxW, hitboxH,
							enemy[j].pos, enemy[j].w, enemy[j].h);
						// �������Ă���H
						if (ans == TRUE)
						{
							// �����������̏���
							
							AdjustEnemyHP(&enemy[j], -10);

							AddScore(100);

							// �G�t�F�N�g����
							SetEffect(enemy[j].pos.x, enemy[j].pos.y, 3, 3);
						}
					}
				}
			}


			bulletCount++;
		}
	}


}

//=============================================================================
// �`�揈��
//=============================================================================
void DrawWeapon(void)
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

	for (int i = 0; i < BULLET_MAX; i++)
	{
		if (g_Weapon[i].use == TRUE)		// ���̃o���b�g���g���Ă���H
		{									// Yes
			// �e�N�X�`���ݒ�
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_Weapon[i].texNo]);

			//�o���b�g�̈ʒu��e�N�X�`���[���W�𔽉f
			float px = g_Weapon[i].pos.x - bg->pos.x;	// �o���b�g�̕\���ʒuX
			float py = g_Weapon[i].pos.y - bg->pos.y;	// �o���b�g�̕\���ʒuY
			float pw = g_Weapon[i].w;		// �o���b�g�̕\����
			float ph = g_Weapon[i].h;		// �o���b�g�̕\������

			int textureDivideX = g_WeaponTypes[g_Weapon[i].type].textureDivideX;
			int textureDivideY = g_WeaponTypes[g_Weapon[i].type].textureDivideY;

			float tw = 1.0f / textureDivideX;	// �e�N�X�`���̕�
			float th = 1.0f / textureDivideY;	// �e�N�X�`���̍���
			float tx = (float)(g_Weapon[i].patternAnim % textureDivideX) * tw;	// �e�N�X�`���̍���X���W
			float ty = (float)(g_Weapon[i].patternAnim / textureDivideX) * th;	// �e�N�X�`���̍���Y���W

			// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
			SetSpriteColorRotation(g_VertexBuffer, 
				px, py, pw, ph, 
				tx, ty, tw, th,
				XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
				g_Weapon[i].rot.z);

			// �|���S���`��
			GetDeviceContext()->Draw(4, 0);
		}
	}

}


//=============================================================================
// �o���b�g�\���̂̐擪�A�h���X���擾
//=============================================================================
WEAPON *GetWeapon(void)
{
	return &g_Weapon[0];
}


//=============================================================================
// �o���b�g�̔��ːݒ�
//=============================================================================
void SetWeapon(XMFLOAT3 pos, XMFLOAT3 move, int weaponType, int weaponDirection)
{
	// �������g�p�̒e�����������甭�˂��Ȃ�( =����ȏ㌂�ĂȂ����Ď� )
	for (int i = 0; i < BULLET_MAX; i++)
	{
		if (g_Weapon[i].use == FALSE)		// ���g�p��Ԃ̃o���b�g��������
		{
			g_Weapon[i].use = TRUE;			// �g�p��Ԃ֕ύX����
			g_Weapon[i].pos = pos;			// ���W���Z�b�g
			g_Weapon[i].move = move;
			g_Weapon[i].w = g_WeaponTypes[weaponType].width;
			g_Weapon[i].h = g_WeaponTypes[weaponType].height;
			g_Weapon[i].duration = -1;
			g_Weapon[i].elapsedTime = 0;
			g_Weapon[i].move.z = 0;         // 2D �Q�[��������BZ�����g��Ȃ��B
			g_Weapon[i].dir = weaponDirection;
			g_Weapon[i].patternAnim = 0;

			g_Weapon[i].texNo = weaponType;
			return;							// 1���Z�b�g�����̂ŏI������
		}
	}
}

