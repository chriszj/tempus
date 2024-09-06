//=============================================================================
//
// エネミー処理 [enemy.cpp]
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
// マクロ定義
//*****************************************************************************
#define TEXTURE_WIDTH				(200/2)	// キャラサイズ
#define TEXTURE_HEIGHT				(200/2)	// 
#define TEXTURE_MAX					(2)		// テクスチャの数

#define TEXTURE_PATTERN_DIVIDE_X	(1)		// アニメパターンのテクスチャ内分割数（X)
#define TEXTURE_PATTERN_DIVIDE_Y	(1)		// アニメパターンのテクスチャ内分割数（Y)
#define ANIM_PATTERN_NUM			(TEXTURE_PATTERN_DIVIDE_X*TEXTURE_PATTERN_DIVIDE_Y)	// アニメーションパターン数
#define ANIM_WAIT					(4)		// アニメーションの切り替わるWait値


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
void ParseTiles(MAPTILELAYER* mapTileLayer, const char* rawData);
void ParseMap(TILESET tilesets[], MAPTILELAYER mapTileLayers[], FIELDOBJECTGROUP objectGroups[], char* file);

//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer				*g_VertexBuffer = NULL;				// 頂点情報
static ID3D11ShaderResourceView	*g_Texture[TILESET_MAX] = { NULL };	// テクスチャ情報

static char g_TilemapFolder[] = "data/TILEMAP/Crypt/";
static char g_dataFolder[] = "data/";
static char* g_debugTextures[1] = {
	"data/TEXTURE/bar_white.png",
};
static int debugTextureIndex = -1;

static TILESET g_Tilesets[TILESET_MAX];
static MAPTILELAYER g_MapTileLayers[MAP_LAYER_MAX];
static FIELDOBJECTGROUP g_ObjectGroups[MAP_OBJLAYERS_MAX];

static BOOL		g_Load = FALSE;			// 初期化を行ったかのフラグ
static TILE	 g_Tiles[TILES_PER_LAYER_MAX];		// タイル構造体

static int	 g_TileCount = TILES_PER_LAYER_MAX;

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitField(void)
{
	ID3D11Device* pDevice = GetDevice();

	ParseMap(g_Tilesets,g_MapTileLayers, g_ObjectGroups, "data/TILEMAP/Crypt/level1.tmx");

	//テクスチャ生成
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

	// 壁の当たり判定のデバッグのテキスチャー
	if (MAP_DRAW_DEBUG_WALLS) {

		D3DX11CreateShaderResourceViewFromFile(GetDevice(),
			g_debugTextures[0],
			NULL,
			NULL,
			&g_Texture[TILESET_MAX-1],
			NULL);

	}


	// 頂点バッファ生成
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
// 終了処理
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
			g_MapTileLayers[ml].tiles[t] = {};
		}
		g_MapTileLayers[ml].Reset();
	}

	g_Load = FALSE;
}

//=============================================================================
// 更新処理
//=============================================================================
void UpdateField(void)
{
	if (g_Load == FALSE) return;
	


	//// エネミー全滅チェック
	//if (g_EnemyCount <= 0)
	//{
 //		SetFade(FADE_OUT, MODE_RESULT);
	//}

#ifdef _DEBUG	// デバッグ情報を表示する


#endif

}

