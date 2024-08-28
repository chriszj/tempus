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


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer				*g_VertexBuffer = NULL;				// 頂点情報
static ID3D11ShaderResourceView	*g_Texture[TILESET_MAX] = { NULL };	// テクスチャ情報

static char* g_TilemapFolder = "data/TILEMAP/";

static TILESET g_Tilesets[TILESET_MAX];
static MAPLAYER g_MapLayers[MAP_LAYER_MAX];

static BOOL		g_Load = FALSE;			// 初期化を行ったかのフラグ
static TILE	 g_Tiles[TILES_PER_LAYER_MAX];		// エネミー構造体

static int	 g_TileCount = TILES_PER_LAYER_MAX;

static float offsetx = 200.0f;
static float offsety = 200.0f;

static float moveFactor = 200.0f;
static float tileTime = 25.0f;



//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitField(void)
{
	ID3D11Device* pDevice = GetDevice();

	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file("data/TILEMAP/map.tmx");

	// マップチップの準備
	if (result)
	{

		int tilesetNodeCount = 0;

		// マップチップのテキスチャーの準備
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

		// マップチップのレイヤーの準備
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

	//テクスチャ生成
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


	// 頂点バッファ生成
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX_3D) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	GetDevice()->CreateBuffer(&bd, NULL, &g_VertexBuffer);


	// タイル準備
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
		g_Tiles[i].pos = XMFLOAT3(200.0f + i*200.0f, 100.0f, 0.0f);	// 中心点から表示
		g_Tiles[i].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Tiles[i].scl = XMFLOAT3(1.0f, 1.0f, 1.0f);
		g_Tiles[i].w = TEXTURE_WIDTH;
		g_Tiles[i].h = TEXTURE_HEIGHT;
		g_Tiles[i].texNo = 0;

		g_Tiles[i].countAnim = 0;
		g_Tiles[i].patternAnim = 0;

		g_Tiles[i].move = XMFLOAT3(4.0f, 0.0f, 0.0f);		// 移動量

		g_Tiles[i].tileTime = 0.0f;			// 線形補間用のタイマーをクリア
		g_Tiles[i].tblNo = 0;			// 再生する行動データテーブルNoをセット
		g_Tiles[i].tblMax = 0;			// 再生する行動データテーブルのレコード数をセット
	}

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

	g_Load = FALSE;
}

