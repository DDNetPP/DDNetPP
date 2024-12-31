/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <base/hash_ctxt.h>
#include <base/log.h>
#include <base/math.h>
#include <base/system.h>

#include <engine/client/updater.h>
#include <engine/shared/linereader.h>
#include <engine/storage.h>

#include <unordered_set>

#ifdef CONF_PLATFORM_HAIKU
#include <cstdlib>
#endif

#include <zlib.h>

class CStorage : public IStorage
{
public:
	char m_aaStoragePaths[MAX_PATHS][IO_MAX_PATH_LENGTH];
	int m_NumPaths;
	char m_aDatadir[IO_MAX_PATH_LENGTH];
	char m_aUserdir[IO_MAX_PATH_LENGTH];
	char m_aCurrentdir[IO_MAX_PATH_LENGTH];
	char m_aBinarydir[IO_MAX_PATH_LENGTH];

	CStorage()
	{
		mem_zero(m_aaStoragePaths, sizeof(m_aaStoragePaths));
		m_NumPaths = 0;
		m_aDatadir[0] = '\0';
		m_aUserdir[0] = '\0';
		m_aCurrentdir[0] = '\0';
		m_aBinarydir[0] = '\0';
	}

	int Init(int StorageType, int NumArgs, const char **ppArguments)
	{
#if defined(CONF_PLATFORM_ANDROID)
		// See InitAndroid in android_main.cpp for details about Android storage handling.
		// The current working directory is set to the app specific external storage location
		// on Android. The user data is stored within a folder "user" in the external storage.
		str_copy(m_aUserdir, "user");
#else
		// get userdir
		char aFallbackUserdir[IO_MAX_PATH_LENGTH];
		if(fs_storage_path("DDNet", m_aUserdir, sizeof(m_aUserdir)))
		{
			log_error("storage", "could not determine user directory");
		}
		if(fs_storage_path("Teeworlds", aFallbackUserdir, sizeof(aFallbackUserdir)))
		{
			log_error("storage", "could not determine fallback user directory");
		}

		if((m_aUserdir[0] == '\0' || !fs_is_dir(m_aUserdir)) && aFallbackUserdir[0] != '\0' && fs_is_dir(aFallbackUserdir))
		{
			str_copy(m_aUserdir, aFallbackUserdir);
		}
#endif

		// get datadir
		FindDatadir(ppArguments[0]);

		// get binarydir
		FindBinarydir(ppArguments[0]);

		// get currentdir
		if(!fs_getcwd(m_aCurrentdir, sizeof(m_aCurrentdir)))
		{
			log_error("storage", "could not determine current directory");
		}

		// load paths from storage.cfg
		LoadPaths(ppArguments[0]);

		if(!m_NumPaths)
		{
			dbg_msg("storage", "using standard paths");
			AddDefaultPaths();
		}

		// add save directories
		if(StorageType != STORAGETYPE_BASIC && m_NumPaths && (!m_aaStoragePaths[TYPE_SAVE][0] || fs_makedir_rec_for(m_aaStoragePaths[TYPE_SAVE]) || !fs_makedir(m_aaStoragePaths[TYPE_SAVE])))
		{
			if(StorageType == STORAGETYPE_CLIENT)
			{
				CreateFolder("screenshots", TYPE_SAVE);
				CreateFolder("screenshots/auto", TYPE_SAVE);
				CreateFolder("screenshots/auto/stats", TYPE_SAVE);
				CreateFolder("maps", TYPE_SAVE);
				CreateFolder("maps/auto", TYPE_SAVE);
				CreateFolder("mapres", TYPE_SAVE);
				CreateFolder("downloadedmaps", TYPE_SAVE);
				CreateFolder("skins", TYPE_SAVE);
				CreateFolder("skins7", TYPE_SAVE);
				CreateFolder("downloadedskins", TYPE_SAVE);
				CreateFolder("themes", TYPE_SAVE);
				CreateFolder("communityicons", TYPE_SAVE);
				CreateFolder("assets", TYPE_SAVE);
				CreateFolder("assets/emoticons", TYPE_SAVE);
				CreateFolder("assets/entities", TYPE_SAVE);
				CreateFolder("assets/game", TYPE_SAVE);
				CreateFolder("assets/particles", TYPE_SAVE);
				CreateFolder("assets/hud", TYPE_SAVE);
				CreateFolder("assets/extras", TYPE_SAVE);
#if defined(CONF_VIDEORECORDER)
				CreateFolder("videos", TYPE_SAVE);
#endif
			}
			CreateFolder("dumps", TYPE_SAVE);
			CreateFolder("demos", TYPE_SAVE);
			CreateFolder("demos/auto", TYPE_SAVE);
			CreateFolder("demos/auto/race", TYPE_SAVE);
			CreateFolder("demos/auto/server", TYPE_SAVE);
			CreateFolder("demos/replays", TYPE_SAVE);
			CreateFolder("editor", TYPE_SAVE);
			CreateFolder("ghosts", TYPE_SAVE);
			CreateFolder("teehistorian", TYPE_SAVE);
		}

		return m_NumPaths ? 0 : 1;
	}

