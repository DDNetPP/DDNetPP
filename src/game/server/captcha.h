#ifndef GAME_SERVER_CAPTCHA_H
#define GAME_SERVER_CAPTCHA_H

class CGameContext;

class CCaptcha
{
private:
	bool CheckGenerate();
	void Generate();
	void GenBigText(int type);
	void GenQuestion(int type);
	void ShowQuestion();
	void SendChat(const char *pMsg);
	bool Score(int value);

	int RandNum() { return rand() % 9 + 48; }
	int RandAlpha() { return rand() % 23 + 65; }
	int RandAlphaNum() { return (rand() % 2) ? RandNum() : RandAlpha(); }

	int m_ClientID;
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
	CCaptcha(CGameContext *pGameServer, int ClientID);

	bool Prompt(const char *pAnswer);

	int GetScore() { return m_Score; }
	bool IsHuman() { return m_IsHuman; }
};

#endif