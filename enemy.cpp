//=============================================================================
//
// エネミー処理 [enemy.cpp]
// Author : 
//
//=============================================================================
#include "enemy.h"
#include "bg.h"
#include "player.h"
#include "fade.h"
#include "collision.h"
#include "field.h"
#include "timemachine.h"
#include "effect.h"
#include "item.h"
#include "sound.h"
#include "score.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TEXTURE_WIDTH				(200/2)	// キャラサイズ
#define TEXTURE_HEIGHT				(200/2)	// 
#define TEXTURE_MAX					(5)		// テクスチャの数

#define TEXTURE_PATTERN_DIVIDE_X	(12)		// アニメパターンのテクスチャ内分割数（X)
#define TEXTURE_PATTERN_DIVIDE_Y	(1)		// アニメパターンのテクスチャ内分割数（Y)
#define ANIM_PATTERN_NUM			(TEXTURE_PATTERN_DIVIDE_X*TEXTURE_PATTERN_DIVIDE_Y)	// アニメーションパターン数
#define ANIM_WAIT					(4)		// アニメーションの切り替わるWait値
#define CMD_WAIT                    (8)

#define ENEMY_INVINCIBILITY_MAX    (30)

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
void SetCharacterState(int state, ENEMY* player, BOOL resetAnim);
void PushToTimeState(TIMESTATE* timeState, ENEMY* player);
void PullFromTimeState(TIMESTATE* timeState, ENEMY* player);

//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer				*g_VertexBuffer = NULL;				// 頂点情報
static ID3D11ShaderResourceView	*g_Texture[TEXTURE_MAX] = { NULL };	// テクスチャ情報

static char *g_TexturName[TEXTURE_MAX] = {
	"data/TEXTURE/candle-burning-only fire.png",
	"data/TEXTURE/bar_white.png",
	"data/TEXTURE/bar_white.png",
	"data/TEXTURE/bar_white.png",
	"data/TEXTURE/bar_white.png"
};


static BOOL		g_Load = FALSE;			// 初期化を行ったかのフラグ
static ENEMY	g_Enemy[ENEMY_MAX];		// エネミー構造体

static int		g_EnemyCount = ENEMY_MAX;

static float offsetx = 200.0f;
static float offsety = 200.0f;

static float moveFactor = 200.0f;
static float e_time = 25.0f;

static BOOL g_swordGiven = FALSE;
static BOOL g_masterKeyGiven = FALSE;
static BOOL g_keyGiven = FALSE;

static ENEMYTYPE g_EnemyTypes[ENEMY_TYPE_MAX] = {
	{
		ENEMY_TYPE_SKELETON,
		{
			{CHAR_ANIM_IDLE, 16, 4, 1, 6, 4},
			{CHAR_ANIM_WALK, 0, 4, 1, 6, 4},
			{CHAR_ANIM_FALL, 0, 4, 1, 6, 4},
			{CHAR_ANIM_DIE, 48, 4, 0, 10, 4},
			{CHAR_ANIM_ATTACK, 0, 4, 1, 6, 4}
		},
		20
	},
	{
		ENEMY_TYPE_SKELETON_WARRIOR,
		{
			{CHAR_ANIM_IDLE, 0, 4, 1, 6, 4},
			{CHAR_ANIM_WALK, 16, 4, 1, 6, 4},
			{CHAR_ANIM_FALL, 0, 4, 1, 6, 4},
			{CHAR_ANIM_DIE, 48, 4, 0, 10, 4},
			{CHAR_ANIM_ATTACK, 64, 4, 0, 6, 4}
		},
		100
	},
	{
		ENEMY_TYPE_MIMIC,
		{
			{CHAR_ANIM_IDLE, 0, 4, 1, 6, 4},
			{CHAR_ANIM_WALK, 16, 4, 1, 6, 4},
			{CHAR_ANIM_FALL, 0, 4, 1, 6, 4},
			{CHAR_ANIM_DIE, 48, 4, 0, 10, 4},
			{CHAR_ANIM_ATTACK, 64, 4, 0, 6, 4}
		},
		200
	},
	{
		ENEMY_TYPE_ROGUE_SKELETON,
		{
			{CHAR_ANIM_IDLE, 0, 4, 1, 6, 4},
			{CHAR_ANIM_WALK, 16, 4, 1, 6, 4},
			{CHAR_ANIM_FALL, 0, 4, 1, 6, 4},
			{CHAR_ANIM_DIE, 48, 4, 0, 10, 4},
			{CHAR_ANIM_ATTACK, 64, 4, 0, 6, 4}
		},
		200
	},
	{
		ENEMY_TYPE_SKELETON_KEY,
		{
			{CHAR_ANIM_IDLE, 16, 4, 1, 6, 4},
			{CHAR_ANIM_WALK, 0, 4, 1, 6, 4},
			{CHAR_ANIM_FALL, 0, 4, 1, 6, 4},
			{CHAR_ANIM_DIE, 48, 4, 0, 10, 4},
			{CHAR_ANIM_ATTACK, 0, 4, 1, 6, 4}
		},
		20
	},
};

