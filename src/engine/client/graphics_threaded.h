#ifndef ENGINE_CLIENT_GRAPHICS_THREADED_H
#define ENGINE_CLIENT_GRAPHICS_THREADED_H

#include <engine/graphics.h>
#include <engine/shared/config.h>

#include <cstddef>
#include <string>
#include <vector>

constexpr int CMD_BUFFER_DATA_BUFFER_SIZE = 1024 * 1024 * 2;
constexpr int CMD_BUFFER_CMD_BUFFER_SIZE = 1024 * 256;

class CCommandBuffer
{
	class CBuffer
	{
		unsigned char *m_pData;
		unsigned m_Size;
		unsigned m_Used;

	public:
		CBuffer(unsigned BufferSize)
		{
			m_Size = BufferSize;
			m_pData = new unsigned char[m_Size];
			m_Used = 0;
		}

		~CBuffer()
		{
			delete[] m_pData;
			m_pData = 0x0;
			m_Used = 0;
			m_Size = 0;
		}

		void Reset()
		{
			m_Used = 0;
		}

		void *Alloc(unsigned Requested, unsigned Alignment = alignof(std::max_align_t))
		{
			size_t Offset = reinterpret_cast<uintptr_t>(m_pData + m_Used) % Alignment;
			if(Offset)
				Offset = Alignment - Offset;

			if(Requested + Offset + m_Used > m_Size)
				return 0;

			void *pPtr = &m_pData[m_Used + Offset];
			m_Used += Requested + Offset;
			return pPtr;
		}

		unsigned char *DataPtr() { return m_pData; }
		unsigned DataSize() const { return m_Size; }
		unsigned DataUsed() const { return m_Used; }
	};

public:
	CBuffer m_CmdBuffer;
	size_t m_CommandCount = 0;
	size_t m_RenderCallCount = 0;

	CBuffer m_DataBuffer;

	enum
	{
		MAX_TEXTURES = 1024 * 8,
		MAX_VERTICES = 32 * 1024,
	};

	enum ECommandBufferCMD
	{
		// command groups
		CMDGROUP_CORE = 0, // commands that everyone has to implement
		CMDGROUP_PLATFORM_GL = 10000, // commands specific to a platform
		CMDGROUP_PLATFORM_SDL = 20000,

		//
		CMD_FIRST = CMDGROUP_CORE,
		CMD_NOP = CMD_FIRST,

		//
		CMD_RUNBUFFER,

		// synchronization
		CMD_SIGNAL,

		// texture commands
		CMD_TEXTURE_CREATE,
		CMD_TEXTURE_DESTROY,
		CMD_TEXTURE_UPDATE,
		CMD_TEXT_TEXTURES_CREATE,
		CMD_TEXT_TEXTURES_DESTROY,
		CMD_TEXT_TEXTURE_UPDATE,

		// rendering
		CMD_CLEAR,
		CMD_RENDER,
		CMD_RENDER_TEX3D,

		// opengl 2.0+ commands (some are just emulated and only exist in opengl 3.3+)
		CMD_CREATE_BUFFER_OBJECT, // create vbo
		CMD_RECREATE_BUFFER_OBJECT, // recreate vbo
		CMD_UPDATE_BUFFER_OBJECT, // update vbo
		CMD_COPY_BUFFER_OBJECT, // copy vbo to another
		CMD_DELETE_BUFFER_OBJECT, // delete vbo

		CMD_CREATE_BUFFER_CONTAINER, // create vao
		CMD_DELETE_BUFFER_CONTAINER, // delete vao
		CMD_UPDATE_BUFFER_CONTAINER, // update vao

		CMD_INDICES_REQUIRED_NUM_NOTIFY, // create indices that are required

		CMD_RENDER_TILE_LAYER, // render a tilelayer
		CMD_RENDER_BORDER_TILE, // render one tile multiple times
		CMD_RENDER_BORDER_TILE_LINE, // render an amount of tiles multiple times
		CMD_RENDER_QUAD_LAYER, // render a quad layer
		CMD_RENDER_TEXT, // render text
		CMD_RENDER_QUAD_CONTAINER, // render a quad buffer container
		CMD_RENDER_QUAD_CONTAINER_EX, // render a quad buffer container with extended parameters
		CMD_RENDER_QUAD_CONTAINER_SPRITE_MULTIPLE, // render a quad buffer container as sprite multiple times

		// swap
		CMD_SWAP,
		CMD_FINISH,

		// misc
		CMD_MULTISAMPLING,
		CMD_VSYNC,
		CMD_TRY_SWAP_AND_SCREENSHOT,
		CMD_UPDATE_VIEWPORT,

		// in Android a window that minimizes gets destroyed
		CMD_WINDOW_CREATE_NTF,
		CMD_WINDOW_DESTROY_NTF,

		CMD_COUNT,
	};

	enum
	{
		TEXFORMAT_INVALID = 0,
		TEXFORMAT_RGBA,

		TEXFLAG_NOMIPMAPS = 1,
		TEXFLAG_TO_3D_TEXTURE = (1 << 3),
		TEXFLAG_TO_2D_ARRAY_TEXTURE = (1 << 4),
		TEXFLAG_TO_3D_TEXTURE_SINGLE_LAYER = (1 << 5),
		TEXFLAG_TO_2D_ARRAY_TEXTURE_SINGLE_LAYER = (1 << 6),
		TEXFLAG_NO_2D_TEXTURE = (1 << 7),
	};

	enum
	{
		//
		PRIMTYPE_INVALID = 0,
		PRIMTYPE_LINES,
		PRIMTYPE_QUADS,
		PRIMTYPE_TRIANGLES,
	};

	enum
	{
		BLEND_NONE = 0,
		BLEND_ALPHA,
		BLEND_ADDITIVE,
	};

	enum
	{
		WRAP_REPEAT = 0,
		WRAP_CLAMP,
	};

	typedef vec2 SPoint;
	typedef vec2 STexCoord;
	typedef GL_SColorf SColorf;
	typedef GL_SColor SColor;
	typedef GL_SVertex SVertex;
	typedef GL_SVertexTex3D SVertexTex3D;
	typedef GL_SVertexTex3DStream SVertexTex3DStream;

	struct SCommand
	{
	public:
		SCommand(unsigned Cmd) :
			m_Cmd(Cmd), m_pNext(nullptr) {}
		unsigned m_Cmd;
		SCommand *m_pNext;
	};
	SCommand *m_pCmdBufferHead;
	SCommand *m_pCmdBufferTail;

