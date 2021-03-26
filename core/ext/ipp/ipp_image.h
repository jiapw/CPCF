#pragma once

//////////////////////////////////////////////////////////////////////
// Cross-Platform Core Foundation (CPCF)
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
//     * Neither the name of CPCF.  nor the names of its
//       contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//////////////////////////////////////////////////////////////////////

#include "ipp_core.h"
// OpenEXR
#include "openexr/IlmImf/ImfOutputFile.h"
#include "openexr/IlmImf/ImfInputFile.h"
#include "openexr/IlmImf/ImfChannelList.h"

#include "../../os/file_dir.h"
#include "../../rt/runtime_base.h"
#include <math.h>

#define ASSERT_SIZE(x1,x2) ASSERT((x1).GetWidth() == (x2).GetWidth() && (x1).GetHeight() == (x2).GetHeight())

#define IPPARG_IMG(x)	 ((x).GetImageData()),((int)(x).GetStep())
#define IPPARG_IMG2(x)	 ((x).GetImageData()),(x),((int)(x).GetStep())

namespace ipp
{

enum _tagImageCodec
{
	ImageCodec_Auto = 0,
	ImageCodec_PNG,
	ImageCodec_JPG,
	ImageCodec_GIF,
	ImageCodec_GIF_ANI,
	ImageCodec_PFM,
	ImageCodec_EXR,			
	ImageCodec_EXR_PIZ = ImageCodec_EXR,		// exr with wavelet, lossy
	ImageCodec_EXR_ZIP,		// exr with zlib, lossless
	ImageCodec_EXR_PXR24,	// exr with lossy 24-bit float compression
	ImageCodec_EXR_END,
	//ImageCodec_BMP,
	ImageCodec_Unk = -1,
};

enum _tagCodecFlag
{
	JPEG_ColorSampleNONE    = 0,    /* Corresponds to "No Subsampling". */
									/* Valid on a JPEG w/ any number of channels. */
	JPEG_ColorSample411     = 1,    /* Valid on a JPEG w/ 3 channels. */
	JPEG_ColorSample422     = 2,    /* Valid on a JPEG w/ 3 channels. */
};

namespace _details
{
class ImageCodec
{
protected:
	rt::Buffer<BYTE>	m_TempBuffer;
	int					m_BufferUsedLen;
	bool				_SetBufferSize(int size){ m_BufferUsedLen=0; return m_TempBuffer.SetSize(rt::max((UINT)size,(UINT)m_TempBuffer.GetSize())); }
public:
	ImageCodec(){ m_BufferUsedLen = 0; }
	LPCBYTE				GetOutput()const { return m_TempBuffer; }
	UINT				GetOutputSize()const { return m_BufferUsedLen; }
};
} // namespace _details

class ImageDecoder: public _details::ImageCodec
{
	int		m_DecodedImageWidth;
	int		m_DecodedImageHeight;
	int		m_DecodedImageStep;
	int		m_DecodedImageChannel;
	int		m_FrameCount;
	DWORD	m_ImageCodec;

public:
	ImageDecoder(){ m_DecodedImageWidth = m_DecodedImageHeight = m_DecodedImageStep = m_DecodedImageChannel = 0; }
	//bool	DecodeHeader(LPCBYTE image, UINT len, DWORD image_codec = ImageCodec_Auto);
	static	_tagImageCodec	DecodeFormat(LPCBYTE image, UINT DataLen);
	bool	Decode(LPCVOID image, UINT len, DWORD image_codec = ImageCodec_Auto);

	UINT	GetImageWidth()const { return m_DecodedImageWidth; }
	UINT	GetImageHeight()const { return m_DecodedImageHeight; }
	UINT	GetImageStep()const { return m_DecodedImageStep; }
	UINT	GetImageChannel()const { return m_DecodedImageChannel; }
	UINT	GetImageCodec()const { return m_ImageCodec; }
	UINT	GetFrameCount() const { return m_FrameCount; }

	LPCBYTE	GetOutput(UINT frame = 0)const { return m_TempBuffer.Begin() + frame*m_DecodedImageStep*m_DecodedImageHeight; }
	UINT	GetOutputSize()const { return m_DecodedImageStep*m_DecodedImageHeight; }
};

class ImageEncoder: public _details::ImageCodec
{
	int		m_Quality;
	int		m_Flag;
public:
	ImageEncoder(){ m_Quality = 95; m_Flag = 0; }
	void	SetQualityRatio(int quality){ ASSERT(quality<=100 && quality>=0); m_Quality = quality; }
	void	SetSubSamplingType(int	mode = 0){ m_Flag = mode; }

