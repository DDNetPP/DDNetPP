/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */

#include "datafile.h"

#include <base/hash_ctxt.h>
#include <base/log.h>
#include <base/math.h>
#include <base/system.h>
#include <engine/storage.h>

#include "uuid_manager.h"

#include <cstdlib>

static const int DEBUG = 0;

enum
{
	OFFSET_UUID_TYPE = 0x8000,
};

struct CItemEx
{
	int m_aUuid[sizeof(CUuid) / 4];

	static CItemEx FromUuid(CUuid Uuid)
	{
		CItemEx Result;
		for(int i = 0; i < (int)sizeof(CUuid) / 4; i++)
			Result.m_aUuid[i] = bytes_be_to_int(&Uuid.m_aData[i * 4]);
		return Result;
	}

	CUuid ToUuid() const
	{
		CUuid Result;
		for(int i = 0; i < (int)sizeof(CUuid) / 4; i++)
			int_to_bytes_be(&Result.m_aData[i * 4], m_aUuid[i]);
		return Result;
	}
};

struct CDatafileItemType
{
	int m_Type;
	int m_Start;
	int m_Num;
};

struct CDatafileItem
{
	int m_TypeAndID;
	int m_Size;
};

struct CDatafileHeader
{
	char m_aID[4];
	int m_Version;
	int m_Size;
	int m_Swaplen;
	int m_NumItemTypes;
	int m_NumItems;
	int m_NumRawData;
	int m_ItemSize;
	int m_DataSize;
};

struct CDatafileInfo
{
	CDatafileItemType *m_pItemTypes;
	int *m_pItemOffsets;
	int *m_pDataOffsets;
	int *m_pDataSizes;

	char *m_pItemStart;
	char *m_pDataStart;
};

struct CDatafile
{
	IOHANDLE m_File;
	SHA256_DIGEST m_Sha256;
	unsigned m_Crc;
	CDatafileInfo m_Info;
	CDatafileHeader m_Header;
	int m_DataStartOffset;
	char **m_ppDataPtrs;
	char *m_pData;
};

