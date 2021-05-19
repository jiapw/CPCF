#include "gl_shader.h"
#include "../os/file_dir.h"

namespace gl
{

namespace _details
{
#if defined(PLATFORM_OPENGL_SUPPORT)
	#define GLSL_MACRO_GLAPI	"#define SHADER_API_OPENGL\n" \
								"#define mediump\n" \
								"#define highp\n"	\
								"#define lowp\n"	\
								"#define SHADER_PRECISION(p,t)\n"	\

#elif defined(PLATFORM_OPENGL_ES_SUPPORT)
	#define GLSL_MACRO_GLAPI	"#define SHADER_API_OPENGL_ES\n"	\
								"#define SHADER_PRECISION(p,t)	precision p t;\n"	\

#endif

#if defined(PLATFORM_WIN)
	#define GLSL_MACRO_PLATFORM "#define SHADER_PLATFORM_WIN\n"
#elif defined(PLATFORM_MAC)
	#define GLSL_MACRO_PLATFORM "#define SHADER_PLATFORM_MAC\n"
#elif defined(PLATFORM_IOS)
	#define GLSL_MACRO_PLATFORM "#define SHADER_PLATFORM_IOS\n"
#elif defined(PLATFORM_ANDROID)
	#define GLSL_MACRO_PLATFORM "#define SHADER_PLATFORM_ANDROID\n"
#endif

static const LPCSTR glsl_predefined = GLSL_MACRO_GLAPI GLSL_MACRO_PLATFORM "\n\n";
}

LPCSTR ShaderSourceCodeLibrary::defaultVertexShaderSourceCode = 
"SHADER_PRECISION(mediump,float)\n"
"attribute vec4 _Position;"
"attribute vec4 _Color;"
"uniform mat4 _MVP;"
"varying vec4 fragColor;"
"void main()"
"{"
"	gl_Position = _MVP*_Position;"
"	fragColor = _Color;"
"}";

LPCSTR ShaderSourceCodeLibrary::defaultFragmentShaderSourceCode = 
"SHADER_PRECISION(mediump,float)\n"
"varying vec4 fragColor;"
"void main()"
"{	gl_FragColor = fragColor;"
"}";


/////////////////////////////////////////////////////////////
// ShaderSourceCodeLibrary
ShaderSourceCodeLibrary::~ShaderSourceCodeLibrary()
{
	RemoveAll();
}

ShaderSourceCodeLibrary::ShaderSourceCodeLibrary()
{
}

void ShaderSourceCodeLibrary::SetPredefines(const rt::String_Ref& defines)
{
	_Predefines = defines;
}

const ShaderSourceCodeLibrary::_SourceCode*	ShaderSourceCodeLibrary::_GetSourceCode(const rt::String_Ref& name) const
{
	t_SourceCodes::const_iterator it = _SourceCodes.find(name);
	if(it != _SourceCodes.end())
		return it->second;
	return NULL;
}

ShaderSourceCodeLibrary::_ManagedShader* ShaderSourceCodeLibrary::_GetManagedShader(ShaderProgram* p)
{
	t_ManagedShaders::iterator it = _ManagedShaders.find(p);
	if(it != _ManagedShaders.end())
		return it->second;

	return NULL;
}

bool ShaderSourceCodeLibrary::_LookProgmaOnce(rt::String_Ref& code)
{
	bool has = false;
	rt::String_Ref line;
	while(code.GetNextLine(line))
	{
		rt::String_Ref p = line.TrimSpace();
		if(p.StartsWith("#pragma") && p[7]<=' ' && p.TrimLeft(7).TrimSpace() == "once")
		{
			line.Fill(' ');
			has = true;
		}
	}

	return has;
}

ShaderSourceCodeLibrary& ShaderSourceCodeLibrary::AddSourceCode(const rt::String_Ref& name, const rt::String_Ref& code)
{
	ASSERT(_GetSourceCode(name) == NULL);
	_SourceCode* p = new _SourceCode;
	ASSERT(p);

	p->LastModify = os::Timestamp::Get();
	p->Name = name;
	p->SourceCode = code;
	p->IncludeOnce = _LookProgmaOnce(p->SourceCode);

	_SourceCodes[p->Name] = p;

	return *this;
}

ShaderSourceCodeLibrary& ShaderSourceCodeLibrary::AddSourceCodes(const ShaderProgram& shader, const rt::String_Ref& vertex_shader_code, const rt::String_Ref& fragment_shader_code)
{
	if(!vertex_shader_code.IsEmpty())
		AddSourceCode(shader.GetVertexShaderName(), vertex_shader_code);

	if(!fragment_shader_code.IsEmpty())
		AddSourceCode(shader.GetFragmentShaderName(), fragment_shader_code);

	return *this;
}

ShaderSourceCodeLibrary& ShaderSourceCodeLibrary::AddExternalFile(const rt::String_Ref& name, const rt::String_Ref& extern_filename)
{
	ASSERT(_GetSourceCode(name) == NULL);

	rt::tos::StringOnStack<1024> fn(extern_filename);
	os::FileRead<char>	file(fn);
	if(file.GetSize())
	{
		_SourceCode* p = new _SourceCode;
		ASSERT(p);

		p->LastModify = os::Timestamp::Get();
		p->Name = name;
		p->SourceCode = rt::String_Ref(file, file.GetSize());
		p->IncludeOnce = _LookProgmaOnce(p->SourceCode);
		p->ExternFilename = extern_filename;

		_SourceCodes[p->Name] = p;
	}
	else
	{	_LOG("ShaderLib: Failed to open "<<extern_filename);
	}

	return *this;
}

void ShaderSourceCodeLibrary::RebuildAll()
{
	t_ManagedShaders::iterator it = _ManagedShaders.begin();
	for(; it != _ManagedShaders.end(); it++)
		it->second->_pProgram->Rebuild();
}

void ShaderSourceCodeLibrary::RemoveAll()
{
	{	t_SourceCodes::iterator it = _SourceCodes.begin();
		for(;it != _SourceCodes.end(); it++)
			_SafeDel(it->second);
		_SourceCodes.clear();
	}

	{	t_ManagedShaders::iterator it = _ManagedShaders.begin();
		for(;it != _ManagedShaders.end(); it++)
			_SafeDel(it->second);
		_ManagedShaders.clear();
	}
}

ShaderSourceCodeLibrary& ShaderSourceCodeLibrary::AddManagedShader(ShaderProgram* pShader)
{
	ASSERT(_ManagedShaders.find(pShader) == _ManagedShaders.end());

	_ManagedShader* p = new _ManagedShader;
	ASSERT(p);
	p->_pProgram = pShader;
	_ManagedShaders[pShader] = p;

	return *this;
}

namespace _details
{
	ShaderSourceCodeLibrary _ShadeLib;
}


ShaderSourceCodeLibrary* ShaderSourceCodeLibrary::Get()
{
	return &_details::_ShadeLib;
}


/////////////////////////////////////////////////////////////
// ShaderProgramBase
void ShaderProgramBase::Create()
{
	ASSERT(_Handle==NULL); 
	_Handle = glCreateProgram();
	_LogGLError;
	_EnabledVertexAttributeLocations = 0;
}

void ShaderProgramBase::Destroy()
{
	if(_Handle)
	{	glDeleteProgram(_Handle);
		_Handle = NULL;
	}
}

void ShaderProgramBase::GetInfoLog(rt::String& str)
{
	str.SetLength(GetInfoLogLength());
	glGetProgramInfoLog(_Handle, (int)str.GetLength(), NULL, str.Begin());
	_LogGLError;
}

void ShaderProgramBase::Link()
{	
	ASSERT(_Handle); 
	try
	{	/*
		if(_GeometryEnabled)
		{
			ASSERT(ProgramParameteriEXT);
			int temp;
			glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT,&temp);
			ProgramParameteriEXT(_Handle, GL_GEOMETRY_VERTICES_OUT_EXT, temp-1);
			ProgramParameteriEXT(_Handle, GL_GEOMETRY_INPUT_TYPE_EXT, _GeometryInputMode);
			ProgramParameteriEXT(_Handle, GL_GEOMETRY_OUTPUT_TYPE_EXT, _GeometryOutputMode);
		}
		*/
		glLinkProgram(_Handle);
		_LogGLError; 
		//DumpCompiledInformation();
	}
	catch(...)  //stop exception
	{	_LogGLError; 
	}
}

void ShaderProgramBase::DumpCompiledInformation()
{
	_LOG("\tAttached Shaders :\t"<<GetAttachedShaderCount());
	_LOG("\tActived Attribute:\t"<<GetActivedAttribCount());
	_LOG("\tActived Uniform  :\t"<<GetActivedUniformCount());
}

void ShaderProgramBase::AddShader(GLhandle sh)
{
	ASSERT(_Handle);
	ASSERT(sh);

	glAttachShader(_Handle,sh);
	_LogGLError;
}

void ShaderProgramBase::RemoveShader(GLhandle sh)
{
	ASSERT(_Handle);
	ASSERT(sh);

	glDetachShader(_Handle,sh);
	_LogGLError;
}

void ShaderProgramBase::BindAttribLocation(LPCSTR VarName,int AttribIndex)
{
	ASSERT(_Handle);
	ASSERT(VarName);
	
	glBindAttribLocation(_Handle,AttribIndex,VarName);
	_LogGLError;
}

GLhandle ShaderProgramBase::GetCurrentProgram()
{
	GLint ret;
	glGetIntegerv(GL_CURRENT_PROGRAM, &ret);
	return (GLhandle)(SSIZE_T)ret;
}


void ShaderProgramBase::GetActivedUniform(int index,ShaderVarible& varible)
{
	varible.Name.SetLength(GetMaxUniformNameLength());
	varible.Index = index;

	glGetActiveUniform(_Handle,index,(int)varible.Name.GetLength(),NULL,&varible.Size,&varible.Type,varible.Name.Begin());
	_LogGLError;
}

void ShaderProgramBase::GetActivedAttrib(int index,ShaderVarible& varible)
{
	varible.Name.SetLength(GetMaxAttribNameLength());
	varible.Index = index;

	glGetActiveAttrib(_Handle,index,(int)varible.Name.GetLength(),NULL,&varible.Size,&varible.Type,varible.Name.Begin());
	_LogGLError;
}

/*
void ShaderProgramBase::SetGeometryShaderIOMode(int input, int output)
{
	_GeometryEnabled = true;
	_GeometryInputMode = input;
	_GeometryOutputMode = output;
}
*/

/////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////
// Shader Object

void ShaderVarible::GetTypeString(rt::String& str)
{
	ASSERT(Size);
	switch(Type)
	{
	case GL_FLOAT					: str = "float"; break;
	case GL_FLOAT_VEC2				: str = "vec2"; break;
	case GL_FLOAT_VEC3				: str = "vec3"; break;
	case GL_FLOAT_VEC4				: str = "vec4"; break;
	case GL_INT						: str = "int"; break;
	case GL_INT_VEC2				: str = "ivec2"; break;
	case GL_INT_VEC3				: str = "ivec3"; break;
	case GL_INT_VEC4				: str = "ivec4"; break;
	case GL_BOOL					: str = "bool"; break;
	case GL_BOOL_VEC2				: str = "bvec2"; break;
	case GL_BOOL_VEC3				: str = "bvec3"; break;
	case GL_BOOL_VEC4				: str = "bvec4"; break;
	case GL_FLOAT_MAT2				: str = "mat2"; break;
	case GL_FLOAT_MAT3				: str = "mat3"; break;
	case GL_FLOAT_MAT4				: str = "mat4"; break;
	case GL_SAMPLER_CUBE			: str = "samplerCube"; break;
	case GL_SAMPLER_2D				: str = "sampler2D"; break;
#if defined(PLATFORM_OPENGL_SUPPORT)
	case GL_SAMPLER_1D_ARB			: str = "sampler1D"; break;
	case GL_SAMPLER_3D				: str = "sampler3D"; break;
	case GL_SAMPLER_1D_SHADOW_ARB		: str = "sampler1DShadow"; break;
	case GL_SAMPLER_2D_SHADOW		: str = "sampler2DShadow"; break;
	case GL_SAMPLER_2D_RECT_ARB			: str = "sampler2DRect"; break;
	case GL_SAMPLER_2D_RECT_SHADOW_ARB	: str = "sampler2DRectShadow"; break;
#endif
	default: str = "[UnknownType]";
	}

	str += rt::String_Ref() + ' ' + Name;
	if(Size > 1)str += rt::String_Ref() + '[' + Size + ']';
}

void ShaderCode::Create(GLenum ShaderType)
{
	ASSERT(_Handle==NULL); 
	_Handle = glCreateShader(ShaderType); 
	_Type = ShaderType;
	_LogGLError; 
}

void ShaderCode::Destroy()
{
	if(_Handle)
	{	glDeleteShader(_Handle);
		_Handle = NULL;
	}
}


void ShaderCode::GetInfoLog(rt::String& str)
{
	str.SetLength(GetInfoLogLength());
	glGetShaderInfoLog(_Handle, (int)str.GetLength(), NULL, str.Begin());
	_LogGLError;
}


void ShaderCode::Compile()
{
	ASSERT(_Handle);
	glCompileShader(_Handle);
	_LogGLError;
}
	
bool ShaderCode::SetSourceCode(LPCSTR pStr, LPCSTR predefines)
{ 
	ASSERT(pStr);

	if(GL_GEOMETRY_SHADER_EXT == _Type)
	{
		static int out_vmax_hardware = -1;
		if(out_vmax_hardware == -1)
		{
			out_vmax_hardware = 1024;
			glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT,&out_vmax_hardware);
		}

		LPCSTR p = pStr;
		int in_mode = -1;
		int out_mode = -1;
		int out_vmax = -1;

		while(in_mode == -1 || out_mode == -1)
		{
			LPCSTR l,m;
			if((l = strstr(p,"layout")))
			{
				p = l+6;
				while(*p<=' ')p++;
				if(*p != '(')continue;
				p++;
				m = p;
				while(*p++!=')')
					if(*p == '\0')goto END_OF_LAYOUT_PARSE;

				rt::String_Ref f[2];
				int co = rt::String_Ref(m, p-1).Split(f,2,',');

				p++;
				while(*p<=' ')p++;

				if(in_mode == -1 && p[0] == 'i' && p[1] == 'n')
				{	// parse input mode
					rt::String_Ref mode = f[0].TrimSpace();
					if(mode == rt::SS("points")){ in_mode = GL_POINTS; }else
					if(mode == rt::SS("lines")){ in_mode = GL_LINES; }else
					if(mode == rt::SS("lines_adjacency")){ in_mode = GL_LINES_ADJACENCY; }else
					if(mode == rt::SS("triangles")){ in_mode = GL_TRIANGLES; }else
					if(mode == rt::SS("triangles_adjacency")){ in_mode = GL_TRIANGLES_ADJACENCY; }
				}
				else 
				if(out_mode == -1 && p[0] == 'o' && p[1] == 'u' && p[2] == 't')
				{	// parse output mode
					rt::String_Ref mode = f[0].TrimSpace();
					if(mode == rt::SS("points")){ out_mode = GL_POINTS; }else
					if(mode == rt::SS("line_strip")){ out_mode = GL_LINE_STRIP; }else
					if(mode == rt::SS("triangle_strip")){ out_mode = GL_TRIANGLE_STRIP; }
					if(co == 2)
					{
						rt::String_Ref q[2];
						if(	2 == f[1].TrimSpace().Split(q,2,'=') &&
							q[0].TrimSpace() == rt::SS("max_vertices")
						)
						{	q[1].ToNumber(out_vmax);
						}
					}
				}

			}
			else break;
		}

	END_OF_LAYOUT_PARSE:

		if(in_mode == -1 || out_mode == -1)
		{
			_LOG_WARNING("Geometry shader I/O Mode specification is not found.");
			return false;
		}

		_GeometryShaderInMode = in_mode;
		_GeometryShaderOutMode = out_mode;
		_GeometryShaderOutVerticesMax = out_vmax;
	}

