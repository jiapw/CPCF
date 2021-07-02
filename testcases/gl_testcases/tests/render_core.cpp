#include "render_core.h"

#if !defined(PLATFORM_WIN)
#include "CoreFoundation/CoreFoundation.h"
#include <string>
#endif

#define GNAME "R8"
//#define GNAME "USB"


RenderCore::RenderCore()
	:_ShaderProgram("/app/_default.vsh", "/app/_default.fsh")
	,_progTextured("/app/textured.vsh", "/app/textured.fsh")
{
	_GlobalArcball_StateVersion = 0;

}


#if	defined(PLATFORM_WIN)
bool RenderCore::Init(HWND mainwnd)
{
	if(!_RC.Create(mainwnd))
	{	_LOG("RenderContext creation failed.");
		return false;
	}

	return (PostInit()) &&
	_Pump.Init(mainwnd, os::UIDEV_KEYBOARD|os::UIDEV_MOUSE|os::UIDEV_VIEWPORT|os::UIDEV_EVENTTYPE_MOUSEMOVE);

	return true;
}
#elif defined(PLATFORM_MAC) || defined(PLATFORM_IOS)
bool RenderCore::Init(LPVOID pNSOpenGLView)
{
	if(!_RC.Create(pNSOpenGLView))
	{	_LOG("RenderContext creation failed.");
		return false;
	}

	return (PostInit()) &&
	_Pump.Init(pNSOpenGLView,os::UIDEV_KEYBOARD|os::UIDEV_MOUSE|os::UIDEV_VIEWPORT|os::UIDEV_EVENTTYPE_MOUSEMOVE);
}
#else
	ASSERT_STATIC(0);
#endif

