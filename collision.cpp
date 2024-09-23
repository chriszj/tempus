//=============================================================================
//
// �����蔻�菈�� [collision.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "collision.h"


//*****************************************************************************
// �}�N����`
//*****************************************************************************


//*****************************************************************************
// �\���̒�`
//*****************************************************************************


//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************


//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************


//=============================================================================
// BB�ɂ�铖���蔻�菈��
// ��]�͍l�����Ȃ�
// �߂�l�F�������Ă���true
//=============================================================================
BOOL CollisionBB(XMFLOAT3 objApos, float objAWidth, float objAHeight,
	XMFLOAT3 objBPos, float objBWidth, float objBHeight)
{
	BOOL ans = FALSE;	// �O����Z�b�g���Ă���

	// ���W�����S�_�Ȃ̂Ōv�Z���₷�������ɂ��Ă���
	objAWidth /= 2;
	objAHeight /= 2;
	objBWidth /= 2;
	objBHeight /= 2;

	// �o�E���f�B���O�{�b�N�X(BB)�̏���
	if ((objApos.x + objAWidth > objBPos.x - objBWidth) &&
		(objApos.x - objAWidth < objBPos.x + objBWidth) &&
		(objApos.y + objAHeight > objBPos.y - objBHeight) &&
		(objApos.y - objAHeight < objBPos.y + objBHeight))
	{
		// �����������̏���
		ans = TRUE;
	}

	return ans;
}

BOOL CollisionBB(XMFLOAT3 objAPos, COLLIDER2DBOX objAcollider, XMFLOAT3 objBPos, COLLIDER2DBOX objBCollider)
{

	objAPos.x += objAcollider.offsetX;
	objAPos.y += objAcollider.offsetY;

	objBPos.x += objBCollider.offsetX;
	objBPos.y += objBCollider.offsetY;

	return CollisionBB(objAPos,objAcollider.width, objAcollider.height, objBPos, objBCollider.width, objBCollider.height);

}

//=============================================================================
// BC�ɂ�铖���蔻�菈��
// �T�C�Y�͔��a
// �߂�l�F�������Ă���TRUE
//=============================================================================
BOOL CollisionBC(XMFLOAT3 pos1, XMFLOAT3 pos2, float r1, float r2)
{
	BOOL ans = FALSE;						// �O����Z�b�g���Ă���

	float len = (r1 + r2) * (r1 + r2);		// ���a��2�悵����
	XMVECTOR temp = XMLoadFloat3(&pos1) - XMLoadFloat3(&pos2);
	temp = XMVector3LengthSq(temp);			// 2�_�Ԃ̋����i2�悵�����j
	float lenSq = 0.0f;
	XMStoreFloat(&lenSq, temp);

	// ���a��2�悵������苗�����Z���H
	if (len > lenSq)
	{
		ans = TRUE;	// �������Ă���
	}

	return ans;
}