static TILESET* g_EnemiesTileset;

static INTERPOLATION_DATA g_MoveTbl0[] = {
	//座標									回転率							拡大率					時間
	{ XMFLOAT3( offsetx,  offsety, 0.0f),	XMFLOAT3(0.0f, 0.0f, 5.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f),	e_time },
	{ XMFLOAT3( offsetx + moveFactor,  offsety, 0.0f),	XMFLOAT3(0.0f, 0.0f, 2.0f),	XMFLOAT3(0.3f, 1.0f, 1.0f),	e_time },
	{ XMFLOAT3( offsetx, offsety + (moveFactor*0.9f), 0.0f),	XMFLOAT3(0.0f, 0.0f, -3.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f),	e_time },
	{ XMFLOAT3( offsetx + (moveFactor/2),  offsety - (moveFactor*0.4f), 0.0f),	XMFLOAT3(0.0f, 0.0f, 3.0f),	XMFLOAT3(0.3f, 1.0f, 1.0f),	e_time},
	{ XMFLOAT3( offsetx + moveFactor, offsety + (moveFactor*0.9f), 0.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),	XMFLOAT3(-1.0f, 1.0f, 1.0f),	e_time },

};


static INTERPOLATION_DATA g_MoveTbl1[] = {
	//座標									回転率							拡大率							時間
	{ XMFLOAT3(1700.0f,   0.0f, 0.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f),	60 },
	{ XMFLOAT3(1700.0f,  SCREEN_HEIGHT, 0.0f),XMFLOAT3(0.0f, 0.0f, 6.28f),	XMFLOAT3(2.0f, 2.0f, 1.0f),	60 },
};


static INTERPOLATION_DATA g_MoveTbl2[] = {
	//座標									回転率							拡大率							時間
	{ XMFLOAT3(3000.0f, 100.0f, 0.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f),	60 },
	{ XMFLOAT3(3000 + SCREEN_WIDTH, 100.0f, 0.0f),	XMFLOAT3(0.0f, 0.0f, 6.28f),	XMFLOAT3(1.0f, 1.0f, 1.0f),	60 },
};


static INTERPOLATION_DATA* g_MoveTblAdr[] =
{
	g_MoveTbl0,
	g_MoveTbl1,
	g_MoveTbl2,

};