	void LoadPaths(const char *pArgv0)
	{
		// check current directory
		IOHANDLE File = io_open("storage.cfg", IOFLAG_READ);
		if(!File)
		{
			// check usable path in argv[0]
			unsigned int Pos = ~0U;
			for(unsigned i = 0; pArgv0[i]; i++)
				if(pArgv0[i] == '/' || pArgv0[i] == '\\')
					Pos = i;
			if(Pos < IO_MAX_PATH_LENGTH)
			{
				char aBuffer[IO_MAX_PATH_LENGTH];
				str_copy(aBuffer, pArgv0, Pos + 1);
				str_append(aBuffer, "/storage.cfg");
				File = io_open(aBuffer, IOFLAG_READ);
			}

			if(Pos >= IO_MAX_PATH_LENGTH || !File)
			{
				dbg_msg("storage", "couldn't open storage.cfg");
				return;
			}
		}

		CLineReader LineReader;
		if(!LineReader.OpenFile(File))
		{
			dbg_msg("storage", "couldn't open storage.cfg");
			return;
		}
		while(const char *pLine = LineReader.Get())
		{
			const char *pLineWithoutPrefix = str_startswith(pLine, "add_path ");
			if(pLineWithoutPrefix)
			{
				AddPath(pLineWithoutPrefix);
			}
		}

		if(!m_NumPaths)
			dbg_msg("storage", "no paths found in storage.cfg");
	}

	void AddDefaultPaths()
	{
		AddPath("$USERDIR");
		AddPath("$DATADIR");
		AddPath("$CURRENTDIR");
	}

	void AddPath(const char *pPath)
	{
		if(!pPath[0])
		{
			log_error("storage", "cannot add empty path");
			return;
		}
		if(m_NumPaths >= MAX_PATHS)
		{
			log_error("storage", "cannot add path '%s', the maximum number of paths is %d", pPath, MAX_PATHS);
			return;
		}

		if(!str_comp(pPath, "$USERDIR"))
		{
			if(m_aUserdir[0])
			{
				str_copy(m_aaStoragePaths[m_NumPaths++], m_aUserdir);
				dbg_msg("storage", "added path '$USERDIR' ('%s')", m_aUserdir);
			}
			else
			{
				log_error("storage", "cannot add path '$USERDIR' because it could not be determined");
			}
		}
		else if(!str_comp(pPath, "$DATADIR"))
		{
			if(m_aDatadir[0])
			{
				str_copy(m_aaStoragePaths[m_NumPaths++], m_aDatadir);
				dbg_msg("storage", "added path '$DATADIR' ('%s')", m_aDatadir);
			}
			else
			{
				log_error("storage", "cannot add path '$DATADIR' because it could not be determined");
			}
		}
		else if(!str_comp(pPath, "$CURRENTDIR"))
		{
			m_aaStoragePaths[m_NumPaths++][0] = '\0';
			dbg_msg("storage", "added path '$CURRENTDIR' ('%s')", m_aCurrentdir);
		}
		else if(str_utf8_check(pPath))
		{
			if(fs_is_dir(pPath))
			{
				str_copy(m_aaStoragePaths[m_NumPaths++], pPath);
				dbg_msg("storage", "added path '%s'", pPath);
			}
			else
			{
				log_error("storage", "cannot add path '%s', which is not a directory", pPath);
			}
		}
		else
		{
			log_error("storage", "cannot add path containing invalid UTF-8");
		}
	}

