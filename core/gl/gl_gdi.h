#pragma once

/**
 * @file gl_gdi.h
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
#include "gl_basic.h"
#include "gl_texture.h"
#include "gl_shader.h"
#include "../os/file_dir.h"
#include "../ext/ipp/ipp_image.h"

namespace gl
{
/** \addtogroup gl_gdi
 * @ingroup gl
 *  @{
 */
typedef os::U16CHAR		GDICHAR;

/**
 * @brief Reading Unicode Glyph Bitmap File Format (*.ufg)
 * 
 */
class gdiFont
{
	friend class gdiCanvas;

	rt::Buffer<BYTE>				_glyph_internal_buffer; ///< holding loaded glyph raw alpha map
	UINT							_glyph_internal_pixelstep;
	UINT							_glyph_internal_bytestep;
	GDICHAR							_glyph_current_code;
	UINT							_glyph_w;
	UINT							_glyph_x_offset;
	LPBYTE							_glyph_current_buf;

	rt::Buffer<BYTE>				_glyph_map;
	UINT							_glyph_map_file_offset;
	LPCBYTE							_GetGlyph(GDICHAR code,UINT block_id,INT& x, INT& width, INT&height);
	LPCBYTE							_GetGlyph(GDICHAR code,INT& x, INT& width, INT&height);
	UINT							_glyph_map_width;
	UINT							_glyph_map_height;

	int								_font_hspace;
	int								_font_vspace;

protected:
	enum
	{	_CodeBlock_Flag_FixWidth = 1,
	};
#pragma pack(1)
	struct _CodeBlock
	{	GDICHAR Min;
		GDICHAR Max;
	};
	struct _CodeBlock_Info
	{	DWORD Flag;
		UINT GlyphIndexTable_Width;		///< if(_CodeBlock_Flag_FixWidth) it is width, otherwise it is offset on GlyphIndexTable
		UINT GlyphBitmap_StartPixel;	//
		UINT Code_perline;				///< fixed-width only
		UINT Padding_perline;			///< fixed-width only
		int	 Margin; 
		int  Baseline;
	};
	struct _FontGlyphIndex
	{	
		WORD	GlyphWidth;
		UINT	OffsetToGlyphBitmap; ///< offset relative to GlyphBitmap_Offset
	};
#pragma pack()

	rt::Buffer<_CodeBlock>			m_CodeBlock_Region; ///< use for search
	rt::Buffer<_CodeBlock_Info>		m_CodeBlock_Info;	///< |- Same length
	rt::Buffer<_FontGlyphIndex>		m_GlyphIndexTable;
	os::File						m_GlyphFile;
	UINT							m_BPP;

	UINT							m_MinWidth;
	UINT							m_MaxWidth;
	UINT							m_AvgWidth;
	UINT							m_Height;
	UINT							m_MapPixelStep;
	UINT							m_MapByteStep;
	UINT							m_MapByteLineStep;

protected:
	gl::Texture1c8u					m_FontTexture;
	GLint							_GetBindedTexture();
	struct _GlyphTextureClip
	{	int x,y,width,margin;
	};
	bool							_GetGlyphTextureClip(GDICHAR c, _GlyphTextureClip* p);
	bool							_Open(rt::_File* pFile, DWORD flag);

public:
	enum _tagGlyphFont
	{	FLAG_KEEP_IN_MEMORY = 0x0001,
	};

	gdiFont();
	~gdiFont();
/** @name BasicOperation
*/
///@{
	void	SetFontSpacing(int h, int v = -1);
	bool	Open(LPCBYTE ufg_file, int len);	///< always FLAG_KEEP_IN_MEMORY
	bool	Open(LPCSTR filename, DWORD flag = FLAG_KEEP_IN_MEMORY);
	void	Close();
///@}
/** @name GlyphMap
*/
///@{
	UINT	GetGlyphMapWidth() const;
	UINT	GetGlyphMapHeight() const;
	LPCVOID	GetGlyphMap() const;
	bool	HasGlyphMap() const { return _glyph_map.GetSize()!=0; }
///@}

	UINT	GetMaxCode() const;
	UINT	GetMinCode() const;
	UINT	GetStep() const;		///< in byte
	INT 	GetPixelStep() const;	///< in pixel
	bool	IsGlyphExisted(GDICHAR code) const;
	bool	IsNonleading(GDICHAR code) const;
	bool	IsNondiscrete(GDICHAR code, BYTE& blockid) const;
	INT 	GetGlyphWidth(GDICHAR code, BYTE& region_id) const;  ///< return width and region_id
	INT		GetGlyphWidth(GDICHAR code) const; ///< zero indicate non-existed
	INT		GetGlyphHeight(GDICHAR code) const;///< zero indicate non-existed

