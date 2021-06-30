#include "gl_basic.h"
#include "gl_shader.h"
#include "../os/multi_thread.h"
#include <math.h>

#if	defined(PLATFORM_MAC) || defined(PLATFORM_IOS)
extern "C" void _objc_opengl_rendercontext_swapbuffer(LPVOID pNSOpenGLContext);

#if	defined(PLATFORM_MAC)
extern "C" void _objc_opengl_swapinterval(LPVOID pNSOpenGLContext, GLint v);
#endif

#if defined(PLATFORM_IOS)
extern "C" double _objc_opengl_get_screen_scale();
#endif
#endif

#if defined(PLATFORM_WIN)

bool _glExternsionLoaded = false;

#define EACHITEM(FUNCPROTO, Name)	FUNCPROTO Name = NULL;
GLEXTENTIONS_FUNCTIONS
#undef EACHITEM

#endif

namespace gl
{

#if	defined(PLATFORM_IOS)
float RenderContext::_ScreenScale = 1;
#endif

RenderContext::RenderContext()
{
	rc = NULL;

#if		defined(PLATFORM_WIN)
	AssocWnd = NULL;
	dc = NULL;
#else
#endif
}

void RenderContext::SetBackgroundColor(const rt::Vec3b& c)
{
	Apply();
	glClearColor(c.x/255.0f,c.y/255.0f,c.z/255.0f,1.0f);
}

void RenderContext::_PostCreation()
{
	SetBackgroundColor(rt::Vec3b(240, 200, 40));
#if defined(PLATFORM_OPENGL_SUPPORT)
	glPolygonMode(GL_FRONT,GL_FILL);
	glPolygonMode(GL_BACK,GL_LINE);
#endif
	_pOpenGLExtensionsString = (LPCSTR)glGetString(GL_EXTENSIONS);

#if defined(PLATFORM_WIN)
	if(!_glExternsionLoaded)
	{	
		_glExternsionLoaded = true;

#define EACHITEM(FUNCPROTO, Name)	((PROC&)Name) = wglGetProcAddress(#Name);
		GLEXTENTIONS_FUNCTIONS
#undef EACHITEM
	}
#undef LOAD_GLEXT_FUNCTIONS
#endif
	
#if	defined(PLATFORM_IOS)
	_ScreenScale = (float)_objc_opengl_get_screen_scale();
#endif
}
	
void RenderContext::ReportDeviceDetails(rt::String& out)
{
	struct GetGLVal {
		GLint name;
		GetGLVal(int pname)
		{	glGetIntegerv(pname, &name);
			_LogGLError;
		}
		operator int () const { return name; }
	};
	
	out.Empty();
	// Vertex/Fragment Shader
	out =	rt::SS("OpenGL Driver Details") +
			rt::SS("\n - Vendor: ") + (LPCSTR)glGetString(GL_VENDOR) + 
			rt::SS("\n - Render: ") + (LPCSTR)glGetString(GL_RENDERER) +
			rt::SS("\n - Version: ") + (LPCSTR)glGetString(GL_VERSION) +
			rt::SS("\n - Shading Language: ") + ((LPCSTR)glGetString(GL_SHADING_LANGUAGE_VERSION)) + 
				(IsExtensionSupported(" GL_ARB_gpu_shader_fp64 ")?rt::String_Ref(" FP64"):rt::String_Ref()) +
			rt::SS("\nBasic Pipeline") +
			rt::SS("\n - Max Texture Size: ") + GetGLVal(GL_MAX_TEXTURE_SIZE) + 
			rt::SS("\n - Max Cubemap Size: ") + GetGLVal(GL_MAX_CUBE_MAP_TEXTURE_SIZE) + 
			rt::SS("\n - Max Render Buffer Size: ") + GetGLVal(GL_MAX_RENDERBUFFER_SIZE) + 
			rt::SS("\n - Max Anisotropy: ") + GetGLVal(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT) + 
			rt::SS("\nVertex Shader") + 
			rt::SS("\n - Max Vertex Attributes: ") + GetGLVal(GL_MAX_VERTEX_ATTRIBS) +
			rt::SS("\n - Max Uniform Vectors: ") + GetGLVal(GL_MAX_VERTEX_UNIFORM_VECTORS) +
			rt::SS("\n - Max Varying Vectors: ") + GetGLVal(GL_MAX_VARYING_VECTORS) +
			rt::SS("\n - Max Texture Units: ") + GetGLVal(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS) +
			rt::SS("\nFragment Shader") +
			rt::SS("\n - Max Uniform Vectors: ") + GetGLVal(GL_MAX_FRAGMENT_UNIFORM_VECTORS) +
			rt::SS("\n - Max Texture Units: ") + GetGLVal(GL_MAX_TEXTURE_IMAGE_UNITS);

	if(IsExtensionSupported(" ARB_geometry_shader4 "))
	{
		out += rt::SS("\nGeometry Shader") + 
			   rt::SS("\n - Max Output Vertices: ") + GetGLVal(GL_MAX_GEOMETRY_OUTPUT_VERTICES_ARB) + 
			   rt::SS("\n - Max Output Total Componets: ") + GetGLVal(GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS_ARB) + 
			   rt::SS("\n - Max Input Varying Vectors: ") + GetGLVal(GL_MAX_VERTEX_VARYING_COMPONENTS_ARB) + 
			   rt::SS("\n - Max Output Varying Vectors: ") + GetGLVal(GL_MAX_GEOMETRY_VARYING_COMPONENTS_ARB) + 
			   rt::SS("\n - Max Uniform Vectors: ") + GetGLVal(GL_MAX_GEOMETRY_UNIFORM_COMPONENTS_ARB) + 
			   rt::SS("\n - Max Texture Units: ") + GetGLVal(GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS_ARB);
	}
}

#if		defined(PLATFORM_WIN)
bool RenderContext::Create(HWND hWnd,DWORD AddFlags,UINT ColorBits,UINT DepthBits,UINT StencilBits,UINT AccumBits,UINT AlphaBits,UINT AuxBuf,bool DoubleBuf)
{
	ASSERT(dc == NULL);
	VERIFY(dc = ::GetDC(hWnd));
	AssocWnd = hWnd;

	PIXELFORMATDESCRIPTOR pfd;
	ZeroMemory(&pfd,sizeof(PIXELFORMATDESCRIPTOR));

	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.cAccumBits = AccumBits;
	pfd.cAlphaBits = AlphaBits;
	pfd.cAuxBuffers = AuxBuf;
	pfd.cColorBits = ColorBits;
	pfd.cDepthBits = DepthBits;
	pfd.cStencilBits = StencilBits;
	pfd.iLayerType = PFD_MAIN_PLANE;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL;
	if(DoubleBuf)pfd.dwFlags|=PFD_DOUBLEBUFFER;
	pfd.dwFlags|=AddFlags;

	int pixelformat;

    if(!(pixelformat = ChoosePixelFormat(dc, &pfd)))
    {
		::ReleaseDC(AssocWnd,dc);
		dc=0;
        return false;
    }

    if(!SetPixelFormat(dc, pixelformat, &pfd))
    {
		::ReleaseDC(AssocWnd,dc);
		dc=0;
        return false;
    }

	rc = wglCreateContext(dc);
	if(!rc)
	{	ReleaseDC(AssocWnd,dc);
		dc=0;
        return false;
	}

	_PostCreation();
	return true;
}
#elif	defined(PLATFORM_MAC) || defined(PLATFORM_IOS)
#else
	ASSERT_STATIC(0);
#endif

void RenderContext::Destroy()
{
#if		defined(PLATFORM_WIN)
	wglDeleteContext(rc);
	rc = NULL;
#pragma warning(push)
#pragma warning(disable:4302)
	if( ((DWORD)AssocWnd) != 0xffffffff )
#pragma warning(pop)
		ReleaseDC(AssocWnd,dc);
	dc = NULL;
#elif	defined(PLATFORM_MAC)
	// do nothing
#elif	defined(PLATFORM_IOS)
	_objc_opengl_rendercontext_destroy(rc);
#else
#endif
}


void StartRendering(DWORD mode)
{
	if(mode&RENDER_ZTEST_ON)
	{	glEnable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	}
	else
	{	glDisable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT);
	}
}