	struct SState
	{
		int m_BlendMode;
		int m_WrapMode;
		int m_Texture;
		SPoint m_ScreenTL;
		SPoint m_ScreenBR;

		// clip
		bool m_ClipEnable;
		int m_ClipX;
		int m_ClipY;
		int m_ClipW;
		int m_ClipH;
	};

	struct SCommand_Clear : public SCommand
	{
		SCommand_Clear() :
			SCommand(CMD_CLEAR) {}
		SColorf m_Color;
		bool m_ForceClear;
	};

	struct SCommand_Signal : public SCommand
	{
		SCommand_Signal() :
			SCommand(CMD_SIGNAL) {}
		CSemaphore *m_pSemaphore;
	};

	struct SCommand_RunBuffer : public SCommand
	{
		SCommand_RunBuffer() :
			SCommand(CMD_RUNBUFFER) {}
		CCommandBuffer *m_pOtherBuffer;
	};

	struct SCommand_Render : public SCommand
	{
		SCommand_Render() :
			SCommand(CMD_RENDER) {}
		SState m_State;
		unsigned m_PrimType;
		unsigned m_PrimCount;
		SVertex *m_pVertices; // you should use the command buffer data to allocate vertices for this command
	};

	struct SCommand_RenderTex3D : public SCommand
	{
		SCommand_RenderTex3D() :
			SCommand(CMD_RENDER_TEX3D) {}
		SState m_State;
		unsigned m_PrimType;
		unsigned m_PrimCount;
		SVertexTex3DStream *m_pVertices; // you should use the command buffer data to allocate vertices for this command
	};

	struct SCommand_CreateBufferObject : public SCommand
	{
		SCommand_CreateBufferObject() :
			SCommand(CMD_CREATE_BUFFER_OBJECT) {}

		int m_BufferIndex;

		bool m_DeletePointer;
		void *m_pUploadData;
		size_t m_DataSize;

		int m_Flags; // @see EBufferObjectCreateFlags
	};

	struct SCommand_RecreateBufferObject : public SCommand
	{
		SCommand_RecreateBufferObject() :
			SCommand(CMD_RECREATE_BUFFER_OBJECT) {}

		int m_BufferIndex;

		bool m_DeletePointer;
		void *m_pUploadData;
		size_t m_DataSize;

		int m_Flags; // @see EBufferObjectCreateFlags
	};

	struct SCommand_UpdateBufferObject : public SCommand
	{
		SCommand_UpdateBufferObject() :
			SCommand(CMD_UPDATE_BUFFER_OBJECT) {}

		int m_BufferIndex;

		bool m_DeletePointer;
		void *m_pOffset;
		void *m_pUploadData;
		size_t m_DataSize;
	};

	struct SCommand_CopyBufferObject : public SCommand
	{
		SCommand_CopyBufferObject() :
			SCommand(CMD_COPY_BUFFER_OBJECT) {}

		int m_WriteBufferIndex;
		int m_ReadBufferIndex;

		size_t m_ReadOffset;
		size_t m_WriteOffset;
		size_t m_CopySize;
	};

	struct SCommand_DeleteBufferObject : public SCommand
	{
		SCommand_DeleteBufferObject() :
			SCommand(CMD_DELETE_BUFFER_OBJECT) {}

		int m_BufferIndex;
	};

	struct SCommand_CreateBufferContainer : public SCommand
	{
		SCommand_CreateBufferContainer() :
			SCommand(CMD_CREATE_BUFFER_CONTAINER) {}

		int m_BufferContainerIndex;

		int m_Stride;
		int m_VertBufferBindingIndex;

		int m_AttrCount;
		SBufferContainerInfo::SAttribute *m_pAttributes;
	};

	struct SCommand_UpdateBufferContainer : public SCommand
	{
		SCommand_UpdateBufferContainer() :
			SCommand(CMD_UPDATE_BUFFER_CONTAINER) {}

		int m_BufferContainerIndex;

		int m_Stride;
		int m_VertBufferBindingIndex;

		int m_AttrCount;
		SBufferContainerInfo::SAttribute *m_pAttributes;
	};

	struct SCommand_DeleteBufferContainer : public SCommand
	{
		SCommand_DeleteBufferContainer() :
			SCommand(CMD_DELETE_BUFFER_CONTAINER) {}

		int m_BufferContainerIndex;
		bool m_DestroyAllBO;
	};

	struct SCommand_IndicesRequiredNumNotify : public SCommand
	{
		SCommand_IndicesRequiredNumNotify() :
			SCommand(CMD_INDICES_REQUIRED_NUM_NOTIFY) {}

		unsigned int m_RequiredIndicesNum;
	};

	struct SCommand_RenderTileLayer : public SCommand
	{
		SCommand_RenderTileLayer() :
			SCommand(CMD_RENDER_TILE_LAYER) {}
		SState m_State;
		SColorf m_Color; // the color of the whole tilelayer -- already enveloped

		// the char offset of all indices that should be rendered, and the amount of renders
		char **m_pIndicesOffsets;
		unsigned int *m_pDrawCount;

		int m_IndicesDrawNum;
		int m_BufferContainerIndex;
	};

	struct SCommand_RenderBorderTile : public SCommand
	{
		SCommand_RenderBorderTile() :
			SCommand(CMD_RENDER_BORDER_TILE) {}
		SState m_State;
		SColorf m_Color; // the color of the whole tilelayer -- already enveloped
		char *m_pIndicesOffset; // you should use the command buffer data to allocate vertices for this command
		unsigned int m_DrawNum;
		int m_BufferContainerIndex;

		vec2 m_Offset;
		vec2 m_Dir;
		int m_JumpIndex;
	};

	struct SCommand_RenderBorderTileLine : public SCommand
	{
		SCommand_RenderBorderTileLine() :
			SCommand(CMD_RENDER_BORDER_TILE_LINE) {}
		SState m_State;
		SColorf m_Color; // the color of the whole tilelayer -- already enveloped
		char *m_pIndicesOffset; // you should use the command buffer data to allocate vertices for this command
		unsigned int m_IndexDrawNum;
		unsigned int m_DrawNum;
		int m_BufferContainerIndex;

		vec2 m_Offset;
		vec2 m_Dir;
	};

	struct SCommand_RenderQuadLayer : public SCommand
	{
		SCommand_RenderQuadLayer() :
			SCommand(CMD_RENDER_QUAD_LAYER) {}
		SState m_State;

