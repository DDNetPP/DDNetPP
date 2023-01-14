#include <base/detect.h>

#ifndef CONF_BACKEND_OPENGL_ES
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_messagebox.h>

#include <base/math.h>
#include <cstdlib>

#include <engine/shared/config.h>
#include <engine/shared/localization.h>

#include <base/tl/threading.h>

#if defined(CONF_VIDEORECORDER)
#include <engine/shared/video.h>
#endif

#include "backend_sdl.h"

#if defined(CONF_HEADLESS_CLIENT)
#include "backend/null/backend_null.h"
#endif

#if !defined(CONF_BACKEND_OPENGL_ES)
#include "backend/opengl/backend_opengl3.h"
#endif

#if defined(CONF_BACKEND_OPENGL_ES3) || defined(CONF_BACKEND_OPENGL_ES)
#include "backend/opengles/backend_opengles3.h"
#endif

#if defined(CONF_BACKEND_VULKAN)
#include "backend/vulkan/backend_vulkan.h"
#endif

#include "graphics_threaded.h"

#include <engine/graphics.h>

class IStorage;

// ------------ CGraphicsBackend_Threaded

void CGraphicsBackend_Threaded::ThreadFunc(void *pUser)
{
	auto *pSelf = (CGraphicsBackend_Threaded *)pUser;
	std::unique_lock<std::mutex> Lock(pSelf->m_BufferSwapMutex);
	// notify, that the thread started
	pSelf->m_Started = true;
	pSelf->m_BufferSwapCond.notify_all();
	while(!pSelf->m_Shutdown)
	{
		pSelf->m_BufferSwapCond.wait(Lock, [&pSelf] { return pSelf->m_pBuffer != nullptr || pSelf->m_Shutdown; });
		if(pSelf->m_pBuffer)
		{
#ifdef CONF_PLATFORM_MACOS
			CAutoreleasePool AutoreleasePool;
#endif
			pSelf->m_pProcessor->RunBuffer(pSelf->m_pBuffer);

			pSelf->m_pBuffer = nullptr;
			pSelf->m_BufferInProcess.store(false, std::memory_order_relaxed);
			pSelf->m_BufferSwapCond.notify_all();

#if defined(CONF_VIDEORECORDER)
			if(IVideo::Current())
				IVideo::Current()->NextVideoFrameThread();
#endif
		}
	}
}

CGraphicsBackend_Threaded::CGraphicsBackend_Threaded(TTranslateFunc &&TranslateFunc) :
	m_TranslateFunc(std::move(TranslateFunc))
{
	m_pBuffer = nullptr;
	m_pProcessor = nullptr;
	m_Shutdown = true;
	m_BufferInProcess.store(false, std::memory_order_relaxed);
}

void CGraphicsBackend_Threaded::StartProcessor(ICommandProcessor *pProcessor)
{
	dbg_assert(m_Shutdown, "Processor was already not shut down.");
	m_Shutdown = false;
	m_pProcessor = pProcessor;
	std::unique_lock<std::mutex> Lock(m_BufferSwapMutex);
	m_pThread = thread_init(ThreadFunc, this, "Graphics thread");
	// wait for the thread to start
	m_BufferSwapCond.wait(Lock, [this]() -> bool { return m_Started; });
}

void CGraphicsBackend_Threaded::StopProcessor()
{
	dbg_assert(!m_Shutdown, "Processor was already shut down.");
	m_Shutdown = true;
	{
		std::unique_lock<std::mutex> Lock(m_BufferSwapMutex);
		m_Warning = m_pProcessor->GetWarning();
		m_BufferSwapCond.notify_all();
	}
	thread_wait(m_pThread);
}

void CGraphicsBackend_Threaded::RunBuffer(CCommandBuffer *pBuffer)
{
#ifdef CONF_WEBASM
	// run everything single threaded for now, context binding in a thread seems to not work as of now
	if(!m_pProcessor->HasError())
	{
		RunBufferSingleThreadedUnsafe(pBuffer);
	}
	else
	{
		ProcessError();
	}
#else
	WaitForIdle();
	std::unique_lock<std::mutex> Lock(m_BufferSwapMutex);
	if(!m_pProcessor->HasError())
	{
		m_pBuffer = pBuffer;
		m_BufferInProcess.store(true, std::memory_order_relaxed);
		m_BufferSwapCond.notify_all();
	}
	else
	{
		ProcessError();
	}
#endif
}

void CGraphicsBackend_Threaded::RunBufferSingleThreadedUnsafe(CCommandBuffer *pBuffer)
{
	m_pProcessor->RunBuffer(pBuffer);
}

bool CGraphicsBackend_Threaded::IsIdle() const
{
	return !m_BufferInProcess.load(std::memory_order_relaxed);
}

void CGraphicsBackend_Threaded::WaitForIdle()
{
	std::unique_lock<std::mutex> Lock(m_BufferSwapMutex);
	m_BufferSwapCond.wait(Lock, [this]() { return m_pBuffer == nullptr; });
}

void CGraphicsBackend_Threaded::ProcessError()
{
	const auto &Error = m_pProcessor->GetError();
	std::string VerboseStr;
	for(const auto &ErrStr : Error.m_vErrors)
	{
		if(ErrStr.m_RequiresTranslation)
			VerboseStr.append(std::string(m_TranslateFunc(ErrStr.m_Err.c_str(), "")) + "\n");
		else
			VerboseStr.append(ErrStr.m_Err + "\n");
	}
	const auto CreatedMsgBox = TryCreateMsgBox(true, "Graphics Assertion", VerboseStr.c_str());
	// check if error msg can be shown, then assert
	dbg_assert(!CreatedMsgBox, VerboseStr.c_str());
}

bool CGraphicsBackend_Threaded::GetWarning(std::vector<std::string> &WarningStrings)
{
	if(HasWarning())
	{
		m_Warning.m_WarningType = GFX_WARNING_TYPE_NONE;
		WarningStrings = m_Warning.m_vWarnings;
		return true;
	}
	return false;
}

// ------------ CCommandProcessorFragment_General

void CCommandProcessorFragment_General::Cmd_Signal(const CCommandBuffer::SCommand_Signal *pCommand)
{
	pCommand->m_pSemaphore->Signal();
}

bool CCommandProcessorFragment_General::RunCommand(const CCommandBuffer::SCommand *pBaseCommand)
{
	switch(pBaseCommand->m_Cmd)
	{
	case CCommandBuffer::CMD_NOP: break;
	case CCommandBuffer::CMD_SIGNAL: Cmd_Signal(static_cast<const CCommandBuffer::SCommand_Signal *>(pBaseCommand)); break;
	default: return false;
	}

	return true;
}

// ------------ CCommandProcessorFragment_SDL
void CCommandProcessorFragment_SDL::Cmd_Init(const SCommand_Init *pCommand)
{
	m_GLContext = pCommand->m_GLContext;
	m_pWindow = pCommand->m_pWindow;
	if(m_GLContext)
		SDL_GL_MakeCurrent(m_pWindow, m_GLContext);
}

void CCommandProcessorFragment_SDL::Cmd_Shutdown(const SCommand_Shutdown *pCommand)
{
	if(m_GLContext)
		SDL_GL_MakeCurrent(NULL, NULL);
}

void CCommandProcessorFragment_SDL::Cmd_Swap(const CCommandBuffer::SCommand_Swap *pCommand)
{
	if(m_GLContext)
		SDL_GL_SwapWindow(m_pWindow);
}

void CCommandProcessorFragment_SDL::Cmd_VSync(const CCommandBuffer::SCommand_VSync *pCommand)
{
	if(m_GLContext)
		*pCommand->m_pRetOk = SDL_GL_SetSwapInterval(pCommand->m_VSync) == 0;
}

