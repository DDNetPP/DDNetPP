/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/graphics.h>
#include <engine/map.h>
#include <engine/storage.h>
#include <engine/textrender.h>
#include <game/generated/client_data.h>
#include <game/mapitems.h>

#include <game/layers.h>

#include "mapimages.h"

#include <game/client/gameclient.h>

const char *const gs_apModEntitiesNames[] = {
	"ddnet",
	"ddrace",
	"race",
	"blockworlds",
	"fng",
	"vanilla",
	"f-ddrace",
};

CMapImages::CMapImages() :
	CMapImages(100)
{
}

CMapImages::CMapImages(int TextureSize)
{
	m_Count = 0;
	m_TextureScale = TextureSize;
	mem_zero(m_aEntitiesIsLoaded, sizeof(m_aEntitiesIsLoaded));
	m_SpeedupArrowIsLoaded = false;

	mem_zero(m_aTextureUsedByTileOrQuadLayerFlag, sizeof(m_aTextureUsedByTileOrQuadLayerFlag));

	str_copy(m_aEntitiesPath, "editor/entities_clear");

	static_assert(std::size(gs_apModEntitiesNames) == MAP_IMAGE_MOD_TYPE_COUNT, "Mod name string count is not equal to mod type count");
}

void CMapImages::OnInit()
{
	InitOverlayTextures();

	if(str_comp(g_Config.m_ClAssetsEntites, "default") == 0)
		str_copy(m_aEntitiesPath, "editor/entities_clear");
	else
	{
		str_format(m_aEntitiesPath, sizeof(m_aEntitiesPath), "assets/entities/%s", g_Config.m_ClAssetsEntites);
	}
}

void CMapImages::OnMapLoadImpl(class CLayers *pLayers, IMap *pMap)
{
	// unload all textures
	for(int i = 0; i < m_Count; i++)
	{
		Graphics()->UnloadTexture(&(m_aTextures[i]));
		m_aTextureUsedByTileOrQuadLayerFlag[i] = 0;
	}
	m_Count = 0;

	int Start;
	pMap->GetType(MAPITEMTYPE_IMAGE, &Start, &m_Count);

	m_Count = clamp(m_Count, 0, 64);

	for(int g = 0; g < pLayers->NumGroups(); g++)
	{
		CMapItemGroup *pGroup = pLayers->GetGroup(g);
		if(!pGroup)
		{
			continue;
		}

		for(int l = 0; l < pGroup->m_NumLayers; l++)
		{
			CMapItemLayer *pLayer = pLayers->GetLayer(pGroup->m_StartLayer + l);
			if(pLayer->m_Type == LAYERTYPE_TILES)
			{
				CMapItemLayerTilemap *pTLayer = (CMapItemLayerTilemap *)pLayer;
				if(pTLayer->m_Image != -1 && pTLayer->m_Image < (int)(std::size(m_aTextures)))
				{
					m_aTextureUsedByTileOrQuadLayerFlag[pTLayer->m_Image] |= 1;
				}
			}
			else if(pLayer->m_Type == LAYERTYPE_QUADS)
			{
				CMapItemLayerQuads *pQLayer = (CMapItemLayerQuads *)pLayer;
				if(pQLayer->m_Image != -1 && pQLayer->m_Image < (int)(std::size(m_aTextures)))
				{
					m_aTextureUsedByTileOrQuadLayerFlag[pQLayer->m_Image] |= 2;
				}
			}
		}
	}

	int TextureLoadFlag = Graphics()->HasTextureArrays() ? IGraphics::TEXLOAD_TO_2D_ARRAY_TEXTURE : IGraphics::TEXLOAD_TO_3D_TEXTURE;

	// load new textures
	for(int i = 0; i < m_Count; i++)
	{
		int LoadFlag = (((m_aTextureUsedByTileOrQuadLayerFlag[i] & 1) != 0) ? TextureLoadFlag : 0) | (((m_aTextureUsedByTileOrQuadLayerFlag[i] & 2) != 0) ? 0 : (Graphics()->IsTileBufferingEnabled() ? IGraphics::TEXLOAD_NO_2D_TEXTURE : 0));
		CMapItemImage *pImg = (CMapItemImage *)pMap->GetItem(Start + i, 0, 0);
		if(pImg->m_External)
		{
			char aPath[IO_MAX_PATH_LENGTH];
			char *pName = (char *)pMap->GetData(pImg->m_ImageName);
			str_format(aPath, sizeof(aPath), "mapres/%s.png", pName);
			m_aTextures[i] = Graphics()->LoadTexture(aPath, IStorage::TYPE_ALL, CImageInfo::FORMAT_AUTO, LoadFlag);
		}
		else
		{
			void *pData = pMap->GetData(pImg->m_ImageData);
			char *pName = (char *)pMap->GetData(pImg->m_ImageName);
			char aTexName[128];
			str_format(aTexName, sizeof(aTexName), "%s %s", "embedded:", pName);
			m_aTextures[i] = Graphics()->LoadTextureRaw(pImg->m_Width, pImg->m_Height, CImageInfo::FORMAT_RGBA, pData, CImageInfo::FORMAT_RGBA, LoadFlag, aTexName);
			pMap->UnloadData(pImg->m_ImageData);
		}
	}
}

