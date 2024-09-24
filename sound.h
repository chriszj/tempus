//=============================================================================
//
// �T�E���h���� [sound.h]
// https://opengameart.org/content/ambient-relaxing-loop
//
//=============================================================================
#pragma once

#include <windows.h>
#include "xaudio2.h"						// �T�E���h�����ŕK�v

//*****************************************************************************
// �T�E���h�t�@�C��
//*****************************************************************************
enum 
{
	SOUND_LABEL_BGM_MAIN_MENU,	// BGM0
	SOUND_LABEL_BGM_MAIN_LEVEL,	// BGM1
	SOUND_LABEL_BGM_YOUWIN,	// BGM2
	SOUND_LABEL_BGM_YOULOOSE,	// BGM2
	SOUND_LABEL_SE_FOOTSTEP_1,		// ������
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
	SOUND_LABEL_MAX,
};

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
bool InitSound(HWND hWnd);
void UninitSound(void);
void PlaySound(int label);
void StopSound(int label);
void StopSound(void);