void CCommandProcessorFragment_SDL::Cmd_WindowCreateNtf(const CCommandBuffer::SCommand_WindowCreateNtf *pCommand)
{
	m_pWindow = SDL_GetWindowFromID(pCommand->m_WindowID);
	// Android destroys windows when they are not visible, so we get the new one and work with that
	// The graphic context does not need to be recreated, just unbound see @see SCommand_WindowDestroyNtf
#ifdef CONF_PLATFORM_ANDROID
	if(m_GLContext)
		SDL_GL_MakeCurrent(m_pWindow, m_GLContext);
	dbg_msg("gfx", "render surface created.");
#endif
}

void CCommandProcessorFragment_SDL::Cmd_WindowDestroyNtf(const CCommandBuffer::SCommand_WindowDestroyNtf *pCommand)
{
	// Unbind the graphic context from the window, so it does not get destroyed
#ifdef CONF_PLATFORM_ANDROID
	dbg_msg("gfx", "render surface destroyed.");
	if(m_GLContext)
		SDL_GL_MakeCurrent(NULL, NULL);
#endif
}

CCommandProcessorFragment_SDL::CCommandProcessorFragment_SDL() = default;

bool CCommandProcessorFragment_SDL::RunCommand(const CCommandBuffer::SCommand *pBaseCommand)
{
	switch(pBaseCommand->m_Cmd)
	{
	case CCommandBuffer::CMD_WINDOW_CREATE_NTF: Cmd_WindowCreateNtf(static_cast<const CCommandBuffer::SCommand_WindowCreateNtf *>(pBaseCommand)); break;
	case CCommandBuffer::CMD_WINDOW_DESTROY_NTF: Cmd_WindowDestroyNtf(static_cast<const CCommandBuffer::SCommand_WindowDestroyNtf *>(pBaseCommand)); break;
	case CCommandBuffer::CMD_SWAP: Cmd_Swap(static_cast<const CCommandBuffer::SCommand_Swap *>(pBaseCommand)); break;
	case CCommandBuffer::CMD_VSYNC: Cmd_VSync(static_cast<const CCommandBuffer::SCommand_VSync *>(pBaseCommand)); break;
	case CCommandBuffer::CMD_MULTISAMPLING: break;
	case CMD_INIT: Cmd_Init(static_cast<const SCommand_Init *>(pBaseCommand)); break;
	case CMD_SHUTDOWN: Cmd_Shutdown(static_cast<const SCommand_Shutdown *>(pBaseCommand)); break;
	case CCommandProcessorFragment_GLBase::CMD_PRE_INIT: break;
	case CCommandProcessorFragment_GLBase::CMD_POST_SHUTDOWN: break;
	default: return false;
	}

	return true;
}

// ------------ CCommandProcessor_SDL_GL

void CCommandProcessor_SDL_GL::HandleError()
{
	auto &Error = GetError();
	switch(Error.m_ErrorType)
	{
	case GFX_ERROR_TYPE_INIT:
		Error.m_vErrors.emplace_back(SGFXErrorContainer::SError{true, Localizable("Failed during initialization. Try to change gfx_backend to OpenGL or Vulkan in settings_ddnet.cfg in the config directory and try again.", "Graphics error")});
		break;
	case GFX_ERROR_TYPE_OUT_OF_MEMORY_IMAGE:
		[[fallthrough]];
	case GFX_ERROR_TYPE_OUT_OF_MEMORY_BUFFER:
		[[fallthrough]];
	case GFX_ERROR_TYPE_OUT_OF_MEMORY_STAGING:
		Error.m_vErrors.emplace_back(SGFXErrorContainer::SError{true, Localizable("Out of VRAM. Try removing custom assets (skins, entities, etc.), especially those with high resolution.", "Graphics error")});
		break;
	case GFX_ERROR_TYPE_RENDER_RECORDING:
		Error.m_vErrors.emplace_back(SGFXErrorContainer::SError{true, Localizable("An error during command recording occurred. Try to update your GPU drivers.", "Graphics error")});
		break;
	case GFX_ERROR_TYPE_RENDER_CMD_FAILED:
		Error.m_vErrors.emplace_back(SGFXErrorContainer::SError{true, Localizable("A render command failed. Try to update your GPU drivers.", "Graphics error")});
		break;
	case GFX_ERROR_TYPE_RENDER_SUBMIT_FAILED:
		Error.m_vErrors.emplace_back(SGFXErrorContainer::SError{true, Localizable("Submitting the render commands failed. Try to update your GPU drivers.", "Graphics error")});
		break;
	case GFX_ERROR_TYPE_SWAP_FAILED:
		Error.m_vErrors.emplace_back(SGFXErrorContainer::SError{true, Localizable("Failed to swap framebuffers. Try to update your GPU drivers.", "Graphics error")});
		break;
	case GFX_ERROR_TYPE_UNKNOWN:
		[[fallthrough]];
	default:
		Error.m_vErrors.emplace_back(SGFXErrorContainer::SError{true, Localizable("Unknown error. Try to change gfx_backend to OpenGL or Vulkan in settings_ddnet.cfg in the config directory and try again.", "Graphics error")});
		break;
	}
}

void CCommandProcessor_SDL_GL::HandleWarning()
{
	auto &Warn = GetWarning();
	switch(Warn.m_WarningType)
	{
	case GFX_WARNING_TYPE_INIT_FAILED:
		Warn.m_vWarnings.emplace_back(Localizable("Could not initialize the given graphics backend, reverting to the default backend now.", "Graphics error"));
		break;
	case GFX_WARNING_TYPE_INIT_FAILED_MISSING_INTEGRATED_GPU_DRIVER:
		Warn.m_vWarnings.emplace_back(Localizable("Could not initialize the given graphics backend, this is probably caused because you didn't install the driver of the integrated graphics card.", "Graphics error"));
		break;
	case GFX_WARNING_MISSING_EXTENSION:
		// ignore this warning for now
		return;
	case GFX_WARNING_LOW_ON_MEMORY:
		// ignore this warning for now
		return;
	default:
		dbg_msg("gfx", "unhandled warning %d", (int)Warn.m_WarningType);
		break;
	}
}

void CCommandProcessor_SDL_GL::RunBuffer(CCommandBuffer *pBuffer)
{
	m_pGLBackend->StartCommands(pBuffer->m_CommandCount, pBuffer->m_RenderCallCount);

	for(CCommandBuffer::SCommand *pCommand = pBuffer->Head(); pCommand; pCommand = pCommand->m_pNext)
	{
		auto Res = m_pGLBackend->RunCommand(pCommand);
		if(Res == ERunCommandReturnTypes::RUN_COMMAND_COMMAND_HANDLED)
		{
			continue;
		}
		else if(Res == ERunCommandReturnTypes::RUN_COMMAND_COMMAND_ERROR)
		{
			m_Error = m_pGLBackend->GetError();
			HandleError();
			return;
		}
		else if(Res == ERunCommandReturnTypes::RUN_COMMAND_COMMAND_WARNING)
		{
			m_Warning = m_pGLBackend->GetWarning();
			HandleWarning();
			return;
		}

		if(m_SDL.RunCommand(pCommand))
			continue;

		if(m_General.RunCommand(pCommand))
			continue;

		dbg_msg("gfx", "unknown command %d", pCommand->m_Cmd);
	}

	m_pGLBackend->EndCommands();
}