bool CDataFileReader::Open(class IStorage *pStorage, const char *pFilename, int StorageType)
{
	log_trace("datafile", "loading. filename='%s'", pFilename);

	IOHANDLE File = pStorage->OpenFile(pFilename, IOFLAG_READ, StorageType);
	if(!File)
	{
		dbg_msg("datafile", "could not open '%s'", pFilename);
		return false;
	}

	// take the CRC of the file and store it
	unsigned Crc = 0;
	SHA256_DIGEST Sha256;
	{
		enum
		{
			BUFFER_SIZE = 64 * 1024
		};

		SHA256_CTX Sha256Ctxt;
		sha256_init(&Sha256Ctxt);
		unsigned char aBuffer[BUFFER_SIZE];

		while(true)
		{
			unsigned Bytes = io_read(File, aBuffer, BUFFER_SIZE);
			if(Bytes == 0)
				break;
			Crc = crc32(Crc, aBuffer, Bytes);
			sha256_update(&Sha256Ctxt, aBuffer, Bytes);
		}
		Sha256 = sha256_finish(&Sha256Ctxt);

		io_seek(File, 0, IOSEEK_START);
	}

	// TODO: change this header
	CDatafileHeader Header;
	if(sizeof(Header) != io_read(File, &Header, sizeof(Header)))
	{
		dbg_msg("datafile", "couldn't load header");
		return false;
	}
	if(Header.m_aID[0] != 'A' || Header.m_aID[1] != 'T' || Header.m_aID[2] != 'A' || Header.m_aID[3] != 'D')
	{
		if(Header.m_aID[0] != 'D' || Header.m_aID[1] != 'A' || Header.m_aID[2] != 'T' || Header.m_aID[3] != 'A')
		{
			dbg_msg("datafile", "wrong signature. %x %x %x %x", Header.m_aID[0], Header.m_aID[1], Header.m_aID[2], Header.m_aID[3]);
			return false;
		}
	}

#if defined(CONF_ARCH_ENDIAN_BIG)
	swap_endian(&Header, sizeof(int), sizeof(Header) / sizeof(int));
#endif
	if(Header.m_Version != 3 && Header.m_Version != 4)
	{
		dbg_msg("datafile", "wrong version. version=%x", Header.m_Version);
		return false;
	}

	// read in the rest except the data
	unsigned Size = 0;
	Size += Header.m_NumItemTypes * sizeof(CDatafileItemType);
	Size += (Header.m_NumItems + Header.m_NumRawData) * sizeof(int);
	if(Header.m_Version == 4)
		Size += Header.m_NumRawData * sizeof(int); // v4 has uncompressed data sizes as well
	Size += Header.m_ItemSize;

	unsigned AllocSize = Size;
	AllocSize += sizeof(CDatafile); // add space for info structure
	AllocSize += Header.m_NumRawData * sizeof(void *); // add space for data pointers

	CDatafile *pTmpDataFile = (CDatafile *)malloc(AllocSize);
	pTmpDataFile->m_Header = Header;
	pTmpDataFile->m_DataStartOffset = sizeof(CDatafileHeader) + Size;
	pTmpDataFile->m_ppDataPtrs = (char **)(pTmpDataFile + 1);
	pTmpDataFile->m_pData = (char *)(pTmpDataFile + 1) + Header.m_NumRawData * sizeof(char *);
	pTmpDataFile->m_File = File;
	pTmpDataFile->m_Sha256 = Sha256;
	pTmpDataFile->m_Crc = Crc;

	// clear the data pointers
	mem_zero(pTmpDataFile->m_ppDataPtrs, Header.m_NumRawData * sizeof(void *));

	// read types, offsets, sizes and item data
	unsigned ReadSize = io_read(File, pTmpDataFile->m_pData, Size);
	if(ReadSize != Size)
	{
		io_close(pTmpDataFile->m_File);
		free(pTmpDataFile);
		dbg_msg("datafile", "couldn't load the whole thing, wanted=%d got=%d", Size, ReadSize);
		return false;
	}

	Close();
	m_pDataFile = pTmpDataFile;

#if defined(CONF_ARCH_ENDIAN_BIG)
	swap_endian(m_pDataFile->m_pData, sizeof(int), minimum(static_cast<unsigned>(Header.m_Swaplen), Size) / sizeof(int));
#endif

	if(DEBUG)
	{
		dbg_msg("datafile", "allocsize=%d", AllocSize);
		dbg_msg("datafile", "readsize=%d", ReadSize);
		dbg_msg("datafile", "swaplen=%d", Header.m_Swaplen);
		dbg_msg("datafile", "item_size=%d", m_pDataFile->m_Header.m_ItemSize);
	}

	m_pDataFile->m_Info.m_pItemTypes = (CDatafileItemType *)m_pDataFile->m_pData;
	m_pDataFile->m_Info.m_pItemOffsets = (int *)&m_pDataFile->m_Info.m_pItemTypes[m_pDataFile->m_Header.m_NumItemTypes];
	m_pDataFile->m_Info.m_pDataOffsets = &m_pDataFile->m_Info.m_pItemOffsets[m_pDataFile->m_Header.m_NumItems];
	m_pDataFile->m_Info.m_pDataSizes = &m_pDataFile->m_Info.m_pDataOffsets[m_pDataFile->m_Header.m_NumRawData];

	if(Header.m_Version == 4)
		m_pDataFile->m_Info.m_pItemStart = (char *)&m_pDataFile->m_Info.m_pDataSizes[m_pDataFile->m_Header.m_NumRawData];
	else
		m_pDataFile->m_Info.m_pItemStart = (char *)&m_pDataFile->m_Info.m_pDataOffsets[m_pDataFile->m_Header.m_NumRawData];
	m_pDataFile->m_Info.m_pDataStart = m_pDataFile->m_Info.m_pItemStart + m_pDataFile->m_Header.m_ItemSize;

	log_trace("datafile", "loading done. datafile='%s'", pFilename);

	return true;
}

int CDataFileReader::NumData() const
{
	if(!m_pDataFile)
	{
		return 0;
	}
	return m_pDataFile->m_Header.m_NumRawData;
}

// returns the size in the file
int CDataFileReader::GetFileDataSize(int Index)
{
	if(!m_pDataFile)
	{
		return 0;
	}

	if(Index == m_pDataFile->m_Header.m_NumRawData - 1)
		return m_pDataFile->m_Header.m_DataSize - m_pDataFile->m_Info.m_pDataOffsets[Index];
	return m_pDataFile->m_Info.m_pDataOffsets[Index + 1] - m_pDataFile->m_Info.m_pDataOffsets[Index];
}

