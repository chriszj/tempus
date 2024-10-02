//=============================================================================
//
// プレイヤー処理 [player.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "player.h"
#include "input.h"
#include "bg.h"
#include "field.h"
#include "weapon.h"
#include "item.h"
#include "enemy.h"
#include "collision.h"
#include "score.h"
#include "file.h"
#include "timemachine.h"
#include "gui.h"
#include <string>
#include <iostream>
#include "effect.h"
#include "interactables.h"
#include "sound.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TEXTURE_WIDTH				(200/2)	// キャラサイズ
#define TEXTURE_HEIGHT				(200/2)	// 
#define TEXTURE_MAX					(4)		// テクスチャの数

#define TEXTURE_PATTERN_DIVIDE_X	(4)		// アニメパターンのテクスチャ内分割数（X)
#define TEXTURE_PATTERN_DIVIDE_Y	(40)		// アニメパターンのテクスチャ内分割数（Y)
#define ANIM_PATTERN_NUM			(TEXTURE_PATTERN_DIVIDE_X*TEXTURE_PATTERN_DIVIDE_Y)	// アニメーションパターン数
#define ANIM_WAIT					(6)		// アニメーションの切り替わるWait値

#define PLAYER_ANIM_WALK_INDEX      (0)

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

#define PLAYER_INVINCIBILITY_MAX    (30)
#define PLAYER_MAX_HP               (100)

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
void DrawPlayerOffset(int no);
void PushToTimeState(TIMESTATE* timeState, PLAYER* player);
void PullFromTimeState(TIMESTATE* timeState, PLAYER* player);
void SetCharacterState(int state, PLAYER* player, BOOL resetAnim);


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer				*g_VertexBuffer = NULL;				// 頂点情報
static ID3D11ShaderResourceView	*g_Texture[TEXTURE_MAX] = { NULL };	// テクスチャ情報

static char *g_TexturName[TEXTURE_MAX] = {
	"data/TEXTURE/character_hero.png",
	"data/TEXTURE/shadow000.jpg",
	"data/TEXTURE/items_sheet.png",
	"data/TEXTURE/bar_white.png"
};

static ANIM_DATA g_anims[CHAR_ANIM_MAX] =
{
	{CHAR_ANIM_IDLE, 0, 4, 1, 6, 8},
	{CHAR_ANIM_WALK, 32, 4, 1, 6, 8},
	{CHAR_ANIM_FALL, 64, 4, 0, 20, 1},
	{CHAR_ANIM_DIE, 96, 4, 0, 10, 8},
	{CHAR_ANIM_ATTACK, 128, 4, 0, 6, 8}
};

static BOOL		g_Load = FALSE;				// 初期化を行ったかのフラグ
static PLAYER	g_Player[PLAYER_MAX];		// プレイヤー構造体
static int		g_PlayerCount = PLAYER_MAX;	// 生きてるプレイヤーの数
static MAPOBJECT* g_CheckPoints;

static int      g_jumpCnt = 0;
static int		g_jump[PLAYER_JUMP_CNT_MAX] =
{
	-15, -14, -13, -12, -11, -10, -9, -8, -7, -6, -5,-4,-3,-2,-1,
	  1,   2,   3,   4,   5,   6,  7,  8,  9, 10, 11,12,13,14,15
};

static BMPTEXT* g_HPText;
static BMPTEXT* g_CoinText;
static BMPTEXT* g_KeysText;
static BMPTEXT* g_MKeyText;

