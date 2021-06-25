#pragma once

/**
 * @file gl_shader.h
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
/** \addtogroup Gl 
 * @ingroup CPCF
 *  @{
 */
#include "gl_basic.h"
#include "gl_texture.h"
#include "../rt/string_type.h"
#include "../os/file_dir.h"


namespace gl
{
/** \addtogroup Gl_Shader
 * @ingroup Gl
 *  @{
 */
namespace _details
{
	typedef rt::hash_set<rt::String_Ref, rt::String_Ref::hash_compare>	t_Dependency;
};

#define DEF_STATUS_QUERY(type,name_func,arug) type name_func() \
		{ GLint ret = 0; if(IsCreated()){ _Getiv(_Handle,arug,&ret); _LogGLError; } return ret; }

class ShaderVarible
{
public:
	int			Index;	///<available for attrib varibles
	int			Size;
	rt::String	Name;
	GLenum		Type;
public:
	void GetTypeString(rt::String& str);
};

class ShaderCode
{
	friend class ShaderProgram;
	_details::t_Dependency* __pDependency;
	rt::String				__Preprocessed;
	bool					__MergeCode(const rt::String_Ref& code_name);
	
protected:
	GLhandle	_Handle;
	DWORD		_Type;
	int			_GeometryShaderInMode;
	int			_GeometryShaderOutMode;
	int			_GeometryShaderOutVerticesMax;
public:
	enum
	{	VertexShader = GL_VERTEX_SHADER,
		FragmentShader = GL_FRAGMENT_SHADER,
		GeometryShader = GL_GEOMETRY_SHADER_EXT
	};

	ShaderCode(){ _Handle = NULL; }
	operator GLhandle() const { return _Handle; }
	
	void Create(GLenum ShaderType);
	void Compile();
	bool SetSourceCode(LPCSTR pStr, LPCSTR predefines = NULL);
	bool LoadSourceCode(const rt::String_Ref& name, LPCSTR predefines, _details::t_Dependency* pUpdateDependency = NULL); ///< load from shadersourcecodelibrary
	bool IsCompiled();

	bool IsCreated() const { return 0 != _Handle; }
	void Destroy();
	void GetInfoLog(rt::String& str);

#define _Getiv	glGetShaderiv
	DEF_STATUS_QUERY(int,GetSourceCodeLength,GL_SHADER_SOURCE_LENGTH);
	DEF_STATUS_QUERY(int,GetInfoLogLength,GL_INFO_LOG_LENGTH);
#undef _Getiv
};

class ShaderProgramBase
{
protected:
	GLhandle	_Handle;
	int			_NextTextureUnit;
	DWORD		_EnabledVertexAttributeLocations;
	
public:
	ShaderProgramBase(){
		_Handle = NULL;
		_EnabledVertexAttributeLocations = 0;
	}
	void Create();
	void AddShader(GLhandle sh);
	void RemoveShader(GLhandle sh);
	void Link();
	void Use(){ ASSERT(_Handle); glUseProgram(_Handle); _NextTextureUnit = 0; _LogGLError; }
	bool Validate(){ ASSERT(_Handle); glValidateProgram(_Handle); _LogGLError; return IsLinked(); }

	operator GLhandle() const { return _Handle; }
	static void DisableShaderProgram(){ glUseProgram(NULL); _LogGLError; }

	void BindAttribLocation(LPCSTR VarName,int AttribIndex);
	int  GetAttribLocation(LPCSTR Varname){ ASSERT(_Handle); return glGetAttribLocation(_Handle,Varname); }
	bool IsAttribAvailable(LPCSTR Varname){ return -1 != GetAttribLocation(Varname); }
	int	 Get(LPCSTR Varname){ ASSERT(_Handle); return glGetUniformLocation(_Handle,Varname); }

	#define _Getiv	glGetProgramiv
	DEF_STATUS_QUERY(bool,IsLinked,GL_LINK_STATUS);
	DEF_STATUS_QUERY(bool,IsReadyToRun,GL_VALIDATE_STATUS);
	