		int m_BufferContainerIndex;
		SQuadRenderInfo *m_pQuadInfo;
		int m_QuadNum;
		int m_QuadOffset;
	};

	struct SCommand_RenderText : public SCommand
	{
		SCommand_RenderText() :
			SCommand(CMD_RENDER_TEXT) {}
		SState m_State;

		int m_BufferContainerIndex;
		int m_TextureSize;

		int m_TextTextureIndex;
		int m_TextOutlineTextureIndex;

		int m_DrawNum;
		ColorRGBA m_TextColor;
		ColorRGBA m_TextOutlineColor;
	};

	struct SCommand_RenderQuadContainer : public SCommand
	{
		SCommand_RenderQuadContainer() :
			SCommand(CMD_RENDER_QUAD_CONTAINER) {}
		SState m_State;

		int m_BufferContainerIndex;

		unsigned int m_DrawNum;
		void *m_pOffset;
	};

	struct SCommand_RenderQuadContainerEx : public SCommand
	{
		SCommand_RenderQuadContainerEx() :
			SCommand(CMD_RENDER_QUAD_CONTAINER_EX) {}
		SState m_State;

		int m_BufferContainerIndex;

		float m_Rotation;
		SPoint m_Center;

		SColorf m_VertexColor;

		unsigned int m_DrawNum;
		void *m_pOffset;
	};

	struct SCommand_RenderQuadContainerAsSpriteMultiple : public SCommand
	{
		SCommand_RenderQuadContainerAsSpriteMultiple() :
			SCommand(CMD_RENDER_QUAD_CONTAINER_SPRITE_MULTIPLE) {}
		SState m_State;

		int m_BufferContainerIndex;

		IGraphics::SRenderSpriteInfo *m_pRenderInfo;

		SPoint m_Center;
		SColorf m_VertexColor;

		unsigned int m_DrawNum;
		unsigned int m_DrawCount;
		void *m_pOffset;
	};

	struct SCommand_TrySwapAndScreenshot : public SCommand
	{
		SCommand_TrySwapAndScreenshot() :
			SCommand(CMD_TRY_SWAP_AND_SCREENSHOT) {}
		CImageInfo *m_pImage; // processor will fill this out, the one who adds this command must free the data as well
		bool *m_pSwapped;
	};

	struct SCommand_Swap : public SCommand
	{
		SCommand_Swap() :
			SCommand(CMD_SWAP) {}
	};

	struct SCommand_Finish : public SCommand
	{
		SCommand_Finish() :
			SCommand(CMD_FINISH) {}
	};

	struct SCommand_VSync : public SCommand
	{
		SCommand_VSync() :
			SCommand(CMD_VSYNC) {}

		int m_VSync;
		bool *m_pRetOk;
	};

	struct SCommand_MultiSampling : public SCommand
	{
		SCommand_MultiSampling() :
			SCommand(CMD_MULTISAMPLING) {}

		uint32_t m_RequestedMultiSamplingCount;
		uint32_t *m_pRetMultiSamplingCount;
		bool *m_pRetOk;
	};

	struct SCommand_Update_Viewport : public SCommand
	{
		SCommand_Update_Viewport() :
			SCommand(CMD_UPDATE_VIEWPORT) {}

		int m_X;
		int m_Y;
		int m_Width;
		int m_Height;
		bool m_ByResize; // resized by an resize event.. a hint to make clear that the viewport update can be deferred if wanted
	};

	struct SCommand_Texture_Create : public SCommand
	{
		SCommand_Texture_Create() :
			SCommand(CMD_TEXTURE_CREATE) {}

		// texture information
		int m_Slot;

		int m_Width;
		int m_Height;
		int m_PixelSize;
		int m_Format;
		int m_StoreFormat;
		int m_Flags;
		void *m_pData; // will be freed by the command processor
	};

	struct SCommand_Texture_Update : public SCommand
	{
		SCommand_Texture_Update() :
			SCommand(CMD_TEXTURE_UPDATE) {}

		// texture information
		int m_Slot;

		int m_X;
		int m_Y;
		int m_Width;
		int m_Height;
		int m_Format;
		void *m_pData; // will be freed by the command processor
	};

	struct SCommand_Texture_Destroy : public SCommand
	{
		SCommand_Texture_Destroy() :
			SCommand(CMD_TEXTURE_DESTROY) {}

		// texture information
		int m_Slot;
	};

	struct SCommand_TextTextures_Create : public SCommand
	{
		SCommand_TextTextures_Create() :
			SCommand(CMD_TEXT_TEXTURES_CREATE) {}

		// texture information
		int m_Slot;
		int m_SlotOutline;

		int m_Width;
		int m_Height;

		void *m_pTextData;
		void *m_pTextOutlineData;
	};

	struct SCommand_TextTextures_Destroy : public SCommand
	{
		SCommand_TextTextures_Destroy() :
			SCommand(CMD_TEXT_TEXTURES_DESTROY) {}

		// texture information
		int m_Slot;
		int m_SlotOutline;
	};

	struct SCommand_TextTexture_Update : public SCommand
	{
		SCommand_TextTexture_Update() :
			SCommand(CMD_TEXT_TEXTURE_UPDATE) {}

		// texture information
		int m_Slot;

		int m_X;
		int m_Y;
		int m_Width;
		int m_Height;
		void *m_pData; // will be freed by the command processor
	};

	struct SCommand_WindowCreateNtf : public CCommandBuffer::SCommand
	{
		SCommand_WindowCreateNtf() :
			SCommand(CMD_WINDOW_CREATE_NTF) {}

		uint32_t m_WindowID;
	};

	struct SCommand_WindowDestroyNtf : public CCommandBuffer::SCommand
	{
		SCommand_WindowDestroyNtf() :
			SCommand(CMD_WINDOW_DESTROY_NTF) {}

		uint32_t m_WindowID;
	};

	//
	CCommandBuffer(unsigned CmdBufferSize, unsigned DataBufferSize) :
		m_CmdBuffer(CmdBufferSize), m_DataBuffer(DataBufferSize), m_pCmdBufferHead(nullptr), m_pCmdBufferTail(nullptr)
	{
	}

	void *AllocData(unsigned WantedSize)
	{
		return m_DataBuffer.Alloc(WantedSize);
	}