//=============================================================================
// 描画処理
//=============================================================================
void DrawField(int layer)
{

	// 頂点バッファ設定
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

	// マトリクス設定
	SetWorldViewProjection2D();

	// プリミティブトポロジ設定
	GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// マテリアル設定
	MATERIAL material;
	ZeroMemory(&material, sizeof(material));
	material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	SetMaterial(material);

	BG* bg = GetBG();

	int mapTileLayersCount = sizeof(g_MapTileLayers) / sizeof(*g_MapTileLayers);

	if (layer >= mapTileLayersCount || layer < 0)
	{
		OutputDebugStringA("No layer MAP FOUND!");
		return;
	}

	MAPTILELAYER* mapTileLayerToDraw = &g_MapTileLayers[layer];

	if (mapTileLayerToDraw->id < 0)
	{
		OutputDebugStringA("Map Layer is empty!");
		return;
	}

	for (int i = 0; i < TILES_PER_LAYER_MAX; i++)
	{
		if (mapTileLayerToDraw->tiles[i].use == TRUE)			// このエネミーが使われている？
		{									// Yes

			
			// テクスチャ設定
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[mapTileLayerToDraw->tiles[i].texNo]);

			//エネミーの位置やテクスチャー座標を反映
			float px = mapTileLayerToDraw->tiles[i].pos.x - bg->pos.x;	// エネミーの表示位置X
			float py = mapTileLayerToDraw->tiles[i].pos.y - bg->pos.y;	// エネミーの表示位置Y
			float pw = mapTileLayerToDraw->tiles[i].w;		// エネミーの表示幅
			float ph = mapTileLayerToDraw->tiles[i].h;		// エネミーの表示高さ

			float tw = mapTileLayerToDraw->tiles[i].textureWidth;	// テクスチャの幅
			float th = mapTileLayerToDraw->tiles[i].textureHeigt;	// テクスチャの高さ
			float tx = mapTileLayerToDraw->tiles[i].textureU;	// テクスチャの左上X座標
			float ty = mapTileLayerToDraw->tiles[i].textureV;	// テクスチャの左上Y座標

			if (px < -pw ||
				px > SCREEN_WIDTH + pw ||
				py < -ph ||
				py > SCREEN_HEIGHT + ph)
			{
				continue;
			}

			// １枚のポリゴンの頂点とテクスチャ座標を設定
			SetSpriteColorRotation(g_VertexBuffer, px, py, pw, ph, tx, ty, tw, th,
				XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
				mapTileLayerToDraw->tiles[i].rot.z);

			// ポリゴン描画
			GetDeviceContext()->Draw(4, 0);
		}
	}

	// 壁の当たり判定の表示
	if (!MAP_DRAW_DEBUG_WALLS)
		return;

	MAPOBJECT* walls = GetMapObjectsFromLayer(MAPOBJLAYER_WALL);
	for (int w = 0; w < MAP_OBJECTS_PER_LAYER_MAX; w++) 
	{

		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[TILESET_MAX-1]);

		//エネミーの位置やテクスチャー座標を反映
		float px = walls[w].x - bg->pos.x;
		float py = walls[w].y - bg->pos.y;	// エネミーの表示位置Y
		float pw = walls[w].width;		// エネミーの表示幅
		float ph = walls[w].height;		// エネミーの表示高さ

		// アニメーション用
		//float tw = 1.0f / TEXTURE_PATTERN_DIVIDE_X;	// テクスチャの幅
		//float th = 1.0f / TEXTURE_PATTERN_DIVIDE_Y;	// テクスチャの高さ
		//float tx = (float)(g_Player[i].patternAnim % TEXTURE_PATTERN_DIVIDE_X) * tw;	// テクスチャの左上X座標
		//float ty = (float)(g_Player[i].patternAnim / TEXTURE_PATTERN_DIVIDE_X) * th;	// テクスチャの左上Y座標

		float tw = 1.0f;	// テクスチャの幅
		float th = 1.0f;	// テクスチャの高さ
		float tx = 0.0f;	// テクスチャの左上X座標
		float ty = 0.0f;	// テクスチャの左上Y座標

		// １枚のポリゴンの頂点とテクスチャ座標を設定
		SetSpriteColorRotation(g_VertexBuffer, px, py, pw, ph, tx, ty, tw, th,
			XMFLOAT4(0.0f, 0.0f, 1.0f, 0.2f),0.0f);

		// ポリゴン描画
		GetDeviceContext()->Draw(4, 0);

	}

	PLAYER* player = GetPlayer();

	for (int p = 0; p < PLAYER_MAX; p++)
	{
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[TILESET_MAX - 1]);

		//エネミーの位置やテクスチャー座標を反映
		float px = player[p].pos.x + player[p].collider.offsetX - bg->pos.x;
		float py = player[p].pos.y + player[p].collider.offsetY - bg->pos.y;	// エネミーの表示位置Y
		float pw = player[p].collider.width;		// エネミーの表示幅
		float ph = player[p].collider.height;		// エネミーの表示高さ

		// アニメーション用
		//float tw = 1.0f / TEXTURE_PATTERN_DIVIDE_X;	// テクスチャの幅
		//float th = 1.0f / TEXTURE_PATTERN_DIVIDE_Y;	// テクスチャの高さ
		//float tx = (float)(g_Player[i].patternAnim % TEXTURE_PATTERN_DIVIDE_X) * tw;	// テクスチャの左上X座標
		//float ty = (float)(g_Player[i].patternAnim / TEXTURE_PATTERN_DIVIDE_X) * th;	// テクスチャの左上Y座標

		float tw = 1.0f;	// テクスチャの幅
		float th = 1.0f;	// テクスチャの高さ
		float tx = 0.0f;	// テクスチャの左上X座標
		float ty = 0.0f;	// テクスチャの左上Y座標

		// １枚のポリゴンの頂点とテクスチャ座標を設定
		SetSpriteColorRotation(g_VertexBuffer, px, py, pw, ph, tx, ty, tw, th,
			XMFLOAT4(0.0f, 1.0f, 0.0f, 0.2f), 0.0f);

		// ポリゴン描画
		GetDeviceContext()->Draw(4, 0);
	}

}