	DEF_STATUS_QUERY(int,GetAttachedShaderCount,GL_ATTACHED_SHADERS);
	DEF_STATUS_QUERY(int,GetActivedAttribCount,GL_ACTIVE_ATTRIBUTES);
	DEF_STATUS_QUERY(int,GetMaxAttribNameLength,GL_ACTIVE_ATTRIBUTE_MAX_LENGTH);
	DEF_STATUS_QUERY(int,GetActivedUniformCount,GL_ACTIVE_UNIFORMS);
	DEF_STATUS_QUERY(int,GetMaxUniformNameLength,GL_ACTIVE_UNIFORM_MAX_LENGTH);
	DEF_STATUS_QUERY(int,GetInfoLogLength,GL_INFO_LOG_LENGTH);
	#undef _Getiv

	void DumpCompiledInformation();

	bool IsCreated() const { return NULL != _Handle; }
	void Destroy();
	void GetInfoLog(rt::String& str);

	void GetActivedUniform(int index, ShaderVarible& varible);
	void GetActivedAttrib(int index, ShaderVarible& varible);
	static GLhandle GetCurrentProgram();

public:
#define UFLO	ASSERT(_Handle); ASSERT(Name); GLint loc = glGetUniformLocation(_Handle,Name); if(loc<0)_LOG_WARNING("Uniform "<<Name<<" is not found.");

	INLFUNC void SetUniform(LPCSTR Name, float x){ UFLO; glUniform1f(loc,x); }
	INLFUNC void SetUniform(LPCSTR Name, float x,float y){ UFLO; glUniform2f(loc,x,y); }
	INLFUNC void SetUniform(LPCSTR Name, float x,float y,float z){ UFLO; glUniform3f(loc,x,y,z); }
	INLFUNC void SetUniform(LPCSTR Name, float x,float y,float z,float w){ UFLO; glUniform4f(loc,x,y,z,w); }
    INLFUNC void SetUniformArray1(LPCSTR Name, const float* p, int count){ UFLO; ASSERT(p); glUniform1fv(loc,count,p); }
    INLFUNC void SetUniformArray2(LPCSTR Name, const float* p, int count){ UFLO; ASSERT(p); glUniform2fv(loc,count,p); }
    INLFUNC void SetUniformArray3(LPCSTR Name, const float* p, int count){ UFLO; ASSERT(p); glUniform3fv(loc,count,p); }
    INLFUNC void SetUniformArray4(LPCSTR Name, const float* p, int count){ UFLO; ASSERT(p); glUniform4fv(loc,count,p); }
    INLFUNC void SetUniform(LPCSTR Name, const rt::Vec2f& v){ UFLO; glUniform2f(loc,v.x,v.y); }
    INLFUNC void SetUniform(LPCSTR Name, const rt::Vec3f& v){ UFLO; glUniform3f(loc,v.x,v.y,v.z); }
    INLFUNC void SetUniform(LPCSTR Name, const rt::Vec4f& v){ UFLO; glUniform4f(loc,v.x,v.y,v.z,v.w); }
    INLFUNC void SetUniformArray2(LPCSTR Name, const rt::Vec2f* p, int count){ UFLO; ASSERT(p); glUniform2fv(loc,count,p[0]); }
    INLFUNC void SetUniformArray3(LPCSTR Name, const rt::Vec3f* p, int count){ UFLO; ASSERT(p); glUniform3fv(loc,count,p[0]); }
    INLFUNC void SetUniformArray4(LPCSTR Name, const rt::Vec4f* p, int count){ UFLO; ASSERT(p); glUniform4fv(loc,count,p[0]); }
    INLFUNC void SetUniformMatrix2x2(LPCSTR Name,const float* mat,bool transpose = false){ UFLO; ASSERT(mat); glUniformMatrix2fv(loc,1,transpose,mat); }
    INLFUNC void SetUniformMatrix3x3(LPCSTR Name,const float* mat,bool transpose = false){ UFLO; ASSERT(mat); glUniformMatrix3fv(loc,1,transpose,mat); }
    INLFUNC void SetUniformMatrix4x4(LPCSTR Name,const float* mat,bool transpose = false){ UFLO; ASSERT(mat); glUniformMatrix4fv(loc,1,transpose,mat); }
    