	template<class T>
	bool AddCommandUnsafe(const T &Command)
	{
		// make sure that we don't do something stupid like ->AddCommand(&Cmd);
		(void)static_cast<const SCommand *>(&Command);

		// allocate and copy the command into the buffer
		T *pCmd = (T *)m_CmdBuffer.Alloc(sizeof(*pCmd), alignof(T));
		if(!pCmd)
			return false;
		*pCmd = Command;
		pCmd->m_pNext = nullptr;

		if(m_pCmdBufferTail)
			m_pCmdBufferTail->m_pNext = pCmd;
		if(!m_pCmdBufferHead)
			m_pCmdBufferHead = pCmd;
		m_pCmdBufferTail = pCmd;

		++m_CommandCount;

		return true;
	}

	SCommand *Head()
	{
		return m_pCmdBufferHead;
	}

	void Reset()
	{
		m_pCmdBufferHead = m_pCmdBufferTail = nullptr;
		m_CmdBuffer.Reset();
		m_DataBuffer.Reset();

		m_CommandCount = 0;
		m_RenderCallCount = 0;
	}

	void AddRenderCalls(size_t RenderCallCountToAdd)
	{
		m_RenderCallCount += RenderCallCountToAdd;
	}
};

enum EGraphicsBackendErrorCodes
{
	GRAPHICS_BACKEND_ERROR_CODE_UNKNOWN = -1,
	GRAPHICS_BACKEND_ERROR_CODE_NONE = 0,
	GRAPHICS_BACKEND_ERROR_CODE_GL_CONTEXT_FAILED,
	GRAPHICS_BACKEND_ERROR_CODE_GL_VERSION_FAILED,
	GRAPHICS_BACKEND_ERROR_CODE_SDL_INIT_FAILED,
	GRAPHICS_BACKEND_ERROR_CODE_SDL_SCREEN_REQUEST_FAILED,
	GRAPHICS_BACKEND_ERROR_CODE_SDL_SCREEN_INFO_REQUEST_FAILED,
	GRAPHICS_BACKEND_ERROR_CODE_SDL_SCREEN_RESOLUTION_REQUEST_FAILED,
	GRAPHICS_BACKEND_ERROR_CODE_SDL_WINDOW_CREATE_FAILED,
};

// interface for the graphics backend
// all these functions are called on the main thread
class IGraphicsBackend
{
public:
	enum
	{
		INITFLAG_FULLSCREEN = 1 << 0,
		INITFLAG_VSYNC = 1 << 1,
		INITFLAG_RESIZABLE = 1 << 2,
		INITFLAG_BORDERLESS = 1 << 3,
		INITFLAG_HIGHDPI = 1 << 4,
		INITFLAG_DESKTOP_FULLSCREEN = 1 << 5,
	};

	virtual ~IGraphicsBackend() = default;

	virtual int Init(const char *pName, int *pScreen, int *pWidth, int *pHeight, int *pRefreshRate, int *pFsaaSamples, int Flags, int *pDesktopWidth, int *pDesktopHeight, int *pCurrentWidth, int *pCurrentHeight, class IStorage *pStorage) = 0;
	virtual int Shutdown() = 0;

	virtual uint64_t TextureMemoryUsage() const = 0;
	virtual uint64_t BufferMemoryUsage() const = 0;
	virtual uint64_t StreamedMemoryUsage() const = 0;
	virtual uint64_t StagingMemoryUsage() const = 0;

	virtual const TTWGraphicsGPUList &GetGPUs() const = 0;

	virtual void GetVideoModes(CVideoMode *pModes, int MaxModes, int *pNumModes, int HiDPIScale, int MaxWindowWidth, int MaxWindowHeight, int Screen) = 0;
	virtual void GetCurrentVideoMode(CVideoMode &CurMode, int HiDPIScale, int MaxWindowWidth, int MaxWindowHeight, int Screen) = 0;

	virtual int GetNumScreens() const = 0;

	virtual void Minimize() = 0;
	virtual void Maximize() = 0;
	virtual void SetWindowParams(int FullscreenMode, bool IsBorderless, bool AllowResizing) = 0;
	virtual bool SetWindowScreen(int Index) = 0;
	virtual bool UpdateDisplayMode(int Index) = 0;
	virtual int GetWindowScreen() = 0;
	virtual int WindowActive() = 0;
	virtual int WindowOpen() = 0;
	virtual void SetWindowGrab(bool Grab) = 0;
	// returns true, if the video mode changed
	virtual bool ResizeWindow(int w, int h, int RefreshRate) = 0;
	virtual void GetViewportSize(int &w, int &h) = 0;
	virtual void NotifyWindow() = 0;

	virtual void WindowDestroyNtf(uint32_t WindowID) = 0;
	virtual void WindowCreateNtf(uint32_t WindowID) = 0;

	virtual void RunBuffer(CCommandBuffer *pBuffer) = 0;
	virtual void RunBufferSingleThreadedUnsafe(CCommandBuffer *pBuffer) = 0;
	virtual bool IsIdle() const = 0;
	virtual void WaitForIdle() = 0;

	virtual bool GetDriverVersion(EGraphicsDriverAgeType DriverAgeType, int &Major, int &Minor, int &Patch, const char *&pName, EBackendType BackendType) = 0;
	// checks if the current values of the config are a graphics modern API
	virtual bool IsConfigModernAPI() { return false; }
	virtual bool UseTrianglesAsQuad() { return false; }
	virtual bool HasTileBuffering() { return false; }
	virtual bool HasQuadBuffering() { return false; }
	virtual bool HasTextBuffering() { return false; }
	virtual bool HasQuadContainerBuffering() { return false; }
	virtual bool Has2DTextureArrays() { return false; }
	virtual const char *GetErrorString() { return NULL; }

	virtual const char *GetVendorString() = 0;
	virtual const char *GetVersionString() = 0;
	virtual const char *GetRendererString() = 0;

	// be aware that this function should only be called from the graphics thread, and even then you should really know what you are doing
	virtual TGLBackendReadPresentedImageData &GetReadPresentedImageDataFuncUnsafe() = 0;

	virtual bool GetWarning(std::vector<std::string> &WarningStrings) = 0;
};

class CGraphics_Threaded : public IEngineGraphics
{
	enum
	{
		NUM_CMDBUFFERS = 2,

		DRAWING_QUADS = 1,
		DRAWING_LINES = 2,
		DRAWING_TRIANGLES = 3
	};

	CCommandBuffer::SState m_State;
	IGraphicsBackend *m_pBackend;
	bool m_GLTileBufferingEnabled;
	bool m_GLQuadBufferingEnabled;
	bool m_GLTextBufferingEnabled;
	bool m_GLQuadContainerBufferingEnabled;
	bool m_GLHasTextureArrays;
	bool m_GLUseTrianglesAsQuad;

