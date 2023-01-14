/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_CLIENT_COMPONENTS_MAPIMAGES_H
#define GAME_CLIENT_COMPONENTS_MAPIMAGES_H

#include <engine/graphics.h>

#include <game/client/component.h>

enum EMapImageEntityLayerType
{
	MAP_IMAGE_ENTITY_LAYER_TYPE_ALL_EXCEPT_SWITCH = 0,
	MAP_IMAGE_ENTITY_LAYER_TYPE_SWITCH,

	MAP_IMAGE_ENTITY_LAYER_TYPE_COUNT,
};

enum EMapImageModType
{
	MAP_IMAGE_MOD_TYPE_DDNET = 0,
	MAP_IMAGE_MOD_TYPE_DDRACE,
	MAP_IMAGE_MOD_TYPE_RACE,
	MAP_IMAGE_MOD_TYPE_BLOCKWORLDS,
	MAP_IMAGE_MOD_TYPE_FNG,
	MAP_IMAGE_MOD_TYPE_VANILLA,
	MAP_IMAGE_MOD_TYPE_FDDRACE,

	MAP_IMAGE_MOD_TYPE_COUNT,
};

extern const char *const gs_apModEntitiesNames[];

class CMapImages : public CComponent
{
	friend class CBackground;
	friend class CMenuBackground;

	IGraphics::CTextureHandle m_aTextures[64];
	int m_aTextureUsedByTileOrQuadLayerFlag[64]; // 0: nothing, 1(as flag): tile layer, 2(as flag): quad layer
	int m_Count;

	char m_aEntitiesPath[IO_MAX_PATH_LENGTH];

	bool HasFrontLayer(EMapImageModType ModType);
	bool HasSpeedupLayer(EMapImageModType ModType);
	bool HasSwitchLayer(EMapImageModType ModType);
	bool HasTeleLayer(EMapImageModType ModType);
	bool HasTuneLayer(EMapImageModType ModType);

public:
	CMapImages();
	CMapImages(int TextureSize);
	virtual int Sizeof() const override { return sizeof(*this); }

	IGraphics::CTextureHandle Get(int Index) const { return m_aTextures[Index]; }
	int Num() const { return m_Count; }

	void OnMapLoadImpl(class CLayers *pLayers, class IMap *pMap);
	virtual void OnMapLoad() override;
	virtual void OnInit() override;
	void LoadBackground(class CLayers *pLayers, class IMap *pMap);

	// DDRace
	IGraphics::CTextureHandle GetEntities(EMapImageEntityLayerType EntityLayerType);
	IGraphics::CTextureHandle GetSpeedupArrow();

	IGraphics::CTextureHandle GetOverlayBottom();
	IGraphics::CTextureHandle GetOverlayTop();
	IGraphics::CTextureHandle GetOverlayCenter();

	void SetTextureScale(int Scale);
	int GetTextureScale();

	void ChangeEntitiesPath(const char *pPath);

private:
	bool m_aEntitiesIsLoaded[MAP_IMAGE_MOD_TYPE_COUNT * 2];
	bool m_SpeedupArrowIsLoaded;
	IGraphics::CTextureHandle m_aaEntitiesTextures[MAP_IMAGE_MOD_TYPE_COUNT * 2][MAP_IMAGE_ENTITY_LAYER_TYPE_COUNT];
	IGraphics::CTextureHandle m_SpeedupArrowTexture;
	IGraphics::CTextureHandle m_OverlayBottomTexture;
	IGraphics::CTextureHandle m_OverlayTopTexture;
	IGraphics::CTextureHandle m_OverlayCenterTexture;
	IGraphics::CTextureHandle m_TransparentTexture;
	int m_TextureScale;

	void InitOverlayTextures();
	IGraphics::CTextureHandle UploadEntityLayerText(int TextureSize, int MaxWidth, int YOffset);
	void UpdateEntityLayerText(void *pTexBuffer, int ImageColorChannelCount, int TexWidth, int TexHeight, int TextureSize, int MaxWidth, int YOffset, int NumbersPower, int MaxNumber = -1);
};

#endif