	bool	Encode(LPCBYTE pData,int Channel,int Width,int Height,int Step, DWORD codec = ImageCodec_JPG);	// codec:=_tagImageCodec
	static _tagImageCodec CodecFromExtName(const rt::String_Ref& filename);
};

namespace image_codec
{
	class _PFM_Header
	{	friend bool		_Open_PFM(LPCSTR fn,_PFM_Header* pHeader);
		friend bool		_Read_PFM(const _PFM_Header* pHeader,LPFLOAT pData,UINT ch,UINT step);
		os::File		file;
	public:
		UINT			width;
		UINT			height;
		UINT			ch;
	};
	extern bool _Write_PFM(LPCSTR fn,LPCFLOAT pData,UINT ch,UINT w,UINT h,UINT step);
	extern bool _Open_PFM(LPCSTR fn,_PFM_Header* pHeader);
	extern bool _Read_PFM(const _PFM_Header* pHeader,LPFLOAT pData,UINT ch,UINT step);
}

namespace _details
{

template<typename t_Value,UINT Channel>
struct _ValueType
{	typedef rt::Vec<t_Value,Channel> t_Val;
};
	template<typename t_Value>
	struct _ValueType<t_Value, 1>
	{	typedef t_Value t_Val;
	};

template<typename t_Value>
struct _ValueEnum;
	template<> struct _ValueEnum<BYTE>
	{	static BYTE	One(){ return 255; }
	};
	template<> struct _ValueEnum<float>
	{	static float One(){ return 1.0f; }
	};


} // namespace _details


template<typename t_Value,UINT Channel>
class Signal_Ref
{
public:
	typedef typename _details::_ValueType<t_Value,Channel>::t_Val t_Val;
	static const int chan_num = Channel;

	typedef t_Val*			LPValueType;
	typedef const t_Val*	LPCValueType;

	typedef Signal_Ref<t_Value,Channel>	Ref;

protected:
	t_Val*	_p;
	UINT	_len;

public:
	FORCEINL static UINT	GetChannels(){ return chan_num;}
	FORCEINL static UINT	GetBPS(){ return (sizeof(t_Val)/chan_num)<<3;}
	FORCEINL UINT           GetLength() const { return _len; }
	FORCEINL LPVOID         GetBits(){ return _p; }
	FORCEINL LPCVOID		GetBits() const { return _p; }

	FORCEINL Signal_Ref(const t_Val* p = nullptr, UINT len = 0){ _p = (t_Val*)p; _len = len; }
	FORCEINL Signal_Ref(const Signal_Ref& x):Signal_Ref(x._p, x._len){}

	template<typename T,UINT ch>
	FORCEINL const Signal_Ref& operator = (const Signal_Ref<T,ch> & x){ CopyFrom(x); return *this; }

	FORCEINL t_Val& operator()(int x){ return _p[x]; }
	FORCEINL const t_Val& operator()(int x) const { return _p[x]; }

	FORCEINL Ref GetRef(UINT x,UINT len) const { ASSERT(x+len<=GetLength()); return Ref(&_p[x],len); }
	FORCEINL Ref GetRef_Inside(UINT border) const {	ASSERT(border*2<GetLength()); return GetSubSignal(border,GetLength()-border*2);	}

	FORCEINL bool IsEmpty() const { _p == nullptr || _len == 0; }

	FORCEINL operator LPValueType (){ return _p; }
	FORCEINL operator LPCValueType () const { return _p; }

};

template<typename t_Val,UINT Channel>
class Signal: public Signal_Ref<t_Val, Channel>
{
	typedef Signal_Ref<t_Val, Channel> _SC;
	FORCEINL void __SafeFree()
	{
		_SafeFree32AL(_SC::_p);
	}
public:
	typedef Signal_Ref<t_Val,Channel>	Ref;
public:
	FORCEINL Signal(){}
	FORCEINL Signal(const Signal_Ref<t_Val, Channel>& x){ Ref::CopyFrom(x); }
	INLFUNC  bool SetLength(UINT co)
	{	if(co == _SC::_len){ return true; }
		else
		{	__SafeFree();
			if(co)
			{
				_SC::_p = _Malloc32AL(BYTE, co*sizeof(_SC::t_Val));
				if(_SC::_p == nullptr){ _SC::_len = 0; return false; }
			}
			_SC::_len = co;
			return true;
		}
	}
};


template<typename t_Value,UINT Channel>
class Volume_Ref
{
public:
	typedef rt::Vec<t_Value,Channel> t_Val;
	static const int chan_num = Channel;

	typedef t_Value*		LPValueType;
	typedef const t_Value*	LPCValueType;