CCommandProcessor_SDL_GL::CCommandProcessor_SDL_GL(EBackendType BackendType, int GLMajor, int GLMinor, int GLPatch)
{
	m_BackendType = BackendType;

#if defined(CONF_HEADLESS_CLIENT)
	m_pGLBackend = new CCommandProcessorFragment_Null();
#else
	if(BackendType == BACKEND_TYPE_OPENGL_ES)
	{
#if defined(CONF_BACKEND_OPENGL_ES) || defined(CONF_BACKEND_OPENGL_ES3)
		if(GLMajor < 3)
		{
			m_pGLBackend = new CCommandProcessorFragment_OpenGLES();
		}
		else
		{
			m_pGLBackend = new CCommandProcessorFragment_OpenGLES3();
		}
#endif
	}
	else if(BackendType == BACKEND_TYPE_OPENGL)
	{
#if !defined(CONF_BACKEND_OPENGL_ES)
		if(GLMajor < 2)
		{
			m_pGLBackend = new CCommandProcessorFragment_OpenGL();
		}
		if(GLMajor == 2)
		{
			m_pGLBackend = new CCommandProcessorFragment_OpenGL2();
		}
		if(GLMajor == 3 && GLMinor == 0)
		{
			m_pGLBackend = new CCommandProcessorFragment_OpenGL3();
		}
		else if((GLMajor == 3 && GLMinor == 3) || GLMajor >= 4)
		{
			m_pGLBackend = new CCommandProcessorFragment_OpenGL3_3();
		}
#endif
	}
	else if(BackendType == BACKEND_TYPE_VULKAN)
	{
#if defined(CONF_BACKEND_VULKAN)
		m_pGLBackend = CreateVulkanCommandProcessorFragment();
#endif
	}
#endif
}

CCommandProcessor_SDL_GL::~CCommandProcessor_SDL_GL()
{
	delete m_pGLBackend;
}

SGFXErrorContainer &CCommandProcessor_SDL_GL::GetError()
{
	return m_Error;
}

void CCommandProcessor_SDL_GL::ErroneousCleanup()
{
	return m_pGLBackend->ErroneousCleanup();
}

SGFXWarningContainer &CCommandProcessor_SDL_GL::GetWarning()
{
	return m_Warning;
}

// ------------ CGraphicsBackend_SDL_GL

static bool BackendInitGlew(EBackendType BackendType, int &GlewMajor, int &GlewMinor, int &GlewPatch)
{
	if(BackendType == BACKEND_TYPE_OPENGL)
	{
#ifndef CONF_BACKEND_OPENGL_ES
		// support graphic cards that are pretty old(and linux)
		glewExperimental = GL_TRUE;
#ifdef CONF_GLEW_HAS_CONTEXT_INIT
		if(GLEW_OK != glewContextInit())
#else
		if(GLEW_OK != glewInit())
#endif
			return false;

#ifdef GLEW_VERSION_4_6
		if(GLEW_VERSION_4_6)
		{
			GlewMajor = 4;
			GlewMinor = 6;
			GlewPatch = 0;
			return true;
		}
#endif
#ifdef GLEW_VERSION_4_5
		if(GLEW_VERSION_4_5)
		{
			GlewMajor = 4;
			GlewMinor = 5;
			GlewPatch = 0;
			return true;
		}
#endif
// Don't allow GL 3.3, if the driver doesn't support at least OpenGL 4.5
#ifndef CONF_FAMILY_WINDOWS
		if(GLEW_VERSION_4_4)
		{
			GlewMajor = 4;
			GlewMinor = 4;
			GlewPatch = 0;
			return true;
		}
		if(GLEW_VERSION_4_3)
		{
			GlewMajor = 4;
			GlewMinor = 3;
			GlewPatch = 0;
			return true;
		}
		if(GLEW_VERSION_4_2)
		{
			GlewMajor = 4;
			GlewMinor = 2;
			GlewPatch = 0;
			return true;
		}
		if(GLEW_VERSION_4_1)
		{
			GlewMajor = 4;
			GlewMinor = 1;
			GlewPatch = 0;
			return true;
		}
		if(GLEW_VERSION_4_0)
		{
			GlewMajor = 4;
			GlewMinor = 0;
			GlewPatch = 0;
			return true;
		}
		if(GLEW_VERSION_3_3)
		{
			GlewMajor = 3;
			GlewMinor = 3;
			GlewPatch = 0;
			return true;
		}
#endif
		if(GLEW_VERSION_3_0)
		{
			GlewMajor = 3;
			GlewMinor = 0;
			GlewPatch = 0;
			return true;
		}
		if(GLEW_VERSION_2_1)
		{
			GlewMajor = 2;
			GlewMinor = 1;
			GlewPatch = 0;
			return true;
		}
		if(GLEW_VERSION_2_0)
		{
			GlewMajor = 2;
			GlewMinor = 0;
			GlewPatch = 0;
			return true;
		}
		if(GLEW_VERSION_1_5)
		{
			GlewMajor = 1;
			GlewMinor = 5;
			GlewPatch = 0;
			return true;
		}
		if(GLEW_VERSION_1_4)
		{
			GlewMajor = 1;
			GlewMinor = 4;
			GlewPatch = 0;
			return true;
		}
		if(GLEW_VERSION_1_3)
		{
			GlewMajor = 1;
			GlewMinor = 3;
			GlewPatch = 0;
			return true;
		}
		if(GLEW_VERSION_1_2_1)
		{
			GlewMajor = 1;
			GlewMinor = 2;
			GlewPatch = 1;
			return true;
		}
		if(GLEW_VERSION_1_2)
		{
			GlewMajor = 1;
			GlewMinor = 2;
			GlewPatch = 0;
			return true;
		}
		if(GLEW_VERSION_1_1)
		{
			GlewMajor = 1;
			GlewMinor = 1;
			GlewPatch = 0;
			return true;
		}
#endif
	}
	else if(BackendType == BACKEND_TYPE_OPENGL_ES)
	{
		// just assume the version we need
		GlewMajor = 3;
		GlewMinor = 0;
		GlewPatch = 0;

		return true;
	}

	return true;
}

static int IsVersionSupportedGlew(EBackendType BackendType, int VersionMajor, int VersionMinor, int VersionPatch, int GlewMajor, int GlewMinor, int GlewPatch)
{
	int InitError = 0;

	if(BackendType == BACKEND_TYPE_OPENGL)
	{
		if(VersionMajor >= 4 && GlewMajor < 4)
		{
			InitError = -1;
		}
		else if(VersionMajor >= 3 && GlewMajor < 3)
		{
			InitError = -1;
		}
		else if(VersionMajor == 3 && GlewMajor == 3)
		{
			if(VersionMinor >= 3 && GlewMinor < 3)
			{
				InitError = -1;
			}
			if(VersionMinor >= 2 && GlewMinor < 2)
			{
				InitError = -1;
			}
			if(VersionMinor >= 1 && GlewMinor < 1)
			{
				InitError = -1;
			}
			if(VersionMinor >= 0 && GlewMinor < 0)
			{
				InitError = -1;
			}
		}
		else if(VersionMajor >= 2 && GlewMajor < 2)
		{
			InitError = -1;
		}
		else if(VersionMajor == 2 && GlewMajor == 2)
		{
			if(VersionMinor >= 1 && GlewMinor < 1)
			{
				InitError = -1;
			}
			if(VersionMinor >= 0 && GlewMinor < 0)
			{
				InitError = -1;
			}
		}
		else if(VersionMajor >= 1 && GlewMajor < 1)
		{
			InitError = -1;
		}
		else if(VersionMajor == 1 && GlewMajor == 1)
		{
			if(VersionMinor >= 5 && GlewMinor < 5)
			{
				InitError = -1;
			}
			if(VersionMinor >= 4 && GlewMinor < 4)
			{
				InitError = -1;
			}
			if(VersionMinor >= 3 && GlewMinor < 3)
			{
				InitError = -1;
			}
			if(VersionMinor >= 2 && GlewMinor < 2)
			{
				InitError = -1;
			}
			else if(VersionMinor == 2 && GlewMinor == 2)
			{
				if(VersionPatch >= 1 && GlewPatch < 1)
				{
					InitError = -1;
				}
				if(VersionPatch >= 0 && GlewPatch < 0)
				{
					InitError = -1;
				}
			}
			if(VersionMinor >= 1 && GlewMinor < 1)
			{
				InitError = -1;
			}
			if(VersionMinor >= 0 && GlewMinor < 0)
			{
				InitError = -1;
			}
		}
	}

	return InitError;
}

