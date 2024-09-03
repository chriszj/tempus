//=============================================================================
//
// �O���t�B�b�N���[�U�[�C���^�[�t�F�[�X���� [gui.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "score.h"
#include "sprite.h"
#include "gui.h"
#include "pugixml.hpp"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define TEXTURE_WIDTH				(16)	// �L�����T�C�Y
#define TEXTURE_HEIGHT				(32)	// 

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
void ParseFont(BMPFONT* font, char* file);

//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static ID3D11Buffer				*g_VertexBuffer = NULL;		// ���_���
static ID3D11ShaderResourceView* g_Texture[FONTTEXTURE_MAX] = { NULL };	// �e�N�X�`�����

static char g_FontFolder[] = "data/TEXTURE/FONT/";

static BMPFONT g_Font;
static BMPTEXT g_Texts[TEXTOBJECTS_MAX];

static bool						g_Use;						// true:�g���Ă���  false:���g�p
static float					g_w, g_h;					// ���ƍ���
static XMFLOAT3					g_Pos;						// �|���S���̍��W
static int						g_TexNo;					// �e�N�X�`���ԍ�

static int						g_Score;					// �X�R�A



//=============================================================================
// ����������
//=============================================================================
HRESULT InitGUI(void)
{
	ID3D11Device *pDevice = GetDevice();


	ParseFont(&g_Font, "data/TEXTURE/FONT/cjk-jp.fnt");

	//�e�N�X�`������
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


	// ���_�o�b�t�@����
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX_3D) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	GetDevice()->CreateBuffer(&bd, NULL, &g_VertexBuffer);


	// �v���C���[�̏�����
	g_Use   = true;
	g_w     = TEXTURE_WIDTH;
	g_h     = TEXTURE_HEIGHT;
	g_Pos   = { 500.0f, 20.0f, 0.0f };
	g_TexNo = 0;

	g_Score = 0;	// �X�R�A�̏�����

	return S_OK;
}

//=============================================================================
// �I������
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

	g_Font.Reset();

}

//=============================================================================
// �X�V����
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

#ifdef _DEBUG	// �f�o�b�O����\������
	//char *str = GetDebugStr();
	//sprintf(&str[strlen(str)], " PX:%.2f PY:%.2f", g_Pos.x, g_Pos.y);
	
#endif

}

//=============================================================================
// �`�揈��
//=============================================================================
void DrawGUI(void)
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

			// �e�N�X�`���ݒ�
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[fontChar->page]);

			// �X�R�A�̈ʒu��e�N�X�`���[���W�𔽉f
			float px = g_Texts[t].x + textCharIndex;	// �X�R�A�̕\���ʒuX
			float py = g_Texts[t].y + (textLineIndex * g_Font.lineHeight * g_Texts[t].scale);			// �X�R�A�̕\���ʒuY

			px -= fontChar->xoffset/2 * g_Texts[t].scale;
			py += fontChar->yoffset/2 * g_Texts[t].scale;

			float pw = fontChar->width * g_Texts[t].scale;				// �X�R�A�̕\����
			float ph = fontChar->height * g_Texts[t].scale;				// �X�R�A�̕\������

			float tx = fontChar->textureU;		// �e�N�X�`���̕�
			float ty = fontChar->textureV;		// �e�N�X�`���̍���
			float tw = fontChar->textureWidth;			// �e�N�X�`���̍���X���W
			float th = fontChar->textureHeigt;			// �e�N�X�`���̍���Y���W

			textCharIndex += pw + (fontChar->xoffset * g_Texts[t].scale);

			// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
			SetSpriteColor(g_VertexBuffer, px, py, pw, ph, tx, ty, tw, th,
				XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

			// �|���S���`��
			GetDeviceContext()->Draw(4, 0);

		}
		
	}

}


void ParseFont(BMPFONT* font, char* file) {

	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(file);

	// �}�b�v�`�b�v�̏���
	if (result)
	{

		pugi::xml_node fontNode = doc.child("font");

		font->lineHeight = fontNode.child("common").attribute("lineHeight").as_int();
		font->base = fontNode.child("common").attribute("base").as_int();
		font->scaleW = fontNode.child("common").attribute("scaleW").as_int();
		font->scaleH = fontNode.child("common").attribute("scaleH").as_int();

		int fontPagesCount = 0;

		// �t�H���g�̃e�L�X�`���[�̏���
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
}

//=============================================================================
// �X�R�A�����Z����
// ����:add :�ǉ�����_���B�}�C�i�X���\
//=============================================================================
BMPTEXT* GetUnusedText()
{

	for (int i = 0; i < TEXTOBJECTS_MAX; i++)
	{

		if (!g_Texts[i].use)
		{
			g_Texts[i].use = true;
			return &g_Texts[i];
		}

	}

	return NULL;
}

void SetText(BMPTEXT* bmpText, wchar_t* text) { 

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
		
		g_Texts[i].use = false;
	
	}
}