	typedef Volume_Ref<t_Value,Channel>	Ref;

protected:
	t_Val*	_p;
	UINT	Width,Height,Depth;
	UINT	StepSize_WH;
	UINT	StepSize_H;

public:
	FORCEINL static UINT	GetChannels(){ return chan_num;}
	FORCEINL static UINT	GetBPS(){ return (sizeof(t_Val)/chan_num)<<3;}
	FORCEINL UINT			GetWidth() const { return Width; }
	FORCEINL UINT			GetHeight() const { return Height; }
	FORCEINL UINT			GetDepth() const { return Depth; }
	FORCEINL LPVOID			GetBits(){ return _p; }
	FORCEINL LPCVOID		GetBits() const { return _p; }

	FORCEINL Volume_Ref(const t_Val* p = nullptr, UINT w = 0,UINT h = 0,UINT d = 0,UINT step_wh = 0,UINT step_h = 0){ _p = (t_Val*)p; Width = w; Height = h; Depth = d; StepSize_WH = step_wh; StepSize_H = step_h; }
	FORCEINL Volume_Ref(const Volume_Ref& x):Volume_Ref(x._p, x.Width, x.Height, x.Depth, x.StepSize_WH, x.StepSize_H){}

	template<typename T,UINT ch>
	FORCEINL const Volume_Ref& operator = (const Volume_Ref<T,ch> & x){ CopyFrom(x); return *this; }

	FORCEINL Volume_Ref& Set(t_Value v)
	{	for(UINT z = 0; z<Depth; z++)for(UINT y = 0; y<Height; y++)for(UINT x = 0; x<Width; x++)
			(*this)(x,y,z) = v;
		return *this;
	}

	//FORCEINL Ref GetRef(UINT x,UINT len) const { ASSERT(x+len<=GetLength()); return Ref(&_p[x],len); }
	//FORCEINL Ref GetRef_Inside(UINT border) const {	ASSERT(border*2<GetLength()); return GetSubSignal(border,GetLength()-border*2);	}

	FORCEINL t_Val& operator()(int x, int y, int z){ return *(x + (t_Val*)&((LPBYTE)_p)[y*StepSize_H + z*StepSize_WH]); }
	FORCEINL const t_Val& operator()(int x, int y, int z) const { return *(x + (const t_Val*)&((LPCBYTE)_p)[x + y*StepSize_H + z*StepSize_WH]); }
	FORCEINL bool IsEmpty() const { _p == nullptr || Width == 0 || Height == 0 || Depth == 0; }
};

template<typename t_Value,UINT Channel>
class Volume: public Volume_Ref<t_Value, Channel>
{
	typedef Volume_Ref<t_Value, Channel> _SC;
	FORCEINL void __SafeFree(){ _SafeFree32AL(_SC::_p); }
public:
	typedef Volume_Ref<t_Value,Channel>	Ref;
public:
	FORCEINL Volume(){}
	FORCEINL Volume(const Signal_Ref<t_Value, Channel>& x){ SetSize(x); Ref::CopyFrom(x); }
	INLFUNC  bool SetSize(UINT w, UINT h, UINT d)
	{	if(w == _SC::Width && h == _SC::Height && d == _SC::Depth){ return true; }
		else
		{	__SafeFree();
			_SC::Width = _SC::Height = _SC::Depth = 0;
			if(w && h && d)
			{	
				int step_h = _EnlargeTo32AL(sizeof(t_Value)*Channel*w);
				_SC::_p = (_Malloc32AL(BYTE, step_h*h*d));
				if(_SC::_p)
				{	_SC::Width = d; _SC::Height = h; _SC::Depth = d;
					_SC::StepSize_WH = step_h*_SC::Height;
					_SC::StepSize_H = step_h;
				}else return false;
			}
			return true;
		}
	}
};

template<typename t_Value,int Channel>
class Image;

template<typename t_Value,int Channel>
class Image_Ref
{
public:
	typedef rt::Vec<t_Value,Channel>	t_Val;
	typedef t_Value						ValueType;
	typedef t_Value*					LPValueType;
	typedef const t_Value*				LPCValueType;
	static const int chan_num = Channel;

protected:
	UINT	Width;
	UINT	Height;
	UINT	Step_Bytes; //Step = Width*sizeof(t_Val) + Pad_Bytes
	t_Val*	lpData;