void RenderContext::EnableVSync(bool enable)
{
#if defined(PLATFORM_WIN)
	wglSwapIntervalEXT(enable!=0);
#elif defined(PLATFORM_MAC)
	_objc_opengl_swapinterval(rc,enable!=0);
#endif
}

void FinalizeRendering(const RenderContext& rc)
{
	glFinish();

#if		defined(PLATFORM_WIN)
		SwapBuffers(rc.GetDC());
#elif	defined(PLATFORM_MAC) || defined(PLATFORM_IOS)
		_objc_opengl_rendercontext_swapbuffer((HGLRC)rc);
#else
		ASSERT_STATIC(0);
#endif
}
	
void SetViewport(int x, int y, int w, int h)
{
#if defined(PLATFORM_IOS)
	glViewport(x*RenderContext::_ScreenScale, y*RenderContext::_ScreenScale, RenderContext::_ScreenScale*w, RenderContext::_ScreenScale*h);
#else
	glViewport(x, y, w, h);
#endif
}
	
void SetLineWidth(float w)
{
#if defined(PLATFORM_IOS)
	glLineWidth(w*RenderContext::_ScreenScale);
#else
	glLineWidth(w);
#endif
}

#ifdef PLATFORM_DEBUG_BUILD
namespace _details
{
	os::AtomicLock	_GLErrorAssetion;
}
#endif

LPCSTR GetLastError()
{
#ifdef PLATFORM_DEBUG_BUILD
	if(!_details::_GLErrorAssetion.TryLock())return NULL;
#endif

	LPCSTR ret = NULL;

	if(RenderContext::GetCurrent()){} // no RC in current thread
	else
	{	ret = "GL_NO_CONTEXT";
		goto END_OF_FUNC_GLE;
	}

    {
	int ec = glGetError();
	if(ec == GL_NO_ERROR)goto END_OF_FUNC_GLE;

    {
	bool	InvalidOp = false;
	bool	GetNextError = true;
wglCheckError_NextError:
	{
		switch(ec)
		{
		case GL_INVALID_ENUM:
			ret =  ("GL_INVALID_ENUM");
			break;
		case GL_INVALID_VALUE:
			ret =  ("GL_INVALID_VALUE");
			break;
		case GL_INVALID_OPERATION:
			if(InvalidOp)
				GetNextError = false;
			else
			{
				InvalidOp = true;
				ret =  ("GL_INVALID_OPERATION");
			}
			break;
#if defined(PLATFORM_OPENGL_SUPPORT)
		case GL_STACK_OVERFLOW:
			ret =  ("GL_STACK_OVERFLOW");
			break;
		case GL_STACK_UNDERFLOW:
			ret =  ("GL_STACK_UNDERFLOW");
			break;
#endif
		case GL_OUT_OF_MEMORY:
			ret =  ("GL_OUT_OF_MEMORY");
			break;
		}
	}

	if(GetNextError&&((ec = glGetError())!=GL_NO_ERROR))
		goto wglCheckError_NextError;
    }
    }
    
END_OF_FUNC_GLE:
#ifdef PLATFORM_DEBUG_BUILD
	_details::_GLErrorAssetion.Unlock();
#endif	
	return ret;
}


