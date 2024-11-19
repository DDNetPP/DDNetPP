#ifndef GAME_SERVER_CAPTCHA_H
#define GAME_SERVER_CAPTCHA_H

class CGameContext;

class CCaptcha
{
private:
	bool CheckGenerate();
	void Generate();
	void GenBigText(int Type);
	void GenQuestion(int Type);
	void ShowQuestion();
	void SendChat(const char *pMsg);
	bool Score(int value);

	int RandNum();
	int RandAlpha();
	int RandAlphaNum();

	int m_ClientId;
	int m_Score;
	bool m_IsHuman;
	char m_aBigText[7];
	char m_aQuestion[128];
	char m_aAnswer[128];

	CGameContext *m_pGameServer;
	CGameContext *GameServer() const { return m_pGameServer; }

	enum
	{
		ALPHA,
		NUM,
		ALPHANUM
	};

	enum
	{
		MATH,
		HARDCODE
	};

public:
	CCaptcha(CGameContext *pGameServer, int ClientId);

	bool Prompt(const char *pAnswer);

	int GetScore() const { return m_Score; }
	bool IsHuman() const { return m_IsHuman; }
};

#endif