// returns the size of the resulting data
int CDataFileReader::GetDataSize(int Index)
{
	if(!m_pDataFile)
	{
		return 0;
	}

	if(m_pDataFile->m_Header.m_Version == 4)
		return m_pDataFile->m_Info.m_pDataSizes[Index];
	else
		return GetFileDataSize(Index);
}

void *CDataFileReader::GetDataImpl(int Index, int Swap)
{
	if(!m_pDataFile)
	{
		return 0;
	}

	if(Index < 0 || Index >= m_pDataFile->m_Header.m_NumRawData)
		return 0;

	// load it if needed
	if(!m_pDataFile->m_ppDataPtrs[Index])
	{
		// fetch the data size
		int DataSize = GetFileDataSize(Index);
#if defined(CONF_ARCH_ENDIAN_BIG)
		int SwapSize = DataSize;
#endif

		if(m_pDataFile->m_Header.m_Version == 4)
		{
			// v4 has compressed data
			void *pTemp = malloc(DataSize);
			unsigned long UncompressedSize = m_pDataFile->m_Info.m_pDataSizes[Index];
			unsigned long s;

			log_trace("datafile", "loading data index=%d size=%d uncompressed=%lu", Index, DataSize, UncompressedSize);
			m_pDataFile->m_ppDataPtrs[Index] = (char *)malloc(UncompressedSize);

			// read the compressed data
			io_seek(m_pDataFile->m_File, m_pDataFile->m_DataStartOffset + m_pDataFile->m_Info.m_pDataOffsets[Index], IOSEEK_START);
			io_read(m_pDataFile->m_File, pTemp, DataSize);

			// decompress the data, TODO: check for errors
			s = UncompressedSize;
			uncompress((Bytef *)m_pDataFile->m_ppDataPtrs[Index], &s, (Bytef *)pTemp, DataSize);
#if defined(CONF_ARCH_ENDIAN_BIG)
			SwapSize = s;
#endif

			// clean up the temporary buffers
			free(pTemp);
		}
		else
		{
			// load the data
			log_trace("datafile", "loading data index=%d size=%d", Index, DataSize);
			m_pDataFile->m_ppDataPtrs[Index] = (char *)malloc(DataSize);
			io_seek(m_pDataFile->m_File, m_pDataFile->m_DataStartOffset + m_pDataFile->m_Info.m_pDataOffsets[Index], IOSEEK_START);
			io_read(m_pDataFile->m_File, m_pDataFile->m_ppDataPtrs[Index], DataSize);
		}

#if defined(CONF_ARCH_ENDIAN_BIG)
		if(Swap && SwapSize)
			swap_endian(m_pDataFile->m_ppDataPtrs[Index], sizeof(int), SwapSize / sizeof(int));
#endif
	}

	return m_pDataFile->m_ppDataPtrs[Index];
}

void *CDataFileReader::GetData(int Index)
{
	return GetDataImpl(Index, 0);
}

void *CDataFileReader::GetDataSwapped(int Index)
{
	return GetDataImpl(Index, 1);
}

void CDataFileReader::UnloadData(int Index)
{
	if(Index < 0 || Index >= m_pDataFile->m_Header.m_NumRawData)
		return;

	//
	free(m_pDataFile->m_ppDataPtrs[Index]);
	m_pDataFile->m_ppDataPtrs[Index] = 0x0;
}

int CDataFileReader::GetItemSize(int Index) const
{
	if(!m_pDataFile)
		return 0;
	if(Index == m_pDataFile->m_Header.m_NumItems - 1)
		return m_pDataFile->m_Header.m_ItemSize - m_pDataFile->m_Info.m_pItemOffsets[Index] - sizeof(CDatafileItem);
	return m_pDataFile->m_Info.m_pItemOffsets[Index + 1] - m_pDataFile->m_Info.m_pItemOffsets[Index] - sizeof(CDatafileItem);
}

int CDataFileReader::GetExternalItemType(int InternalType)
{
	if(InternalType <= OFFSET_UUID_TYPE || InternalType == ITEMTYPE_EX)
	{
		return InternalType;
	}
	int TypeIndex = FindItemIndex(ITEMTYPE_EX, InternalType);
	if(TypeIndex < 0 || GetItemSize(TypeIndex) < (int)sizeof(CItemEx))
	{
		return InternalType;
	}
	const CItemEx *pItemEx = (const CItemEx *)GetItem(TypeIndex, 0, 0);
	// Propagate UUID_UNKNOWN, it doesn't hurt.
	return g_UuidManager.LookupUuid(pItemEx->ToUuid());
}