EBackendType CGraphicsBackend_SDL_GL::DetectBackend()
{
	EBackendType RetBackendType = BACKEND_TYPE_OPENGL;
#if defined(CONF_BACKEND_VULKAN)
	const char *pEnvDriver = SDL_getenv("DDNET_DRIVER");
	if(pEnvDriver && str_comp_nocase(pEnvDriver, "GLES") == 0)
		RetBackendType = BACKEND_TYPE_OPENGL_ES;
	else if(pEnvDriver && str_comp_nocase(pEnvDriver, "Vulkan") == 0)
		RetBackendType = BACKEND_TYPE_VULKAN;
	else if(pEnvDriver && str_comp_nocase(pEnvDriver, "OpenGL") == 0)
		RetBackendType = BACKEND_TYPE_OPENGL;
	else if(pEnvDriver == nullptr)
	{
		// load the config backend
		const char *pConfBackend = g_Config.m_GfxBackend;
		if(str_comp_nocase(pConfBackend, "GLES") == 0)
			RetBackendType = BACKEND_TYPE_OPENGL_ES;
		else if(str_comp_nocase(pConfBackend, "Vulkan") == 0)
			RetBackendType = BACKEND_TYPE_VULKAN;
		else if(str_comp_nocase(pConfBackend, "OpenGL") == 0)
			RetBackendType = BACKEND_TYPE_OPENGL;
	}
#else
	RetBackendType = BACKEND_TYPE_OPENGL;
#endif
#if !defined(CONF_BACKEND_OPENGL_ES) && !defined(CONF_BACKEND_OPENGL_ES3)
	if(RetBackendType == BACKEND_TYPE_OPENGL_ES)
		RetBackendType = BACKEND_TYPE_OPENGL;
#elif defined(CONF_BACKEND_OPENGL_ES)
	if(RetBackendType == BACKEND_TYPE_OPENGL)
		RetBackendType = BACKEND_TYPE_OPENGL_ES;
#endif
	return RetBackendType;
}

void CGraphicsBackend_SDL_GL::ClampDriverVersion(EBackendType BackendType)
{
	if(BackendType == BACKEND_TYPE_OPENGL)
	{
		// clamp the versions to existing versions(only for OpenGL major <= 3)
		if(g_Config.m_GfxGLMajor == 1)
		{
			g_Config.m_GfxGLMinor = clamp(g_Config.m_GfxGLMinor, 1, 5);
			if(g_Config.m_GfxGLMinor == 2)
				g_Config.m_GfxGLPatch = clamp(g_Config.m_GfxGLPatch, 0, 1);
			else
				g_Config.m_GfxGLPatch = 0;
		}
		else if(g_Config.m_GfxGLMajor == 2)
		{
			g_Config.m_GfxGLMinor = clamp(g_Config.m_GfxGLMinor, 0, 1);
			g_Config.m_GfxGLPatch = 0;
		}
		else if(g_Config.m_GfxGLMajor == 3)
		{
			g_Config.m_GfxGLMinor = clamp(g_Config.m_GfxGLMinor, 0, 3);
			if(g_Config.m_GfxGLMinor < 3)
				g_Config.m_GfxGLMinor = 0;
			g_Config.m_GfxGLPatch = 0;
		}
	}
	else if(BackendType == BACKEND_TYPE_OPENGL_ES)
	{
#if !defined(CONF_BACKEND_OPENGL_ES3)
		// Make sure GLES is set to 1.0 (which is equivalent to OpenGL 1.3), if its not set to >= 3.0(which is equivalent to OpenGL 3.3)
		if(g_Config.m_GfxGLMajor < 3)
		{
			g_Config.m_GfxGLMajor = 1;
			g_Config.m_GfxGLMinor = 0;
			g_Config.m_GfxGLPatch = 0;

			// GLES also doesn't know GL_QUAD
			g_Config.m_GfxQuadAsTriangle = 1;
		}
#else
		g_Config.m_GfxGLMajor = 3;
		g_Config.m_GfxGLMinor = 0;
		g_Config.m_GfxGLPatch = 0;
#endif
	}
	else if(BackendType == BACKEND_TYPE_VULKAN)
	{
#if defined(CONF_BACKEND_VULKAN)
		g_Config.m_GfxGLMajor = gs_BackendVulkanMajor;
		g_Config.m_GfxGLMinor = gs_BackendVulkanMinor;
		g_Config.m_GfxGLPatch = 0;
#endif
	}
}

bool CGraphicsBackend_SDL_GL::TryCreateMsgBox(bool AsError, const char *pTitle, const char *pMsg)
{
	m_pProcessor->ErroneousCleanup();
	SDL_DestroyWindow(m_pWindow);
	SDL_ShowSimpleMessageBox(AsError ? SDL_MESSAGEBOX_ERROR : SDL_MESSAGEBOX_WARNING, pTitle, pMsg, nullptr);
	return true;
}

bool CGraphicsBackend_SDL_GL::IsModernAPI(EBackendType BackendType)
{
	if(BackendType == BACKEND_TYPE_OPENGL)
		return (g_Config.m_GfxGLMajor == 3 && g_Config.m_GfxGLMinor == 3) || g_Config.m_GfxGLMajor >= 4;
	else if(BackendType == BACKEND_TYPE_OPENGL_ES)
		return g_Config.m_GfxGLMajor >= 3;
	else if(BackendType == BACKEND_TYPE_VULKAN)
		return true;

	return false;
}

bool CGraphicsBackend_SDL_GL::GetDriverVersion(EGraphicsDriverAgeType DriverAgeType, int &Major, int &Minor, int &Patch, const char *&pName, EBackendType BackendType)
{
	if(BackendType == BACKEND_TYPE_AUTO)
		BackendType = m_BackendType;
	if(BackendType == BACKEND_TYPE_OPENGL)
	{
		pName = "OpenGL";
#ifndef CONF_BACKEND_OPENGL_ES
		if(DriverAgeType == GRAPHICS_DRIVER_AGE_TYPE_LEGACY)
		{
			Major = 1;
			Minor = 4;
			Patch = 0;
			return true;
		}
		else if(DriverAgeType == GRAPHICS_DRIVER_AGE_TYPE_DEFAULT)
		{
			Major = 3;
			Minor = 0;
			Patch = 0;
			return true;
		}
		else if(DriverAgeType == GRAPHICS_DRIVER_AGE_TYPE_MODERN)
		{
			Major = 3;
			Minor = 3;
			Patch = 0;
			return true;
		}
#endif
	}
	else if(BackendType == BACKEND_TYPE_OPENGL_ES)
	{
		pName = "GLES";
#ifdef CONF_BACKEND_OPENGL_ES
		if(DriverAgeType == GRAPHICS_DRIVER_AGE_TYPE_LEGACY)
		{
			Major = 1;
			Minor = 0;
			Patch = 0;
			return true;
		}
		else if(DriverAgeType == GRAPHICS_DRIVER_AGE_TYPE_DEFAULT)
		{
			Major = 3;
			Minor = 0;
			Patch = 0;
			// there isn't really a default one
			return false;
		}
#endif
#ifdef CONF_BACKEND_OPENGL_ES3
		if(DriverAgeType == GRAPHICS_DRIVER_AGE_TYPE_MODERN)
		{
			Major = 3;
			Minor = 0;
			Patch = 0;
			return true;
		}
#endif
	}
	else if(BackendType == BACKEND_TYPE_VULKAN)
	{
		pName = "Vulkan";
#ifdef CONF_BACKEND_VULKAN
		if(DriverAgeType == GRAPHICS_DRIVER_AGE_TYPE_DEFAULT)
		{
			Major = gs_BackendVulkanMajor;
			Minor = gs_BackendVulkanMinor;
			Patch = 0;
			return true;
		}
#else
		return false;
#endif
	}
	return false;
}