void CMapImages::OnMapLoad()
{
	IMap *pMap = Kernel()->RequestInterface<IMap>();
	CLayers *pLayers = m_pClient->Layers();
	OnMapLoadImpl(pLayers, pMap);
}

void CMapImages::LoadBackground(class CLayers *pLayers, class IMap *pMap)
{
	OnMapLoadImpl(pLayers, pMap);
}

bool CMapImages::HasFrontLayer(EMapImageModType ModType)
{
	return ModType == MAP_IMAGE_MOD_TYPE_DDNET || ModType == MAP_IMAGE_MOD_TYPE_DDRACE;
}

bool CMapImages::HasSpeedupLayer(EMapImageModType ModType)
{
	return ModType == MAP_IMAGE_MOD_TYPE_DDNET || ModType == MAP_IMAGE_MOD_TYPE_DDRACE;
}

bool CMapImages::HasSwitchLayer(EMapImageModType ModType)
{
	return ModType == MAP_IMAGE_MOD_TYPE_DDNET || ModType == MAP_IMAGE_MOD_TYPE_DDRACE;
}

bool CMapImages::HasTeleLayer(EMapImageModType ModType)
{
	return ModType == MAP_IMAGE_MOD_TYPE_DDNET || ModType == MAP_IMAGE_MOD_TYPE_DDRACE;
}

bool CMapImages::HasTuneLayer(EMapImageModType ModType)
{
	return ModType == MAP_IMAGE_MOD_TYPE_DDNET || ModType == MAP_IMAGE_MOD_TYPE_DDRACE;
}

