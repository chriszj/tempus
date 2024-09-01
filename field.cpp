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

static char g_TilemapFolder[] = "data/TILEMAP/";
static char g_dataFolder[] = "data/";
static char* g_debugTextures[1] = {
	"data/TEXTURE/bar_white.png",
};
static int debugTextureIndex = -1;

static TILESET g_Tilesets[TILESET_MAX];
static TILELAYER g_TileLayers[MAP_LAYER_MAX];
static FIELDOBJECTGROUP g_ObjectGroups[MAP_OBJGROUPS_MAX];

static BOOL		g_Load = FALSE;			// ���������s�������̃t���O
static TILE	 g_Tiles[TILES_PER_LAYER_MAX];		// �G�l�~�[�\����

static int	 g_TileCount = TILES_PER_LAYER_MAX;

static float offsetx = 200.0f;
static float offsety = 200.0f;

static float moveFactor = 200.0f;
static float tileTime = 25.0f;

void ParseTiles(TILELAYER* mapLayer, const char* rawData)
{
	// �^�C������
	g_TileCount = 0;

	int converted = 0;
	const char* tok = rawData;
	int i = 0;

	int tileIndex_X = 0;
	int tileIndex_Y = 0;

	// �f�[�^�̉��
	do
	{
		int tileId;

		converted = sscanf(tok, "%d", &tileId);

		tok = strchr(tok, ',');

		if (tok == NULL)
			break;

		TILE nTile;

		nTile.id = tileId;
		nTile.use = tileId > 0 ? true : false; // �^�C�� ID �� 0 �̏ꍇ�A�^�C���͕\������܂���B

		if (nTile.use) {

			TILESET* relatedTileSet = GetTilesetFromTileID(nTile.id);

			if (relatedTileSet != NULL) {

				float newX = (tileIndex_X * relatedTileSet->tileWidth) + relatedTileSet->tileWidth / 2;
				float newY = (tileIndex_Y * relatedTileSet->tileHeight) + relatedTileSet->tileWidth / 2;

				nTile.pos = XMFLOAT3(newX*MAP_SCALE, newY*MAP_SCALE, 0.0f);
				nTile.rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
				nTile.scl = XMFLOAT3(1.0f*MAP_SCALE, 1.0f*MAP_SCALE, 1.0f*MAP_SCALE);
				nTile.texNo = relatedTileSet->id;

				float tileSetTileWidth = relatedTileSet->tileWidth;
				float tileSetTileHeight = relatedTileSet->tileHeight;

				nTile.w = tileSetTileWidth*nTile.scl.x;
				nTile.h = tileSetTileHeight*nTile.scl.y;

				int textureIndex = nTile.id - relatedTileSet->firstGID;

				int textureUIndex = textureIndex % relatedTileSet->columns;
				int textureVIndex = (int)(textureIndex / (relatedTileSet->tileCount / relatedTileSet->columns));

				int tileSetTextureWidth = relatedTileSet->textureW;
				int tileSetTextureHeight = relatedTileSet->textureH;

				nTile.textureU = (textureUIndex * tileSetTileWidth) / tileSetTextureWidth;
				nTile.textureV = (textureVIndex * tileSetTileHeight) / tileSetTextureHeight;
				nTile.textureWidth = tileSetTileWidth / tileSetTextureWidth;
				nTile.textureHeigt = tileSetTileHeight / tileSetTextureHeight;

			}

		}

		mapLayer->tiles[i] = nTile;
		g_TileCount++;

		tileIndex_X++;

		if (tileIndex_X >mapLayer->width - 1)
		{
			tileIndex_X = 0;
			tileIndex_Y++;
		}


		tok += 1;

		i++;

	} while (converted != 0);

	
}