	INLFUNC void SetUniform(LPCSTR Name, int x){ UFLO; glUniform1i(loc,x); }
	INLFUNC void SetUniform(LPCSTR Name, int x,int y){ UFLO; glUniform2i(loc,x,y); }
	INLFUNC void SetUniform(LPCSTR Name, int x,int y,int z){ UFLO; glUniform3i(loc,x,y,z); }
	INLFUNC void SetUniform(LPCSTR Name, int x,int y,int z,int w){ UFLO; glUniform4i(loc,x,y,z,w); }
    INLFUNC void SetUniformArray1(LPCSTR Name, const int* p, int count){ UFLO; ASSERT(p); glUniform1iv(loc,count,p); }
    INLFUNC void SetUniformArray2(LPCSTR Name, const int* p, int count){ UFLO; ASSERT(p); glUniform2iv(loc,count,p); }
    INLFUNC void SetUniformArray3(LPCSTR Name, const int* p, int count){ UFLO; ASSERT(p); glUniform3iv(loc,count,p); }
    INLFUNC void SetUniformArray4(LPCSTR Name, const int* p, int count){ UFLO; ASSERT(p); glUniform4iv(loc,count,p); }
    INLFUNC void SetUniform(LPCSTR Name, const rt::Vec2i& v){ UFLO; glUniform2i(loc,v.x,v.y); }
    INLFUNC void SetUniform(LPCSTR Name, const rt::Vec3i& v){ UFLO; glUniform3i(loc,v.x,v.y,v.z); }
    INLFUNC void SetUniform(LPCSTR Name, const rt::Vec4i& v){ UFLO; glUniform4i(loc,v.x,v.y,v.z,v.w); }
    INLFUNC void SetUniformArray2(LPCSTR Name, const rt::Vec2i* p, int count){ UFLO; ASSERT(p); glUniform2iv(loc,count,p[0]); }
    INLFUNC void SetUniformArray3(LPCSTR Name, const rt::Vec3i* p, int count){ UFLO; ASSERT(p); glUniform3iv(loc,count,p[0]); }
    INLFUNC void SetUniformArray4(LPCSTR Name, const rt::Vec4i* p, int count){ UFLO; ASSERT(p); glUniform4iv(loc,count,p[0]); }
    
