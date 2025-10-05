/* DDNet++ 2022 */

#include "letters.h"

#include "../gamecontext.h"

CLetters::CLetters(CGameContext *pGameServer)
{
	m_pGameServer = pGameServer;
}

void CLetters::UpdateBuffers(int Ascii, int Offset)
{
	if(Ascii <= 0 || Ascii >= ASCII_TABLE_SIZE)
		return;

	// printf("**** Updating buffers ascii=%d ****\n", ascii);
	// printf("offset %d (index)\n", offset);
	int NumOffset = Offset * (ASCII_CHAR_SIZE / 2);
	// printf("NumOffset %d (char size)\n", NumOffset);
	for(int i = 0; i < 5; i++)
	{
		for(int k = NumOffset; k < NumOffset + ASCII_PADDING; k++)
		{
			m_aaAsciiBuf[i][k * 2] = ' '; // padding
			m_aaAsciiBuf[i][(k * 2) + 1] = ' '; // padding
			// printf("padding indicies %d %d\n", k*2, k*2+1);
		}
		int NumK = NumOffset + ASCII_CHAR_WIDTH + ASCII_PADDING;
		for(int BufferIndex = NumOffset + ASCII_PADDING, TableIndex = 0; BufferIndex < NumK; BufferIndex++, TableIndex++)
		{
			m_aaAsciiBuf[i][BufferIndex * 2] = gs_aaaAsciiTable[Ascii][i][TableIndex] ? ':' : ' '; // colons have same width as spaces in default tw chat font
			m_aaAsciiBuf[i][(BufferIndex * 2) + 1] = gs_aaaAsciiTable[Ascii][i][TableIndex] ? ':' : ' ';
			// printf("filling indicies %d %d  k %d/%d\n", k*2, k*2+1, k, numK);
		}
		int Nullterm = (Offset + 1) * ASCII_CHAR_SIZE;
		m_aaAsciiBuf[i][Nullterm] = '\0';
		// printf("terminating at %d\n", nullterm);
	}
}

void CLetters::SendChat(int ClientId, int Ascii)
{
	UpdateBuffers(Ascii);
	for(auto &AsciiBuf : m_aaAsciiBuf)
		GameServer()->SendChatTarget(ClientId, AsciiBuf);
}

void CLetters::ToUpper(char *pStr)
{
	while(*pStr++)
		*pStr = str_uppercase(*pStr);
}

void CLetters::SendChat(int ClientId, const char *pStr)
{
	char aUpper[ASCII_BUF_LENGTH + 1];
	str_copy(aUpper, pStr, sizeof(aUpper));
	ToUpper(aUpper);
	printf("CLetters::SendChat(id=%d, str=%s) upper=%s\n", ClientId, pStr, aUpper);
	for(int i = 0; i < ASCII_BUF_LENGTH; i++)
	{
		if(!aUpper[i])
			continue;

		UpdateBuffers((int)aUpper[i], i);
	}
	for(auto &AsciiBuf : m_aaAsciiBuf)
		GameServer()->SendChatTarget(ClientId, AsciiBuf);
}

void CLetters::DebugPrint(int Ascii)
{
	UpdateBuffers(Ascii);
	for(auto &AsciiBuf : m_aaAsciiBuf)
		printf("%s\n", AsciiBuf);
}