	void FindDatadir(const char *pArgv0)
	{
		// 1) use data-dir in PWD if present
		if(fs_is_dir("data/mapres"))
		{
			str_copy(m_aDatadir, "data");
			return;
		}

#if defined(DATA_DIR)
		// 2) use compiled-in data-dir if present
		if(fs_is_dir(DATA_DIR "/mapres"))
		{
			str_copy(m_aDatadir, DATA_DIR, sizeof(m_aDatadir));
			return;
		}
#endif

		// 3) check for usable path in argv[0]
		{
#ifdef CONF_PLATFORM_HAIKU
			pArgv0 = realpath(pArgv0, NULL);
#endif
			unsigned int Pos = ~0U;
			for(unsigned i = 0; pArgv0[i]; i++)
				if(pArgv0[i] == '/' || pArgv0[i] == '\\')
					Pos = i;

			if(Pos < IO_MAX_PATH_LENGTH)
			{
				char aBuf[IO_MAX_PATH_LENGTH];
				char aDir[IO_MAX_PATH_LENGTH];
				str_copy(aDir, pArgv0, Pos + 1);
				str_format(aBuf, sizeof(aBuf), "%s/data/mapres", aDir);
				if(fs_is_dir(aBuf))
				{
					str_format(m_aDatadir, sizeof(m_aDatadir), "%s/data", aDir);
					return;
				}
			}
		}
#ifdef CONF_PLATFORM_HAIKU
		free((void *)pArgv0);
#endif

#if defined(CONF_FAMILY_UNIX)
		// 4) check for all default locations
		{
			const char *apDirs[] = {
				"/usr/share/ddnet",
				"/usr/share/games/ddnet",
				"/usr/local/share/ddnet",
				"/usr/local/share/games/ddnet",
				"/usr/pkg/share/ddnet",
				"/usr/pkg/share/games/ddnet",
				"/opt/ddnet"};

			for(const char *pDir : apDirs)
			{
				char aBuf[128];
				str_format(aBuf, sizeof(aBuf), "%s/data/mapres", pDir);
				if(fs_is_dir(aBuf))
				{
					str_format(m_aDatadir, sizeof(m_aDatadir), "%s/data", pDir);
					return;
				}
			}
		}
#endif

		dbg_msg("storage", "warning: no data directory found");
	}

	void FindBinarydir(const char *pArgv0)
	{
#if defined(BINARY_DIR)
		str_copy(m_aBinarydir, BINARY_DIR, sizeof(m_aBinarydir));
		return;
#endif

		// check for usable path in argv[0]
		{
			unsigned int Pos = ~0U;
			for(unsigned i = 0; pArgv0[i]; i++)
				if(pArgv0[i] == '/' || pArgv0[i] == '\\')
					Pos = i;

			if(Pos < IO_MAX_PATH_LENGTH)
			{
				char aBuf[IO_MAX_PATH_LENGTH];
				str_copy(m_aBinarydir, pArgv0, Pos + 1);
				str_format(aBuf, sizeof(aBuf), "%s/" PLAT_SERVER_EXEC, m_aBinarydir);
				if(fs_is_file(aBuf))
				{
					return;
				}
#if defined(CONF_PLATFORM_MACOS)
				str_append(m_aBinarydir, "/../../../DDNet-Server.app/Contents/MacOS");
				str_format(aBuf, sizeof(aBuf), "%s/" PLAT_SERVER_EXEC, m_aBinarydir);
				if(fs_is_file(aBuf))
				{
					return;
				}
#endif
			}
		}

		// no binary directory found, use $PATH on Posix, $PWD on Windows
		m_aBinarydir[0] = '\0';
	}

	int NumPaths() const override
	{
		return m_NumPaths;
	}

	struct SListDirectoryInfoUniqueCallbackData
	{
		FS_LISTDIR_CALLBACK_FILEINFO m_pfnDelegate;
		void *m_pDelegateUser;
		std::unordered_set<std::string> m_Seen;
	};