int CDataFileReader::GetInternalItemType(int ExternalType)
{
	if(ExternalType < OFFSET_UUID)
	{
		return ExternalType;
	}
	CUuid Uuid = g_UuidManager.GetUuid(ExternalType);
	int Start, Num;
	GetType(ITEMTYPE_EX, &Start, &Num);
	for(int i = Start; i < Start + Num; i++)
	{
		if(GetItemSize(i) < (int)sizeof(CItemEx))
		{
			continue;
		}
		int ID;
		if(Uuid == ((const CItemEx *)GetItem(i, 0, &ID))->ToUuid())
		{
			return ID;
		}
	}
	return -1;
}

void *CDataFileReader::GetItem(int Index, int *pType, int *pID)
{
	if(!m_pDataFile)
	{
		if(pType)
			*pType = 0;
		if(pID)
			*pID = 0;
		return 0;
	}

	CDatafileItem *pItem = (CDatafileItem *)(m_pDataFile->m_Info.m_pItemStart + m_pDataFile->m_Info.m_pItemOffsets[Index]);
	if(pType)
	{
		// remove sign extension
		*pType = GetExternalItemType((pItem->m_TypeAndID >> 16) & 0xffff);
	}
	if(pID)
	{
		*pID = pItem->m_TypeAndID & 0xffff;
	}
	return (void *)(pItem + 1);
}

void CDataFileReader::GetType(int Type, int *pStart, int *pNum)
{
	*pStart = 0;
	*pNum = 0;

	if(!m_pDataFile)
		return;

	Type = GetInternalItemType(Type);
	for(int i = 0; i < m_pDataFile->m_Header.m_NumItemTypes; i++)
	{
		if(m_pDataFile->m_Info.m_pItemTypes[i].m_Type == Type)
		{
			*pStart = m_pDataFile->m_Info.m_pItemTypes[i].m_Start;
			*pNum = m_pDataFile->m_Info.m_pItemTypes[i].m_Num;
			return;
		}
	}
}

int CDataFileReader::FindItemIndex(int Type, int ID)
{
	if(!m_pDataFile)
	{
		return -1;
	}

	int Start, Num;
	GetType(Type, &Start, &Num);
	for(int i = 0; i < Num; i++)
	{
		int ItemID;
		GetItem(Start + i, 0, &ItemID);
		if(ID == ItemID)
		{
			return Start + i;
		}
	}
	return -1;
}

void *CDataFileReader::FindItem(int Type, int ID)
{
	int Index = FindItemIndex(Type, ID);
	if(Index < 0)
	{
		return 0;
	}
	return GetItem(Index, 0, 0);
}

int CDataFileReader::NumItems() const
{
	if(!m_pDataFile)
		return 0;
	return m_pDataFile->m_Header.m_NumItems;
}

bool CDataFileReader::Close()
{
	if(!m_pDataFile)
		return true;

	// free the data that is loaded
	int i;
	for(i = 0; i < m_pDataFile->m_Header.m_NumRawData; i++)
		free(m_pDataFile->m_ppDataPtrs[i]);

	io_close(m_pDataFile->m_File);
	free(m_pDataFile);
	m_pDataFile = 0;
	return true;
}

SHA256_DIGEST CDataFileReader::Sha256() const
{
	if(!m_pDataFile)
	{
		SHA256_DIGEST Result;
		for(unsigned char &d : Result.data)
		{
			d = 0xff;
		}
		return Result;
	}
	return m_pDataFile->m_Sha256;
}

unsigned CDataFileReader::Crc() const
{
	if(!m_pDataFile)
		return 0xFFFFFFFF;
	return m_pDataFile->m_Crc;
}

int CDataFileReader::MapSize() const
{
	if(!m_pDataFile)
		return 0;
	return m_pDataFile->m_Header.m_Size + 16;
}

IOHANDLE CDataFileReader::File()
{
	if(!m_pDataFile)
		return 0;
	return m_pDataFile->m_File;
}

CDataFileWriter::CDataFileWriter()
{
	m_File = 0;
	m_pItemTypes = static_cast<CItemTypeInfo *>(calloc(MAX_ITEM_TYPES, sizeof(CItemTypeInfo)));
	m_pItems = static_cast<CItemInfo *>(calloc(MAX_ITEMS, sizeof(CItemInfo)));
	m_pDatas = static_cast<CDataInfo *>(calloc(MAX_DATAS, sizeof(CDataInfo)));
}

