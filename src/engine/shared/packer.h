/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef ENGINE_SHARED_PACKER_H
#define ENGINE_SHARED_PACKER_H

class CPacker
{
public:
	enum
	{
		PACKER_BUFFER_SIZE = 1024 * 64
	};

private:
	unsigned char m_aBuffer[PACKER_BUFFER_SIZE];
	unsigned char *m_pCurrent;
	unsigned char *m_pEnd;
	bool m_Error;

public:
	void Reset();
	void AddInt(int i);
	void AddString(const char *pStr, int Limit = PACKER_BUFFER_SIZE);
	void AddRaw(const void *pData, int Size);

	int Size() const { return (int)(m_pCurrent - m_aBuffer); }
	const unsigned char *Data() const { return m_aBuffer; }
	bool Error() const { return m_Error; }
};

class CUnpacker
{
	const unsigned char *m_pStart;
	const unsigned char *m_pCurrent;
	const unsigned char *m_pEnd;
	bool m_Error;

public:
	enum
	{
		SANITIZE = 1,
		SANITIZE_CC = 2,
		SKIP_START_WHITESPACES = 4
	};

	void Reset(const void *pData, int Size);
	int GetInt();
	int GetIntOrDefault(int Default);
	int GetUncompressedInt();
	int GetUncompressedIntOrDefault(int Default);
	const char *GetString(int SanitizeType = SANITIZE);
	const unsigned char *GetRaw(int Size);
	bool Error() const { return m_Error; }

	int CompleteSize() const { return m_pEnd - m_pStart; }
	const unsigned char *CompleteData() const { return m_pStart; }
};

#endif
