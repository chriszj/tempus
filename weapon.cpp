//=============================================================================
//
// バレット処理 [bullet.cpp]
// Author : 
//
//=============================================================================
#include "weapon.h"
#include "enemy.h"
#include "collision.h"
#include "score.h"
#include "bg.h"
#include "effect.h"
#include "sound.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TEXTURE_WIDTH				(100/2)	// キャラサイズ
#define TEXTURE_HEIGHT				(100/2)	// 
#define TEXTURE_MAX					(2)		// テクスチャの数

#define TEXTURE_PATTERN_DIVIDE_X	(5)		// アニメパターンのテクスチャ内分割数（X)
#define TEXTURE_PATTERN_DIVIDE_Y	(4)		// アニメパターンのテクスチャ内分割数（Y)
#define ANIM_PATTERN_NUM			(TEXTURE_PATTERN_DIVIDE_X*TEXTURE_PATTERN_DIVIDE_Y)	// アニメーションパターン数
#define ANIM_WAIT					(4)		// アニメーションの切り替わるWait値


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer				*g_VertexBuffer = NULL;				// 頂点情報
static ID3D11ShaderResourceView	*g_Texture[TEXTURE_MAX] = { NULL };	// テクスチャ情報

static char *g_TexturName[] = {
	"data/TEXTURE/weapon.png",
	"data/TEXTURE/weapon2.png"
};

static BOOL		g_Load = FALSE;			// 初期化を行ったかのフラグ
static WEAPON	g_Weapon[BULLET_MAX];	// バレット構造体

static WEAPON_TYPE g_WeaponTypes[WEAPON_TYPE_MAX] =
{
	{WEAPON_TYPE_SWORD, TRUE, TRUE, 200, 200, 5, 4, 4},
	{WEAPON_TYPE_MAGIC_SWORD, TRUE, TRUE, 200, 200, 5, 4, 4}
};

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitWeapon(void)
{
	ID3D11Device *pDevice = GetDevice();

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


	// バレット構造体の初期化
	for (int i = 0; i < BULLET_MAX; i++)
	{
		g_Weapon[i].use   = FALSE;			// 未使用（発射されていない弾）
		g_Weapon[i].w     = TEXTURE_WIDTH;
		g_Weapon[i].h     = TEXTURE_HEIGHT;
		g_Weapon[i].duration = -1;
		g_Weapon[i].elapsedTime = 0;
		g_Weapon[i].pos   = XMFLOAT3(300, 300.0f, 0.0f);
		g_Weapon[i].rot   = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Weapon[i].texNo = 0;

		g_Weapon[i].countAnim = 0;
		g_Weapon[i].patternAnim = 0;

		g_Weapon[i].move = XMFLOAT3(0.0f, -BULLET_SPEED, 0.0f);	// 移動量を初期化
	}
	
	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitWeapon(void)
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

}

//=============================================================================
// 更新処理
//=============================================================================
void UpdateWeapon(void)
{
	if (g_Load == FALSE) return;
	int bulletCount = 0;				// 処理したバレットの数

	for (int i = 0; i < BULLET_MAX; i++)
	{
		if (g_Weapon[i].use == TRUE)	// このバレットが使われている？
		{								// Yes
			// アニメーション  
			g_Weapon[i].countAnim++;
			if ((g_Weapon[i].countAnim % g_WeaponTypes[g_Weapon[i].type].animWait) == 0)
			{

				// パターンの切り替え
				int animStateIndex = 0;
				int frameCountX = g_WeaponTypes[g_Weapon[i].type].textureDivideX;

				if (g_WeaponTypes[g_Weapon[i].type].hasDirectionSprites)
				{
					animStateIndex += g_Weapon[i].dir * frameCountX;
				}

				// パターンの切り替え
				g_Weapon[i].patternAnim = animStateIndex + (g_Weapon[i].patternAnim + 1) % frameCountX;

				if (g_WeaponTypes[g_Weapon[i].type].destroyOnAnimationEnd)
				{
					int lastFrame = animStateIndex + frameCountX;
					if (g_Weapon[i].patternAnim + 1 >= lastFrame)
						g_Weapon[i].use = FALSE;
				}

			}

			// バレットの移動処理
			XMVECTOR pos  = XMLoadFloat3(&g_Weapon[i].pos);
			XMVECTOR move = XMLoadFloat3(&g_Weapon[i].move);
			pos += move;
			XMStoreFloat3(&g_Weapon[i].pos, pos);

			// 画面外まで進んだ？
			BG* bg = GetBG();
			if (g_Weapon[i].pos.y < (-g_Weapon[i].h/2))		// 自分の大きさを考慮して画面外か判定している
			{
				g_Weapon[i].use = false;
			}
			if (g_Weapon[i].pos.y > (bg->h + g_Weapon[i].h/2))	// 自分の大きさを考慮して画面外か判定している
			{
				g_Weapon[i].use = false;
			}

			// 当たり判定処理
			{
				ENEMY* enemy = GetEnemy();

				// エネミーの数分当たり判定を行う
				for (int j = 0; j < ENEMY_MAX; j++)
				{
					// 生きてるエネミーと当たり判定をする
					if (enemy[j].use == TRUE)
					{
						XMFLOAT3 hitBoxPos = g_Weapon[i].pos;
						float hitboxW = g_Weapon[i].w * 0.2f;
						float hitboxH = g_Weapon[i].h * 0.2f;

						hitboxH /= 2;
						hitboxW /= 2;
						
						switch (g_Weapon[i].dir)
						{
							case WEAPON_DIR_UP:
								//hitboxH /= 2;
								hitboxW *= 0.7f;
								hitBoxPos.y -= hitboxH;
								break;
							case WEAPON_DIR_RIGHT:
								//hitboxW /= 2;
								hitboxH *= 0.7f;
								hitBoxPos.x += hitboxW;
								break;
							case WEAPON_DIR_DOWN:
								//hitboxH /= 2;
								hitboxW *= 0.7f;
								hitBoxPos.y += hitboxH;
								break;
							case WEAPON_DIR_LEFT:
								//hitboxW /= 2;
								hitboxH *= 0.7f;
								hitBoxPos.x -= hitboxW;
								break;
						
						}


						BOOL ans = CollisionBB(hitBoxPos, hitboxW, hitboxH,
							enemy[j].pos, enemy[j].w, enemy[j].h);
						// 当たっている？
						if (ans == TRUE)
						{
							// 当たった時の処理
							
							int dmg = -25;

							if (g_Weapon[i].texNo == 1)
								dmg *= 4;

							AdjustEnemyHP(&enemy[j], dmg);

							//AddScore(100);

							// エフェクト発生
							//SetEffect(enemy[j].pos.x, enemy[j].pos.y, 3, 3);
						}
					}
				}
			}


			bulletCount++;
		}
	}


}