CDataFileWriter::~CDataFileWriter()
{
	free(m_pItemTypes);
	m_pItemTypes = 0;
	for(int i = 0; i < m_NumItems; i++)
		free(m_pItems[i].m_pData);
	for(int i = 0; i < m_NumDatas; ++i)
		free(m_pDatas[i].m_pCompressedData);
	free(m_pItems);
	m_pItems = 0;
	free(m_pDatas);
	m_pDatas = 0;
}

bool CDataFileWriter::OpenFile(class IStorage *pStorage, const char *pFilename, int StorageType)
{
	dbg_assert(!m_File, "a file already exists");
	m_File = pStorage->OpenFile(pFilename, IOFLAG_WRITE, StorageType);
	return m_File != 0;
}

void CDataFileWriter::Init()
{
	dbg_assert(!m_File, "a file already exists");
	m_NumItems = 0;
	m_NumDatas = 0;
	m_NumItemTypes = 0;
	m_NumExtendedItemTypes = 0;
	mem_zero(m_pItemTypes, sizeof(CItemTypeInfo) * MAX_ITEM_TYPES);
	mem_zero(m_aExtendedItemTypes, sizeof(m_aExtendedItemTypes));

	for(int i = 0; i < MAX_ITEM_TYPES; i++)
	{
		m_pItemTypes[i].m_First = -1;
		m_pItemTypes[i].m_Last = -1;
	}
}

bool CDataFileWriter::Open(class IStorage *pStorage, const char *pFilename, int StorageType)
{
	Init();
	return OpenFile(pStorage, pFilename, StorageType);
}

int CDataFileWriter::GetTypeFromIndex(int Index)
{
	return ITEMTYPE_EX - Index - 1;
}

int CDataFileWriter::GetExtendedItemTypeIndex(int Type)
{
	for(int i = 0; i < m_NumExtendedItemTypes; i++)
	{
		if(m_aExtendedItemTypes[i] == Type)
		{
			return i;
		}
	}

	// Type not found, add it.
	dbg_assert(m_NumExtendedItemTypes < MAX_EXTENDED_ITEM_TYPES, "too many extended item types");
	int Index = m_NumExtendedItemTypes++;
	m_aExtendedItemTypes[Index] = Type;

	CItemEx ExtendedType = CItemEx::FromUuid(g_UuidManager.GetUuid(Type));
	AddItem(ITEMTYPE_EX, GetTypeFromIndex(Index), sizeof(ExtendedType), &ExtendedType);
	return Index;
}

int CDataFileWriter::AddItem(int Type, int ID, int Size, void *pData)
{
	dbg_assert((Type >= 0 && Type < MAX_ITEM_TYPES) || Type >= OFFSET_UUID, "incorrect type");
	dbg_assert(m_NumItems < 1024, "too many items");
	dbg_assert(Size % sizeof(int) == 0, "incorrect boundary");

	if(Type >= OFFSET_UUID)
	{
		Type = GetTypeFromIndex(GetExtendedItemTypeIndex(Type));
	}

	m_pItems[m_NumItems].m_Type = Type;
	m_pItems[m_NumItems].m_ID = ID;
	m_pItems[m_NumItems].m_Size = Size;

	// copy data
	m_pItems[m_NumItems].m_pData = malloc(Size);
	mem_copy(m_pItems[m_NumItems].m_pData, pData, Size);

	if(!m_pItemTypes[Type].m_Num) // count item types
		m_NumItemTypes++;

	// link
	m_pItems[m_NumItems].m_Prev = m_pItemTypes[Type].m_Last;
	m_pItems[m_NumItems].m_Next = -1;

	if(m_pItemTypes[Type].m_Last != -1)
		m_pItems[m_pItemTypes[Type].m_Last].m_Next = m_NumItems;
	m_pItemTypes[Type].m_Last = m_NumItems;

	if(m_pItemTypes[Type].m_First == -1)
		m_pItemTypes[Type].m_First = m_NumItems;

	m_pItemTypes[Type].m_Num++;

	m_NumItems++;
	return m_NumItems - 1;
}