static void DisplayToVideoMode(CVideoMode *pVMode, SDL_DisplayMode *pMode, int HiDPIScale, int RefreshRate)
{
	pVMode->m_CanvasWidth = pMode->w * HiDPIScale;
	pVMode->m_CanvasHeight = pMode->h * HiDPIScale;
	pVMode->m_WindowWidth = pMode->w;
	pVMode->m_WindowHeight = pMode->h;
	pVMode->m_RefreshRate = RefreshRate;
	pVMode->m_Red = SDL_BITSPERPIXEL(pMode->format);
	pVMode->m_Green = SDL_BITSPERPIXEL(pMode->format);
	pVMode->m_Blue = SDL_BITSPERPIXEL(pMode->format);
	pVMode->m_Format = pMode->format;
}

void CGraphicsBackend_SDL_GL::GetVideoModes(CVideoMode *pModes, int MaxModes, int *pNumModes, int HiDPIScale, int MaxWindowWidth, int MaxWindowHeight, int ScreenID)
{
	SDL_DisplayMode DesktopMode;
	int maxModes = SDL_GetNumDisplayModes(ScreenID);
	int numModes = 0;

	// Only collect fullscreen modes when requested, that makes sure in windowed mode no refresh rates are shown that aren't supported without
	// fullscreen anyway(except fullscreen desktop)
	bool IsFullscreenDestkop = m_pWindow != NULL && (((SDL_GetWindowFlags(m_pWindow) & SDL_WINDOW_FULLSCREEN_DESKTOP) == SDL_WINDOW_FULLSCREEN_DESKTOP) || g_Config.m_GfxFullscreen == 3);
	bool CollectFullscreenModes = m_pWindow == NULL || ((SDL_GetWindowFlags(m_pWindow) & SDL_WINDOW_FULLSCREEN) != 0 && !IsFullscreenDestkop);

	if(SDL_GetDesktopDisplayMode(ScreenID, &DesktopMode) < 0)
	{
		dbg_msg("gfx", "unable to get display mode: %s", SDL_GetError());
	}

	constexpr int ModeCount = 256;
	SDL_DisplayMode aModes[ModeCount];
	int NumModes = 0;
	for(int i = 0; i < maxModes && NumModes < ModeCount; i++)
	{
		SDL_DisplayMode mode;
		if(SDL_GetDisplayMode(ScreenID, i, &mode) < 0)
		{
			dbg_msg("gfx", "unable to get display mode: %s", SDL_GetError());
			continue;
		}

		aModes[NumModes] = mode;
		++NumModes;
	}

	auto &&ModeInsert = [&](SDL_DisplayMode &mode) {
		if(numModes < MaxModes)
		{
			// if last mode was equal, ignore this one --- in fullscreen this can really only happen if the screen
			// supports different color modes
			// in non fullscren these are the modes that show different refresh rate, but are basically the same
			if(numModes > 0 && pModes[numModes - 1].m_WindowWidth == mode.w && pModes[numModes - 1].m_WindowHeight == mode.h && (pModes[numModes - 1].m_RefreshRate == mode.refresh_rate || (mode.refresh_rate != DesktopMode.refresh_rate && !CollectFullscreenModes)))
				return;

			DisplayToVideoMode(&pModes[numModes], &mode, HiDPIScale, !CollectFullscreenModes ? DesktopMode.refresh_rate : mode.refresh_rate);
			numModes++;
		}
	};

	for(int i = 0; i < NumModes; i++)
	{
		SDL_DisplayMode &mode = aModes[i];

		if(mode.w > MaxWindowWidth || mode.h > MaxWindowHeight)
			continue;

		ModeInsert(mode);

		if(IsFullscreenDestkop)
			break;

		if(numModes >= MaxModes)
			break;
	}
	*pNumModes = numModes;
}

void CGraphicsBackend_SDL_GL::GetCurrentVideoMode(CVideoMode &CurMode, int HiDPIScale, int MaxWindowWidth, int MaxWindowHeight, int ScreenID)
{
	SDL_DisplayMode DPMode;
	// if "real" fullscreen, obtain the video mode for that
	if((SDL_GetWindowFlags(m_pWindow) & SDL_WINDOW_FULLSCREEN_DESKTOP) == SDL_WINDOW_FULLSCREEN)
	{
		if(SDL_GetCurrentDisplayMode(ScreenID, &DPMode))
		{
			dbg_msg("gfx", "unable to get display mode: %s", SDL_GetError());
		}
	}
	else
	{
		if(SDL_GetDesktopDisplayMode(ScreenID, &DPMode) < 0)
		{
			dbg_msg("gfx", "unable to get display mode: %s", SDL_GetError());
		}
		else
		{
			int Width = 0;
			int Height = 0;
			SDL_GL_GetDrawableSize(m_pWindow, &Width, &Height);
			DPMode.w = Width;
			DPMode.h = Height;
		}
	}
	DisplayToVideoMode(&CurMode, &DPMode, HiDPIScale, DPMode.refresh_rate);
}

CGraphicsBackend_SDL_GL::CGraphicsBackend_SDL_GL(TTranslateFunc &&TranslateFunc) :
	CGraphicsBackend_Threaded(std::move(TranslateFunc))
{
	mem_zero(m_aErrorString, std::size(m_aErrorString));
}