void DrawUnitBox(ShaderProgramBase* pShader)
{
	_LogGLError;
	int posloc = pShader->GetAttribLocation("_Position");
	int clrloc = pShader->GetAttribLocation("_Color");
	
	rt::Vec4f clr_pad;
	glGetFloatv(GL_COLOR_CLEAR_VALUE,(GLfloat*)&clr_pad);

#define CLR_VARY 0.5f

	if(clr_pad.x>0.5f)clr_pad.x-=CLR_VARY;
	if(clr_pad.y>0.5f)clr_pad.y-=CLR_VARY;
	if(clr_pad.z>0.5f)clr_pad.z-=CLR_VARY;

	glEnableVertexAttribArray(posloc);
	glEnableVertexAttribArray(clrloc);

	{	
#define GLVBEGIN	vi = 0;

#define GLV(x,y,z)	clr[vi][0] = x*CLR_VARY+clr_pad.r;	\
					clr[vi][1] = y*CLR_VARY+clr_pad.g;	\
					clr[vi][2] = z*CLR_VARY+clr_pad.b;	\
					pos[vi][0] = (x-0.01f)*1.02f - 0.5f;\
					pos[vi][1] = (y-0.01f)*1.02f - 0.5f;\
					pos[vi][2] = (z-0.01f)*1.02f - 0.5f;\
					vi++;

#define GLVEND		glVertexAttribPointer(posloc, 3, GL_FLOAT, GL_FALSE, 0, pos[0]);		\
					glVertexAttribPointer(clrloc, 3, GL_FLOAT, GL_FALSE, 0, clr[0]);		\
					glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

		glCullFace(GL_FRONT);
		glEnable(GL_CULL_FACE);
#if defined(PLATFORM_OPENGL_SUPPORT)
		glPolygonMode(GL_BACK,GL_FILL);
#endif

		glDepthMask(false);
		{	float clr[4][3];
			float pos[4][3];
			int   vi;
			////Z
			GLVBEGIN GLV(0,1,0); GLV(1,1,0); GLV(1,0,0); GLV(0,0,0); GLVEND
			GLVBEGIN GLV(0,0,1); GLV(1,0,1); GLV(1,1,1); GLV(0,1,1); GLVEND
			////Y
			GLVBEGIN GLV(0,0,0); GLV(1,0,0); GLV(1,0,1); GLV(0,0,1); GLVEND
			GLVBEGIN GLV(0,1,1); GLV(1,1,1); GLV(1,1,0); GLV(0,1,0); GLVEND
			////X
			GLVBEGIN GLV(0,0,1); GLV(0,1,1); GLV(0,1,0); GLV(0,0,0); GLVEND
			GLVBEGIN GLV(1,0,0); GLV(1,1,0); GLV(1,1,1); GLV(1,0,1); GLVEND
		}
		glDepthMask(true);
				
		SetLineWidth(0.5f);
		{
			static const float pos[8][3] = 
			{	{-0.01f - 0.5f, 1.01f - 0.5f,-0.01f - 0.5f},
				{ 1.01f - 0.5f, 1.01f - 0.5f,-0.01f - 0.5f},
				{ 1.01f - 0.5f,-0.01f - 0.5f,-0.01f - 0.5f},
				{ 1.01f - 0.5f,-0.01f - 0.5f, 1.01f - 0.5f},
				{-0.01f - 0.5f,-0.01f - 0.5f, 1.01f - 0.5f},
				{-0.01f - 0.5f, 1.01f - 0.5f, 1.01f - 0.5f},
				{ 1.01f - 0.5f, 1.01f - 0.5f, 1.01f - 0.5f},
				{ 1.01f - 0.5f, 1.01f - 0.5f,-0.01f - 0.5f}
			};
			static const float clr[8][3] = 
			{	{ 0.0f, 0.0f, 0.0f},
				{ 0.0f, 0.0f, 0.0f},
				{ 0.0f, 0.0f, 0.0f},
				{ 0.0f, 0.0f, 0.0f},
				{ 0.0f, 0.0f, 0.0f},
				{ 0.0f, 0.0f, 0.0f},
				{ 0.0f, 0.0f, 0.0f},
				{ 0.0f, 0.0f, 0.0f}
			};
			glVertexAttribPointer(posloc, 3, GL_FLOAT, GL_FALSE, 0, pos[0]);
			glVertexAttribPointer(clrloc, 3, GL_FLOAT, GL_FALSE, 0, clr[0]);
			glDrawArrays(GL_LINE_STRIP, 0, 8);

			static const float pos_line[4][3] = 
			{	{ -0.01f - 0.5f, 1.01f - 0.5f, 1.01f - 0.5f},
				{ -0.01f - 0.5f, 1.01f - 0.5f,-0.01f - 0.5f},
				{  1.01f - 0.5f, 1.01f - 0.5f, 1.01f - 0.5f},
				{  1.01f - 0.5f,-0.01f - 0.5f, 1.01f - 0.5f}
			};
			glVertexAttribPointer(posloc, 3, GL_FLOAT, GL_FALSE, 0, pos_line[0]);
			glVertexAttribPointer(clrloc, 3, GL_FLOAT, GL_FALSE, 0, clr[0]);
			glDrawArrays(GL_LINES, 0, 4);
		}

		{	SetLineWidth(2);
			static const float pos[6][3] = 
			{	{-0.01f - 0.5f,-0.01f - 0.5f,-0.01f - 0.5f},
				{ 1.11f - 0.5f,-0.01f - 0.5f,-0.01f - 0.5f},
				{-0.01f - 0.5f,-0.01f - 0.5f,-0.01f - 0.5f},
				{-0.01f - 0.5f, 1.11f - 0.5f,-0.01f - 0.5f},
				{-0.01f - 0.5f,-0.01f - 0.5f,-0.01f - 0.5f},
				{-0.01f - 0.5f,-0.01f - 0.5f, 1.11f - 0.5f}
			};
			static const float clr[6][3] = 
			{
				{1,0,0},
				{1,0,0},
				{0,1,0},
				{0,1,0},
				{0,0,1},
				{0,0,1}
			};
			glVertexAttribPointer(posloc, 3, GL_FLOAT, GL_FALSE, 0, pos[0]);
			glVertexAttribPointer(clrloc, 3, GL_FLOAT, GL_FALSE, 0, clr[0]);
			glDrawArrays(GL_LINES, 0, 6);
		}

		SetLineWidth(0.5f);
	}
	glDisableVertexAttribArray(posloc);
	glDisableVertexAttribArray(clrloc);
}

} // namespace gl


