#ifndef GAME_SERVER_LETTERS_H
#define GAME_SERVER_LETTERS_H

#include "../ddpp/ascii_table.h"
#include "../gamecontext.h"

class CGameContext;

class CLetters
{
public:
	CLetters(CGameContext *pGameServer);

	void ToUpper(char *pStr);
	void UpdateBuffers(int Ascii, int offset = 0);
	void SendChat(int ClientID, int Ascii);
	void SendChat(int ClientID, const char *pStr);
	void DebugPrint(int Ascii);

private:
	char m_aaAsciiBuf[5][(ASCII_CHAR_SIZE * ASCII_BUF_LENGTH) + 1 /* ((size*double+ASCII_PADDING)*ASCII_BUF_LENGTH)+null terminator */];

	CGameContext *m_pGameServer;
	CGameContext *GameServer() const { return m_pGameServer; }
};

#endif
