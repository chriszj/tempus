//=============================================================================
//
// グラフィックユーザーインターフェース処理 [gui.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "score.h"
#include "sprite.h"
#include "gui.h"
#include "pugixml.hpp"
#include "player.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TEXTURE_WIDTH				(16)	// キャラサイズ
#define TEXTURE_HEIGHT				(32)	// 
#define TEXTURE_MAX                 (3)     

#define TEXTURE_HUD_DIVIDEX         (1)
#define TEXTURE_HUD_DIVIDEY         (1)
#define TEXTURE_HUD_WIDTH           (128)
#define TEXTURE_HUD_HEIGHT          (32)

#define GUIIMAGES_MAX               (6)

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
void ParseFont(BMPFONT* font, char* file);

//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer				*g_VertexBuffer = NULL;		// 頂点情報
static ID3D11ShaderResourceView* g_Texture[FONTTEXTURE_MAX] = { NULL };	// テクスチャ情報
static ID3D11ShaderResourceView* g_GUITexture[TEXTURE_MAX] = { NULL };	// テクスチャ情報

static char g_FontFolder[] = "data/TEXTURE/FONT/";

static char* g_TexturName[TEXTURE_MAX] = {
	"data/TEXTURE/common/icon-hud.png",
	"data/TEXTURE/common/time-effect-bg.png",
	"data/TEXTURE/common/time-effect.png",
};

static BMPFONT g_Font;
static BMPTEXT g_Texts[TEXTOBJECTS_MAX];

static BOOL						g_Use;						// true:使っている  false:未使用
static float					g_w, g_h;					// 幅と高さ
static XMFLOAT3					g_Pos;						// ポリゴンの座標
static int						g_TexNo;					// テクスチャ番号



static int						g_Score;					// スコア

static BOOL                     g_TimeMachineGUIActive;
static BOOL						g_PlayerGUIActive;

static GUIIMAGE	                g_GUIImages[GUIIMAGES_MAX];

