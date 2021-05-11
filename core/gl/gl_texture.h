#pragma once

/**
 * @file gl_texture.h
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
#include "gl_basic.h"
#include "../rt/small_math.h"
#include "../rt/buffer_type.h"

/** \addtogroup gl 
 * @ingroup CPCF
 *  @{
 */
namespace gl
{
/** \addtogroup gl_texture
 * @ingroup gl
 *  @{
 */
namespace _details
{

template<UINT TexDefaultFormat,UINT TexDefaultType>
struct TexelType;
template<> struct TexelType<GL_RGB,		GL_UNSIGNED_BYTE>{ typedef BYTE t_Val; static const int t_Chan = 3; };
template<> struct TexelType<GL_RGBA,	GL_UNSIGNED_BYTE>{ typedef BYTE t_Val; static const int t_Chan = 4; };
template<> struct TexelType<GL_RGB,		GL_FLOAT>{ typedef float t_Val; static const int t_Chan = 3; };
template<> struct TexelType<GL_RGBA,	GL_FLOAT>{ typedef float t_Val; static const int t_Chan = 4; };
template<> struct TexelType<GL_RED,		GL_UNSIGNED_BYTE>{ typedef BYTE t_Val; static const int t_Chan = 1; };
template<> struct TexelType<GL_RG,		GL_UNSIGNED_BYTE>{ typedef BYTE t_Val; static const int t_Chan = 2; };
template<> struct TexelType<GL_RED,		GL_FLOAT>{ typedef float t_Val; static const int t_Chan = 1; };
template<> struct TexelType<GL_RG,		GL_FLOAT>{ typedef float t_Val; static const int t_Chan = 2; };
template<> struct TexelType<GL_DEPTH_COMPONENT,	GL_FLOAT>{ typedef float t_Val; static const int t_Chan = 1; };

template<typename T, int CHAN>
void _mipmap_scale_image(const rt::Vec<T,CHAN>* src, rt::Vec<T,CHAN>* dst, int w, int h, int src_step, int dst_step)
{
	h/=2;	w/=2;
	LPCBYTE src_scan = (LPCBYTE)src;
	LPBYTE  dst_scan = (LPBYTE)dst;

	for(int y=0;y<h;y++,src_scan += src_step*2,dst_scan += dst_step)
	{
		const rt::Vec<T,CHAN>* s0 = (const rt::Vec<T,CHAN>*)src_scan;
		const rt::Vec<T,CHAN>* s1 = (const rt::Vec<T,CHAN>*)(src_scan + src_step);
		rt::Vec<T,CHAN>* d = (rt::Vec<T,CHAN>*)dst_scan;

		for(int x=0;x<w;x++)
			d[x].AvgQuad(s0[x*2],s0[x*2+1],s1[x*2],s1[x*2+1]);
	}
}

};

class TextureBase
{
protected:
	GLuint	_TexName;
	void	_GenTextureName();

public:
	enum _TextureFilter {      
		FilterNearest			= GL_NEAREST,  ///< GL_NEAREST 
		FilterLinear			= GL_LINEAR,  ///< GL_LINEAR  
		FilterNearestMipmap		= GL_NEAREST_MIPMAP_NEAREST,  ///< GL_NEAREST_MIPMAP_NEAREST
		FilterLinearMipmap		= GL_NEAREST_MIPMAP_LINEAR,   ///< GL_NEAREST_MIPMAP_LINEAR
		FilterBilinearMipmap	= GL_LINEAR_MIPMAP_NEAREST, ///< GL_LINEAR_MIPMAP_NEAREST
		FilterTrilinearMipmap	= GL_LINEAR_MIPMAP_LINEAR ///< GL_LINEAR_MIPMAP_LINEAR
	};

	enum _TextureWrapMode {
		WrapRepeat = GL_REPEAT,
		WrapRepeatMirrored  = GL_MIRRORED_REPEAT,
		WrapClampToEdge	= GL_CLAMP_TO_EDGE
	};