	FORCEINL t_Value*		GetValueAddress(UINT x, UINT y){ return (t_Value*)GetPixelAddress(x,y); }
	FORCEINL const t_Value*	GetValueAddress(UINT x, UINT y)const{ return (const t_Value*)GetPixelAddress(x,y); }

public:
	FORCEINL void Attach(LPCVOID p = nullptr, UINT w = 0, UINT h = 0, UINT step = 0){ Step_Bytes = step?step:w*Channel*sizeof(t_Value); Width = w; Height = h; lpData = (t_Val*)p; }
	FORCEINL void Attach(const Image_Ref& x){ Attach(x.lpData, x.Width, x.Height, x.Step_Bytes); }
	Image_Ref(LPCVOID p = nullptr, UINT w = 0, UINT h = 0, UINT step = 0){ Attach(p,w,h,step); }
	Image_Ref(const Image_Ref& x){ Attach(x); }
	template<typename T,int ch>
	FORCEINL const Image_Ref<t_Value, Channel>& operator = (const Image_Ref<T,ch> & x){ CopyFrom(x); return *this; }
	FORCEINL const Image_Ref<t_Value, Channel>& operator = (const Image_Ref<t_Value, Channel>& x){ CopyFrom(x); return *this; }
	operator const IppiSize& () const { return (IppiSize&)(*this); }
	FORCEINL static UINT	GetChannels(){ return chan_num;}
	FORCEINL static UINT	GetBPV(){ return sizeof(t_Value)<<3;}
	FORCEINL static UINT	GetBPP(){ return Channel*sizeof(t_Value)<<3;}
	FORCEINL UINT			GetWidth() const { return Width; }
	FORCEINL UINT			GetHeight() const { return Height; }
	FORCEINL UINT			GetStep() const { return Step_Bytes; }	// in Bytes
	FORCEINL LPVOID			GetBits(){ return lpData; }
	FORCEINL LPCVOID		GetBits() const { return lpData; }
	FORCEINL bool			IsEmpty() const { return lpData == nullptr || Width == 0 || Height == 0; }
	FORCEINL t_Val &		operator ()(UINT x, UINT y){ return *(t_Val*)(&((LPBYTE)lpData)[y*Step_Bytes+x*sizeof(t_Val)]); }
	FORCEINL const t_Val &	operator ()(UINT x, UINT y) const{ return *(t_Val*)(&((LPBYTE)lpData)[y*Step_Bytes+x*sizeof(t_Val)]); }
	FORCEINL t_Val*			GetPixelAddress(UINT x, UINT y){ return &(*this)(x,y); }
	FORCEINL const t_Val*	GetPixelAddress(UINT x, UINT y) const { return &(*this)(x,y); }
	FORCEINL t_Val*			GetScanline(UINT y){ return (t_Val*)(((LPBYTE)lpData) + y*Step_Bytes); }
	FORCEINL const t_Val*	GetScanline(UINT y) const { return (const t_Val*)(((LPCBYTE)lpData) + y*Step_Bytes); }
	FORCEINL t_Val			GetInterplotedPixel(float x, float y) const
	{	t_Val	ret;
		if(x>0 && x<Width-1 && y>0 && y<Height-1)
		{	int ix = (int)x;
			int iy = (int)y;
			ret.Interpolate((*this)(ix, iy), (*this)(ix+1, iy), (*this)(ix, iy+1), (*this)(ix+1, iy+1), x - ix, y - iy);
			return ret;
		}
		if(x>0 && x<Width-1)
		{
			t_Val	ret;	int ix = (int)x;
			if(y<0)
				ret.Interpolate((*this)(ix,0), (*this)(ix+1,0), x - ix);
			else
				ret.Interpolate((*this)(ix,Height-1), (*this)(ix+1,Height-1), x - ix);
			return ret;
		}
		if(y>0 && y<Height-1)
		{
			t_Val	ret;	int iy = (int)y;
			if(x<0)
				ret.Interpolate((*this)(0, iy), (*this)(0, iy+1), y - iy);
			else
				ret.Interpolate((*this)(Width-1,iy), (*this)(Width-1,iy+1), y - iy);
			return ret;
		}
		if(x<0 && y<0)
		{	return (*this)(0,0);
		}
		else if(x>=Width-1 && y<0)
		{	return (*this)(Width-1,0);
		}
		else if(x<0 && y>=Height-1)
		{	return (*this)(0,Height-1);
		}
		else
		{	return (*this)(Width-1,Height-1);
		}
	}
public:
	typedef Image_Ref<t_Value,Channel>	Ref;
	typedef Image_Ref<t_Value,1>		Ref_1C;