	INT		MeasureStringWidth(os::LPCU16CHAR text, UINT length) const;	 // utf16
	void	CopyGlyphs(os::LPCU16CHAR text, UINT length, ipp::Image_1c8u& img, int padding = -1, bool vertical = false); ///< len should <= 1024

	INLFUNC INT		MeasureStringWidth(const os::__UTF16& utf16) const { return MeasureStringWidth(utf16, (UINT)utf16.GetLength()); }
	INLFUNC void	CopyGlyphs(os::__UTF16& utf16, ipp::Image_1c8u& img, int padding = -1, bool vertical = false){ CopyGlyphs(utf16, (UINT)utf16.GetLength(), img, padding, vertical); }

	INLFUNC bool	IsOpen() const{ return m_CodeBlock_Region.GetSize()!=0; }
	INLFUNC UINT	GetBPP()const{ return m_BPP; }
	INLFUNC UINT	GetGlyphMaxWidth() const{ return m_MaxWidth; }
	INLFUNC UINT	GetGlyphAvgWidth() const{ return m_AvgWidth; }
	INLFUNC UINT	GetGlyphMinWidth() const{ return m_MinWidth; }
	INLFUNC UINT	GetGlyphMaxHeight() const{ return m_Height; }
	INLFUNC bool	IsGlyphVolatile() const{ return _glyph_map.GetSize()!=0; }
	INLFUNC UINT	GetRegionCount() const{ return (UINT)m_CodeBlock_Region.GetSize(); }
	INLFUNC LPCBYTE	GetGlyph(GDICHAR code,INT& x, INT& w, INT& h) const ///< the buffer is overwritten if IsGlyphVolatile()==true
	{	return rt::_CastToNonconst(this)->_GetGlyph(code,x,w,h); }
	INLFUNC LPCBYTE	GetGlyph(GDICHAR code, UINT region_id,INT& x, INT& w, INT& h) const ///< the buffer is overwritten if IsGlyphVolatile()==true
	{	return rt::_CastToNonconst(this)->_GetGlyph(code,region_id,x,w,h); }

};

enum _tagDrawingParam
{	
	PNAME_FORCE_NEAREST_SAMPLING = 0,	///< affect DrawImage only
	PNAME_TEXT_ALIGNMENT,
	PNAME_TEXT_VALIGNMENT,
	PNAME_MAX,

	TEXT_ALIGNMENT_LEFT = 0x1000,
	TEXT_ALIGNMENT_TOP  = 0x1000,
	TEXT_ALIGNMENT_CENTER = 0x1001,
	TEXT_ALIGNMENT_RIGHT = 0x1002,
	TEXT_ALIGNMENT_BOTTOM = 0x1002,
};

class gdiCanvas
{
	friend class gdiCanvasLayout;
	//enum{ DRAWTEXT_STRING_LENGTH_MAX = 512 };
	enum _tagDrawingMode
	{	DM_NONE = 0,
		DM_DRAWIMAGE,
		DM_DRAWPRIMITIVE,
		DM_DRAWPRIMITIVE_UNICOLOR,
		DM_DRAWTEXT,
	};
protected:
	int		dp_text_alignment;
	int		dp_text_valignment;
	bool	dp_force_nearest_sampling;

protected:
	int			_ViewWidth;
	int			_ViewHeight;
	int			_ViewLeft;
	int			_ViewTop;
	rt::Mat4x4f _Projection;

public:
	const rt::Mat4x4f& GetProjection() const { return _Projection; }

public:
	void	SetDrawingParam(DWORD param_name, int value);
	int		GetDrawingParam(DWORD param_name);
	int		GetCanvasWidth() const { return _ViewWidth; }
	int		GetCanvasHeight() const { return _ViewHeight; }
	void	SetCanvasPlacement(int left, int top, int right, int bottom);

protected:
	_tagDrawingMode	_DrawingMode;
	TextureBase		_texDrawImage;

	ShaderProgram	_shDrawImage;
	ShaderProgram	_shDrawPrimitive;
	ShaderProgram	_shDrawPrimitiveUniformColor;
	ShaderProgram	_shDrawText;
	void			_SetDrawingMode(_tagDrawingMode dm);

	gdiFont*		_pFont;
	int				_FontHeight;

protected:
	template<int PRIMITIVE>
	INLFUNC void _DrawLines(const rt::Vec2s* p, int points, const rt::Vec4b& color, float width)
	{	_SetDrawingMode(DM_DRAWPRIMITIVE_UNICOLOR);
		SetLineWidth(width);
		if(color.a != 255)glEnable(GL_BLEND);
		_shDrawPrimitiveUniformColor.SetUniform("_UniformColor", rt::Vec4f(color.r/255.0f,color.g/255.0f,color.b/255.0f,color.a/255.0f));
		_shDrawPrimitiveUniformColor.SetVertexAttributePointer("_Position", p, 2, GL_SHORT);
		glDrawArrays(PRIMITIVE, 0, 4);
		if(color.a != 255)glDisable(GL_BLEND);
	}

public:
	gdiCanvas();
	~gdiCanvas(){ Destroy(); }