IGraphics::CTextureHandle CMapImages::GetEntities(EMapImageEntityLayerType EntityLayerType)
{
	EMapImageModType EntitiesModType = MAP_IMAGE_MOD_TYPE_DDNET;
	bool EntitiesAreMasked = !GameClient()->m_GameInfo.m_DontMaskEntities;

	if(GameClient()->m_GameInfo.m_EntitiesFDDrace)
		EntitiesModType = MAP_IMAGE_MOD_TYPE_FDDRACE;
	else if(GameClient()->m_GameInfo.m_EntitiesDDNet)
		EntitiesModType = MAP_IMAGE_MOD_TYPE_DDNET;
	else if(GameClient()->m_GameInfo.m_EntitiesDDRace)
		EntitiesModType = MAP_IMAGE_MOD_TYPE_DDRACE;
	else if(GameClient()->m_GameInfo.m_EntitiesRace)
		EntitiesModType = MAP_IMAGE_MOD_TYPE_RACE;
	else if(GameClient()->m_GameInfo.m_EntitiesBW)
		EntitiesModType = MAP_IMAGE_MOD_TYPE_BLOCKWORLDS;
	else if(GameClient()->m_GameInfo.m_EntitiesFNG)
		EntitiesModType = MAP_IMAGE_MOD_TYPE_FNG;
	else if(GameClient()->m_GameInfo.m_EntitiesVanilla)
		EntitiesModType = MAP_IMAGE_MOD_TYPE_VANILLA;

	if(!m_aEntitiesIsLoaded[(EntitiesModType * 2) + (int)EntitiesAreMasked])
	{
		m_aEntitiesIsLoaded[(EntitiesModType * 2) + (int)EntitiesAreMasked] = true;

		// any mod that does not mask, will get all layers unmasked
		bool WasUnknown = !EntitiesAreMasked;

		char aPath[64];
		str_format(aPath, sizeof(aPath), "%s/%s.png", m_aEntitiesPath, gs_apModEntitiesNames[EntitiesModType]);

		bool GameTypeHasFrontLayer = HasFrontLayer(EntitiesModType) || WasUnknown;
		bool GameTypeHasSpeedupLayer = HasSpeedupLayer(EntitiesModType) || WasUnknown;
		bool GameTypeHasSwitchLayer = HasSwitchLayer(EntitiesModType) || WasUnknown;
		bool GameTypeHasTeleLayer = HasTeleLayer(EntitiesModType) || WasUnknown;
		bool GameTypeHasTuneLayer = HasTuneLayer(EntitiesModType) || WasUnknown;

		int TextureLoadFlag = 0;
		if(Graphics()->IsTileBufferingEnabled())
			TextureLoadFlag = (Graphics()->HasTextureArrays() ? IGraphics::TEXLOAD_TO_2D_ARRAY_TEXTURE : IGraphics::TEXLOAD_TO_3D_TEXTURE) | IGraphics::TEXLOAD_NO_2D_TEXTURE;

		CImageInfo ImgInfo;
		bool ImagePNGLoaded = false;
		if(Graphics()->LoadPNG(&ImgInfo, aPath, IStorage::TYPE_ALL))
			ImagePNGLoaded = true;
		else
		{
			bool TryDefault = true;
			// try as single ddnet replacement
			if(EntitiesModType == MAP_IMAGE_MOD_TYPE_DDNET)
			{
				str_format(aPath, sizeof(aPath), "%s.png", m_aEntitiesPath);
				if(Graphics()->LoadPNG(&ImgInfo, aPath, IStorage::TYPE_ALL))
				{
					ImagePNGLoaded = true;
					TryDefault = false;
				}
			}

			if(!ImagePNGLoaded && TryDefault)
			{
				// try default
				str_format(aPath, sizeof(aPath), "editor/entities_clear/%s.png", gs_apModEntitiesNames[EntitiesModType]);
				if(Graphics()->LoadPNG(&ImgInfo, aPath, IStorage::TYPE_ALL))
				{
					ImagePNGLoaded = true;
				}
			}
		}

		if(ImagePNGLoaded && ImgInfo.m_Width > 0 && ImgInfo.m_Height > 0)
		{
			int ColorChannelCount = 4;
			if(ImgInfo.m_Format == CImageInfo::FORMAT_SINGLE_COMPONENT)
				ColorChannelCount = 1;
			else if(ImgInfo.m_Format == CImageInfo::FORMAT_RGB)
				ColorChannelCount = 3;
			else if(ImgInfo.m_Format == CImageInfo::FORMAT_RGBA)
				ColorChannelCount = 4;

			int BuildImageSize = ColorChannelCount * ImgInfo.m_Width * ImgInfo.m_Height;

			uint8_t *pTmpImgData = (uint8_t *)ImgInfo.m_pData;
			uint8_t *pBuildImgData = (uint8_t *)malloc(BuildImageSize);

			// build game layer
			for(int n = 0; n < MAP_IMAGE_ENTITY_LAYER_TYPE_COUNT; ++n)
			{
				bool BuildThisLayer = true;
				if(n == MAP_IMAGE_ENTITY_LAYER_TYPE_ALL_EXCEPT_SWITCH && !GameTypeHasFrontLayer &&
					!GameTypeHasSpeedupLayer && !GameTypeHasTeleLayer && !GameTypeHasTuneLayer)
					BuildThisLayer = false;
				else if(n == MAP_IMAGE_ENTITY_LAYER_TYPE_SWITCH && !GameTypeHasSwitchLayer)
					BuildThisLayer = false;

				dbg_assert(!m_aaEntitiesTextures[(EntitiesModType * 2) + (int)EntitiesAreMasked][n].IsValid(), "entities texture already loaded when it should not be");

				if(BuildThisLayer)
				{
					// set everything transparent
					mem_zero(pBuildImgData, BuildImageSize);

					for(int i = 0; i < 256; ++i)
					{
						bool ValidTile = i != 0;
						int TileIndex = i;
						if(EntitiesAreMasked)
						{
							if(EntitiesModType == MAP_IMAGE_MOD_TYPE_DDNET || EntitiesModType == MAP_IMAGE_MOD_TYPE_DDRACE)
							{
								if(EntitiesModType == MAP_IMAGE_MOD_TYPE_DDNET || TileIndex != TILE_BOOST)
								{
									if(n == MAP_IMAGE_ENTITY_LAYER_TYPE_ALL_EXCEPT_SWITCH && !IsValidGameTile((int)TileIndex) && !IsValidFrontTile((int)TileIndex) && !IsValidSpeedupTile((int)TileIndex) &&
										!IsValidTeleTile((int)TileIndex) && !IsValidTuneTile((int)TileIndex))
										ValidTile = false;
									else if(n == MAP_IMAGE_ENTITY_LAYER_TYPE_SWITCH)
									{
										if(!IsValidSwitchTile((int)TileIndex))
											ValidTile = false;
									}
								}
							}
							else if((EntitiesModType == MAP_IMAGE_MOD_TYPE_RACE) && IsCreditsTile((int)TileIndex))
							{
								ValidTile = false;
							}
							else if((EntitiesModType == MAP_IMAGE_MOD_TYPE_FNG) && IsCreditsTile((int)TileIndex))
							{
								ValidTile = false;
							}
							else if((EntitiesModType == MAP_IMAGE_MOD_TYPE_VANILLA) && IsCreditsTile((int)TileIndex))
							{
								ValidTile = false;
							}
						}

						if(EntitiesModType == MAP_IMAGE_MOD_TYPE_DDNET || EntitiesModType == MAP_IMAGE_MOD_TYPE_DDRACE)
						{
							if(n == MAP_IMAGE_ENTITY_LAYER_TYPE_SWITCH && TileIndex == TILE_SWITCHTIMEDOPEN)
								TileIndex = 8;
						}

						int X = TileIndex % 16;
						int Y = TileIndex / 16;

						int CopyWidth = ImgInfo.m_Width / 16;
						int CopyHeight = ImgInfo.m_Height / 16;
						if(ValidTile)
						{
							Graphics()->CopyTextureBufferSub(pBuildImgData, pTmpImgData, ImgInfo.m_Width, ImgInfo.m_Height, ColorChannelCount, X * CopyWidth, Y * CopyHeight, CopyWidth, CopyHeight);
						}
					}

					m_aaEntitiesTextures[(EntitiesModType * 2) + (int)EntitiesAreMasked][n] = Graphics()->LoadTextureRaw(ImgInfo.m_Width, ImgInfo.m_Height, ImgInfo.m_Format, pBuildImgData, ImgInfo.m_Format, TextureLoadFlag, aPath);
				}
				else
				{
					if(!m_TransparentTexture.IsValid())
					{
						// set everything transparent
						mem_zero(pBuildImgData, BuildImageSize);

						m_TransparentTexture = Graphics()->LoadTextureRaw(ImgInfo.m_Width, ImgInfo.m_Height, ImgInfo.m_Format, pBuildImgData, ImgInfo.m_Format, TextureLoadFlag, aPath);
					}
					m_aaEntitiesTextures[(EntitiesModType * 2) + (int)EntitiesAreMasked][n] = m_TransparentTexture;
				}
			}

			free(pBuildImgData);

			Graphics()->FreePNG(&ImgInfo);
		}
	}

	return m_aaEntitiesTextures[(EntitiesModType * 2) + (int)EntitiesAreMasked][EntityLayerType];
}