	static int ListDirectoryInfoUniqueCallback(const CFsFileInfo *pInfo, int IsDir, int Type, void *pUser)
	{
		SListDirectoryInfoUniqueCallbackData *pData = static_cast<SListDirectoryInfoUniqueCallbackData *>(pUser);
		auto [_, InsertionTookPlace] = pData->m_Seen.emplace(pInfo->m_pName);
		if(InsertionTookPlace)
			return pData->m_pfnDelegate(pInfo, IsDir, Type, pData->m_pDelegateUser);
		return 0;
	}

	void ListDirectoryInfo(int Type, const char *pPath, FS_LISTDIR_CALLBACK_FILEINFO pfnCallback, void *pUser) override
	{
		char aBuffer[IO_MAX_PATH_LENGTH];
		if(Type == TYPE_ALL)
		{
			SListDirectoryInfoUniqueCallbackData Data;
			Data.m_pfnDelegate = pfnCallback;
			Data.m_pDelegateUser = pUser;
			// list all available directories
			for(int i = TYPE_SAVE; i < m_NumPaths; ++i)
				fs_listdir_fileinfo(GetPath(i, pPath, aBuffer, sizeof(aBuffer)), ListDirectoryInfoUniqueCallback, i, &Data);
		}
		else if(Type >= TYPE_SAVE && Type < m_NumPaths)
		{
			// list wanted directory
			fs_listdir_fileinfo(GetPath(Type, pPath, aBuffer, sizeof(aBuffer)), pfnCallback, Type, pUser);
		}
		else
		{
			dbg_assert(false, "Type invalid");
		}
	}

	struct SListDirectoryUniqueCallbackData
	{
		FS_LISTDIR_CALLBACK m_pfnDelegate;
		void *m_pDelegateUser;
		std::unordered_set<std::string> m_Seen;
	};

	static int ListDirectoryUniqueCallback(const char *pName, int IsDir, int Type, void *pUser)
	{
		SListDirectoryUniqueCallbackData *pData = static_cast<SListDirectoryUniqueCallbackData *>(pUser);
		auto [_, InsertionTookPlace] = pData->m_Seen.emplace(pName);
		if(InsertionTookPlace)
			return pData->m_pfnDelegate(pName, IsDir, Type, pData->m_pDelegateUser);
		return 0;
	}

	void ListDirectory(int Type, const char *pPath, FS_LISTDIR_CALLBACK pfnCallback, void *pUser) override
	{
		char aBuffer[IO_MAX_PATH_LENGTH];
		if(Type == TYPE_ALL)
		{
			SListDirectoryUniqueCallbackData Data;
			Data.m_pfnDelegate = pfnCallback;
			Data.m_pDelegateUser = pUser;
			// list all available directories
			for(int i = TYPE_SAVE; i < m_NumPaths; ++i)
				fs_listdir(GetPath(i, pPath, aBuffer, sizeof(aBuffer)), ListDirectoryUniqueCallback, i, &Data);
		}
		else if(Type >= TYPE_SAVE && Type < m_NumPaths)
		{
			// list wanted directory
			fs_listdir(GetPath(Type, pPath, aBuffer, sizeof(aBuffer)), pfnCallback, Type, pUser);
		}
		else
		{
			dbg_assert(false, "Type invalid");
		}
	}

	const char *GetPath(int Type, const char *pDir, char *pBuffer, unsigned BufferSize)
	{
		if(Type == TYPE_ABSOLUTE)
		{
			str_copy(pBuffer, pDir, BufferSize);
		}
		else
		{
			str_format(pBuffer, BufferSize, "%s%s%s", m_aaStoragePaths[Type], !m_aaStoragePaths[Type][0] ? "" : "/", pDir);
		}
		return pBuffer;
	}

	void TranslateType(int &Type, const char *pPath)
	{
		if(Type == TYPE_SAVE_OR_ABSOLUTE)
			Type = fs_is_relative_path(pPath) ? TYPE_SAVE : TYPE_ABSOLUTE;
		else if(Type == TYPE_ALL_OR_ABSOLUTE)
			Type = fs_is_relative_path(pPath) ? TYPE_ALL : TYPE_ABSOLUTE;
	}