	LPCSTR s[4];
	switch(_Type)
	{
	case VertexShader:
		s[0] = "#define SHADER_VERTEX\n";
		break;
	case FragmentShader:
		s[0] = "#define SHADER_FRAGMENT\n";
		break;
	case GeometryShader:
		s[0] = "#define SHADER_GEOMETRY\n";
		break;
	default: ASSERT(0);
	}

	s[1] = _details::glsl_predefined;
	
	if(predefines)
	{	
		s[2] = predefines;
		s[3] = pStr;
		glShaderSource(_Handle,4,s,NULL);
	}
	else
	{
		s[2] = pStr;
		glShaderSource(_Handle,3,s,NULL);
	}

	_LogGLError;
	return true;
}


bool ShaderCode::__MergeCode(const rt::String_Ref& code_name)
{
	const ShaderSourceCodeLibrary::_SourceCode* code = ShaderSourceCodeLibrary::Get()->_GetSourceCode(code_name);
	if(code)
	{
		rt::String_Ref line;
		while(code->SourceCode.GetNextLine(line))
		{
			if(line.TrimLeftSpace().StartsWith("#import"))
			{
				rt::String_Ref inc = line.TrimSpace();
				int start = 0;
				int end = 0;
				if(inc[7] <= ' ' && (start = inc.FindCharacter('<') > 7 && (end = inc.FindCharacter('>', start) > start)))
				{
					if(!__MergeCode(inc.SubStr(start+1, end - start)))return false;
				}
				else
				{
					_LOG("ShadeLib: illegal import statement \""<<inc<<'\"');
				}
			}
			else
			{	__Preprocessed += line;
				__Preprocessed += '\r';
				__Preprocessed += '\n';
			}
		}

		return true;
	}

	return false;
}