static BOOL g_reachedGoal;

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitPlayer(void)
{
	g_reachedGoal = FALSE;

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

	g_CheckPoints = GetMapObjectsFromLayer(MAPOBJLAYER_LOCATIONS);

	if (g_CheckPoints != NULL)
	{
		startPosition = XMFLOAT3((g_CheckPoints[0].x) - (TEXTURE_WIDTH / 4), (g_CheckPoints[0].y)-(TEXTURE_HEIGHT/2), 0.0f);
	}

	// プレイヤー構造体の初期化
	for (int i = 0; i < PLAYER_MAX; i++)
	{
		g_PlayerCount++;

		g_Player[i].use = TRUE;
		g_Player[i].pos = startPosition;	// 中心点から表示
		g_Player[i].lastCheckPointPos = startPosition;
		g_Player[i].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Player[i].w = TEXTURE_WIDTH;
		g_Player[i].h = TEXTURE_HEIGHT;
		g_Player[i].collider = COLLIDER2DBOX(PLAYER_COLLIDER_OFFSETX, PLAYER_COLLIDER_OFFSETY, PLAYER_COLLIDER_WIDTH, PLAYER_COLLIDER_HEIGHT);
		g_Player[i].texNo = 0;

		g_Player[i].maxInvincibilityTime = PLAYER_INVINCIBILITY_MAX;
		g_Player[i].hp = PLAYER_MAX_HP;

		g_Player[i].countAnim = 0;
		g_Player[i].patternAnim = 0;

		g_Player[i].move = XMFLOAT3(4.0f, 0.0f, 0.0f);		// 移動量

		g_Player[i].dir = CHAR_DIR_DOWN;					// 下向きにしとくか
		g_Player[i].moving = FALSE;							// 移動中フラグ
		g_Player[i].patternAnim = g_Player[i].dir * TEXTURE_PATTERN_DIVIDE_X;
		g_Player[i].currentAnimState = CHAR_ANIM_IDLE;

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

		g_Player[i].inventoryKeys = 0;
		g_Player[i].inventorySouls = 0;
		g_Player[i].inventoryCoins = 0;
		g_Player[i].inventorySwords = 0;
		g_Player[i].inventoryMasterKeys = 0;

		g_Player[i].usedInventoryKeys = 0;
		g_Player[i].usedInventoryMasterKeys = 0;
		g_Player[i].lastSwitchOrderActivated = -1;

		// 最初のタイムステートを登録する
		g_Player[i].timeState.status = WRITABLE;
		PushToTimeState(&g_Player[i].timeState,&g_Player[i]);

		RegisterObjectTimeState(&g_Player[i].timeState);
	}

	g_HPText = GetUnusedText();
	g_HPText->x = 50;
	g_HPText->y = 25;
	g_HPText->scale = 0.7f;

	g_CoinText = GetUnusedText();
	g_CoinText->x = 50;
	g_CoinText->y = SCREEN_HEIGHT - 50;
	g_CoinText->scale = 0.7f;

	g_KeysText = GetUnusedText();
	g_KeysText->x = SCREEN_WIDTH - 50;
	g_KeysText->y = SCREEN_HEIGHT - 100;
	g_KeysText->scale = 0.7f;

	g_MKeyText = GetUnusedText();
	g_MKeyText->x = SCREEN_WIDTH - 50;
	g_MKeyText->y = SCREEN_HEIGHT - 50;
	g_MKeyText->scale = 0.7f;

	SetPlayerGUI(TRUE);

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitPlayer(void)
{

	if (g_Load == FALSE) return;

	SetPlayerGUI(FALSE);

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

	g_HPText->use = FALSE;
	g_HPText = NULL;

	g_CoinText->use = FALSE;
	g_CoinText = NULL;

	g_KeysText->use = FALSE;
	g_KeysText = NULL;
	
	g_MKeyText->use = FALSE;
	g_MKeyText = NULL;

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


				g_Player[i].countAnim += 1.0f;
				if (g_Player[i].countAnim > g_anims[g_Player[i].currentAnimState].animWait)
				{
					g_Player[i].countAnim = 0.0f;
					// パターンの切り替え
					int animStateIndex = g_anims[g_Player[i].currentAnimState].startFrame;
					int frameCountX = g_anims[g_Player[i].currentAnimState].frameCountX;

					if (g_anims[g_Player[i].currentAnimState].numDirectionalFrames > 1)
					{
						animStateIndex += g_Player[i].dir * frameCountX;
					}

					if (((int)g_Player[i].patternAnim % 2 == 0) && g_anims[g_Player[i].currentAnimState].id == CHAR_ANIM_WALK) {

						int randStepSnd = rand() % 3;

						if(randStepSnd == 0 )
							PlaySound(SOUND_LABEL_SE_FOOTSTEP_1);
						else if (randStepSnd == 1)
							PlaySound(SOUND_LABEL_SE_FOOTSTEP_2);
						else
							PlaySound(SOUND_LABEL_SE_FOOTSTEP_3);

					}

					g_Player[i].patternAnim = (animStateIndex) + ((g_Player[i].patternAnim + 1) % frameCountX);

					if (!g_anims[g_Player[i].currentAnimState].cancellable)
					{
						int lastFrame = animStateIndex + frameCountX;
						if (g_Player[i].patternAnim + 1 >= lastFrame) {
							
							if (g_anims[g_Player[i].currentAnimState].id == CHAR_ANIM_FALL)
								g_Player[i].pos = g_Player[i].lastCheckPointPos;

							SetCharacterState(CHAR_ANIM_IDLE, &g_Player[i], TRUE);
						}
					}

				}

				if (g_anims[g_Player[i].currentAnimState].cancellable) 
					SetCharacterState(CHAR_ANIM_IDLE, &g_Player[i], FALSE);

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

					if (g_anims[g_Player[i].currentAnimState].cancellable) {

						int directionMod = 0;

						int weaponDir = 0;

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

						switch (g_Player[i].dir)
						{
							case CHAR_DIR_UP:
							case CHAR_DIR_UP_RIGHT:
								weaponDir = WEAPON_DIR_UP;
								break;
							case CHAR_DIR_RIGHT:
							case CHAR_DIR_RIGHT_DOWN:
								weaponDir = WEAPON_DIR_RIGHT;
								break;
							case CHAR_DIR_DOWN:
							case CHAR_DIR_DOWN_LEFT:
								weaponDir = WEAPON_DIR_DOWN;
								break;
							default:
								weaponDir = WEAPON_DIR_LEFT;
								break;
						}

						// アニメーション  
						if (g_Player[i].moving == TRUE)
						{
							SetCharacterState(CHAR_ANIM_WALK, &g_Player[i], FALSE);
						}

						if (GetKeyboardTrigger(DIK_J) || IsButtonTriggered(0, BUTTON_B))
						{
							if (g_Player[i].inventorySwords > 0) {
								SetCharacterState(CHAR_ANIM_ATTACK, &g_Player[i], TRUE);

								int weapon = WEAPON_TYPE_SWORD;

								if (g_Player[i].inventorySwords > 1)
									weapon = WEAPON_TYPE_MAGIC_SWORD;

								SetWeapon(g_Player[i].pos, { 0.0f, 0.0f,0.0f }, weapon, weaponDir);
							}
							
						}

						if (GetKeyboardTrigger(DIK_U))
						{
							//SetCharacterState(CHAR_ANIM_FALL, &g_Player[i], TRUE);
							MakePlayerFall(&g_Player[i], 10);
						}
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

					// INTERACTABLES ドア・揺れる床
					{
						INTERACTABLE* interactables = GetInteractables();

						// エネミーの数分当たり判定を行う
						for (int in = 0; in < INTERACTABLES_MAX; in++)
						{

							if (interactables[in].use == FALSE)
								continue;

							if (interactables[in].type == INTERACTABLES_DOOR || interactables[in].type == INTERACTABLES_MASTER_DOOR || interactables[in].type == INTERACTABLES_SWITCH_DOOR)
							{

								if ( (interactables[in].active /*&& interactables[in].activationStatus != -1*/))
								{

									XMFLOAT3 intPos = XMFLOAT3(interactables[in].pos.x, interactables[in].pos.y, 0.0f);

									// X方の当たり判定
									BOOL ansX = CollisionBB(newXPos, g_Player[i].collider, intPos, interactables[in].collider);

									if (ansX)
									{
										g_Player[i].move.x = 0;
										newXPos.x = g_Player[i].pos.x;
									}

									// Y方の当たり判定
									BOOL ansY = CollisionBB(newYPos, g_Player[i].collider, intPos, interactables[in].collider);

									if (ansY)
									{
										g_Player[i].move.y = 0;
										newYPos.y = g_Player[i].pos.y;
									}

									XMFLOAT3 interactionPos = g_Player[i].pos;

									switch (g_Player[i].dir) {

									case CHAR_DIR_UP:
									case CHAR_DIR_LEFT_UP:
									case CHAR_DIR_UP_RIGHT:
										interactionPos.y -= 20;
										break;
									case CHAR_DIR_DOWN:
									case CHAR_DIR_DOWN_LEFT:
									case CHAR_DIR_RIGHT_DOWN:
										interactionPos.y += 20;
										break;
									}


									BOOL ans = CollisionBB(interactionPos, g_Player[i].w / 2, g_Player[i].h / 2,
										interactables[in].pos, interactables[in].w / 2, interactables[in].h / 2);
									//// 当たっている？
									if (ans == TRUE)
									{
										

										if (GetKeyboardTrigger(DIK_K) || IsButtonTriggered(0, BUTTON_X)) {

											if (interactables[in].type == 0) {

												if (g_Player[i].inventoryKeys - g_Player[i].usedInventoryKeys > 0) {
													SetInteractable(&interactables[in], FALSE);
													g_Player[i].usedInventoryKeys++;
													PlaySound(SOUND_LABEL_SE_DOOR_OPEN);
												}
												else
												{
													PlaySound(SOUND_LABEL_SE_DOOR_LOCKED);
												}
											}
											else {

												if (g_Player[i].inventoryMasterKeys - g_Player[i].usedInventoryMasterKeys > 0) {
													SetInteractable(&interactables[in], FALSE);
													g_reachedGoal = TRUE;
													g_Player[i].usedInventoryMasterKeys++;
													PlaySound(SOUND_LABEL_SE_MASTER_DOOR_OPEN);
												}
												else
												{
													PlaySound(SOUND_LABEL_SE_DOOR_LOCKED);
												}

											}

										}



									}

								}

							}
							

						}

					}

					g_Player[i].pos.x = newXPos.x;
					g_Player[i].pos.y = newYPos.y;


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
										case ITEM_TYPE_KEY:
											g_Player[i].inventoryKeys++;
											PlaySound(SOUND_LABEL_SE_ITEM_UP);
											break;
										case ITEM_TYPE_HEALTH_POTION:
											g_Player[i].hp += 40;
											PlaySound(SOUND_LABEL_SE_ITEM_UP);
											break;
										case ITEM_TYPE_SWORD:
											g_Player[i].inventorySwords ++;
											PlaySound(SOUND_LABEL_SE_ITEM_UP);
											break;
										case ITEM_TYPE_MASTER_SWORD:
											g_Player[i].inventorySwords++;
											PlaySound(SOUND_LABEL_SE_ITEM_UP);
											AddScore(10000);
											break;
										case ITEM_TYPE_MASTER_KEY:
											g_Player[i].inventoryMasterKeys++;
											PlaySound(SOUND_LABEL_SE_ITEM_UP);
											AddScore(5000);
											break;
										case ITEM_TYPE_COIN:
											g_Player[i].inventoryCoins++;
											PlaySound(SOUND_LABEL_SE_COIN_UP);
											AddScore(150);
											break;
										case ITEM_TYPE_MONEY:
											g_Player[i].inventoryCoins += 20;
											PlaySound(SOUND_LABEL_SE_MONEY_UP);
											AddScore(600);
											break;
										default:
											break;
									}

									item[itm].use = FALSE;
								}
							}
						}

					}

					// チェックポイント処理
					{
						for (int cp = 0; cp < MAP_OBJECTS_PER_LAYER_MAX; cp++)
						{
							XMFLOAT3 checkPointPos = { g_CheckPoints[cp].x, g_CheckPoints[cp].y, 0.0f };

							BOOL ans = CollisionBC(g_Player[i].pos, checkPointPos, g_Player[i].w/2, 1);
							// 当たっている？
							if (ans == TRUE)
							{
								g_Player[i].lastCheckPointPos = checkPointPos;
							}
						}
					}

					

					if (g_Player[i].invincibilityTime >= 0)
						g_Player[i].invincibilityTime--;

					// バレット処理
					/*if (GetKeyboardTrigger(DIK_SPACE))
					{
						XMFLOAT3 pos = g_Player[i].pos;
						pos.y += g_Player[i].jumpY;
						SetWeapon(pos);
					}

					if (IsButtonTriggered(0, BUTTON_B))
					{
						XMFLOAT3 pos = g_Player[i].pos;
						pos.y += g_Player[i].jumpY;
						SetWeapon(pos);
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

	/*
	std::wstring wstr2 = std::to_wstring(ENEMY_MAX - GetEnemyCount());

	wstr2.append(L" / ");
	wstr2.append( std::to_wstring(ENEMY_MAX));

	SetText(g_snapshotIndex, (wchar_t*)wstr2.c_str());
	*/

	std::wstring hString = L"";

	hString.append(std::to_wstring(g_Player[0].hp));

	SetText(g_HPText, (wchar_t*)hString.c_str());


	std::wstring cString = L"x ";

	cString.append(std::to_wstring(g_Player[0].inventoryCoins));

	SetText(g_CoinText, (wchar_t*)cString.c_str());

	std::wstring kString = L"x ";

	kString.append(std::to_wstring(g_Player[0].inventoryKeys - g_Player[0].usedInventoryKeys));

	SetText(g_KeysText, (wchar_t*)kString.c_str());



	std::wstring mKString = L"x ";

	mKString.append(std::to_wstring(g_Player[0].inventoryMasterKeys - g_Player[0].usedInventoryMasterKeys));

	SetText(g_MKeyText, (wchar_t*)mKString.c_str());


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

			if(g_Player[i].currentAnimState != CHAR_ANIM_FALL)
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

			BOOL skipFrame = FALSE;

			if ((g_Player[i].invincibilityTime >= 0) && ((int)g_Player[i].invincibilityTime % 2 == 0))
				skipFrame = TRUE;

			if (skipFrame)
				return;

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