void ParseMap(TILESET tilesets[], MAPTILELAYER mapTileLayers[], FIELDOBJECTGROUP objectLayers[], char* file)
{

	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(file);

	// マップチップの準備
	if (result)
	{

		int tilesetNodeCount = 0;

		// マップチップのテキスチャーの準備
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

				memcpy(tileset.textureSource + strlen(tileset.textureSource), textureSource + 6, strlen(textureSource));

				tileset.textureW = tilesetTextureNode.attribute("width").as_int();
				tileset.textureH = tilesetTextureNode.attribute("height").as_int();

				
				////　カスタムタイル
				int customTileCount = 0;
				for (pugi::xml_node xmlCustomTile : tilesetInnerNode.children("tile")) 
				{

					tileset.customTiles[customTileCount].id = xmlCustomTile.attribute("id").as_int();
					tileset.customTiles[customTileCount].x = xmlCustomTile.attribute("x").as_int();
					tileset.customTiles[customTileCount].y = xmlCustomTile.attribute("y").as_int();
					tileset.customTiles[customTileCount].width = xmlCustomTile.attribute("width").as_int();
					tileset.customTiles[customTileCount].height = xmlCustomTile.attribute("height").as_int();

					const char* tileTextureSource = xmlCustomTile.child("image").attribute("source").value();

					memcpy(tileset.customTiles[customTileCount].textureSource, g_dataFolder, strlen(g_dataFolder));

					memcpy(tileset.customTiles[customTileCount].textureSource + strlen(tileset.customTiles[customTileCount].textureSource), tileTextureSource + 6, strlen(tileTextureSource));
				
					tileset.customTiles[customTileCount].textureW = xmlCustomTile.child("image").attribute("width").as_int();
				    tileset.customTiles[customTileCount].textureH = xmlCustomTile.child("image").attribute("height").as_int();
					
					customTileCount++;
				}

				tilesets[tilesetNodeCount] = tileset;
				tilesetNodeCount++;

			}
		}

		int mapTileLayerNodeCount = 0;

		// マップチップのレイヤーの準備
		for (pugi::xml_node mapTileLayerNode : doc.child("map").children("layer"))
		{

			/*std::string str = "Level is: ";
			str.append(mapTileLayerNode.attribute("name").value());

			OutputDebugStringA(str.c_str());*/

			mapTileLayers[mapTileLayerNodeCount].id = mapTileLayerNode.attribute("id").as_int();

			const char* tLName = mapTileLayerNode.attribute("name").value();
			memcpy(mapTileLayers[mapTileLayerNodeCount].name, tLName, strlen(tLName));

			const char* tLClass = mapTileLayerNode.attribute("class").value();
			memcpy(mapTileLayers[mapTileLayerNodeCount].layerClass, tLClass, strlen(tLClass));

			mapTileLayers[mapTileLayerNodeCount].width = mapTileLayerNode.attribute("width").as_float();
			mapTileLayers[mapTileLayerNodeCount].height = mapTileLayerNode.attribute("height").as_float();

			char debug[128] = "";

			memcpy(debug, tLClass, strlen(tLClass));

			int result = strcmp(debug, MAP_DEBUG_KEY);

			if (result == 0 && !MAP_DRAW_DEBUG)
			{
				mapTileLayerNodeCount++;
				continue;
			}

			ParseTiles(&mapTileLayers[mapTileLayerNodeCount], mapTileLayerNode.child("data").child_value());

			mapTileLayerNodeCount++;
		}

		int objectGroupNodeCount = 0;

		for (pugi::xml_node objectGroupNode : doc.child("map").children("objectgroup"))
		{

			objectLayers[objectGroupNodeCount].id = objectGroupNode.attribute("id").as_int();

			const char* oGName = objectGroupNode.attribute("name").value();
			memcpy(objectLayers[objectGroupNodeCount].name, oGName, strlen(oGName));

			const char* oGClass = objectGroupNode.attribute("class").value();
			memcpy(objectLayers[objectGroupNodeCount].objectGroupClass, oGClass, strlen(oGClass));

			int objectsNodeCount = 0;

			for (pugi::xml_node fieldObjectNode : objectGroupNode.children("object")) {

				objectLayers[objectGroupNodeCount].fObjects[objectsNodeCount].id = fieldObjectNode.attribute("id").as_int();

				objectLayers[objectGroupNodeCount].fObjects[objectsNodeCount].gid = fieldObjectNode.attribute("gid").as_int();

				const char* oName = fieldObjectNode.attribute("name").value();
				memcpy(objectLayers[objectGroupNodeCount].fObjects[objectsNodeCount].name, oName, strlen(oName));

				const char* oClass = fieldObjectNode.attribute("type").value();
				memcpy(objectLayers[objectGroupNodeCount].fObjects[objectsNodeCount].objectType, oClass, strlen(oClass));

				objectLayers[objectGroupNodeCount].fObjects[objectsNodeCount].width = fieldObjectNode.attribute("width").as_float() * MAP_SCALE;
				objectLayers[objectGroupNodeCount].fObjects[objectsNodeCount].height = fieldObjectNode.attribute("height").as_float() * MAP_SCALE;

				objectLayers[objectGroupNodeCount].fObjects[objectsNodeCount].x = (fieldObjectNode.attribute("x").as_float() * MAP_SCALE) + objectLayers[objectGroupNodeCount].fObjects[objectsNodeCount].width / 2;
				objectLayers[objectGroupNodeCount].fObjects[objectsNodeCount].y = (fieldObjectNode.attribute("y").as_float() * MAP_SCALE) + objectLayers[objectGroupNodeCount].fObjects[objectsNodeCount].height / 2;


				objectsNodeCount++;

			}

			objectGroupNodeCount++;

		}

	}

}