	IOHANDLE OpenFile(const char *pFilename, int Flags, int Type, char *pBuffer = 0, int BufferSize = 0) override
	{
		TranslateType(Type, pFilename);

		char aBuffer[IO_MAX_PATH_LENGTH];
		if(!pBuffer)
		{
			pBuffer = aBuffer;
			BufferSize = sizeof(aBuffer);
		}

		if(Type == TYPE_ABSOLUTE)
		{
			return io_open(GetPath(TYPE_ABSOLUTE, pFilename, pBuffer, BufferSize), Flags);
		}
		if(str_startswith(pFilename, "mapres/../skins/"))
		{
			pFilename = pFilename + 10; // just start from skins/
		}
		if(pFilename[0] == '/' || pFilename[0] == '\\' || str_find(pFilename, "../") != NULL || str_find(pFilename, "..\\") != NULL
#ifdef CONF_FAMILY_WINDOWS
			|| (pFilename[0] && pFilename[1] == ':')
#endif
		)
		{
			// don't escape base directory
		}
		else if(Flags & IOFLAG_WRITE)
		{
			return io_open(GetPath(TYPE_SAVE, pFilename, pBuffer, BufferSize), Flags);
		}
		else
		{
			if(Type == TYPE_ALL)
			{
				// check all available directories
				for(int i = TYPE_SAVE; i < m_NumPaths; ++i)
				{
					IOHANDLE Handle = io_open(GetPath(i, pFilename, pBuffer, BufferSize), Flags);
					if(Handle)
						return Handle;
				}
			}
			else if(Type >= TYPE_SAVE && Type < m_NumPaths)
			{
				// check wanted directory
				IOHANDLE Handle = io_open(GetPath(Type, pFilename, pBuffer, BufferSize), Flags);
				if(Handle)
					return Handle;
			}
			else
			{
				dbg_assert(false, "Type invalid");
			}
		}

		pBuffer[0] = 0;
		return 0;
	}

	template<typename F>
	bool GenericExists(const char *pFilename, int Type, F &&CheckFunction)
	{
		TranslateType(Type, pFilename);

		char aBuffer[IO_MAX_PATH_LENGTH];
		if(Type == TYPE_ALL)
		{
			// check all available directories
			for(int i = TYPE_SAVE; i < m_NumPaths; ++i)
			{
				if(CheckFunction(GetPath(i, pFilename, aBuffer, sizeof(aBuffer))))
					return true;
			}
			return false;
		}
		else if(Type == TYPE_ABSOLUTE || (Type >= TYPE_SAVE && Type < m_NumPaths))
		{
			// check wanted directory
			return CheckFunction(GetPath(Type, pFilename, aBuffer, sizeof(aBuffer)));
		}
		else
		{
			dbg_assert(false, "Type invalid");
			return false;
		}
	}

	bool FileExists(const char *pFilename, int Type) override
	{
		return GenericExists(pFilename, Type, fs_is_file);
	}

	bool FolderExists(const char *pFilename, int Type) override
	{
		return GenericExists(pFilename, Type, fs_is_dir);
	}

	bool ReadFile(const char *pFilename, int Type, void **ppResult, unsigned *pResultLen) override
	{
		IOHANDLE File = OpenFile(pFilename, IOFLAG_READ, Type);
		if(!File)
		{
			*ppResult = nullptr;
			*pResultLen = 0;
			return false;
		}
		const bool ReadSuccess = io_read_all(File, ppResult, pResultLen);
		io_close(File);
		if(!ReadSuccess)
		{
			*ppResult = nullptr;
			*pResultLen = 0;
			return false;
		}
		return true;
	}

	char *ReadFileStr(const char *pFilename, int Type) override
	{
		IOHANDLE File = OpenFile(pFilename, IOFLAG_READ, Type);
		if(!File)
			return nullptr;
		char *pResult = io_read_all_str(File);
		io_close(File);
		return pResult;
	}

	bool RetrieveTimes(const char *pFilename, int Type, time_t *pCreated, time_t *pModified) override
	{
		dbg_assert(Type == TYPE_ABSOLUTE || (Type >= TYPE_SAVE && Type < m_NumPaths), "Type invalid");

		char aBuffer[IO_MAX_PATH_LENGTH];
		return fs_file_time(GetPath(Type, pFilename, aBuffer, sizeof(aBuffer)), pCreated, pModified) == 0;
	}

