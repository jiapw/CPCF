#include "gl_gdi.h"
#include <math.h>

using namespace rt;

namespace gl
{

namespace _details
{
	//////////////////////////////////////////////////
	// File Format for Unicode Glyph Bitmap (*.ufg)
	#pragma pack(1)
	struct _FontGroup
	{
		UINT		CodeMin;
		UINT		CodeMax;
		int			GlyphWidth_IndexOffset;		// <0 is Fixed width font and >0 is Offset to GlyphIndexTable
		UINT		GlyphBitmap_PixelStart;		// in pixel of width
		int			Margin;
		int			Baseline;
	};

	struct _UFG_Header
	{
		enum
		{	SIGN_MAGIC = 0x4d424655,
			Flag_Swizzled_GlyphLine = 0x80000000,
		};

		DWORD		Sign; // "UFBM"
		DWORD		Flag_BPP; // low 16bit is 8BPP / 4BPP / 2BPP / 1BPP, high 16bit is flag
		UINT		GlyphHeight;
		UINT		GlyphStep;
		UINT		FontGroupCount;
		UINT		FontGroups_Offset;
		UINT		GlyphIndexTable_Size;   //in byte
		UINT		GlyphIndexTable_Offset;
		UINT		GlyphBitmap_Size;		//in byte
		UINT		GlyphBitmap_Offset;
	};
	#pragma pack()

} // namespace _details


gdiFont::gdiFont()
{
	_glyph_current_buf = NULL;
}

gdiFont::~gdiFont()
{
	Close();
}

UINT gdiFont::GetMaxCode() const
{
	if(m_CodeBlock_Region.GetSize())
	{
		return m_CodeBlock_Region[m_CodeBlock_Region.GetSize()-1].Max;
	}
	return 0;
}

UINT gdiFont::GetMinCode() const
{
	if(m_CodeBlock_Region.GetSize())
	{
		return m_CodeBlock_Region[0].Min;
	}
	return 0;
}

void gdiFont::Close()
{
	m_GlyphFile.Close();
	_glyph_current_code = 0;

	m_FontTexture.Destroy();
	m_CodeBlock_Region.SetSize();
	m_CodeBlock_Info.SetSize();
	_glyph_map.SetSize();
	_glyph_internal_buffer.SetSize();
}

UINT gdiFont::GetGlyphMapWidth() const
{
	return _glyph_map_width;
}

UINT gdiFont::GetGlyphMapHeight() const
{
	return _glyph_map_height;
}

LPCVOID	gdiFont::GetGlyphMap() const
{
	return _glyph_map;
}

GLint gdiFont::_GetBindedTexture()
{
	ASSERT(HasGlyphMap());	// Must open with FLAG_KEEP_IN_MEMORY
	if(!m_FontTexture.IsTexture())
	{
		m_FontTexture.Use();
		m_FontTexture.DefineTexture(GetGlyphMapWidth(), GetGlyphMapHeight(), (LPCBYTE)GetGlyphMap());
	}

	return m_FontTexture.GetTextureName();
}

bool gdiFont::Open(LPCSTR fn, DWORD flag)
{
	Close();

	if(!m_GlyphFile.Open(fn, os::File::Normal_Read))return false;
	bool ret = _Open(&m_GlyphFile, flag);
	if(flag & FLAG_KEEP_IN_MEMORY)
		m_GlyphFile.Close();

	return ret;
}

void gdiFont::SetFontSpacing(int h, int v)
{
	_font_hspace = h;
	_font_vspace = v;
}

bool gdiFont::Open(LPCBYTE ufg_file, int len)
{
	Close();
	os::MemoryFileRef f((LPVOID)ufg_file, len);
	return _Open(&f, FLAG_KEEP_IN_MEMORY);
}

bool gdiFont::_Open(rt::_File* pFile, DWORD flag)
{
	rt::_File& gfile = *pFile;
	_font_hspace = 0;
	_font_vspace = 0;

	//if(!gfile.Open(fn, os::File::Normal_Read))return false;

	//check header
	_details::_UFG_Header hdr;
	{
		if(	gfile.Read(&hdr,sizeof(hdr))!=sizeof(hdr) ||
			hdr.Sign!=_details::_UFG_Header::SIGN_MAGIC    || 
			(0xffff&hdr.Flag_BPP)>8 || hdr.FontGroupCount==0
		  )return false;

		if(hdr.FontGroupCount>255)	// supported maximum number of font groups is 255.
			return false;
		
		_glyph_map_file_offset = hdr.GlyphBitmap_Offset;
		m_Height = hdr.GlyphHeight;
		m_BPP = hdr.Flag_BPP&0xffff;
		m_MapPixelStep = hdr.GlyphStep;
		m_MapByteStep  = hdr.GlyphStep*m_BPP/8;
		m_MapByteLineStep =  hdr.GlyphHeight*m_MapByteStep;
		if(hdr.Flag_BPP&_details::_UFG_Header::Flag_Swizzled_GlyphLine)return false; // no Swizzle support
		m_CodeBlock_Region.SetSize(hdr.FontGroupCount);
		m_CodeBlock_Info.SetSize(hdr.FontGroupCount);

		_glyph_map_width = hdr.GlyphStep*8/m_BPP;
		_glyph_map_height = hdr.GlyphBitmap_Size/hdr.GlyphStep;
	}

	if(m_BPP<4)return false; //supports 4/8 BPP only

	//load groups
	UINT GlyphIndexTable_Len = 0;
	m_MaxWidth = 0;
	m_MinWidth = 100000;
	int code_count = 0;
	ULONGLONG total_with = 0;
	{	
		for(UINT i=0;i<m_CodeBlock_Region.GetSize();i++)
		{
			_details::_FontGroup		fg;

			gfile.Seek((int)hdr.FontGroups_Offset + i*sizeof(fg));
			gfile.Read(&fg,sizeof(fg));

			m_CodeBlock_Region[i].Min = fg.CodeMin;
			m_CodeBlock_Region[i].Max = fg.CodeMax;

			if(fg.GlyphWidth_IndexOffset <0)
			{	//Fix-width
				m_CodeBlock_Info[i].Flag = _CodeBlock_Flag_FixWidth;
				m_CodeBlock_Info[i].GlyphIndexTable_Width = -fg.GlyphWidth_IndexOffset;
				m_CodeBlock_Info[i].Code_perline = m_MapPixelStep/m_CodeBlock_Info[i].GlyphIndexTable_Width;
				m_CodeBlock_Info[i].Padding_perline = m_MapPixelStep%m_CodeBlock_Info[i].GlyphIndexTable_Width;
				m_MaxWidth = rt::max(m_MaxWidth,m_CodeBlock_Info[i].GlyphIndexTable_Width);
				m_MinWidth = rt::min(m_MinWidth,m_CodeBlock_Info[i].GlyphIndexTable_Width);
				total_with += m_CodeBlock_Info[i].GlyphIndexTable_Width*(fg.CodeMax - fg.CodeMin + 1);
				code_count += fg.CodeMax - fg.CodeMin + 1;
			}
			else
			{
				m_CodeBlock_Info[i].Flag = 0;
				m_CodeBlock_Info[i].GlyphIndexTable_Width = fg.GlyphWidth_IndexOffset;
				m_CodeBlock_Info[i].Code_perline = 0;
				m_CodeBlock_Info[i].Padding_perline = 0;
				GlyphIndexTable_Len += fg.CodeMax - fg.CodeMin + 1;
			}
			
			m_CodeBlock_Info[i].GlyphBitmap_StartPixel = fg.GlyphBitmap_PixelStart;
			m_CodeBlock_Info[i].Margin = fg.Margin;
			m_CodeBlock_Info[i].Baseline = fg.Baseline;
		}
	}

	if(GlyphIndexTable_Len*sizeof(_FontGlyphIndex) != hdr.GlyphIndexTable_Size)
	{
		Close();
		return false;
	}

	//load Glyph index table
	m_GlyphIndexTable.SetSize(GlyphIndexTable_Len);
	if(GlyphIndexTable_Len)
	{	// search for max width
		gfile.Seek((int)hdr.GlyphIndexTable_Offset);
		if(	gfile.Read(m_GlyphIndexTable,sizeof(_FontGlyphIndex)*GlyphIndexTable_Len)
			!=sizeof(_FontGlyphIndex)*GlyphIndexTable_Len
		  )return false;

		for(UINT i=0;i<m_GlyphIndexTable.GetSize();i++)
		{	int cw = m_GlyphIndexTable[i].GlyphWidth;
			m_MaxWidth = rt::max((int)m_MaxWidth,cw);
			m_MinWidth = rt::min((int)m_MinWidth,cw);
			code_count++;
			total_with += cw;
		}
	}

	m_AvgWidth = (UINT)(total_with/code_count);

	UINT step = (m_MaxWidth*m_BPP+7)/8 + 1;
	m_MaxWidth = step*8/m_BPP;

	if(flag&FLAG_KEEP_IN_MEMORY)
	{	
		VERIFY(_glyph_map.SetSize(hdr.GlyphBitmap_Size));

		gfile.Seek(_glyph_map_file_offset);
		if(gfile.Read(_glyph_map,hdr.GlyphBitmap_Size) != hdr.GlyphBitmap_Size)
		{
			Close();
			return false;
		}
	}
	else
	{
		_glyph_internal_pixelstep = ((m_MaxWidth*m_BPP+127)&(~127))/m_BPP;
		_glyph_internal_bytestep = _glyph_internal_pixelstep*m_BPP/8;
		_glyph_internal_buffer.SetSize(_glyph_internal_bytestep*m_Height);
	}

	return true;
}

UINT gdiFont::GetStep() const		// in byte
{
	return GetPixelStep()*m_BPP/8;
}

INT gdiFont::GetPixelStep() const	// in pixel
{
	return (INT)(_glyph_map.GetSize()?m_MapPixelStep:_glyph_internal_pixelstep);
}

bool gdiFont::IsGlyphExisted(GDICHAR code) const
{
	for(UINT i=0;i<m_CodeBlock_Region.GetSize();i++)
		if(code>=m_CodeBlock_Region[i].Min && code<=m_CodeBlock_Region[i].Max)
		{
			return	m_CodeBlock_Info[i].Flag&_CodeBlock_Flag_FixWidth ||
					m_GlyphIndexTable[m_CodeBlock_Info[i].GlyphIndexTable_Width + (code - m_CodeBlock_Region[i].Min)].GlyphWidth >0;
		}

	return false;
}

INT gdiFont::GetGlyphWidth(GDICHAR code, BYTE& block_id) const
{
	for(UINT i=0;i<m_CodeBlock_Region.GetSize();i++)
		if(code>=m_CodeBlock_Region[i].Min && code<=m_CodeBlock_Region[i].Max)
		{	block_id = i;
			if(m_CodeBlock_Info[i].Flag&_CodeBlock_Flag_FixWidth)
				return (INT)m_CodeBlock_Info[i].GlyphIndexTable_Width;
			else
				return (INT)(m_GlyphIndexTable[m_CodeBlock_Info[i].GlyphIndexTable_Width + (code - m_CodeBlock_Region[i].Min)].GlyphWidth);
		}

	block_id = 0xff;
	return 0;
}

INT gdiFont::GetGlyphWidth(GDICHAR code) const
{
	for(UINT i=0;i<m_CodeBlock_Region.GetSize();i++)
		if(code>=m_CodeBlock_Region[i].Min && code<=m_CodeBlock_Region[i].Max)
		{
			if(m_CodeBlock_Info[i].Flag&_CodeBlock_Flag_FixWidth)
				return (INT)m_CodeBlock_Info[i].GlyphIndexTable_Width;
			else
				return (INT)(m_GlyphIndexTable[m_CodeBlock_Info[i].GlyphIndexTable_Width + (code - m_CodeBlock_Region[i].Min)].GlyphWidth);
		}

	return 0;
}

INT gdiFont::GetGlyphHeight(GDICHAR code) const
{
	return (INT)m_Height;
}

INT gdiFont::MeasureStringWidth(os::LPCU16CHAR text, UINT len) const
{
	UINT total = 0;
	while(*text)
		total += GetGlyphWidth(*text++);
	return (INT)total;
}

void gdiFont::CopyGlyphs(os::LPCU16CHAR text, UINT len, ipp::Image_1c8u& img, int padding, bool vertical)
{
	ASSERT(m_BPP == 8);
	ASSERT(len <= 1024);
	UINT total = 0;
	int* block_ids = (int*)alloca(sizeof(int)*len);
	int avail = 0;

	for(UINT c=0;c<len;c++)
	{
		block_ids[c] = -1;
		os::U16CHAR code = text[c];
		for(UINT i=0;i<m_CodeBlock_Region.GetSize();i++)
			if(code>=m_CodeBlock_Region[i].Min && code<=m_CodeBlock_Region[i].Max)
			{
				block_ids[c] = i;
				int w;
				if(m_CodeBlock_Info[i].Flag&_CodeBlock_Flag_FixWidth)
					w = m_CodeBlock_Info[i].GlyphIndexTable_Width;
				else
					w = (m_GlyphIndexTable[m_CodeBlock_Info[i].GlyphIndexTable_Width + (code - m_CodeBlock_Region[i].Min)].GlyphWidth);

				total += w;
				avail++;
			}
	}

	if(!vertical)
	{
		img.SetSize((total + padding*(avail-1) + 3)&0xfffffffc, m_Height);
		img.Zero();
		int x = 0;
		for(UINT i=0;i<len;i++)
			if(block_ids[i]>=0)
			{	int off,w,h;
				LPCBYTE p = _GetGlyph(text[i], (UINT)block_ids[i], off, w, h);
				img.GetSub(x,0,w,h).Add(ipp::ImageRef_1c8u(p+off,w,h,GetStep()));
				x = rt::max(x + padding + w, 0);
			}
	}
	else
	{	
		img.SetSize((m_MaxWidth + 3)&0xfffffffc, m_Height*avail + padding*(avail-1));
		img.Zero();
		int y = 0;
		for(UINT i=0;i<len;i++)
			if(block_ids[i]>=0)
			{	int off,w,h;
				LPCBYTE p = _GetGlyph(text[i], (UINT)block_ids[i], off, w, h);
				img.GetSub((m_MaxWidth - w)/2,y,w,h).Add(ipp::ImageRef_1c8u(p+off,w,h,GetStep()));
				y = rt::max((int)(y + padding + m_Height), 0);
			}
	}
}

LPCBYTE gdiFont::_GetGlyph(GDICHAR code,INT& x, INT& width, INT&height)
{
	for(UINT i=0;i<m_CodeBlock_Region.GetSize();i++)
		if(code>=m_CodeBlock_Region[i].Min && code<=m_CodeBlock_Region[i].Max)
		{
			return _GetGlyph(code,i,x,width,height);
		}

	return NULL;
}

bool gdiFont::_GetGlyphTextureClip(GDICHAR code, _GlyphTextureClip* p)
{
	ASSERT(IsOpen());
	ASSERT(_glyph_map.GetSize()); // Must Open with FLAG_KEEP_IN_MEMORY
	UINT i=0;
	for(;i<m_CodeBlock_Region.GetSize();i++)
		if(code>=m_CodeBlock_Region[i].Min && code<=m_CodeBlock_Region[i].Max)
		{
			goto CODEBLOCK_FOUND;
		}
	return false;

CODEBLOCK_FOUND:
	int px_offset;

	ASSERT(i<m_CodeBlock_Region.GetSize());
	ASSERT(code>=m_CodeBlock_Region[i].Min && code<=m_CodeBlock_Region[i].Max);

	if(m_CodeBlock_Info[i].Flag&_CodeBlock_Flag_FixWidth)
	{
		_glyph_w = m_CodeBlock_Info[i].GlyphIndexTable_Width;
		UINT code_offset = code-m_CodeBlock_Region[i].Min;
		px_offset= m_CodeBlock_Info[i].GlyphBitmap_StartPixel + _glyph_w*code_offset
				   + m_CodeBlock_Info[i].Padding_perline*(code_offset/m_CodeBlock_Info[i].Code_perline);
	}
	else
	{
		_FontGlyphIndex& gi = m_GlyphIndexTable[m_CodeBlock_Info[i].GlyphIndexTable_Width + (code - m_CodeBlock_Region[i].Min)];
		_glyph_w = gi.GlyphWidth;
		px_offset= gi.OffsetToGlyphBitmap;

		if(_glyph_w){}else return false;
	}

	p->margin = m_CodeBlock_Info[i].Margin;
	p->width = _glyph_w;
	p->x = px_offset%m_MapPixelStep;
	p->y = (px_offset/m_MapPixelStep)*m_Height;

	return true;
}

LPCBYTE gdiFont::_GetGlyph(GDICHAR code,UINT i,INT& x, INT& width, INT& height)
{
	ASSERT(IsOpen());

	UINT px_offset;
	height = m_Height;

	ASSERT(i<m_CodeBlock_Region.GetSize());

	if(code!=_glyph_current_code)
	{
		_glyph_current_code = 0;
		ASSERT(code>=m_CodeBlock_Region[i].Min && code<=m_CodeBlock_Region[i].Max);

		if(m_CodeBlock_Info[i].Flag&_CodeBlock_Flag_FixWidth)
		{
			_glyph_w = m_CodeBlock_Info[i].GlyphIndexTable_Width;
			UINT code_offset = code-m_CodeBlock_Region[i].Min;
			px_offset= m_CodeBlock_Info[i].GlyphBitmap_StartPixel + _glyph_w*code_offset
					   + m_CodeBlock_Info[i].Padding_perline*(code_offset/m_CodeBlock_Info[i].Code_perline);
		}
		else
		{
			_FontGlyphIndex& gi = m_GlyphIndexTable[m_CodeBlock_Info[i].GlyphIndexTable_Width + (code - m_CodeBlock_Region[i].Min)];
			_glyph_w = gi.GlyphWidth;
			px_offset= gi.OffsetToGlyphBitmap;

			if(_glyph_w){}else return NULL;
		}

		width = _glyph_w;

		UINT off_y = px_offset/m_MapPixelStep;
		UINT off_x = px_offset%m_MapPixelStep;

		_glyph_current_code = code;

		if(_glyph_map.GetSize())
		{	// refer to internal map
			x = _glyph_x_offset = off_x;
			return _glyph_current_buf = &_glyph_map[ off_y*m_MapByteLineStep ];
		}
		else
		{	// load map
			UINT off_byte = (off_x*m_BPP)/8;
			x = _glyph_x_offset = off_x-(off_byte*8/m_BPP);
			UINT width_byte = (_glyph_w*m_BPP+7)/8 + 1;

			UINT seek_pos = _glyph_map_file_offset + off_y*m_MapByteLineStep + off_byte;
			for(UINT i=0;i<m_Height;i++,seek_pos+=m_MapByteStep)
			{
				m_GlyphFile.Seek(seek_pos);
				m_GlyphFile.Read(&_glyph_internal_buffer[_glyph_internal_bytestep*i],width_byte);
			}

			return _glyph_current_buf = _glyph_internal_buffer;
		}
	}
	else
	{
		width = _glyph_w;
		x = _glyph_x_offset;
		return _glyph_current_buf;
	}

	return NULL;
}

bool gdiFont::IsNonleading(GDICHAR code) const
{
	return false;
}

bool gdiFont::IsNondiscrete(GDICHAR code, BYTE& blockid) const
{
	return false;
}


gdiCanvas::gdiCanvas()
{
	dp_force_nearest_sampling = false;
	dp_text_valignment = TEXT_ALIGNMENT_TOP;
	dp_text_alignment = TEXT_ALIGNMENT_LEFT;
	_DrawingMode = DM_NONE;
	_pFont = NULL;
}

#define PARAM_ENTRIES()																			\
	switch(param_name)																			\
	{																							\
	PARAM_ENTRY(bool,PNAME_FORCE_NEAREST_SAMPLING,	dp_force_nearest_sampling);					\
	PARAM_ENTRY(int ,PNAME_TEXT_ALIGNMENT,			dp_text_alignment);							\
	PARAM_ENTRY(int ,PNAME_TEXT_VALIGNMENT,			dp_text_valignment);						\
	default: ASSERT(0);																			\
	}																							\
		

void gdiCanvas::SetDrawingParam(DWORD param_name, int value)
{
#define PARAM_ENTRY(type,name,varible)	case name: varible=(type)value; break;
	PARAM_ENTRIES()
#undef PARAM_ENTRY
}

int gdiCanvas::GetDrawingParam(DWORD param_name)
{
#define PARAM_ENTRY(type,name,varible)	case name: return varible;
	PARAM_ENTRIES()
#undef PARAM_ENTRY
	return 0;
}

bool gdiCanvas::Create()
{
	_DrawingMode = DM_NONE;

	_shDrawPrimitive.BuildWithSourceCodes(
		gl::ShaderSourceCodeLibrary::defaultVertexShaderSourceCode, 
		gl::ShaderSourceCodeLibrary::defaultFragmentShaderSourceCode
	);

	_shDrawImage.BuildWithSourceCodes(
		"SHADER_PRECISION(mediump,float)\n"	
		"attribute vec4 _Position;"
		"attribute vec2 _TexCoord;"
		"varying vec2 texCoord;"
		"uniform mat4 _MVP;"
		"void main()"
		"{	gl_Position = _MVP*_Position;"
		"	texCoord = _TexCoord;"
		"}"
		,
		"SHADER_PRECISION(mediump,float)\n"
		"uniform sampler2D _Texture;"
		"varying vec2 texCoord;"
		"void main()"
		"{	gl_FragColor = texture2D(_Texture, texCoord);"
		"}"
	);

	_shDrawPrimitiveUniformColor.BuildWithSourceCodes(
		"SHADER_PRECISION(mediump,float)\n"
		"attribute vec4 _Position;"
		"uniform mat4 _MVP;"
		"void main()"
		"{	gl_Position = _MVP*_Position;"
		"}"
		,
		"SHADER_PRECISION(mediump,float)\n"
		"uniform lowp vec4 _UniformColor;"
		"void main()"
		"{	gl_FragColor = _UniformColor;"
		"}"
	);

	_shDrawText.BuildWithSourceCodes(
		"SHADER_PRECISION(mediump,float)\n"
		"varying highp vec2 texCoord;"
		"attribute vec4 _Position;"
		"attribute highp vec2 _TexCoord;"
		"uniform mat4 _MVP;"
		"void main()"
		"{	texCoord = _TexCoord;"
		"	gl_Position = _MVP*_Position;"
		"}"
		,
		"SHADER_PRECISION(mediump,float)\n"
		"uniform lowp vec4 _UniformColor;"
		"uniform sampler2D _Texture;"
		"varying highp vec2 texCoord;"
		"void main()"
		"{	gl_FragColor = vec4(_UniformColor.x,_UniformColor.y,_UniformColor.z,_UniformColor.w*texture2D(_Texture, texCoord).x);"
		"}"
	);
	
	return _shDrawImage.Validate() && _shDrawPrimitive.Validate() && _shDrawText.Validate();
}

void gdiCanvas::Destroy()
{
	if(_texDrawImage.IsTexture())_texDrawImage.Destroy();
	_shDrawImage.Destroy();
}

void gdiCanvas::SetCanvasPlacement(int left, int top, int right, int bottom)
{
	static const float _ViewZfar = 10;
	static const float _ViewZnear = -2;

	_ViewWidth = right - left;
	_ViewHeight = bottom - top;
	_ViewLeft = left;
	_ViewTop = top;

	left = 0;
	right = _ViewWidth;
	top = _ViewHeight;
	bottom = 0;

	_Projection.Zero();

	_Projection.m[0][0] = 2/(float)(right - left);
	_Projection.m[1][1] = 2/(float)(bottom - top);
	_Projection.m[2][2] = 2/(float)(_ViewZfar - _ViewZnear);
	_Projection.m[3][3] = 1;
	_Projection.m[3][0] = -(right + left)/(float)(right - left);
	_Projection.m[3][1] = -(bottom + top)/(float)(bottom - top);
	_Projection.m[3][2] = -(_ViewZfar + _ViewZnear)/(float)(_ViewZfar - _ViewZnear);

	gl::SetViewport(_ViewLeft, _ViewTop, _ViewWidth, _ViewHeight);
}

void gdiCanvas::Use()
{
	_DrawingMode = DM_NONE;
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	SetViewport(_ViewLeft, _ViewTop, _ViewWidth, _ViewHeight);
	_LogGLError;	// Call gdiCanvas::SetCanvasPlacement before using
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void gdiCanvas::Reuse()
{
	_DrawingMode = DM_NONE;
}

void gdiCanvas::_SetDrawingMode(_tagDrawingMode dm)
{
	if(dm == _DrawingMode)return;
	_DrawingMode = dm;

	switch(dm)
	{
	case DM_DRAWIMAGE:
		_shDrawImage.Use();
		_shDrawImage.SetUniformMatrix4x4("_MVP",_Projection);
		_shDrawImage.SetTexture<GL_TEXTURE_2D>("_Texture",_texDrawImage.GetTextureName());
		break;
	case DM_DRAWPRIMITIVE:
		_shDrawPrimitive.Use();
		_shDrawPrimitive.SetUniformMatrix4x4("_MVP",_Projection);
		break;
	case DM_DRAWPRIMITIVE_UNICOLOR:
		_shDrawPrimitiveUniformColor.Use();
		_shDrawPrimitiveUniformColor.SetUniformMatrix4x4("_MVP",_Projection);
		break;
	case DM_DRAWTEXT:
		_shDrawText.Use();
		_shDrawText.SetUniformMatrix4x4("_MVP",_Projection);
		break;
	default: ASSERT(0);
	}

	gl::VertexAttributeBuffer::UseNull();
}

void gdiCanvas::FillRect(short x, short y, short w, short h, const rt::Vec4b& color1, const rt::Vec4b& color2, bool vertical)
{	
	_SetDrawingMode(DM_DRAWPRIMITIVE);
	if(color1.a != 255 || color2.a != 255)glEnable(GL_BLEND);
	short pos[] = { x,y, x,(short)(y+h), (short)(x+w),(short)(y+h), (short)(x+w),y };
	rt::Vec4b col[] = { color1, (vertical?color2:color1), color2, (vertical?color1:color2) };
	_shDrawPrimitive.SetVertexAttributePointer("_Color", col, 4, GL_UNSIGNED_BYTE);
	_shDrawPrimitive.SetVertexAttributePointer("_Position", pos, 2, GL_SHORT);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	_shDrawPrimitive.UnsetAllVertexAttributePointers();
	if(color1.a != 255 || color2.a != 255)glDisable(GL_BLEND);
}


void gdiCanvas::DrawRect(short x, short y, short w, short h, const rt::Vec4b& color, float width)
{
	_SetDrawingMode(DM_DRAWPRIMITIVE_UNICOLOR);
	if(color.a != 255)glEnable(GL_BLEND);
	SetLineWidth(width);
	short pos[] = { x,y, x,(short)(y+h), (short)(x+w),(short)(y+h), (short)(x+w),y };
	_shDrawPrimitiveUniformColor.SetVertexAttributePointer("_Position", pos, 2, GL_SHORT);
	_shDrawPrimitiveUniformColor.SetUniform("_UniformColor", rt::Vec4f(color.r/255.0f,color.g/255.0f,color.b/255.0f,color.a/255.0f));
	glDrawArrays(GL_LINE_LOOP, 0, 4);
	_shDrawPrimitiveUniformColor.UnsetAllVertexAttributePointers();
	if(color.a != 255)glDisable(GL_BLEND);
}

void gdiCanvas::SetFont(gdiFont& font, UINT height)
{
	if(font.HasGlyphMap())
	{
		if(height)
		{	_FontHeight = height;
		}
		else{ _FontHeight = font.GetGlyphHeight(0); }

		_pFont = &font;
	}
}

void gdiCanvas::DrawText(const rt::String_Ref& text, short x_in, short y, const rt::Vec4b& color)
{
	if(!_pFont || !_pFont->HasGlyphMap())
		return;

	typedef rt::Vec2s t_Pos;
	int len = text.GetLength();

	_SetDrawingMode(DM_DRAWTEXT);
	glEnable(GL_BLEND);
	_shDrawText.SetUniformMatrix4x4("_MVP",_Projection);
	_shDrawText.SetUniform("_UniformColor", rt::Vec4f(color.r/255.0f,color.g/255.0f,color.b/255.0f,color.a/255.0f));
	_shDrawText.UnsetAllTextures();
	_shDrawText.SetTexture<GL_TEXTURE_2D>("_Texture", _pFont->_GetBindedTexture());

	float fontscale = _FontHeight/(float)_pFont->GetGlyphMaxHeight();
	int vertex_count_max = 6*(1 + len);

	int bytesize = (sizeof(t_Pos) + sizeof(rt::Vec2f))*vertex_count_max;
	t_Pos* pos = (t_Pos*)alloca(sizeof(t_Pos)*vertex_count_max);
	rt::Vec2f* tex = (rt::Vec2f*)alloca(sizeof(rt::Vec2f)*vertex_count_max);

	y -= (dp_text_valignment - TEXT_ALIGNMENT_LEFT)*_FontHeight/2 + _pFont->_font_vspace;
	int texwidth = _pFont->GetGlyphMapWidth();
	int texheight = _pFont->GetGlyphMapHeight();
	int texystep = _pFont->GetGlyphMaxHeight();

	float x=0;
	int vertex = 0;
	LPCSTR pt = text.Begin();
	LPCSTR ptend = text.End();
	for(int c=0;c<len && pt<ptend;c++)
	{	
		GDICHAR code = os::__UTF16::GetNextU16CHAR(pt);
		gdiFont::_GlyphTextureClip	gtc;
		if(!_pFont->_GetGlyphTextureClip(code, &gtc) && gtc.width<=0)continue;

		t_Pos* p = &pos[vertex];
		rt::Vec2f* t = &tex[vertex];
		float xw = gtc.width*fontscale;

		p[0] = t_Pos((short)(0.5f + x),y);
		t[0] = rt::Vec2f(gtc.x/(float)texwidth, gtc.y/(float)texheight);
		p[2] = t_Pos((short)(x+xw + 0.5f),y);
		t[2] = rt::Vec2f((gtc.x+gtc.width)/(float)texwidth, gtc.y/(float)texheight);
		p[1] = t_Pos((short)(x+xw + 0.5f),(short)(y+_FontHeight));
		t[1] = rt::Vec2f((gtc.x+gtc.width)/(float)texwidth, (gtc.y+texystep)/(float)texheight);

		p[3] = p[0];
		t[3] = t[0];
		p[5] = p[1];
		t[5] = t[1];
		p[4] = t_Pos((short)(0.5f + x),(short)(y+_FontHeight));
		t[4] = rt::Vec2f(gtc.x/(float)texwidth, (gtc.y+texystep)/(float)texheight);

		x += xw + _pFont->_font_hspace + gtc.margin;
		vertex+=6;
	}

	int shiftx = (int)(x_in - (x-_pFont->_font_hspace)*(dp_text_alignment - TEXT_ALIGNMENT_LEFT)/2 + 0.5f);
	for(int c=0;c<vertex;c++)
		pos[c].x += shiftx;

	if(vertex)
	{
		_shDrawText.SetVertexAttributePointer("_Position", pos, 2, GL_SHORT);
		_shDrawText.SetVertexAttributePointer("_TexCoord", tex, 2, GL_FLOAT);
		glDrawArrays(GL_TRIANGLES, 0, vertex);
		_shDrawText.UnsetAllVertexAttributePointers();
	}
		
	glDisable(GL_BLEND);
}

gdiCanvasLayout::gdiCanvasLayout(const gdiCanvas& canvas)
	:_Canvas(canvas)
{
	_OffsetSpeed = 2;
	_Offset = 0;
	_Scale = 1;
	_ContentSize = -1;
	_ZoomSpeedExp = 1;
}

void gdiCanvasLayout::SetContentSize(int w, int h)
{
	_ContentSize.x = w;
	_ContentSize.y = h;
}

void gdiCanvasLayout::SetTranslateSpeed(double sx, double sy)
{
	_OffsetSpeed.x = sx;
	_OffsetSpeed.y = sy;
}

void gdiCanvasLayout::SetZoomSpeed(double szoom_x, double szoom_y)
{
	_ZoomSpeedExp.x = szoom_x;
	_ZoomSpeedExp.y = szoom_y;
}

void gdiCanvasLayout::OnUserInputEvent(const os::UserInputEvent& x)
{
	if(_ContentSize.Min() <= 0)return;

	switch(x.Type)
	{
	case os::UIEVT_MOUSE_DRAGBEGIN:
		_LastMouseX = x.Position.x;
		_LastMouseY = x.Position.y;
		break;
	case os::UIEVT_MOUSE_DRAG:
		_Offset.x += (x.Position.x - _LastMouseX)*_OffsetSpeed.x;
		_Offset.y += (x.Position.y - _LastMouseY)*_OffsetSpeed.y;
		_LastMouseX = x.Position.x;
		_LastMouseY = x.Position.y;
		break;
	case os::UIEVT_MOUSE_WHEEL:
		{		
			double ss_x = exp(x.Delta*_ZoomSpeedExp.x*0.001);
			double ss_y = exp(x.Delta*_ZoomSpeedExp.y*0.001);

			double px = x.Position.x - _Canvas._ViewLeft - _Offset.x;
			double py = x.Position.y - _Canvas._ViewTop - _Offset.y;

			_Offset.x -= px*(ss_x-1);
			_Offset.y -= py*(ss_y-1);

			_Scale.x *= ss_x;
			_Scale.y *= ss_y;
		}
		break;
	}
}

void gdiCanvasLayout::ApplyTo(ShaderProgramBase& shader, LPCSTR layout_name, bool as_double_type)
{
//#if !defined(PLATFORM_MAC)
//	if(as_double_type)
//	{
//		Vec4d v(_Offset.x, _Offset.y, _Scale.y, _Scale.y);
//		shader.SetUniform(layout_name, v);
//	}
//	else
//#endif
	{
		Vec4f v((float)_Offset.x, (float)_Offset.y, (float)_Scale.y, (float)_Scale.y);
		shader.SetUniform(layout_name, v);
	}
}



} // namespace gl