int CGraphicsBackend_SDL_GL::Init(const char *pName, int *pScreen, int *pWidth, int *pHeight, int *pRefreshRate, int *pFsaaSamples, int Flags, int *pDesktopWidth, int *pDesktopHeight, int *pCurrentWidth, int *pCurrentHeight, IStorage *pStorage)
{
#if defined(CONF_HEADLESS_CLIENT)
	int InitError = 0;
	const char *pErrorStr = NULL;
	int GlewMajor = 0;
	int GlewMinor = 0;
	int GlewPatch = 0;
	IsVersionSupportedGlew(m_BackendType, g_Config.m_GfxGLMajor, g_Config.m_GfxGLMinor, g_Config.m_GfxGLPatch, GlewMajor, GlewMinor, GlewPatch);
	BackendInitGlew(m_BackendType, GlewMajor, GlewMinor, GlewPatch);
#else
	// print sdl version
	{
		SDL_version Compiled;
		SDL_version Linked;

		SDL_VERSION(&Compiled);
		SDL_GetVersion(&Linked);
		dbg_msg("sdl", "SDL version %d.%d.%d (compiled = %d.%d.%d)", Linked.major, Linked.minor, Linked.patch,
			Compiled.major, Compiled.minor, Compiled.patch);
	}

	if(!SDL_WasInit(SDL_INIT_VIDEO))
	{
		if(SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
		{
			dbg_msg("gfx", "unable to init SDL video: %s", SDL_GetError());
			return EGraphicsBackendErrorCodes::GRAPHICS_BACKEND_ERROR_CODE_SDL_INIT_FAILED;
		}
	}

	EBackendType OldBackendType = m_BackendType;
	m_BackendType = DetectBackend();
	// little fallback for Vulkan
	if(OldBackendType != BACKEND_TYPE_AUTO)
	{
		if(m_BackendType == BACKEND_TYPE_VULKAN)
		{
			// try default opengl settings
			str_copy(g_Config.m_GfxBackend, "OpenGL");
			g_Config.m_GfxGLMajor = 3;
			g_Config.m_GfxGLMinor = 0;
			g_Config.m_GfxGLPatch = 0;
			// do another analysis round too, just in case
			g_Config.m_Gfx3DTextureAnalysisRan = 0;
			g_Config.m_GfxDriverIsBlocked = 0;

			SDL_setenv("DDNET_DRIVER", "OpenGL", 1);
			m_BackendType = DetectBackend();
		}
	}

	ClampDriverVersion(m_BackendType);

	bool UseModernGL = IsModernAPI(m_BackendType);

	bool IsOpenGLFamilyBackend = m_BackendType == BACKEND_TYPE_OPENGL || m_BackendType == BACKEND_TYPE_OPENGL_ES;

	if(IsOpenGLFamilyBackend)
	{
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, g_Config.m_GfxGLMajor);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, g_Config.m_GfxGLMinor);
	}

	dbg_msg("gfx", "Created %s %d.%d context.", ((m_BackendType == BACKEND_TYPE_VULKAN) ? "Vulkan" : "OpenGL"), g_Config.m_GfxGLMajor, g_Config.m_GfxGLMinor);

	if(m_BackendType == BACKEND_TYPE_OPENGL)
	{
		if(g_Config.m_GfxGLMajor == 3 && g_Config.m_GfxGLMinor == 0)
		{
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
		}
		else if(UseModernGL)
		{
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		}
	}
	else if(m_BackendType == BACKEND_TYPE_OPENGL_ES)
	{
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
	}

	if(IsOpenGLFamilyBackend)
	{
		*pFsaaSamples = std::clamp(*pFsaaSamples, 0, 8);
	}

	// set screen
	SDL_Rect ScreenPos;
	m_NumScreens = SDL_GetNumVideoDisplays();
	if(m_NumScreens > 0)
	{
		*pScreen = clamp(*pScreen, 0, m_NumScreens - 1);
		if(SDL_GetDisplayBounds(*pScreen, &ScreenPos) != 0)
		{
			dbg_msg("gfx", "unable to retrieve screen information: %s", SDL_GetError());
			return EGraphicsBackendErrorCodes::GRAPHICS_BACKEND_ERROR_CODE_SDL_SCREEN_INFO_REQUEST_FAILED;
		}
	}
	else
	{
		dbg_msg("gfx", "unable to retrieve number of screens: %s", SDL_GetError());
		return EGraphicsBackendErrorCodes::GRAPHICS_BACKEND_ERROR_CODE_SDL_SCREEN_REQUEST_FAILED;
	}

	// store desktop resolution for settings reset button
	SDL_DisplayMode DisplayMode;
	if(SDL_GetDesktopDisplayMode(*pScreen, &DisplayMode))
	{
		dbg_msg("gfx", "unable to get desktop resolution: %s", SDL_GetError());
		return EGraphicsBackendErrorCodes::GRAPHICS_BACKEND_ERROR_CODE_SDL_SCREEN_RESOLUTION_REQUEST_FAILED;
	}

	bool IsDesktopChanged = *pDesktopWidth == 0 || *pDesktopHeight == 0 || *pDesktopWidth != DisplayMode.w || *pDesktopHeight != DisplayMode.h;

	*pDesktopWidth = DisplayMode.w;
	*pDesktopHeight = DisplayMode.h;

	// fetch supported video modes
	bool SupportedResolution = false;

	CVideoMode aModes[256];
	int ModesCount = 0;
	int IndexOfResolution = -1;
	GetVideoModes(aModes, std::size(aModes), &ModesCount, 1, *pDesktopWidth, *pDesktopHeight, *pScreen);

	for(int i = 0; i < ModesCount; i++)
	{
		if(*pWidth == aModes[i].m_WindowWidth && *pHeight == aModes[i].m_WindowHeight && (*pRefreshRate == aModes[i].m_RefreshRate || *pRefreshRate == 0))
		{
			SupportedResolution = true;
			IndexOfResolution = i;
			break;
		}
	}

	// set flags
	int SdlFlags = SDL_WINDOW_INPUT_GRABBED | SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_MOUSE_FOCUS;
	SdlFlags |= (IsOpenGLFamilyBackend) ? SDL_WINDOW_OPENGL : SDL_WINDOW_VULKAN;
	if(Flags & IGraphicsBackend::INITFLAG_HIGHDPI)
		SdlFlags |= SDL_WINDOW_ALLOW_HIGHDPI;
	if(Flags & IGraphicsBackend::INITFLAG_RESIZABLE)
		SdlFlags |= SDL_WINDOW_RESIZABLE;
	if(Flags & IGraphicsBackend::INITFLAG_BORDERLESS)
		SdlFlags |= SDL_WINDOW_BORDERLESS;
	if(Flags & IGraphicsBackend::INITFLAG_FULLSCREEN)
		SdlFlags |= SDL_WINDOW_FULLSCREEN;
	else if(Flags & (IGraphicsBackend::INITFLAG_DESKTOP_FULLSCREEN))
		SdlFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;

	bool IsFullscreen = (SdlFlags & SDL_WINDOW_FULLSCREEN) != 0 || g_Config.m_GfxFullscreen == 3;
	// use desktop resolution as default resolution, clamp resolution if users's display is smaller than we remembered
	// if the user starts in fullscreen, and the resolution was not found use the desktop one
	if((IsFullscreen && !SupportedResolution) || *pWidth == 0 || *pHeight == 0 || (IsDesktopChanged && (!SupportedResolution || !IsFullscreen) && (*pWidth > *pDesktopWidth || *pHeight > *pDesktopHeight)))
	{
		*pWidth = *pDesktopWidth;
		*pHeight = *pDesktopHeight;
		*pRefreshRate = DisplayMode.refresh_rate;
	}

	// if in fullscreen and refresh rate wasn't set yet, just use the one from the found list
	if(*pRefreshRate == 0 && SupportedResolution)
	{
		*pRefreshRate = aModes[IndexOfResolution].m_RefreshRate;
	}
	else if(*pRefreshRate == 0)
	{
		*pRefreshRate = DisplayMode.refresh_rate;
	}

	// set gl attributes
	if(IsOpenGLFamilyBackend)
	{
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		if(*pFsaaSamples)
		{
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, *pFsaaSamples);
		}
		else
		{
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
		}
	}

	if(g_Config.m_InpMouseOld)
		SDL_SetHint(SDL_HINT_MOUSE_RELATIVE_MODE_WARP, "1");

	m_pWindow = SDL_CreateWindow(
		pName,
		SDL_WINDOWPOS_CENTERED_DISPLAY(*pScreen),
		SDL_WINDOWPOS_CENTERED_DISPLAY(*pScreen),
		*pWidth,
		*pHeight,
		SdlFlags);

	// set caption
	if(m_pWindow == NULL)
	{
		dbg_msg("gfx", "unable to create window: %s", SDL_GetError());
		if(m_BackendType == BACKEND_TYPE_VULKAN)
			return EGraphicsBackendErrorCodes::GRAPHICS_BACKEND_ERROR_CODE_GL_CONTEXT_FAILED;
		else
			return EGraphicsBackendErrorCodes::GRAPHICS_BACKEND_ERROR_CODE_SDL_WINDOW_CREATE_FAILED;
	}

	int GlewMajor = 0;
	int GlewMinor = 0;
	int GlewPatch = 0;

	if(IsOpenGLFamilyBackend)
	{
		m_GLContext = SDL_GL_CreateContext(m_pWindow);

		if(m_GLContext == NULL)
		{
			SDL_DestroyWindow(m_pWindow);
			dbg_msg("gfx", "unable to create graphic context: %s", SDL_GetError());
			return EGraphicsBackendErrorCodes::GRAPHICS_BACKEND_ERROR_CODE_GL_CONTEXT_FAILED;
		}

		if(!BackendInitGlew(m_BackendType, GlewMajor, GlewMinor, GlewPatch))
		{
			SDL_GL_DeleteContext(m_GLContext);
			SDL_DestroyWindow(m_pWindow);
			return EGraphicsBackendErrorCodes::GRAPHICS_BACKEND_ERROR_CODE_UNKNOWN;
		}
	}

	int InitError = 0;
	const char *pErrorStr = NULL;

	InitError = IsVersionSupportedGlew(m_BackendType, g_Config.m_GfxGLMajor, g_Config.m_GfxGLMinor, g_Config.m_GfxGLPatch, GlewMajor, GlewMinor, GlewPatch);

	// SDL_GL_GetDrawableSize reports HiDPI resolution even with SDL_WINDOW_ALLOW_HIGHDPI not set, which is wrong
	if(SdlFlags & SDL_WINDOW_ALLOW_HIGHDPI && IsOpenGLFamilyBackend)
		SDL_GL_GetDrawableSize(m_pWindow, pCurrentWidth, pCurrentHeight);
	else
		SDL_GetWindowSize(m_pWindow, pCurrentWidth, pCurrentHeight);

	if(IsOpenGLFamilyBackend)
	{
		SDL_GL_SetSwapInterval(Flags & IGraphicsBackend::INITFLAG_VSYNC ? 1 : 0);
		SDL_GL_MakeCurrent(NULL, NULL);
	}

	if(InitError != 0)
	{
		if(m_GLContext)
			SDL_GL_DeleteContext(m_GLContext);
		SDL_DestroyWindow(m_pWindow);

		// try setting to glew supported version
		g_Config.m_GfxGLMajor = GlewMajor;
		g_Config.m_GfxGLMinor = GlewMinor;
		g_Config.m_GfxGLPatch = GlewPatch;

		return EGraphicsBackendErrorCodes::GRAPHICS_BACKEND_ERROR_CODE_GL_VERSION_FAILED;
	}