IGraphics::CTextureHandle CMapImages::GetSpeedupArrow()
{
	if(!m_SpeedupArrowIsLoaded)
	{
		int TextureLoadFlag = (Graphics()->HasTextureArrays() ? IGraphics::TEXLOAD_TO_2D_ARRAY_TEXTURE_SINGLE_LAYER : IGraphics::TEXLOAD_TO_3D_TEXTURE_SINGLE_LAYER) | IGraphics::TEXLOAD_NO_2D_TEXTURE;
		m_SpeedupArrowTexture = Graphics()->LoadTexture(g_pData->m_aImages[IMAGE_SPEEDUP_ARROW].m_pFilename, IStorage::TYPE_ALL, CImageInfo::FORMAT_AUTO, TextureLoadFlag);

		m_SpeedupArrowIsLoaded = true;
	}

	return m_SpeedupArrowTexture;
}

IGraphics::CTextureHandle CMapImages::GetOverlayBottom()
{
	return m_OverlayBottomTexture;
}

IGraphics::CTextureHandle CMapImages::GetOverlayTop()
{
	return m_OverlayTopTexture;
}

IGraphics::CTextureHandle CMapImages::GetOverlayCenter()
{
	return m_OverlayCenterTexture;
}

void CMapImages::ChangeEntitiesPath(const char *pPath)
{
	if(str_comp(pPath, "default") == 0)
		str_copy(m_aEntitiesPath, "editor/entities_clear");
	else
	{
		str_format(m_aEntitiesPath, sizeof(m_aEntitiesPath), "assets/entities/%s", pPath);
	}

	for(int i = 0; i < MAP_IMAGE_MOD_TYPE_COUNT * 2; ++i)
	{
		if(m_aEntitiesIsLoaded[i])
		{
			for(int n = 0; n < MAP_IMAGE_ENTITY_LAYER_TYPE_COUNT; ++n)
			{
				if(m_aaEntitiesTextures[i][n].IsValid())
					Graphics()->UnloadTexture(&(m_aaEntitiesTextures[i][n]));
				m_aaEntitiesTextures[i][n] = IGraphics::CTextureHandle();
			}

			m_aEntitiesIsLoaded[i] = false;
		}
	}
}