bool ShaderCode::LoadSourceCode(const rt::String_Ref& name, LPCSTR predefines, _details::t_Dependency* pUpdateDependency) // load from shadersourcecodelibrary
{
	_details::t_Dependency _dep;
	__pDependency = pUpdateDependency?pUpdateDependency:&_dep;

	if(__MergeCode(name))
	{
		return SetSourceCode(__Preprocessed, predefines);
	}

	return false;
}


bool ShaderCode::IsCompiled()
{
	GLint ret = 0; 
	if(IsCreated())
	{	glGetShaderiv(_Handle,GL_COMPILE_STATUS,&ret); _LogGLError; 
	} 
	return ret;
}

/////////////////////////////////////////////
// Program project

void ShaderProgram::_ctor_init(const rt::String_Ref& vertex_sh, const rt::String_Ref& geometry_sh, const rt::String_Ref& fragment_sh)
{
	_VertexShaderName = vertex_sh;
	_FragmentShaderName = fragment_sh;
	_GeometryShaderName = geometry_sh;

	_LastBuilt = 0;
	_is_managed = true;

	ShaderSourceCodeLibrary::Get()->AddManagedShader(this);
}

ShaderProgram::ShaderProgram()
{
	_LastBuilt = 0;
	_is_managed = false;
}

