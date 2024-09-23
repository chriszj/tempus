//=============================================================================
//
// �G�l�~�[���� [enemy.cpp]
// Author : 
//
//=============================================================================
#include "props.h"
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
void PushToTimeState(TIMESTATE* timeState, PROP* interactable);
void PullFromTimeState(TIMESTATE* timeState, PROP* interactable);

//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static ID3D11Buffer				*g_VertexBuffer = NULL;				// ���_���
static ID3D11ShaderResourceView	*g_Texture[TILESET_CUSTOM_TILES_MAX] = { NULL };	// �e�N�X�`�����

static BOOL		g_Load = FALSE;			// ���������s�������̃t���O
static PROP	g_Props[PROPS_MAX];		// �G�l�~�[�\����

static int		g_InteractablesCount = PROPS_MAX;

static float offsetx = 200.0f;
static float offsety = 200.0f;

static TILESET* g_PropsTileset;

//=============================================================================
// ����������
//=============================================================================
HRESULT InitProps(void)
{
	MAPOBJECT* mapObjects = GetMapObjectsFromLayer(MAPOBJLAYER_PROPS);

	ID3D11Device *pDevice = GetDevice();

	g_PropsTileset = GetTilesetByName(TILESET_PROPS_NAME);

	//�e�N�X�`������
	for (int i = 0; i < TILESET_CUSTOM_TILES_MAX; i++)
	{

		if (g_PropsTileset->customTiles[i].id < 0)
			continue;

		g_Texture[i] = NULL;
		D3DX11CreateShaderResourceViewFromFile(GetDevice(),
			g_PropsTileset->customTiles[i].textureSource,
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
	g_InteractablesCount = 0;
	for (int i = 0; i < PROPS_MAX; i++)
	{

		g_InteractablesCount++;
		g_Props[i].use = mapObjects[i].id > 0? TRUE: FALSE;

		if (!g_Props[i].use)
			continue;

		g_Props[i].active = TRUE;
		
		g_Props[i].pos = XMFLOAT3((mapObjects[i].x ), (mapObjects[i].y - (mapObjects[i].height)), 0.0f);
		g_Props[i].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Props[i].scl = XMFLOAT3(1.0f, 1.0f, 1.0f);
		g_Props[i].w = mapObjects[i].width;
		g_Props[i].h = mapObjects[i].height;
		g_Props[i].texNo = mapObjects[i].gid - g_PropsTileset->firstGID;
		//�@�g���Ă���e�L�X�`���[�Ŏ�ނ��킩���
		g_Props[i].id = g_Props[i].texNo;

		g_Props[i].countAnim = 0;

		g_Props[i].animDivideX = g_PropsTileset->customTiles[g_Props[i].texNo].animDivideX;
		g_Props[i].animDivideY = g_PropsTileset->customTiles[g_Props[i].texNo].animDivideY;
		g_Props[i].patternAnimNum = g_PropsTileset->customTiles[g_Props[i].texNo].patternAnimNum;

		g_Props[i].move = XMFLOAT3(4.0f, 0.0f, 0.0f);		// �ړ���

		PushToTimeState(&g_Props[i].timeState, &g_Props[i]);
		RegisterObjectTimeState(&g_Props[i].timeState);
	}

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// �I������
//=============================================================================
void UninitProps(void)
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

	for (int e = 0; e < PROPS_MAX; e++)
	{
		g_Props[e].use = FALSE;
	}

	g_Load = FALSE;
}

//=============================================================================
// �X�V����
//=============================================================================
void UpdateProps(void)
{
	if (g_Load == FALSE) return;
	g_InteractablesCount = 0;			// �����Ă�G�l�~�[�̐�

	for (int i = 0; i < PROPS_MAX; i++)
	{
		// �����Ă�G�l�~�[��������������
		if (g_Props[i].use == TRUE)
		{
			if (g_Props[i].id == 1);
				g_InteractablesCount++;		// �����Ă��G�̐�

			if (g_Props[i].timeState.status == READ_ONLY)
			{

				PullFromTimeState(&g_Props[i].timeState, &g_Props[i]);
				continue;
			}
			
			// �n�`�Ƃ̓����蔻��p�ɍ��W�̃o�b�N�A�b�v������Ă���
			XMFLOAT3 pos_old = g_Props[i].pos;

			// �A�j���[�V����  
			g_Props[i].countAnim += 1.0f;
			if (g_Props[i].countAnim > ANIM_WAIT)
			{
				g_Props[i].countAnim = 0.0f;

				int nextFrame = 1;

				g_Props[i].patternAnim = (g_Props[i].patternAnim + nextFrame) % g_Props[i].patternAnimNum;
			}

			PushToTimeState(&g_Props[i].timeState, &g_Props[i]);
		}
	}

#ifdef _DEBUG	// �f�o�b�O����\������


#endif

}

//=============================================================================
// �`�揈��
//=============================================================================
void DrawProps(void)
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

	for (int i = 0; i < PROPS_MAX; i++)
	{
		if (g_Props[i].use == TRUE)			// ���̃G�l�~�[���g���Ă���H
		{									// Yes
			// �e�N�X�`���ݒ�
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_Props[i].texNo]);

			//�G�l�~�[�̈ʒu��e�N�X�`���[���W�𔽉f
			float px = g_Props[i].pos.x - bg->pos.x;	// �G�l�~�[�̕\���ʒuX
			float py = g_Props[i].pos.y - bg->pos.y;	// �G�l�~�[�̕\���ʒuY
			float pw = g_Props[i].w;		// �G�l�~�[�̕\����
			float ph = g_Props[i].h;		// �G�l�~�[�̕\������

			/*int customTileWidth = g_ItemsTileset->customTiles[g_Item[i].texNo].width;
			int customTileTextureW = g_ItemsTileset->customTiles[g_Item[i].texNo].textureW;
			int divideX = customTileTextureW / customTileWidth;

			int customTileHeight = g_ItemsTileset->customTiles[g_Item[i].texNo].height;
			int customTileTextureH = g_ItemsTileset->customTiles[g_Item[i].texNo].textureH;
			int divideY = customTileTextureH / customTileHeight;*/

			// �A�j���[�V�����p
			float tw = 1.0f / g_Props[i].animDivideX;	// �e�N�X�`���̕�
			float th = 1.0f / g_Props[i].animDivideY;	// �e�N�X�`���̍���
			float tx = (float)(g_Props[i].patternAnim % g_Props[i].animDivideX) * tw;	// �e�N�X�`���̍���X���W
			float ty = (float)(g_Props[i].patternAnim / g_Props[i].animDivideX) * th;	// �e�N�X�`���̍���Y���W

			//float tw = 1.0f;	// �e�N�X�`���̕�
			//float th = 1.0f;	// �e�N�X�`���̍���
			//float tx = 0.0f;	// �e�N�X�`���̍���X���W
			//float ty = 0.0f;	// �e�N�X�`���̍���Y���W

			// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
			SetSpriteColorRotation(g_VertexBuffer, px, py, pw, ph, tx, ty, tw, th,
				XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
				g_Props[i].rot.z);

			// �|���S���`��
			GetDeviceContext()->Draw(4, 0);
		}
	}

}

void PushToTimeState(TIMESTATE* timeState, PROP* interactable)
{
	timeState->active = interactable->active;
	timeState->countAnim = interactable->countAnim;
	timeState->patternAnim = interactable->patternAnim;
}

void PullFromTimeState(TIMESTATE* timeState, PROP* interactable)
{
	interactable->active = timeState->active;
	interactable->countAnim = timeState->countAnim;
	interactable->patternAnim = timeState->patternAnim;
}


//=============================================================================
// ������\���̂̐擪�A�h���X���擾
//=============================================================================
PROP* GetProps(void)
{
	return &g_Props[0];
}


// �����Ă�G�l�~�[�̐�
int GetPropsCount(void)
{
	return g_InteractablesCount;
}