	FORCEINL Ref GetSub(const Size& sz){ return Ref(lpData,sz.width,sz.height,GetStep());	}
	FORCEINL Ref GetSub(UINT w,UINT h){ return Ref(lpData,w,h,GetStep()); }
	FORCEINL Ref Crop(int x,int y,UINT w,UINT h)
	{	if(x<0){ w += x; x=0; }
		if(y<0){ h += y; y=0; }
		return Ref(GetPixelAddress(x,y),rt::min(Width - x,w),rt::min(Height - y,h),GetStep());
	}
	FORCEINL const Ref Crop(int x,int y,UINT w,UINT h) const { return rt::_CastToNonconst(this)->Crop(x,y,w,h); }
	FORCEINL const Ref GetSub(UINT x,UINT y,UINT w,UINT h) const
	{	return rt::_CastToNonconst(this)->GetSub(x,y,w,h); }
	FORCEINL Ref GetSub(UINT x,UINT y,UINT w,UINT h)
	{	ASSERT(x + w <= Width);
		ASSERT(y + h <= Height);
		return Ref(GetPixelAddress(x,y),w,h,GetStep());
	}
	
	FORCEINL Ref GetSub_Inside(UINT border)
	{	ASSERT(border*2<GetWidth());
		ASSERT(border*2<GetHeight());
		return GetSub(border,border,GetWidth()-border*2,GetHeight()-border*2);
	}
	FORCEINL const Ref GetSub_Inside(UINT border) const
	{	return rt::_CastToNonconst(this)->GetSub_Inside(border); }

