/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef ENGINE_STORAGE_H
#define ENGINE_STORAGE_H

#include "kernel.h"

#include <set>
#include <string>

enum
{
	MAX_PATHS = 16
};

class IStorage : public IInterface
{
	MACRO_INTERFACE("storage", 0)
public:
	enum
	{
		TYPE_SAVE = 0,
		TYPE_ALL = -1,
		TYPE_ABSOLUTE = -2,
		/**
		 * Translates to TYPE_SAVE if a path is relative
		 * and to TYPE_ABSOLUTE if a path is absolute.
		 * Only usable with OpenFile, ReadFile, ReadFileStr
		 * and GetCompletePath.
		 */
		TYPE_SAVE_OR_ABSOLUTE = -3,
		/**
		 * Translates to TYPE_ALL if a path is relative
		 * and to TYPE_ABSOLUTE if a path is absolute.
		 * Only usable with OpenFile, ReadFile, ReadFileStr
		 * and GetCompletePath.
		 */
		TYPE_ALL_OR_ABSOLUTE = -4,

		STORAGETYPE_BASIC = 0,
		STORAGETYPE_SERVER,
		STORAGETYPE_CLIENT,
	};

	virtual void ListDirectory(int Type, const char *pPath, FS_LISTDIR_CALLBACK pfnCallback, void *pUser) = 0;
	virtual void ListDirectoryInfo(int Type, const char *pPath, FS_LISTDIR_CALLBACK_FILEINFO pfnCallback, void *pUser) = 0;
	virtual IOHANDLE OpenFile(const char *pFilename, int Flags, int Type, char *pBuffer = nullptr, int BufferSize = 0) = 0;
	virtual bool ReadFile(const char *pFilename, int Type, void **ppResult, unsigned *pResultLen) = 0;
	virtual char *ReadFileStr(const char *pFilename, int Type) = 0;
	virtual bool FindFile(const char *pFilename, const char *pPath, int Type, char *pBuffer, int BufferSize) = 0;
	virtual size_t FindFiles(const char *pFilename, const char *pPath, int Type, std::set<std::string> *pEntries) = 0;
	virtual bool RemoveFile(const char *pFilename, int Type) = 0;
	virtual bool RenameFile(const char *pOldFilename, const char *pNewFilename, int Type) = 0;
	virtual bool CreateFolder(const char *pFoldername, int Type) = 0;
	virtual void GetCompletePath(int Type, const char *pDir, char *pBuffer, unsigned BufferSize) = 0;

	virtual bool RemoveBinaryFile(const char *pFilename) = 0;
	virtual bool RenameBinaryFile(const char *pOldFilename, const char *pNewFilename) = 0;
	virtual const char *GetBinaryPath(const char *pFilename, char *pBuffer, unsigned BufferSize) = 0;
	virtual const char *GetBinaryPathAbsolute(const char *pFilename, char *pBuffer, unsigned BufferSize) = 0;

	static void StripPathAndExtension(const char *pFilename, char *pBuffer, int BufferSize);
	static const char *FormatTmpPath(char *aBuf, unsigned BufSize, const char *pPath);
};

extern IStorage *CreateStorage(int StorageType, int NumArgs, const char **ppArguments);
extern IStorage *CreateLocalStorage();
extern IStorage *CreateTempStorage(const char *pDirectory);

#endif
