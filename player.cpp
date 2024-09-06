//=============================================================================
//
// プレイヤー処理 [player.cpp]
// Author : 
//
//=============================================================================
#include "player.h"
#include "input.h"
#include "bg.h"
#include "field.h"
#include "bullet.h"
#include "item.h"
#include "enemy.h"
#include "collision.h"
#include "score.h"
#include "file.h"
#include "timemachine.h"
#include "gui.h"
#include <string>
#include <iostream>

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TEXTURE_WIDTH				(200/2)	// キャラサイズ
#define TEXTURE_HEIGHT				(200/2)	// 
#define TEXTURE_MAX					(2)		// テクスチャの数

#define TEXTURE_PATTERN_DIVIDE_X	(4)		// アニメパターンのテクスチャ内分割数（X)
#define TEXTURE_PATTERN_DIVIDE_Y	(8)		// アニメパターンのテクスチャ内分割数（Y)
#define ANIM_PATTERN_NUM			(TEXTURE_PATTERN_DIVIDE_X*TEXTURE_PATTERN_DIVIDE_Y)	// アニメーションパターン数
#define ANIM_WAIT					(6)		// アニメーションの切り替わるWait値

// プレイヤーの画面内配置座標
#define PLAYER_DISP_X				(SCREEN_WIDTH/2)
#define PLAYER_DISP_Y				(SCREEN_HEIGHT/2 + TEXTURE_HEIGHT)

// ジャンプ処理
#define	PLAYER_JUMP_CNT_MAX			(30)		// 30フレームで着地する
#define	PLAYER_JUMP_Y_MAX			(300.0f)	// ジャンプの高さ

#define PLAYER_COLLIDER_WIDTH       (50.0f)
#define PLAYER_COLLIDER_HEIGHT      (50.0f)
#define PLAYER_COLLIDER_OFFSETX     (-4.0f)
#define PLAYER_COLLIDER_OFFSETY     (12.0f)


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
void DrawPlayerOffset(int no);
void PushToTimeState(TIMESTATE* timeState, PLAYER* player);
void PullFromTimeState(TIMESTATE* timeState, PLAYER* player);


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer				*g_VertexBuffer = NULL;				// 頂点情報
static ID3D11ShaderResourceView	*g_Texture[TEXTURE_MAX] = { NULL };	// テクスチャ情報

static char *g_TexturName[TEXTURE_MAX] = {
	"data/TEXTURE/character_walk2.png",
	"data/TEXTURE/shadow000.jpg",
};

static BOOL		g_Load = FALSE;				// 初期化を行ったかのフラグ
static PLAYER	g_Player[PLAYER_MAX];		// プレイヤー構造体
static int		g_PlayerCount = PLAYER_MAX;	// 生きてるプレイヤーの数

static int      g_jumpCnt = 0;
static int		g_jump[PLAYER_JUMP_CNT_MAX] =
{
	-15, -14, -13, -12, -11, -10, -9, -8, -7, -6, -5,-4,-3,-2,-1,
	  1,   2,   3,   4,   5,   6,  7,  8,  9, 10, 11,12,13,14,15
};