//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitEnemy(void)
{

	g_swordGiven = FALSE;
	g_masterKeyGiven = FALSE;
	g_keyGiven = FALSE;

	MAPOBJECT* mapObjects = GetMapObjectsFromLayer(MAPOBJLAYER_ENEMIES);

	ID3D11Device *pDevice = GetDevice();

	g_EnemiesTileset = GetTilesetByName(TILESET_ENEMIES_NAME);

	//テクスチャ生成
	for (int i = 0; i < TEXTURE_MAX; i++)
	{

		if (g_EnemiesTileset->customTiles[i].id < 0)
			continue;

		g_Texture[i] = NULL;
		D3DX11CreateShaderResourceViewFromFile(GetDevice(),
			g_EnemiesTileset->customTiles[i].textureSource,
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

	MAPOBJECT* fieldPositions = GetMapObjectsFromLayer(MAPOBJLAYER_ENEMY);

	// エネミー構造体の初期化
	g_EnemyCount = 0;
	for (int i = 0; i < ENEMY_MAX; i++)
	{

		g_EnemyCount++;
		g_Enemy[i].use = mapObjects[i].id > 0 ? TRUE : FALSE;

		if (!g_Enemy[i].use)
			continue;

		g_Enemy[i].alive = TRUE;

		g_Enemy[i].maxInvincibilityTime = ENEMY_INVINCIBILITY_MAX;

		g_Enemy[i].pos = XMFLOAT3((mapObjects[i].x), (mapObjects[i].y - (mapObjects[i].height)), 0.0f);	// 中心点から表示
		g_Enemy[i].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Enemy[i].scl = XMFLOAT3(1.0f, 1.0f, 1.0f);
		g_Enemy[i].w = mapObjects[i].width;
		g_Enemy[i].h = mapObjects[i].height;
		g_Enemy[i].collider = COLLIDER2DBOX(0.0f, 0.0f, (float)mapObjects[i].width, (float)mapObjects[i].height);
		g_Enemy[i].texNo = mapObjects[i].gid - g_EnemiesTileset->firstGID;
		g_Enemy[i].type = g_Enemy[i].texNo;

		g_Enemy[i].hp = g_EnemyTypes[g_Enemy[i].type].maxHP;

		g_Enemy[i].countAnim = 0;
		g_Enemy[i].patternAnim = 0;

		g_Enemy[i].dir = ENEMY_DIR_DOWN;
		g_Enemy[i].currentAnimState = CHAR_ANIM_IDLE;

		

		g_Enemy[i].animDivideX = g_EnemiesTileset->customTiles[g_Enemy[i].texNo].animDivideX;
		g_Enemy[i].animDivideY = g_EnemiesTileset->customTiles[g_Enemy[i].texNo].animDivideY;
		g_Enemy[i].patternAnimNum = g_EnemiesTileset->customTiles[g_Enemy[i].texNo].patternAnimNum;

		g_Enemy[i].move = XMFLOAT3(4.0f, 0.0f, 0.0f);		// 移動量

		g_Enemy[i].target = NULL;
		g_Enemy[i].countForNextCmd = 0;
		g_Enemy[i].nextCmdWait = 50;

		g_Enemy[i].time = 0.0f;			// 線形補間用のタイマーをクリア
		g_Enemy[i].tblNo = 0;			// 再生する行動データテーブルNoをセット
		g_Enemy[i].tblMax = 0;			// 再生する行動データテーブルのレコード数をセット

		PushToTimeState(&g_Enemy[i].timeState, &g_Enemy[i]);

		RegisterObjectTimeState(&g_Enemy[i].timeState);
	}

	//// 0番だけ線形補間で動かしてみる
	//g_Enemy[0].time = 0.0f;		// 線形補間用のタイマーをクリア
	//g_Enemy[0].tblNo = 0;		// 再生するアニメデータの先頭アドレスをセット
	//g_Enemy[0].tblMax = sizeof(g_MoveTbl0) / sizeof(INTERPOLATION_DATA);	// 再生するアニメデータのレコード数をセット

	//// 1番だけ線形補間で動かしてみる
	//g_Enemy[1].time = 0.0f;		// 線形補間用のタイマーをクリア
	//g_Enemy[1].tblNo = 1;		// 再生するアニメデータの先頭アドレスをセット
	//g_Enemy[1].tblMax = sizeof(g_MoveTbl1) / sizeof(INTERPOLATION_DATA);	// 再生するアニメデータのレコード数をセット

	//// 2番だけ線形補間で動かしてみる
	//g_Enemy[2].time = 0.0f;		// 線形補間用のタイマーをクリア
	//g_Enemy[2].tblNo = 2;		// 再生するアニメデータの先頭アドレスをセット
	//g_Enemy[2].tblMax = sizeof(g_MoveTbl2) / sizeof(INTERPOLATION_DATA);	// 再生するアニメデータのレコード数をセット

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitEnemy(void)
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

	for (int e = 0; e < ENEMY_MAX; e++) 
	{
		g_Enemy[e].use = FALSE;
	}

	g_Load = FALSE;
}

//=============================================================================
// 更新処理
//=============================================================================
void UpdateEnemy(void)
{
	if (g_Load == FALSE) return;
	g_EnemyCount = 0;			// 生きてるエネミーの数

	for (int i = 0; i < ENEMY_MAX; i++)
	{
		// 生きてるエネミーだけ処理をする
		if (g_Enemy[i].use == TRUE)
		{

			g_EnemyCount++;		// 生きてた敵の数

			if (g_Enemy[i].timeState.status == READ_ONLY)
			{

				PullFromTimeState(&g_Enemy[i].timeState, &g_Enemy[i]);
				continue;
			}

			if (!g_Enemy[i].alive)
			{
				continue;
			}
			
			// 地形との当たり判定用に座標のバックアップを取っておく
			XMFLOAT3 pos_old = g_Enemy[i].pos;

			// アニメーション  
			g_Enemy[i].countAnim += 1.0f;
			if (g_Enemy[i].countAnim > g_EnemyTypes[g_Enemy[i].type].animStates[g_Enemy[i].currentAnimState].animWait)
			{
				g_Enemy[i].countAnim = 0.0f;
				// パターンの切り替え

				ANIM_DATA currentAnimState = g_EnemyTypes[g_Enemy[i].type].animStates[g_Enemy[i].currentAnimState];

				int animStateIndex = currentAnimState.startFrame;
				int frameCountX = currentAnimState.frameCountX;

				if (currentAnimState.numDirectionalFrames > 1)
				{
					animStateIndex += g_Enemy[i].dir * frameCountX;
				}

				int je;

				if (currentAnimState.id == CHAR_ANIM_ATTACK)
					je = 0;

				g_Enemy[i].patternAnim = (animStateIndex)+((g_Enemy[i].patternAnim + 1) % frameCountX);

				if (!currentAnimState.cancellable)
				{
					int lastFrame = animStateIndex + frameCountX;
					if (g_Enemy[i].patternAnim + 1 >= lastFrame) {
						
						if (currentAnimState.id == CHAR_ANIM_DIE)
							g_Enemy[i].alive = FALSE;
						
						SetCharacterState(CHAR_ANIM_IDLE, &g_Enemy[i], FALSE);
						
					}
				}
			}

			if (g_EnemyTypes[g_Enemy[i].type].animStates[g_Enemy[i].currentAnimState].cancellable)
				SetCharacterState(CHAR_ANIM_IDLE, &g_Enemy[i], FALSE);

			float speed = 1;

			g_Enemy[i].moving = FALSE;
			g_Enemy[i].move.x = 0;
			g_Enemy[i].move.y = 0;

			g_Enemy[i].target = NULL;

			// 丸当たり安定、プレイヤーが近い？
			PLAYER* player = GetPlayer();

			for (int p = 0; p < PLAYER_MAX; p++) {

				int distance = 300;

				if (g_Enemy[i].type == ENEMY_TYPE_MIMIC)
					distance = 80;

				BOOL ans = CollisionBC(g_Enemy[i].pos, player[p].pos, distance, player[p].w);

				if (ans) {
				
					BOOL found;

					switch (g_Enemy[i].dir)
					{
						case ENEMY_DIR_UP:
							found = player[p].pos.y <= g_Enemy[i].pos.y;
							break;
						case ENEMY_DIR_RIGHT:
							found = player[p].pos.x >= g_Enemy[i].pos.x;
							break;
						case ENEMY_DIR_DOWN:
							found = player[p].pos.y >= g_Enemy[i].pos.y;
							break;
						case ENEMY_DIR_LEFT:
							found = player[p].pos.x <= g_Enemy[i].pos.x;
							break;

						default:
							break;
					}

					if (g_Enemy[i].type == ENEMY_TYPE_MIMIC)
						found = true;

					if (found) {
						g_Enemy[i].target = &player[p];
						break;
					}
				
				}

				

			}

			if (g_EnemyTypes[g_Enemy[i].type].animStates[g_Enemy[i].currentAnimState].cancellable) {

				// If Wandering mode or enemy is skeleton
				if (g_Enemy[i].target != NULL && g_Enemy[i].type != ENEMY_TYPE_SKELETON && g_Enemy[i].type != ENEMY_TYPE_SKELETON_KEY) {


					PLAYER* player = GetPlayer();

					float x = (g_Enemy[i].target->pos.x - g_Enemy[i].pos.x);
					float y = (g_Enemy[i].target->pos.y - g_Enemy[i].pos.y);

					float norm = sqrt(x * x + y * y);

					float speed = 3;

					if (g_Enemy[i].type == ENEMY_TYPE_MIMIC || g_Enemy[i].type == ENEMY_TYPE_ROGUE_SKELETON)
						speed = 4;

					x *= speed / norm;
					y *= speed / norm;

					g_Enemy[i].move.x = x;
					g_Enemy[i].move.y = y;

					g_Enemy[i].moving = TRUE;

					if (abs(g_Enemy[i].move.x) > abs(g_Enemy[i].move.y))
					{
						g_Enemy[i].dir = g_Enemy[i].move.x > 0 ? ENEMY_DIR_RIGHT : ENEMY_DIR_LEFT;
					}
					else
					{
						g_Enemy[i].dir = g_Enemy[i].move.y > 0 ? ENEMY_DIR_DOWN : ENEMY_DIR_UP;
					}

					// アニメーション  
					if (g_Enemy[i].moving == TRUE)
					{
						SetCharacterState(CHAR_ANIM_WALK, &g_Enemy[i], FALSE);
					}


					if (norm < 40 && g_Enemy[i].currentAnimState != CHAR_ANIM_ATTACK)
					{
						SetCharacterState(CHAR_ANIM_ATTACK, &g_Enemy[i], TRUE);

						XMFLOAT3 hitBox = g_Enemy[i].pos;

						hitBox.x += x;
						hitBox.y += y;

						BOOL damageDone = CollisionBC(g_Enemy[i].pos, g_Enemy[i].target->pos, 300, g_Enemy[i].target->w);

						if (damageDone)
						{

							int dmg = -5;

							if (g_Enemy[i].type == ENEMY_TYPE_MIMIC)
								dmg = -10;

							if (g_Enemy[i].type == ENEMY_TYPE_ROGUE_SKELETON)
								dmg = -7;

							AdjustPlayerHP(g_Enemy[i].target, dmg);
							PlaySound(SOUND_LABEL_SE_SWORD_SWIPE_1);
						}

					}

				}
				else
				{

					if (g_Enemy[i].type != ENEMY_TYPE_MIMIC) {

						g_Enemy[i].countForNextCmd += 1.0f;
						if (g_Enemy[i].countForNextCmd > g_Enemy[i].nextCmdWait)
						{
							g_Enemy[i].countForNextCmd = 0;

							g_Enemy[i].roamingCmdX = (rand() % 3) - 1;
							g_Enemy[i].roamingCmdY = (rand() % 3) - 1;

						}

						if (g_EnemyTypes[g_Enemy[i].type].animStates[g_Enemy[i].currentAnimState].cancellable) {



							if (g_Enemy[i].roamingCmdX > 0)
							{
								g_Enemy[i].move.y += speed;
								g_Enemy[i].dir = ENEMY_DIR_DOWN;

								g_Enemy[i].moving = TRUE;
							}
							else if (g_Enemy[i].roamingCmdX < 0)
							{
								g_Enemy[i].move.y -= speed;
								g_Enemy[i].dir = ENEMY_DIR_UP;

								g_Enemy[i].moving = TRUE;
							}

							if (g_Enemy[i].roamingCmdY > 0)
							{
								g_Enemy[i].move.x += speed;
								g_Enemy[i].dir = ENEMY_DIR_RIGHT;

								g_Enemy[i].moving = TRUE;
							}
							else if (g_Enemy[i].roamingCmdY < 0)
							{
								g_Enemy[i].move.x -= speed;
								g_Enemy[i].dir = ENEMY_DIR_LEFT;

								g_Enemy[i].moving = TRUE;
							}

							if (abs(g_Enemy[i].move.x) > 0 && abs(g_Enemy[i].move.y > 0))
							{
								g_Enemy[i].move.x /= 2;
								g_Enemy[i].move.y /= 2;
							}



							int attack = rand();

							// アニメーション  
							if (g_Enemy[i].moving == TRUE)
							{
								SetCharacterState(CHAR_ANIM_WALK, &g_Enemy[i], FALSE);
							}

							if (attack > 0)
							{
								//SetCharacterState(CHAR_ANIM_ATTACK, &g_Enemy[i], TRUE);



								//SetWeapon(g_Player[i].pos, { 0.0f, 0.0f,0.0f }, WEAPON_TYPE_SWORD, weaponDir);

							}


						}
					}
				}

			}
			
			//　フィールドの当たり判定
			MAPOBJECT* walls = GetMapObjectsFromLayer(MAPOBJLAYER_WALL);

			XMFLOAT3 newXPos = XMFLOAT3(g_Enemy[i].pos);
			XMFLOAT3 newYPos = XMFLOAT3(g_Enemy[i].pos);
			newXPos.x += g_Enemy[i].move.x;
			newYPos.y += g_Enemy[i].move.y;

			for (int w = 0; w < MAP_OBJECTS_PER_LAYER_MAX; w++)
			{
				XMFLOAT3 wallPos = XMFLOAT3(walls[w].x, walls[w].y, 0.0f);
				COLLIDER2DBOX wallCollider = COLLIDER2DBOX(0.0f, 0.0f, walls[w].width, walls[w].height);

				// X方の当たり判定
				BOOL ansX = CollisionBB(newXPos, g_Enemy[i].collider, wallPos, wallCollider);

				if (ansX)
				{
					g_Enemy[i].move.x = 0;
					newXPos.x = g_Enemy[i].pos.x;
				}

				// Y方の当たり判定
				BOOL ansY = CollisionBB(newYPos, g_Enemy[i].collider, wallPos, wallCollider);

				if (ansY)
				{
					g_Enemy[i].move.y = 0;
					newYPos.y = g_Enemy[i].pos.y;
				}


			}

			g_Enemy[i].pos.x = newXPos.x;
			g_Enemy[i].pos.y = newYPos.y;

			if (g_Enemy[i].invincibilityTime >= 0)
				g_Enemy[i].invincibilityTime--;

			PushToTimeState(&g_Enemy[i].timeState, &g_Enemy[i]);

			// Humming enemies


			// 移動が終わったらエネミーとの当たり判定
			{
				PLAYER* player = GetPlayer();

				// エネミーの数分当たり判定を行う
				for (int j = 0; j < ENEMY_MAX; j++)
				{
					// 生きてるエネミーと当たり判定をする
					if (player[j].use == TRUE)
					{
						BOOL ans = CollisionBB(g_Enemy[i].pos, g_Enemy[i].w, g_Enemy[i].h,
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
void DrawEnemy(void)
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

	for (int i = 0; i < ENEMY_MAX; i++)
	{
		if (g_Enemy[i].use == TRUE)			// このエネミーが使われている？
		{									// Yes
			// テクスチャ設定
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_Enemy[i].texNo]);

			//エネミーの位置やテクスチャー座標を反映
			float px = g_Enemy[i].pos.x - bg->pos.x;	// エネミーの表示位置X
			float py = g_Enemy[i].pos.y - bg->pos.y;	// エネミーの表示位置Y
			float pw = g_Enemy[i].w;		// エネミーの表示幅
			float ph = g_Enemy[i].h;		// エネミーの表示高さ

			// アニメーション用
			float tw = 1.0f / g_Enemy[i].animDivideX;	// テクスチャの幅
			float th = 1.0f / g_Enemy[i].animDivideY;	// テクスチャの高さ
			float tx = (float)(g_Enemy[i].patternAnim % g_Enemy[i].animDivideX) * tw;	// テクスチャの左上X座標
			float ty = (float)(g_Enemy[i].patternAnim / g_Enemy[i].animDivideX) * th;	// テクスチャの左上Y座標

			//float tw = 1.0f;	// テクスチャの幅
			//float th = 1.0f;	// テクスチャの高さ
			//float tx = 0.0f;	// テクスチャの左上X座標
			//float ty = 0.0f;	// テクスチャの左上Y座標

			// １枚のポリゴンの頂点とテクスチャ座標を設定
			SetSpriteColorRotation(g_VertexBuffer, px, py, pw, ph, tx, ty, tw, th,
				XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
				g_Enemy[i].rot.z);

			// ポリゴン描画
			GetDeviceContext()->Draw(4, 0);
		}
	}


	// ゲージのテスト
	{
		// 下敷きのゲージ（枠的な物）
		// テクスチャ設定
		//GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[1]);

		////ゲージの位置やテクスチャー座標を反映
		//float px = 600.0f;		// ゲージの表示位置X
		//float py =  10.0f;		// ゲージの表示位置Y
		//float pw = 300.0f;		// ゲージの表示幅
		//float ph =  30.0f;		// ゲージの表示高さ

		//float tw = 1.0f;	// テクスチャの幅
		//float th = 1.0f;	// テクスチャの高さ
		//float tx = 0.0f;	// テクスチャの左上X座標
		//float ty = 0.0f;	// テクスチャの左上Y座標

		//// １枚のポリゴンの頂点とテクスチャ座標を設定
		//SetSpriteLTColor(g_VertexBuffer,
		//	px, py, pw, ph,
		//	tx, ty, tw, th,
		//	XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f));

		//// ポリゴン描画
		//GetDeviceContext()->Draw(4, 0);


		//// エネミーの数に従ってゲージの長さを表示してみる
		//// テクスチャ設定
		//GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[1]);

		////ゲージの位置やテクスチャー座標を反映
		//pw = pw * ((float)g_EnemyCount / ENEMY_MAX);

		//// １枚のポリゴンの頂点とテクスチャ座標を設定
		//SetSpriteLTColor(g_VertexBuffer,
		//	px, py, pw, ph,
		//	tx, ty, tw, th,
		//	XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f));

		//// ポリゴン描画
		//GetDeviceContext()->Draw(4, 0);


	}




}

void SetCharacterState(int state, ENEMY* enemy, BOOL resetAnim) {

	enemy->currentAnimState = state;

	if (resetAnim == TRUE) {

		int animStateIndex = g_EnemyTypes[enemy->type].animStates[enemy->currentAnimState].startFrame;
		int frameCountX = g_EnemyTypes[enemy->type].animStates[enemy->currentAnimState].frameCountX;

		if (enemy->currentAnimState != CHAR_ANIM_FALL)
		{
			animStateIndex += enemy->dir * frameCountX;
		}

		enemy->patternAnim = animStateIndex;
	}
}

void PushToTimeState(TIMESTATE* timeState, ENEMY* enemy)
{
	timeState->x = enemy->pos.x;
	timeState->y = enemy->pos.y;
	timeState->countAnim = enemy->countAnim;
	timeState->patternAnim = enemy->patternAnim;
	timeState->alive = enemy->alive;
	timeState->invincibilityTime = enemy->invincibilityTime;
	timeState->health = enemy->hp;
}

void PullFromTimeState(TIMESTATE* timeState, ENEMY* enemy)
{
	enemy->pos.x = timeState->x;
	enemy->pos.y = timeState->y;
	enemy->countAnim = timeState->countAnim;
	enemy->patternAnim = timeState->patternAnim;
	enemy->alive = timeState->alive;
	enemy->invincibilityTime = timeState->invincibilityTime;
	enemy->hp = timeState->health;
}


//=============================================================================
// Enemy構造体の先頭アドレスを取得
//=============================================================================
ENEMY* GetEnemy(void)
{
	return &g_Enemy[0];
}


// 生きてるエネミーの数
int GetEnemyCount(void)
{
	return g_EnemyCount;
}

void AdjustEnemyHP(ENEMY* enemy, int ammount) 
{


	if (enemy->invincibilityTime < 0) {

		enemy->hp += ammount;
		enemy->invincibilityTime = enemy->maxInvincibilityTime;

		int effectID = 3;
		int seID = SOUND_LABEL_SE_SWORD_HIT_1;

		if (ammount < -25) {
			effectID = 0;
			seID = SOUND_LABEL_SE_SWORD_HIT_2;
		}

		SetEffect(enemy->pos.x, enemy->pos.y, 3, effectID);
		PlaySound(seID);

		if (enemy->hp < 0) {
			enemy->hp = 0;
			SetCharacterState(CHAR_ANIM_DIE, enemy, TRUE);


			if (enemy->type == ENEMY_TYPE_MIMIC && !g_swordGiven) {
				SetItem(enemy->pos, ITEM_TYPE_MASTER_SWORD);
				AddScore(7000);
				g_swordGiven = TRUE;
			}
			else if (enemy->type == ENEMY_TYPE_ROGUE_SKELETON && !g_masterKeyGiven)
			{
				SetItem(enemy->pos, ITEM_TYPE_MASTER_KEY);
				AddScore(5000);
				g_masterKeyGiven = TRUE;
			}
			else if (enemy->type == ENEMY_TYPE_SKELETON_KEY && !g_keyGiven) 
			{
				SetItem(enemy->pos, ITEM_TYPE_KEY);
				AddScore(1000);
				g_keyGiven = TRUE;
			}
			else {

				int ran = rand() % 10;

				AddScore(500);

				if(ran > 5)
					SetItem(enemy->pos, ITEM_TYPE_HEALTH_POTION);
			}
		}

	}


}


