#pragma once

/**
 * @file gl_pal.h
 * @author JP Wang (wangjiaping@idea.edu.cn)
 * @brief 
 * @version 1.0
 * @date 2021-04-30
 * 
 * @copyright  
 * Cross-Platform Core Foundation (CPCF)
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *      * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *      * Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials provided
 *        with the distribution.
 *      * Neither the name of CPCF.  nor the names of its
 *        contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *  
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.   
 */
/** \addtogroup gl 
 * @ingroup CPCF
 *  @{
 */
/** \addtogroup gl_pal
 * @ingroup gl
 *  @{
 */
#include "../rt/runtime_base.h"

#if defined(PLATFORM_OPENGL_SUPPORT)
	#if defined(PLATFORM_WIN)
		#include <gl/GL.h>
		#include <gl/GLu.h>
		#include "glext_inc.h"	///< http://www.opengl.org/registry/api/glext.h
		#include "wglext_inc.h"
	#elif defined(PLATFORM_MAC)
        //#define GL_GLEXT_LEGACY
		#include <OpenGL/gl.h>
		#include <OpenGL/glext.h>
        //#include "glext_inc.h"
		#include <OpenGL/glu.h>

#define GL_LINES_ADJACENCY            0x000A
#define GL_LINE_STRIP_ADJACENCY       0x000B
#define GL_TRIANGLES_ADJACENCY        0x000C
#define GL_TRIANGLE_STRIP_ADJACENCY   0x000D
#define GL_PROGRAM_POINT_SIZE_ARB         0x8642
#define GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS_ARB 0x8C29
#define GL_FRAMEBUFFER_ATTACHMENT_LAYERED_ARB 0x8DA7
#define GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS_ARB 0x8DA8
#define GL_FRAMEBUFFER_INCOMPLETE_LAYER_COUNT_ARB 0x8DA9
#define GL_GEOMETRY_SHADER_ARB            0x8DD9
#define GL_GEOMETRY_VERTICES_OUT_ARB      0x8DDA
#define GL_GEOMETRY_INPUT_TYPE_ARB        0x8DDB
#define GL_GEOMETRY_OUTPUT_TYPE_ARB       0x8DDC
#define GL_MAX_GEOMETRY_VARYING_COMPONENTS_ARB 0x8DDD
#define GL_MAX_VERTEX_VARYING_COMPONENTS_ARB 0x8DDE
#define GL_MAX_GEOMETRY_UNIFORM_COMPONENTS_ARB 0x8DDF
#define GL_MAX_GEOMETRY_OUTPUT_VERTICES_ARB 0x8DE0
#define GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS_ARB 0x8DE1

		extern "C" LPVOID	_objc_opengl_rendercontext_get(LPVOID pNSOpenGLView); ///< return NSOpenGLContext*
		extern "C" bool		_objc_opengl_rendercontext_makecurrent(LPVOID pNSOpenGLContext);
		extern "C" LPVOID	_objc_opengl_rendercontext_current(); ///< return NSOpenGLContext*
	#endif
#elif defined(PLATFORM_OPENGL_ES_SUPPORT)
	#include <OpenGLES/ES2/gl.h>
	#include <OpenGLES/ES2/glext.h>
	#if defined(PLATFORM_IOS)
	extern "C" bool		_objc_opengl_rendercontext_makecurrent(LPVOID pEAGLContext);
	extern "C" LPVOID	_objc_opengl_rendercontext_current(); ///< return EAGLContext*
	extern "C" LPVOID	_objc_opengl_rendercontext_create(LPVOID pUIView, GLuint* pRenderingBuffers);  ///< EAGLContext*
	extern "C" void		_objc_opengl_rendercontext_destroy(LPVOID pEAGLContext);  ///< EAGLContext*
	#endif
#else
	ASSERT_STATIC(0);
#endif

#if	defined(PLATFORM_WIN)
	#pragma comment(lib,"opengl32.lib")
	#pragma comment(lib,"glu32.lib")
#endif


// Map OpenGL to OpenGL/ES2

#if defined(PLATFORM_OPENGL_SUPPORT)

typedef GLhandleARB GLhandle;

#if defined(PLATFORM_WIN)
#define PLATFROM_DEPENDENT_FUNC \
EACHITEM(PFNWGLSWAPINTERVALEXTPROC				, wglSwapIntervalEXT)

#else
#define PLATFROM_DEPENDENT_FUNC

#endif

#if defined(PLATFORM_WIN) //|| defined(PLATFORM_MAC)