void ParseMap(TILESET tilesets[], TILELAYER tileLayers[], FIELDOBJECTGROUP objectGroups[], char* file)
{

	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(file);

	// �}�b�v�`�b�v�̏���
	if (result)
	{

		int tilesetNodeCount = 0;

		// �}�b�v�`�b�v�̃e�L�X�`���[�̏���
		for (pugi::xml_node xmltileset : doc.child("map").children("tileset"))
		{
			TILESET tileset;
			tileset.id = tilesetNodeCount;
			tileset.firstGID = xmltileset.attribute("firstgid").as_int();

			char path[128] = {};
			const char* source = xmltileset.attribute("source").value();

			memcpy(path, g_TilemapFolder, strlen(g_TilemapFolder));

			memcpy(path + strlen(path), source, strlen(source));

			memcpy(tileset.source, path, strlen(path));

			pugi::xml_document tilesetDoc;
			pugi::xml_parse_result tilesetDocResult = tilesetDoc.load_file(tileset.source);

			if (tilesetDocResult) {

				pugi::xml_node tilesetInnerNode = tilesetDoc.child("tileset");

				const char* name = tilesetInnerNode.attribute("name").value();
				memcpy(tileset.name, name, strlen(name));

				tileset.tileWidth = tilesetInnerNode.attribute("tilewidth").as_float();
				tileset.tileHeight = tilesetInnerNode.attribute("tileheight").as_float();
				tileset.tileCount = tilesetInnerNode.attribute("tilecount").as_int();
				tileset.columns = tilesetInnerNode.attribute("columns").as_int();

				pugi::xml_node tilesetTextureNode = tilesetInnerNode.child("image");

				const char* textureSource = tilesetTextureNode.attribute("source").value();

				memcpy(tileset.textureSource, g_dataFolder, strlen(g_dataFolder));

				memcpy(tileset.textureSource + strlen(tileset.textureSource), textureSource + 3, strlen(textureSource));

				tileset.textureW = tilesetTextureNode.attribute("width").as_int();
				tileset.textureH = tilesetTextureNode.attribute("height").as_int();

				tilesets[tilesetNodeCount] = tileset;
				tilesetNodeCount++;

			}
		}

		int tileLayerNodeCount = 0;

		// �}�b�v�`�b�v�̃��C���[�̏���
		for (pugi::xml_node tileLayerNode : doc.child("map").children("layer"))
		{

			/*std::string str = "Level is: ";
			str.append(tileLayerNode.attribute("name").value());

			OutputDebugStringA(str.c_str());*/

			tileLayers[tileLayerNodeCount].id = tileLayerNode.attribute("id").as_int();

			const char* tLName = tileLayerNode.attribute("name").value();
			memcpy(tileLayers[tileLayerNodeCount].name, tLName, strlen(tLName));

			const char* tLClass = tileLayerNode.attribute("class").value();
			memcpy(tileLayers[tileLayerNodeCount].layerClass, tLClass, strlen(tLClass));

			tileLayers[tileLayerNodeCount].width = tileLayerNode.attribute("width").as_float();
			tileLayers[tileLayerNodeCount].height = tileLayerNode.attribute("height").as_float();

			char debug[128] = "";

			memcpy(debug, tLClass, strlen(tLClass));

			int result = strcmp(debug, MAP_DEBUG_KEY);

			if (result == 0 && !MAP_DRAW_DEBUG)
			{
				tileLayerNodeCount++;
				continue;
			}

			ParseTiles(&tileLayers[tileLayerNodeCount], tileLayerNode.child("data").child_value());

			tileLayerNodeCount++;
		}

		int objectGroupNodeCount = 0;

		for (pugi::xml_node objectGroupNode : doc.child("map").children("objectgroup")) 
		{
		
			objectGroups[objectGroupNodeCount].id = objectGroupNode.attribute("id").as_int();

			const char* oGName = objectGroupNode.attribute("name").value();
			memcpy(objectGroups[objectGroupNodeCount].name, oGName, strlen(oGName));

			const char* oGClass = objectGroupNode.attribute("class").value();
			memcpy(objectGroups[objectGroupNodeCount].objectGroupClass, oGClass, strlen(oGClass));

			int objectsNodeCount = 0;

			for (pugi::xml_node fieldObjectNode : objectGroupNode.children("object")) {

				objectGroups[objectGroupNodeCount].fObjects[objectsNodeCount].id = fieldObjectNode.attribute("id").as_int();;

				const char* oName = fieldObjectNode.attribute("name").value();
				memcpy(objectGroups[objectGroupNodeCount].fObjects[objectsNodeCount].name, oName, strlen(oName));

				const char* oClass = fieldObjectNode.attribute("type").value();
				memcpy(objectGroups[objectGroupNodeCount].fObjects[objectsNodeCount].objectType, oClass, strlen(oClass));

				objectGroups[objectGroupNodeCount].fObjects[objectsNodeCount].width = fieldObjectNode.attribute("width").as_float() * MAP_SCALE;
				objectGroups[objectGroupNodeCount].fObjects[objectsNodeCount].height = fieldObjectNode.attribute("height").as_float() * MAP_SCALE;

				objectGroups[objectGroupNodeCount].fObjects[objectsNodeCount].x = (fieldObjectNode.attribute("x").as_float() * MAP_SCALE) + objectGroups[objectGroupNodeCount].fObjects[objectsNodeCount].width / 2;
				objectGroups[objectGroupNodeCount].fObjects[objectsNodeCount].y = (fieldObjectNode.attribute("y").as_float() * MAP_SCALE) + objectGroups[objectGroupNodeCount].fObjects[objectsNodeCount].height / 2;
				

				objectsNodeCount++;

			}

			objectGroupNodeCount++;			
			
		}

	}

}