	CCommandBuffer *m_apCommandBuffers[NUM_CMDBUFFERS];
	CCommandBuffer *m_pCommandBuffer;
	unsigned m_CurrentCommandBuffer;

	//
	class IStorage *m_pStorage;
	class IConsole *m_pConsole;

	int m_CurIndex;

	CCommandBuffer::SVertex m_aVertices[CCommandBuffer::MAX_VERTICES];
	CCommandBuffer::SVertexTex3DStream m_aVerticesTex3D[CCommandBuffer::MAX_VERTICES];
	int m_NumVertices;

	CCommandBuffer::SColor m_aColor[4];
	CCommandBuffer::STexCoord m_aTexture[4];

	bool m_RenderEnable;

	float m_Rotation;
	int m_Drawing;
	bool m_DoScreenshot;
	char m_aScreenshotName[128];

	CTextureHandle m_InvalidTexture;

	std::vector<int> m_vTextureIndices;
	int m_FirstFreeTexture;
	int m_TextureMemoryUsage;

	std::vector<uint8_t> m_vSpriteHelper;

	bool m_WarnPngliteIncompatibleImages = false;

	std::vector<SWarning> m_vWarnings;

	// is a non full windowed (in a sense that the viewport won't include the whole window),
	// forced viewport, so that it justifies our UI ratio needs
	bool m_IsForcedViewport = false;

	struct SVertexArrayInfo
	{
		SVertexArrayInfo() :
			m_FreeIndex(-1) {}
		// keep a reference to it, so we can free the ID
		int m_AssociatedBufferObjectIndex;

		int m_FreeIndex;
	};
	std::vector<SVertexArrayInfo> m_vVertexArrayInfo;
	int m_FirstFreeVertexArrayInfo;

	std::vector<int> m_vBufferObjectIndices;
	int m_FirstFreeBufferObjectIndex;

	struct SQuadContainer
	{
		SQuadContainer(bool AutomaticUpload = true)
		{
			m_vQuads.clear();
			m_QuadBufferObjectIndex = m_QuadBufferContainerIndex = -1;
			m_FreeIndex = -1;

			m_AutomaticUpload = AutomaticUpload;
		}

		struct SQuad
		{
			CCommandBuffer::SVertex m_aVertices[4];
		};

		std::vector<SQuad> m_vQuads;

		int m_QuadBufferObjectIndex;
		int m_QuadBufferContainerIndex;

		int m_FreeIndex;

		bool m_AutomaticUpload;
	};
	std::vector<SQuadContainer> m_vQuadContainers;
	int m_FirstFreeQuadContainer;

	struct SWindowResizeListener
	{
		SWindowResizeListener(WINDOW_RESIZE_FUNC pFunc, void *pUser) :
			m_pFunc(std::move(pFunc)), m_pUser(pUser) {}
		WINDOW_RESIZE_FUNC m_pFunc;
		void *m_pUser;
	};
	std::vector<SWindowResizeListener> m_vResizeListeners;

	void *AllocCommandBufferData(unsigned AllocSize);

	void AddVertices(int Count);
	void AddVertices(int Count, CCommandBuffer::SVertex *pVertices);
	void AddVertices(int Count, CCommandBuffer::SVertexTex3DStream *pVertices);

	template<typename TName>
	void Rotate(const CCommandBuffer::SPoint &rCenter, TName *pPoints, int NumPoints)
	{
		float c = cosf(m_Rotation);
		float s = sinf(m_Rotation);
		float x, y;
		int i;

		TName *pVertices = pPoints;
		for(i = 0; i < NumPoints; i++)
		{
			x = pVertices[i].m_Pos.x - rCenter.x;
			y = pVertices[i].m_Pos.y - rCenter.y;
			pVertices[i].m_Pos.x = x * c - y * s + rCenter.x;
			pVertices[i].m_Pos.y = x * s + y * c + rCenter.y;
		}
	}

	template<typename TName, typename TFunc>
	bool AddCmd(TName &Cmd, TFunc &&FailFunc, const char *pFailStr)
	{
		if(!m_pCommandBuffer->AddCommandUnsafe(Cmd))
		{
			// kick command buffer and try again
			KickCommandBuffer();

			if(!FailFunc())
				return false;

			if(!m_pCommandBuffer->AddCommandUnsafe(Cmd))
			{
				dbg_msg("graphics", "%s", pFailStr);
				return false;
			}
		}
		return true;
	}

	void KickCommandBuffer();

	void AddBackEndWarningIfExists();

	void AdjustViewport(bool SendViewportChangeToBackend);

	int IssueInit();
	int InitWindow();

public:
	CGraphics_Threaded();

	void ClipEnable(int x, int y, int w, int h) override;
	void ClipDisable() override;

	void BlendNone() override;
	void BlendNormal() override;
	void BlendAdditive() override;

	void WrapNormal() override;
	void WrapClamp() override;

	uint64_t TextureMemoryUsage() const override;
	uint64_t BufferMemoryUsage() const override;
	uint64_t StreamedMemoryUsage() const override;
	uint64_t StagingMemoryUsage() const override;

	const TTWGraphicsGPUList &GetGPUs() const override;

	void MapScreen(float TopLeftX, float TopLeftY, float BottomRightX, float BottomRightY) override;
	void GetScreen(float *pTopLeftX, float *pTopLeftY, float *pBottomRightX, float *pBottomRightY) override;

	void LinesBegin() override;
	void LinesEnd() override;
	void LinesDraw(const CLineItem *pArray, int Num) override;

	int UnloadTexture(IGraphics::CTextureHandle *pIndex) override;
	IGraphics::CTextureHandle LoadTextureRaw(int Width, int Height, int Format, const void *pData, int StoreFormat, int Flags, const char *pTexName = NULL) override;
	int LoadTextureRawSub(IGraphics::CTextureHandle TextureID, int x, int y, int Width, int Height, int Format, const void *pData) override;

	bool LoadTextTextures(int Width, int Height, CTextureHandle &TextTexture, CTextureHandle &TextOutlineTexture, void *pTextData, void *pTextOutlineData) override;
	bool UnloadTextTextures(CTextureHandle &TextTexture, CTextureHandle &TextOutlineTexture) override;
	bool UpdateTextTexture(CTextureHandle TextureID, int x, int y, int Width, int Height, const void *pData) override;