bool ShaderProgram::Rebuild(DWORD invalidated)
{
	bool failed = false;

	ASSERT(_is_managed);

	if(!IsCreated())Create();

	ShaderSourceCodeLibrary::_ManagedShader* pMan = ShaderSourceCodeLibrary::Get()->_GetManagedShader(this);

	if(!_VertexShaderName.IsEmpty() && (invalidated&VERTEX_SHADER_INVALIDATED))
	{
		if(!_vsh.IsCreated())_vsh.Create(gl::ShaderCode::VertexShader);
		if(_vsh.LoadSourceCode(_VertexShaderName, _Predefines, pMan?(&pMan->_VertexShaderDependency):NULL))
			_vsh.Compile();
		if(_vsh.IsCompiled())
			AddShader(_vsh);
		else
		{	rt::String err;
			_vsh.GetInfoLog(err);
			_LOG(err);
			failed = true;
		}
	}

	if(!_FragmentShaderName.IsEmpty() && (invalidated&FRAGMENT_SHADER_INVALIDATED))
	{
		if(!_fsh.IsCreated())_fsh.Create(gl::ShaderCode::FragmentShader);
		if(_fsh.LoadSourceCode(_FragmentShaderName, _Predefines, pMan?(&pMan->_FragmentShaderDependency):NULL))
			_fsh.Compile();
		if(_fsh.IsCompiled())
			AddShader(_fsh);
		else
		{	rt::String err;
			_vsh.GetInfoLog(err);
			_LOG(err);
			failed = true;
		}
	}

	if(!_GeometryShaderName.IsEmpty() && (invalidated&GEOMETRY_SHADER_INVALIDATED))
	{
		if(!_gsh.IsCreated())_gsh.Create(gl::ShaderCode::GeometryShader);
		if(_gsh.LoadSourceCode(_GeometryShaderName, _Predefines, pMan?(&pMan->_GeometryShaderDependency):NULL))
			_gsh.Compile();

		if(_gsh.IsCompiled())
			AddShader(_gsh);
		else
		{	rt::String err;
			_vsh.GetInfoLog(err);
			_LOG(err);
			failed = true;
		}
	}

	if(!failed)
	{
		Link();
		if(Validate()){ return true; }
		else
		{	rt::String err;
			GetInfoLog(err);
			_LOG(err);
		}
	}

	return false;
}