static BMPTEXT* g_Score;

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitPlayer(void)
{
	ID3D11Device* pDevice = GetDevice();

	//テクスチャ生成
	for (int i = 0; i < TEXTURE_MAX; i++)
	{
		g_Texture[i] = NULL;
		D3DX11CreateShaderResourceViewFromFile(GetDevice(),
			g_TexturName[i],
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

	g_PlayerCount = 0;						// 生きてるプレイヤーの数

	XMFLOAT3 startPosition = XMFLOAT3(400.0f, 400.0f, 0.0f);

	MAPOBJECT* fieldPositions = GetMapObjectByNameFromLayer(MAPOBJLAYER_LOCATIONS, MAPOBJTYPE_PLAYERSTART);

	if (fieldPositions != NULL)
	{
		startPosition = XMFLOAT3((fieldPositions[0].x) - (TEXTURE_WIDTH / 4), (fieldPositions[0].y)-(TEXTURE_HEIGHT/2), 0.0f);
	}

	// プレイヤー構造体の初期化
	for (int i = 0; i < PLAYER_MAX; i++)
	{
		g_PlayerCount++;

		g_Player[i].use = TRUE;
		g_Player[i].pos = startPosition;	// 中心点から表示
		g_Player[i].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Player[i].w = TEXTURE_WIDTH;
		g_Player[i].h = TEXTURE_HEIGHT;
		g_Player[i].collider = COLLIDER2DBOX(PLAYER_COLLIDER_OFFSETX, PLAYER_COLLIDER_OFFSETY, PLAYER_COLLIDER_WIDTH, PLAYER_COLLIDER_HEIGHT);
		g_Player[i].texNo = 0;

		g_Player[i].countAnim = 0;
		g_Player[i].patternAnim = 0;

		g_Player[i].move = XMFLOAT3(4.0f, 0.0f, 0.0f);		// 移動量

		g_Player[i].dir = CHAR_DIR_DOWN;					// 下向きにしとくか
		g_Player[i].moving = FALSE;							// 移動中フラグ
		g_Player[i].patternAnim = g_Player[i].dir * TEXTURE_PATTERN_DIVIDE_X;

		// ジャンプの初期化
		g_Player[i].jump = FALSE;
		g_Player[i].jumpCnt = 0;
		g_Player[i].jumpY = 0.0f;
		g_Player[i].jumpYMax = PLAYER_JUMP_Y_MAX;

		// 分身用
		g_Player[i].dash = FALSE;
		for (int j = 0; j < PLAYER_OFFSET_CNT; j++)
		{
			g_Player[i].offset[j] = g_Player[i].pos;
		}

		// 最初のタイムステートを登録する
		g_Player[i].timeState.status = WRITABLE;
		PushToTimeState(&g_Player[i].timeState,&g_Player[i]);

		RegisterObjectTimeState(&g_Player[i].timeState);
	}

	g_Score = GetUnusedText();
	g_Score->x = SCREEN_WIDTH / 2;
	g_Score->y = 25;
	g_Score->scale = 1;

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitPlayer(void)
{

	if (g_Load == FALSE) return;

	for (int i = 0; i < PLAYER_MAX; i++)
	{
		g_PlayerCount++;

		UnregisterObjectTimeState(&g_Player[i].timeState);

	}

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

	g_Score->use = FALSE;
	g_Score = NULL;

	g_Load = FALSE;
}

//=============================================================================
// 更新処理
//=============================================================================
void UpdatePlayer(void)
{
	if (g_Load == FALSE) return;
	g_PlayerCount = 0;				// 生きてるプレイヤーの数

	for (int i = 0; i < PLAYER_MAX; i++)
	{
		// 生きてるプレイヤーだけ処理をする
		if (g_Player[i].use == TRUE)
		{

			g_PlayerCount++;		// 生きてるプレイヤーの数

			if (g_Player[i].timeState.status == READ_ONLY)
			{

				PullFromTimeState(&g_Player[i].timeState, &g_Player[i]);
				
			}
			else {
				

				// アニメーション  
				if (g_Player[i].moving == TRUE)
				{
					g_Player[i].countAnim += 1.0f;
					if (g_Player[i].countAnim > ANIM_WAIT)
					{
						g_Player[i].countAnim = 0.0f;
						// パターンの切り替え
						g_Player[i].patternAnim = (g_Player[i].dir * TEXTURE_PATTERN_DIVIDE_X) + ((g_Player[i].patternAnim + 1) % TEXTURE_PATTERN_DIVIDE_X);
					}
				}

				// キー入力で移動 
				{
					float speed = PLAYER_WALK_SPEED;

					g_Player[i].moving = FALSE;
					g_Player[i].dash = FALSE;
					g_Player[i].move.x = 0;
					g_Player[i].move.y = 0;

					/*if (GetKeyboardPress(DIK_C) || IsButtonPressed(0, BUTTON_A))
					{
						speed *= 4;
						g_Player[i].dash = TRUE;
					}*/

					int directionMod = 0;

					if (GetKeyboardPress(DIK_S) || IsButtonPressed(0, BUTTON_DOWN))
					{
						g_Player[i].move.y += speed;
						directionMod++;
						g_Player[i].dir = CHAR_DIR_DOWN;
						g_Player[i].moving = TRUE;
					}
					else if (GetKeyboardPress(DIK_W) || IsButtonPressed(0, BUTTON_UP))
					{
						g_Player[i].move.y -= speed;
						directionMod--;
						g_Player[i].dir = CHAR_DIR_UP;
						g_Player[i].moving = TRUE;
					}

					if (GetKeyboardPress(DIK_D) || IsButtonPressed(0, BUTTON_RIGHT))
					{
						g_Player[i].move.x += speed;
						g_Player[i].dir = CHAR_DIR_RIGHT + directionMod;
						g_Player[i].moving = TRUE;
					}
					else if (GetKeyboardPress(DIK_A) || IsButtonPressed(0, BUTTON_LEFT))
					{
						g_Player[i].move.x -= speed;
						g_Player[i].dir = CHAR_DIR_LEFT - directionMod;
						g_Player[i].moving = TRUE;
					}

					

					// MAP外チェック
					BG* bg = GetBG();

					if (g_Player[i].pos.x < 0.0f)
					{
						g_Player[i].pos.x = 0.0f;
					}

					if (g_Player[i].pos.x > bg->w)
					{
						g_Player[i].pos.x = bg->w;
					}

					if (g_Player[i].pos.y < 0.0f)
					{
						g_Player[i].pos.y = 0.0f;
					}

					if (g_Player[i].pos.y > bg->h)
					{
						g_Player[i].pos.y = bg->h;
					}

					//　フィールドの当たり判定
					MAPOBJECT* walls = GetMapObjectsFromLayer(MAPOBJLAYER_WALL);

					XMFLOAT3 newXPos = XMFLOAT3(g_Player[i].pos);
					XMFLOAT3 newYPos = XMFLOAT3(g_Player[i].pos);
					newXPos.x += g_Player[i].move.x;
					newYPos.y += g_Player[i].move.y;

					for (int w = 0; w < MAP_OBJECTS_PER_LAYER_MAX; w++)
					{
						XMFLOAT3 wallPos = XMFLOAT3(walls[w].x, walls[w].y, 0.0f);
						COLLIDER2DBOX wallCollider = COLLIDER2DBOX(0.0f, 0.0f, walls[w].width, walls[w].height);

						// X方の当たり判定
						BOOL ansX = CollisionBB(newXPos, g_Player[i].collider, wallPos, wallCollider);

						if (ansX)
						{
							g_Player[i].move.x = 0;
							newXPos.x = g_Player[i].pos.x;
						}

						// Y方の当たり判定
						BOOL ansY = CollisionBB(newYPos, g_Player[i].collider, wallPos, wallCollider);

						if (ansY)
						{
							g_Player[i].move.y = 0;
							newYPos.y = g_Player[i].pos.y;
						}


					}

					g_Player[i].pos.x = newXPos.x;
					g_Player[i].pos.y = newYPos.y;


					// 移動が終わったらエネミーとの当たり判定
					{
						ENEMY* enemy = GetEnemy();

						// エネミーの数分当たり判定を行う
						for (int j = 0; j < ENEMY_MAX; j++)
						{
							// 生きてるエネミーと当たり判定をする
							if (enemy[j].use == TRUE)
							{
								BOOL ans = CollisionBB(g_Player[i].pos, g_Player[i].w/2, g_Player[i].h/2,
									enemy[j].pos, enemy[j].w/2, enemy[j].h/2);
								// 当たっている？
								if (ans == TRUE)
								{
									// 当たった時の処理
									enemy[j].use = FALSE;
									AddScore(10);
								}
							}
						}
					}

					// アイテムの当たり判定
					{
						ITEM* item = GetItem();

						// エネミーの数分当たり判定を行う
						for (int itm = 0; itm < ITEM_MAX; itm++)
						{
							// 生きてるエネミーと当たり判定をする
							if (item[itm].use == TRUE)
							{
								BOOL ans = CollisionBB(g_Player[i].pos, g_Player[i].w / 2, g_Player[i].h / 2,
									item[itm].pos, item[itm].w / 2, item[itm].h / 2);
								// 当たっている？
								if (ans == TRUE)
								{
									// 当たった時の処理
									switch (item[itm].id)
									{
									case KEY:
										g_Player[i].inventoryKeys++;
										break;
									case SOUL:
										g_Player[i].inventorySouls++;
										break;
									default:
										break;
									}

									item[itm].use = FALSE;
								}
							}
						}

					}

					// バレット処理
					/*if (GetKeyboardTrigger(DIK_SPACE))
					{
						XMFLOAT3 pos = g_Player[i].pos;
						pos.y += g_Player[i].jumpY;
						SetBullet(pos);
					}

					if (IsButtonTriggered(0, BUTTON_B))
					{
						XMFLOAT3 pos = g_Player[i].pos;
						pos.y += g_Player[i].jumpY;
						SetBullet(pos);
					}*/

					/// タイムステートの登録
					PushToTimeState(&g_Player[i].timeState, &g_Player[i]);


				}

			}

			// 地形との当たり判定用に座標のバックアップを取っておく
			XMFLOAT3 pos_old = g_Player[i].pos;

			// 分身用
			for (int j = PLAYER_OFFSET_CNT - 1; j > 0; j--)
			{
				g_Player[i].offset[j] = g_Player[i].offset[j - 1];
			}
			g_Player[i].offset[0] = pos_old;

			// MAP外チェック
			BG* bg = GetBG();

			// プレイヤーの立ち位置からMAPのスクロール座標を計算する
			bg->pos.x = g_Player[i].pos.x - PLAYER_DISP_X;
			if (bg->pos.x < 0) bg->pos.x = 0;
			if (bg->pos.x > bg->w - SCREEN_WIDTH) bg->pos.x = bg->w - SCREEN_WIDTH;

			bg->pos.y = g_Player[i].pos.y - PLAYER_DISP_Y;
			if (bg->pos.y < 0) bg->pos.y = 0;
			if (bg->pos.y > bg->h - SCREEN_HEIGHT) bg->pos.y = bg->h - SCREEN_HEIGHT;

		}
	}

	if (GetKeyboardTrigger(DIK_K) || IsButtonTriggered(0, BUTTON_X)) {
		
		DeactivateTimeMachine();
		
	}

	if (GetKeyboardPress(DIK_Q) || IsButtonPressed(0,BUTTON_L)) {

		RewindTimeMachine(60);

	}

	if (GetKeyboardPress(DIK_E) || IsButtonPressed(0, BUTTON_R)) {

		FastForwardTimeMachine(60);

	}


	// 現状をセーブする
	if (GetKeyboardTrigger(DIK_S))
	{
		SaveData();
	}

	std::wstring wstr = std::to_wstring(GetScore());

	SetText(g_Score, (wchar_t*)wstr.c_str());


#ifdef _DEBUG	// デバッグ情報を表示する


#endif

}

//=============================================================================
// 描画処理
//=============================================================================
void DrawPlayer(void)
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

	for (int i = 0; i < PLAYER_MAX; i++)
	{
		if (g_Player[i].use == TRUE)		// このプレイヤーが使われている？
		{									// Yes

			{	// 影表示
				SetBlendState(BLEND_MODE_SUBTRACT);	// 減算合成

				// テクスチャ設定
				GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[1]);

				float px = g_Player[i].pos.x - bg->pos.x;	// プレイヤーの表示位置X
				float py = g_Player[i].pos.y - bg->pos.y;	// プレイヤーの表示位置Y
				float pw = g_Player[i].w;		// プレイヤーの表示幅
				float ph = g_Player[i].h/4;		// プレイヤーの表示高さ
				py += 35.0f;		// 足元に表示

				float tw = 1.0f;	// テクスチャの幅
				float th = 1.0f;	// テクスチャの高さ
				float tx = 0.0f;	// テクスチャの左上X座標
				float ty = 0.0f;	// テクスチャの左上Y座標

				// １枚のポリゴンの頂点とテクスチャ座標を設定
				SetSpriteColor(g_VertexBuffer, px, py, pw, ph, tx, ty, tw, th,
					XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

				// ポリゴン描画
				GetDeviceContext()->Draw(4, 0);

				SetBlendState(BLEND_MODE_ALPHABLEND);	// 半透明処理を元に戻す

			}

			// プレイヤーの分身を描画
			if (g_Player[i].dash)
			{	// ダッシュ中だけ分身処理
				DrawPlayerOffset(i);
			}

			// テクスチャ設定
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_Player[i].texNo]);

			//プレイヤーの位置やテクスチャー座標を反映
			float px = g_Player[i].pos.x - bg->pos.x;	// プレイヤーの表示位置X
			float py = g_Player[i].pos.y - bg->pos.y;	// プレイヤーの表示位置Y
			float pw = g_Player[i].w;		// プレイヤーの表示幅
			float ph = g_Player[i].h;		// プレイヤーの表示高さ

			py += g_Player[i].jumpY;		// ジャンプ中の高さを足す

			// アニメーション用
			float tw = 1.0f / TEXTURE_PATTERN_DIVIDE_X;	// テクスチャの幅
			float th = 1.0f / TEXTURE_PATTERN_DIVIDE_Y;	// テクスチャの高さ
			float tx = (float)(g_Player[i].patternAnim % TEXTURE_PATTERN_DIVIDE_X) * tw;	// テクスチャの左上X座標
			float ty = (float)(g_Player[i].patternAnim / TEXTURE_PATTERN_DIVIDE_X) * th;	// テクスチャの左上Y座標

			//float tw = 1.0f;	// テクスチャの幅
			//float th = 1.0f;	// テクスチャの高さ
			//float tx = 0.0f;	// テクスチャの左上X座標
			//float ty = 0.0f;	// テクスチャの左上Y座標

			// １枚のポリゴンの頂点とテクスチャ座標を設定
			SetSpriteColorRotation(g_VertexBuffer, px, py, pw, ph, tx, ty, tw, th,
				XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
				g_Player[i].rot.z);

			// ポリゴン描画
			GetDeviceContext()->Draw(4, 0);

		}
	}


}