BOOL HasReachedGoal(void) {
	return g_reachedGoal;
}

void AdjustPlayerHP(PLAYER* player, int ammount) {


	if (player->invincibilityTime < 0) {
		player->hp += ammount;
		player->invincibilityTime = player->maxInvincibilityTime;

		SetEffect(player->pos.x, player->pos.y, 3, 4);

		int randSnd = rand() % 10;

		if (randSnd >= 5)
			PlaySound(SOUND_LABEL_SE_PAIN_1);
		else
			PlaySound(SOUND_LABEL_SE_PAIN_2);

		PlaySound(SOUND_LABEL_SE_SWORD_HIT_1);

		if (player->hp < 0)
			player->use = FALSE;
	}
}


void MakePlayerFall(PLAYER* player, int fallDmg) {
	
	if (g_anims[player->currentAnimState].id == CHAR_ANIM_FALL)
		return;

	player->hp += fallDmg;
	SetCharacterState(CHAR_ANIM_FALL, player, TRUE);

	int randSnd = 6;//rand() % 10;

	//if (randSnd >= 5)
		PlaySound(SOUND_LABEL_SE_FALL);
	//else
		PlaySound(SOUND_LABEL_SE_FALL_SCREAM);

	if (player->hp < 0)
		player->use = FALSE;

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
	timeState->lastCheckPosX = player->lastCheckPointPos.x;
	timeState->lastCheckPosY = player->lastCheckPointPos.y;
	timeState->countAnim = player->countAnim;
	timeState->patternAnim = player->patternAnim;
	timeState->invincibilityTime = player->invincibilityTime;
	timeState->usedInventoryKeys = player->usedInventoryKeys;
	timeState->usedInventoryMKeys = player->usedInventoryMasterKeys;
	timeState->lastSwitchOrderActivated = player->lastSwitchOrderActivated;
}

void PullFromTimeState(TIMESTATE* timeState, PLAYER* player) 
{
	player->pos.x = timeState->x;
	player->pos.y = timeState->y;
	player->lastCheckPointPos.x = timeState->lastCheckPosX;
	player->lastCheckPointPos.y = timeState->lastCheckPosY;
	player->countAnim = timeState->countAnim;
	player->patternAnim = timeState->patternAnim;
	player->dash = TRUE;
	player->invincibilityTime = timeState->invincibilityTime;
	player->usedInventoryKeys = timeState->usedInventoryKeys;
	player->usedInventoryMasterKeys = timeState->usedInventoryMKeys;
	player->lastSwitchOrderActivated = timeState->lastSwitchOrderActivated;

}

void SetCharacterState(int state, PLAYER* player, BOOL resetAnim) {

	player->currentAnimState = state;

	if (resetAnim == TRUE) {

		int animStateIndex = g_anims[player->currentAnimState].startFrame;
		int frameCountX = g_anims[player->currentAnimState].frameCountX;

		if (player->currentAnimState != CHAR_ANIM_FALL)
		{
			animStateIndex += player->dir * frameCountX;
		}

		player->patternAnim = animStateIndex;
	}
}



