#include "gamecontext.h"

#include "letters.h"

CLetters::CLetters(CGameContext *pGameServer)
{
	m_pGameServer = pGameServer;
}

void CLetters::UpdateBuffers(int ascii, int offset)
{
	if(ascii <= 0 || ascii >= ASCII_TABLE_SIZE)
		return;

	// printf("**** Updating buffers ascii=%d ****\n", ascii);
	// printf("offset %d (index)\n", offset);
	int NumOffset = offset * (ASCII_CHAR_SIZE / 2);
	// printf("NumOffset %d (char size)\n", NumOffset);
	for(int i = 0; i < 5; i++)
	{
		for(int k = NumOffset; k < NumOffset + ASCII_PADDING; k++)
		{
			m_aaAsciiBuf[i][k * 2] = ' '; // padding
			m_aaAsciiBuf[i][k * 2 + 1] = ' '; // padding
			// printf("padding indicies %d %d\n", k*2, k*2+1);
		}
		int numK = NumOffset + ASCII_CHAR_WIDTH + ASCII_PADDING;
		for(int k = NumOffset + ASCII_PADDING, kt = 0; k < numK; k++, kt++)
		{
			m_aaAsciiBuf[i][k * 2] = m_aaaAsciiTable[ascii][i][kt] ? ':' : ' '; // colons have same width as spaces in default tw chat font
			m_aaAsciiBuf[i][k * 2 + 1] = m_aaaAsciiTable[ascii][i][kt] ? ':' : ' ';
			// printf("filling indicies %d %d  k %d/%d\n", k*2, k*2+1, k, numK);
		}
		int nullterm = (offset + 1) * ASCII_CHAR_SIZE;
		m_aaAsciiBuf[i][nullterm] = '\0';
		// printf("terminating at %d\n", nullterm);
	}
}

void CLetters::SendChat(int ClientID, int ascii)
{
	UpdateBuffers(ascii);
	for(int i = 0; i < 5; i++)
		GameServer()->SendChatTarget(ClientID, m_aaAsciiBuf[i]);
}

void CLetters::ToUpper(char *pStr)
{
	while(*pStr++)
		*pStr = str_uppercase(*pStr);
}

void CLetters::SendChat(int ClientID, const char *pStr)
{
	char aUpper[ASCII_BUF_LENGTH + 1];
	str_copy(aUpper, pStr, sizeof(aUpper));
	ToUpper(aUpper);
	printf("CLetters::SendChat(id=%d, str=%s) upper=%s\n", ClientID, pStr, aUpper);
	for(int i = 0; i < ASCII_BUF_LENGTH; i++)
	{
		if(!aUpper[i])
			continue;

		UpdateBuffers((int)aUpper[i], i);
	}
	for(int i = 0; i < 5; i++)
		GameServer()->SendChatTarget(ClientID, m_aaAsciiBuf[i]);
}

void CLetters::DebugPrint(int ascii)
{
	UpdateBuffers(ascii);
	for(int i = 0; i < 5; i++)
		printf("%s\n", m_aaAsciiBuf[i]);
}