//=============================================================================
// 描画処理
//=============================================================================
void DrawWeapon(void)
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

	for (int i = 0; i < BULLET_MAX; i++)
	{
		if (g_Weapon[i].use == TRUE)		// このバレットが使われている？
		{									// Yes
			// テクスチャ設定
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_Weapon[i].texNo]);

			//バレットの位置やテクスチャー座標を反映
			float px = g_Weapon[i].pos.x - bg->pos.x;	// バレットの表示位置X
			float py = g_Weapon[i].pos.y - bg->pos.y;	// バレットの表示位置Y
			float pw = g_Weapon[i].w;		// バレットの表示幅
			float ph = g_Weapon[i].h;		// バレットの表示高さ

			int textureDivideX = g_WeaponTypes[g_Weapon[i].type].textureDivideX;
			int textureDivideY = g_WeaponTypes[g_Weapon[i].type].textureDivideY;

			float tw = 1.0f / textureDivideX;	// テクスチャの幅
			float th = 1.0f / textureDivideY;	// テクスチャの高さ
			float tx = (float)(g_Weapon[i].patternAnim % textureDivideX) * tw;	// テクスチャの左上X座標
			float ty = (float)(g_Weapon[i].patternAnim / textureDivideX) * th;	// テクスチャの左上Y座標

			// １枚のポリゴンの頂点とテクスチャ座標を設定
			SetSpriteColorRotation(g_VertexBuffer, 
				px, py, pw, ph, 
				tx, ty, tw, th,
				XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
				g_Weapon[i].rot.z);

			// ポリゴン描画
			GetDeviceContext()->Draw(4, 0);
		}
	}

}


//=============================================================================
// バレット構造体の先頭アドレスを取得
//=============================================================================
WEAPON *GetWeapon(void)
{
	return &g_Weapon[0];
}


//=============================================================================
// バレットの発射設定
//=============================================================================
void SetWeapon(XMFLOAT3 pos, XMFLOAT3 move, int weaponType, int weaponDirection)
{
	// もし未使用の弾が無かったら発射しない( =これ以上撃てないって事 )
	for (int i = 0; i < BULLET_MAX; i++)
	{
		if (g_Weapon[i].use == FALSE)		// 未使用状態のバレットを見つける
		{
			g_Weapon[i].use = TRUE;			// 使用状態へ変更する
			g_Weapon[i].pos = pos;			// 座標をセット
			g_Weapon[i].move = move;
			g_Weapon[i].w = g_WeaponTypes[weaponType].width;
			g_Weapon[i].h = g_WeaponTypes[weaponType].height;
			g_Weapon[i].duration = -1;
			g_Weapon[i].elapsedTime = 0;
			g_Weapon[i].move.z = 0;         // 2D ゲームだから。Z軸を使わない。
			g_Weapon[i].dir = weaponDirection;
			g_Weapon[i].patternAnim = 0;

			g_Weapon[i].texNo = g_WeaponTypes[weaponType].type;

			int randSnd = rand() % 10;

			switch (weaponType)
			{

			case WEAPON_TYPE_MAGIC_SWORD:

				if (randSnd >= 5)
					PlaySound(SOUND_LABEL_SE_SWORD_2_SWIPE_1);
				else
					PlaySound(SOUND_LABEL_SE_SWORD_2_SWIPE_2);
					
				break;

			default:

				if (randSnd >= 5)
					PlaySound(SOUND_LABEL_SE_SWORD_SWIPE_1);
				else
					PlaySound(SOUND_LABEL_SE_SWORD_SWIPE_2);

				break;
			}

			return;							// 1発セットしたので終了する
		}
	}
}

