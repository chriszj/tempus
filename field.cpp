//=============================================================================
//
// �G�l�~�[���� [enemy.cpp]
// Author : 
//
//=============================================================================
#include "field.h"
#include "bg.h"
#include "player.h"
#include "fade.h"
#include "collision.h"
#include "pugixml.hpp"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define TEXTURE_WIDTH				(200/2)	// �L�����T�C�Y
#define TEXTURE_HEIGHT				(200/2)	// 
#define TEXTURE_MAX					(2)		// �e�N�X�`���̐�

#define TEXTURE_PATTERN_DIVIDE_X	(1)		// �A�j���p�^�[���̃e�N�X�`�����������iX)
#define TEXTURE_PATTERN_DIVIDE_Y	(1)		// �A�j���p�^�[���̃e�N�X�`�����������iY)
#define ANIM_PATTERN_NUM			(TEXTURE_PATTERN_DIVIDE_X*TEXTURE_PATTERN_DIVIDE_Y)	// �A�j���[�V�����p�^�[����
#define ANIM_WAIT					(4)		// �A�j���[�V�����̐؂�ւ��Wait�l


//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************


//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static ID3D11Buffer				*g_VertexBuffer = NULL;				// ���_���
static ID3D11ShaderResourceView	*g_Texture[TILESET_MAX] = { NULL };	// �e�N�X�`�����

static char* g_TilemapFolder = "data/TILEMAP/";

static TILESET g_Tilesets[TILESET_MAX];
static MAPLAYER g_MapLayers[MAP_LAYER_MAX];

static BOOL		g_Load = FALSE;			// ���������s�������̃t���O
static TILE	 g_Tiles[TILES_PER_LAYER_MAX];		// �G�l�~�[�\����

static int	 g_TileCount = TILES_PER_LAYER_MAX;

static float offsetx = 200.0f;
static float offsety = 200.0f;

static float moveFactor = 200.0f;
static float tileTime = 25.0f;