//=============================================================================
// ����������
//=============================================================================
HRESULT InitField(void)
{
	ID3D11Device* pDevice = GetDevice();

	ParseMap(g_Tilesets,g_TileLayers, g_ObjectGroups, "data/TILEMAP/map.tmx");

	//�e�N�X�`������
	for (int i = 0; i < TILESET_MAX; i++)
	{

		g_Texture[i] = NULL;

		if (g_Tilesets[i].id < 0)
			continue;

		D3DX11CreateShaderResourceViewFromFile(GetDevice(),
			g_Tilesets[i].textureSource,
			NULL,
			NULL,
			&g_Texture[i],
			NULL);

	}

	// �ǂ̓����蔻��̃f�o�b�O�̃e�L�X�`���[
	if (MAP_DRAW_DEBUG_WALLS) {

		D3DX11CreateShaderResourceViewFromFile(GetDevice(),
			g_debugTextures[0],
			NULL,
			NULL,
			&g_Texture[TILESET_MAX-1],
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

	for (int ts = 0; ts < TILESET_MAX; ts++)
	{
		g_Tilesets[ts] = {};
	}

	for (int ml = 0; ml < MAP_LAYER_MAX; ml++)
	{
		for (int t = 0; t < TILES_PER_LAYER_MAX; t++)
		{
			g_TileLayers[ml].tiles[t] = {};
		}
		g_TileLayers[ml].Reset();
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
void DrawField(int layer)
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

	int tileLayersCount = sizeof(g_TileLayers) / sizeof(*g_TileLayers);

	if (layer >= tileLayersCount || layer < 0)
	{
		OutputDebugStringA("No layer MAP FOUND!");
		return;
	}

	TILELAYER* mapLayerToDraw = &g_TileLayers[layer];

	if (mapLayerToDraw->id < 0)
	{
		OutputDebugStringA("Map Layer is empty!");
		return;
	}

	for (int i = 0; i < TILES_PER_LAYER_MAX; i++)
	{
		if (mapLayerToDraw->tiles[i].use == TRUE)			// ���̃G�l�~�[���g���Ă���H
		{									// Yes
			// �e�N�X�`���ݒ�
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[mapLayerToDraw->tiles[i].texNo]);

			//�G�l�~�[�̈ʒu��e�N�X�`���[���W�𔽉f
			float px = mapLayerToDraw->tiles[i].pos.x - bg->pos.x;	// �G�l�~�[�̕\���ʒuX
			float py = mapLayerToDraw->tiles[i].pos.y - bg->pos.y;	// �G�l�~�[�̕\���ʒuY
			float pw = mapLayerToDraw->tiles[i].w;		// �G�l�~�[�̕\����
			float ph = mapLayerToDraw->tiles[i].h;		// �G�l�~�[�̕\������

			float tw = mapLayerToDraw->tiles[i].textureWidth;	// �e�N�X�`���̕�
			float th = mapLayerToDraw->tiles[i].textureHeigt;	// �e�N�X�`���̍���
			float tx = mapLayerToDraw->tiles[i].textureU;	// �e�N�X�`���̍���X���W
			float ty = mapLayerToDraw->tiles[i].textureV;	// �e�N�X�`���̍���Y���W

			// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
			SetSpriteColorRotation(g_VertexBuffer, px, py, pw, ph, tx, ty, tw, th,
				XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
				mapLayerToDraw->tiles[i].rot.z);

			// �|���S���`��
			GetDeviceContext()->Draw(4, 0);
		}
	}

	// �ǂ̓����蔻��̕\��
	if (!MAP_DRAW_DEBUG_WALLS)
		return;

	FIELDOBJECT* walls = GetFieldObjectsFromGroup(FOBJGROUP_WALL);
	for (int w = 0; w < MAP_OBJGRP_OBJ_MAX; w++) 
	{

		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[TILESET_MAX-1]);

		//�G�l�~�[�̈ʒu��e�N�X�`���[���W�𔽉f
		float px = walls[w].x - bg->pos.x;
		float py = walls[w].y - bg->pos.y;	// �G�l�~�[�̕\���ʒuY
		float pw = walls[w].width;		// �G�l�~�[�̕\����
		float ph = walls[w].height;		// �G�l�~�[�̕\������

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
			XMFLOAT4(0.0f, 0.0f, 1.0f, 0.2f),0.0f);

		// �|���S���`��
		GetDeviceContext()->Draw(4, 0);

	}

	PLAYER* player = GetPlayer();

	for (int p = 0; p < PLAYER_MAX; p++)
	{
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[TILESET_MAX - 1]);

		//�G�l�~�[�̈ʒu��e�N�X�`���[���W�𔽉f
		float px = player[p].pos.x + player[p].collider.offsetX - bg->pos.x;
		float py = player[p].pos.y + player[p].collider.offsetY - bg->pos.y;	// �G�l�~�[�̕\���ʒuY
		float pw = player[p].collider.width;		// �G�l�~�[�̕\����
		float ph = player[p].collider.height;		// �G�l�~�[�̕\������

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
			XMFLOAT4(0.0f, 1.0f, 0.0f, 0.2f), 0.0f);

		// �|���S���`��
		GetDeviceContext()->Draw(4, 0);
	}

}