static PLAYER*                  g_registeredPlayer;

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitGUI(void)
{
	ID3D11Device *pDevice = GetDevice();

	ParseFont(&g_Font, "data/TEXTURE/FONT/cjk-jp.fnt");

	//テクスチャ生成
	for (int i = 0; i < FONTTEXTURE_MAX; i++)
	{
		g_Texture[i] = NULL;

		if (g_Font.pages[i].id < 0)
			continue;

		D3DX11CreateShaderResourceViewFromFile(GetDevice(),
			g_Font.pages[i].file,
			NULL,
			NULL,
			&g_Texture[i],
			NULL);
	}

	for (int i = 0; i < TEXTURE_MAX; i++)
	{
		g_GUITexture[i] = NULL;


		D3DX11CreateShaderResourceViewFromFile(GetDevice(),
			g_TexturName[i],
			NULL,
			NULL,
			&g_GUITexture[i],
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

	g_registeredPlayer = NULL;

	// プレイヤーの初期化
	g_Use   = TRUE;
	g_w     = TEXTURE_WIDTH;
	g_h     = TEXTURE_HEIGHT;
	g_Pos   = { 500.0f, 20.0f, 0.0f };
	g_TexNo = 0;


	for (int i = 0; i < GUIIMAGES_MAX; i++)
	{
		g_GUIImages[i].use = FALSE;

		g_GUIImages[i].iconID = i;

		switch (g_GUIImages[i].iconID) {

		case GUI_ICON_HP:
			g_GUIImages[i].texNo = 0;
			g_GUIImages[i].patternNo = 0;
			g_GUIImages[i].pos = { 25, 30 };
			g_GUIImages[i].width = 32;
			g_GUIImages[i].height = 32;
			g_GUIImages[i].texWidth = 128;
			g_GUIImages[i].texHeight = 32;
			g_GUIImages[i].divideX = 4;
			g_GUIImages[i].divideY = 1;
			break;
		case GUI_ICON_TIME:
			g_GUIImages[i].texNo = 0;
			g_GUIImages[i].patternNo = 1;
			g_GUIImages[i].pos = { SCREEN_WIDTH - 85, 30 };
			g_GUIImages[i].width = 32;
			g_GUIImages[i].height = 32;
			g_GUIImages[i].texWidth = 128;
			g_GUIImages[i].texHeight = 32;
			g_GUIImages[i].divideX = 4;
			g_GUIImages[i].divideY = 1;
			break;
		case GUI_ICON_KEY:
			g_GUIImages[i].texNo = 0;
			g_GUIImages[i].patternNo = 2;
			g_GUIImages[i].pos = { SCREEN_WIDTH - 75, SCREEN_HEIGHT - 90 };
			g_GUIImages[i].width = 32;
			g_GUIImages[i].height = 32;
			g_GUIImages[i].texWidth = 128;
			g_GUIImages[i].texHeight = 32;
			g_GUIImages[i].divideX = 4;
			g_GUIImages[i].divideY = 1;
			break;
		case GUI_ICON_MKEY:
			g_GUIImages[i].texNo = 0;
			g_GUIImages[i].patternNo = 3;
			g_GUIImages[i].pos = { SCREEN_WIDTH - 75, SCREEN_HEIGHT - 40 };
			g_GUIImages[i].width = 32;
			g_GUIImages[i].height = 32;
			g_GUIImages[i].texWidth = 128;
			g_GUIImages[i].texHeight = 32;
			g_GUIImages[i].divideX = 4;
			g_GUIImages[i].divideY = 1;
			break;
		case GUI_ICON_TM_BG:
			g_GUIImages[i].texNo = 1;
			g_GUIImages[i].patternNo = 0;
			g_GUIImages[i].pos = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
			g_GUIImages[i].width = SCREEN_WIDTH;
			g_GUIImages[i].height = SCREEN_HEIGHT;
			g_GUIImages[i].texWidth = 1920;
			g_GUIImages[i].texHeight = 1200;
			g_GUIImages[i].divideX = 1;
			g_GUIImages[i].divideY = 1;
			break;
		case GUI_ICON_TM_SIMBOL:
			g_GUIImages[i].texNo = 2;
			g_GUIImages[i].patternNo = 0;
			g_GUIImages[i].pos = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
			g_GUIImages[i].width = 1920;
			g_GUIImages[i].height = 1200;
			g_GUIImages[i].texWidth = 1920;
			g_GUIImages[i].texHeight = 1200;
			g_GUIImages[i].divideX = 1;
			g_GUIImages[i].divideY = 1;
			break;

		}

	}

	g_Score = 0;	// スコアの初期化

	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitGUI(void)
{
	if (g_VertexBuffer)
	{
		g_VertexBuffer->Release();
		g_VertexBuffer = NULL;
	}

	for (int i = 0; i < FONTTEXTURE_MAX; i++)
	{
		if (g_Texture[i])
		{
			g_Texture[i]->Release();
			g_Texture[i] = NULL;
		}
	}

	for (int i = 0; i < TEXTURE_MAX; i++)
	{
		if (g_GUITexture[i])
		{
			g_GUITexture[i]->Release();
			g_GUITexture[i] = NULL;
		}
	}

	g_Font.Reset();

}

//=============================================================================
// 更新処理
//=============================================================================
void UpdateGUI(void)
{

	/*for (int t = 0; t < TEXTOBJECTS_MAX; t++)
	{

		if (!g_Texts[t].use)
			continue;

		for (int c = 0; c < TEXTCHAR_MAX; c++) {

			BMPCHAR* fontChar = g_Texts[t].textChars[c];

			if (fontChar->id < 0)
				continue;



			fontChar =   
		}
	}*/

#ifdef _DEBUG	// デバッグ情報を表示する
	//char *str = GetDebugStr();
	//sprintf(&str[strlen(str)], " PX:%.2f PY:%.2f", g_Pos.x, g_Pos.y);
	
#endif

}

//=============================================================================
// 描画処理
//=============================================================================
void DrawGUI(void)
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

	

	for (int t = 0; t < TEXTOBJECTS_MAX; t++) 
	{
		
		if (!g_Texts[t].use)
			continue;

		int textLineIndex = 0;

		float textCharIndex = 0;
	
		for (int c = 0; c < TEXTCHAR_MAX; c++) {

			BMPCHAR* fontChar = g_Texts[t].textChars[c];

			if (fontChar == NULL)
				continue;

			if (fontChar->id == 10)
			{
				textLineIndex++;
				textCharIndex = 0;
				continue;
			}

			// テクスチャ設定
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[fontChar->page]);

			// スコアの位置やテクスチャー座標を反映
			float px = g_Texts[t].x + textCharIndex;	// スコアの表示位置X
			float py = g_Texts[t].y + (textLineIndex * g_Font.lineHeight * g_Texts[t].scale);			// スコアの表示位置Y

			px -= fontChar->xoffset/2 * g_Texts[t].scale;
			py += fontChar->yoffset/2 * g_Texts[t].scale;

			float pw = fontChar->width * g_Texts[t].scale;				// スコアの表示幅
			float ph = fontChar->height * g_Texts[t].scale;				// スコアの表示高さ

			float tx = fontChar->textureU;		// テクスチャの幅
			float ty = fontChar->textureV;		// テクスチャの高さ
			float tw = fontChar->textureWidth;			// テクスチャの左上X座標
			float th = fontChar->textureHeigt;			// テクスチャの左上Y座標

			textCharIndex += pw + (fontChar->xoffset * g_Texts[t].scale);

			// １枚のポリゴンの頂点とテクスチャ座標を設定
			SetSpriteColor(g_VertexBuffer, px, py, pw, ph, tx, ty, tw, th,
				XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

			// ポリゴン描画
			GetDeviceContext()->Draw(4, 0);

		}
		
	}

	for (int gi = 0; gi < GUIIMAGES_MAX; gi ++) {
	
		if (!g_GUIImages[gi].use)
			continue;

		// テクスチャ設定
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_GUITexture[g_GUIImages[gi].texNo]);

		//SetBlendState(BLEND_MODE_ALPHABLEND);

		// スコアの位置やテクスチャー座標を反映
		float px = g_GUIImages[gi].pos.x;
		float py = g_GUIImages[gi].pos.y;		// スコアの表示位置Y


		float pw = g_GUIImages[gi].width;				// スコアの表示幅
		float ph = g_GUIImages[gi].height;				// スコアの表示高さ

		float tw = 1.0f / g_GUIImages[gi].divideX;// テクスチャの幅
		float th = 1.0f / g_GUIImages[gi].divideY;	// テクスチャの高さ
		float tx = (float)(g_GUIImages[gi].patternNo % g_GUIImages[gi].divideX) * tw;	// テクスチャの左上X座標
		float ty = (float)(g_GUIImages[gi].patternNo / g_GUIImages[gi].divideX) * th;	// テクスチャの左上Y座標

		float alpha = 1.0f;

		float rotation = 0.0f;

		if (g_GUIImages[gi].iconID == GUI_ICON_TM_BG)
			alpha = 0.2f;

		if (g_GUIImages[gi].iconID == GUI_ICON_TM_SIMBOL) {
			
			rotation = GetTimeMachineElapsedTime_ms();
			alpha = 0.3f;
		}

		// １枚のポリゴンの頂点とテクスチャ座標を設定
		SetSpriteColorRotation(g_VertexBuffer, px, py, pw, ph, tx, ty, tw, th,
			XMFLOAT4(1.0f, 1.0f, 1.0f, alpha), rotation);

		// ポリゴン描画
		GetDeviceContext()->Draw(4, 0);

	}

}


void ParseFont(BMPFONT* font, char* file) {

	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(file);

	// マップチップの準備
	if (result)
	{

		pugi::xml_node fontNode = doc.child("font");

		font->lineHeight = fontNode.child("common").attribute("lineHeight").as_int();
		font->base = fontNode.child("common").attribute("base").as_int();
		font->scaleW = fontNode.child("common").attribute("scaleW").as_int();
		font->scaleH = fontNode.child("common").attribute("scaleH").as_int();

		int fontPagesCount = 0;

		// フォントのテキスチャーの準備
		for (pugi::xml_node xmlFontPage : fontNode.child("pages").children("page"))
		{

			BMPPAGE* fontPage = &font->pages[fontPagesCount];

			fontPage->id = xmlFontPage.attribute("id").as_int();

			char path[128] = {};
			const char* source = xmlFontPage.attribute("file").value();

			memcpy(path, g_FontFolder, strlen(g_FontFolder));
			memcpy(path + strlen(path), source, strlen(source));
			memcpy(fontPage->file, path, strlen(path));

			fontPagesCount++;
		}

		int fontCharsCount = 0;

		for (pugi::xml_node xmlFontPage : fontNode.child("chars").children("char"))
		{
			BMPCHAR* fontChar = &font->chars[fontCharsCount];

			fontChar->id = xmlFontPage.attribute("id").as_int();
			// <char id="32" x="303" y="2046" width="3" height="1" xoffset="-1" yoffset="31" xadvance="6" page="0" chnl="15" />
			fontChar->x = xmlFontPage.attribute("x").as_int();
			fontChar->y = xmlFontPage.attribute("y").as_int();
			fontChar->width = xmlFontPage.attribute("width").as_int();
			fontChar->height = xmlFontPage.attribute("height").as_int();
			fontChar->xoffset = xmlFontPage.attribute("xoffset").as_int();
			fontChar->yoffset = xmlFontPage.attribute("yoffset").as_int();
			fontChar->xadvance = xmlFontPage.attribute("xadvance").as_int();
			fontChar->page = xmlFontPage.attribute("page").as_int();
			fontChar->chnl = xmlFontPage.attribute("chnl").as_int();

			fontChar->textureU = (float)fontChar->x / (float)font->scaleW;
			fontChar->textureV = (float)fontChar->y / (float)font->scaleH;
			fontChar->textureWidth = (float)fontChar->width / (float)font->scaleW;
			fontChar->textureHeigt = (float)fontChar->height / (float)font->scaleH;

			fontCharsCount++;

		}

		BMPCHAR* lineBreakChar = &font->chars[fontCharsCount];
		lineBreakChar->id = 10;
		lineBreakChar->x = 0;
		lineBreakChar->y = 0;
		lineBreakChar->width = 0;
		lineBreakChar->height = 0;
		lineBreakChar->xoffset = 0;
		lineBreakChar->yoffset = 0;
		lineBreakChar->xadvance = 0;
		lineBreakChar->page = 0;
		lineBreakChar->chnl = 0;

		lineBreakChar->textureU = 0;
		lineBreakChar->textureV = 0;
		lineBreakChar->textureWidth = 0;
		lineBreakChar->textureHeigt = 0;

	}

	if (g_registeredPlayer != NULL)
	{



	}

}

//=============================================================================
// スコアを加算する
// 引数:add :追加する点数。マイナスも可能
//=============================================================================
BMPTEXT* GetUnusedText()
{

	for (int i = 0; i < TEXTOBJECTS_MAX; i++)
	{

		if (!g_Texts[i].use)
		{
			g_Texts[i].use = TRUE;
			return &g_Texts[i];
		}

	}

	return NULL;
}

void SetText(BMPTEXT* bmpText, wchar_t* text) { 

	memset(bmpText->textChars, 0, TEXTCHAR_MAX);

	int charLength = wcslen(text);
	memcpy(bmpText->richText, text, charLength*sizeof(wchar_t));

	for (int i = 0; i < charLength; i++) {

		uint32_t unicodeId = text[i];

		for (int c = 0; c < FONTCHAR_MAX; c++) {

			if (g_Font.chars[c].id == unicodeId) {
				bmpText->textChars[i] = &g_Font.chars[c];
				break;
			}
		}

	}

	

	int ok = 1;

}

void ClearGUI(void) 
{
	for (int i = 0; i < TEXTOBJECTS_MAX; i++) 
	{
		
		g_Texts[i].use = FALSE;
	
	}
}

void RegisterPlayer(PLAYER* player) {
	g_registeredPlayer = player;
}
void UnregisterPlayer(PLAYER* player) {
	g_registeredPlayer = NULL;
}

void SetPlayerGUI(BOOL enable) {
	g_GUIImages[GUI_ICON_HP].use = enable;
	g_GUIImages[GUI_ICON_TIME].use = enable;
	g_GUIImages[GUI_ICON_KEY].use = enable;
	g_GUIImages[GUI_ICON_MKEY].use = enable;
}

void SetTMGUI(BOOL enable) {

	g_GUIImages[GUI_ICON_TM_BG].use = enable;
	g_GUIImages[GUI_ICON_TM_SIMBOL].use = enable;

}