//=============================================================================
// 更新処理
//=============================================================================
void UpdateField(void)
{
	if (g_Load == FALSE) return;
	g_TileCount = 0;			// 生きてるエネミーの数

	for (int i = 0; i < TILES_PER_LAYER_MAX; i++)
	{
		// 生きてるエネミーだけ処理をする
		if (g_Tiles[i].use == TRUE)
		{
			g_TileCount++;		// 生きてた敵の数
			
			// 地形との当たり判定用に座標のバックアップを取っておく
			XMFLOAT3 pos_old = g_Tiles[i].pos;

			// アニメーション  
			g_Tiles[i].countAnim += 1.0f;
			if (g_Tiles[i].countAnim > ANIM_WAIT)
			{
				g_Tiles[i].countAnim = 0.0f;
				// パターンの切り替え
				g_Tiles[i].patternAnim = (g_Tiles[i].patternAnim + 1) % ANIM_PATTERN_NUM;
			}

			// 移動処理
			//if (g_Tiles[i].tblMax > 0)	// 線形補間を実行する？
			//{	// 線形補間の処理
			//	int nowNo = (int)g_Tiles[i].tileTime;			// 整数分であるテーブル番号を取り出している
			//	int maxNo = g_Tiles[i].tblMax;				// 登録テーブル数を数えている
			//	int nextNo = (nowNo + 1) % maxNo;			// 移動先テーブルの番号を求めている
			//	INTERPOLATION_DATA* tbl = g_MoveTblAdr[g_Tiles[i].tblNo];	// 行動テーブルのアドレスを取得
			//	
			//	XMVECTOR nowPos = XMLoadFloat3(&tbl[nowNo].pos);	// XMVECTORへ変換
			//	XMVECTOR nowRot = XMLoadFloat3(&tbl[nowNo].rot);	// XMVECTORへ変換
			//	XMVECTOR nowScl = XMLoadFloat3(&tbl[nowNo].scl);	// XMVECTORへ変換
			//	
			//	XMVECTOR Pos = XMLoadFloat3(&tbl[nextNo].pos) - nowPos;	// XYZ移動量を計算している
			//	XMVECTOR Rot = XMLoadFloat3(&tbl[nextNo].rot) - nowRot;	// XYZ回転量を計算している
			//	XMVECTOR Scl = XMLoadFloat3(&tbl[nextNo].scl) - nowScl;	// XYZ拡大率を計算している
			//	
			//	float nowTime = g_Tiles[i].tileTime - nowNo;	// 時間部分である少数を取り出している
			//	
			//	Pos *= nowTime;								// 現在の移動量を計算している
			//	Rot *= nowTime;								// 現在の回転量を計算している
			//	Scl *= nowTime;								// 現在の拡大率を計算している

			//	// 計算して求めた移動量を現在の移動テーブルXYZに足している＝表示座標を求めている
			//	XMStoreFloat3(&g_Tiles[i].pos, nowPos + Pos);

			//	// 計算して求めた回転量を現在の移動テーブルに足している
			//	XMStoreFloat3(&g_Tiles[i].rot, nowRot + Rot);

			//	// 計算して求めた拡大率を現在の移動テーブルに足している
			//	XMStoreFloat3(&g_Tiles[i].scl, nowScl + Scl);
			//	g_Tiles[i].w = TEXTURE_WIDTH * g_Tiles[i].scl.x;
			//	g_Tiles[i].h = TEXTURE_HEIGHT * g_Tiles[i].scl.y;

			//	// frameを使て時間経過処理をする
			//	g_Tiles[i].tileTime += 1.0f / tbl[nowNo].frame;	// 時間を進めている
			//	if ((int)g_Tiles[i].tileTime >= maxNo)			// 登録テーブル最後まで移動したか？
			//	{
			//		g_Tiles[i].tileTime -= maxNo;				// ０番目にリセットしつつも小数部分を引き継いでいる
			//	}

			//}

			// 移動が終わったらエネミーとの当たり判定
			{
				PLAYER* player = GetPlayer();

				// エネミーの数分当たり判定を行う
				for (int j = 0; j < TILES_PER_LAYER_MAX; j++)
				{
					// 生きてるエネミーと当たり判定をする
					if (player[j].use == TRUE)
					{
						BOOL ans = CollisionBB(g_Tiles[i].pos, g_Tiles[i].w, g_Tiles[i].h,
							player[j].pos, player[j].w, player[j].h);
						// 当たっている？
						if (ans == TRUE)
						{
							// 当たった時の処理
							player[j].use = FALSE;	// デバッグで一時的に無敵にしておくか
						}
					}
				}
			}
		}
	}


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
void DrawField(void)
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

	for (int i = 0; i < TILES_PER_LAYER_MAX; i++)
	{
		if (g_Tiles[i].use == TRUE)			// このエネミーが使われている？
		{									// Yes
			// テクスチャ設定
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_Tiles[i].texNo]);

			//エネミーの位置やテクスチャー座標を反映
			float px = g_Tiles[i].pos.x - bg->pos.x;	// エネミーの表示位置X
			float py = g_Tiles[i].pos.y - bg->pos.y;	// エネミーの表示位置Y
			float pw = g_Tiles[i].w;		// エネミーの表示幅
			float ph = g_Tiles[i].h;		// エネミーの表示高さ

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
				XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
				g_Tiles[i].rot.z);

			// ポリゴン描画
			GetDeviceContext()->Draw(4, 0);
		}
	}


	// ゲージのテスト
	{
		// 下敷きのゲージ（枠的な物）
		// テクスチャ設定
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[1]);

		//ゲージの位置やテクスチャー座標を反映
		float px = 600.0f;		// ゲージの表示位置X
		float py =  10.0f;		// ゲージの表示位置Y
		float pw = 300.0f;		// ゲージの表示幅
		float ph =  30.0f;		// ゲージの表示高さ

		float tw = 1.0f;	// テクスチャの幅
		float th = 1.0f;	// テクスチャの高さ
		float tx = 0.0f;	// テクスチャの左上X座標
		float ty = 0.0f;	// テクスチャの左上Y座標

		// １枚のポリゴンの頂点とテクスチャ座標を設定
		SetSpriteLTColor(g_VertexBuffer,
			px, py, pw, ph,
			tx, ty, tw, th,
			XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f));

		// ポリゴン描画
		GetDeviceContext()->Draw(4, 0);


		// エネミーの数に従ってゲージの長さを表示してみる
		// テクスチャ設定
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[1]);

		//ゲージの位置やテクスチャー座標を反映
		pw = pw * ((float)g_TileCount / TILES_PER_LAYER_MAX);

		// １枚のポリゴンの頂点とテクスチャ座標を設定
		SetSpriteLTColor(g_VertexBuffer,
			px, py, pw, ph,
			tx, ty, tw, th,
			XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f));

		// ポリゴン描画
		GetDeviceContext()->Draw(4, 0);


	}




}


//=============================================================================
// Enemy構造体の先頭アドレスを取得
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