void ParseTiles(MAPTILELAYER* mapTileLayer, const char* rawData)
{
	// タイル準備
	g_TileCount = 0;

	int converted = 0;
	const char* tok = rawData;
	int i = 0;

	int tileIndex_X = 0;
	int tileIndex_Y = 0;

	// データの解析
	do
	{
		int tileId;

		converted = sscanf(tok, "%d", &tileId);

		tok = strchr(tok, ',');

		if (tok == NULL)
			break;

		TILE nTile;

		nTile.id = tileId;
		nTile.use = tileId > 0 ? TRUE : FALSE; // タイル ID が 0 の場合、タイルは表示されません。

		if (nTile.use) {

			TILESET* relatedTileSet = GetTilesetFromTileID(nTile.id);

			if (relatedTileSet != NULL) {

				float newX = (tileIndex_X * relatedTileSet->tileWidth) + relatedTileSet->tileWidth / 2;
				float newY = (tileIndex_Y * relatedTileSet->tileHeight) + relatedTileSet->tileWidth / 2;

				nTile.pos = XMFLOAT3(newX * MAP_SCALE, newY * MAP_SCALE, 0.0f);
				nTile.rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
				nTile.scl = XMFLOAT3(1.0f * MAP_SCALE, 1.0f * MAP_SCALE, 1.0f * MAP_SCALE);
				nTile.texNo = relatedTileSet->id;

				float tileSetTileWidth = relatedTileSet->tileWidth;
				float tileSetTileHeight = relatedTileSet->tileHeight;

				nTile.w = tileSetTileWidth * nTile.scl.x;
				nTile.h = tileSetTileHeight * nTile.scl.y;

				int textureIndex = nTile.id - relatedTileSet->firstGID;

				int textureUIndex = textureIndex % relatedTileSet->columns;

				int textureVIndex = floor(textureIndex / relatedTileSet->columns);

				int tileSetTextureWidth = relatedTileSet->textureW;
				int tileSetTextureHeight = relatedTileSet->textureH;

				nTile.textureU = (textureUIndex * tileSetTileWidth) / tileSetTextureWidth;
				nTile.textureV = (textureVIndex * tileSetTileHeight) / tileSetTextureHeight;
				nTile.textureWidth = tileSetTileWidth / tileSetTextureWidth;
				nTile.textureHeigt = tileSetTileHeight / tileSetTextureHeight;

			}

		}

		mapTileLayer->tiles[i] = nTile;
		g_TileCount++;

		tileIndex_X++;

		if (tileIndex_X > mapTileLayer->width - 1)
		{
			tileIndex_X = 0;
			tileIndex_Y++;
		}


		tok += 1;

		i++;

	} while (converted != 0);


}

