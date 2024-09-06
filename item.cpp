//=============================================================================
//
// �G�l�~�[���� [enemy.cpp]
// Author : 
//
//=============================================================================
#include "item.h"
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
static ITEM	g_Item[ITEM_MAX];		// �G�l�~�[�\����

static int		g_ItemCount = ITEM_MAX;

static float offsetx = 200.0f;
static float offsety = 200.0f;

static TILESET* g_ItemsTileset;



//=============================================================================
// ����������
//=============================================================================
HRESULT InitItem(void)
{
	MAPOBJECT* mapObjects = GetMapObjectsFromLayer(MAPOBJLAYER_ITEMS);

	ID3D11Device *pDevice = GetDevice();

	g_ItemsTileset = GetTilesetByName(TILESET_ITEMS_NAME);

	//�e�N�X�`������
	for (int i = 0; i < TILESET_CUSTOM_TILES_MAX; i++)
	{

		if (g_ItemsTileset->customTiles[i].id < 0)
			continue;

		g_Texture[i] = NULL;
		D3DX11CreateShaderResourceViewFromFile(GetDevice(),
			g_ItemsTileset->customTiles[i].textureSource,
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
	for (int i = 0; i < ITEM_MAX; i++)
	{

		g_ItemCount++;
		g_Item[i].use = mapObjects[i].id > 0? TRUE: FALSE;

		if (!g_Item[i].use)
			continue;

		
		g_Item[i].pos = XMFLOAT3((mapObjects[i].x ), (mapObjects[i].y - (mapObjects[i].height)), 0.0f);
		g_Item[i].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Item[i].scl = XMFLOAT3(1.0f, 1.0f, 1.0f);
		g_Item[i].w = mapObjects[i].width;
		g_Item[i].h = mapObjects[i].height;
		g_Item[i].texNo = mapObjects[i].gid - g_ItemsTileset->firstGID;
		//�@�g���Ă���e�L�X�`���[�Ŏ�ނ��킩���
		g_Item[i].id = g_Item[i].texNo;

		g_Item[i].countAnim = 0;
		g_Item[i].patternAnim = 0;

		int customTileWidth = g_ItemsTileset->customTiles[g_Item[i].texNo].width;
		int customTileTextureW = g_ItemsTileset->customTiles[g_Item[i].texNo].textureW;

		int divideX = customTileTextureW / customTileWidth;

		int customTileHeight = g_ItemsTileset->customTiles[g_Item[i].texNo].height;
		int customTileTextureH = g_ItemsTileset->customTiles[g_Item[i].texNo].textureH;
		int divideY = customTileTextureH / customTileHeight;

		g_Item[i].animDivideX = divideX;
		g_Item[i].animDivideY = divideY;
		g_Item[i].patternAnimNum = divideX*divideY;

		g_Item[i].move = XMFLOAT3(4.0f, 0.0f, 0.0f);		// �ړ���

		
	}

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// �I������
//=============================================================================
void UninitItem(void)
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

	g_Load = FALSE;
}

//=============================================================================
// �X�V����
//=============================================================================
void UpdateItem(void)
{
	if (g_Load == FALSE) return;
	g_ItemCount = 0;			// �����Ă�G�l�~�[�̐�

	for (int i = 0; i < ITEM_MAX; i++)
	{
		// �����Ă�G�l�~�[��������������
		if (g_Item[i].use == TRUE)
		{
			g_ItemCount++;		// �����Ă��G�̐�
			
			// �n�`�Ƃ̓����蔻��p�ɍ��W�̃o�b�N�A�b�v������Ă���
			XMFLOAT3 pos_old = g_Item[i].pos;

			// �A�j���[�V����  
			g_Item[i].countAnim += 1.0f;
			if (g_Item[i].countAnim > ANIM_WAIT)
			{
				g_Item[i].countAnim = 0.0f;
				// �p�^�[���̐؂�ւ�
				g_Item[i].patternAnim = (g_Item[i].patternAnim + 1) % g_Item[i].patternAnimNum;
			}

			// �ړ����I�������G�l�~�[�Ƃ̓����蔻��
			{
				PLAYER* player = GetPlayer();

				// �G�l�~�[�̐��������蔻����s��
				for (int j = 0; j < ITEM_MAX; j++)
				{
					// �����Ă�G�l�~�[�Ɠ����蔻�������
					if (player[j].use == TRUE)
					{
						BOOL ans = CollisionBB(g_Item[i].pos, g_Item[i].w, g_Item[i].h,
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
void DrawItem(void)
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

	for (int i = 0; i < ITEM_MAX; i++)
	{
		if (g_Item[i].use == TRUE)			// ���̃G�l�~�[���g���Ă���H
		{									// Yes
			// �e�N�X�`���ݒ�
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_Item[i].texNo]);

			//�G�l�~�[�̈ʒu��e�N�X�`���[���W�𔽉f
			float px = g_Item[i].pos.x - bg->pos.x;	// �G�l�~�[�̕\���ʒuX
			float py = g_Item[i].pos.y - bg->pos.y;	// �G�l�~�[�̕\���ʒuY
			float pw = g_Item[i].w;		// �G�l�~�[�̕\����
			float ph = g_Item[i].h;		// �G�l�~�[�̕\������

			/*int customTileWidth = g_ItemsTileset->customTiles[g_Item[i].texNo].width;
			int customTileTextureW = g_ItemsTileset->customTiles[g_Item[i].texNo].textureW;
			int divideX = customTileTextureW / customTileWidth;

			int customTileHeight = g_ItemsTileset->customTiles[g_Item[i].texNo].height;
			int customTileTextureH = g_ItemsTileset->customTiles[g_Item[i].texNo].textureH;
			int divideY = customTileTextureH / customTileHeight;*/

			// �A�j���[�V�����p
			float tw = 1.0f / g_Item[i].animDivideX;	// �e�N�X�`���̕�
			float th = 1.0f / g_Item[i].animDivideY;	// �e�N�X�`���̍���
			float tx = (float)(g_Item[i].patternAnim % g_Item[i].animDivideX) * tw;	// �e�N�X�`���̍���X���W
			float ty = (float)(g_Item[i].patternAnim / g_Item[i].animDivideX) * th;	// �e�N�X�`���̍���Y���W

			//float tw = 1.0f;	// �e�N�X�`���̕�
			//float th = 1.0f;	// �e�N�X�`���̍���
			//float tx = 0.0f;	// �e�N�X�`���̍���X���W
			//float ty = 0.0f;	// �e�N�X�`���̍���Y���W

			// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
			SetSpriteColorRotation(g_VertexBuffer, px, py, pw, ph, tx, ty, tw, th,
				XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
				g_Item[i].rot.z);

			// �|���S���`��
			GetDeviceContext()->Draw(4, 0);
		}
	}

}


//=============================================================================
// Enemy�\���̂̐擪�A�h���X���擾
//=============================================================================
ITEM* GetItem(void)
{
	return &g_Item[0];
}


// �����Ă�G�l�~�[�̐�
int GetItemCount(void)
{
	return g_ItemCount;
}