bool RenderCore::PostInit()
{
	VERIFY(_RC.Apply());
	_RC.SetBackgroundColor(rt::Vec3b(10,10,10));
	_RC.EnableVSync(false);

	// get assets
	{
//#if !defined(PLATFORM_WIN)
//		os::SetAppSandboxAsCurrentDirectory("opengl_test");
//#endif
		_LOG("loading resource ...");

		//{	WebResource texw("http://jiapingwang.com/texture.png");
		//	if(texw.GetLength())
		//	{
		//		ipp::ImageDecoder	dec;
		//		if(dec.Decode(texw.GetData(), texw.GetLength()))
		//		{
		//			_Texture.Use();
		//			//_Texture.DefineTexture(dec.GetImageWidth(), dec.GetImageHeight(), dec.GetOutput());
		//			_Texture.DefineTextureMipmapped(gl::Texture2D::TEXFMT_1c8u + dec.GetImageChannel() - 1, dec.GetImageWidth(), dec.GetImageHeight(), dec.GetOutput());
		//			_LogGLError;
		//			_Texture.TextureFilterSet(gl::TextureBase::FilterTrilinearMipmap, gl::TextureBase::FilterLinear, 8);
		//			_LogGLError;
		//		}
		//	}
		//}

		//{	WebResource texw("http://jiapingwang.com/logo.png");
		//	if(texw.GetLength())_imgLogo.Load(texw.GetData(), texw.GetLength());
		//}
		//
		//{	WebResource texw("http://jiapingwang.com/ECJ_14_2048.ufg");
		//	_Font.Open(texw.GetData(), texw.GetLength());
		//}

		//{	WebResource texw("http://jiapingwang.com/R8.xscene");
		//	_R8.Load("R8.xscene");
		//}

		//gl::Object::ConvertObjMtlPair(GNAME ".obj", GNAME ".xscene");
		//_R8.Load(GNAME ".xscene");

		//{
		//	struct _Pt
		//	{
		//		rt::Vec3f	pos;
		//		rt::Vec3f	reserved[2];
		//		rt::Vec3f	color;
		//	};

		//	os::File file;
		//	file.Open("test.bin", os::File::Normal_Read);
		//	rt::Vec3i header;
		//	file.Read(header, sizeof(header));
		//	_LOG("header = "<<header);

		//	rt::Buffer<_Pt>	d;
		//	d.SetSize(header.z);
		//	file.Read(d, header.z*sizeof(_Pt));

		//	rt::Buffer<rt::Vec3f>	vbo;
		//	vbo.SetSize(2*header.z);
		//	_PointCount = header.z;

		//	for(int i=0;i<header.z;i++)
		//	{	
		//		vbo[i] = d[i].pos;
		//		vbo[i] *= 0.05f;
		//		vbo[i+header.z] = d[i].color;
		//	}

		//	_PointCloud.Create();
		//	_PointCloud.Use();
		//	_PointCloud.SetSize(sizeof(float)*2*3*_PointCount, vbo);
		//}

		gl::StopUsingVBO();

		//_Font.Open("cjk_16.ufg", gl::gdiFont::FLAG_KEEP_IN_MEMORY);
#if !defined(PLATFORM_WIN)
        CFBundleRef mainBundle = CFBundleGetMainBundle();
            CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
            char path[PATH_MAX];
            if (!CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (UInt8 *)path, PATH_MAX))
            {
                // error!
            }
            CFRelease(resourcesURL);
            chdir(path);
        std::string t(path);
        std::string fn=t+"/cjk_14.ufg";
        _Font.Open(fn.c_str(),gl::gdiFont::FLAG_KEEP_IN_MEMORY);
#elif defined(PLATFORM_WIN)
        _Font.Open("cjk_14.ufg", gl::gdiFont::FLAG_KEEP_IN_MEMORY);
#endif
		
		_Font.SetFontSpacing(-4);
		//{	ipp::Image_1c8u		copy;
		//	ipp::ImageRef_1c8u	test(_Font.GetGlyphMap(), _Font.GetGlyphMapWidth(), _Font.GetGlyphMapHeight(), _Font.GetStep());
		//	_Font.CopyGlyphs(os::__UTF16(os::__UTF8(L"Testing²âÊÔÒ»ÏÂ")),copy);
		//	test.Save("glyph_map.png", ipp::ImageCodec_PNG);
		//	copy.Save("glyph_test.png", ipp::ImageCodec_PNG);

		//	//_Texture.Use();
		//	//_Texture.DefineTexture(_Font.GetGlyphMapWidth(), _Font.GetGlyphMapHeight(), (LPCBYTE)_Font.GetGlyphMap());
		//}
		//{	
		//	_Font.CopyGlyphs(os::__UTF16("CFP/OpenGL Test"), _imgCaption, -5, false);
		//	if(!_imgCaption.IsEmpty())
		//	{	_texCaption.DefineTexture(_imgCaption.GetWidth(), _imgCaption.GetHeight(), _imgCaption.GetImageData(), _imgCaption.GetStep());
		//		_LogGLError;
		//	}
		//}
	}

	//_LOG("GL Externsion: "<<_RC.GetExtensionString());
	rt::String glstat;
	_RC.ReportDeviceDetails(glstat);
	_LOG(glstat);

	// GDI
	VERIFY(_DC.Create());

	{	// shaders
		(*gl::ShaderSourceCodeLibrary::Get())
		.AddSourceCodes(_ShaderProgram, 
			"SHADER_PRECISION(mediump,float)\n"
			"attribute vec4 _Position;"
			"attribute vec4 _Color;"
			"uniform mat4 _MVP;"
			"varying vec4 fragColor;"
			"void main()"
			"{"
			"	gl_Position = _MVP*_Position;"
			"	fragColor = _Color;"
			"}"
			,
			"SHADER_PRECISION(mediump,float)\n"
			"varying vec4 fragColor;"
			"void main()"
			"{"
			"	gl_FragColor = fragColor;"
			"}"
		)
		.AddSourceCodes(_progTextured, 
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
			//"{	gl_FragColor = textureLod(_Texture, texCoord, 0);"
			//"{	gl_FragColor = vec4(texCoord.x, texCoord.y, 0,0);"
			"}"
		)
		.RebuildAll();

	}
	
	
	_Pump.AddSubscriber(this, os::UIDEV_KEYBOARD|os::UIDEV_MOUSE|os::UIDEV_VIEWPORT);

	//_Cam.SetViewParams( rt::Vec3f(0,0,-1.8f), rt::Vec3f(0,0,0) );
	
	return true;
}