void ShaderProgram::SetPredefines(LPCSTR pred)
{
	_Predefines = pred;
}

void ShaderProgram::BuildWithSourceCodes(LPCSTR vsh, LPCSTR gsh, LPCSTR fsh)
{
	ASSERT(!_is_managed);

	if(!IsCreated())Create();
	if(fsh && !_fsh.IsCreated())_fsh.Create(gl::ShaderCode::FragmentShader);
	if(gsh && !_gsh.IsCreated())_gsh.Create(gl::ShaderCode::GeometryShader);
	if(vsh && !_vsh.IsCreated())_vsh.Create(gl::ShaderCode::VertexShader);

	//bool err_occ = false;

	if(fsh)
	{
		_fsh.SetSourceCode(fsh, _Predefines);
		_fsh.Compile();
		if(_fsh.IsCompiled())
		{	
			AddShader(_fsh); 
		}
		else
		{	rt::String err;
			_fsh.GetInfoLog(err);
			_LOG_WARNING(err);

		}
	}

	if(gsh && _gsh.SetSourceCode(gsh, _Predefines))
	{
		_gsh.Compile();
		if(_gsh.IsCompiled())
		{
			AddShader(_gsh); 
#if defined(PLATFORM_MAC)
            glProgramParameteriEXT(*(GLuint*)&_Handle,GL_GEOMETRY_INPUT_TYPE_EXT,_gsh._GeometryShaderInMode);
            glProgramParameteriEXT(*(GLuint*)&_Handle,GL_GEOMETRY_OUTPUT_TYPE_EXT,_gsh._GeometryShaderOutMode);
            glProgramParameteriEXT(*(GLuint*)&_Handle,GL_GEOMETRY_VERTICES_OUT_EXT,_gsh._GeometryShaderOutVerticesMax);
#else
			glProgramParameteriEXT(_Handle,GL_GEOMETRY_INPUT_TYPE_EXT,_gsh._GeometryShaderInMode);
			glProgramParameteriEXT(_Handle,GL_GEOMETRY_OUTPUT_TYPE_EXT,_gsh._GeometryShaderOutMode);
			glProgramParameteriEXT(_Handle,GL_GEOMETRY_VERTICES_OUT_EXT,_gsh._GeometryShaderOutVerticesMax);
#endif
			_LogGLError;
		}
		else
		{	rt::String err;
			_gsh.GetInfoLog(err);
			_LOG_WARNING(err);
		}
	}

	if(vsh)
	{
		_vsh.SetSourceCode(vsh, _Predefines);
		_vsh.Compile();
		if(_vsh.IsCompiled())
		{
			AddShader(_vsh); 
		}
		else
		{	rt::String err;
			_vsh.GetInfoLog(err);
			_LOG_WARNING(err);
		}
	}

	Link();
	if(Validate()){}
	else
	{	rt::String err;
		GetInfoLog(err);
		_LOG_WARNING(err);
	}
}

void ShaderProgram::Destroy()
{
	if(!IsCreated())ShaderProgramBase::Destroy();
	if(!_fsh.IsCreated())_fsh.Destroy();
	if(!_vsh.IsCreated())_vsh.Destroy();
	if(!_gsh.IsCreated())_gsh.Destroy();
}




} // namespace gl