	CTextureHandle LoadSpriteTextureImpl(CImageInfo &FromImageInfo, int x, int y, int w, int h);
	CTextureHandle LoadSpriteTexture(CImageInfo &FromImageInfo, struct CDataSprite *pSprite) override;
	CTextureHandle LoadSpriteTexture(CImageInfo &FromImageInfo, struct client_data7::CDataSprite *pSprite) override;

	bool IsImageSubFullyTransparent(CImageInfo &FromImageInfo, int x, int y, int w, int h) override;
	bool IsSpriteTextureFullyTransparent(CImageInfo &FromImageInfo, struct client_data7::CDataSprite *pSprite) override;

	// simple uncompressed RGBA loaders
	IGraphics::CTextureHandle LoadTexture(const char *pFilename, int StorageType, int StoreFormat, int Flags) override;
	int LoadPNG(CImageInfo *pImg, const char *pFilename, int StorageType) override;
	void FreePNG(CImageInfo *pImg) override;

	bool CheckImageDivisibility(const char *pFileName, CImageInfo &Img, int DivX, int DivY, bool AllowResize) override;
	bool IsImageFormatRGBA(const char *pFileName, CImageInfo &Img) override;

	void CopyTextureBufferSub(uint8_t *pDestBuffer, uint8_t *pSourceBuffer, int FullWidth, int FullHeight, int ColorChannelCount, int SubOffsetX, int SubOffsetY, int SubCopyWidth, int SubCopyHeight) override;
	void CopyTextureFromTextureBufferSub(uint8_t *pDestBuffer, int DestWidth, int DestHeight, uint8_t *pSourceBuffer, int SrcWidth, int SrcHeight, int ColorChannelCount, int SrcSubOffsetX, int SrcSubOffsetY, int SrcSubCopyWidth, int SrcSubCopyHeight) override;

	bool ScreenshotDirect();

	void TextureSet(CTextureHandle TextureID) override;

	void Clear(float r, float g, float b, bool ForceClearNow = false) override;

	void QuadsBegin() override;
	void QuadsEnd() override;
	void QuadsTex3DBegin() override;
	void QuadsTex3DEnd() override;
	void TrianglesBegin() override;
	void TrianglesEnd() override;
	void QuadsEndKeepVertices() override;
	void QuadsDrawCurrentVertices(bool KeepVertices = true) override;
	void QuadsSetRotation(float Angle) override;

	template<typename TName>
	void SetColor(TName *pVertex, int ColorIndex)
	{
		TName *pVert = pVertex;
		pVert->m_Color = m_aColor[ColorIndex];
	}

	void SetColorVertex(const CColorVertex *pArray, int Num) override;
	void SetColor(float r, float g, float b, float a) override;
	void SetColor(ColorRGBA Color) override;
	void SetColor4(ColorRGBA TopLeft, ColorRGBA TopRight, ColorRGBA BottomLeft, ColorRGBA BottomRight) override;

	// go through all vertices and change their color (only works for quads)
	void ChangeColorOfCurrentQuadVertices(float r, float g, float b, float a) override;
	void ChangeColorOfQuadVertices(int QuadOffset, unsigned char r, unsigned char g, unsigned char b, unsigned char a) override;

	void QuadsSetSubset(float TlU, float TlV, float BrU, float BrV) override;
	void QuadsSetSubsetFree(
		float x0, float y0, float x1, float y1,
		float x2, float y2, float x3, float y3, int Index = -1) override;

	void QuadsDraw(CQuadItem *pArray, int Num) override;

	template<typename TName>
	void QuadsDrawTLImpl(TName *pVertices, const CQuadItem *pArray, int Num)
	{
		CCommandBuffer::SPoint Center;

		dbg_assert(m_Drawing == DRAWING_QUADS, "called Graphics()->QuadsDrawTL without begin");

		if(g_Config.m_GfxQuadAsTriangle && !m_GLUseTrianglesAsQuad)
		{
			for(int i = 0; i < Num; ++i)
			{
				// first triangle
				pVertices[m_NumVertices + 6 * i].m_Pos.x = pArray[i].m_X;
				pVertices[m_NumVertices + 6 * i].m_Pos.y = pArray[i].m_Y;
				pVertices[m_NumVertices + 6 * i].m_Tex = m_aTexture[0];
				SetColor(&pVertices[m_NumVertices + 6 * i], 0);

				pVertices[m_NumVertices + 6 * i + 1].m_Pos.x = pArray[i].m_X + pArray[i].m_Width;
				pVertices[m_NumVertices + 6 * i + 1].m_Pos.y = pArray[i].m_Y;
				pVertices[m_NumVertices + 6 * i + 1].m_Tex = m_aTexture[1];
				SetColor(&pVertices[m_NumVertices + 6 * i + 1], 1);

				pVertices[m_NumVertices + 6 * i + 2].m_Pos.x = pArray[i].m_X + pArray[i].m_Width;
				pVertices[m_NumVertices + 6 * i + 2].m_Pos.y = pArray[i].m_Y + pArray[i].m_Height;
				pVertices[m_NumVertices + 6 * i + 2].m_Tex = m_aTexture[2];
				SetColor(&pVertices[m_NumVertices + 6 * i + 2], 2);

				// second triangle
				pVertices[m_NumVertices + 6 * i + 3].m_Pos.x = pArray[i].m_X;
				pVertices[m_NumVertices + 6 * i + 3].m_Pos.y = pArray[i].m_Y;
				pVertices[m_NumVertices + 6 * i + 3].m_Tex = m_aTexture[0];
				SetColor(&pVertices[m_NumVertices + 6 * i + 3], 0);

				pVertices[m_NumVertices + 6 * i + 4].m_Pos.x = pArray[i].m_X + pArray[i].m_Width;
				pVertices[m_NumVertices + 6 * i + 4].m_Pos.y = pArray[i].m_Y + pArray[i].m_Height;
				pVertices[m_NumVertices + 6 * i + 4].m_Tex = m_aTexture[2];
				SetColor(&pVertices[m_NumVertices + 6 * i + 4], 2);

				pVertices[m_NumVertices + 6 * i + 5].m_Pos.x = pArray[i].m_X;
				pVertices[m_NumVertices + 6 * i + 5].m_Pos.y = pArray[i].m_Y + pArray[i].m_Height;
				pVertices[m_NumVertices + 6 * i + 5].m_Tex = m_aTexture[3];
				SetColor(&pVertices[m_NumVertices + 6 * i + 5], 3);

				if(m_Rotation != 0)
				{
					Center.x = pArray[i].m_X + pArray[i].m_Width / 2;
					Center.y = pArray[i].m_Y + pArray[i].m_Height / 2;

					Rotate(Center, &pVertices[m_NumVertices + 6 * i], 6);
				}
			}

			AddVertices(3 * 2 * Num, pVertices);
		}
		else
		{
			for(int i = 0; i < Num; ++i)
			{
				pVertices[m_NumVertices + 4 * i].m_Pos.x = pArray[i].m_X;
				pVertices[m_NumVertices + 4 * i].m_Pos.y = pArray[i].m_Y;
				pVertices[m_NumVertices + 4 * i].m_Tex = m_aTexture[0];
				SetColor(&pVertices[m_NumVertices + 4 * i], 0);

				pVertices[m_NumVertices + 4 * i + 1].m_Pos.x = pArray[i].m_X + pArray[i].m_Width;
				pVertices[m_NumVertices + 4 * i + 1].m_Pos.y = pArray[i].m_Y;
				pVertices[m_NumVertices + 4 * i + 1].m_Tex = m_aTexture[1];
				SetColor(&pVertices[m_NumVertices + 4 * i + 1], 1);

				pVertices[m_NumVertices + 4 * i + 2].m_Pos.x = pArray[i].m_X + pArray[i].m_Width;
				pVertices[m_NumVertices + 4 * i + 2].m_Pos.y = pArray[i].m_Y + pArray[i].m_Height;
				pVertices[m_NumVertices + 4 * i + 2].m_Tex = m_aTexture[2];
				SetColor(&pVertices[m_NumVertices + 4 * i + 2], 2);

				pVertices[m_NumVertices + 4 * i + 3].m_Pos.x = pArray[i].m_X;
				pVertices[m_NumVertices + 4 * i + 3].m_Pos.y = pArray[i].m_Y + pArray[i].m_Height;
				pVertices[m_NumVertices + 4 * i + 3].m_Tex = m_aTexture[3];
				SetColor(&pVertices[m_NumVertices + 4 * i + 3], 3);

				if(m_Rotation != 0)
				{
					Center.x = pArray[i].m_X + pArray[i].m_Width / 2;
					Center.y = pArray[i].m_Y + pArray[i].m_Height / 2;

					Rotate(Center, &pVertices[m_NumVertices + 4 * i], 4);
				}
			}

			AddVertices(4 * Num, pVertices);
		}
	}