	bool CalculateHashes(const char *pFilename, int Type, SHA256_DIGEST *pSha256, unsigned *pCrc) override
	{
		dbg_assert(pSha256 != nullptr || pCrc != nullptr, "At least one output argument required");

		IOHANDLE File = OpenFile(pFilename, IOFLAG_READ, Type);
		if(!File)
			return false;

		SHA256_CTX Sha256Ctxt;
		if(pSha256 != nullptr)
			sha256_init(&Sha256Ctxt);
		if(pCrc != nullptr)
			*pCrc = 0;
		unsigned char aBuffer[64 * 1024];
		while(true)
		{
			unsigned Bytes = io_read(File, aBuffer, sizeof(aBuffer));
			if(Bytes == 0)
				break;
			if(pSha256 != nullptr)
				sha256_update(&Sha256Ctxt, aBuffer, Bytes);
			if(pCrc != nullptr)
				*pCrc = crc32(*pCrc, aBuffer, Bytes);
		}
		if(pSha256 != nullptr)
			*pSha256 = sha256_finish(&Sha256Ctxt);

		io_close(File);
		return true;
	}

	struct CFindCBData
	{
		CStorage *m_pStorage;
		const char *m_pFilename;
		const char *m_pPath;
		char *m_pBuffer;
		int m_BufferSize;
	};

	static int FindFileCallback(const char *pName, int IsDir, int Type, void *pUser)
	{
		CFindCBData Data = *static_cast<CFindCBData *>(pUser);
		if(IsDir)
		{
			if(pName[0] == '.')
				return 0;

			// search within the folder
			char aBuf[IO_MAX_PATH_LENGTH];
			char aPath[IO_MAX_PATH_LENGTH];
			str_format(aPath, sizeof(aPath), "%s/%s", Data.m_pPath, pName);
			Data.m_pPath = aPath;
			fs_listdir(Data.m_pStorage->GetPath(Type, aPath, aBuf, sizeof(aBuf)), FindFileCallback, Type, &Data);
			if(Data.m_pBuffer[0])
				return 1;
		}
		else if(!str_comp(pName, Data.m_pFilename))
		{
			// found the file = end
			str_format(Data.m_pBuffer, Data.m_BufferSize, "%s/%s", Data.m_pPath, Data.m_pFilename);
			return 1;
		}

		return 0;
	}

	bool FindFile(const char *pFilename, const char *pPath, int Type, char *pBuffer, int BufferSize) override
	{
		dbg_assert(BufferSize >= 1, "BufferSize invalid");

		pBuffer[0] = 0;

		CFindCBData Data;
		Data.m_pStorage = this;
		Data.m_pFilename = pFilename;
		Data.m_pPath = pPath;
		Data.m_pBuffer = pBuffer;
		Data.m_BufferSize = BufferSize;

		char aBuf[IO_MAX_PATH_LENGTH];
		if(Type == TYPE_ALL)
		{
			// search within all available directories
			for(int i = TYPE_SAVE; i < m_NumPaths; ++i)
			{
				fs_listdir(GetPath(i, pPath, aBuf, sizeof(aBuf)), FindFileCallback, i, &Data);
				if(pBuffer[0])
					return true;
			}
		}
		else if(Type >= TYPE_SAVE && Type < m_NumPaths)
		{
			// search within wanted directory
			fs_listdir(GetPath(Type, pPath, aBuf, sizeof(aBuf)), FindFileCallback, Type, &Data);
		}
		else
		{
			dbg_assert(false, "Type invalid");
		}

		return pBuffer[0] != 0;
	}

	struct SFindFilesCallbackData
	{
		CStorage *m_pStorage;
		const char *m_pFilename;
		const char *m_pPath;
		std::set<std::string> *m_pEntries;
	};

	static int FindFilesCallback(const char *pName, int IsDir, int Type, void *pUser)
	{
		SFindFilesCallbackData Data = *static_cast<SFindFilesCallbackData *>(pUser);
		if(IsDir)
		{
			if(pName[0] == '.')
				return 0;

			// search within the folder
			char aBuf[IO_MAX_PATH_LENGTH];
			char aPath[IO_MAX_PATH_LENGTH];
			str_format(aPath, sizeof(aPath), "%s/%s", Data.m_pPath, pName);
			Data.m_pPath = aPath;
			fs_listdir(Data.m_pStorage->GetPath(Type, aPath, aBuf, sizeof(aBuf)), FindFilesCallback, Type, &Data);
		}
		else if(!str_comp(pName, Data.m_pFilename))
		{
			char aBuffer[IO_MAX_PATH_LENGTH];
			str_format(aBuffer, sizeof(aBuffer), "%s/%s", Data.m_pPath, Data.m_pFilename);
			Data.m_pEntries->emplace(aBuffer);
		}

		return 0;
	}

