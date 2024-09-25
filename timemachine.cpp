//=============================================================================
//
// �^�C���}�V������ [score.cpp]
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
// �}�N����`
//*****************************************************************************
#define TIMESTATES_PER_SECOND		(60)	// �L�����T�C�Y
#define TEXTURE_HEIGHT				(32)	// 
#define TEXTURE_MAX					(1)		// �e�N�X�`���̐�


//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************


//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static ID3D11Buffer				*g_VertexBuffer = NULL;		// ���_���
static ID3D11ShaderResourceView	*g_Texture[TEXTURE_MAX] = { NULL };	// �e�N�X�`�����

static char *g_TexturName[] = {
	"data/TEXTURE/number16x32.png",
};


static bool						g_Use;						// true:�g���Ă���  false:���g�p
static BOOL                     g_Active;

static TIMESTATE g_Timeline[TIMEOBJECTS_MAX][TIMESTATE_MAX];
static TIMESTATE* g_RegisteredObjTS[TIMEOBJECTS_MAX];

static DWORD g_CurrentTime, g_LastExecutedTime;
int g_ElapsedTime, g_LastElapsedTime;
static int g_CurrentTimeStateIndex = 0;
static int g_LastTimeStateRecorded = -1;

static float					g_w, g_h;					// ���ƍ���
static XMFLOAT3					g_Pos;						// �|���S���̍��W
static int						g_TexNo;					// �e�N�X�`���ԍ�

static int						g_Score;					// �X�R�A

static BMPTEXT*	g_ElapsedTimeText;
static BMPTEXT* g_snapshotIndex;

//=============================================================================
// ����������
//=============================================================================
HRESULT InitTimeMachine(void)
{
	//ID3D11Device *pDevice = GetDevice();

	////�e�N�X�`������
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


	//// ���_�o�b�t�@����
	//D3D11_BUFFER_DESC bd;
	//ZeroMemory(&bd, sizeof(bd));
	//bd.Usage = D3D11_USAGE_DYNAMIC;
	//bd.ByteWidth = sizeof(VERTEX_3D) * 4;
	//bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	//bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	//GetDevice()->CreateBuffer(&bd, NULL, &g_VertexBuffer);


	//// �v���C���[�̏�����
	//g_Use   = true;
	//g_w     = TEXTURE_WIDTH;
	//g_h     = TEXTURE_HEIGHT;
	//g_Pos   = { 500.0f, 20.0f, 0.0f };
	//g_TexNo = 0;

	//g_Score = 0;	// �X�R�A�̏�����

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
// �I������
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
// �X�V����
//=============================================================================
void UpdateTimeMachine(void)
{
	g_CurrentTime = timeGetTime();

	g_CurrentTimeStateIndex = g_ElapsedTime / (1000 / TIMESTATES_PER_SECOND);

	// �^�C���}�V�����g���Ǝ��Ԃ��~�߂ăR���g���[���ł���B
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

			// �o�^
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


#ifdef _DEBUG	// �f�o�b�O����\������
	//char *str = GetDebugStr();
	//sprintf(&str[strlen(str)], " PX:%.2f PY:%.2f", g_Pos.x, g_Pos.y);
	
#endif

}

//=============================================================================
// �`�揈��
//=============================================================================
void DrawTimeMachineGUI(void)
{
	//// ���_�o�b�t�@�ݒ�
	//UINT stride = sizeof(VERTEX_3D);
	//UINT offset = 0;
	//GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

	//// �}�g���N�X�ݒ�
	//SetWorldViewProjection2D();

	//// �v���~�e�B�u�g�|���W�ݒ�
	//GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	//// �}�e���A���ݒ�
	//MATERIAL material;
	//ZeroMemory(&material, sizeof(material));
	//material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	//SetMaterial(material);

	//// �e�N�X�`���ݒ�
	//GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_TexNo]);

	//// ��������������
	//int number = g_Score;
	//for (int i = 0; i < SCORE_DIGIT; i++)
	//{
	//	// ����\�����錅�̐���
	//	float x = (float)(number % 10);

	//	// �X�R�A�̈ʒu��e�N�X�`���[���W�𔽉f
	//	float px = g_Pos.x - g_w*i;	// �X�R�A�̕\���ʒuX
	//	float py = g_Pos.y;			// �X�R�A�̕\���ʒuY
	//	float pw = g_w;				// �X�R�A�̕\����
	//	float ph = g_h;				// �X�R�A�̕\������

	//	float tw = 1.0f / 10;		// �e�N�X�`���̕�
	//	float th = 1.0f / 1;		// �e�N�X�`���̍���
	//	float tx = x * tw;			// �e�N�X�`���̍���X���W
	//	float ty = 0.0f;			// �e�N�X�`���̍���Y���W

	//	// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
	//	SetSpriteColor(g_VertexBuffer, px, py, pw, ph, tx, ty, tw, th,
	//		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

	//	// �|���S���`��
	//	GetDeviceContext()->Draw(4, 0);

	//	// ���̌���
	//	number /= 10;
	//}
}


//=============================================================================
// �X�R�A�����Z����
// ����:add :�ǉ�����_���B�}�C�i�X���\
//=============================================================================
void RegisterObjectTimeState(TIMESTATE* state) 
{
	for (int i = 0; i < TIMEOBJECTS_MAX; i++) {
	
		if (g_RegisteredObjTS[i] == state) {

#ifdef _DEBUG	// �f�o�b�O����\������
			//char *str = GetDebugStr();
			//sprintf(&str[strlen(str)], "�^�C���X�e�[�g�����ǉ����ꂽ�I");

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

