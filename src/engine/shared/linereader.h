/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef ENGINE_SHARED_LINEREADER_H
#define ENGINE_SHARED_LINEREADER_H
#include <base/system.h>

// buffered stream for reading lines, should perhaps be something smaller
class CLineReader
{
	char m_aBuffer[4 * 8192 + 1]; // 1 additional byte for null termination
	unsigned m_BufferPos;
	unsigned m_BufferSize;
	unsigned m_BufferMaxSize;
	IOHANDLE m_File;

public:
	void Init(IOHANDLE File);
	char *Get(); // Returned string is only valid until next Get() call
};
#endif