	size_t FindFiles(const char *pFilename, const char *pPath, int Type, std::set<std::string> *pEntries) override
	{
		SFindFilesCallbackData Data;
		Data.m_pStorage = this;
		Data.m_pFilename = pFilename;
		Data.m_pPath = pPath;
		Data.m_pEntries = pEntries;

		char aBuf[IO_MAX_PATH_LENGTH];
		if(Type == TYPE_ALL)
		{
			// search within all available directories
			for(int i = TYPE_SAVE; i < m_NumPaths; ++i)
			{
				fs_listdir(GetPath(i, pPath, aBuf, sizeof(aBuf)), FindFilesCallback, i, &Data);
			}
		}
		else if(Type >= TYPE_SAVE && Type < m_NumPaths)
		{
			// search within wanted directory
			fs_listdir(GetPath(Type, pPath, aBuf, sizeof(aBuf)), FindFilesCallback, Type, &Data);
		}
		else
		{
			dbg_assert(false, "Type invalid");
		}

		return pEntries->size();
	}

	bool RemoveFile(const char *pFilename, int Type) override
	{
		dbg_assert(Type == TYPE_ABSOLUTE || (Type >= TYPE_SAVE && Type < m_NumPaths), "Type invalid");

		char aBuffer[IO_MAX_PATH_LENGTH];
		GetPath(Type, pFilename, aBuffer, sizeof(aBuffer));

		bool Success = !fs_remove(aBuffer);
		if(!Success)
			dbg_msg("storage", "failed to remove: %s", aBuffer);
		return Success;
	}

	bool RemoveFolder(const char *pFilename, int Type) override
	{
		dbg_assert(Type == TYPE_ABSOLUTE || (Type >= TYPE_SAVE && Type < m_NumPaths), "Type invalid");

		char aBuffer[IO_MAX_PATH_LENGTH];
		GetPath(Type, pFilename, aBuffer, sizeof(aBuffer));

		bool Success = !fs_removedir(aBuffer);
		if(!Success)
			dbg_msg("storage", "failed to remove: %s", aBuffer);
		return Success;
	}

	bool RemoveBinaryFile(const char *pFilename) override
	{
		char aBuffer[IO_MAX_PATH_LENGTH];
		GetBinaryPath(pFilename, aBuffer, sizeof(aBuffer));

		bool Success = !fs_remove(aBuffer);
		if(!Success)
			dbg_msg("storage", "failed to remove binary: %s", aBuffer);
		return Success;
	}

	bool RenameFile(const char *pOldFilename, const char *pNewFilename, int Type) override
	{
		dbg_assert(Type >= TYPE_SAVE && Type < m_NumPaths, "Type invalid");

		char aOldBuffer[IO_MAX_PATH_LENGTH];
		char aNewBuffer[IO_MAX_PATH_LENGTH];
		GetPath(Type, pOldFilename, aOldBuffer, sizeof(aOldBuffer));
		GetPath(Type, pNewFilename, aNewBuffer, sizeof(aNewBuffer));

		bool Success = !fs_rename(aOldBuffer, aNewBuffer);
		if(!Success)
			dbg_msg("storage", "failed to rename: %s -> %s", aOldBuffer, aNewBuffer);
		return Success;
	}

	bool RenameBinaryFile(const char *pOldFilename, const char *pNewFilename) override
	{
		char aOldBuffer[IO_MAX_PATH_LENGTH];
		char aNewBuffer[IO_MAX_PATH_LENGTH];
		GetBinaryPath(pOldFilename, aOldBuffer, sizeof(aOldBuffer));
		GetBinaryPath(pNewFilename, aNewBuffer, sizeof(aNewBuffer));

		if(fs_makedir_rec_for(aNewBuffer) < 0)
		{
			dbg_msg("storage", "cannot create folder for: %s", aNewBuffer);
			return false;
		}

		bool Success = !fs_rename(aOldBuffer, aNewBuffer);
		if(!Success)
			dbg_msg("storage", "failed to rename: %s -> %s", aOldBuffer, aNewBuffer);
		return Success;
	}

