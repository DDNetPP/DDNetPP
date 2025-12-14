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

// turns `"en"` into `0` (`LANG_EN`)
int str_to_lang_id(const char *pLanguage);

// Translate strings shown to users
// (localize)
//
// @param Language can be `LANG_EN` or `LANG_RU`
// @param pStr english input text that should be translated
//
// @return translated string
const char *str_ddpp_loc(int Language, const char *pStr);

#endif
