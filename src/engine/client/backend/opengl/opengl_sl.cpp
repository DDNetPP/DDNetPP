#include "opengl_sl.h"

#include <base/detect.h>
#include <base/log.h>

#if defined(BACKEND_AS_OPENGL_ES) || !defined(CONF_BACKEND_OPENGL_ES)

#include <engine/client/backend/glsl_shader_compiler.h>
#include <engine/graphics.h>
#include <engine/shared/linereader.h>
#include <engine/storage.h>

#include <cstdio>
#include <string>
#include <vector>

#ifndef BACKEND_AS_OPENGL_ES
#include <GL/glew.h>
#else
#include <GLES3/gl3.h>
#endif

bool CGLSL::LoadShader(CGLSLCompiler *pCompiler, IStorage *pStorage, const char *pFile, int Type)
{
	if(m_IsLoaded)
		return true;

	CLineReader LineReader;
	std::vector<std::string> vLines;
	if(!LineReader.OpenFile(pStorage->OpenFile(pFile, IOFLAG_READ, IStorage::TYPE_ALL)))
	{
		return false;
	}

	EBackendType BackendType = pCompiler->m_IsOpenGLES ? BACKEND_TYPE_OPENGL_ES : BACKEND_TYPE_OPENGL;
	bool IsNewOpenGL = (BackendType == BACKEND_TYPE_OPENGL ? (pCompiler->m_OpenGLVersionMajor >= 4 || (pCompiler->m_OpenGLVersionMajor == 3 && pCompiler->m_OpenGLVersionMinor == 3)) : pCompiler->m_OpenGLVersionMajor >= 3);
	std::string GLShaderStringPostfix = std::string(" core\r\n");
	if(BackendType == BACKEND_TYPE_OPENGL_ES)
		GLShaderStringPostfix = std::string(" es\r\n");
	//add compiler specific values
	if(IsNewOpenGL)
		vLines.push_back(std::string("#version ") + std::string(std::to_string(pCompiler->m_OpenGLVersionMajor)) + std::string(std::to_string(pCompiler->m_OpenGLVersionMinor)) + std::string(std::to_string(pCompiler->m_OpenGLVersionPatch)) + GLShaderStringPostfix);
	else
	{
		if(pCompiler->m_OpenGLVersionMajor == 3)
		{
			if(pCompiler->m_OpenGLVersionMinor == 0)
				vLines.emplace_back("#version 130 \r\n");
			if(pCompiler->m_OpenGLVersionMinor == 1)
				vLines.emplace_back("#version 140 \r\n");
			if(pCompiler->m_OpenGLVersionMinor == 2)
				vLines.emplace_back("#version 150 \r\n");
		}
		else if(pCompiler->m_OpenGLVersionMajor == 2)
		{
			if(pCompiler->m_OpenGLVersionMinor == 0)
				vLines.emplace_back("#version 110 \r\n");
			if(pCompiler->m_OpenGLVersionMinor == 1)
				vLines.emplace_back("#version 120 \r\n");
		}
	}

	if(BackendType == BACKEND_TYPE_OPENGL_ES)
	{
		if(Type == GL_FRAGMENT_SHADER)
		{
			vLines.emplace_back("precision highp float; \r\n");
			vLines.emplace_back("precision highp sampler2D; \r\n");
			vLines.emplace_back("precision highp sampler3D; \r\n");
			vLines.emplace_back("precision highp samplerCube; \r\n");
			vLines.emplace_back("precision highp samplerCubeShadow; \r\n");
			vLines.emplace_back("precision highp sampler2DShadow; \r\n");
			vLines.emplace_back("precision highp sampler2DArray; \r\n");
			vLines.emplace_back("precision highp sampler2DArrayShadow; \r\n");
		}
	}

	for(const CGLSLCompiler::SGLSLCompilerDefine &Define : pCompiler->m_vDefines)
	{
		vLines.push_back(std::string("#define ") + Define.m_DefineName + std::string(" ") + Define.m_DefineValue + std::string("\r\n"));
	}

	if(Type == GL_FRAGMENT_SHADER && !IsNewOpenGL && pCompiler->m_OpenGLVersionMajor <= 3 && pCompiler->m_HasTextureArray)
	{
		vLines.emplace_back("#extension GL_EXT_texture_array : enable\r\n");
	}

	while(const char *pReadLine = LineReader.Get())
	{
		std::string Line;
		pCompiler->ParseLine(Line, pReadLine, Type == GL_FRAGMENT_SHADER ? GLSL_SHADER_COMPILER_TYPE_FRAGMENT : GLSL_SHADER_COMPILER_TYPE_VERTEX);
		Line.append("\r\n");
		vLines.push_back(Line);
	}

	const char **ShaderCode = new const char *[vLines.size()];

	for(size_t i = 0; i < vLines.size(); ++i)
	{
		ShaderCode[i] = vLines[i].c_str();
	}

	const TWGLuint ShaderId = glCreateShader(Type);

	glShaderSource(ShaderId, vLines.size(), ShaderCode, NULL);
	glCompileShader(ShaderId);

	delete[] ShaderCode;

	TWGLint CompilationStatus;
	glGetShaderiv(ShaderId, GL_COMPILE_STATUS, &CompilationStatus);

	if(CompilationStatus == GL_FALSE)
	{
		TWGLint LogLength = 0;
		glGetShaderiv(ShaderId, GL_INFO_LOG_LENGTH, &LogLength);
		if(LogLength > 0)
		{
			std::string Log(LogLength, '\0');
			glGetShaderInfoLog(ShaderId, Log.size(), nullptr, &Log.front());
			if(Log[Log.size() - 2] == '\n')
			{
				Log[Log.size() - 2] = '\0';
			}
			log_error("gfx/opengl/shader", "Failed to compile shader file '%s'. The compiler returned:\n%s", pFile, Log.c_str());
		}
		else
		{
			log_error("gfx/opengl/shader", "Failed to compile shader file '%s'. The compiler did not return an error.", pFile);
		}
		glDeleteShader(ShaderId);
		return false;
	}

	m_Type = Type;
	m_IsLoaded = true;
	m_ShaderId = ShaderId;
	return true;
}

void CGLSL::DeleteShader()
{
	if(!IsLoaded())
		return;
	m_IsLoaded = false;
	glDeleteShader(m_ShaderId);
}

bool CGLSL::IsLoaded() const
{
	return m_IsLoaded;
}

TWGLuint CGLSL::GetShaderId() const
{
	return m_ShaderId;
}

CGLSL::CGLSL()
{
	m_IsLoaded = false;
}

CGLSL::~CGLSL()
{
	DeleteShader();
}

#endif