//=============================================================================
// ����������
//=============================================================================
HRESULT InitField(void)
{
	ID3D11Device* pDevice = GetDevice();

	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file("data/TILEMAP/map.tmx");

	// �}�b�v�`�b�v�̏���
	if (result)
	{

		int tilesetNodeCount = 0;

		// �}�b�v�`�b�v�̃e�L�X�`���[�̏���
		for (pugi::xml_node xmltileset : doc.child("map").children("tileset"))
		{
			TILESET tileset;
			tileset.firstgid = xmltileset.attribute("firstgid").as_int();

			char path[128] = {};
			const char* source = xmltileset.attribute("source").value();

			memcpy(path, g_TilemapFolder, strlen(g_TilemapFolder));

			memcpy(path + strlen(path), source, strlen(source));


			tileset.source = path;

			pugi::xml_document tilesetDoc;
			pugi::xml_parse_result tilesetDocResult = tilesetDoc.load_file(tileset.source);

			if (tilesetDocResult) {

				pugi::xml_node tilesetInnerNode = tilesetDoc.child("tileset");

				tileset.name = tilesetInnerNode.attribute("name").value();
				tileset.tilewidth = tilesetInnerNode.attribute("name").as_int();
				tileset.tileheight = tilesetInnerNode.attribute("tileheight").as_int();
				tileset.tilecount = tilesetInnerNode.attribute("tilecount").as_int();
				tileset.columns = tilesetInnerNode.attribute("columns").as_int();

				pugi::xml_node tilesetTextureNode = tilesetInnerNode.child("image");

				tileset.textureSource = tilesetTextureNode.attribute("source").value();
				tileset.textureW = tilesetTextureNode.attribute("width").as_int();
				tileset.textureH = tilesetTextureNode.attribute("height").as_int();

				g_Tilesets[tilesetNodeCount] = tileset;
				tilesetNodeCount++;

			}
		}

		int mapLayerNodeCount = 0;

		// �}�b�v�`�b�v�̃��C���[�̏���
		for (pugi::xml_node mapLayerNode : doc.child("map").children("layer"))
		{


			std::string str = "Level is: ";
			str.append(mapLayerNode.attribute("name").value());

			OutputDebugStringA(str.c_str());


			MAPLAYER mapLayer;

			mapLayer.id = mapLayerNode.attribute("id").as_int();
			mapLayer.name = mapLayerNode.attribute("name").value();
			mapLayer.width = mapLayerNode.attribute("width").as_int();
			mapLayer.height = mapLayerNode.attribute("height").as_int();

			mapLayer.rawData = mapLayerNode.child("data").child_value();

			g_MapLayers[mapLayerNodeCount] = mapLayer;

			mapLayerNodeCount++;
		}

	}

	//�e�N�X�`������
	for (int i = 0; i < TILESET_MAX; i++)
	{

		g_Texture[i] = NULL;

		if (g_Tilesets[i].firstgid < 0)
			continue;

		D3DX11CreateShaderResourceViewFromFile(GetDevice(),
			g_Tilesets[i].textureSource,
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


	// �^�C������
	g_TileCount = 0;

	for (int ml = 0; ml < MAP_LAYER_MAX; ml++)
	{
	
		//int getSize(char* s) {
		//	char* t; // first copy the pointer to not change the original
		//	int size = 0;

		//	for (t = s; *t != '\0'; t++) {
		//		size++;
		//	}

		//	return size;
		//}
	
	}

	for (int i = 0; i < TILES_PER_LAYER_MAX; i++)
	{
		g_TileCount++;
		g_Tiles[i].use = TRUE;
		g_Tiles[i].pos = XMFLOAT3(200.0f + i*200.0f, 100.0f, 0.0f);	// ���S�_����\��
		g_Tiles[i].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Tiles[i].scl = XMFLOAT3(1.0f, 1.0f, 1.0f);
		g_Tiles[i].w = TEXTURE_WIDTH;
		g_Tiles[i].h = TEXTURE_HEIGHT;
		g_Tiles[i].texNo = 0;

		g_Tiles[i].countAnim = 0;
		g_Tiles[i].patternAnim = 0;

		g_Tiles[i].move = XMFLOAT3(4.0f, 0.0f, 0.0f);		// �ړ���

		g_Tiles[i].tileTime = 0.0f;			// ���`��ԗp�̃^�C�}�[���N���A
		g_Tiles[i].tblNo = 0;			// �Đ�����s���f�[�^�e�[�u��No���Z�b�g
		g_Tiles[i].tblMax = 0;			// �Đ�����s���f�[�^�e�[�u���̃��R�[�h�����Z�b�g
	}

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// �I������
//=============================================================================
void UninitField(void)
{
	if (g_Load == FALSE) return;

	if (g_VertexBuffer)
	{
		g_VertexBuffer->Release();
		g_VertexBuffer = NULL;
	}

	for (int i = 0; i < TILESET_MAX; i++)
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
void UpdateField(void)
{
	if (g_Load == FALSE) return;
	g_TileCount = 0;			// �����Ă�G�l�~�[�̐�

	for (int i = 0; i < TILES_PER_LAYER_MAX; i++)
	{
		// �����Ă�G�l�~�[��������������
		if (g_Tiles[i].use == TRUE)
		{
			g_TileCount++;		// �����Ă��G�̐�
			
			// �n�`�Ƃ̓����蔻��p�ɍ��W�̃o�b�N�A�b�v������Ă���
			XMFLOAT3 pos_old = g_Tiles[i].pos;

			// �A�j���[�V����  
			g_Tiles[i].countAnim += 1.0f;
			if (g_Tiles[i].countAnim > ANIM_WAIT)
			{
				g_Tiles[i].countAnim = 0.0f;
				// �p�^�[���̐؂�ւ�
				g_Tiles[i].patternAnim = (g_Tiles[i].patternAnim + 1) % ANIM_PATTERN_NUM;
			}

			// �ړ�����
			//if (g_Tiles[i].tblMax > 0)	// ���`��Ԃ����s����H
			//{	// ���`��Ԃ̏���
			//	int nowNo = (int)g_Tiles[i].tileTime;			// �������ł���e�[�u���ԍ������o���Ă���
			//	int maxNo = g_Tiles[i].tblMax;				// �o�^�e�[�u�����𐔂��Ă���
			//	int nextNo = (nowNo + 1) % maxNo;			// �ړ���e�[�u���̔ԍ������߂Ă���
			//	INTERPOLATION_DATA* tbl = g_MoveTblAdr[g_Tiles[i].tblNo];	// �s���e�[�u���̃A�h���X���擾
			//	
			//	XMVECTOR nowPos = XMLoadFloat3(&tbl[nowNo].pos);	// XMVECTOR�֕ϊ�
			//	XMVECTOR nowRot = XMLoadFloat3(&tbl[nowNo].rot);	// XMVECTOR�֕ϊ�
			//	XMVECTOR nowScl = XMLoadFloat3(&tbl[nowNo].scl);	// XMVECTOR�֕ϊ�
			//	
			//	XMVECTOR Pos = XMLoadFloat3(&tbl[nextNo].pos) - nowPos;	// XYZ�ړ��ʂ��v�Z���Ă���
			//	XMVECTOR Rot = XMLoadFloat3(&tbl[nextNo].rot) - nowRot;	// XYZ��]�ʂ��v�Z���Ă���
			//	XMVECTOR Scl = XMLoadFloat3(&tbl[nextNo].scl) - nowScl;	// XYZ�g�嗦���v�Z���Ă���
			//	
			//	float nowTime = g_Tiles[i].tileTime - nowNo;	// ���ԕ����ł��鏭�������o���Ă���
			//	
			//	Pos *= nowTime;								// ���݂̈ړ��ʂ��v�Z���Ă���
			//	Rot *= nowTime;								// ���݂̉�]�ʂ��v�Z���Ă���
			//	Scl *= nowTime;								// ���݂̊g�嗦���v�Z���Ă���

			//	// �v�Z���ċ��߂��ړ��ʂ����݂̈ړ��e�[�u��XYZ�ɑ����Ă��遁�\�����W�����߂Ă���
			//	XMStoreFloat3(&g_Tiles[i].pos, nowPos + Pos);

			//	// �v�Z���ċ��߂���]�ʂ����݂̈ړ��e�[�u���ɑ����Ă���
			//	XMStoreFloat3(&g_Tiles[i].rot, nowRot + Rot);

			//	// �v�Z���ċ��߂��g�嗦�����݂̈ړ��e�[�u���ɑ����Ă���
			//	XMStoreFloat3(&g_Tiles[i].scl, nowScl + Scl);
			//	g_Tiles[i].w = TEXTURE_WIDTH * g_Tiles[i].scl.x;
			//	g_Tiles[i].h = TEXTURE_HEIGHT * g_Tiles[i].scl.y;

			//	// frame���g�Ď��Ԍo�ߏ���������
			//	g_Tiles[i].tileTime += 1.0f / tbl[nowNo].frame;	// ���Ԃ�i�߂Ă���
			//	if ((int)g_Tiles[i].tileTime >= maxNo)			// �o�^�e�[�u���Ō�܂ňړ��������H
			//	{
			//		g_Tiles[i].tileTime -= maxNo;				// �O�ԖڂɃ��Z�b�g�������������������p���ł���
			//	}

			//}

			// �ړ����I�������G�l�~�[�Ƃ̓����蔻��
			{
				PLAYER* player = GetPlayer();

				// �G�l�~�[�̐��������蔻����s��
				for (int j = 0; j < TILES_PER_LAYER_MAX; j++)
				{
					// �����Ă�G�l�~�[�Ɠ����蔻�������
					if (player[j].use == TRUE)
					{
						BOOL ans = CollisionBB(g_Tiles[i].pos, g_Tiles[i].w, g_Tiles[i].h,
							player[j].pos, player[j].w, player[j].h);
						// �������Ă���H
						if (ans == TRUE)
						{
							// �����������̏���
							player[j].use = FALSE;	// �f�o�b�O�ňꎞ�I�ɖ��G�ɂ��Ă�����
						}
					}
				}
			}
		}
	}


	//// �G�l�~�[�S�Ń`�F�b�N
	//if (g_EnemyCount <= 0)
	//{
 //		SetFade(FADE_OUT, MODE_RESULT);
	//}

#ifdef _DEBUG	// �f�o�b�O����\������


#endif

}

//=============================================================================
// �`�揈��
//=============================================================================
void DrawField(void)
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

	for (int i = 0; i < TILES_PER_LAYER_MAX; i++)
	{
		if (g_Tiles[i].use == TRUE)			// ���̃G�l�~�[���g���Ă���H
		{									// Yes
			// �e�N�X�`���ݒ�
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_Tiles[i].texNo]);

			//�G�l�~�[�̈ʒu��e�N�X�`���[���W�𔽉f
			float px = g_Tiles[i].pos.x - bg->pos.x;	// �G�l�~�[�̕\���ʒuX
			float py = g_Tiles[i].pos.y - bg->pos.y;	// �G�l�~�[�̕\���ʒuY
			float pw = g_Tiles[i].w;		// �G�l�~�[�̕\����
			float ph = g_Tiles[i].h;		// �G�l�~�[�̕\������

			// �A�j���[�V�����p
			//float tw = 1.0f / TEXTURE_PATTERN_DIVIDE_X;	// �e�N�X�`���̕�
			//float th = 1.0f / TEXTURE_PATTERN_DIVIDE_Y;	// �e�N�X�`���̍���
			//float tx = (float)(g_Player[i].patternAnim % TEXTURE_PATTERN_DIVIDE_X) * tw;	// �e�N�X�`���̍���X���W
			//float ty = (float)(g_Player[i].patternAnim / TEXTURE_PATTERN_DIVIDE_X) * th;	// �e�N�X�`���̍���Y���W

			float tw = 1.0f;	// �e�N�X�`���̕�
			float th = 1.0f;	// �e�N�X�`���̍���
			float tx = 0.0f;	// �e�N�X�`���̍���X���W
			float ty = 0.0f;	// �e�N�X�`���̍���Y���W

			// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
			SetSpriteColorRotation(g_VertexBuffer, px, py, pw, ph, tx, ty, tw, th,
				XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
				g_Tiles[i].rot.z);

			// �|���S���`��
			GetDeviceContext()->Draw(4, 0);
		}
	}


	// �Q�[�W�̃e�X�g
	{
		// ���~���̃Q�[�W�i�g�I�ȕ��j
		// �e�N�X�`���ݒ�
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[1]);

		//�Q�[�W�̈ʒu��e�N�X�`���[���W�𔽉f
		float px = 600.0f;		// �Q�[�W�̕\���ʒuX
		float py =  10.0f;		// �Q�[�W�̕\���ʒuY
		float pw = 300.0f;		// �Q�[�W�̕\����
		float ph =  30.0f;		// �Q�[�W�̕\������

		float tw = 1.0f;	// �e�N�X�`���̕�
		float th = 1.0f;	// �e�N�X�`���̍���
		float tx = 0.0f;	// �e�N�X�`���̍���X���W
		float ty = 0.0f;	// �e�N�X�`���̍���Y���W

		// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
		SetSpriteLTColor(g_VertexBuffer,
			px, py, pw, ph,
			tx, ty, tw, th,
			XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f));

		// �|���S���`��
		GetDeviceContext()->Draw(4, 0);


		// �G�l�~�[�̐��ɏ]���ăQ�[�W�̒�����\�����Ă݂�
		// �e�N�X�`���ݒ�
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[1]);

		//�Q�[�W�̈ʒu��e�N�X�`���[���W�𔽉f
		pw = pw * ((float)g_TileCount / TILES_PER_LAYER_MAX);

		// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
		SetSpriteLTColor(g_VertexBuffer,
			px, py, pw, ph,
			tx, ty, tw, th,
			XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f));

		// �|���S���`��
		GetDeviceContext()->Draw(4, 0);


	}




}


//=============================================================================
// Enemy�\���̂̐擪�A�h���X���擾
//=============================================================================
TILE* GetField(void)
{
	return &g_Tiles[0];
}


// �����Ă�G�l�~�[�̐�
int GetFieldCount(void)
{
	return g_TileCount;
}


