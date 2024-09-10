//=============================================================================
//
// �G�l�~�[���� [enemy.cpp]
// Author : 
//
//=============================================================================
#include "interactables.h"
#include "bg.h"
#include "player.h"
#include "fade.h"
#include "collision.h"
#include "field.h"
#include "gui.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define TEXTURE_WIDTH				(200/2)	// �L�����T�C�Y
#define TEXTURE_HEIGHT				(200/2)	// 
#define TEXTURE_MAX					(2)		// �e�N�X�`���̐�

#define ANIM_WAIT					(4)		// �A�j���[�V�����̐؂�ւ��Wait�l


//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************


//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static ID3D11Buffer				*g_VertexBuffer = NULL;				// ���_���
static ID3D11ShaderResourceView	*g_Texture[TILESET_CUSTOM_TILES_MAX] = { NULL };	// �e�N�X�`�����

static char *g_TexturName[TEXTURE_MAX] = {
	"data/TEXTURE/candle-burning-only fire.png",
	"data/TEXTURE/bar_white.png",
};


static BOOL		g_Load = FALSE;			// ���������s�������̃t���O
static INTERACTABLE	g_Interactables[INTERACTABLES_MAX];		// �G�l�~�[�\����

static int		g_ItemCount = INTERACTABLES_MAX;

static float offsetx = 200.0f;
static float offsety = 200.0f;

static TILESET* g_InteractablesTileset;

static INTERACTABLETYPES g_InteractableTypes[INTERACTABLES_TYPES_MAX] =
{
	{INTERACTABLES_DOOR, TRUE, FALSE },
	{INTERACTABLES_MASTER_DOOR, FALSE, FALSE },
	{INTERACTABLES_DUMMY, FALSE, TRUE}
};

