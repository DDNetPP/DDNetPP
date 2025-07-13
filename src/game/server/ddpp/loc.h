#ifndef GAME_SERVER_DDPP_LOC_H
#define GAME_SERVER_DDPP_LOC_H

class CGameContext;

enum
{
	LANG_EN,
	LANG_RU,
};

class CLoc
{
	CGameContext *m_pGameContext;

public:
	CLoc(CGameContext *pGameContext);

	const char *DDPPLocalize(const char *pStr, int ClientId) const;
	CGameContext *GameServer() const;
};

#endif