	FORCEINL Ref GetSub_Inside(UINT border_x,UINT border_y)
	{	ASSERT(border_x*2<GetWidth());
		ASSERT(border_y*2<GetHeight());
		return GetSub(border_x,border_y,GetWidth()-border_x*2,GetHeight()-border_y*2);
	}
	FORCEINL const Ref GetSub_Inside(UINT border_x,UINT border_y) const
	{	return rt::_CastToNonconst(this)->GetSub_Inside(border_x,border_y); 
	}
	FORCEINL Ref GetSub_MaxBox(UINT aspect_x = 1, UINT asqect_y = 1)
	{	if(GetWidth()*asqect_y <= GetHeight()*aspect_x)
		{	int h = GetWidth()*asqect_y/aspect_x;
			return GetSub(0, (GetHeight()-h)/2, GetWidth(), h);
		}
		else
		{	int w = GetHeight()*aspect_x/asqect_y;
			return GetSub((GetWidth()-w)/2, 0, w, GetHeight());
		}
	}
public:
	FORCEINL const Size&		GetRegion()const{ return *((const Size*)this); }
	FORCEINL LPValueType		GetImageData() const { return (LPValueType)GetBits(); }

public:
	INLFUNC  bool Save(LPCSTR fn, _tagImageCodec ic = ImageCodec_Auto) const	// ipp::GetEnv()->JpegEncodeQuality for controling of the quality of the jpeg file
	{	if(IsEmpty())return false;
		if(ic == ImageCodec_Auto)ic = ImageEncoder::CodecFromExtName(fn);
		if(ic == ImageCodec_PFM)
		{
			if(	rt::IsTypeSame<float, t_Value>::Result)
			{	return image_codec::_Write_PFM(fn, (LPCFLOAT)this->lpData, Channel, GetWidth(), GetHeight(), GetStep());	}
			else
			{	Image<float, Channel> img_temp;
				img_temp.SetSize(GetWidth(), GetHeight());
				img_temp.CopyFrom(*this);
				return img_temp.Save(fn, ic);
			}
		}
		else if(ic>=ImageCodec_EXR && ic<ImageCodec_EXR_END)
		{
			Imf::Compression cm;
			switch(ic)
			{	case ImageCodec_EXR_ZIP:	cm = Imf::ZIP_COMPRESSION;		break;
				case ImageCodec_EXR_PXR24:	cm = Imf::PXR24_COMPRESSION;	break;
				case ImageCodec_EXR_PIZ:	cm = Imf::PIZ_COMPRESSION;		break;
				default: return false;
			}

			Imf::PixelType pt;
			if(rt::IsTypeSame<float, t_Value>::Result)
			{	pt = Imf::FLOAT;
			}
			else if(rt::IsTypeSame<UINT, t_Value>::Result)
			{	pt = Imf::UINT;
			}
			else
			{	Image<float, Channel> img_temp;
				img_temp.SetSize(GetWidth(), GetHeight());
				img_temp.CopyFrom(*this);
				return img_temp.Save(fn, ic);
			}

			Imf::Header header(GetWidth(), GetHeight(), 1.0f, Imath::V2f(0,0), 1.0f, Imf::INCREASING_Y, cm);
			Imf::FrameBuffer frameBuffer;
			switch(Channel)
			{	case 1: header.channels().insert ("G", Imf::Channel (pt));
						frameBuffer.insert ("G", Imf::Slice(pt,	(char *) lpData, sizeof(t_Val),	GetStep()));
						break;
				case 2:	header.channels().insert ("G", Imf::Channel (pt));
						header.channels().insert ("Z", Imf::Channel (pt));
						frameBuffer.insert ("G", Imf::Slice(pt,	(char *)lpData					  , sizeof(t_Val),	GetStep()));
						frameBuffer.insert ("Z", Imf::Slice(pt,	((char *)lpData) + sizeof(t_Value), sizeof(t_Val),	GetStep()));
						break;
				case 3: header.channels().insert ("R", Imf::Channel (pt));
						header.channels().insert ("G", Imf::Channel (pt));
						header.channels().insert ("B", Imf::Channel (pt));
						frameBuffer.insert ("R", Imf::Slice(pt,	(char *)lpData					    , sizeof(t_Val),	GetStep()));
						frameBuffer.insert ("G", Imf::Slice(pt,	((char *)lpData) + sizeof(t_Value)  , sizeof(t_Val),	GetStep()));
						frameBuffer.insert ("B", Imf::Slice(pt,	((char *)lpData) + sizeof(t_Value)*2, sizeof(t_Val),	GetStep()));
						break;
				case 4:	header.channels().insert ("R", Imf::Channel (pt));
						header.channels().insert ("G", Imf::Channel (pt));
						header.channels().insert ("B", Imf::Channel (pt));
						header.channels().insert ("A", Imf::Channel (pt));
						frameBuffer.insert ("R", Imf::Slice(pt,	(char *)lpData					    , sizeof(t_Val),	GetStep()));
						frameBuffer.insert ("G", Imf::Slice(pt,	((char *)lpData) + sizeof(t_Value)  , sizeof(t_Val),	GetStep()));
						frameBuffer.insert ("B", Imf::Slice(pt,	((char *)lpData) + sizeof(t_Value)*2, sizeof(t_Val),	GetStep()));
						frameBuffer.insert ("A", Imf::Slice(pt,	((char *)lpData) + sizeof(t_Value)*3, sizeof(t_Val),	GetStep()));
						break;
				default: return false;
			}

			try{
				os::File::CreateDirectories(fn);
				Imf::OutputFile file(fn, header);
				file.setFrameBuffer(frameBuffer);
				file.writePixels(GetHeight());
				return true;
			}
			catch(...){ return false; }
		}
		else if(ic != ImageCodec_Unk)
		{	// LDR formats
			if(rt::IsTypeSame<BYTE, t_Value>::Result)
			{
				os::File		file;
				ImageEncoder	enc;

				if(ic == ImageCodec_JPG)
					enc.SetQualityRatio(GetEnv()->JpegEncodeQuality);

				if(	enc.Encode((LPCBYTE)lpData, chan_num, GetWidth(), GetHeight(), GetStep(), ic) &&
					file.Open(fn, os::File::Normal_Write, true) &&
					file.Write(enc.GetOutput(), enc.GetOutputSize()) == enc.GetOutputSize()
				){	return true; }
				else
				{	file.Close();
					os::File::Remove(fn);
					return false;
				}
			}
			else
			{	Image<BYTE, Channel> img_temp;
				img_temp.SetSize(GetWidth(), GetHeight());
				img_temp.CopyFrom(*this);
				return img_temp.Save(fn, ic);
			}
		}
		return false;
	}

public:
	template<typename VAL, int CHAN2>
	FORCEINL void CopyFrom(const Image_Ref<VAL,CHAN2> & in)
	{	ASSERT_SIZE(*this, in);
		for(UINT y=0;y<Height;y++)
		{	t_Val* d = GetScanline(y);
			t_Val* dend = d + Width;
			const typename Image_Ref<VAL,CHAN2>::t_Val* s = in.GetScanline(y);
			for(;d<dend;d++,s++)*d = *s;
	}	}
	template<typename VAL>
	FORCEINL void Mean(rt::Vec<VAL, Channel>& mean) const
	{	mean = 0;
		for(UINT y=0;y<Height;y++)
			for(UINT x=0;x<Width;x++)
				mean += (*this)(x,y);
		mean /= Width*Height;
	}
	template<typename VAL>
	FORCEINL void Sum(rt::Vec<VAL, Channel>& sum) const
	{	rt::Vec<double, Channel> sumv(0);
		for(UINT y=0;y<Height;y++)
			for(UINT x=0;x<Width;x++)
				sumv += (*this)(x,y);
		sum = sumv;
	}
	FORCEINL void Set(const t_Val& v)
	{	for(UINT y=0;y<Height;y++)
			for(UINT x=0;x<Width;x++)
				(*this)(x,y) = v;
	}
	FORCEINL void MinMax(t_Val& vmin, t_Val& vmax) const
	{	vmin = vmax = (*this)(0,0);
		for(UINT y=0;y<Height;y++)
			for(UINT x=0;x<Width;x++)
			{	const t_Val& p = (*this)(x,y);
				vmin.Min(vmin, p);
				vmax.Max(vmax, p);
			}
	}
	FORCEINL void CopyFrom(const Ref & in)
	{	ASSERT_SIZE(*this, in);
		for(UINT y=0;y<Height;y++)
			memcpy(GetScanline(y), in.GetScanline(y), sizeof(t_Val)*Width);
	}
	FORCEINL void Zero()
	{	for(UINT y=0;y<Height;y++)
			memset(GetScanline(y), 0, sizeof(t_Val)*Width);
	}
	template<typename VAL>
	FORCEINL void Add(const Image_Ref<VAL,Channel>& in)
	{	ASSERT_SIZE(*this, in);
		for(UINT y=0;y<Height;y++)
		{	t_Val* d = GetScanline(y);
			t_Val* dend = d + Width;
			const typename Image_Ref<VAL,Channel>::t_Val* s = in.GetScanline(y);
			for(;d<dend;d++,s++)*d += *s;
	}	}
};



template<typename t_Value,int Channel>
class Image:public Image_Ref<t_Value, Channel>
{
    typedef rt::Vec<t_Value, Channel>	t_Val;
	typedef Image_Ref<t_Value, Channel>	Ref;
    typedef Image_Ref<t_Value, Channel>	_SC;

