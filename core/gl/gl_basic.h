#pragma once

/**
 * @file gl_basic.h
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
#include "../os/user_inputs.h"
#include "../rt/small_math.h"
#include "../rt/string_type.h"
#include "gl_pal.h"

#if defined(PLATFORM_IOS)
extern "C" void _objc_opengl_setup_disaply_link(LPVOID pUIView);
#endif

namespace gl
{
/** \addtogroup gl_basic
 * @ingroup gl
 *  @{
 */
class ShaderProgramBase;

#if		defined(PLATFORM_WIN)
#elif	defined(PLATFORM_MAC) || defined(PLATFORM_IOS)
	typedef	LPVOID HGLRC;
#else
#endif

class RenderContext
{
#if		defined(PLATFORM_WIN)
	HWND AssocWnd;
	HDC	dc;
public:
	INLFUNC HDC GetDC() const { return dc; }
#elif	defined(PLATFORM_MAC)
#endif

#if	defined(PLATFORM_IOS)
	GLuint _RenderBuffers[3];	// {RenderBuffer, FrameBuffer, DepthBuffer}
public:
	static float _ScreenScale;
#endif
protected:
	HGLRC rc;
	void _PostCreation();
public:
	RenderContext();
	INLFUNC ~RenderContext(){ if(rc)Destroy(); }
	INLFUNC operator HGLRC ()const{return rc;}
	INLFUNC bool	IsOk() const{ return rc!=0; }
	INLFUNC static HGLRC	GetCurrent()
	{
#if		defined(PLATFORM_WIN)
		return wglGetCurrentContext();
#elif	defined(PLATFORM_MAC) || defined(PLATFORM_IOS)
		return _objc_opengl_rendercontext_current();
#else
#endif
	}
	void Destroy();

#if		defined(PLATFORM_WIN)
	bool Create(HWND hWnd = NULL,DWORD AddFlags=0,UINT ColorBits = 24,UINT DepthBits = 24,UINT StencilBits = 0,UINT AccumBits = 0,UINT AlphaBits = 0,UINT AuxBuf = 0,bool DoubleBuf = true);
	INLFUNC bool Apply() const { return wglMakeCurrent(dc,rc); }
#elif	defined(PLATFORM_MAC)
	INLFUNC bool Create(LPVOID pNSOpenGLView)
	{	if((rc = _objc_opengl_rendercontext_get(pNSOpenGLView)))
		{	_PostCreation();	return true;}
		else return false;
	}
	INLFUNC bool Apply() const { return _objc_opengl_rendercontext_makecurrent(rc); }
#elif	defined(PLATFORM_IOS)
	INLFUNC bool Create(LPVOID pUIView)
	{	if((rc = _objc_opengl_rendercontext_create(pUIView,&_RenderBuffers[0])))
        {
            _objc_opengl_setup_disaply_link(pUIView);
            _PostCreation();	return true;
        }
		else return false;
	}
	INLFUNC bool Apply() const { return _objc_opengl_rendercontext_makecurrent(rc); }
#endif

protected:
	LPCSTR		_pOpenGLExtensionsString;
	
public:
	void SetBackgroundColor(const rt::Vec3b& c);
	void EnableVSync(bool enable);
	INLFUNC bool IsExtensionSupported(LPCSTR ExtName){ return (_pOpenGLExtensionsString && strstr(_pOpenGLExtensionsString,ExtName)); }
	INLFUNC LPCSTR GetExtensionString() const { return _pOpenGLExtensionsString; }
	void ReportDeviceDetails(rt::String& out);
};


class FramerateEstimator
{	static const int interval = 500;
	UINT		FrameCount;
	int			Start;	///< 10ms
	float		Fps;
public:
	FramerateEstimator(){ FrameCount = 0; Fps = 0; }
	INLFUNC void	Rendered(UINT t) ///< msec
	{	if(FrameCount == 0)Start = t - interval/2;
		FrameCount++;
		if(t > (UINT)(Start + interval))
		{	Fps = FrameCount*1000.0f/(t - Start);
			if(FrameCount > 10)
			{	FrameCount/=2;
				Start = (Start + t)/2;
			}
		}
	}
	INLFUNC float	GetFramerate() const { return Fps; }
};


enum _tagRenderingMode
{
	RENDER_ZTEST_ON = 0x100,
	RENDER_SCENEMASK = 0xff
};

