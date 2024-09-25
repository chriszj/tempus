//=============================================================================
//
// スコア処理 [score.h]
// Author : 
//
//=============================================================================
#pragma once


//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TIMESTATE_MAX			(5000)		// スコアの最大値
#define TIMEOBJECTS_MAX			(100)			// 桁数
#define TIMELIMIT_SECONDS        (50)

//*****************************************************************************
// 構造体定義
//*****************************************************************************

enum TIMEOBJECTSTATUS
{
	AWAKE,
	LIVING,
	DEAD
};

enum TIMESTATESTATUS
{
	WRITABLE,
	READ_ONLY
};

struct TIMESTATE
{
	//    <char id="32" x="303" y="2046" width="3" height="1" xoffset="-1" yoffset="31" xadvance="6" page="0" chnl="15" />

	int id = -1;
	int elapsedTimeStamp;
	float x, y, invincibilityTime, lastCheckPosX, lastCheckPosY;
	int status, health, countAnim, patternAnim, usedInventoryKeys, usedInventoryMKeys, interactionMode, lastSwitchOrderActivated;
	XMFLOAT4 color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	BOOL alive, active;

};


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitTimeMachine(void);
void UninitTimeMachine(void);
void UpdateTimeMachine(void);
void DrawTimeMachineGUI(void);

void RegisterObjectTimeState(TIMESTATE* state);
void UnregisterObjectTimeState(TIMESTATE* state);
void ActivateTimeMachine(void);
void DeactivateTimeMachine(void);
BOOL IsTimeMachineActive(void);
void RewindTimeMachine(int speedMiliseconds);
void FastForwardTimeMachine(int speedMiliseconds);
int GetTimeMachineElapsedTime();
int GetTimeMachineElapsedTime_ms();



