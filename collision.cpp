//=============================================================================
//
// 当たり判定処理 [collision.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "collision.h"


//*****************************************************************************
// マクロ定義
//*****************************************************************************


//*****************************************************************************
// 構造体定義
//*****************************************************************************


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************


//=============================================================================
// BBによる当たり判定処理
// 回転は考慮しない
// 戻り値：当たってたらtrue
//=============================================================================
BOOL CollisionBB(XMFLOAT3 objApos, float objAWidth, float objAHeight,
	XMFLOAT3 objBPos, float objBWidth, float objBHeight)
{
	BOOL ans = FALSE;	// 外れをセットしておく

	// 座標が中心点なので計算しやすく半分にしている
	objAWidth /= 2;
	objAHeight /= 2;
	objBWidth /= 2;
	objBHeight /= 2;

	// バウンディングボックス(BB)の処理
	if ((objApos.x + objAWidth > objBPos.x - objBWidth) &&
		(objApos.x - objAWidth < objBPos.x + objBWidth) &&
		(objApos.y + objAHeight > objBPos.y - objBHeight) &&
		(objApos.y - objAHeight < objBPos.y + objBHeight))
	{
		// 当たった時の処理
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
// BCによる当たり判定処理
// サイズは半径
// 戻り値：当たってたらTRUE
//=============================================================================
BOOL CollisionBC(XMFLOAT3 pos1, XMFLOAT3 pos2, float r1, float r2)
{
	BOOL ans = FALSE;						// 外れをセットしておく

	float len = (r1 + r2) * (r1 + r2);		// 半径を2乗した物
	XMVECTOR temp = XMLoadFloat3(&pos1) - XMLoadFloat3(&pos2);
	temp = XMVector3LengthSq(temp);			// 2点間の距離（2乗した物）
	float lenSq = 0.0f;
	XMStoreFloat(&lenSq, temp);

	// 半径を2乗した物より距離が短い？
	if (len > lenSq)
	{
		ans = TRUE;	// 当たっている
	}

	return ans;
}