void CMapImages::SetTextureScale(int Scale)
{
	if(m_TextureScale == Scale)
		return;

	m_TextureScale = Scale;

	if(Graphics() && m_OverlayCenterTexture.IsValid()) // check if component was initialized
	{
		// reinitialize component
		Graphics()->UnloadTexture(&m_OverlayBottomTexture);
		Graphics()->UnloadTexture(&m_OverlayTopTexture);
		Graphics()->UnloadTexture(&m_OverlayCenterTexture);

		m_OverlayBottomTexture = IGraphics::CTextureHandle();
		m_OverlayTopTexture = IGraphics::CTextureHandle();
		m_OverlayCenterTexture = IGraphics::CTextureHandle();

		InitOverlayTextures();
	}
}

int CMapImages::GetTextureScale()
{
	return m_TextureScale;
}

IGraphics::CTextureHandle CMapImages::UploadEntityLayerText(int TextureSize, int MaxWidth, int YOffset)
{
	void *pMem = calloc(1024 * 1024 * 4, 1);

	UpdateEntityLayerText(pMem, 4, 1024, 1024, TextureSize, MaxWidth, YOffset, 0);
	UpdateEntityLayerText(pMem, 4, 1024, 1024, TextureSize, MaxWidth, YOffset, 1);
	UpdateEntityLayerText(pMem, 4, 1024, 1024, TextureSize, MaxWidth, YOffset, 2, 255);

	int TextureLoadFlag = (Graphics()->HasTextureArrays() ? IGraphics::TEXLOAD_TO_2D_ARRAY_TEXTURE : IGraphics::TEXLOAD_TO_3D_TEXTURE) | IGraphics::TEXLOAD_NO_2D_TEXTURE;
	IGraphics::CTextureHandle Texture = Graphics()->LoadTextureRaw(1024, 1024, CImageInfo::FORMAT_RGBA, pMem, CImageInfo::FORMAT_RGBA, TextureLoadFlag);
	free(pMem);

	return Texture;
}