extern void		StartRendering(DWORD mode = RENDER_ZTEST_ON);
extern void		FinalizeRendering(const RenderContext& rc);
extern void		SetViewport(int x, int y, int w, int h);
extern void		SetLineWidth(float w);
extern LPCSTR	GetLastError();
extern void		DrawUnitBox(ShaderProgramBase* pShader);

/** @}*/	
} // namespace gl

#if defined(_DEBUG)
#define _LogGLError	{ LPCSTR err; if((err = gl::GetLastError())){ _LOG(rt::String_Ref(err)<<rt::SS(" in " __FUNCTION__ ":L")<<__LINE__<<':'<<rt::SS(__FILE__)); ASSERT(0); } }
#else
#define _LogGLError {}
#endif

namespace gl
{
/** \addtogroup gl_basic
 * @ingroup gl
 *  @{
 */
class Camera
{
protected:
	rt::Mat4x4f		_MVP;
	rt::Mat4x4f		_ModelView;
	rt::Mat4x4f		_Projection;
	rt::Mat3x3f		_NormalTransform;
	bool			_MVPDirty;

	int				_ViewLeft;
	int				_ViewTop;
	int				_ViewRight;
	int				_ViewBottom;
	float			_ViewFovy;
	float			_ViewZnear;
	float			_ViewZfar;

	void _UpdatePerspective();

public:
	Camera();

	INLFUNC const rt::Mat4x4f& GetMVP() const 
	{	if(_MVPDirty)
		{	*(rt::_CastToNonconst(&_MVPDirty)) = false;	
			rt::_CastToNonconst(&_MVP)->Product(_Projection,_ModelView);
			rt::Mat3x3f a = _MVP;
			a.Inverse(*(rt::_CastToNonconst(&_NormalTransform)));
			(rt::_CastToNonconst(&_NormalTransform))->Transpose();
		}
		return _MVP;
	}
	INLFUNC const rt::Mat3x3f& GetNormalTransform() const
	{	GetMVP();
		return _NormalTransform;
	}
	void Use() const;

	void LoadIdentity();
	void Translate(float x, float y, float z);
	void Scale(float s);

	UINT GetViewportWidth() const { return _ViewRight - _ViewLeft; }
	UINT GetViewportHeight() const { return _ViewBottom -_ViewTop; }
	void SetViewport(int left, int top, int right, int bottom);
	void SetFOV(float fovy);
	void SetDepthRange(float z_near, float z_far);

	INLFUNC void Multiply(const rt::Mat4x4f& mat){ rt::Mat4x4f a = _ModelView; _ModelView.Product(a, mat); _MVPDirty = true; }
	void ApplyTo(ShaderProgramBase& shader, LPCSTR mvp_name = "_MVP", LPCSTR normal_transform_name = NULL) const;  // call after Use
};



/*
class FlyCamera : public Camera, public os::UserInputSubscriber 
{
protected:
	rt::Vec3f _vEyePt;
	rt::Vec3f _vLookAtPt;

	float _fCameraYawAngle;
	float _fCameraPitchAngle;
	float _fCameraPitchAngleWhenMouseDown;
	float _fCameraYawAngleWhenMouseDown;
	int _mouseXDown;
	int _mouseYDown;

	bool _bLeftDown, _bRightDown, _bUpDown, _bDownDown;

	ULONGLONG _timeStampLast;

public:
	FlyCamera() : Camera(), _timeStampLast(0) {}
	
	void SetViewParams( const rt::Vec3f &eyePt, const rt::Vec3f &lookAtPt );
	void OnUserInputEvent(const os::UserInputEvent& x);
	void OnRendering( ULONGLONG timeStamp );
};
*/

namespace _details
{

class VertexBufferObjectBase
{
protected:
	GLuint	VertexBufferName;
	DWORD	LastStorageHint;
	GLint	BufferSize;
public:
	enum
	{	StorageHint_NoChange = 0,
		StorageHint_OnBoard  = GL_STATIC_DRAW,
		StorageHint_Driver   = GL_DYNAMIC_DRAW,
		StorageHint_System	 = GL_STREAM_DRAW,
		// GL_STREAM_DRAW, GL_STREAM_READ, GL_STREAM_COPY, GL_STATIC_DRAW, GL_STATIC_READ, GL_STATIC_COPY, GL_DYNAMIC_DRAW, GL_DYNAMIC_READ, or GL_DYNAMIC_COPY.
		/*
		AccessFlag_ReadOnly  = GL_READ_ONLY,
		AccessFlag_WriteOnly = GL_WRITE_ONLY,
		AccessFlag_ReadWrite = GL_READ_WRITE
		 */
	};
public:
	INLFUNC GLuint GetBufferName(){ return VertexBufferName; }
	VertexBufferObjectBase(){ VertexBufferName = NULL; BufferSize = 0; }
	~VertexBufferObjectBase(){ Destroy(); }
	bool	Create(DWORD StorageHint = StorageHint_OnBoard);  ///< alloc buffer
	void	Destroy();
	
