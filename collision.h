//=============================================================================
//
// 当たり判定処理 [collision.h]
// Author : 
//
//=============================================================================
#pragma once


//*****************************************************************************
// マクロ定義
//*****************************************************************************


//*****************************************************************************
// 構造体定義
//*****************************************************************************
struct COLLIDER2DBOX {

	float offsetX = 0;
	float offsetY = 0;
	float width = 0; 
	float height = 0;

	COLLIDER2DBOX(){};

	COLLIDER2DBOX( float parOffsetX, float parOffsetY, float parWidth, float parHeight) {
		
		offsetX = parOffsetX;
		offsetY = parOffsetY;
		width = parWidth;
		height = parHeight;

	}

};

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
BOOL CollisionBB(XMFLOAT3 mpos, float mw, float mh, XMFLOAT3 ypos, float yw, float yh);
BOOL CollisionBB(XMFLOAT3 mpos, COLLIDER2DBOX mcollider, XMFLOAT3 ypos, COLLIDER2DBOX ycollider);
BOOL CollisionBC(XMFLOAT3 pos1, XMFLOAT3 pos2, float r1, float r2);