#endif // CONF_HEADLESS_CLIENT

	// start the command processor
	dbg_assert(m_pProcessor == nullptr, "Processor was not cleaned up properly.");
	m_pProcessor = new CCommandProcessor_SDL_GL(m_BackendType, g_Config.m_GfxGLMajor, g_Config.m_GfxGLMinor, g_Config.m_GfxGLPatch);
	StartProcessor(m_pProcessor);

	// issue init commands for OpenGL and SDL
	CCommandBuffer CmdBuffer(1024, 512);
	CCommandProcessorFragment_GLBase::SCommand_PreInit CmdPre;
	CmdPre.m_pWindow = m_pWindow;
	CmdPre.m_Width = *pCurrentWidth;
	CmdPre.m_Height = *pCurrentHeight;
	CmdPre.m_pVendorString = m_aVendorString;
	CmdPre.m_pVersionString = m_aVersionString;
	CmdPre.m_pRendererString = m_aRendererString;
	CmdPre.m_pGPUList = &m_GPUList;
	CmdBuffer.AddCommandUnsafe(CmdPre);
	RunBufferSingleThreadedUnsafe(&CmdBuffer);
	CmdBuffer.Reset();

	// run sdl first to have the context in the thread
	CCommandProcessorFragment_SDL::SCommand_Init CmdSDL;
	CmdSDL.m_pWindow = m_pWindow;
	CmdSDL.m_GLContext = m_GLContext;
	CmdBuffer.AddCommandUnsafe(CmdSDL);
	RunBuffer(&CmdBuffer);
	WaitForIdle();
	CmdBuffer.Reset();

	if(InitError == 0)
	{
		CCommandProcessorFragment_GLBase::SCommand_Init CmdGL;
		CmdGL.m_pWindow = m_pWindow;
		CmdGL.m_Width = *pCurrentWidth;
		CmdGL.m_Height = *pCurrentHeight;
		CmdGL.m_pTextureMemoryUsage = &m_TextureMemoryUsage;
		CmdGL.m_pBufferMemoryUsage = &m_BufferMemoryUsage;
		CmdGL.m_pStreamMemoryUsage = &m_StreamMemoryUsage;
		CmdGL.m_pStagingMemoryUsage = &m_StagingMemoryUsage;
		CmdGL.m_pGPUList = &m_GPUList;
		CmdGL.m_pReadPresentedImageDataFunc = &m_ReadPresentedImageDataFunc;
		CmdGL.m_pStorage = pStorage;
		CmdGL.m_pCapabilities = &m_Capabilites;
		CmdGL.m_pInitError = &InitError;
		CmdGL.m_RequestedMajor = g_Config.m_GfxGLMajor;
		CmdGL.m_RequestedMinor = g_Config.m_GfxGLMinor;
		CmdGL.m_RequestedPatch = g_Config.m_GfxGLPatch;
		CmdGL.m_GlewMajor = GlewMajor;
		CmdGL.m_GlewMinor = GlewMinor;
		CmdGL.m_GlewPatch = GlewPatch;
		CmdGL.m_pErrStringPtr = &pErrorStr;
		CmdGL.m_pVendorString = m_aVendorString;
		CmdGL.m_pVersionString = m_aVersionString;
		CmdGL.m_pRendererString = m_aRendererString;
		CmdGL.m_RequestedBackend = m_BackendType;
		CmdBuffer.AddCommandUnsafe(CmdGL);

		RunBuffer(&CmdBuffer);
		WaitForIdle();
		CmdBuffer.Reset();
	}

	if(InitError != 0)
	{
		if(InitError != -2)
		{
			// shutdown the context, as it might have been initialized
			CCommandProcessorFragment_GLBase::SCommand_Shutdown CmdGL;
			CmdBuffer.AddCommandUnsafe(CmdGL);
			RunBuffer(&CmdBuffer);
			WaitForIdle();
			CmdBuffer.Reset();
		}

		CCommandProcessorFragment_SDL::SCommand_Shutdown Cmd;
		CmdBuffer.AddCommandUnsafe(Cmd);
		RunBuffer(&CmdBuffer);
		WaitForIdle();
		CmdBuffer.Reset();

		CCommandProcessorFragment_GLBase::SCommand_PostShutdown CmdPost;
		CmdBuffer.AddCommandUnsafe(CmdPost);
		RunBufferSingleThreadedUnsafe(&CmdBuffer);
		CmdBuffer.Reset();

		// stop and delete the processor
		StopProcessor();
		delete m_pProcessor;
		m_pProcessor = nullptr;

		if(m_GLContext)
			SDL_GL_DeleteContext(m_GLContext);
		SDL_DestroyWindow(m_pWindow);

		// try setting to version string's supported version
		if(InitError == -2)
		{
			g_Config.m_GfxGLMajor = m_Capabilites.m_ContextMajor;
			g_Config.m_GfxGLMinor = m_Capabilites.m_ContextMinor;
			g_Config.m_GfxGLPatch = m_Capabilites.m_ContextPatch;
		}

		if(pErrorStr != NULL)
		{
			str_copy(m_aErrorString, pErrorStr);
		}

		return EGraphicsBackendErrorCodes::GRAPHICS_BACKEND_ERROR_CODE_GL_VERSION_FAILED;
	}

	{
		CCommandBuffer::SCommand_Update_Viewport CmdSDL2;
		CmdSDL2.m_X = 0;
		CmdSDL2.m_Y = 0;

		CmdSDL2.m_Width = *pCurrentWidth;
		CmdSDL2.m_Height = *pCurrentHeight;
		CmdSDL2.m_ByResize = true;
		CmdBuffer.AddCommandUnsafe(CmdSDL2);
		RunBuffer(&CmdBuffer);
		WaitForIdle();
		CmdBuffer.Reset();
	}

	// return
	return EGraphicsBackendErrorCodes::GRAPHICS_BACKEND_ERROR_CODE_NONE;
}

int CGraphicsBackend_SDL_GL::Shutdown()
{
	// issue a shutdown command
	CCommandBuffer CmdBuffer(1024, 512);
	CCommandProcessorFragment_GLBase::SCommand_Shutdown CmdGL;
	CmdBuffer.AddCommandUnsafe(CmdGL);
	RunBuffer(&CmdBuffer);
	WaitForIdle();
	CmdBuffer.Reset();

	CCommandProcessorFragment_SDL::SCommand_Shutdown Cmd;
	CmdBuffer.AddCommandUnsafe(Cmd);
	RunBuffer(&CmdBuffer);
	WaitForIdle();
	CmdBuffer.Reset();

	CCommandProcessorFragment_GLBase::SCommand_PostShutdown CmdPost;
	CmdBuffer.AddCommandUnsafe(CmdPost);
	RunBufferSingleThreadedUnsafe(&CmdBuffer);
	CmdBuffer.Reset();

	// stop and delete the processor
	StopProcessor();
	delete m_pProcessor;
	m_pProcessor = nullptr;

	if(m_GLContext != nullptr)
		SDL_GL_DeleteContext(m_GLContext);
	SDL_DestroyWindow(m_pWindow);

	SDL_QuitSubSystem(SDL_INIT_VIDEO);
	return 0;
}

uint64_t CGraphicsBackend_SDL_GL::TextureMemoryUsage() const
{
	return m_TextureMemoryUsage;
}

