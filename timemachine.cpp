//=============================================================================
//
// タイムマシン処理 [score.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "timemachine.h"
#include "sprite.h"
#include "gui.h"
#include "enemy.h"
#include <iostream>
#include <chrono>
#include <ctime>
#include <string>
#include <iostream>
#include <algorithm>

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TIMESTATES_PER_SECOND		(60)	// キャラサイズ
#define TEXTURE_HEIGHT				(32)	// 
#define TEXTURE_MAX					(1)		// テクスチャの数


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer				*g_VertexBuffer = NULL;		// 頂点情報
static ID3D11ShaderResourceView	*g_Texture[TEXTURE_MAX] = { NULL };	// テクスチャ情報

static char *g_TexturName[] = {
	"data/TEXTURE/number16x32.png",
};


static bool						g_Use;						// true:使っている  false:未使用
static BOOL                     g_Active;

static TIMESTATE g_Timeline[TIMEOBJECTS_MAX][TIMESTATE_MAX];
static TIMESTATE* g_RegisteredObjTS[TIMEOBJECTS_MAX];

static DWORD g_CurrentTime, g_LastExecutedTime;
int g_ElapsedTime, g_LastElapsedTime;
static int g_CurrentTimeStateIndex = 0;
static int g_LastTimeStateRecorded = -1;

static float					g_w, g_h;					// 幅と高さ
static XMFLOAT3					g_Pos;						// ポリゴンの座標
static int						g_TexNo;					// テクスチャ番号

static int						g_Score;					// スコア