#define GLEXTENTIONS_FUNCTIONS	\
EACHITEM(PFNGLUNIFORM1FARBPROC					, glUniform1fARB) \
EACHITEM(PFNGLUNIFORM2FARBPROC					, glUniform2fARB) \
EACHITEM(PFNGLUNIFORM3FARBPROC					, glUniform3fARB) \
EACHITEM(PFNGLUNIFORM4FARBPROC					, glUniform4fARB) \
EACHITEM(PFNGLUNIFORM1IARBPROC					, glUniform1iARB) \
EACHITEM(PFNGLUNIFORM2IARBPROC					, glUniform2iARB) \
EACHITEM(PFNGLUNIFORM3IARBPROC					, glUniform3iARB) \
EACHITEM(PFNGLUNIFORM4IARBPROC					, glUniform4iARB) \
EACHITEM(PFNGLUNIFORM1FVARBPROC					, glUniform1fvARB) \
EACHITEM(PFNGLUNIFORM2FVARBPROC					, glUniform2fvARB) \
EACHITEM(PFNGLUNIFORM3FVARBPROC					, glUniform3fvARB) \
EACHITEM(PFNGLUNIFORM4FVARBPROC					, glUniform4fvARB) \
EACHITEM(PFNGLUNIFORM1IVARBPROC					, glUniform1ivARB) \
EACHITEM(PFNGLUNIFORM2IVARBPROC					, glUniform2ivARB) \
EACHITEM(PFNGLUNIFORM3IVARBPROC					, glUniform3ivARB) \
EACHITEM(PFNGLUNIFORM4IVARBPROC					, glUniform4ivARB) \
EACHITEM(PFNGLUNIFORMMATRIX2FVARBPROC			, glUniformMatrix2fvARB) \
EACHITEM(PFNGLUNIFORMMATRIX3FVARBPROC			, glUniformMatrix3fvARB) \
EACHITEM(PFNGLUNIFORMMATRIX4FVARBPROC			, glUniformMatrix4fvARB) \
EACHITEM(PFNGLGETUNIFORMFVARBPROC				, glGetUniformfvARB) \
EACHITEM(PFNGLGETUNIFORMIVARBPROC				, glGetUniformivARB) \
EACHITEM(PFNGLDETACHOBJECTARBPROC				, glDetachObjectARB) \
EACHITEM(PFNGLDELETEOBJECTARBPROC				, glDeleteObjectARB) \
EACHITEM(PFNGLCREATESHADEROBJECTARBPROC			, glCreateShaderObjectARB) \
EACHITEM(PFNGLSHADERSOURCEARBPROC				, glShaderSourceARB) \
EACHITEM(PFNGLCOMPILESHADERARBPROC				, glCompileShaderARB) \
EACHITEM(PFNGLCREATEPROGRAMOBJECTARBPROC		, glCreateProgramObjectARB) \
EACHITEM(PFNGLATTACHOBJECTARBPROC				, glAttachObjectARB) \
EACHITEM(PFNGLLINKPROGRAMARBPROC				, glLinkProgramARB) \
EACHITEM(PFNGLUSEPROGRAMOBJECTARBPROC			, glUseProgramObjectARB) \
EACHITEM(PFNGLVALIDATEPROGRAMARBPROC			, glValidateProgramARB) \
EACHITEM(PFNGLGETOBJECTPARAMETERIVARBPROC		, glGetObjectParameterivARB) \
EACHITEM(PFNGLGETINFOLOGARBPROC					, glGetInfoLogARB) \
EACHITEM(PFNGLGETATTACHEDOBJECTSARBPROC			, glGetAttachedObjectsARB) \
EACHITEM(PFNGLGETUNIFORMLOCATIONARBPROC			, glGetUniformLocationARB) \
EACHITEM(PFNGLGETATTRIBLOCATIONARBPROC			, glGetAttribLocationARB) \
EACHITEM(PFNGLGETACTIVEUNIFORMARBPROC			, glGetActiveUniformARB) \
EACHITEM(PFNGLGETSHADERSOURCEARBPROC			, glGetShaderSourceARB) \
EACHITEM(PFNGLBINDATTRIBLOCATIONARBPROC			, glBindAttribLocationARB) \
EACHITEM(PFNGLGETACTIVEATTRIBARBPROC			, glGetActiveAttribARB) \
EACHITEM(PFNGLENABLEVERTEXATTRIBARRAYARBPROC	, glEnableVertexAttribArrayARB) \
EACHITEM(PFNGLVERTEXATTRIBPOINTERARBPROC		, glVertexAttribPointerARB) \
EACHITEM(PFNGLVERTEXATTRIBIPOINTEREXTPROC		, glVertexAttribIPointerEXT) \
EACHITEM(PFNGLDISABLEVERTEXATTRIBARRAYARBPROC	, glDisableVertexAttribArrayARB) \
EACHITEM(PFNGLACTIVETEXTUREARBPROC				, glActiveTextureARB) \
EACHITEM(PFNGLBINDBUFFERARBPROC					, glBindBufferARB) \
EACHITEM(PFNGLDELETEBUFFERSARBPROC				, glDeleteBuffersARB) \
EACHITEM(PFNGLGENBUFFERSARBPROC					, glGenBuffersARB) \
EACHITEM(PFNGLISBUFFERARBPROC					, glIsBufferARB) \
EACHITEM(PFNGLBUFFERDATAARBPROC					, glBufferDataARB) \
EACHITEM(PFNGLBUFFERSUBDATAARBPROC				, glBufferSubDataARB) \
EACHITEM(PFNGLGETBUFFERSUBDATAARBPROC			, glGetBufferSubDataARB) \
EACHITEM(PFNGLMAPBUFFERARBPROC					, glMapBufferARB) \
EACHITEM(PFNGLUNMAPBUFFERARBPROC				, glUnmapBufferARB) \
EACHITEM(PFNGLGETBUFFERPARAMETERIVARBPROC		, glGetBufferParameterivARB) \
EACHITEM(PFNGLGETBUFFERPOINTERVARBPROC			, glGetBufferPointervARB) \
EACHITEM(PFNGLPROGRAMPARAMETERIEXTPROC			, glProgramParameteriEXT) \
EACHITEM(PFNGLUNIFORM1DPROC						, glUniform1d) \
EACHITEM(PFNGLUNIFORM2DPROC						, glUniform2d) \
EACHITEM(PFNGLUNIFORM3DPROC						, glUniform3d) \
EACHITEM(PFNGLUNIFORM4DPROC						, glUniform4d) \
EACHITEM(PFNGLUNIFORM1DVPROC					, glUniform1dv) \
EACHITEM(PFNGLUNIFORM2DVPROC					, glUniform2dv) \
EACHITEM(PFNGLUNIFORM3DVPROC					, glUniform3dv) \
EACHITEM(PFNGLUNIFORM4DVPROC					, glUniform4dv) \
EACHITEM(PFNGLUNIFORMMATRIX2DVPROC				, glUniformMatrix2dv) \
EACHITEM(PFNGLUNIFORMMATRIX3DVPROC				, glUniformMatrix3dv) \
EACHITEM(PFNGLUNIFORMMATRIX4DVPROC				, glUniformMatrix4dv) \
PLATFROM_DEPENDENT_FUNC