	void Attach(LPCVOID p = nullptr, UINT w = 0, UINT h = 0, UINT step = 0){ ASSERT(0); }
	void Attach(const Image_Ref<t_Value, Channel>& x){ ASSERT(0); }

	FORCEINL void	__SafeFree()
	{
		_SafeFree32AL(Ref::lpData);
	}
public:
	FORCEINL Image(){}
	FORCEINL ~Image(){ __SafeFree(); }
	FORCEINL Image(const Image<t_Value, Channel>& x){ if(!x.IsEmpty()){ VERIFY(SetSize(x)); Ref::CopyFrom(x); } }
    FORCEINL Image(UINT w, UINT h){ SetSize(w,h); }
    
    template<typename T, int ch>
    FORCEINL Image(const Image<T, ch>& x){ if(!x.IsEmpty()){ VERIFY(SetSize(x)); Ref::CopyFrom(x); } }
	template<typename T, int ch>
    FORCEINL Image(const Image_Ref<T, ch>& x){ VERIFY(SetSize(x)); Ref::CopyFrom(x); }
    
	template<typename T,UINT ch>
	FORCEINL const Image_Ref<t_Value, Channel>& operator = (const Image_Ref<T,ch> & x){ Ref::CopyFrom(x); return *this; }
	FORCEINL const Ref& operator = (const Ref& x){ Ref::CopyFrom(x); return *this; }
    