	bool CreateFolder(const char *pFoldername, int Type) override
	{
		dbg_assert(Type >= TYPE_SAVE && Type < m_NumPaths, "Type invalid");

		char aBuffer[IO_MAX_PATH_LENGTH];
		GetPath(Type, pFoldername, aBuffer, sizeof(aBuffer));

		bool Success = !fs_makedir(aBuffer);
		if(!Success)
			dbg_msg("storage", "failed to create folder: %s", aBuffer);
		return Success;
	}

	void GetCompletePath(int Type, const char *pDir, char *pBuffer, unsigned BufferSize) override
	{
		TranslateType(Type, pDir);
		dbg_assert(Type >= TYPE_SAVE && Type < m_NumPaths, "Type invalid");
		GetPath(Type, pDir, pBuffer, BufferSize);
	}

	const char *GetBinaryPath(const char *pFilename, char *pBuffer, unsigned BufferSize) override
	{
		str_format(pBuffer, BufferSize, "%s%s%s", m_aBinarydir, !m_aBinarydir[0] ? "" : "/", pFilename);
		return pBuffer;
	}

	const char *GetBinaryPathAbsolute(const char *pFilename, char *pBuffer, unsigned BufferSize) override
	{
		char aBinaryPath[IO_MAX_PATH_LENGTH];
		GetBinaryPath(PLAT_CLIENT_EXEC, aBinaryPath, sizeof(aBinaryPath));
		if(fs_is_relative_path(aBinaryPath))
		{
			if(fs_getcwd(pBuffer, BufferSize))
			{
				str_append(pBuffer, "/", BufferSize);
				str_append(pBuffer, aBinaryPath, BufferSize);
			}
		}
		else
			str_copy(pBuffer, aBinaryPath, BufferSize);
		return pBuffer;
	}

	static IStorage *Create(int StorageType, int NumArgs, const char **ppArguments)
	{
		CStorage *pStorage = new CStorage();
		if(pStorage && pStorage->Init(StorageType, NumArgs, ppArguments))
		{
			dbg_msg("storage", "initialisation failed");
			delete pStorage;
			pStorage = nullptr;
		}
		return pStorage;
	}
};

void IStorage::StripPathAndExtension(const char *pFilename, char *pBuffer, int BufferSize)
{
	const char *pFilenameEnd = pFilename + str_length(pFilename);
	const char *pExtractedName = pFilename;
	const char *pEnd = pFilenameEnd;
	for(const char *pIter = pFilename; *pIter; pIter++)
	{
		if(*pIter == '/' || *pIter == '\\')
		{
			pExtractedName = pIter + 1;
			pEnd = pFilenameEnd;
		}
		else if(*pIter == '.')
		{
			pEnd = pIter;
		}
	}

	int Length = minimum(BufferSize, (int)(pEnd - pExtractedName + 1));
	str_copy(pBuffer, pExtractedName, Length);
}

const char *IStorage::FormatTmpPath(char *aBuf, unsigned BufSize, const char *pPath)
{
	str_format(aBuf, BufSize, "%s.%d.tmp", pPath, pid());
	return aBuf;
}

IStorage *CreateStorage(int StorageType, int NumArgs, const char **ppArguments)
{
	return CStorage::Create(StorageType, NumArgs, ppArguments);
}

IStorage *CreateLocalStorage()
{
	CStorage *pStorage = new CStorage();
	if(pStorage)
	{
		if(!fs_getcwd(pStorage->m_aCurrentdir, sizeof(pStorage->m_aCurrentdir)))
		{
			delete pStorage;
			return NULL;
		}
		pStorage->AddPath("$CURRENTDIR");
	}
	return pStorage;
}
IStorage *CreateTempStorage(const char *pDirectory)
{
	CStorage *pStorage = new CStorage();
	if(!pStorage)
	{
		return nullptr;
	}
	pStorage->AddPath(pDirectory);
	return pStorage;
}