	void QuadsDrawTL(const CQuadItem *pArray, int Num) override;

	void QuadsTex3DDrawTL(const CQuadItem *pArray, int Num) override;

	void QuadsDrawFreeform(const CFreeformItem *pArray, int Num) override;
	void QuadsText(float x, float y, float Size, const char *pText) override;

	void DrawRectExt(float x, float y, float w, float h, float r, int Corners) override;
	void DrawRectExt4(float x, float y, float w, float h, ColorRGBA ColorTopLeft, ColorRGBA ColorTopRight, ColorRGBA ColorBottomLeft, ColorRGBA ColorBottomRight, float r, int Corners) override;
	int CreateRectQuadContainer(float x, float y, float w, float h, float r, int Corners) override;
	void DrawRect(float x, float y, float w, float h, ColorRGBA Color, int Corners, float Rounding) override;
	void DrawRect4(float x, float y, float w, float h, ColorRGBA ColorTopLeft, ColorRGBA ColorTopRight, ColorRGBA ColorBottomLeft, ColorRGBA ColorBottomRight, int Corners, float Rounding) override;
	void DrawCircle(float CenterX, float CenterY, float Radius, int Segments) override;

	const GL_STexCoord *GetCurTextureCoordinates() override
	{
		return m_aTexture;
	}

	const GL_SColor *GetCurColor() override
	{
		return m_aColor;
	}

	int CreateQuadContainer(bool AutomaticUpload = true) override;
	void QuadContainerChangeAutomaticUpload(int ContainerIndex, bool AutomaticUpload) override;
	void QuadContainerUpload(int ContainerIndex) override;
	int QuadContainerAddQuads(int ContainerIndex, CQuadItem *pArray, int Num) override;
	int QuadContainerAddQuads(int ContainerIndex, CFreeformItem *pArray, int Num) override;
	void QuadContainerReset(int ContainerIndex) override;
	void DeleteQuadContainer(int ContainerIndex) override;
	void RenderQuadContainer(int ContainerIndex, int QuadDrawNum) override;
	void RenderQuadContainer(int ContainerIndex, int QuadOffset, int QuadDrawNum, bool ChangeWrapMode = true) override;
	void RenderQuadContainerEx(int ContainerIndex, int QuadOffset, int QuadDrawNum, float X, float Y, float ScaleX = 1.f, float ScaleY = 1.f) override;
	void RenderQuadContainerAsSprite(int ContainerIndex, int QuadOffset, float X, float Y, float ScaleX = 1.f, float ScaleY = 1.f) override;
	void RenderQuadContainerAsSpriteMultiple(int ContainerIndex, int QuadOffset, int DrawCount, SRenderSpriteInfo *pRenderInfo) override;

	template<typename TName>
	void FlushVerticesImpl(bool KeepVertices, int &PrimType, int &PrimCount, int &NumVerts, TName &Command, size_t VertSize)
	{
		Command.m_pVertices = NULL;
		if(m_NumVertices == 0)
			return;

		NumVerts = m_NumVertices;

		if(!KeepVertices)
			m_NumVertices = 0;

		if(m_Drawing == DRAWING_QUADS)
		{
			if(g_Config.m_GfxQuadAsTriangle && !m_GLUseTrianglesAsQuad)
			{
				PrimType = CCommandBuffer::PRIMTYPE_TRIANGLES;
				PrimCount = NumVerts / 3;
			}
			else
			{
				PrimType = CCommandBuffer::PRIMTYPE_QUADS;
				PrimCount = NumVerts / 4;
			}
		}
		else if(m_Drawing == DRAWING_LINES)
		{
			PrimType = CCommandBuffer::PRIMTYPE_LINES;
			PrimCount = NumVerts / 2;
		}
		else if(m_Drawing == DRAWING_TRIANGLES)
		{
			PrimType = CCommandBuffer::PRIMTYPE_TRIANGLES;
			PrimCount = NumVerts / 3;
		}
		else
			return;

		Command.m_pVertices = (decltype(Command.m_pVertices))m_pCommandBuffer->AllocData(VertSize * NumVerts);
		if(Command.m_pVertices == NULL)
		{
			// kick command buffer and try again
			KickCommandBuffer();

			Command.m_pVertices = (decltype(Command.m_pVertices))m_pCommandBuffer->AllocData(VertSize * NumVerts);
			if(Command.m_pVertices == NULL)
			{
				dbg_msg("graphics", "failed to allocate data for vertices");
				return;
			}
		}

		Command.m_State = m_State;

		Command.m_PrimType = PrimType;
		Command.m_PrimCount = PrimCount;

		if(
			!AddCmd(
				Command, [&] {
					Command.m_pVertices = (decltype(Command.m_pVertices))m_pCommandBuffer->AllocData(VertSize * NumVerts);
					if(Command.m_pVertices == NULL)
					{
						dbg_msg("graphics", "failed to allocate data for vertices");
						return false;
					}
					return true;
				},
				"failed to allocate memory for render command"))
		{
			return;
		}

		m_pCommandBuffer->AddRenderCalls(1);
	}