uint64_t CGraphicsBackend_SDL_GL::BufferMemoryUsage() const
{
	return m_BufferMemoryUsage;
}

uint64_t CGraphicsBackend_SDL_GL::StreamedMemoryUsage() const
{
	return m_StreamMemoryUsage;
}

uint64_t CGraphicsBackend_SDL_GL::StagingMemoryUsage() const
{
	return m_StagingMemoryUsage;
}

const TTWGraphicsGPUList &CGraphicsBackend_SDL_GL::GetGPUs() const
{
	return m_GPUList;
}

void CGraphicsBackend_SDL_GL::Minimize()
{
	SDL_MinimizeWindow(m_pWindow);
}

void CGraphicsBackend_SDL_GL::Maximize()
{
	// TODO: SDL
}

void CGraphicsBackend_SDL_GL::SetWindowParams(int FullscreenMode, bool IsBorderless, bool AllowResizing)
{
	if(FullscreenMode > 0)
	{
		bool IsDesktopFullscreen = FullscreenMode == 2;
#ifndef CONF_FAMILY_WINDOWS
		//  special mode for windows only
		IsDesktopFullscreen |= FullscreenMode == 3;
#endif
		if(FullscreenMode == 1)
		{
#if defined(CONF_PLATFORM_MACOS) || defined(CONF_PLATFORM_HAIKU)
			// Todo SDL: remove this when fixed (game freezes when losing focus in fullscreen)
			SDL_SetWindowFullscreen(m_pWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);
#else
			SDL_SetWindowFullscreen(m_pWindow, SDL_WINDOW_FULLSCREEN);
#endif
			SDL_SetWindowResizable(m_pWindow, SDL_TRUE);
		}
		else if(IsDesktopFullscreen)
		{
			SDL_SetWindowFullscreen(m_pWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);
			SDL_SetWindowResizable(m_pWindow, SDL_TRUE);
		}
		else
		{
			SDL_SetWindowFullscreen(m_pWindow, 0);
			SDL_SetWindowBordered(m_pWindow, SDL_TRUE);
			SDL_SetWindowResizable(m_pWindow, SDL_FALSE);
			SDL_DisplayMode DPMode;
			if(SDL_GetDesktopDisplayMode(g_Config.m_GfxScreen, &DPMode) < 0)
			{
				dbg_msg("gfx", "unable to get display mode: %s", SDL_GetError());
			}
			else
			{
				ResizeWindow(DPMode.w, DPMode.h, DPMode.refresh_rate);
				SDL_SetWindowPosition(m_pWindow, SDL_WINDOWPOS_CENTERED_DISPLAY(g_Config.m_GfxScreen), SDL_WINDOWPOS_CENTERED_DISPLAY(g_Config.m_GfxScreen));
			}
		}
	}
	else
	{
		SDL_SetWindowFullscreen(m_pWindow, 0);
		SDL_SetWindowBordered(m_pWindow, SDL_bool(!IsBorderless));
		SDL_SetWindowResizable(m_pWindow, SDL_TRUE);
	}
}

bool CGraphicsBackend_SDL_GL::SetWindowScreen(int Index)
{
	if(Index < 0 || Index >= m_NumScreens)
	{
		return false;
	}

	SDL_Rect ScreenPos;
	if(SDL_GetDisplayBounds(Index, &ScreenPos) != 0)
	{
		return false;
	}

	SDL_SetWindowPosition(m_pWindow,
		SDL_WINDOWPOS_CENTERED_DISPLAY(Index),
		SDL_WINDOWPOS_CENTERED_DISPLAY(Index));

	return UpdateDisplayMode(Index);
}

bool CGraphicsBackend_SDL_GL::UpdateDisplayMode(int Index)
{
	SDL_DisplayMode DisplayMode;
	if(SDL_GetDesktopDisplayMode(Index, &DisplayMode) < 0)
	{
		dbg_msg("gfx", "unable to get display mode: %s", SDL_GetError());
		return false;
	}

	g_Config.m_GfxDesktopWidth = DisplayMode.w;
	g_Config.m_GfxDesktopHeight = DisplayMode.h;

	return true;
}

int CGraphicsBackend_SDL_GL::GetWindowScreen()
{
	return SDL_GetWindowDisplayIndex(m_pWindow);
}

int CGraphicsBackend_SDL_GL::WindowActive()
{
	return m_pWindow && SDL_GetWindowFlags(m_pWindow) & SDL_WINDOW_INPUT_FOCUS;
}

int CGraphicsBackend_SDL_GL::WindowOpen()
{
	return m_pWindow && SDL_GetWindowFlags(m_pWindow) & SDL_WINDOW_SHOWN;
}

void CGraphicsBackend_SDL_GL::SetWindowGrab(bool Grab)
{
	SDL_SetWindowGrab(m_pWindow, Grab ? SDL_TRUE : SDL_FALSE);
}

bool CGraphicsBackend_SDL_GL::ResizeWindow(int w, int h, int RefreshRate)
{
	// don't call resize events when the window is at fullscreen desktop
	if(!m_pWindow || (SDL_GetWindowFlags(m_pWindow) & SDL_WINDOW_FULLSCREEN_DESKTOP) == SDL_WINDOW_FULLSCREEN_DESKTOP)
		return false;

	// if the window is at fullscreen use SDL_SetWindowDisplayMode instead, suggested by SDL
	if(SDL_GetWindowFlags(m_pWindow) & SDL_WINDOW_FULLSCREEN)
	{
#ifdef CONF_FAMILY_WINDOWS
		// in windows make the window windowed mode first, this prevents strange window glitches (other games probably do something similar)
		SetWindowParams(0, true, true);
#endif
		SDL_DisplayMode SetMode = {};
		SDL_DisplayMode ClosestMode = {};
		SetMode.format = 0;
		SetMode.w = w;
		SetMode.h = h;
		SetMode.refresh_rate = RefreshRate;
		SDL_SetWindowDisplayMode(m_pWindow, SDL_GetClosestDisplayMode(g_Config.m_GfxScreen, &SetMode, &ClosestMode));
#ifdef CONF_FAMILY_WINDOWS
		// now change it back to fullscreen, this will restore the above set state, bcs SDL saves fullscreen modes apart from other video modes (as of SDL 2.0.16)
		// see implementation of SDL_SetWindowDisplayMode
		SetWindowParams(1, false, true);
#endif
		return true;
	}
	else
	{
		SDL_SetWindowSize(m_pWindow, w, h);
		if(SDL_GetWindowFlags(m_pWindow) & SDL_WINDOW_MAXIMIZED)
			// remove maximize flag
			SDL_RestoreWindow(m_pWindow);
	}

	return false;
}

void CGraphicsBackend_SDL_GL::GetViewportSize(int &w, int &h)
{
	SDL_GL_GetDrawableSize(m_pWindow, &w, &h);
}

void CGraphicsBackend_SDL_GL::NotifyWindow()
{
#if SDL_MAJOR_VERSION > 2 || (SDL_MAJOR_VERSION == 2 && SDL_PATCHLEVEL >= 16)
	if(SDL_FlashWindow(m_pWindow, SDL_FlashOperation::SDL_FLASH_UNTIL_FOCUSED) != 0)
	{
		// fails if SDL hasn't implemented it
		return;
	}
#endif
}

void CGraphicsBackend_SDL_GL::WindowDestroyNtf(uint32_t WindowID)
{
}

void CGraphicsBackend_SDL_GL::WindowCreateNtf(uint32_t WindowID)
{
	m_pWindow = SDL_GetWindowFromID(WindowID);
}

TGLBackendReadPresentedImageData &CGraphicsBackend_SDL_GL::GetReadPresentedImageDataFuncUnsafe()
{
	return m_ReadPresentedImageDataFunc;
}

IGraphicsBackend *CreateGraphicsBackend(TTranslateFunc &&TranslateFunc) { return new CGraphicsBackend_SDL_GL(std::move(TranslateFunc)); }
