//=============================================================================
//
// エネミー処理 [enemy.cpp]
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
// マクロ定義
//*****************************************************************************
#define TEXTURE_WIDTH				(200/2)	// キャラサイズ
#define TEXTURE_HEIGHT				(200/2)	// 
#define TEXTURE_MAX					(2)		// テクスチャの数

#define ANIM_WAIT					(4)		// アニメーションの切り替わるWait値


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer				*g_VertexBuffer = NULL;				// 頂点情報
static ID3D11ShaderResourceView	*g_Texture[TILESET_CUSTOM_TILES_MAX] = { NULL };	// テクスチャ情報

static char *g_TexturName[TEXTURE_MAX] = {
	"data/TEXTURE/candle-burning-only fire.png",
	"data/TEXTURE/bar_white.png",
};


static BOOL		g_Load = FALSE;			// 初期化を行ったかのフラグ
static ITEM	g_Item[ITEM_MAX];		// エネミー構造体

static int		g_ItemCount = ITEM_MAX;

static float offsetx = 200.0f;
static float offsety = 200.0f;

static TILESET* g_ItemsTileset;



//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitItem(void)
{
	MAPOBJECT* mapObjects = GetMapObjectsFromLayer(MAPOBJLAYER_ITEMS);

	ID3D11Device *pDevice = GetDevice();

	g_ItemsTileset = GetTilesetByName(TILESET_ITEMS_NAME);

	//テクスチャ生成
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

	// 頂点バッファ生成
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX_3D) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	GetDevice()->CreateBuffer(&bd, NULL, &g_VertexBuffer);

	
	// エネミー構造体の初期化
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
		//　使っているテキスチャーで種類がわかれる
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

		g_Item[i].move = XMFLOAT3(4.0f, 0.0f, 0.0f);		// 移動量

		
	}

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// 終了処理
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
// 更新処理
//=============================================================================
void UpdateItem(void)
{
	if (g_Load == FALSE) return;
	g_ItemCount = 0;			// 生きてるエネミーの数

	for (int i = 0; i < ITEM_MAX; i++)
	{
		// 生きてるエネミーだけ処理をする
		if (g_Item[i].use == TRUE)
		{
			g_ItemCount++;		// 生きてた敵の数
			
			// 地形との当たり判定用に座標のバックアップを取っておく
			XMFLOAT3 pos_old = g_Item[i].pos;

			// アニメーション  
			g_Item[i].countAnim += 1.0f;
			if (g_Item[i].countAnim > ANIM_WAIT)
			{
				g_Item[i].countAnim = 0.0f;
				// パターンの切り替え
				g_Item[i].patternAnim = (g_Item[i].patternAnim + 1) % g_Item[i].patternAnimNum;
			}

			// 移動が終わったらエネミーとの当たり判定
			{
				PLAYER* player = GetPlayer();

				// エネミーの数分当たり判定を行う
				for (int j = 0; j < ITEM_MAX; j++)
				{
					// 生きてるエネミーと当たり判定をする
					if (player[j].use == TRUE)
					{
						BOOL ans = CollisionBB(g_Item[i].pos, g_Item[i].w, g_Item[i].h,
							player[j].pos, player[j].w, player[j].h);
						// 当たっている？
						if (ans == TRUE)
						{
							// 当たった時の処理
							//player[j].use = FALSE;	// デバッグで一時的に無敵にしておくか
						}
					}
				}
			}
		}
	}

#ifdef _DEBUG	// デバッグ情報を表示する


#endif

}

//=============================================================================
// 描画処理
//=============================================================================
void DrawItem(void)
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

	for (int i = 0; i < ITEM_MAX; i++)
	{
		if (g_Item[i].use == TRUE)			// このエネミーが使われている？
		{									// Yes
			// テクスチャ設定
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_Item[i].texNo]);

			//エネミーの位置やテクスチャー座標を反映
			float px = g_Item[i].pos.x - bg->pos.x;	// エネミーの表示位置X
			float py = g_Item[i].pos.y - bg->pos.y;	// エネミーの表示位置Y
			float pw = g_Item[i].w;		// エネミーの表示幅
			float ph = g_Item[i].h;		// エネミーの表示高さ

			/*int customTileWidth = g_ItemsTileset->customTiles[g_Item[i].texNo].width;
			int customTileTextureW = g_ItemsTileset->customTiles[g_Item[i].texNo].textureW;
			int divideX = customTileTextureW / customTileWidth;

			int customTileHeight = g_ItemsTileset->customTiles[g_Item[i].texNo].height;
			int customTileTextureH = g_ItemsTileset->customTiles[g_Item[i].texNo].textureH;
			int divideY = customTileTextureH / customTileHeight;*/

			// アニメーション用
			float tw = 1.0f / g_Item[i].animDivideX;	// テクスチャの幅
			float th = 1.0f / g_Item[i].animDivideY;	// テクスチャの高さ
			float tx = (float)(g_Item[i].patternAnim % g_Item[i].animDivideX) * tw;	// テクスチャの左上X座標
			float ty = (float)(g_Item[i].patternAnim / g_Item[i].animDivideX) * th;	// テクスチャの左上Y座標

			//float tw = 1.0f;	// テクスチャの幅
			//float th = 1.0f;	// テクスチャの高さ
			//float tx = 0.0f;	// テクスチャの左上X座標
			//float ty = 0.0f;	// テクスチャの左上Y座標

			// １枚のポリゴンの頂点とテクスチャ座標を設定
			SetSpriteColorRotation(g_VertexBuffer, px, py, pw, ph, tx, ty, tw, th,
				XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
				g_Item[i].rot.z);

			// ポリゴン描画
			GetDeviceContext()->Draw(4, 0);
		}
	}

}


//=============================================================================
// Enemy構造体の先頭アドレスを取得
//=============================================================================
ITEM* GetItem(void)
{
	return &g_Item[0];
}


// 生きてるエネミーの数
int GetItemCount(void)
{
	return g_ItemCount;
}