int CDataFileWriter::AddData(int Size, void *pData, int CompressionLevel)
{
	dbg_assert(m_NumDatas < 1024, "too much data");

	CDataInfo *pInfo = &m_pDatas[m_NumDatas];
	unsigned long s = compressBound(Size);
	void *pCompData = malloc(s); // temporary buffer that we use during compression

	int Result = compress2((Bytef *)pCompData, &s, (Bytef *)pData, Size, CompressionLevel);
	if(Result != Z_OK)
	{
		dbg_msg("datafile", "compression error %d", Result);
		dbg_assert(0, "zlib error");
	}

	pInfo->m_UncompressedSize = Size;
	pInfo->m_CompressedSize = (int)s;
	pInfo->m_pCompressedData = malloc(pInfo->m_CompressedSize);
	mem_copy(pInfo->m_pCompressedData, pCompData, pInfo->m_CompressedSize);
	free(pCompData);

	m_NumDatas++;
	return m_NumDatas - 1;
}

int CDataFileWriter::AddDataSwapped(int Size, void *pData)
{
	dbg_assert(Size % sizeof(int) == 0, "incorrect boundary");

#if defined(CONF_ARCH_ENDIAN_BIG)
	void *pSwapped = malloc(Size); // temporary buffer that we use during compression
	mem_copy(pSwapped, pData, Size);
	swap_endian(pSwapped, sizeof(int), Size / sizeof(int));
	int Index = AddData(Size, pSwapped);
	free(pSwapped);
	return Index;
#else
	return AddData(Size, pData);
#endif
}