	void FlushVertices(bool KeepVertices = false) override;
	void FlushVerticesTex3D() override;

	void RenderTileLayer(int BufferContainerIndex, const ColorRGBA &Color, char **pOffsets, unsigned int *pIndicedVertexDrawNum, size_t NumIndicesOffset) override;
	void RenderBorderTiles(int BufferContainerIndex, const ColorRGBA &Color, char *pIndexBufferOffset, const vec2 &Offset, const vec2 &Dir, int JumpIndex, unsigned int DrawNum) override;
	void RenderBorderTileLines(int BufferContainerIndex, const ColorRGBA &Color, char *pIndexBufferOffset, const vec2 &Offset, const vec2 &Dir, unsigned int IndexDrawNum, unsigned int RedrawNum) override;
	void RenderQuadLayer(int BufferContainerIndex, SQuadRenderInfo *pQuadInfo, int QuadNum, int QuadOffset) override;
	void RenderText(int BufferContainerIndex, int TextQuadNum, int TextureSize, int TextureTextIndex, int TextureTextOutlineIndex, const ColorRGBA &TextColor, const ColorRGBA &TextOutlineColor) override;

	// modern GL functions
	int CreateBufferObject(size_t UploadDataSize, void *pUploadData, int CreateFlags, bool IsMovedPointer = false) override;
	void RecreateBufferObject(int BufferIndex, size_t UploadDataSize, void *pUploadData, int CreateFlags, bool IsMovedPointer = false) override;
	void UpdateBufferObjectInternal(int BufferIndex, size_t UploadDataSize, void *pUploadData, void *pOffset, bool IsMovedPointer = false);
	void CopyBufferObjectInternal(int WriteBufferIndex, int ReadBufferIndex, size_t WriteOffset, size_t ReadOffset, size_t CopyDataSize);
	void DeleteBufferObject(int BufferIndex) override;

	int CreateBufferContainer(SBufferContainerInfo *pContainerInfo) override;
	// destroying all buffer objects means, that all referenced VBOs are destroyed automatically, so the user does not need to save references to them
	void DeleteBufferContainer(int ContainerIndex, bool DestroyAllBO = true) override;
	void UpdateBufferContainerInternal(int ContainerIndex, SBufferContainerInfo *pContainerInfo);
	void IndicesNumRequiredNotify(unsigned int RequiredIndicesCount) override;

	int GetNumScreens() const override;
	void Minimize() override;
	void Maximize() override;
	void WarnPngliteIncompatibleImages(bool Warn) override;
	void SetWindowParams(int FullscreenMode, bool IsBorderless, bool AllowResizing) override;
	bool SetWindowScreen(int Index) override;
	void Move(int x, int y) override;
	void Resize(int w, int h, int RefreshRate) override;
	void GotResized(int w, int h, int RefreshRate) override;
	void UpdateViewport(int X, int Y, int W, int H, bool ByResize) override;
	void AddWindowResizeListener(WINDOW_RESIZE_FUNC pFunc, void *pUser) override;
	int GetWindowScreen() override;

	void WindowDestroyNtf(uint32_t WindowID) override;
	void WindowCreateNtf(uint32_t WindowID) override;

	int WindowActive() override;
	int WindowOpen() override;

	void SetWindowGrab(bool Grab) override;
	void NotifyWindow() override;

	int Init() override;
	void Shutdown() override;

	void TakeScreenshot(const char *pFilename) override;
	void TakeCustomScreenshot(const char *pFilename) override;
	void Swap() override;
	bool SetVSync(bool State) override;
	bool SetMultiSampling(uint32_t ReqMultiSamplingCount, uint32_t &MultiSamplingCountBackend) override;

	int GetVideoModes(CVideoMode *pModes, int MaxModes, int Screen) override;

	virtual int GetDesktopScreenWidth() const { return g_Config.m_GfxDesktopWidth; }
	virtual int GetDesktopScreenHeight() const { return g_Config.m_GfxDesktopHeight; }

	// synchronization
	void InsertSignal(CSemaphore *pSemaphore) override;
	bool IsIdle() const override;
	void WaitForIdle() override;

	SWarning *GetCurWarning() override;

	bool GetDriverVersion(EGraphicsDriverAgeType DriverAgeType, int &Major, int &Minor, int &Patch, const char *&pName, EBackendType BackendType) override { return m_pBackend->GetDriverVersion(DriverAgeType, Major, Minor, Patch, pName, BackendType); }
	bool IsConfigModernAPI() override { return m_pBackend->IsConfigModernAPI(); }
	bool IsTileBufferingEnabled() override { return m_GLTileBufferingEnabled; }
	bool IsQuadBufferingEnabled() override { return m_GLQuadBufferingEnabled; }
	bool IsTextBufferingEnabled() override { return m_GLTextBufferingEnabled; }
	bool IsQuadContainerBufferingEnabled() override { return m_GLQuadContainerBufferingEnabled; }
	bool HasTextureArrays() override { return m_GLHasTextureArrays; }

	const char *GetVendorString() override;
	const char *GetVersionString() override;
	const char *GetRendererString() override;

	TGLBackendReadPresentedImageData &GetReadPresentedImageDataFuncUnsafe() override;
};

typedef std::function<const char *(const char *, const char *)> TTranslateFunc;
extern IGraphicsBackend *CreateGraphicsBackend(TTranslateFunc &&TranslateFunc);

#endif // ENGINE_CLIENT_GRAPHICS_THREADED_H
