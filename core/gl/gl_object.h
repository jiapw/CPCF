#pragma once

/**
 * @file gl_object.h
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
#include "gl_shader.h"
#include "gl_math.h"
/** \addtogroup gl 
 * @ingroup CPCF
 *  @{
 */
namespace gl
{
/** \addtogroup gl_animator
 * @ingroup gl
 *  @{
 */
struct Surface
{
	enum _tagDataFlag
	{	DF_POSITION_3F	= 0x00000001,
		DF_NORMAL_3B	= 0x00000002,
		DF_TEXCOORD_2F	= 0x00000004,
		DF_AMBIENT_1B	= 0x00000008,
		DF_ATTRIB_MASK	= 0x00ffffff,
		DF_INDEX_1U		= 0x10000000,	///< element index is UINT
		DF_INDEX_1W		= 0x20000000,	///< element index is WORD
	};
	int					_MaterialIndex;
	rt::String			_MaterialName;
	DWORD				_DataFlag;
	UINT				_RenderOrder;
	rt::Vec3f			_Center;

	bool operator < (const Surface& s) const;

	VertexAttributeBuffer	_VertexAttrib;
	VertexIndexBuffer		_Triangles;
	UINT					_TriangleCount;
	UINT					_VertexCount;
	int						GetVertexAttributeSize() const; ///< in byte
};

enum _tagMaterialType
{	
	MATERIAL_TYPE_NONE = 0,
	MATERIAL_TYPE_PHONG,
	MATERIAL_TYPE_MAX
};

enum _tagMaterialConfig
{	
	MATERIAL_TYPE_SHIFT		=	16,
	MATERIAL_TYPE_MASK		=	0xff<<MATERIAL_TYPE_SHIFT,

	MATERIAL_USE_ALPHA		=	0x10000000,

	MATERIAL_PHONG			=	MATERIAL_TYPE_PHONG<<MATERIAL_TYPE_SHIFT,
	MATERIAL_PHONG_USE_KA	=	0x0001|MATERIAL_PHONG,
	MATERIAL_PHONG_USE_KD	=	0x0002|MATERIAL_PHONG,
	MATERIAL_PHONG_USE_KS	=	0x0004|MATERIAL_PHONG,

};


struct MaterialPhong
{
	/**
	 * @brief Phong/blin-Phong model
	 * 
	 */
	enum _tagShadingModel
	{
		SHADING_FLAT = 0,					///<  Color on and Ambient off
		SHADING_FLATAMBIENT = 1,			///<  Color on and Ambient on
		SHADING_PHONG = 2,					///<  Highlight on
		SHADING_RAYTRACE = 3,				///<  Reflection on and Ray trace on
		SHADING_RAYTRACE_TRANSPARENT = 4,	///<  Transparency: Glass on, Reflection: Ray trace on
		SHADING_FRESNEL = 5,				///<  Reflection: Fresnel on and Ray trace on
		SHADING_REFRACTION = 6,				///<  Transparency: Refraction on, Reflection: Fresnel off and Ray trace on
		SHADING_REFRACTION_FRESNEL = 7,		///<  Transparency: Refraction on, Reflection: Fresnel on and Ray trace on
		SHADING_REFLECTION = 8,				///<  Reflection on and Ray trace off
		SHADING_TRANSPARENT = 9,			///<  Transparency: Glass on, Reflection: Ray trace off
		SHADING_FULLSHADOWS = 10			///<  Casts shadows onto invisible surfaces
	};

	rt::Vec3f	Ambient;		///< Ka
	rt::Vec3f	Diffuse;		///< Kd
	rt::Vec3f	Specular;		///< Ks
	float		Shininess;		///< Ns
	float		Opacity;		///< d, Tr

	UINT		AmbientMapIndex;
	UINT		DiffuseMapIndex;
	UINT		SpecularMapIndex;
	UINT		ShininessMapIndex;
	UINT		OpacityMapIndex;
};

struct Material 
{
	DWORD			_MaterialConfig;
	rt::String		_Name;
	int				_MaterialShaderIndex;
	BYTE			_MaterialData[sizeof(MaterialPhong)];

	const MaterialPhong& data_phong() const { return *((const MaterialPhong*)_MaterialData); }
	MaterialPhong& data_phong(){ return *((MaterialPhong*)_MaterialData); }
	
	UINT	GetMaterialType() const { return (_MaterialConfig&MATERIAL_TYPE_MASK)>>MATERIAL_TYPE_SHIFT; }
	UINT	GetRenderOrder() const;
	bool	operator < (const Material& x) const { return GetRenderOrder()<x.GetRenderOrder(); }

	void	Use(ShaderProgramBase& prog);
};

struct MaterialShader
{
	DWORD			_MaterialConfig;
	
	ShaderProgram	_Shader;
};

class Object
{
protected:
#pragma pack(1)
	struct Header
	{	static const int Magic_xScene = 0x4e435358;
		static const int FILENAME_SURFACECHUNK = 10;
		static const int FILENAME_MATERIALCHUNK = 11;
		static const int FILENAME_HEADER = 1;
		DWORD			Magic;
		UINT			SurfaceCount;
		UINT			MaterialCount;
	};
	struct SurfaceChunk
	{	
		DWORD				_DataFlag;
		CHAR				_SurfaceName[128];
		CHAR				_MaterialName[128];
		ULONGLONG			_VertexAttribute;
		ULONGLONG			_Triangles;
		UINT				_TriangleCount;
		UINT				_VertexCount;
	};
	struct MaterialChunk
	{	
		CHAR			_MaterialName[128];
		WORD			_MaterialType;	///< _tagMaterialType
		WORD			_MaterialConfig;
		ULONGLONG		_MaterialData;
	};
#pragma pack()

protected:
	rt::Buffer<Texture2D>			_Maps;
	rt::Buffer<Material>			_Materials;
	rt::Buffer<Surface>				_Surfaces;
	rt::BufferEx<MaterialShader>	_Shaders;

	void	_UpdateRenderingOrder();
	int		_GetShaderIndex(DWORD _MaterialConfig);
	void	_BindPhongShader(Material& material);

public:
	~Object(){ Destory(); }
	void Destory();
	bool Load(LPCSTR filename);
	bool Prepare();	///< call with RC used
	void Render(const gl::Camera& cam);
#if defined(PLATFORM_WIN)
	bool ReloadPhongMaterial(LPCSTR mtl_file, LPCSTR mtl_alias);
	static bool ConvertObjMtlPair(LPCSTR obj_file, LPCSTR save_as, bool normalize_size = true);
#endif
};

/** @}*/
} // namespace gl
/** @}*/