int CDataFileWriter::Finish()
{
	if(!m_File)
		return 1;

	int ItemSize = 0;
	int TypesSize, HeaderSize, OffsetSize, FileSize, SwapSize;
	int DataSize = 0;
	CDatafileHeader Header;

	// we should now write this file!
	if(DEBUG)
		dbg_msg("datafile", "writing");

	// calculate sizes
	for(int i = 0; i < m_NumItems; i++)
	{
		if(DEBUG)
			dbg_msg("datafile", "item=%d size=%d (%d)", i, m_pItems[i].m_Size, m_pItems[i].m_Size + (int)sizeof(CDatafileItem));
		ItemSize += m_pItems[i].m_Size + sizeof(CDatafileItem);
	}

	for(int i = 0; i < m_NumDatas; i++)
		DataSize += m_pDatas[i].m_CompressedSize;

	// calculate the complete size
	TypesSize = m_NumItemTypes * sizeof(CDatafileItemType);
	HeaderSize = sizeof(CDatafileHeader);
	OffsetSize = (m_NumItems + m_NumDatas + m_NumDatas) * sizeof(int); // ItemOffsets, DataOffsets, DataUncompressedSizes
	FileSize = HeaderSize + TypesSize + OffsetSize + ItemSize + DataSize;
	SwapSize = FileSize - DataSize;

	(void)SwapSize;

	if(DEBUG)
		dbg_msg("datafile", "num_m_aItemTypes=%d TypesSize=%d m_aItemsize=%d DataSize=%d", m_NumItemTypes, TypesSize, ItemSize, DataSize);

	// construct Header
	{
		Header.m_aID[0] = 'D';
		Header.m_aID[1] = 'A';
		Header.m_aID[2] = 'T';
		Header.m_aID[3] = 'A';
		Header.m_Version = 4;
		Header.m_Size = FileSize - 16;
		Header.m_Swaplen = SwapSize - 16;
		Header.m_NumItemTypes = m_NumItemTypes;
		Header.m_NumItems = m_NumItems;
		Header.m_NumRawData = m_NumDatas;
		Header.m_ItemSize = ItemSize;
		Header.m_DataSize = DataSize;

		// write Header
		if(DEBUG)
			dbg_msg("datafile", "HeaderSize=%d", (int)sizeof(Header));
#if defined(CONF_ARCH_ENDIAN_BIG)
		swap_endian(&Header, sizeof(int), sizeof(Header) / sizeof(int));
#endif
		io_write(m_File, &Header, sizeof(Header));
	}

	// write types
	for(int i = 0, Count = 0; i < MAX_ITEM_TYPES; i++)
	{
		if(m_pItemTypes[i].m_Num)
		{
			// write info
			CDatafileItemType Info;
			Info.m_Type = i;
			Info.m_Start = Count;
			Info.m_Num = m_pItemTypes[i].m_Num;
			if(DEBUG)
				dbg_msg("datafile", "writing type=%x start=%d num=%d", Info.m_Type, Info.m_Start, Info.m_Num);
#if defined(CONF_ARCH_ENDIAN_BIG)
			swap_endian(&Info, sizeof(int), sizeof(CDatafileItemType) / sizeof(int));
#endif
			io_write(m_File, &Info, sizeof(Info));
			Count += m_pItemTypes[i].m_Num;
		}
	}

	// write item offsets
	for(int i = 0, Offset = 0; i < MAX_ITEM_TYPES; i++)
	{
		if(m_pItemTypes[i].m_Num)
		{
			// write all m_pItems in of this type
			int k = m_pItemTypes[i].m_First;
			while(k != -1)
			{
				if(DEBUG)
					dbg_msg("datafile", "writing item offset num=%d offset=%d", k, Offset);
				int Temp = Offset;
#if defined(CONF_ARCH_ENDIAN_BIG)
				swap_endian(&Temp, sizeof(int), sizeof(Temp) / sizeof(int));
#endif
				io_write(m_File, &Temp, sizeof(Temp));
				Offset += m_pItems[k].m_Size + sizeof(CDatafileItem);

				// next
				k = m_pItems[k].m_Next;
			}
		}
	}

	// write data offsets
	for(int i = 0, Offset = 0; i < m_NumDatas; i++)
	{
		if(DEBUG)
			dbg_msg("datafile", "writing data offset num=%d offset=%d", i, Offset);
		int Temp = Offset;
#if defined(CONF_ARCH_ENDIAN_BIG)
		swap_endian(&Temp, sizeof(int), sizeof(Temp) / sizeof(int));
#endif
		io_write(m_File, &Temp, sizeof(Temp));
		Offset += m_pDatas[i].m_CompressedSize;
	}

	// write data uncompressed sizes
	for(int i = 0; i < m_NumDatas; i++)
	{
		if(DEBUG)
			dbg_msg("datafile", "writing data uncompressed size num=%d size=%d", i, m_pDatas[i].m_UncompressedSize);
		int UncompressedSize = m_pDatas[i].m_UncompressedSize;
#if defined(CONF_ARCH_ENDIAN_BIG)
		swap_endian(&UncompressedSize, sizeof(int), sizeof(UncompressedSize) / sizeof(int));
#endif
		io_write(m_File, &UncompressedSize, sizeof(UncompressedSize));
	}

	// write m_pItems
	for(int i = 0; i < MAX_ITEM_TYPES; i++)
	{
		if(m_pItemTypes[i].m_Num)
		{
			// write all m_pItems in of this type
			int k = m_pItemTypes[i].m_First;
			while(k != -1)
			{
				CDatafileItem Item;
				Item.m_TypeAndID = (i << 16) | m_pItems[k].m_ID;
				Item.m_Size = m_pItems[k].m_Size;
				if(DEBUG)
					dbg_msg("datafile", "writing item type=%x idx=%d id=%d size=%d", i, k, m_pItems[k].m_ID, m_pItems[k].m_Size);

#if defined(CONF_ARCH_ENDIAN_BIG)
				swap_endian(&Item, sizeof(int), sizeof(Item) / sizeof(int));
				swap_endian(m_pItems[k].m_pData, sizeof(int), m_pItems[k].m_Size / sizeof(int));
#endif
				io_write(m_File, &Item, sizeof(Item));
				io_write(m_File, m_pItems[k].m_pData, m_pItems[k].m_Size);

				// next
				k = m_pItems[k].m_Next;
			}
		}
	}

	// write data
	for(int i = 0; i < m_NumDatas; i++)
	{
		if(DEBUG)
			dbg_msg("datafile", "writing data id=%d size=%d", i, m_pDatas[i].m_CompressedSize);
		io_write(m_File, m_pDatas[i].m_pCompressedData, m_pDatas[i].m_CompressedSize);
	}

	// free data
	for(int i = 0; i < m_NumItems; i++)
	{
		free(m_pItems[i].m_pData);
		m_pItems[i].m_pData = 0;
	}
	for(int i = 0; i < m_NumDatas; ++i)
	{
		free(m_pDatas[i].m_pCompressedData);
		m_pDatas[i].m_pCompressedData = 0;
	}

	io_close(m_File);
	m_File = 0;

	if(DEBUG)
		dbg_msg("datafile", "done");
	return 0;
}