#define EACHITEM(FUNCPROTO, Name)	extern FUNCPROTO Name;
GLEXTENTIONS_FUNCTIONS
#undef EACHITEM

#endif // #if defined(PLATFORM_WIN)

#if defined(PLATFORM_MAC)
#define GL_MAX_VERTEX_UNIFORM_VECTORS	GL_MAX_VERTEX_UNIFORM_COMPONENTS_ARB
#define GL_MAX_VARYING_VECTORS			GL_MAX_VARYING_FLOATS_ARB
#define GL_MAX_FRAGMENT_UNIFORM_VECTORS	GL_MAX_FRAGMENT_UNIFORM_COMPONENTS_ARB
#define GL_RGB16F						GL_RGB16F_ARB
#define GL_RGBA16F						GL_RGBA16F_ARB
#define GL_RGB32F						GL_RGB32F_ARB
#define GL_RGBA32F						GL_RGBA32F_ARB
#endif // #if defined(PLATFORM_MAC)


// Map OpenGL/ES2 calls to OpenGL
#define glUniform1f				glUniform1fARB
#define glUniform2f				glUniform2fARB
#define glUniform3f				glUniform3fARB
#define glUniform4f				glUniform4fARB
#define glUniform1i				glUniform1iARB
#define glUniform2i				glUniform2iARB
#define glUniform3i				glUniform3iARB
#define glUniform4i				glUniform4iARB
#define glUniform1fv			glUniform1fvARB
#define glUniform2fv			glUniform2fvARB
#define glUniform3fv			glUniform3fvARB
#define glUniform4fv			glUniform4fvARB
#define glUniform1iv			glUniform1ivARB
#define glUniform2iv			glUniform2ivARB
#define glUniform3iv			glUniform3ivARB
#define glUniform4iv			glUniform4ivARB
#define glUniformMatrix2fv		glUniformMatrix2fvARB
#define glUniformMatrix3fv		glUniformMatrix3fvARB
#define glUniformMatrix4fv		glUniformMatrix4fvARB
#define glGetUniformfv			glGetUniformfvARB
#define glGetUniformiv			glGetUniformivARB
#define glDeleteProgram			glDeleteObjectARB
#define glDeleteShader			glDeleteObjectARB
#define glDetachShader			glDetachObjectARB
#define glCreateShader			glCreateShaderObjectARB
#define glShaderSource			glShaderSourceARB
#define glCompileShader			glCompileShaderARB
#define glCreateProgram			glCreateProgramObjectARB
#define glAttachShader			glAttachObjectARB
#define glLinkProgram			glLinkProgramARB
#define glUseProgram			glUseProgramObjectARB
#define glValidateProgram		glValidateProgramARB
#define glGetShaderiv			glGetObjectParameterivARB
#define glGetProgramiv			glGetObjectParameterivARB
#define glGetProgramInfoLog		glGetInfoLogARB
#define glGetShaderInfoLog		glGetInfoLogARB
#define glGetAttachedShaders	glGetAttachedShadersARB
#define glGetUniformLocation	glGetUniformLocationARB
#define glGetAttribLocation		glGetAttribLocationARB
#define glGetActiveUniform		glGetActiveUniformARB
#define glGetShaderSource		glGetShaderSourceARB
#define glBindAttribLocation	glBindAttribLocationARB
#define glGetActiveAttrib		glGetActiveAttribARB
#define glProgramParameteri		glProgramParameteriEXT

