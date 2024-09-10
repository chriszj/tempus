//=============================================================================
//
// エネミー処理 [enemy.cpp]
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
static INTERACTABLE	g_Interactables[INTERACTABLES_MAX];		// エネミー構造体

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
// 初期化処理
//=============================================================================
HRESULT InitInteractables(void)
{
	MAPOBJECT* mapObjects = GetMapObjectsFromLayer(MAPOBJLAYER_INTERACTABLES);

	ID3D11Device *pDevice = GetDevice();

	g_InteractablesTileset = GetTilesetByName(TILESET_INTERACTABLES_NAME);

	//テクスチャ生成
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
		//　使っているテキスチャーで種類がわかれる
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

		g_Interactables[i].move = XMFLOAT3(4.0f, 0.0f, 0.0f);		// 移動量

		
	}

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// 終了処理
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
// 更新処理
//=============================================================================
void UpdateInteractables(void)
{
	if (g_Load == FALSE) return;
	g_ItemCount = 0;			// 生きてるエネミーの数

	for (int i = 0; i < INTERACTABLES_MAX; i++)
	{
		// 生きてるエネミーだけ処理をする
		if (g_Interactables[i].use == TRUE)
		{
			if (g_Interactables[i].id == 1);
				g_ItemCount++;		// 生きてた敵の数
			
			// 地形との当たり判定用に座標のバックアップを取っておく
			XMFLOAT3 pos_old = g_Interactables[i].pos;

			// アニメーション  
			g_Interactables[i].countAnim += 1.0f;
			if (g_Interactables[i].countAnim > ANIM_WAIT)
			{
				g_Interactables[i].countAnim = 0.0f;

				int nextFrame = 0;
				// パターンの切り替え
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

			// 移動が終わったらエネミーとの当たり判定
			{
				PLAYER* player = GetPlayer();

				// エネミーの数分当たり判定を行う
				for (int j = 0; j < INTERACTABLES_MAX; j++)
				{
					// 生きてるエネミーと当たり判定をする
					if (player[j].use == TRUE)
					{
						BOOL ans = CollisionBB(g_Interactables[i].pos, g_Interactables[i].w, g_Interactables[i].h,
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
void DrawInteractables(void)
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

	for (int i = 0; i < INTERACTABLES_MAX; i++)
	{
		if (g_Interactables[i].use == TRUE)			// このエネミーが使われている？
		{									// Yes
			// テクスチャ設定
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_Interactables[i].texNo]);

			//エネミーの位置やテクスチャー座標を反映
			float px = g_Interactables[i].pos.x - bg->pos.x;	// エネミーの表示位置X
			float py = g_Interactables[i].pos.y - bg->pos.y;	// エネミーの表示位置Y
			float pw = g_Interactables[i].w;		// エネミーの表示幅
			float ph = g_Interactables[i].h;		// エネミーの表示高さ

			/*int customTileWidth = g_ItemsTileset->customTiles[g_Item[i].texNo].width;
			int customTileTextureW = g_ItemsTileset->customTiles[g_Item[i].texNo].textureW;
			int divideX = customTileTextureW / customTileWidth;

			int customTileHeight = g_ItemsTileset->customTiles[g_Item[i].texNo].height;
			int customTileTextureH = g_ItemsTileset->customTiles[g_Item[i].texNo].textureH;
			int divideY = customTileTextureH / customTileHeight;*/

			// アニメーション用
			float tw = 1.0f / g_Interactables[i].animDivideX;	// テクスチャの幅
			float th = 1.0f / g_Interactables[i].animDivideY;	// テクスチャの高さ
			float tx = (float)(g_Interactables[i].patternAnim % g_Interactables[i].animDivideX) * tw;	// テクスチャの左上X座標
			float ty = (float)(g_Interactables[i].patternAnim / g_Interactables[i].animDivideX) * th;	// テクスチャの左上Y座標

			//float tw = 1.0f;	// テクスチャの幅
			//float th = 1.0f;	// テクスチャの高さ
			//float tx = 0.0f;	// テクスチャの左上X座標
			//float ty = 0.0f;	// テクスチャの左上Y座標

			// １枚のポリゴンの頂点とテクスチャ座標を設定
			SetSpriteColorRotation(g_VertexBuffer, px, py, pw, ph, tx, ty, tw, th,
				XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
				g_Interactables[i].rot.z);

			// ポリゴン描画
			GetDeviceContext()->Draw(4, 0);
		}
	}

}


//=============================================================================
// Enemy構造体の先頭アドレスを取得
//=============================================================================
INTERACTABLE* GetInteractables(void)
{
	return &g_Interactables[0];
}


// 生きてるエネミーの数
int GetInteractablesCount(void)
{
	return g_ItemCount;
}

void SetInteractable(INTERACTABLE* interactable, BOOL active) 
{

	interactable->active = active;
}