	TextureBase(){ _TexName = 0; }
	~TextureBase(){ Destroy(); }
	void Destroy();
	bool IsTexture() const { return _TexName; }
	GLuint GetTextureName() const { return _TexName; }
};


template<UINT TexTarget,UINT TexInternalFormat,UINT TexFormat,UINT TexType>
class Texture2DBase:public TextureBase
{
	typedef _details::TexelType<TexFormat, TexType>	TexelTypeTrait;
	typedef typename _details::TexelType<TexFormat, TexType>::t_Val	t_Val;
	typedef rt::Vec<typename TexelTypeTrait::t_Val, TexelTypeTrait::t_Chan>	t_Texel;
public:
	static const int TEXTURE_TARGET = TexTarget;
protected:
	UINT			_WrapMode_S;
	UINT			_WrapMode_T;	
	_TextureFilter	_TexFilterMinify;
	_TextureFilter	_TexFilterMagnify;
	int				_AnisotropyParam;
public:
	INLFUNC static void Enable(){ glEnable(TexTarget); }
	INLFUNC static void Disable(){ glDisable(TexTarget); }
	INLFUNC	void Use() const
	{	rt::_CastToNonconst(this)->_GenTextureName();
		glBindTexture(TexTarget, _TexName);
		glTexParameteri(TexTarget,GL_TEXTURE_WRAP_S,_WrapMode_S);
		glTexParameteri(TexTarget,GL_TEXTURE_WRAP_T,_WrapMode_T);
		glTexParameteri(TexTarget,GL_TEXTURE_MIN_FILTER,_TexFilterMinify);
		glTexParameteri(TexTarget,GL_TEXTURE_MAG_FILTER,_TexFilterMagnify);
		glTexParameteri(TexTarget,GL_TEXTURE_MAX_ANISOTROPY_EXT, _AnisotropyParam);
		_LogGLError;
	}

public:
	Texture2DBase(){ TextureWrapMode(); TextureFilterSet(); }
	void TextureFilterSet(_TextureFilter minify=FilterLinear,_TextureFilter magnify=FilterLinear,GLint AnisotropyParam = 1.0f)
	{	_TexFilterMinify = minify;  
		_TexFilterMagnify = magnify;
		_AnisotropyParam = AnisotropyParam;
	}
	void TextureWrapMode(_TextureWrapMode s_mode = WrapClampToEdge,_TextureWrapMode t_mode = WrapClampToEdge)
	{	_WrapMode_S = s_mode;
		_WrapMode_T = t_mode;
	}
	void DefineTexture(UINT cx,UINT cy,LPCVOID pData, UINT image_step_byte = 0)
	{	Use();
		glPixelStorei(GL_UNPACK_ALIGNMENT,4);
		#if defined(PLATFORM_OPENGL_SUPPORT)
		glPixelStorei(GL_UNPACK_ROW_LENGTH,image_step_byte/sizeof(t_Texel));
		#endif
		glTexImage2D(TexTarget,0,TexInternalFormat,cx,cy,0,TexFormat,TexType,pData);
		_LogGLError;
	}
	void DefineTextureMipmapped(UINT cx,UINT cy,LPCVOID pData, UINT image_step_byte = 0) ///< Must use default format
	{	
		ASSERT(pData);
		Use();
		glPixelStorei(GL_UNPACK_ALIGNMENT,4);
		if(image_step_byte)
		{	image_step_byte /= sizeof(t_Texel);	}
		else
		{	image_step_byte = cx;	}
		rt::Buffer<BYTE>	_buf;
		VERIFY(_buf.SetSize(cy*(cx+4)*sizeof(t_Texel)*5/16));
		t_Texel* _buf0 = (t_Texel*)_buf.Begin();
		t_Texel* _buf1 = (t_Texel*)(_buf.Begin() + cy*(cx+4)*sizeof(t_Texel)/4);
		int level = 0;
		for(;;)
		{
			#if defined(PLATFORM_OPENGL_SUPPORT)
			glPixelStorei(GL_UNPACK_ROW_LENGTH,image_step_byte);
			#endif
			glTexImage2D(TexTarget,level,TexInternalFormat,cx,cy,0, TexFormat, TexType, pData);
			if(cx == 1 && cy == 1)break;

			ASSERT((cx&1) == 0);
			ASSERT((cy&1) == 0);
			_details::_mipmap_scale_image<typename TexelTypeTrait::t_Val, TexelTypeTrait::t_Chan>
			((const t_Texel*)pData, _buf0, cx, cy, (cx*sizeof(t_Texel) + 3)&0xfffffffc, (cx/2*sizeof(t_Texel) + 3)&0xfffffffc);
			pData = (const typename TexelTypeTrait::t_Val*)_buf0;
			rt::Swap(_buf0, _buf1);
			cx/=2;	cy/=2;
			image_step_byte = cx;
			level++;
		}
		_LogGLError;
	}
	void TextureSubImage(int srcX,int srcY,int cx,int cy, const typename TexelTypeTrait::t_Val* pData)
	{	Use();
		glTexSubImage2D(TexTarget,0,srcX,srcY,cx,cy,TexFormat,TexType,pData);
		_LogGLError;
	}
	void CopyTextureImage(GLint width,GLint height,GLint src_x=0,GLint src_y=0)
	{	glCopyTexImage2D(TexTarget,0,TexInternalFormat,src_x,src_y,width,height,0);
		_LogGLError;
	}
	void CopyTextureSubImage(GLint width,GLint height,GLint src_x=0,GLint src_y=0,GLint dst_x=0,GLint dst_y=0)
	{	glCopyTexSubImage2D(TexTarget,0,dst_x,dst_y,src_x,src_y,width,height);
		_LogGLError;
	}
};

/** @name Conventional Textures
*/
///@{
typedef Texture2DBase<GL_TEXTURE_2D,GL_RGB8		,GL_RGB,GL_UNSIGNED_BYTE>				Texture3c8u;
typedef Texture2DBase<GL_TEXTURE_2D,GL_RGBA8	,GL_RGBA,GL_UNSIGNED_BYTE>				Texture4c8u;
typedef Texture2DBase<GL_TEXTURE_2D,GL_R8		,GL_RED,GL_UNSIGNED_BYTE>				Texture1c8u;
typedef Texture2DBase<GL_TEXTURE_2D,GL_RG8		,GL_RG,GL_UNSIGNED_BYTE>				Texture2c8u;
///@}

/** @name HDR Textures
*/
///@{
typedef Texture2DBase<GL_TEXTURE_2D,GL_RGB16F	,GL_RGB,GL_FLOAT>						Texture3c16f;
typedef Texture2DBase<GL_TEXTURE_2D,GL_RGBA16F	,GL_RGBA,GL_FLOAT>						Texture4c16f;
typedef Texture2DBase<GL_TEXTURE_2D,GL_RGB32F	,GL_RGB,GL_FLOAT>						Texture3c32f;
typedef Texture2DBase<GL_TEXTURE_2D,GL_RGBA32F	,GL_RGBA,GL_FLOAT>						Texture4c32f;
typedef Texture2DBase<GL_TEXTURE_2D,GL_R16F		,GL_RED,GL_FLOAT>						Texture1c16f;
typedef Texture2DBase<GL_TEXTURE_2D,GL_RG16F	,GL_RG,GL_FLOAT>						Texture2c16f;
typedef Texture2DBase<GL_TEXTURE_2D,GL_R32F		,GL_RED,GL_FLOAT>						Texture1c32f;
typedef Texture2DBase<GL_TEXTURE_2D,GL_RG32F	,GL_RG,GL_FLOAT>						Texture2c32f;
///@}

/** @name Shadow Textures
*/
///@{
typedef Texture2DBase<GL_TEXTURE_2D,GL_DEPTH_COMPONENT16,GL_DEPTH_COMPONENT,GL_FLOAT>	DepthTexture16f;
typedef Texture2DBase<GL_TEXTURE_2D,GL_DEPTH_COMPONENT32,GL_DEPTH_COMPONENT,GL_FLOAT>	DepthTexture32f;
///@}
class Texture2D:public TextureBase
{
public:
	enum _tagTexFormat
	{
		TEXFMT_None = 0,
		TEXFMT_1c8u,
		TEXFMT_2c8u,
		TEXFMT_3c8u,
		TEXFMT_4c8u,
		TEXFMT_1c16f,
		TEXFMT_2c16f,
		TEXFMT_3c16f,
		TEXFMT_4c16f,
		TEXFMT_1c32f,
		TEXFMT_2c32f,
		TEXFMT_3c32f,
		TEXFMT_4c32f
	};
protected:
	BYTE	_TextureObj[sizeof(Texture1c8u) - sizeof(TextureBase)];
	UINT	_TextureFormat;
public:
	Texture2D();
	INLFUNC void TextureFilterSet(_TextureFilter minify=FilterLinear,_TextureFilter magnify=FilterLinear,GLint AnisotropyParam = 1.0f)
	{	((Texture1c8u*)this)->TextureFilterSet(minify, magnify, AnisotropyParam);
	}
	INLFUNC void TextureWrapMode(_TextureWrapMode s_mode = WrapClampToEdge,_TextureWrapMode t_mode = WrapClampToEdge)
	{	((Texture1c8u*)this)->TextureWrapMode(s_mode, t_mode);
	}
	INLFUNC	void Use() const { ((const Texture1c8u*)this)->Use(); }
	void DefineTexture(UINT TextureFormat, UINT cx,UINT cy, LPCVOID pData, UINT image_step_byte = 0);
	void DefineTextureMipmapped(UINT TextureFormat, UINT cx,UINT cy, LPCVOID pData, UINT image_step_byte = 0);
};

namespace _details
{
template<typename t_Value, int channel>
struct _GLTextureFormat;
template<> struct _GLTextureFormat<BYTE, 1>{ static const int Internal = GL_R8; static const int TexFormat = GL_RED; static const int TexType = GL_UNSIGNED_BYTE; };
template<> struct _GLTextureFormat<BYTE, 2>{ static const int Internal = GL_RG8; static const int TexFormat = GL_RG; static const int TexType = GL_UNSIGNED_BYTE; };
template<> struct _GLTextureFormat<BYTE, 3>{ static const int Internal = GL_RGB8; static const int TexFormat = GL_RGB; static const int TexType = GL_UNSIGNED_BYTE; };
template<> struct _GLTextureFormat<BYTE, 4>{ static const int Internal = GL_RGBA8; static const int TexFormat = GL_RGBA; static const int TexType = GL_UNSIGNED_BYTE; };

template<> struct _GLTextureFormat<float, 1>{ static const int Internal = GL_R32F; static const int TexFormat = GL_RED; static const int TexType = GL_FLOAT; };
template<> struct _GLTextureFormat<float, 2>{ static const int Internal = GL_RG32F; static const int TexFormat = GL_RG; static const int TexType = GL_FLOAT; };
template<> struct _GLTextureFormat<float, 3>{ static const int Internal = GL_RGB32F; static const int TexFormat = GL_RGB; static const int TexType = GL_FLOAT; };
template<> struct _GLTextureFormat<float, 4>{ static const int Internal = GL_RGBA32F; static const int TexFormat = GL_RGBA; static const int TexType = GL_FLOAT; };

}
/** @}*/
} // namespace gl
/** @}*/