void RenderCore::OnUserInputEvent(const os::UserInputEvent& x)
{
	//x.Log();
	
	if(x.Type == os::UIEVT_VIEWPORT_REDRAW || x.Type == os::UIEVT_VIEWPORT_IDLE)
	{	_RC.Apply();
		OnRendering();
	}
	else if(x.Type == os::UIEVT_VIEWPORT_RESIZE)
	{
		_Cam.SetViewport(0,0,x.Size.width, x.Size.height);
#if defined(PLATFORM_OPENGL_SUPPORT)
		_Cam.SetFOV(45);
#else
		_Cam.SetFOV(90);
#endif
		_DC.SetCanvasPlacement(0,0,x.Size.width, x.Size.height);
		//x.Log();

		UpdateCamera();
	}
#if defined(PLATFORM_WIN)
	else if(x.Type == os::UIEVT_KEY_UP)
	{	
		if(x.KeyCode == ' ')
		{
			_R8.ReloadPhongMaterial(GNAME ".mtl", GNAME ".mtlmap");
			OnRendering();
		}
	}
#endif

	if(x.Device & _GlobalArcball.InterestedUserInputDevices())
	{
		_GlobalArcball.OnUserInputEvent(x);
		if(_GlobalArcball.IsStateUpdated(_GlobalArcball_StateVersion))
			UpdateCamera();
	}

	
}

void RenderCore::UpdateCamera()
{
	_Cam.LoadIdentity();
	#if defined(PLATFORM_OPENGL_SUPPORT)
	_Cam.Translate(0,0,-1.8f);
	#else
	_Cam.Translate(0,0,-1.4f);
	#endif
	_Cam.Multiply(_GlobalArcball.GetTransformation());

	OnRendering();
}

