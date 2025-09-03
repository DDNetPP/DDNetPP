#ifndef GAME_MAP_ENVELOPE_EXTREMA_H
#define GAME_MAP_ENVELOPE_EXTREMA_H

#include <engine/map.h>
#include <game/map/render_map.h>

#include <vector>

class CEnvelopeExtrema
{
public:
	CEnvelopeExtrema(IMap *pMap);

	class CEnvelopeExtremaItem
	{
	public:
		bool m_Available;
		ivec2 m_Minima;
		ivec2 m_Maxima;
	};

	const CEnvelopeExtremaItem &GetExtrema(int Env) const;

private:
	void CalculateEnvelope(const CMapItemEnvelope *pEnvelopeItem, int EnvId);
	void CalculateExtrema();

	CEnvelopeExtremaItem m_EnvelopeExtremaItemNone;
	CEnvelopeExtremaItem m_EnvelopeExtremaItemInvalid;

	CMapBasedEnvelopePointAccess m_EnvelopePoints;
	IMap *m_pMap;

	std::vector<CEnvelopeExtremaItem> m_vEnvelopeExtrema;
};

#endif