	bool Create();
	void Destroy();

	void Use();
	void Reuse();

	template<typename t_Value,int Channel>
	INLFUNC void DrawImage(const ipp::Image_Ref<t_Value, Channel>& img, short x, short y){ DrawImage(img, x,y,(short)img.GetWidth(),(short)img.GetHeight()); }
	template<typename t_Value,int Channel>
	INLFUNC void DrawImage(const ipp::Image_Ref<t_Value, Channel>& img, short x, short y, short w, short h)
	{	if(Channel == 4)glEnable(GL_BLEND);
		_SetDrawingMode(DM_DRAWIMAGE);
		typedef _details::_GLTextureFormat<t_Value, Channel>	GLTF;
		glPixelStorei(GL_UNPACK_ALIGNMENT,4);
		#if defined(PLATFORM_OPENGL_SUPPORT)
		glPixelStorei(GL_UNPACK_ROW_LENGTH,img.GetStep()/(sizeof(t_Value)*Channel));
		#endif
		glTexImage2D(GL_TEXTURE_2D,0,GLTF::Internal,img.GetWidth(),img.GetHeight(),0,GLTF::TexFormat,GLTF::TexType,img.GetImageData());
		_LogGLError;
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
		static const int samm[2] = { GL_LINEAR, GL_NEAREST };
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,samm[1&dp_force_nearest_sampling]);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,samm[1&dp_force_nearest_sampling]);
		short pos[] = { x,y, x,(short)(y+h), (short)(x+w),(short)(y+h), (short)(x+w),y };
		float tex[] = { 0,0, 0,1, 1,1, 1,0 };
		_shDrawImage.SetVertexAttributePointer("_Position", pos, 2, GL_SHORT);
		_shDrawImage.SetVertexAttributePointer("_TexCoord", tex, 2, GL_FLOAT);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		if(Channel == 4)glDisable(GL_BLEND);
	}

	INLFUNC void FillRect(short x, short y, short w, short h, const rt::Vec4b& color){ FillRect(x,y,w,h,color,color); }
	void FillRect(short x, short y, short w, short h, const rt::Vec4b& color1,const rt::Vec4b& color2, bool vertical = false);
	void DrawRect(short x, short y, short w, short h, const rt::Vec4b& color, float width = 1);
	
	INLFUNC void DrawLine(short x1, short y1, short x2, short y2, const rt::Vec4b& color, float width = 1){ rt::Vec2s p[2] = { rt::Vec2s(x1,y1), rt::Vec2s(x2,y2) };  _DrawLines<GL_LINES>(p,2,color,width); }
	INLFUNC void DrawLines(const rt::Vec2s* p, short points, const rt::Vec4b& color, float width = 1){ _DrawLines<GL_LINES>(p,points,color,width); }
	INLFUNC void DrawLineStrip(const rt::Vec2s* p, short points, const rt::Vec4b& color, float width = 1){ _DrawLines<GL_LINE_STRIP>(p,points,color,width); }
	INLFUNC void DrawLineLoop(const rt::Vec2s* p, short points, const rt::Vec4b& color, float width = 1){ _DrawLines<GL_LINE_LOOP>(p,points,color,width); }

	void SetFont(gdiFont& font, UINT height = 0);
	void DrawText(const rt::String_Ref& text, short x, short y, const rt::Vec4b& color);
};


class gdiCanvasLayout: public os::UserInputSubscriber
{
	const gdiCanvas& _Canvas;
	int			_LastMouseX;
	int			_LastMouseY;
protected:
	rt::Vec2d	_Scale;
	rt::Vec2d	_Offset;
	rt::Vec2d	_OffsetSpeed;
	rt::Vec2d	_ZoomSpeedExp;
	rt::Vec2i	_ContentSize;
	

public:
	virtual void OnUserInputEvent(const os::UserInputEvent& x);

	gdiCanvasLayout(const gdiCanvas& canvas);
	void SetContentSize(int w, int h);
	void SetTranslateSpeed(double sx, double sy);
	void SetZoomSpeed(double szoom_x, double szoom_y);
	void ApplyTo(ShaderProgramBase& shader, LPCSTR layout_name = "_gdiLayout", bool as_double_type = false);  // vec4 = { offset_x, offset_y, scale_x, scale_y }
};
/** @}*/
} // namespace gl
/** @}*/