gl::Camera::Camera()
{
	_Projection.SetIdentity();
	_ModelView.SetIdentity();
	
	_MVPDirty = true;

	_ViewLeft = 0;
	_ViewTop = 0;
	_ViewRight = 800;
	_ViewBottom = 600;
	_ViewFovy = 60;
	_ViewZnear = 0.1f;
	_ViewZfar = 20;

}

void gl::Camera::SetViewport(int left, int top, int right, int bottom)
{
	ASSERT(right > left);
	ASSERT(bottom > top);

	_ViewLeft = left;
	_ViewTop = top;
	_ViewRight = right;
	_ViewBottom = bottom;

	_UpdatePerspective();
}

void gl::Camera::SetFOV(float fovy)
{
	_ViewFovy = fovy;
	_UpdatePerspective();
}

void gl::Camera::SetDepthRange(float z_near, float z_far)
{
	_ViewZnear = z_near;
	_ViewZfar = z_far;
	_UpdatePerspective();
}

void gl::Camera::Use() const 
{
	gl::SetViewport(_ViewLeft, _ViewTop, _ViewRight-_ViewLeft, _ViewBottom-_ViewTop);
}

void gl::Camera::LoadIdentity()
{
	_ModelView.SetIdentity();
	_MVPDirty = true;
}

void gl::Camera::Translate(float x, float y, float z)
{
	rt::Mat4x4f t;
	t.SetIdentity();
	t.m03 = x;
	t.m13 = y;
	t.m23 = z;
	Multiply(t);
}

void gl::Camera::Scale(float s)
{
	_ModelView.m00 *= s;
	_ModelView.m11 *= s;
	_ModelView.m22 *= s;
	_MVPDirty = true;
}

void gl::Camera::_UpdatePerspective()
{
	double aspect_ratio = (double)(_ViewRight - _ViewLeft)/(double)(_ViewBottom - _ViewTop);
	
	double sine, cotangent, deltaZ;
	double radians = _ViewFovy / 2 * 3.1415926535897932384626433832795 / 180;
	deltaZ = _ViewZfar - _ViewZnear;
	sine = sin(radians);
	ASSERT((deltaZ > 0) && (sine > 0) && (aspect_ratio > 0));
	cotangent = cos(radians) / sine;
		
	_Projection.SetIdentity();
	_Projection.m[0][0] = (float)(cotangent / aspect_ratio);
	_Projection.m[1][1] = (float)cotangent;
	_Projection.m[2][2] = (float)(-(_ViewZfar + _ViewZnear) / deltaZ);
	_Projection.m[2][3] = -1;
	_Projection.m[3][2] = (float)(-2 * _ViewZnear * _ViewZfar / deltaZ);
	_Projection.m[3][3] = 0;

	_MVPDirty = true;
}

void gl::Camera::ApplyTo(ShaderProgramBase& shader, LPCSTR mvp_name, LPCSTR normal_transform_name) const
{
	if(mvp_name)shader.SetUniformMatrix4x4(mvp_name, GetMVP());
	if(normal_transform_name)shader.SetUniformMatrix3x3(normal_transform_name, GetNormalTransform());
}


