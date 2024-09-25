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
#include "sound.h"
#include <vector>
#include <random>

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TEXTURE_WIDTH				(200/2)	// キャラサイズ
#define TEXTURE_HEIGHT				(200/2)	// 
#define TEXTURE_MAX					(2)		// テクスチャの数

#define ANIM_WAIT					(3)		// アニメーションの切り替わるWait値


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
void PushToTimeState(TIMESTATE* timeState, INTERACTABLE* interactable);
void PullFromTimeState(TIMESTATE* timeState, INTERACTABLE* interactable);
void ResetFloorSwitches();
int ActiveFloorSwitchesCount();
BOOL AllFloorSwitchesActive();

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

static int		g_InteractablesCount = INTERACTABLES_MAX;

static float offsetx = 200.0f;
static float offsety = 200.0f;

static TILESET* g_InteractablesTileset;


static std::vector<INTERACTABLE*> g_FloorSwitches;
static std::vector<INTERACTABLE*> g_FloorSwitchesDoors;

static INTERACTABLETYPES g_InteractableTypes[INTERACTABLES_TYPES_MAX] =
{
	{INTERACTABLES_DOOR, TRUE },
	{INTERACTABLES_MASTER_DOOR, FALSE },
	{INTERACTABLES_PIT, TRUE,0, 14, 15, 19},
	{INTERACTABLES_PIT_2, TRUE, 0, 14, 15, 19},
	{INTERACTABLES_FLOOR_SWITCH, FALSE},
	{INTERACTABLES_SWITCH_DOOR, FALSE}
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

	std::vector<int> switchOrder;
	
	// エネミー構造体の初期化
	g_InteractablesCount = 0;
	for (int i = 0; i < INTERACTABLES_MAX; i++)
	{

		g_InteractablesCount++;
		g_Interactables[i].use = mapObjects[i].id > 0? TRUE: FALSE;

		if (!g_Interactables[i].use)
			continue;

		g_Interactables[i].active = TRUE;
		g_Interactables[i].interactionMode = -1;
		g_Interactables[i].id = i;
		
		g_Interactables[i].pos = XMFLOAT3((mapObjects[i].x ), (mapObjects[i].y - (mapObjects[i].height)), 0.0f);
		g_Interactables[i].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Interactables[i].scl = XMFLOAT3(1.0f, 1.0f, 1.0f);
		g_Interactables[i].w = mapObjects[i].width;
		g_Interactables[i].h = mapObjects[i].height;
		g_Interactables[i].texNo = mapObjects[i].gid - g_InteractablesTileset->firstGID -1;
		//　使っているテキスチャーで種類がわかれる
		g_Interactables[i].type = g_Interactables[i].texNo;

		g_Interactables[i].collider = g_InteractablesTileset->customTiles[g_Interactables[i].texNo].collider;

		g_Interactables[i].countAnim = 0;

		g_Interactables[i].animDivideX = g_InteractablesTileset->customTiles[g_Interactables[i].type].animDivideX;
		g_Interactables[i].animDivideY = g_InteractablesTileset->customTiles[g_Interactables[i].type].animDivideY;
		g_Interactables[i].patternAnimNum = g_InteractablesTileset->customTiles[g_Interactables[i].type].patternAnimNum;
		g_Interactables[i].normalModeMinFrame = g_InteractableTypes[g_Interactables[i].type].normalModeMinFrame;
		g_Interactables[i].normalModeMaxFrame = g_InteractableTypes[g_Interactables[i].type].normalModeMaxFrame;
		g_Interactables[i].hostileModeMinFrame = g_InteractableTypes[g_Interactables[i].type].hostileModeMinFrame;
		g_Interactables[i].hostileModeMaxFrame = g_InteractableTypes[g_Interactables[i].type].hostileModeMaxFrame;

		if (g_Interactables[i].type == INTERACTABLES_FLOOR_SWITCH) 
		{
			g_Interactables[i].active = FALSE;
			g_Interactables[i].patternAnim = g_Interactables[i].patternAnimNum - 1;
			switchOrder.push_back(switchOrder.size());
			g_FloorSwitches.push_back(&g_Interactables[i]);
		}
		else if (g_Interactables[i].type == INTERACTABLES_SWITCH_DOOR) 
		{
			g_FloorSwitchesDoors.push_back(&g_Interactables[i]);
		}


		g_Interactables[i].move = XMFLOAT3(4.0f, 0.0f, 0.0f);		// 移動量

		PushToTimeState(&g_Interactables[i].timeState, &g_Interactables[i]);
		RegisterObjectTimeState(&g_Interactables[i].timeState);
	}

	std::random_device rd;
	std::mt19937 g(rd());

	std::shuffle(switchOrder.begin(), switchOrder.end(), g);

	for (int s = 0; s < g_FloorSwitches.size(); s++) {
		g_FloorSwitches[s]->activationOrder = switchOrder[s];
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

	g_FloorSwitches.clear();
	g_FloorSwitchesDoors.clear();

	g_Load = FALSE;
}

//=============================================================================
// 更新処理
//=============================================================================
void UpdateInteractables(void)
{
	if (g_Load == FALSE) return;
	g_InteractablesCount = 0;			// 生きてるエネミーの数

	for (int i = 0; i < INTERACTABLES_MAX; i++)
	{
		// 生きてるエネミーだけ処理をする
		if (g_Interactables[i].use == TRUE)
		{
			
			g_InteractablesCount++;		

			if (g_Interactables[i].timeState.status == READ_ONLY)
			{

				PullFromTimeState(&g_Interactables[i].timeState, &g_Interactables[i]);
				continue;
			}
			
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
					/*else {
						g_Interactables[i].interactionMode = 0;
						g_Interactables[i].active = TRUE;
					}*/
					

				}
				else {
				
					if (g_Interactables[i].patternAnim < g_Interactables[i].patternAnimNum -1)
					{
						nextFrame++;
					}
					/*else {
						g_Interactables[i].interactionMode = 0;
						g_Interactables[i].active = FALSE;
					}*/

				}

				g_Interactables[i].patternAnim = (g_Interactables[i].patternAnim + nextFrame) % g_Interactables[i].patternAnimNum;

				if (g_Interactables[i].normalModeMinFrame >= 0 && g_Interactables[i].normalModeMaxFrame >= 1)
				{
					if (g_Interactables[i].patternAnim >= g_Interactables[i].normalModeMinFrame && g_Interactables[i].patternAnim <= g_Interactables[i].normalModeMaxFrame)
						g_Interactables[i].interactionMode = INTERACTION_MODE_NORMAL;
				}

				if (g_Interactables[i].hostileModeMinFrame >= 0 && g_Interactables[i].hostileModeMaxFrame >= 1)
				{
					if (g_Interactables[i].patternAnim >= g_Interactables[i].hostileModeMinFrame && g_Interactables[i].patternAnim <= g_Interactables[i].hostileModeMaxFrame)
						g_Interactables[i].interactionMode = INTERACTION_MODE_HOSTILE;
				}

			}

			if (g_Interactables[i].type == INTERACTABLES_PIT || g_Interactables[i].type == INTERACTABLES_PIT_2 || g_Interactables[i].type == INTERACTABLES_FLOOR_SWITCH) {

				PLAYER* player = GetPlayer();

				// エネミーの数分当たり判定を行う
				for (int p = 0; p < PLAYER_MAX; p++)
				{
					// 生きてるエネミーと当たり判定をする
					if (player[p].use == TRUE)
					{

						// X方の当たり判定
						BOOL ans = CollisionBB(player[p].pos, player[p].collider, g_Interactables[i].pos, g_Interactables[i].collider);

						// 当たっている？
						if (ans == TRUE)
						{

							if (g_Interactables[i].type == INTERACTABLES_FLOOR_SWITCH) 
							{

								if (player[p].lastSwitchOrderActivated == g_Interactables[i].activationOrder || g_Interactables[i].active)
									continue;

								int nextFloorActivationOrder = (player[p].lastSwitchOrderActivated + 1) % g_FloorSwitches.size();

							
								if (player[p].lastSwitchOrderActivated < 0 || nextFloorActivationOrder == g_Interactables[i].activationOrder)
								{
									SetInteractable(&g_Interactables[i], TRUE);

									switch (ActiveFloorSwitchesCount())
									{
										case 0:
											PlaySound(SOUND_LABEL_SE_SWITCH_OK_0);
											break;
										case 1:
											PlaySound(SOUND_LABEL_SE_SWITCH_OK_1);
											break;
										case 2:
											PlaySound(SOUND_LABEL_SE_SWITCH_OK_2);
											break;
										case 3:
											PlaySound(SOUND_LABEL_SE_SWITCH_OK_3);
											break;
										case 4:
											PlaySound(SOUND_LABEL_SE_SWITCH_OK_4);
											break;
										case 5:
											PlaySound(SOUND_LABEL_SE_SWITCH_OK_5);
											break;
										case 6:
											PlaySound(SOUND_LABEL_SE_SWITCH_OK_6);
											break;
										case 7:
											PlaySound(SOUND_LABEL_SE_SWITCH_OK_7);
											break;
										case 8:
											PlaySound(SOUND_LABEL_SE_SWITCH_OK_8);
											break;
										default:
											PlaySound(SOUND_LABEL_SE_SWITCH_OK_9);
											break;
										
									}

									player[p].lastSwitchOrderActivated = g_Interactables[i].activationOrder;
									
									if (AllFloorSwitchesActive())
									{
										for each (INTERACTABLE* floorSwitchDoor in g_FloorSwitchesDoors)
										{
											SetInteractable(floorSwitchDoor, FALSE);
											PlaySound(SOUND_LABEL_SE_DOOR_OPEN);
										}
									}

								}
								else {
									ResetFloorSwitches();
									PlaySound(SOUND_LABEL_SE_SWITCH_WRONG);
									player[p].lastSwitchOrderActivated = -1;
								}

							}
							else {

								if (g_Interactables[i].interactionMode == INTERACTION_MODE_NORMAL) {

									//一回音をプレイするため
									if (g_Interactables[i].active)
										PlaySound(SOUND_LABEL_SE_ROCK_CRACK);

									SetInteractable(&g_Interactables[i], FALSE);

								}
								else
									MakePlayerFall(player, 10);

							}
								
						}
					}
				}

			}
			

			PushToTimeState(&g_Interactables[i].timeState, &g_Interactables[i]);
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

void PushToTimeState(TIMESTATE* timeState, INTERACTABLE* interactable)
{
	timeState->active = interactable->active;
	timeState->interactionMode = interactable->interactionMode;
	timeState->countAnim = interactable->countAnim;
	timeState->patternAnim = interactable->patternAnim;
}

void PullFromTimeState(TIMESTATE* timeState, INTERACTABLE* interactable)
{
	interactable->active = timeState->active;
	interactable->interactionMode = timeState->interactionMode;
	interactable->countAnim = timeState->countAnim;
	interactable->patternAnim = timeState->patternAnim;
}

void ResetFloorSwitches() 
{
	for each (INTERACTABLE* floorSwitch in g_FloorSwitches)
	{
		SetInteractable(floorSwitch, FALSE);
	}
}

int ActiveFloorSwitchesCount() 
{

	int activeSwitches = 0;

	for each (INTERACTABLE * floorSwitch in g_FloorSwitches)
	{
		if (floorSwitch->active)
			activeSwitches++;
	}

	return activeSwitches;

}

BOOL AllFloorSwitchesActive() 
{

	BOOL allActive = TRUE;

	if (ActiveFloorSwitchesCount() - g_FloorSwitches.size() != 0)
		allActive = FALSE;

	return allActive;

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
	return g_InteractablesCount;
}

void SetInteractable(INTERACTABLE* interactable, BOOL active) 
{

	interactable->active = active;
	//interactable->interactionMode = active ? 1 : -1;
}