#define glEnableVertexAttribArray	glEnableVertexAttribArrayARB
#define glVertexAttribPointer		glVertexAttribPointerARB
#define glDisableVertexAttribArray	glDisableVertexAttribArrayARB

#define glActiveTexture			glActiveTextureARB

#define glBindBuffer			glBindBufferARB
#define glBindBuffer			glBindBufferARB
#define glDeleteBuffers			glDeleteBuffersARB
#define glGenBuffers			glGenBuffersARB
#define glIsBuffer				glIsBufferARB
#define glBufferData			glBufferDataARB
#define glBufferSubData			glBufferSubDataARB
#define glGetBufferSubData		glGetBufferSubDataARB
#define glMapBuffer				glMapBufferARB
#define glUnmapBuffer			glUnmapBufferARB
#define glGetBufferParameteriv	glGetBufferParameterivARB
#define glGetBufferPointerv  	glGetBufferPointervARB

#elif defined(PLATFORM_OPENGL_ES_SUPPORT)

typedef GLint GLhandle;

#define GL_RGB8		GL_RGB
#define GL_RGBA8	GL_RGBA
#define GL_R8		GL_RED_EXT
#define GL_RG8		GL_RG_EXT

#define GL_RED		GL_RED_EXT
#define GL_RG		GL_RG_EXT
#define GL_RGB16F	GL_RGB16F_EXT
#define GL_RGBA16F	GL_RGBA16F_EXT
#define GL_RGB32F	GL_RGB32F_EXT
#define GL_RGBA32F	GL_RGBA32F_EXT
#define GL_R16F		GL_R16F_EXT
#define GL_RG16F	GL_RG16F_EXT
#define GL_R32F		GL_R32F_EXT
#define GL_RG32F	GL_RG32F_EXT
#define GL_DEPTH_COMPONENT32	GL_DEPTH_COMPONENT32_OES

#define glMapBuffer		glMapBufferOES
#define glUnmapBuffer	glUnmapBufferOES

#endif

/*
#ifndef	GL_SAMPLER_1D_ARB
#define GL_SAMPLER_1D_ARB				0x8B5D
#endif

#ifndef	GL_SAMPLER_2D_ARB
#define GL_SAMPLER_2D_ARB				0x8B5E
#endif

#ifndef	GL_SAMPLER_3D_ARB
#define GL_SAMPLER_3D_ARB				0x8B5F
#endif

#ifndef	GL_SAMPLER_CUBE_ARB
#define GL_SAMPLER_CUBE_ARB				0x8B60
#endif

#ifndef	GL_SAMPLER_1D_SHADOW_ARB
#define GL_SAMPLER_1D_SHADOW_ARB		0x8B61
#endif

#ifndef	GL_SAMPLER_2D_SHADOW_ARB
#define GL_SAMPLER_2D_SHADOW_ARB		0x8B62
#endif

#ifndef	GL_SAMPLER_2D_RECT_ARB
#define GL_SAMPLER_2D_RECT_ARB			0x8B63
#endif

#ifndef	GL_SAMPLER_2D_RECT_SHADOW_ARB
#define GL_SAMPLER_2D_RECT_SHADOW_ARB	0x8B64
#endif
*/
/** @}*/
/** @}*/