    /*
    INLFUNC void SetUniform(LPCSTR Name, double x){ UFLO; glUniform1d(loc,x); }
    INLFUNC void SetUniform(LPCSTR Name, double x,double y){ UFLO; glUniform2d(loc,x,y); }
    INLFUNC void SetUniform(LPCSTR Name, double x,double y,double z){ UFLO; glUniform3d(loc,x,y,z); }
    INLFUNC void SetUniform(LPCSTR Name, double x,double y,double z,double w){ UFLO; glUniform4d(loc,x,y,z,w); }
	INLFUNC void SetUniformArray1(LPCSTR Name, const double* p, int count){ UFLO; ASSERT(p); glUniform1dv(loc,count,p); }
	INLFUNC void SetUniformArray2(LPCSTR Name, const double* p, int count){ UFLO; ASSERT(p); glUniform2dv(loc,count,p); }
	INLFUNC void SetUniformArray3(LPCSTR Name, const double* p, int count){ UFLO; ASSERT(p); glUniform3dv(loc,count,p); }
	INLFUNC void SetUniformArray4(LPCSTR Name, const double* p, int count){ UFLO; ASSERT(p); glUniform4dv(loc,count,p); }
	INLFUNC void SetUniform(LPCSTR Name, const rt::Vec2d& v){ UFLO; glUniform2d(loc,v.x,v.y); }
	INLFUNC void SetUniform(LPCSTR Name, const rt::Vec3d& v){ UFLO; glUniform3d(loc,v.x,v.y,v.z); }
	INLFUNC void SetUniform(LPCSTR Name, const rt::Vec4d& v){ UFLO; glUniform4d(loc,v.x,v.y,v.z,v.w); }
	INLFUNC void SetUniformArray2(LPCSTR Name, const rt::Vec2d* p, int count){ UFLO; ASSERT(p); glUniform2dv(loc,count,p[0]); }
	INLFUNC void SetUniformArray3(LPCSTR Name, const rt::Vec3d* p, int count){ UFLO; ASSERT(p); glUniform3dv(loc,count,p[0]); }
	INLFUNC void SetUniformArray4(LPCSTR Name, const rt::Vec4d* p, int count){ UFLO; ASSERT(p); glUniform4dv(loc,count,p[0]); }
	INLFUNC void SetUniformMatrix2x2(LPCSTR Name,const double* mat,bool transpose = false){ UFLO; ASSERT(mat); glUniformMatrix2dv(loc,1,transpose,mat); }
	INLFUNC void SetUniformMatrix3x3(LPCSTR Name,const double* mat,bool transpose = false){ UFLO; ASSERT(mat); glUniformMatrix3dv(loc,1,transpose,mat); }
	INLFUNC void SetUniformMatrix4x4(LPCSTR Name,const double* mat,bool transpose = false){ UFLO; ASSERT(mat); glUniformMatrix4dv(loc,1,transpose,mat); }
    */
#undef UFLO

	INLFUNC void SetVertexAttributePointer(LPCSTR attrib_name, LPCVOID pData, UINT channel, DWORD type, int stride = 0)
	{	int loc = GetAttribLocation(attrib_name);
		_EnabledVertexAttributeLocations |= (1<<loc);
		glEnableVertexAttribArray(loc);
		if(type == GL_INT || type == GL_UNSIGNED_INT)
		{
			glVertexAttribIPointerEXT(loc, channel, type, stride, pData);
		}
		else
		{
			glVertexAttribPointer(loc, channel, type, type == GL_UNSIGNED_BYTE , stride, pData);
		}
		_LogGLError;
	}
	INLFUNC void UnsetAllVertexAttributePointers()
	{	for(UINT i=0;i<32;i++)
		{	int loc = i;
			if((1<<loc)&_EnabledVertexAttributeLocations)
				glDisableVertexAttribArray(loc);
		}
		_EnabledVertexAttributeLocations = 0;
	}
	template<class TextureClass>
	INLFUNC void SetTexture(LPCSTR sampler_name, const TextureClass& tex)
	{	glActiveTexture(GL_TEXTURE0 + _NextTextureUnit);
		tex.Use();
		SetUniform(sampler_name, _NextTextureUnit);
		_NextTextureUnit++;
		_LogGLError;
	}
	template<int TexTarget>
	INLFUNC void SetTexture(LPCSTR sampler_name, GLint TexName)
	{	glActiveTexture(GL_TEXTURE0 + _NextTextureUnit);
		glBindTexture(TexTarget, TexName);
		SetUniform(sampler_name, _NextTextureUnit);
		_NextTextureUnit++;
		_LogGLError;
	}
	INLFUNC void UnsetAllTextures(){ _NextTextureUnit = 0; }	///< will also be called in Use();
};


class ShaderProgram:public ShaderProgramBase
{
	void _ctor_init(const rt::String_Ref& vertex_sh, const rt::String_Ref& geometry_sh, const rt::String_Ref& fragment_sh);
protected:
	gl::ShaderCode	_fsh;
	gl::ShaderCode	_vsh;
	gl::ShaderCode	_gsh;
	__time64_t		_LastBuilt;
	bool			_is_managed;