//=============================================================================
// Field�\���̂̐擪�A�h���X���擾
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

TILESET* GetTilesetFromTileID(int tileId)
{
	for (int t = 0; t < TILESET_MAX; t++) {
	
		int tileIDRangeMinValue = g_Tilesets[t].firstGID;
		int tileIDrangeMaxValue = g_Tilesets[t].firstGID + g_Tilesets[t].tileCount - 1;

		if (tileId >= tileIDRangeMinValue && tileId <= tileIDrangeMaxValue) 
		{
			return &g_Tilesets[t];
		}
	
	}

	return NULL;

}

FIELDOBJECT** GetFieldObjectsByClass(const char* objectType)
{
	FIELDOBJECT* fieldObjects[MAP_OBJGRP_OBJ_MAX];

	int foundObjects = 0;

	for (int og = 0; og < MAP_OBJGROUPS_MAX; og++)
	{

		for (int o = 0; o < MAP_OBJGRP_OBJ_MAX; o++)
		{
			if (strcmp(g_ObjectGroups[og].fObjects[o].objectType, objectType) == 0)
			{
				fieldObjects[foundObjects] = &g_ObjectGroups[og].fObjects[o];
				foundObjects++;
			}
		}

	}

	if (foundObjects == 0)
		return NULL;

	return fieldObjects;

}

FIELDOBJECT* GetFieldObjectsFromGroup(const char* objectGroup)
{
	FIELDOBJECT *fieldObjects = NULL;

	int foundGroups = 0;

	for (int og = 0; og < MAP_OBJGROUPS_MAX; og++)
	{


		if (strcmp(g_ObjectGroups[og].objectGroupClass, objectGroup) == 0)
		{

			fieldObjects = g_ObjectGroups[og].fObjects;
						
			foundGroups++;

			// ���݁A�����ɂ͈�̃O���[�v�݂̂̕Ԃ����ł��܂��B
			break;
		}

	}

	return fieldObjects;

}

FIELDOBJECT* GetFieldObjectByNameFromGroup(const char* objectGroup, const char* objectName)
{
	FIELDOBJECT* fieldObject = NULL;

	int foundGroups = 0;

	for (int og = 0; og < MAP_OBJGROUPS_MAX; og++)
	{


		if (strcmp(g_ObjectGroups[og].objectGroupClass, objectGroup) == 0)
		{

			for (int o = 0; o < MAP_OBJGRP_OBJ_MAX; o++)
			{

				if (strcmp(g_ObjectGroups[og].objectGroupClass, objectGroup) == 0) 
				{
					fieldObject = &g_ObjectGroups[og].fObjects[o];


					// ���݁A�����ɂ͍ŏ��̃I�u�W�F�N�g�݂̂̕Ԃ����ł��܂��B
					break;
				}

			}

			foundGroups++;

			// ���݁A�����ɂ͍ŏ��̃O���[�v�݂̂̕Ԃ����ł��܂��B
			break;
		}

	}

	return fieldObject;

}


