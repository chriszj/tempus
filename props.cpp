//=============================================================================
//
// エネミー処理 [enemy.cpp]
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
// マクロ定義
//*****************************************************************************
#define TEXTURE_WIDTH				(200/2)	// キャラサイズ
#define TEXTURE_HEIGHT				(200/2)	// 
#define TEXTURE_MAX					(2)		// テクスチャの数

#define ANIM_WAIT					(4)		// アニメーションの切り替わるWait値


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
void PushToTimeState(TIMESTATE* timeState, PROP* interactable);
void PullFromTimeState(TIMESTATE* timeState, PROP* interactable);

//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer				*g_VertexBuffer = NULL;				// 頂点情報
static ID3D11ShaderResourceView	*g_Texture[TILESET_CUSTOM_TILES_MAX] = { NULL };	// テクスチャ情報

static BOOL		g_Load = FALSE;			// 初期化を行ったかのフラグ
static PROP	g_Props[PROPS_MAX];		// エネミー構造体

static int		g_InteractablesCount = PROPS_MAX;

static float offsetx = 200.0f;
static float offsety = 200.0f;

static TILESET* g_PropsTileset;

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitProps(void)
{
	MAPOBJECT* mapObjects = GetMapObjectsFromLayer(MAPOBJLAYER_PROPS);

	ID3D11Device *pDevice = GetDevice();

	g_PropsTileset = GetTilesetByName(TILESET_PROPS_NAME);

	//テクスチャ生成
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

	// 頂点バッファ生成
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX_3D) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	GetDevice()->CreateBuffer(&bd, NULL, &g_VertexBuffer);

	
	// エネミー構造体の初期化
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
		//　使っているテキスチャーで種類がわかれる
		g_Props[i].id = g_Props[i].texNo;

		g_Props[i].countAnim = 0;

		g_Props[i].animDivideX = g_PropsTileset->customTiles[g_Props[i].texNo].animDivideX;
		g_Props[i].animDivideY = g_PropsTileset->customTiles[g_Props[i].texNo].animDivideY;
		g_Props[i].patternAnimNum = g_PropsTileset->customTiles[g_Props[i].texNo].patternAnimNum;

		g_Props[i].move = XMFLOAT3(4.0f, 0.0f, 0.0f);		// 移動量

		PushToTimeState(&g_Props[i].timeState, &g_Props[i]);
		RegisterObjectTimeState(&g_Props[i].timeState);
	}

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// 終了処理
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
// 更新処理
//=============================================================================
void UpdateProps(void)
{
	if (g_Load == FALSE) return;
	g_InteractablesCount = 0;			// 生きてるエネミーの数

	for (int i = 0; i < PROPS_MAX; i++)
	{
		// 生きてるエネミーだけ処理をする
		if (g_Props[i].use == TRUE)
		{
			if (g_Props[i].id == 1);
				g_InteractablesCount++;		// 生きてた敵の数

			if (g_Props[i].timeState.status == READ_ONLY)
			{

				PullFromTimeState(&g_Props[i].timeState, &g_Props[i]);
				continue;
			}
			
			// 地形との当たり判定用に座標のバックアップを取っておく
			XMFLOAT3 pos_old = g_Props[i].pos;

			// アニメーション  
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

#ifdef _DEBUG	// デバッグ情報を表示する


#endif

}

//=============================================================================
// 描画処理
//=============================================================================
void DrawProps(void)
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

	for (int i = 0; i < PROPS_MAX; i++)
	{
		if (g_Props[i].use == TRUE)			// このエネミーが使われている？
		{									// Yes
			// テクスチャ設定
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_Props[i].texNo]);

			//エネミーの位置やテクスチャー座標を反映
			float px = g_Props[i].pos.x - bg->pos.x;	// エネミーの表示位置X
			float py = g_Props[i].pos.y - bg->pos.y;	// エネミーの表示位置Y
			float pw = g_Props[i].w;		// エネミーの表示幅
			float ph = g_Props[i].h;		// エネミーの表示高さ

			/*int customTileWidth = g_ItemsTileset->customTiles[g_Item[i].texNo].width;
			int customTileTextureW = g_ItemsTileset->customTiles[g_Item[i].texNo].textureW;
			int divideX = customTileTextureW / customTileWidth;

			int customTileHeight = g_ItemsTileset->customTiles[g_Item[i].texNo].height;
			int customTileTextureH = g_ItemsTileset->customTiles[g_Item[i].texNo].textureH;
			int divideY = customTileTextureH / customTileHeight;*/

			// アニメーション用
			float tw = 1.0f / g_Props[i].animDivideX;	// テクスチャの幅
			float th = 1.0f / g_Props[i].animDivideY;	// テクスチャの高さ
			float tx = (float)(g_Props[i].patternAnim % g_Props[i].animDivideX) * tw;	// テクスチャの左上X座標
			float ty = (float)(g_Props[i].patternAnim / g_Props[i].animDivideX) * th;	// テクスチャの左上Y座標

			//float tw = 1.0f;	// テクスチャの幅
			//float th = 1.0f;	// テクスチャの高さ
			//float tx = 0.0f;	// テクスチャの左上X座標
			//float ty = 0.0f;	// テクスチャの左上Y座標

			// １枚のポリゴンの頂点とテクスチャ座標を設定
			SetSpriteColorRotation(g_VertexBuffer, px, py, pw, ph, tx, ty, tw, th,
				XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
				g_Props[i].rot.z);

			// ポリゴン描画
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
// 小道具構造体の先頭アドレスを取得
//=============================================================================
PROP* GetProps(void)
{
	return &g_Props[0];
}


// 生きてるエネミーの数
int GetPropsCount(void)
{
	return g_InteractablesCount;
}