	rt::String		_VertexShaderName;
	rt::String		_FragmentShaderName;
	rt::String		_GeometryShaderName;
	rt::String		_Predefines;
	
public:
	enum _tagInvalidateFlag
	{	VERTEX_SHADER_INVALIDATED	= 0x0001,
		FRAGMENT_SHADER_INVALIDATED	= 0x0002,
		GEOMETRY_SHADER_INVALIDATED	= 0x0004,
		ALL_SHADER_INVALIDATED		= 0x00FF,
	};
	ShaderProgram();	///< for unmanaged progam
	ShaderProgram(const rt::String_Ref& vertex_sh, const rt::String_Ref& fragment_sh){ _ctor_init(vertex_sh, NULL, fragment_sh); } ///< for managed program
	ShaderProgram(const rt::String_Ref& vertex_sh, const rt::String_Ref& geometry_sh, const rt::String_Ref& fragment_sh){ _ctor_init(vertex_sh, geometry_sh, fragment_sh); } // for managed program

	bool		Rebuild(DWORD invalidated = ALL_SHADER_INVALIDATED);
	LPCSTR		GetVertexShaderName() const { return _VertexShaderName; }
	LPCSTR		GetFragmentShaderName() const { return _FragmentShaderName; }
	LPCSTR		GetGeometryShaderName() const { return _GeometryShaderName; }

	void		SetPredefines(LPCSTR);
	void		BuildWithSourceCodes(LPCSTR vsh, LPCSTR fsh){ BuildWithSourceCodes(vsh,NULL,fsh); }; ///< vertex/fragment
	void		BuildWithSourceCodes(LPCSTR vertex_sh, LPCSTR geometry_sh, LPCSTR fragment_sh);	///< vertex/geometry/fragment
	void		Destroy();
};


class ShaderSourceCodeLibrary
{
	friend class ShaderProgram;
	friend class ShaderCode;
protected:
	rt::String							_Predefines;

protected:
	struct _SourceCode
	{	
		rt::String		Name;
		bool			IncludeOnce;
		__time64_t		LastModify;
		rt::String		SourceCode;
		rt::String		ExternFilename;
	};

	typedef rt::hash_map<rt::String_Ref, _SourceCode*, rt::String_Ref::hash_compare> t_SourceCodes;
	t_SourceCodes		_SourceCodes;
	const _SourceCode*	_GetSourceCode(const rt::String_Ref& name) const;
	static bool			_LookProgmaOnce(rt::String_Ref& code);

	struct _ManagedShader
	{	
		_details::t_Dependency	_VertexShaderDependency;
		_details::t_Dependency	_FragmentShaderDependency;
		_details::t_Dependency	_GeometryShaderDependency;
		ShaderProgram*	_pProgram;
	};

	typedef rt::hash_map<ShaderProgram*, _ManagedShader*, rt::_details::hash_compare_fix<ShaderProgram*> > t_ManagedShaders;
	t_ManagedShaders	_ManagedShaders;
	_ManagedShader*		_GetManagedShader(ShaderProgram* p);

public:
	static LPCSTR	defaultVertexShaderSourceCode;
	static LPCSTR	defaultFragmentShaderSourceCode;

public:
	static ShaderSourceCodeLibrary* Get();
	~ShaderSourceCodeLibrary();
	ShaderSourceCodeLibrary();
	void SetPredefines(const rt::String_Ref& defines = rt::String_Ref());

	ShaderSourceCodeLibrary& AddSourceCode(const rt::String_Ref& name, const rt::String_Ref& code);
	ShaderSourceCodeLibrary& AddSourceCodes(const ShaderProgram& shader, const rt::String_Ref& vertex_shader_code, const rt::String_Ref& fragment_shader_code);

	ShaderSourceCodeLibrary& AddExternalFile(const rt::String_Ref& name, const rt::String_Ref& extern_filename);

	ShaderSourceCodeLibrary& AddManagedShader(ShaderProgram* pShader);

    void Build();
	void RebuildAll();
	void RemoveAll();
};



#undef	DEF_STATUS_QUERY

/** @}*/
} // namespace gl
/** @}*/




