//=============================================================================
//
// �G�l�~�[���� [enemy.cpp]
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

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define TEXTURE_WIDTH				(200/2)	// �L�����T�C�Y
#define TEXTURE_HEIGHT				(200/2)	// 
#define TEXTURE_MAX					(2)		// �e�N�X�`���̐�

#define TEXTURE_PATTERN_DIVIDE_X	(12)		// �A�j���p�^�[���̃e�N�X�`�����������iX)
#define TEXTURE_PATTERN_DIVIDE_Y	(1)		// �A�j���p�^�[���̃e�N�X�`�����������iY)
#define ANIM_PATTERN_NUM			(TEXTURE_PATTERN_DIVIDE_X*TEXTURE_PATTERN_DIVIDE_Y)	// �A�j���[�V�����p�^�[����
#define ANIM_WAIT					(4)		// �A�j���[�V�����̐؂�ւ��Wait�l
#define CMD_WAIT                    (8)

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
void SetCharacterState(int state, ENEMY* player, BOOL resetAnim);
void PushToTimeState(TIMESTATE* timeState, ENEMY* player);
void PullFromTimeState(TIMESTATE* timeState, ENEMY* player);

//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static ID3D11Buffer				*g_VertexBuffer = NULL;				// ���_���
static ID3D11ShaderResourceView	*g_Texture[TEXTURE_MAX] = { NULL };	// �e�N�X�`�����

static char *g_TexturName[TEXTURE_MAX] = {
	"data/TEXTURE/candle-burning-only fire.png",
	"data/TEXTURE/bar_white.png",
};


static BOOL		g_Load = FALSE;			// ���������s�������̃t���O
static ENEMY	g_Enemy[ENEMY_MAX];		// �G�l�~�[�\����

static int		g_EnemyCount = ENEMY_MAX;

static float offsetx = 200.0f;
static float offsety = 200.0f;

static float moveFactor = 200.0f;
static float e_time = 25.0f;

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
	}
};

static TILESET* g_EnemiesTileset;

static INTERPOLATION_DATA g_MoveTbl0[] = {
	//���W									��]��							�g�嗦					����
	{ XMFLOAT3( offsetx,  offsety, 0.0f),	XMFLOAT3(0.0f, 0.0f, 5.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f),	e_time },
	{ XMFLOAT3( offsetx + moveFactor,  offsety, 0.0f),	XMFLOAT3(0.0f, 0.0f, 2.0f),	XMFLOAT3(0.3f, 1.0f, 1.0f),	e_time },
	{ XMFLOAT3( offsetx, offsety + (moveFactor*0.9f), 0.0f),	XMFLOAT3(0.0f, 0.0f, -3.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f),	e_time },
	{ XMFLOAT3( offsetx + (moveFactor/2),  offsety - (moveFactor*0.4f), 0.0f),	XMFLOAT3(0.0f, 0.0f, 3.0f),	XMFLOAT3(0.3f, 1.0f, 1.0f),	e_time},
	{ XMFLOAT3( offsetx + moveFactor, offsety + (moveFactor*0.9f), 0.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),	XMFLOAT3(-1.0f, 1.0f, 1.0f),	e_time },

};


static INTERPOLATION_DATA g_MoveTbl1[] = {
	//���W									��]��							�g�嗦							����
	{ XMFLOAT3(1700.0f,   0.0f, 0.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f),	60 },
	{ XMFLOAT3(1700.0f,  SCREEN_HEIGHT, 0.0f),XMFLOAT3(0.0f, 0.0f, 6.28f),	XMFLOAT3(2.0f, 2.0f, 1.0f),	60 },
};