UINT gl::GetGLTypeSize(DWORD type)
{
	switch(type)
	{
	case GL_BYTE:
	case GL_UNSIGNED_BYTE:
		return 1;
	case GL_SHORT:
	case GL_UNSIGNED_SHORT:
		return 2;
	case GL_INT:
	case GL_UNSIGNED_INT:
	case GL_FLOAT:
		return 4;
	case GL_DOUBLE:
		return 8;
	default: ASSERT(0);
		return 0;
	}
}

const rt::String_Ref& gl::GetGLTypeName(DWORD type)
{
	static const rt::SS _names[] = 
	{
		"char",
		"byte",
		"short",
		"ushort",
		"int",
		"uint",
		"float",
		"double",
		""
	};

	switch(type)
	{
	case GL_BYTE:				return _names[0];
	case GL_UNSIGNED_BYTE:		return _names[1];
	case GL_SHORT:				return _names[2];
	case GL_UNSIGNED_SHORT:		return _names[3];
	case GL_INT:				return _names[4];
	case GL_UNSIGNED_INT:		return _names[5];
	case GL_FLOAT:				return _names[6];
	case GL_DOUBLE:				return _names[7];
	default: ASSERT(0);
		return _names[8];
	}
}


/*
#include "../DirectXMath/DirectXMath.h"
using namespace DirectX;
void gl::FlyCamera::OnUserInputEvent(const os::UserInputEvent& e)
{
	XMVECTOR vUp = XMVectorSet( 0, 1, 0, 0 );
	
	if (e.Type == os::UIEVT_MOUSE_DRAGBEGIN && e.ButtonCode == os::BUTTON_MOUSE_LEFT)
	{
		_fCameraPitchAngleWhenMouseDown = _fCameraPitchAngle;
		_fCameraYawAngleWhenMouseDown = _fCameraYawAngle;
		_mouseXDown = e.Position.x;
		_mouseYDown = e.Position.y;
	} else
	if ( e.Type == os::UIEVT_MOUSE_DRAG && e.ButtonCode == os::BUTTON_MOUSE_LEFT )
	{
		_fCameraPitchAngle = _fCameraPitchAngleWhenMouseDown + (e.Position.x - _mouseXDown) / 1000.0f;
		_fCameraYawAngle = _fCameraYawAngleWhenMouseDown + (e.Position.y - _mouseYDown) / 1000.0f;
		
		_fCameraYawAngle = __max( -XM_PI / 2.0f, _fCameraYawAngle );
		_fCameraYawAngle = __min( +XM_PI / 2.0f, _fCameraYawAngle );

		XMMATRIX matRot = XMMatrixRotationRollPitchYaw( _fCameraYawAngle, -_fCameraPitchAngle, 0 );
		XMVECTOR vWorldAhead = XMVectorSet( 0, 0, 1, 0 );
		vWorldAhead = XMVector3TransformCoord( vWorldAhead, matRot );
		
		XMVECTOR vEyePt = XMLoadFloat3( (XMFLOAT3*)&_vEyePt );
		XMVECTOR vLookAt = XMVectorAdd( vEyePt, vWorldAhead );
		XMStoreFloat3( (XMFLOAT3*)&_vLookAtPt, vLookAt );

		XMMATRIX matView = XMMatrixLookAtRH( vEyePt, vLookAt, vUp );
		XMStoreFloat4x4( (XMFLOAT4X4*)&_ModelView, matView );	

		_MVPDirty = true;
	} else
	if ( e.Type == os::UIEVT_KEY_DOWN )
	{
		switch (e.KeyCode)
		{
			case 'W': _bUpDown = true; break;
			case 'S': _bDownDown = true; break;
			case 'A': _bLeftDown = true; break;
			case 'D': _bRightDown = true; break;
		}		
	} else
	if ( e.Type == os::UIEVT_KEY_UP )
	{
		switch (e.KeyCode)
		{
			case 'W': _bUpDown = false; break;
			case 'S': _bDownDown = false; break;
			case 'A': _bLeftDown = false; break;
			case 'D': _bRightDown = false; break;
		}
	}
}

void gl::FlyCamera::SetViewParams( const rt::Vec3f &eyePt, const rt::Vec3f &lookAtPt )
{
	_bLeftDown = false;
	_bRightDown = false;
	_bUpDown = false;
	_bDownDown = false;
	
	_vEyePt = eyePt;
	_vLookAtPt = lookAtPt;

	XMVECTOR xmEyePt = XMLoadFloat3( (XMFLOAT3*)&eyePt );
	XMVECTOR xmLookAtPt = XMLoadFloat3( (XMFLOAT3*)&lookAtPt );

	XMVECTOR xmUp = XMVectorSet( 0, 1, 0, 0 );
	XMMATRIX matView = XMMatrixLookAtLH( xmEyePt, xmLookAtPt, xmUp );
	XMMATRIX matInvView = XMMatrixInverse( NULL, matView );

	XMVECTOR xmZBasis = matInvView.r[2];
	_fCameraYawAngle = atan2f( XMVectorGetX(xmZBasis), XMVectorGetZ(xmZBasis) );
	float fLen = sqrtf( XMVectorGetZ(xmZBasis) * XMVectorGetZ(xmZBasis) + XMVectorGetX(xmZBasis) * XMVectorGetX(xmZBasis) );
	_fCameraPitchAngle = -atan2f( XMVectorGetY(xmZBasis), fLen );	
	
	matView = XMMatrixLookAtRH( xmEyePt, xmLookAtPt, xmUp );
	XMStoreFloat4x4( (XMFLOAT4X4*)&_ModelView, matView );
	_MVPDirty = true;
	GetMVP();
}

void gl::FlyCamera::OnRendering( ULONGLONG timeStamp )
{
	if ( _bLeftDown || _bRightDown || _bUpDown || _bDownDown )
	{
		XMVECTOR vUp = XMVectorSet( 0, 1, 0, 0 );	
		XMVECTOR vEyePt = XMLoadFloat3( (XMFLOAT3*)&_vEyePt );
		XMVECTOR vLookAtPt = XMLoadFloat3( (XMFLOAT3*)&_vLookAtPt );
		XMVECTOR vDir = XMVectorSubtract( vLookAtPt, vEyePt );
		vDir = XMVector3Normalize( vDir );
		XMVECTOR vSide = XMVector3Cross( vDir, vUp );
		vSide = XMVector3Normalize( vSide );

		float weight = 1E-10f * (timeStamp - _timeStampLast);
		if ( _bLeftDown )
		{
			vEyePt = vEyePt - vSide * weight;
			vLookAtPt = vLookAtPt - vSide * weight;
		}
		if ( _bRightDown )
		{
			vEyePt = vEyePt + vSide * weight;
			vLookAtPt = vLookAtPt + vSide * weight;
		}
		if ( _bUpDown )
		{
			vEyePt = vEyePt + vDir * weight;
			vLookAtPt = vLookAtPt + vDir * weight;
		}
		if ( _bDownDown )
		{
			vEyePt = vEyePt - vDir * weight;
			vLookAtPt = vLookAtPt - vDir * weight;
		}

		XMStoreFloat3( (XMFLOAT3*)&_vEyePt, vEyePt );
		XMStoreFloat3( (XMFLOAT3*)&_vLookAtPt, vLookAtPt );

		XMMATRIX matView = XMMatrixLookAtRH( vEyePt, vLookAtPt, vUp );
		XMStoreFloat4x4( (XMFLOAT4X4*)&_ModelView, matView );
		_MVPDirty = true;
		GetMVP();
	}

	_timeStampLast = timeStamp;
}
*/