	template<typename t_Value2,UINT Channel2>
	FORCEINL bool	SetSize(const Image_Ref<t_Value2, Channel2>& x){ return SetSize(x.GetWidth(), x.GetHeight()); }
	FORCEINL bool	SetSize(const IppiSize& x){ return SetSize(x.width, x.height); }
	INLFUNC  bool	SetSize(UINT w, UINT h)
	{	if(w==Ref::Width && h==Ref::Height){ return true; }
		else
		{	__SafeFree();
			int step_size = 0;
			if(w&&h)
			{
				step_size = (int)_EnlargeTo32AL(sizeof(t_Value)*Channel*w);
				Ref::lpData = (typename Ref::t_Val*)_Malloc32AL(BYTE, step_size*h);
				if(Ref::lpData == nullptr)
				{	Ref::Width=Ref::Height=0;
					return false;
				}
			}
			
			Ref::Width=w; Ref::Height=h; 
			Ref::Step_Bytes = (UINT)step_size;
			return true;
		}
	}
	FORCEINL bool	Load(LPCSTR fn)
	{	
		os::FileRead<BYTE> f(fn);
		_tagImageCodec ic = ImageDecoder::DecodeFormat(f, (UINT)f.GetSize());
		if(ic == ImageCodec_PFM)
		{	if(!rt::IsTypeSame<float, t_Value>::Result)return false;
			image_codec::_PFM_Header h;
			return	image_codec::_Open_PFM(fn, &h) && 
					SetSize(h.width, h.height) &&
                    image_codec::_Read_PFM(&h, (LPFLOAT)_SC::lpData, Channel, _SC::GetStep());
		}
		else if(ic >= ImageCodec_EXR && ic < ImageCodec_EXR_END)
		{
			Imf::InputFile file (fn);
			Imath::Box2i dw = file.header().dataWindow();
			if(!SetSize(dw.max.x - dw.min.x + 1, dw.max.y - dw.min.y + 1))return false;

			Imf::PixelType pt;
			if(rt::IsTypeSame<float, t_Value>::Result)
			{	pt = Imf::FLOAT;
			}
			else if(rt::IsTypeSame<UINT, t_Value>::Result)
			{	pt = Imf::UINT;
			}
			else
			{	
				Image<float, Channel> img_temp;
				if(img_temp.Load(fn))
				{	this->CopyFrom(img_temp);
					return true;
				}
				else 
					return false;
			}

			Imf::FrameBuffer frameBuffer;
			switch(Channel)
			{	case 1: frameBuffer.insert ("G", Imf::Slice(pt,	(char *)_SC::lpData, sizeof(t_Val),	_SC::GetStep()));
						break;
				case 2:	frameBuffer.insert ("G", Imf::Slice(pt,	(char *)_SC::lpData					  , sizeof(t_Val),	_SC::GetStep()));
						frameBuffer.insert ("Z", Imf::Slice(pt,	((char *)_SC::lpData) + sizeof(t_Value), sizeof(t_Val),	_SC::GetStep()));
						break;
				case 3: frameBuffer.insert ("R", Imf::Slice(pt,	(char *)_SC::lpData					    , sizeof(t_Val),	_SC::GetStep()));
						frameBuffer.insert ("G", Imf::Slice(pt,	((char *)_SC::lpData) + sizeof(t_Value)  , sizeof(t_Val),	_SC::GetStep()));
						frameBuffer.insert ("B", Imf::Slice(pt,	((char *)_SC::lpData) + sizeof(t_Value)*2, sizeof(t_Val),	_SC::GetStep()));
						break;
				case 4:	frameBuffer.insert ("R", Imf::Slice(pt,	(char *)_SC::lpData					    , sizeof(t_Val),	_SC::GetStep()));
						frameBuffer.insert ("G", Imf::Slice(pt,	((char *)_SC::lpData) + sizeof(t_Value)  , sizeof(t_Val),	_SC::GetStep()));
						frameBuffer.insert ("B", Imf::Slice(pt,	((char *)_SC::lpData) + sizeof(t_Value)*2, sizeof(t_Val),	_SC::GetStep()));
						frameBuffer.insert ("A", Imf::Slice(pt,	((char *)_SC::lpData) + sizeof(t_Value)*3, sizeof(t_Val),	_SC::GetStep()));
						break;
				default: return false;
			}

			try{
				file.setFrameBuffer (frameBuffer);
				file.readPixels (dw.min.y, dw.max.y);
				return true;
			}
			catch(...)
			{	return false;
			}
		}
		else return Load(f,(UINT)f.GetSize());
	}
	FORCEINL bool	Load(LPCVOID data, UINT data_len, UINT* pOriginalChannel = nullptr)
	{	ImageDecoder	dec;
		if(	data_len &&
			dec.Decode(data,data_len) &&
			SetSize(dec.GetImageWidth(), dec.GetImageHeight())
		)
		{	if(pOriginalChannel)*pOriginalChannel = dec.GetImageChannel();
			switch(dec.GetImageChannel())
			{	case 1:	*((Ref*)this) = Image_Ref<BYTE,1>(dec.GetOutput(), dec.GetImageWidth(), dec.GetImageHeight(), dec.GetImageStep()); return true;
				case 2: *((Ref*)this) = Image_Ref<BYTE,2>(dec.GetOutput(), dec.GetImageWidth(), dec.GetImageHeight(), dec.GetImageStep()); return true;
				case 3: *((Ref*)this) = Image_Ref<BYTE,3>(dec.GetOutput(), dec.GetImageWidth(), dec.GetImageHeight(), dec.GetImageStep()); return true;
				case 4: *((Ref*)this) = Image_Ref<BYTE,4>(dec.GetOutput(), dec.GetImageWidth(), dec.GetImageHeight(), dec.GetImageStep()); return true;
				default: return false;
			}
		}
		return false;
	}
	FORCEINL void	Empty(){ __SafeFree(); }
};

typedef Image_Ref<float,1>	ImageRef_1c32f;
typedef Image_Ref<float,3>	ImageRef_3c32f;
typedef Image_Ref<float,4>	ImageRef_4c32f;

typedef Image_Ref<BYTE,1>	ImageRef_1c8u;
typedef Image_Ref<BYTE,3>	ImageRef_3c8u;
typedef Image_Ref<BYTE,4>	ImageRef_4c8u;

typedef Image<float,1>	Image_1c32f;
typedef Image<float,3>	Image_3c32f;
typedef Image<float,4>	Image_4c32f;

typedef Image<BYTE,1>	Image_1c8u;
typedef Image<BYTE,3>	Image_3c8u;
typedef Image<BYTE,4>	Image_4c8u;


} // namespace ipp