//=============================================================================
// ����������
//=============================================================================
HRESULT InitInteractables(void)
{
	MAPOBJECT* mapObjects = GetMapObjectsFromLayer(MAPOBJLAYER_INTERACTABLES);

	ID3D11Device *pDevice = GetDevice();

	g_InteractablesTileset = GetTilesetByName(TILESET_INTERACTABLES_NAME);

	//�e�N�X�`������
	for (int i = 0; i < TILESET_CUSTOM_TILES_MAX; i++)
	{

		if (g_InteractablesTileset->customTiles[i].id < 0)
			continue;

		g_Texture[i] = NULL;
		D3DX11CreateShaderResourceViewFromFile(GetDevice(),
			g_InteractablesTileset->customTiles[i].textureSource,
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

	
	// �G�l�~�[�\���̂̏�����
	g_ItemCount = 0;
	for (int i = 0; i < INTERACTABLES_MAX; i++)
	{

		g_ItemCount++;
		g_Interactables[i].use = mapObjects[i].id > 0? TRUE: FALSE;

		if (!g_Interactables[i].use)
			continue;

		g_Interactables[i].active = TRUE;
		
		g_Interactables[i].pos = XMFLOAT3((mapObjects[i].x ), (mapObjects[i].y - (mapObjects[i].height)), 0.0f);
		g_Interactables[i].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Interactables[i].scl = XMFLOAT3(1.0f, 1.0f, 1.0f);
		g_Interactables[i].w = mapObjects[i].width;
		g_Interactables[i].h = mapObjects[i].height;
		g_Interactables[i].texNo = mapObjects[i].gid - g_InteractablesTileset->firstGID -1;
		//�@�g���Ă���e�L�X�`���[�Ŏ�ނ��킩���
		g_Interactables[i].id = g_Interactables[i].texNo;
		g_Interactables[i].dummy = g_InteractableTypes[g_Interactables[i].id].isDummy ? TRUE : FALSE;

		g_Interactables[i].countAnim = 0;
		g_Interactables[i].patternAnim = 0;

		int customTileWidth = g_InteractablesTileset->customTiles[g_Interactables[i].texNo].width;
		int customTileTextureW = g_InteractablesTileset->customTiles[g_Interactables[i].texNo].textureW;

		int divideX = customTileTextureW / customTileWidth;

		int customTileHeight = g_InteractablesTileset->customTiles[g_Interactables[i].texNo].height;
		int customTileTextureH = g_InteractablesTileset->customTiles[g_Interactables[i].texNo].textureH;
		int divideY = customTileTextureH / customTileHeight;

		g_Interactables[i].animDivideX = divideX;
		g_Interactables[i].animDivideY = divideY;
		g_Interactables[i].patternAnimNum = divideX*divideY;

		g_Interactables[i].move = XMFLOAT3(4.0f, 0.0f, 0.0f);		// �ړ���

		
	}

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// �I������
//=============================================================================
void UninitInteractables(void)
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

	for (int e = 0; e < INTERACTABLES_MAX; e++)
	{
		g_Interactables[e].use = FALSE;
	}

	g_Load = FALSE;
}

//=============================================================================
// �X�V����
//=============================================================================
void UpdateInteractables(void)
{
	if (g_Load == FALSE) return;
	g_ItemCount = 0;			// �����Ă�G�l�~�[�̐�

	for (int i = 0; i < INTERACTABLES_MAX; i++)
	{
		// �����Ă�G�l�~�[��������������
		if (g_Interactables[i].use == TRUE)
		{
			if (g_Interactables[i].id == 1);
				g_ItemCount++;		// �����Ă��G�̐�
			
			// �n�`�Ƃ̓����蔻��p�ɍ��W�̃o�b�N�A�b�v������Ă���
			XMFLOAT3 pos_old = g_Interactables[i].pos;

			// �A�j���[�V����  
			g_Interactables[i].countAnim += 1.0f;
			if (g_Interactables[i].countAnim > ANIM_WAIT)
			{
				g_Interactables[i].countAnim = 0.0f;

				int nextFrame = 0;
				// �p�^�[���̐؂�ւ�
				if (g_Interactables[i].active)
				{

					if (g_Interactables[i].patternAnim > 0)
					{
						nextFrame--;
					}
					

				}
				else {
				
					if (g_Interactables[i].patternAnim < g_Interactables[i].patternAnimNum -1)
					{
						nextFrame++;
					}

				}

				g_Interactables[i].patternAnim = (g_Interactables[i].patternAnim + nextFrame) % g_Interactables[i].patternAnimNum;
			}

			// �ړ����I�������G�l�~�[�Ƃ̓����蔻��
			{
				PLAYER* player = GetPlayer();

				// �G�l�~�[�̐��������蔻����s��
				for (int j = 0; j < INTERACTABLES_MAX; j++)
				{
					// �����Ă�G�l�~�[�Ɠ����蔻�������
					if (player[j].use == TRUE)
					{
						BOOL ans = CollisionBB(g_Interactables[i].pos, g_Interactables[i].w, g_Interactables[i].h,
							player[j].pos, player[j].w, player[j].h);
						// �������Ă���H
						if (ans == TRUE)
						{
							// �����������̏���
							//player[j].use = FALSE;	// �f�o�b�O�ňꎞ�I�ɖ��G�ɂ��Ă�����
						}
					}
				}
			}
		}
	}

#ifdef _DEBUG	// �f�o�b�O����\������


#endif

}

//=============================================================================
// �`�揈��
//=============================================================================
void DrawInteractables(void)
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

	for (int i = 0; i < INTERACTABLES_MAX; i++)
	{
		if (g_Interactables[i].use == TRUE)			// ���̃G�l�~�[���g���Ă���H
		{									// Yes
			// �e�N�X�`���ݒ�
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_Interactables[i].texNo]);

			//�G�l�~�[�̈ʒu��e�N�X�`���[���W�𔽉f
			float px = g_Interactables[i].pos.x - bg->pos.x;	// �G�l�~�[�̕\���ʒuX
			float py = g_Interactables[i].pos.y - bg->pos.y;	// �G�l�~�[�̕\���ʒuY
			float pw = g_Interactables[i].w;		// �G�l�~�[�̕\����
			float ph = g_Interactables[i].h;		// �G�l�~�[�̕\������

			/*int customTileWidth = g_ItemsTileset->customTiles[g_Item[i].texNo].width;
			int customTileTextureW = g_ItemsTileset->customTiles[g_Item[i].texNo].textureW;
			int divideX = customTileTextureW / customTileWidth;

			int customTileHeight = g_ItemsTileset->customTiles[g_Item[i].texNo].height;
			int customTileTextureH = g_ItemsTileset->customTiles[g_Item[i].texNo].textureH;
			int divideY = customTileTextureH / customTileHeight;*/

			// �A�j���[�V�����p
			float tw = 1.0f / g_Interactables[i].animDivideX;	// �e�N�X�`���̕�
			float th = 1.0f / g_Interactables[i].animDivideY;	// �e�N�X�`���̍���
			float tx = (float)(g_Interactables[i].patternAnim % g_Interactables[i].animDivideX) * tw;	// �e�N�X�`���̍���X���W
			float ty = (float)(g_Interactables[i].patternAnim / g_Interactables[i].animDivideX) * th;	// �e�N�X�`���̍���Y���W

			//float tw = 1.0f;	// �e�N�X�`���̕�
			//float th = 1.0f;	// �e�N�X�`���̍���
			//float tx = 0.0f;	// �e�N�X�`���̍���X���W
			//float ty = 0.0f;	// �e�N�X�`���̍���Y���W

			// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
			SetSpriteColorRotation(g_VertexBuffer, px, py, pw, ph, tx, ty, tw, th,
				XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
				g_Interactables[i].rot.z);

			// �|���S���`��
			GetDeviceContext()->Draw(4, 0);
		}
	}

}


//=============================================================================
// Enemy�\���̂̐擪�A�h���X���擾
//=============================================================================
INTERACTABLE* GetInteractables(void)
{
	return &g_Interactables[0];
}


// �����Ă�G�l�~�[�̐�
int GetInteractablesCount(void)
{
	return g_ItemCount;
}

void SetInteractable(INTERACTABLE* interactable, BOOL active) 
{

	interactable->active = active;
}