	INLFUNC bool	IsCreated() const { return VertexBufferName; }
	INLFUNC DWORD	GetStorageHint(){ return LastStorageHint; }
};

template<UINT VB_ARRAY_TARGET>
class VertexBufferObject:public VertexBufferObjectBase
{
public:
	INLFUNC void Use(){ glBindBuffer(VB_ARRAY_TARGET,VertexBufferName); _LogGLError; }
	INLFUNC static void UseNull(){ glBindBuffer(VB_ARRAY_TARGET,NULL); _LogGLError; }
	INLFUNC GLsizeiptr GetSize(){ return BufferSize; }
	INLFUNC void SetSize(GLsizeiptr size_in_byte,LPCVOID pData = NULL,DWORD StorageHint = StorageHint_NoChange)  ///< Copy data to buffer
	{	if(StorageHint)LastStorageHint = StorageHint;
		if( BufferSize != size_in_byte)
		{
			ASSERT(size_in_byte);
			Use();
			glBufferData(VB_ARRAY_TARGET,size_in_byte,pData,LastStorageHint);
			glGetBufferParameteriv(VB_ARRAY_TARGET,GL_BUFFER_SIZE,&BufferSize);
			_LogGLError; 
		}
	}
	INLFUNC void SetData(LPCVOID pData,GLsizeiptr size_in_byte,GLsizeiptr offset = 0)
	{	ASSERT(pData);
		Use();
		glBufferSubData(VB_ARRAY_TARGET,offset,size_in_byte,pData);
		_LogGLError;
	}
	/*
	INLFUNC void GetData(LPVOID pData,GLsizeiptr size_in_byte,GLsizeiptr offset = 0)
	{	ASSERT(pData);
		Use();
		glGetBufferSubData(VB_ARRAY_TARGET,offset,size_in_byte,pData);
		_LogGLError;
	}
	*/
	INLFUNC LPVOID	LockBuffer(DWORD AccessFlag)
	{	Use();
		LPVOID p = glMapBuffer(VB_ARRAY_TARGET,AccessFlag);
		_LogGLError;
		return p;
	}
	INLFUNC void UnlockBuffer()
	{	Use();
		glUnmapBuffer(VB_ARRAY_TARGET);
		_LogGLError;
	}
};

} // namespace _details

typedef _details::VertexBufferObject<GL_ARRAY_BUFFER>			VertexAttributeBuffer;
typedef _details::VertexBufferObject<GL_ELEMENT_ARRAY_BUFFER>	VertexIndexBuffer;

INLFUNC void StopUsingVBO(){ VertexAttributeBuffer::UseNull(); VertexIndexBuffer::UseNull(); }

extern UINT GetGLTypeSize(DWORD type);
extern const rt::String_Ref& GetGLTypeName(DWORD type);
/** @}*/
} // namespace gl



namespace gl
{
/** \addtogroup gl_basic
 * @ingroup gl
 *  @{
 */
namespace _details
{
	class _VersionedState
	{	UINT	_StateVersion;
	protected:	INLFUNC void UpdateStateVersion(){ _StateVersion++; }
	public:	_VersionedState(){ _StateVersion = 0; }
			INLFUNC bool IsStateUpdated(UINT& ver)
			{	ASSERT(ver<=_StateVersion);
				if(ver<_StateVersion)
				{	ver = _StateVersion;
					return true;
				}
				return false;
			}		
	};
	struct _TransformationBase: public os::UserInputSubscriber, public _VersionedState
	{	virtual const rt::Mat4x4f& GetTransformation() const = 0;
		virtual DWORD InterestedUserInputDevices() const = 0;
		//INLFUNC void Apply() const { glMultMatrixf(GetTransformation()); }
	};
	struct _ProjectionBase: public os::UserInputSubscriber, public _VersionedState
	{	virtual const rt::Mat4x4f& GetProjection() const = 0;
		virtual DWORD InterestedUserInputDevices() const = 0;
		//INLFUNC void Apply() const { glMultMatrixf(GetProjection()); }
	};
} // namespace _details

/** @}*/
} // namespace gl
/** @}*/