//=============================================================================
// Player構造体の先頭アドレスを取得
//=============================================================================
PLAYER* GetPlayer(void)
{
	return &g_Player[0];
}


// 生きてるエネミーの数
int GetPlayerCount(void)
{
	return g_PlayerCount;
}


//=============================================================================
// プレイヤーの分身を描画
//=============================================================================
void DrawPlayerOffset(int no)
{
	BG* bg = GetBG();
	float alpha = 0.0f;

	// テクスチャ設定
	GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_Player[no].texNo]);

	for (int j = PLAYER_OFFSET_CNT - 1; j >= 0; j--)
	{
		//プレイヤーの位置やテクスチャー座標を反映
		float px = g_Player[no].offset[j].x - bg->pos.x;	// プレイヤーの表示位置X
		float py = g_Player[no].offset[j].y - bg->pos.y;	// プレイヤーの表示位置Y
		float pw = g_Player[no].w;		// プレイヤーの表示幅
		float ph = g_Player[no].h;		// プレイヤーの表示高さ

		// アニメーション用
		float tw = 1.0f / TEXTURE_PATTERN_DIVIDE_X;	// テクスチャの幅
		float th = 1.0f / TEXTURE_PATTERN_DIVIDE_Y;	// テクスチャの高さ
		float tx = (float)(g_Player[no].patternAnim % TEXTURE_PATTERN_DIVIDE_X) * tw;	// テクスチャの左上X座標
		float ty = (float)(g_Player[no].patternAnim / TEXTURE_PATTERN_DIVIDE_X) * th;	// テクスチャの左上Y座標


		// １枚のポリゴンの頂点とテクスチャ座標を設定
		SetSpriteColorRotation(g_VertexBuffer, px, py, pw, ph, tx, ty, tw, th,
			XMFLOAT4(1.0f, 1.0f, 1.0f, alpha),
			g_Player[no].rot.z);

		alpha += (1.0f / PLAYER_OFFSET_CNT);

		// ポリゴン描画
		GetDeviceContext()->Draw(4, 0);
	}
}

void PushToTimeState(TIMESTATE* timeState, PLAYER* player) 
{
	timeState->x = player->pos.x;
	timeState->y = player->pos.y;
	timeState->countAnim = player->countAnim;
	timeState->patternAnim = player->patternAnim;
}

void PullFromTimeState(TIMESTATE* timeState, PLAYER* player) 
{
	player->pos.x = timeState->x;
	player->pos.y = timeState->y;
	player->countAnim = timeState->countAnim;
	player->patternAnim = timeState->patternAnim;
	player->dash = TRUE;
}