void CMapImages::UpdateEntityLayerText(void *pTexBuffer, int ImageColorChannelCount, int TexWidth, int TexHeight, int TextureSize, int MaxWidth, int YOffset, int NumbersPower, int MaxNumber)
{
	char aBuf[4];
	int DigitsCount = NumbersPower + 1;

	int CurrentNumber = pow(10, NumbersPower);

	if(MaxNumber == -1)
		MaxNumber = CurrentNumber * 10 - 1;

	str_format(aBuf, sizeof(aBuf), "%d", CurrentNumber);

	int CurrentNumberSuitableFontSize = TextRender()->AdjustFontSize(aBuf, DigitsCount, TextureSize, MaxWidth);
	int UniversalSuitableFontSize = CurrentNumberSuitableFontSize * 0.92f; // should be smoothed enough to fit any digits combination

	YOffset += ((TextureSize - UniversalSuitableFontSize) / 2);

	for(; CurrentNumber <= MaxNumber; ++CurrentNumber)
	{
		str_format(aBuf, sizeof(aBuf), "%d", CurrentNumber);

		float x = (CurrentNumber % 16) * 64;
		float y = (CurrentNumber / 16) * 64;

		int ApproximateTextWidth = TextRender()->CalculateTextWidth(aBuf, DigitsCount, 0, UniversalSuitableFontSize);
		int XOffSet = (MaxWidth - clamp(ApproximateTextWidth, 0, MaxWidth)) / 2;

		TextRender()->UploadEntityLayerText(pTexBuffer, ImageColorChannelCount, TexWidth, TexHeight, (TexWidth / 16) - XOffSet, (TexHeight / 16) - YOffset, aBuf, DigitsCount, x + XOffSet, y + YOffset, UniversalSuitableFontSize);
	}
}

void CMapImages::InitOverlayTextures()
{
	int TextureSize = 64 * m_TextureScale / 100;
	TextureSize = clamp(TextureSize, 2, 64);
	int TextureToVerticalCenterOffset = (64 - TextureSize) / 2; // should be used to move texture to the center of 64 pixels area

	if(!m_OverlayBottomTexture.IsValid())
	{
		m_OverlayBottomTexture = UploadEntityLayerText(TextureSize / 2, 64, 32 + TextureToVerticalCenterOffset / 2);
	}

	if(!m_OverlayTopTexture.IsValid())
	{
		m_OverlayTopTexture = UploadEntityLayerText(TextureSize / 2, 64, TextureToVerticalCenterOffset / 2);
	}

	if(!m_OverlayCenterTexture.IsValid())
	{
		m_OverlayCenterTexture = UploadEntityLayerText(TextureSize, 64, TextureToVerticalCenterOffset);
	}
}