static BMPTEXT*	g_ElapsedTimeText;
static BMPTEXT* g_snapshotIndex;

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitTimeMachine(void)
{
	//ID3D11Device *pDevice = GetDevice();

	////テクスチャ生成
	//for (int i = 0; i < TEXTURE_MAX; i++)
	//{
	//	g_Texture[i] = NULL;
	//	D3DX11CreateShaderResourceViewFromFile(GetDevice(),
	//		g_TexturName[i],
	//		NULL,
	//		NULL,
	//		&g_Texture[i],
	//		NULL);
	//}


	//// 頂点バッファ生成
	//D3D11_BUFFER_DESC bd;
	//ZeroMemory(&bd, sizeof(bd));
	//bd.Usage = D3D11_USAGE_DYNAMIC;
	//bd.ByteWidth = sizeof(VERTEX_3D) * 4;
	//bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	//bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	//GetDevice()->CreateBuffer(&bd, NULL, &g_VertexBuffer);


	//// プレイヤーの初期化
	//g_Use   = true;
	//g_w     = TEXTURE_WIDTH;
	//g_h     = TEXTURE_HEIGHT;
	//g_Pos   = { 500.0f, 20.0f, 0.0f };
	//g_TexNo = 0;

	//g_Score = 0;	// スコアの初期化

	g_ElapsedTime = 0;
	g_LastExecutedTime = g_CurrentTime = timeGetTime();
	g_CurrentTimeStateIndex = 0;

	//g_StartTime = 
	g_ElapsedTimeText = GetUnusedText();
	g_ElapsedTimeText->x = SCREEN_WIDTH - 65;
	g_ElapsedTimeText->y = 25;
	g_ElapsedTimeText->scale = 0.7f;
	/*SetText(g_ElapsedTimeText, L"              Elapsed Time; ");*/

	g_snapshotIndex = GetUnusedText();
	g_snapshotIndex->x = 70;
	g_snapshotIndex->y = 25;
	/*SetText(g_snapshotIndex, L"Enemies; ");*/

	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitTimeMachine(void)
{
	/*if (g_VertexBuffer)
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
	}*/

	if (g_ElapsedTimeText != NULL) {

		g_ElapsedTimeText->use = FALSE;

		g_ElapsedTimeText = NULL;
	}

	if (g_snapshotIndex != NULL) {

		g_snapshotIndex->use = FALSE;

		g_snapshotIndex = NULL;
	}

}

//=============================================================================
// 更新処理
//=============================================================================
void UpdateTimeMachine(void)
{
	g_CurrentTime = timeGetTime();

	g_CurrentTimeStateIndex = g_ElapsedTime / (1000 / TIMESTATES_PER_SECOND);

	// タイムマシンを使うと時間を止めてコントロールできる。
	if (g_Active) {
		g_LastExecutedTime = g_CurrentTime;

		for (int i = 0; i < TIMEOBJECTS_MAX; i++) {

			if (g_RegisteredObjTS[i] != NULL) {

				g_RegisteredObjTS[i]->status = g_Timeline[i][g_CurrentTimeStateIndex].status;
				g_RegisteredObjTS[i]->x = g_Timeline[i][g_CurrentTimeStateIndex].x;
				g_RegisteredObjTS[i]->y = g_Timeline[i][g_CurrentTimeStateIndex].y;
				g_RegisteredObjTS[i]->lastCheckPosX = g_Timeline[i][g_CurrentTimeStateIndex].lastCheckPosX;
				g_RegisteredObjTS[i]->lastCheckPosY = g_Timeline[i][g_CurrentTimeStateIndex].lastCheckPosY;
				g_RegisteredObjTS[i]->countAnim = g_Timeline[i][g_CurrentTimeStateIndex].countAnim;
				g_RegisteredObjTS[i]->patternAnim = g_Timeline[i][g_CurrentTimeStateIndex].patternAnim;
				g_RegisteredObjTS[i]->invincibilityTime = g_Timeline[i][g_CurrentTimeStateIndex].invincibilityTime;
				g_RegisteredObjTS[i]->alive = g_Timeline[i][g_CurrentTimeStateIndex].alive;
				g_RegisteredObjTS[i]->health = g_Timeline[i][g_CurrentTimeStateIndex].health;
				g_RegisteredObjTS[i]->active = g_Timeline[i][g_CurrentTimeStateIndex].active;
				g_RegisteredObjTS[i]->usedInventoryKeys = g_Timeline[i][g_CurrentTimeStateIndex].usedInventoryKeys;
				g_RegisteredObjTS[i]->usedInventoryMKeys = g_Timeline[i][g_CurrentTimeStateIndex].usedInventoryMKeys;
				g_RegisteredObjTS[i]->interactionMode = g_Timeline[i][g_CurrentTimeStateIndex].interactionMode;
				g_RegisteredObjTS[i]->lastSwitchOrderActivated = g_Timeline[i][g_CurrentTimeStateIndex].lastSwitchOrderActivated;
			}

		}

	}
	else {


		if (g_CurrentTimeStateIndex != g_LastTimeStateRecorded) {

			// 登録
			for (int i = 0; i < TIMEOBJECTS_MAX; i++) {
			
				if (g_RegisteredObjTS[i] != NULL) {
				
					TIMESTATE newState;
					newState.id = g_CurrentTimeStateIndex;
					newState.status = READ_ONLY;
					newState.x = g_RegisteredObjTS[i]->x;
					newState.y = g_RegisteredObjTS[i]->y;
					newState.lastCheckPosX = g_RegisteredObjTS[i]->lastCheckPosX;
					newState.lastCheckPosY = g_RegisteredObjTS[i]->lastCheckPosY;
					newState.countAnim = g_RegisteredObjTS[i]->countAnim;
					newState.patternAnim = g_RegisteredObjTS[i]->patternAnim;
					newState.invincibilityTime = g_RegisteredObjTS[i]->patternAnim;
					newState.alive = g_RegisteredObjTS[i]->alive;
					newState.health = g_RegisteredObjTS[i]->health;
					newState.active = g_RegisteredObjTS[i]->active;
					newState.usedInventoryKeys = g_RegisteredObjTS[i]->usedInventoryKeys;
					newState.usedInventoryMKeys = g_RegisteredObjTS[i]->usedInventoryMKeys;
					newState.interactionMode = g_RegisteredObjTS[i]->interactionMode;
					newState.lastSwitchOrderActivated = g_RegisteredObjTS[i]->lastSwitchOrderActivated;

					g_Timeline[i][g_CurrentTimeStateIndex] = newState;


				}
			
			}

			g_LastTimeStateRecorded = g_CurrentTimeStateIndex;
			g_LastElapsedTime = g_ElapsedTime;
		}

	}

	/*if (g_Active == FALSE) {
		
		for (int i = 0; i < TIMEOBJECTS_MAX; i++) 
		{
		
			if (g_RegisteredObjTS[i] != NULL) {
				
				

			}
		
		}

	}
	else {
	
		g_LastExecutedTime = g_CurrentTime;
	
	}*/

	DWORD timeSpan = g_CurrentTime - g_LastExecutedTime;

	g_LastExecutedTime = g_CurrentTime;

	g_ElapsedTime += timeSpan;

	std::wstring tString = L"     ";

	tString.append(std::to_wstring(TIMELIMIT_SECONDS - (g_ElapsedTime / 1000)));

	//std::wstring wstr = std::to_wstring(TIMELIMIT_SECONDS - (g_ElapsedTime / 1000));

	SetText(g_ElapsedTimeText, (wchar_t*)tString.c_str());



	/*std::wstring wstr2 = std::to_wstring(ENEMY_MAX - GetEnemyCount());

	wstr2.append(L" / ");
	wstr2.append( std::to_wstring(ENEMY_MAX));

	SetText(g_snapshotIndex, (wchar_t*)wstr2.c_str());*/


#ifdef _DEBUG	// デバッグ情報を表示する
	//char *str = GetDebugStr();
	//sprintf(&str[strlen(str)], " PX:%.2f PY:%.2f", g_Pos.x, g_Pos.y);
	
#endif

}

//=============================================================================
// 描画処理
//=============================================================================
void DrawTimeMachineGUI(void)
{
	//// 頂点バッファ設定
	//UINT stride = sizeof(VERTEX_3D);
	//UINT offset = 0;
	//GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

	//// マトリクス設定
	//SetWorldViewProjection2D();

	//// プリミティブトポロジ設定
	//GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	//// マテリアル設定
	//MATERIAL material;
	//ZeroMemory(&material, sizeof(material));
	//material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	//SetMaterial(material);

	//// テクスチャ設定
	//GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_TexNo]);

	//// 桁数分処理する
	//int number = g_Score;
	//for (int i = 0; i < SCORE_DIGIT; i++)
	//{
	//	// 今回表示する桁の数字
	//	float x = (float)(number % 10);

	//	// スコアの位置やテクスチャー座標を反映
	//	float px = g_Pos.x - g_w*i;	// スコアの表示位置X
	//	float py = g_Pos.y;			// スコアの表示位置Y
	//	float pw = g_w;				// スコアの表示幅
	//	float ph = g_h;				// スコアの表示高さ

	//	float tw = 1.0f / 10;		// テクスチャの幅
	//	float th = 1.0f / 1;		// テクスチャの高さ
	//	float tx = x * tw;			// テクスチャの左上X座標
	//	float ty = 0.0f;			// テクスチャの左上Y座標

	//	// １枚のポリゴンの頂点とテクスチャ座標を設定
	//	SetSpriteColor(g_VertexBuffer, px, py, pw, ph, tx, ty, tw, th,
	//		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

	//	// ポリゴン描画
	//	GetDeviceContext()->Draw(4, 0);

	//	// 次の桁へ
	//	number /= 10;
	//}
}


//=============================================================================
// スコアを加算する
// 引数:add :追加する点数。マイナスも可能
//=============================================================================
void RegisterObjectTimeState(TIMESTATE* state) 
{
	for (int i = 0; i < TIMEOBJECTS_MAX; i++) {
	
		if (g_RegisteredObjTS[i] == state) {

#ifdef _DEBUG	// デバッグ情報を表示する
			//char *str = GetDebugStr();
			//sprintf(&str[strlen(str)], "タイムステートもう追加された！");

#endif

			break;
		}
		else if (g_RegisteredObjTS[i] == NULL) {
			g_RegisteredObjTS[i] = state;
			break;
		}
	}
}

void UnregisterObjectTimeState(TIMESTATE* state) 
{
	for (int i = 0; i < TIMEOBJECTS_MAX; i++) {

		if (g_RegisteredObjTS[i] == state) {
			g_RegisteredObjTS[i] = NULL;
			break;
		}
		
	}
}

void ActivateTimeMachine(void)
{
	g_Active = TRUE;
	SetTMGUI(TRUE);
}

void DeactivateTimeMachine(void)
{

	for (int i = 0; i < TIMEOBJECTS_MAX; i++) {

		if (g_RegisteredObjTS[i] != NULL) {

			g_RegisteredObjTS[i]->status = WRITABLE;

		}

	}

	g_Active = FALSE;
	SetTMGUI(FALSE);
}

BOOL IsTimeMachineActive(void) 
{
	return g_Active;
}

void RewindTimeMachine(int speedMiliseconds) 
{
	
	g_Active = TRUE;

	g_ElapsedTime -= speedMiliseconds;

	g_ElapsedTime = min(g_LastElapsedTime, max(g_ElapsedTime, 0));

	SetTMGUI(TRUE);

}

void FastForwardTimeMachine(int speedMiliseconds) 
{

	g_Active = TRUE;

	g_ElapsedTime += speedMiliseconds;

	g_ElapsedTime = min(g_LastElapsedTime, max(g_ElapsedTime, 0));

	SetTMGUI(TRUE);

}

int GetTimeMachineElapsedTime() {

	return g_ElapsedTime / 1000;

}

int GetTimeMachineElapsedTime_ms() {

	return g_ElapsedTime;

}