bool gl::_details::VertexBufferObjectBase::Create(DWORD StorageHint)
{
	if(!VertexBufferName)
	{	glGenBuffers(1,&VertexBufferName);
		_LogGLError;
	}

	ASSERT( StorageHint != StorageHint_NoChange );
	LastStorageHint = StorageHint;

	return VertexBufferName;
}

void gl::_details::VertexBufferObjectBase::Destroy()
{
	if(VertexBufferName)
	{
		glDeleteBuffers(1,&VertexBufferName); 
		_LogGLError;
	}

	VertexBufferName = NULL;
	BufferSize = 0;
}


/*



static LPCTSTR TextTail = _T(" Information\n");
static const LPCTSTR TextBar= _T(" Information\n==========================\n");

void _DumpOGLExtensionInfo()
{
	_CheckDump(_T("OpenGL Extensions")<<TextBar);

	LPCSTR pExtString = (LPCSTR)glGetString(GL_EXTENSIONS);
	if(!pExtString)
	{
		_CheckDump(_T("Failed to get OpenGL extension string\n"));
		return;
	}

	///Dump Specific extensions
	if(strstr(pExtString,"GL_ARB_multitexture"))
	{
		_CheckDump(_T("GL_ARB_multitexture")<<TextTail);
		_CheckDump(_T("\tTexture Stage Count: ")<<mt::MaxTextureStage()<<_T("\n\n"));
	}

	if(strstr(pExtString,"GL_EXT_texture_filter_anisotropic"))
	{
		_CheckDump(_T("GL_EXT_texture_filter_anisotropic")<<TextTail);
		{	GLfloat max = -1.0f;
			glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT,&max);
			if(max<0)
			{
				_CheckDump(_T("\tFailed to query MAX_TEXTURE_MAX_ANISOTROPY_EXT"));
			}
			else
			{
				_CheckDump(_T("\tMax Texture Max Anisotropy: ")<<max);
			}
		}
		_CheckDump(_T("\n\n"));
	}

	if(strstr(pExtString,"GL_EXT_texture3D"))
	{
		_CheckDump(_T("GL_EXT_texture3D")<<TextTail);
		{
			GLint sz = 0;
			
			glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE_EXT,&sz);
			if(sz)
			{	_CheckDump(_T("\tMax 3D Texture Size: ")<<sz); }
			else
			{	_CheckDump(_T("\tFailed to query MAX_3D_TEXTURE_SIZE_EXT")); }
		}
		_CheckDump(_T("\n\n"));
	}

	if(strstr(pExtString,"GL_ARB_texture_cube_map"))
	{
		_CheckDump(_T("GL_ARB_texture_cube_map")<<TextTail);
		{
			_CheckDump(_T("\tMax cubemap face size: ")<<ext::CwglCubemapBase::GetMaxFaceSize());
		}
		_CheckDump(_T("\n\n"));
	}

	if(strstr(pExtString,"GL_ATI_draw_buffers"))
	{
		_CheckDump(_T("GL_ATI_draw_buffers")<<TextTail);
		{
			_CheckDump(_T("\tDraw buffer count: ")<<ex::GetDrawBufferCount());
		}
		_CheckDump(_T("\n\n"));
	}

#ifdef INCLUDE_WGLH_SHADER
	if(strstr(pExtString,"GL_ARB_vertex_program"))
	{
		_CheckDump(_T("GL_ARB_vertex_program")<<TextTail);
		if(ext::ARBvpfp::CwglGpuProgram::LoadExtensionEntryPoints())
		{
			_CheckDump(_T("\tMax Environment Parameters: ")<<ext::ARBvpfp::CwglGpuProgram::MaxEnvParam(GL_VERTEX_PROGRAM_ARB));
			_CheckDump(_T("\n\tMax Local Parameters      : ")<<ext::ARBvpfp::CwglGpuProgram::MaxLocalParam(GL_VERTEX_PROGRAM_ARB));
			_CheckDump(_T("\n\tMax Parameter Bindings    : ")<<ext::ARBvpfp::CwglGpuProgram::MaxParamBinding(GL_VERTEX_PROGRAM_ARB));
			_CheckDump(_T("\n\tMax Attribute Bindings    : ")<<ext::ARBvpfp::CwglGpuProgram::MaxAttribBinding(GL_VERTEX_PROGRAM_ARB));
			_CheckDump(_T("\n\tMax Temp Variable         : ")<<ext::ARBvpfp::CwglGpuProgram::MaxTempVariable(GL_VERTEX_PROGRAM_ARB));
			_CheckDump(_T("\n\tMax Instructions          : ")<<ext::ARBvpfp::CwglGpuProgram::MaxInstructions(GL_VERTEX_PROGRAM_ARB));
			_CheckDump(_T("\n\n"));
		}
		else
		{
			_CheckDump(_T("Failed in loading extension entrypoints\n\n"));
		}
	}

	if(strstr(pExtString,"GL_ARB_fragment_program"))
	{
		_CheckDump(_T("GL_ARB_fragment_program")<<TextTail);
		if(ext::ARBvpfp::CwglGpuProgram::LoadExtensionEntryPoints())
		{
			_CheckDump(_T("\tMax Environment Parameters: ")<<ext::ARBvpfp::CwglGpuProgram::MaxEnvParam(GL_FRAGMENT_PROGRAM_ARB));
			_CheckDump(_T("\n\tMax Local Parameters      : ")<<ext::ARBvpfp::CwglGpuProgram::MaxLocalParam(GL_FRAGMENT_PROGRAM_ARB));
			_CheckDump(_T("\n\tMax Parameter Bindings    : ")<<ext::ARBvpfp::CwglGpuProgram::MaxParamBinding(GL_FRAGMENT_PROGRAM_ARB));
			_CheckDump(_T("\n\tMax Attribute Bindings    : ")<<ext::ARBvpfp::CwglGpuProgram::MaxAttribBinding(GL_FRAGMENT_PROGRAM_ARB));
			_CheckDump(_T("\n\tMax Temp Variable         : ")<<ext::ARBvpfp::CwglGpuProgram::MaxTempVariable(GL_FRAGMENT_PROGRAM_ARB));
			_CheckDump(_T("\n\tMax Instructions          : ")<<ext::ARBvpfp::CwglGpuProgram::MaxInstructions(GL_FRAGMENT_PROGRAM_ARB));
			GLint	TexUnit=0,TexCoord=0;
			glGetIntegerv(GL_MAX_TEXTURE_COORDS_ARB,&TexCoord);
			glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS_ARB,&TexUnit);
			_CheckDump(_T("\n\tMax Texture Coord         : ")<<TexCoord);
			_CheckDump(_T("\n\tMax Texture Unit          : ")<<TexUnit);

			_CheckDump(_T("\n\tNative Limitations:"));
			_CheckDump(_T("\n\t\tTotal Instructions: ")<<ext::ARBvpfp::CwglGpuProgram::MaxNativeInstructions(GL_FRAGMENT_PROGRAM_ARB));
			_CheckDump(_T("\n\t\tALU Instructions  : ")<<ext::ARBvpfp::CwglGpuProgram::MaxNativeAluInstructions(GL_FRAGMENT_PROGRAM_ARB));
			_CheckDump(_T("\n\t\tTEX Instructions  : ")<<ext::ARBvpfp::CwglGpuProgram::MaxNativeTexInstructions(GL_FRAGMENT_PROGRAM_ARB));
			_CheckDump(_T("\n\t\tTEX Indirections  : ")<<ext::ARBvpfp::CwglGpuProgram::MaxNativeTexIndirections(GL_FRAGMENT_PROGRAM_ARB));
			_CheckDump(_T("\n\t\tATTRIB Bindings   : ")<<ext::ARBvpfp::CwglGpuProgram::MaxNativeAttribBinding(GL_FRAGMENT_PROGRAM_ARB));
			_CheckDump(_T("\n\t\tPARAM Bindings    : ")<<ext::ARBvpfp::CwglGpuProgram::MaxNativeParamBinding(GL_FRAGMENT_PROGRAM_ARB));
			_CheckDump(_T("\n\t\tTEMP Bindings     : ")<<ext::ARBvpfp::CwglGpuProgram::MaxNativeTempVariable(GL_FRAGMENT_PROGRAM_ARB));
			_CheckDump(_T("\n\n"));
		}
		else
		{
			_CheckDump(_T("Failed in loading extension entrypoints\n\n"));
		}
	}

	if(strstr(pExtString,"GL_ARB_shading_language"))
	{
		_CheckDump(_T("OpenGL Shading Language")<<TextTail);
		if(glsl::LanguageObject::LoadExtensionEntryPoints())
		{
			_CheckDump(_T("\tVersion = ")<<glsl::LanguageObject::GetVersion());
			_CheckDump(_T("\n\tHardware Limitation:"));
			_CheckDump(_T("\n\t\tVarying floats        :")<<glsl::LanguageObject::Fragment_MaxVarying());
			_CheckDump(_T("\n\t\tTexture unit          :")<<glsl::LanguageObject::Vertex_MaxTextureUnitCombined());
			_CheckDump(_T("\n\t\tTexture coord         :")<<glsl::LanguageObject::Vertex_MaxTextureCoord());

			_CheckDump(_T("\n\t\tVertex attribute      :")<<glsl::LanguageObject::Vertex_MaxAttrib());
			_CheckDump(_T("\n\t\tVertex texture unit   :")<<glsl::LanguageObject::Vertex_MaxTextureUnit());
			_CheckDump(_T("\n\t\tVertex uniform        :")<<glsl::LanguageObject::Vertex_MaxUniform());

			_CheckDump(_T("\n\t\tFragment texture unit :")<<glsl::LanguageObject::Fragment_MaxTextureUnit());
			_CheckDump(_T("\n\t\tFragment uniform      :")<<glsl::LanguageObject::Fragment_MaxUniform());

			_CheckDump(_T("\n\n"));
		}
	}
#endif // INCLUDE_WGLH_SHADER

	_CheckDump(_T('\n'));
}

void _DumpOGLInfo(HDC dc = NULL)
{
	_CheckDump(_T("OpenGL")<<TextBar);

	{
		LPCSTR pStr;
		_CheckDump(_T("Vendor    : "));
		pStr = (LPCSTR)glGetString(GL_VENDOR);
		if(pStr)_CheckDump(pStr);

		_CheckDump(_T("\nRender    : "));
		pStr = (LPCSTR)glGetString(GL_RENDERER);
		if(pStr)_CheckDump(pStr);

		_CheckDump(_T("\nVersion   : "));
		pStr = (LPCSTR)glGetString(GL_VERSION);
		if(pStr)_CheckDump(pStr);

		_CheckDump(_T("\nExtensions: "));
		pStr = (LPCSTR)glGetString(GL_EXTENSIONS);
		if(pStr)
		{
			char enbuf[MAX_PATH];
			enbuf[0] = ' ';
			enbuf[1] = '\t';
			char * pen = &enbuf[2];

			LPCSTR head = pStr;
			LPCSTR ps=NULL;
			int	len;

			while(ps=strchr(head,' '))
			{
				len = (int)(ps-head);
				ASSERT(len<(MAX_PATH-6));

				memcpy(pen,head,len);

				pen[len] = '\n';
				pen[len+1] = '\0';
				_CheckDump(enbuf);

				head = ps+1;
				enbuf[0] = '\t';
			}

			*pen = '\0';
			_CheckDump(enbuf<<head);
		}      

#ifdef CodeLib_wglh_ext_Included
		if( wglh::ext::CwglExtensions::LoadExtensionEntryPoints() )
		{
			_CheckDump("\nWGL Extensions");
			char enbuf[MAX_PATH];
			enbuf[0] = ':';
			enbuf[1] = ' ';
			char * pen = &enbuf[2];

			LPCSTR head = wglh::ext::CwglExtensions::GetExtensionStringWGL();
			ASSERT(head);

			LPCSTR ps=NULL;
			int	len;

			while(ps=strchr(head,' '))
			{
				len = (int)(ps-head);
				ASSERT(len<(MAX_PATH-6));

				memcpy(pen,head,len);

				pen[len] = '\n';
				pen[len+1] = '\0';
				_CheckDump(enbuf);

				head = ps+1;
				enbuf[0] = '\t';
				enbuf[1] = '\t';
			}

			*pen = '\0';
			_CheckDump(enbuf<<head);
		}
#endif
	}

	_CheckDump("\n\n");
}


void DumpOpenGLInformation(HWND	GL_Wnd)
{
	if(GL_Wnd)
	{	HDC hdc = ::GetDC(GL_Wnd);
		_DumpOGLInfo(hdc);
		::ReleaseDC(GL_Wnd,hdc);
		_DumpOGLExtensionInfo();
	}
	else
	{
		wglh::CwglWnd	TestWnd;

		if(TestWnd.Create())
		{
			if(TestWnd.InitGL())
			{
				HDC dc = ::GetDC(TestWnd);
				_DumpOGLInfo(dc);
				::ReleaseDC(GL_Wnd,dc);
				_DumpOGLExtensionInfo();
				return;
			}
			else
			{
				_CheckDumpSrcLoc;
			}
		}
		else
		{
			_CheckDumpSrcLoc;
		}

		_CheckDump("Failed to create OpenGL test window.\n");
	}
}



};
*/