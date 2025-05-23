//=============================================================================
//
// サウンド処理 [sound.h]
// https://opengameart.org/content/ambient-relaxing-loop
//
//=============================================================================
#pragma once

#include <windows.h>
#include "xaudio2.h"						// サウンド処理で必要

//*****************************************************************************
// サウンドファイル
//*****************************************************************************
enum 
{
	SOUND_LABEL_BGM_MAIN_MENU,	// BGM0
	SOUND_LABEL_BGM_MAIN_LEVEL,	// BGM1
	SOUND_LABEL_BGM_YOUWIN,	// BGM2
	SOUND_LABEL_BGM_YOULOOSE,	// BGM2
	SOUND_LABEL_SE_FOOTSTEP_1,		// 爆発音
	SOUND_LABEL_SE_FOOTSTEP_2,
	SOUND_LABEL_SE_FOOTSTEP_3,
	SOUND_LABEL_SE_SWORD_HIT_1,	// 
	SOUND_LABEL_SE_SWORD_HIT_2,	// 
	SOUND_LABEL_SE_PAIN_1,		// 
	SOUND_LABEL_SE_PAIN_2,	// 
	SOUND_LABEL_SE_FALL_SCREAM,
	SOUND_LABEL_SE_SWORD_SWIPE_1,	// 
	SOUND_LABEL_SE_SWORD_SWIPE_2,		// 
	SOUND_LABEL_SE_SWORD_2_SWIPE_1,		// 
	SOUND_LABEL_SE_SWORD_2_SWIPE_2,		// 
	SOUND_LABEL_SE_FALL,
	SOUND_LABEL_SE_ROCK_CRACK,
	SOUND_LABEL_SE_DOOR_LOCKED,
	SOUND_LABEL_SE_DOOR_OPEN,
	SOUND_LABEL_SE_MASTER_DOOR_OPEN,
	SOUND_LABEL_SE_SWITCH_OK_0,
	SOUND_LABEL_SE_SWITCH_OK_1,
	SOUND_LABEL_SE_SWITCH_OK_2,
	SOUND_LABEL_SE_SWITCH_OK_3,
	SOUND_LABEL_SE_SWITCH_OK_4,
	SOUND_LABEL_SE_SWITCH_OK_5,
	SOUND_LABEL_SE_SWITCH_OK_6,
	SOUND_LABEL_SE_SWITCH_OK_7,
	SOUND_LABEL_SE_SWITCH_OK_8,
	SOUND_LABEL_SE_SWITCH_OK_9,
	SOUND_LABEL_SE_SWITCH_WRONG,
	SOUND_LABEL_SE_COIN_UP,
	SOUND_LABEL_SE_MONEY_UP,
	SOUND_LABEL_SE_ITEM_UP,
	SOUND_LABEL_MAX,
};

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
bool InitSound(HWND hWnd);
void UninitSound(void);
void PlaySound(int label);
void StopSound(int label);
void StopSound(void);