//=============================================================================
// Field構造体の先頭アドレスを取得
//=============================================================================
TILE* GetField(void)
{
	return &g_Tiles[0];
}


// 生きてるエネミーの数
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

TILESET* GetTilesetByName(const char* name) 
{

	for (int t = 0; t < TILESET_MAX; t++) {

		int tileIDRangeMinValue = g_Tilesets[t].firstGID;
		int tileIDrangeMaxValue = g_Tilesets[t].firstGID + g_Tilesets[t].tileCount - 1;

		if (strcmp(name, g_Tilesets[t].name)== 0)
		{
			return &g_Tilesets[t];
		}

	}

	return NULL;

}

MAPOBJECT** GetMapObjectsByClass(const char* objectType)
{
	MAPOBJECT* fieldObjects[MAP_OBJECTS_PER_LAYER_MAX];

	int foundObjects = 0;

	for (int og = 0; og < MAP_OBJLAYERS_MAX; og++)
	{

		for (int o = 0; o < MAP_OBJECTS_PER_LAYER_MAX; o++)
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

MAPOBJECT* GetMapObjectsFromLayer(const char* objectLayer)
{
	MAPOBJECT *fieldObjects = NULL;

	int foundGroups = 0;

	for (int og = 0; og < MAP_OBJLAYERS_MAX; og++)
	{


		if (strcmp(g_ObjectGroups[og].objectGroupClass, objectLayer) == 0)
		{

			fieldObjects = g_ObjectGroups[og].fObjects;
						
			foundGroups++;

			// 現在、検索には一つのグループのみの返しができます。
			break;
		}

	}

	return fieldObjects;

}

MAPOBJECT* GetMapObjectByNameFromLayer(const char* objectLayer, const char* objectName)
{
	MAPOBJECT* fieldObject = NULL;

	int foundGroups = 0;

	for (int og = 0; og < MAP_OBJLAYERS_MAX; og++)
	{


		if (strcmp(g_ObjectGroups[og].objectGroupClass, objectLayer) == 0)
		{

			for (int o = 0; o < MAP_OBJECTS_PER_LAYER_MAX; o++)
			{

				if (strcmp(g_ObjectGroups[og].objectGroupClass, objectLayer) == 0) 
				{
					fieldObject = &g_ObjectGroups[og].fObjects[o];


					// 現在、検索には最初のオブジェクトのみの返しができます。
					break;
				}

			}

			foundGroups++;

			// 現在、検索には最初のグループのみの返しができます。
			break;
		}

	}

	return fieldObject;

}