void RenderCore::OnRendering()
{

	
	//gl::StartRendering(gl::RENDER_ZTEST_ON);
	gl::StartRendering(0);
	_LogGLError;
	
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	const os::UserInputEvent* pEvt = _Pump.GetCurrentEvent();
	
	_Cam.Use();
	//_Cam.OnRendering( pEvt->Timestamp );
	_ShaderProgram.Use();
	_ShaderProgram.SetUniformMatrix4x4("_MVP", _Cam.GetMVP());
	_LogGLError;

	//_PointCloud.Use();
	//_ShaderProgram.SetVertexAttributePointer("_Position", 0, 3, GL_FLOAT);
	//_ShaderProgram.SetVertexAttributePointer("_Color", (LPVOID)(sizeof(float)*3*_PointCount), 3, GL_FLOAT);

	//glDrawArrays(GL_POINTS, 0, _PointCount);
	//_LogGLError;

	//_PointCloud.UseNull();


	//float vertex[] = 
	//{	-0.9f,	0.9f,	0,
	//	-0.9f,	-0.9f,	0,
	//	0.9f,	-0.9f,	0,
	//	0.9f,	0.9f,	0
	//};

	//float color[] = 
	//{	0.0f,	0.9f,	0,
	//	0.0f,	0.0f,	0,
	//	0.9f,	0.0f,	0,
	//	0.9f,	0.9f,	0
	//};

	//int indics[] = { 0,1,2,3 };
	//
	//int posloc = _ShaderProgram->GetAttribLocation("_Position");
	//int clrloc = _ShaderProgram->GetAttribLocation("_Color");
	//
	//glEnableVertexAttribArray(posloc);
	//glEnableVertexAttribArray(clrloc);

	//glVertexAttribPointer(posloc, 3, GL_FLOAT, GL_FALSE, 0, vertex);
	//glVertexAttribPointer(clrloc, 3, GL_FLOAT, GL_FALSE, 0, color);

	//glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, indics);

	gl::DrawUnitBox(&_ShaderProgram);

	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);
	//_R8.Render(_Cam);
	//gl::StopUsingVBO();

	/*
	_progTextured.Use();
	_progTextured.SetCamera(_Cam, "_MVP");
	_progTextured.SetTexture("_Texture", _Texture);

	
	{	float vertex[] = 
		{	
			0.45f,	0.45f,	-0.1f,
			0.45f,	-0.45f,	0.2f,
			-0.45f,	-0.45f,	0.2f,
			-0.45f,	0.45f,	-0.1f
		};
		float texcoord[] = 
		{	
			1,	0,	
			1,	1,	
			0,	1,	
			0,	0
		};

		_progTextured.SetVertexAttributePointer("_Position", vertex, 3, GL_FLOAT);
		_progTextured.SetVertexAttributePointer("_TexCoord", texcoord, 2, GL_FLOAT);
		_LogGLError;
	
		//glDepthMask(false);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		_LogGLError;
	}
	*/

	glDisable(GL_CULL_FACE);

	_DC.Use();
	_DC.SetFont(_Font);
	_DC.SetDrawingParam(gl::PNAME_TEXT_ALIGNMENT, gl::TEXT_ALIGNMENT_CENTER);
	_DC.SetDrawingParam(gl::PNAME_TEXT_VALIGNMENT, gl::TEXT_ALIGNMENT_CENTER);

	_DC.FillRect(10, 10, 70, 16, rt::Vec4b(80,80,80,160));
	rt::tos::StringOnStack<> s(rt::String_Ref() + _FPS.GetFramerate() + rt::String_Ref(" Hz",3));
	_DC.DrawText(s, 45, 18, rt::Vec4b(255));

	_LogGLError; 
	_DC.FillRect(0, _DC.GetCanvasHeight() - 70, _DC.GetCanvasWidth(), 70, rt::Vec4b(150,150,150,160), rt::Vec4b(150,150,150,20));
	_DC.DrawLine(0, _DC.GetCanvasHeight() - 72, _DC.GetCanvasWidth(), _DC.GetCanvasHeight() - 72, rt::Vec4b(50,50,50,200), 4);
	//_DC.DrawImage(_imgLogo, 30,_DC.GetCanvasHeight() - 100, 96, 96);

	/*
	_DC.FillRect(0,_DC.GetCanvasHeight()/2-15, _DC.GetCanvasWidth()/2, 30, rt::Vec4b(0,0,0,0), rt::Vec4b(0,0,0,255));
	_DC.FillRect(_DC.GetCanvasWidth()/2,_DC.GetCanvasHeight()/2-15, _DC.GetCanvasWidth()/2, 30, rt::Vec4b(0,0,0,255), rt::Vec4b(0,0,0,0));
	_DC.DrawRect(0,_DC.GetCanvasHeight()/2-15, _DC.GetCanvasWidth()+1, 30, rt::Vec4b(120,120,120,240), 1);
	*/
	
	LPCSTR string = "\x48\x65\x6c\x6c\x6f\x20\x57\x6f\x72\x6c\x64\x20\xe4\xbd\xa0\xe5\xa5\xbd\xe4\xb8"
					"\x96\xe7\x95\x8c\x20\xe3\x81\x93\xe3\x82\x93\xe3\x81\xab\xe3\x81\xa1\xe3\x81\xaf"
					"\xe4\xb8\x96\xe7\x95\x8c\x20\x4f\x6c\xc3\xa1\x20\x6d\x75\x6e\x64\x6f";
	_DC.DrawText(string, _DC.GetCanvasWidth()/2,_DC.GetCanvasHeight() - 40, rt::Vec4b(255));


	gl::ShaderProgram::DisableShaderProgram();
	gl::FinalizeRendering(_RC);

	if(pEvt)
		_FPS.Rendered((UINT)(pEvt->Timestamp/1000000));
}