static INTERPOLATION_DATA g_MoveTbl2[] = {
	//���W									��]��							�g�嗦							����
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
// ����������
//=============================================================================
HRESULT InitEnemy(void)
{
	MAPOBJECT* mapObjects = GetMapObjectsFromLayer(MAPOBJLAYER_ENEMIES);

	ID3D11Device *pDevice = GetDevice();

	g_EnemiesTileset = GetTilesetByName(TILESET_ENEMIES_NAME);

	//�e�N�X�`������
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


	// ���_�o�b�t�@����
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX_3D) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	GetDevice()->CreateBuffer(&bd, NULL, &g_VertexBuffer);

	MAPOBJECT* fieldPositions = GetMapObjectsFromLayer(MAPOBJLAYER_ENEMY);

	// �G�l�~�[�\���̂̏�����
	g_EnemyCount = 0;
	for (int i = 0; i < ENEMY_MAX; i++)
	{

		g_EnemyCount++;
		g_Enemy[i].use = mapObjects[i].id > 0 ? TRUE : FALSE;

		if (!g_Enemy[i].use)
			continue;

		g_Enemy[i].alive = TRUE;

		g_Enemy[i].pos = XMFLOAT3((mapObjects[i].x), (mapObjects[i].y - (mapObjects[i].height)), 0.0f);	// ���S�_����\��
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

		int customTileWidth = g_EnemiesTileset->customTiles[g_Enemy[i].texNo].width;
		int customTileTextureW = g_EnemiesTileset->customTiles[g_Enemy[i].texNo].textureW;

		int divideX = customTileTextureW / customTileWidth;

		int customTileHeight = g_EnemiesTileset->customTiles[g_Enemy[i].texNo].height;
		int customTileTextureH = g_EnemiesTileset->customTiles[g_Enemy[i].texNo].textureH;
		int divideY = customTileTextureH / customTileHeight;

		g_Enemy[i].animDivideX = divideX;
		g_Enemy[i].animDivideY = divideY;
		g_Enemy[i].patternAnimNum = divideX * divideY;

		g_Enemy[i].move = XMFLOAT3(4.0f, 0.0f, 0.0f);		// �ړ���

		g_Enemy[i].target = NULL;
		g_Enemy[i].countForNextCmd = 0;
		g_Enemy[i].nextCmdWait = 50;

		g_Enemy[i].time = 0.0f;			// ���`��ԗp�̃^�C�}�[���N���A
		g_Enemy[i].tblNo = 0;			// �Đ�����s���f�[�^�e�[�u��No���Z�b�g
		g_Enemy[i].tblMax = 0;			// �Đ�����s���f�[�^�e�[�u���̃��R�[�h�����Z�b�g

		PushToTimeState(&g_Enemy[i].timeState, &g_Enemy[i]);

		RegisterObjectTimeState(&g_Enemy[i].timeState);
	}

	//// 0�Ԃ������`��Ԃœ������Ă݂�
	//g_Enemy[0].time = 0.0f;		// ���`��ԗp�̃^�C�}�[���N���A
	//g_Enemy[0].tblNo = 0;		// �Đ�����A�j���f�[�^�̐擪�A�h���X���Z�b�g
	//g_Enemy[0].tblMax = sizeof(g_MoveTbl0) / sizeof(INTERPOLATION_DATA);	// �Đ�����A�j���f�[�^�̃��R�[�h�����Z�b�g

	//// 1�Ԃ������`��Ԃœ������Ă݂�
	//g_Enemy[1].time = 0.0f;		// ���`��ԗp�̃^�C�}�[���N���A
	//g_Enemy[1].tblNo = 1;		// �Đ�����A�j���f�[�^�̐擪�A�h���X���Z�b�g
	//g_Enemy[1].tblMax = sizeof(g_MoveTbl1) / sizeof(INTERPOLATION_DATA);	// �Đ�����A�j���f�[�^�̃��R�[�h�����Z�b�g

	//// 2�Ԃ������`��Ԃœ������Ă݂�
	//g_Enemy[2].time = 0.0f;		// ���`��ԗp�̃^�C�}�[���N���A
	//g_Enemy[2].tblNo = 2;		// �Đ�����A�j���f�[�^�̐擪�A�h���X���Z�b�g
	//g_Enemy[2].tblMax = sizeof(g_MoveTbl2) / sizeof(INTERPOLATION_DATA);	// �Đ�����A�j���f�[�^�̃��R�[�h�����Z�b�g

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// �I������
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
// �X�V����
//=============================================================================
void UpdateEnemy(void)
{
	if (g_Load == FALSE) return;
	g_EnemyCount = 0;			// �����Ă�G�l�~�[�̐�

	for (int i = 0; i < ENEMY_MAX; i++)
	{
		// �����Ă�G�l�~�[��������������
		if (g_Enemy[i].use == TRUE)
		{

			g_EnemyCount++;		// �����Ă��G�̐�

			if (g_Enemy[i].timeState.status == READ_ONLY)
			{

				PullFromTimeState(&g_Enemy[i].timeState, &g_Enemy[i]);
				continue;
			}

			if (g_Enemy[i].hp <= 0)
				return;
			
			// �n�`�Ƃ̓����蔻��p�ɍ��W�̃o�b�N�A�b�v������Ă���
			XMFLOAT3 pos_old = g_Enemy[i].pos;

			// �A�j���[�V����  
			g_Enemy[i].countAnim += 1.0f;
			if (g_Enemy[i].countAnim > g_EnemyTypes[g_Enemy[i].type].animStates[g_Enemy[i].currentAnimState].animWait)
			{
				g_Enemy[i].countAnim = 0.0f;
				// �p�^�[���̐؂�ւ�

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
						
						
						SetCharacterState(CHAR_ANIM_IDLE, &g_Enemy[i], TRUE);
						
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

			// �ۓ��������A�v���C���[���߂��H
			PLAYER* player = GetPlayer();

			for (int p = 0; p < PLAYER_MAX; p++) {

				BOOL ans = CollisionBC(g_Enemy[i].pos, player[p].pos, 300, player[p].w);

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

					if (found) {
						g_Enemy[i].target = &player[p];
						break;
					}
				
				}

				

			}

			if (g_EnemyTypes[g_Enemy[i].type].animStates[g_Enemy[i].currentAnimState].cancellable) {

				// If Wandering mode or enemy is skeleton
				if (g_Enemy[i].target != NULL && g_Enemy[i].type != ENEMY_TYPE_SKELETON) {


					PLAYER* player = GetPlayer();

					float x = (g_Enemy[i].target->pos.x - g_Enemy[i].pos.x);
					float y = (g_Enemy[i].target->pos.y - g_Enemy[i].pos.y);

					float norm = sqrt(x * x + y * y);

					x *= 3 / norm;
					y *= 3 / norm;

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

					// �A�j���[�V����  
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
							AdjustPlayerHP(g_Enemy[i].target, -5);
						}

					}

				}
				else
				{

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

						// �A�j���[�V����  
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
			
			//�@�t�B�[���h�̓����蔻��
			MAPOBJECT* walls = GetMapObjectsFromLayer(MAPOBJLAYER_WALL);

			XMFLOAT3 newXPos = XMFLOAT3(g_Enemy[i].pos);
			XMFLOAT3 newYPos = XMFLOAT3(g_Enemy[i].pos);
			newXPos.x += g_Enemy[i].move.x;
			newYPos.y += g_Enemy[i].move.y;

			for (int w = 0; w < MAP_OBJECTS_PER_LAYER_MAX; w++)
			{
				XMFLOAT3 wallPos = XMFLOAT3(walls[w].x, walls[w].y, 0.0f);
				COLLIDER2DBOX wallCollider = COLLIDER2DBOX(0.0f, 0.0f, walls[w].width, walls[w].height);

				// X���̓����蔻��
				BOOL ansX = CollisionBB(newXPos, g_Enemy[i].collider, wallPos, wallCollider);

				if (ansX)
				{
					g_Enemy[i].move.x = 0;
					newXPos.x = g_Enemy[i].pos.x;
				}

				// Y���̓����蔻��
				BOOL ansY = CollisionBB(newYPos, g_Enemy[i].collider, wallPos, wallCollider);

				if (ansY)
				{
					g_Enemy[i].move.y = 0;
					newYPos.y = g_Enemy[i].pos.y;
				}


			}

			g_Enemy[i].pos.x = newXPos.x;
			g_Enemy[i].pos.y = newYPos.y;

			PushToTimeState(&g_Enemy[i].timeState, &g_Enemy[i]);

			// Humming enemies



			//// �ړ�����
			//if (g_Enemy[i].tblMax > 0)	// ���`��Ԃ����s����H
			//{	// ���`��Ԃ̏���
			//	int nowNo = (int)g_Enemy[i].time;			// �������ł���e�[�u���ԍ������o���Ă���
			//	int maxNo = g_Enemy[i].tblMax;				// �o�^�e�[�u�����𐔂��Ă���
			//	int nextNo = (nowNo + 1) % maxNo;			// �ړ���e�[�u���̔ԍ������߂Ă���
			//	INTERPOLATION_DATA* tbl = g_MoveTblAdr[g_Enemy[i].tblNo];	// �s���e�[�u���̃A�h���X���擾
			//	
			//	XMVECTOR nowPos = XMLoadFloat3(&tbl[nowNo].pos);	// XMVECTOR�֕ϊ�
			//	XMVECTOR nowRot = XMLoadFloat3(&tbl[nowNo].rot);	// XMVECTOR�֕ϊ�
			//	XMVECTOR nowScl = XMLoadFloat3(&tbl[nowNo].scl);	// XMVECTOR�֕ϊ�
			//	
			//	XMVECTOR Pos = XMLoadFloat3(&tbl[nextNo].pos) - nowPos;	// XYZ�ړ��ʂ��v�Z���Ă���
			//	XMVECTOR Rot = XMLoadFloat3(&tbl[nextNo].rot) - nowRot;	// XYZ��]�ʂ��v�Z���Ă���
			//	XMVECTOR Scl = XMLoadFloat3(&tbl[nextNo].scl) - nowScl;	// XYZ�g�嗦���v�Z���Ă���
			//	
			//	float nowTime = g_Enemy[i].time - nowNo;	// ���ԕ����ł��鏭�������o���Ă���
			//	
			//	Pos *= nowTime;								// ���݂̈ړ��ʂ��v�Z���Ă���
			//	Rot *= nowTime;								// ���݂̉�]�ʂ��v�Z���Ă���
			//	Scl *= nowTime;								// ���݂̊g�嗦���v�Z���Ă���

			//	// �v�Z���ċ��߂��ړ��ʂ����݂̈ړ��e�[�u��XYZ�ɑ����Ă��遁�\�����W�����߂Ă���
			//	XMStoreFloat3(&g_Enemy[i].pos, nowPos + Pos);

			//	// �v�Z���ċ��߂���]�ʂ����݂̈ړ��e�[�u���ɑ����Ă���
			//	XMStoreFloat3(&g_Enemy[i].rot, nowRot + Rot);

			//	// �v�Z���ċ��߂��g�嗦�����݂̈ړ��e�[�u���ɑ����Ă���
			//	XMStoreFloat3(&g_Enemy[i].scl, nowScl + Scl);
			//	g_Enemy[i].w = TEXTURE_WIDTH * g_Enemy[i].scl.x;
			//	g_Enemy[i].h = TEXTURE_HEIGHT * g_Enemy[i].scl.y;

			//	// frame���g�Ď��Ԍo�ߏ���������
			//	g_Enemy[i].time += 1.0f / tbl[nowNo].frame;	// ���Ԃ�i�߂Ă���
			//	if ((int)g_Enemy[i].time >= maxNo)			// �o�^�e�[�u���Ō�܂ňړ��������H
			//	{
			//		g_Enemy[i].time -= maxNo;				// �O�ԖڂɃ��Z�b�g�������������������p���ł���
			//	}

			//}

			// �ړ����I�������G�l�~�[�Ƃ̓����蔻��
			{
				PLAYER* player = GetPlayer();

				// �G�l�~�[�̐��������蔻����s��
				for (int j = 0; j < ENEMY_MAX; j++)
				{
					// �����Ă�G�l�~�[�Ɠ����蔻�������
					if (player[j].use == TRUE)
					{
						BOOL ans = CollisionBB(g_Enemy[i].pos, g_Enemy[i].w, g_Enemy[i].h,
							player[j].pos, player[j].w, player[j].h);
						// �������Ă���H
						if (ans == TRUE)
						{
							// �����������̏���
							//player[j].use = FALSE;	// �f�o�b�O�ňꎞ�I�ɖ��G�ɂ��Ă�����
						}
					}
				}
			}
		}
	}


	//// �G�l�~�[�S�Ń`�F�b�N
	//if (g_EnemyCount <= 0)
	//{
 //		SetFade(FADE_OUT, MODE_RESULT);
	//}

#ifdef _DEBUG	// �f�o�b�O����\������


#endif

}

//=============================================================================
// �`�揈��
//=============================================================================
void DrawEnemy(void)
{
	// ���_�o�b�t�@�ݒ�
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

	// �}�g���N�X�ݒ�
	SetWorldViewProjection2D();

	// �v���~�e�B�u�g�|���W�ݒ�
	GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// �}�e���A���ݒ�
	MATERIAL material;
	ZeroMemory(&material, sizeof(material));
	material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	SetMaterial(material);

	BG* bg = GetBG();

	for (int i = 0; i < ENEMY_MAX; i++)
	{
		if (g_Enemy[i].use == TRUE)			// ���̃G�l�~�[���g���Ă���H
		{									// Yes
			// �e�N�X�`���ݒ�
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_Enemy[i].texNo]);

			//�G�l�~�[�̈ʒu��e�N�X�`���[���W�𔽉f
			float px = g_Enemy[i].pos.x - bg->pos.x;	// �G�l�~�[�̕\���ʒuX
			float py = g_Enemy[i].pos.y - bg->pos.y;	// �G�l�~�[�̕\���ʒuY
			float pw = g_Enemy[i].w;		// �G�l�~�[�̕\����
			float ph = g_Enemy[i].h;		// �G�l�~�[�̕\������

			// �A�j���[�V�����p
			float tw = 1.0f / g_Enemy[i].animDivideX;	// �e�N�X�`���̕�
			float th = 1.0f / g_Enemy[i].animDivideY;	// �e�N�X�`���̍���
			float tx = (float)(g_Enemy[i].patternAnim % g_Enemy[i].animDivideX) * tw;	// �e�N�X�`���̍���X���W
			float ty = (float)(g_Enemy[i].patternAnim / g_Enemy[i].animDivideX) * th;	// �e�N�X�`���̍���Y���W

			//float tw = 1.0f;	// �e�N�X�`���̕�
			//float th = 1.0f;	// �e�N�X�`���̍���
			//float tx = 0.0f;	// �e�N�X�`���̍���X���W
			//float ty = 0.0f;	// �e�N�X�`���̍���Y���W

			// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
			SetSpriteColorRotation(g_VertexBuffer, px, py, pw, ph, tx, ty, tw, th,
				XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
				g_Enemy[i].rot.z);

			// �|���S���`��
			GetDeviceContext()->Draw(4, 0);
		}
	}


	// �Q�[�W�̃e�X�g
	{
		// ���~���̃Q�[�W�i�g�I�ȕ��j
		// �e�N�X�`���ݒ�
		//GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[1]);

		////�Q�[�W�̈ʒu��e�N�X�`���[���W�𔽉f
		//float px = 600.0f;		// �Q�[�W�̕\���ʒuX
		//float py =  10.0f;		// �Q�[�W�̕\���ʒuY
		//float pw = 300.0f;		// �Q�[�W�̕\����
		//float ph =  30.0f;		// �Q�[�W�̕\������

		//float tw = 1.0f;	// �e�N�X�`���̕�
		//float th = 1.0f;	// �e�N�X�`���̍���
		//float tx = 0.0f;	// �e�N�X�`���̍���X���W
		//float ty = 0.0f;	// �e�N�X�`���̍���Y���W

		//// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
		//SetSpriteLTColor(g_VertexBuffer,
		//	px, py, pw, ph,
		//	tx, ty, tw, th,
		//	XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f));

		//// �|���S���`��
		//GetDeviceContext()->Draw(4, 0);


		//// �G�l�~�[�̐��ɏ]���ăQ�[�W�̒�����\�����Ă݂�
		//// �e�N�X�`���ݒ�
		//GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[1]);

		////�Q�[�W�̈ʒu��e�N�X�`���[���W�𔽉f
		//pw = pw * ((float)g_EnemyCount / ENEMY_MAX);

		//// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
		//SetSpriteLTColor(g_VertexBuffer,
		//	px, py, pw, ph,
		//	tx, ty, tw, th,
		//	XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f));

		//// �|���S���`��
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
}

void PullFromTimeState(TIMESTATE* timeState, ENEMY* enemy)
{
	enemy->pos.x = timeState->x;
	enemy->pos.y = timeState->y;
	enemy->countAnim = timeState->countAnim;
	enemy->patternAnim = timeState->patternAnim;
	
}


//=============================================================================
// Enemy�\���̂̐擪�A�h���X���擾
//=============================================================================
ENEMY* GetEnemy(void)
{
	return &g_Enemy[0];
}


// �����Ă�G�l�~�[�̐�
int GetEnemyCount(void)
{
	return g_EnemyCount;
}

void AdjustEnemyHP(ENEMY* enemy, int ammount) 
{
	
	enemy->hp += ammount;

	if (enemy->hp < 0) {
		enemy->hp = 0;
		SetCharacterState(CHAR_ANIM_DIE, enemy, TRUE);
	}

}


