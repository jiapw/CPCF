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

#include "../os/predefines.h"
#include "runtime_base.h"
#include <float.h>
#include <limits.h>
#include <tuple>

namespace rt
{

////////////////////////////////////////////////////
// true if T1 T2 is the same type
template<typename T1, typename T2>
struct IsTypeSame{ static const bool Result = false; };
	template<typename T>
	struct IsTypeSame<T,T>{ static const bool Result = true; };


//////////////////////////////////////////////////
// Remove reference
template<class T>
struct Remove_Reference{ typedef T t_Result; };
	template<class T>
	struct Remove_Reference<T&>{ typedef T t_Result; };
	template<class T>
	struct Remove_Reference<T&&>{ typedef T t_Result; };

//////////////////////////////////////////////////
// Remove qualifiers
template<class T>
struct Remove_Qualifer{ typedef T t_Result; };
	template<class T>
	struct Remove_Qualifer<const T>{ typedef T t_Result; };
	template<class T>
	struct Remove_Qualifer<volatile T>{ typedef T t_Result; };

//////////////////////////////////////////////////
// Remove qualifiers and reference
template<class T>
class Remove_QualiferAndRef
{protected:	typedef typename Remove_Reference<T>::t_Result		_NoRef;
 public:	typedef typename Remove_Qualifer<_NoRef>::t_Result	t_Result;
};
    
} // namespace rt


///////////////////////////////////////////////////////////
// Compiler intrinsics
#if defined(_MSC_VER) && defined(_MSC_FULL_VER) && (_MSC_FULL_VER >=140050215)
#define COMPILER_INTRINSICS_IS_POD(T)	__is_pod(T)
//true if the type is a class or union with no constructor or private or protected
//non-static members, no base classes, and no virtual functions
#define COMPILER_INTRINSICS_IS_CONVERTIBLE(from,to)		__is_convertible_to(from, to)
//true if the first type can be converted to the second type
#define COMPILER_INTRINSICS_IS_BASE_OF(base,derived)	__is_base_of(base,derived)
//true if 'derived' type inherits 'base' type
#else
#define COMPILER_INTRINSICS_IS_POD(T)	__is_pod(T)
#define COMPILER_INTRINSICS_IS_CONVERTIBLE(from,to)		(rt::_details::_IsConvertible<from, to>::Result)
#define COMPILER_INTRINSICS_IS_BASE_OF(base,derived)	(rt::_details::_IsBaseOf<base, derived>::Result)

namespace rt
{	namespace _details
    {	////////////////////////////////////////////////////
        // true if convertible
        template<typename t_From, typename t_To>
        class _IsConvertible // t_From is assumed that can be used as function return type
        {	class t_big{ char __no_name[2]; };
            typedef char t_small;
#ifdef __INTEL_COMPILER
#	pragma warning(disable: 1595) //warning by intel compiler
#endif //warning #1595: non-POD (Plain Old Data) class type passed through ellipsis
            static t_small	MatchType(t_To);
            static t_big	MatchType(...);
            static t_From	from_obj;
        public:
            static const bool Result = sizeof(MatchType(from_obj))==sizeof(t_small) ;
#ifdef __INTEL_COMPILER
#	pragma warning(default: 1684) //warning by intel compiler
#endif //warning #1595: non-POD (Plain Old Data) class type passed through ellipsis
        };
        
        template<typename t_Base, typename t_Derived>
        class _IsBaseOf
        {	typedef typename rt::Remove_QualiferAndRef<t_Base>::t_Result t_BaseRaw;
            typedef typename rt::Remove_QualiferAndRef<t_Derived>::t_Result t_DerivedRaw;
        public:
            static const int Result =	rt::_details::_IsConvertible<const t_DerivedRaw*,const t_BaseRaw* >::Result && //pointer can convert
            (!rt::IsTypeSame<t_BaseRaw,t_DerivedRaw >::Result) &&				 //not the same type
            (!rt::IsTypeSame<const t_Base*,const void * >::Result);			 //t_BaseRaw is not void
        };
    } //namespace _meta_
} //namespace rt
#endif

///////////////////////////////////////////////////
// UnitTests Class in Global Scope
// Allows access to protected/private members in Unit Test
namespace rt
{

#define TYPETRAITS_UNITTEST_OPEN_ACCESS	friend struct rt::UnitTests;
#define TYPETRAITS_UNITTEST(x)	{ _LOG("=== UnitTest<"<<#x<<">: BEGIN ==="); rt::UnitTests::x(); _LOG("=== UnitTest<"<<#x<<">: END =====\n"); _CheckHeap; }
struct UnitTests;

}


namespace rt
{

//////////////////////////////////////////////////////////////
// Qualifiers that can not be nest specified (const/volatile/&)
static const int _typeid_expression		=-5;	// classes for expressions in expr_templ.h
static const int _typeid_virtual_array	=-4;	// class for simulate array from scalar in expr_templ.h
static const int _typeid_volatile		=-3;
static const int _typeid_const			=-2;	
static const int _typeid_ref			=-1;	

static const int _typeid_class			=0;
static const int _typeid_void			=1;

static const int _typeid_bool			=2;		//like unsigned char
static const int _typeid_8s				=3;
static const int _typeid_8u				=4;
static const int _typeid_16s			=5;
static const int _typeid_16u			=6;
static const int _typeid_32s			=7;
static const int _typeid_32u			=8;
static const int _typeid_64s			=9;
static const int _typeid_64u			=10;

static const int _typeid_32f			=11;
static const int _typeid_64f			=12;

static const int _typeid_unknown		=0xff;

// composited types
static const int _typeid_pointer		=0x100;	// C pointer
static const int _typeid_array			=0x200;	// C array
static const int _typeid_vec			=0x300;	// small vec, fixed size
static const int _typeid_buffer			=0x400;	// rt::Buffer/BufferEx
static const int _typeid_string			=0x500;	// rt::String/String_Ref

namespace _details
{
    #define __TypeTraitsSubstituteType(varname, t_default)						\
	template <typename T>														\
	class _SubstituteType_t_ ## varname											\
	{	template<typename V>													\
		struct _Result{ typedef V t_Result; };									\
		template<typename C> static auto __Test(void*) -> decltype(C::__Type ## varname(), _Result<typename C::__Type ## varname>());	\
		template<typename> static _Result<t_default> __Test(...);				\
		typedef decltype(__Test<T>(0)) __RetType;								\
	public:	typedef typename __RetType::t_Result t_Result;						\
	};
 
    
	__TypeTraitsSubstituteType(Val, T)
	__TypeTraitsSubstituteType(Element, void)
	__TypeTraitsSubstituteType(Signed, T)
	__TypeTraitsSubstituteType(Unsigned, T)

#undef __TypeTraitsSubstituteType

#define __TypeTraitsSubstituteConst(varname, valtype, t_default)						\
	template <typename T>																\
	class _SubstituteConst_ ## varname													\
	{	template<valtype v>																\
		struct _Result{ static const valtype Result = v; };								\
		template<typename C> static auto __Test(void*) -> _Result<C::__ ## varname>;	\
		template<typename> static _Result<t_default> __Test(...);						\
	public:	static const valtype Result = decltype(__Test<T>(0))::Result;				\
	};																					\

	__TypeTraitsSubstituteConst(Typeid, int, _typeid_unknown)
	__TypeTraitsSubstituteConst(IsPOD, bool, COMPILER_INTRINSICS_IS_POD(T))
	__TypeTraitsSubstituteConst(IsNumeric, bool, false)

#undef __TypeTraitsSubstituteConst
}

#define TYPETRAITS_DECLARE_NON_POD		public: static const bool __IsPOD = false;
#define TYPETRAITS_DECLARE_POD			public: static const bool __IsPOD = true;
#define TYPETRAITS_DECLARE_VARSIZE_POD	public: static const bool __IsVarSizePOD = true;

////////////////////////////////////////////////////////////////////////////
// TypeTraits
template<typename T>
class TypeTraits
{
public:
	typedef typename _details::_SubstituteType_t_Val<T>::t_Result       t_Val;
	typedef typename _details::_SubstituteType_t_Element<T>::t_Result	t_Element;
	typedef typename _details::_SubstituteType_t_Signed<T>::t_Result	t_Signed;
	typedef typename _details::_SubstituteType_t_Unsigned<T>::t_Result	t_Unsigned;

	static const int Typeid			= _details::_SubstituteConst_Typeid<T>::Result;
	static const bool IsPOD			= _details::_SubstituteConst_IsPOD<T>::Result;
	static const bool IsNumeric		= _details::_SubstituteConst_IsNumeric<T>::Result;
};

// make sure it is declared in namespace rt
#define TYPETRAITS_DEFINE(T_,Ele_,is_pod,is_num)			\
	template<>												\
	class TypeTraits<T_>									\
	{														\
	public:													\
		typedef T_		t_Val;								\
		typedef Ele_	t_Element;							\
		typedef T_		t_Signed;							\
		typedef T_		t_Unsigned;							\
		static const int Typeid		= _typeid_unknown;		\
		static const bool IsPOD		= is_pod;				\
		static const bool IsNumeric	= is_num;				\
	};														\


// void type
template<>
class TypeTraits<void>
{
public:	
	typedef void t_Val;
	typedef void t_Element;
	typedef void t_Signed;
	typedef void t_Unsigned;
	static const int Typeid = _typeid_void ;
	static const bool IsPOD = false ;
	static const bool IsNumeric = false ;
};
    

// const qualifier
template<typename t_RawType>
class TypeTraits< const t_RawType >
{
public:
	typedef t_RawType	t_Nested;
	typedef typename TypeTraits<t_Nested>::t_Val		t_Val;
	typedef typename TypeTraits<t_Nested>::t_Element	t_Element;
	typedef typename TypeTraits<t_Nested>::t_Signed		t_Signed;
	typedef typename TypeTraits<t_Nested>::t_Unsigned	t_Unsigned;
	static const int Typeid = _typeid_const ;
	static const bool IsPOD = TypeTraits<t_Nested>::IsPOD;
	static const bool IsNumeric = TypeTraits<t_Nested>::IsNumeric;
};
// volatile qualifier
template<typename t_RawType>
class TypeTraits< volatile t_RawType >
{	
public:
	typedef t_RawType		t_Nested;
	typedef typename TypeTraits<t_Nested>::t_Val		t_Val;
	typedef typename TypeTraits<t_Nested>::t_Element	t_Element;
	typedef typename TypeTraits<t_Nested>::t_Signed		t_Signed;
	typedef typename TypeTraits<t_Nested>::t_Unsigned	t_Unsigned;
	static const int Typeid = _typeid_volatile ;
	static const bool IsPOD = TypeTraits<t_Nested>::IsPOD;
	static const bool IsNumeric = TypeTraits<t_Nested>::IsNumeric;
};

    
// reference
template<typename t_RawType>
class TypeTraits< t_RawType& >
{	typedef t_RawType T;
public:
	typedef t_RawType		t_Nested;
	typedef typename TypeTraits<t_Nested>::t_Val		t_Val;
	typedef typename TypeTraits<t_Nested>::t_Element	t_Element;
	typedef typename TypeTraits<t_Nested>::t_Signed		t_Signed;
	typedef typename TypeTraits<t_Nested>::t_Unsigned	t_Unsigned;
	static const int Typeid = _typeid_ref ;
	static const bool IsPOD = TypeTraits<t_Nested>::IsPOD;
	static const bool IsNumeric = TypeTraits<t_Nested>::IsNumeric;
};

    
// C array
template<typename t_Ele, size_t t_Size>
class TypeTraits< t_Ele[t_Size] >
{	typedef t_Ele T[t_Size];
public:
	typedef typename TypeTraits<t_Ele>::t_Val		t_Val[t_Size];
	typedef t_Ele									t_Element;
	typedef typename TypeTraits<t_Ele>::t_Signed	t_Signed[t_Size];
	typedef typename TypeTraits<t_Ele>::t_Unsigned	t_Unsigned[t_Size];
	static const int Typeid = _typeid_array ;
	static const bool IsPOD = TypeTraits<t_Ele>::IsPOD;
	static const bool IsNumeric = false ;	
};
// C pointer
template<typename t_Ele>
class TypeTraits< t_Ele* >	//All pointers
{	typedef t_Ele *T;
public:
	typedef t_Ele*				t_Accum;
	typedef t_Ele*				t_Val;
	typedef t_Ele				t_Element;
	//typedef typename TypeTraits<t_Ele>::t_Signed*	t_Signed;			// t_Ele might be a incomplete type
	//typedef typename TypeTraits<t_Ele>::t_Unsigned*	t_Unsigned;		// t_Ele might be a incomplete type
	static const int Typeid = _typeid_pointer;
	static const bool IsPOD = true ;
	static const bool IsNumeric = false ;	
	static const bool IsArray = false ;
	static const bool IsImage = false ;
};



/////////////////////////////////////////////////////////////////
//all built-in num types
#define __NumT(T_,Val_,Accum_,Signed_,Unsigned_,TypeId_,Eps_,Min_,Max_,Unit_,tos_tw,tos_wap)	\
		template<>																\
		class TypeTraits<T_>													\
		{public:	typedef Accum_		t_Accum;								\
					typedef Val_		t_Val;									\
					typedef void		t_Element;								\
					typedef Signed_		t_Signed;								\
					typedef Unsigned_	t_Unsigned;								\
					static const int	Typeid = TypeId_ ;						\
					static const bool	IsPOD = true ;							\
					static const bool	IsNumeric = true ;						\
					template<class ostream>										\
					static void TypeName(ostream & outstr){ outstr<<(#T_); }	\
		 public:	FORCEINL static constexpr T_	Epsilon(){ return Eps_; }	\
					FORCEINL static constexpr T_	MinVal(){ return Min_; }	\
					FORCEINL static constexpr T_	MaxVal(){ return Max_; }	\
					FORCEINL static constexpr T_	UnitVal(){ return Unit_; }	\
		};																		\

//all integer types
#define __NumInt(T_,Val_,Accum_,Signed_,Unsigned_,TypeId_,Min_,Max_,Unit_,tos_tw)	\
		__NumT(T_,Val_,Accum_,Signed_,Unsigned_,TypeId_,1,Min_,Max_,Unit_,tos_tw,0)
//      |Type			 |Value T		  |Accum T		   |Signed Type		|Unsigned Type		|Type Id		|Min		|Max		|Unit			|tot|
#ifdef _CHAR_UNSIGNED																												
__NumInt(char			 ,int			  ,int			   ,signed char		,unsigned char		,_typeid_8u	  	,0			,UCHAR_MAX	,UCHAR_MAX		,3	)
#else				
__NumInt(char			 ,int			  ,int			   ,signed char		,unsigned char		,_typeid_8s	  	,SCHAR_MIN	,SCHAR_MAX	,SCHAR_MAX		,4	)
#endif																																					
__NumInt(signed char	 ,int			  ,int			   ,signed char		,unsigned char		,_typeid_8s	  	,SCHAR_MIN	,SCHAR_MAX	,SCHAR_MAX		,4	)	
__NumInt(short			 ,int			  ,int			   ,signed short	,unsigned short		,_typeid_16s	,SHRT_MIN	,SHRT_MAX	,SHRT_MAX		,6	)	
__NumInt(int			 ,int			  ,__int64		   ,signed int		,unsigned int		,_typeid_32s	,INT_MIN	,INT_MAX	,SHRT_MAX		,6	)	
__NumInt(bool			 ,int			  ,int			   ,bool			,bool				,_typeid_bool	,0			,1			,1				,1	)	
__NumInt(unsigned char	 ,int			  ,int			   ,signed char		,unsigned char		,_typeid_8u	  	,0			,UCHAR_MAX	,UCHAR_MAX		,3	)	
__NumInt(unsigned short	 ,int			  ,int			   ,signed short	,unsigned short		,_typeid_16u	,0			,USHRT_MAX	,SHRT_MAX		,5	)	
__NumInt(unsigned long	 ,unsigned long	  ,__uint64		   ,signed long		,unsigned long		,_typeid_32u	,0			,ULONG_MAX	,SHRT_MAX		,5	)	
__NumInt(long			 ,long			  ,__int64		   ,signed long		,unsigned long		,_typeid_32s	,LONG_MIN	,LONG_MAX	,SHRT_MAX		,6	)	
__NumInt(unsigned int	 ,unsigned int	  ,__uint64		   ,signed int		,unsigned int		,_typeid_32u	,0			,UINT_MAX	,SHRT_MAX		,5	)
#if !defined(PLATFORM_LINUX)
__NumInt(__int64		 ,__int64		  ,__int64		   ,__int64         ,__uint64           ,_typeid_64s	,LLONG_MIN	,LLONG_MAX	,SHRT_MAX		,8	)
__NumInt(__uint64		 ,__uint64		  ,__uint64		   ,__int64			,__uint64           ,_typeid_64u	,0			,ULLONG_MAX	,SHRT_MAX		,7	)
#else
__NumInt(LONGLONG		 ,LONGLONG		  ,LONGLONG		   ,LONGLONG        ,ULONGLONG          ,_typeid_64s	,LLONG_MIN	,LLONG_MAX	,SHRT_MAX		,8	)
__NumInt(ULONGLONG		 ,ULONGLONG		  ,ULONGLONG	   ,LONGLONG		,ULONGLONG          ,_typeid_64u	,0			,ULLONG_MAX	,SHRT_MAX		,7	)
#endif
#undef __NumInt

//all float-point types
#define __NumFloat(T_,Accum_,TypeId_,Eps_,Min_,Max_,tos_tw,tos_wap)			\
		__NumT(T_,T_,Accum_,T_,T_,TypeId_,Eps_,Min_,Max_,1,tos_tw,tos_wap)
//		  |Type		    |Accum T	|Type Id		|EPSILON					|Min		|Max		|tot|wap|
__NumFloat(float		,double		,_typeid_32f	,(float)(FLT_EPSILON*10)	,-FLT_MAX	,FLT_MAX	,7	,4	)
__NumFloat(double		,long double,_typeid_64f	,DBL_EPSILON*100			,-DBL_MAX	,DBL_MAX	,9	,6	)
__NumFloat(long double	,long double,_typeid_64f	,LDBL_EPSILON*100			,-LDBL_MAX	,LDBL_MAX	,9	,6	)
#undef __NumFloat

#undef __NumT
//all built-in num types
/////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////
// Trait num type properties
template<typename T_in>
struct NumericTraits
{	typedef typename rt::Remove_QualiferAndRef<T_in>::t_Result T;
	static const int _Typeid = (int)rt::TypeTraits<T>::Typeid ;
	static const bool IsInteger = (_Typeid>=_typeid_bool)&&(_Typeid<=_typeid_64u) ;
	static const bool IsFloat = (_Typeid==_typeid_32f)||(_Typeid==_typeid_64f) ;
	static const bool IsUnsigned = _Typeid==_typeid_8u || _Typeid==_typeid_16u || _Typeid==_typeid_32u || _Typeid==_typeid_64u;
	static const bool IsSigned = _Typeid==_typeid_8s || _Typeid==_typeid_16s || _Typeid==_typeid_32s || _Typeid==_typeid_64s || _Typeid==_typeid_32f || _Typeid==_typeid_64f;
	static const bool IsBuiltInNumeric = (_Typeid>=_typeid_bool)&&(_Typeid<=_typeid_64f) ;
	// non-num is rated as 0xffff
};

template<int len>
struct IsZeroBits
{	static bool IsZero(LPCBYTE p)
	{	return 0 == *(size_t*)p && IsZeroBits<len - sizeof(size_t)>::IsZero(p + sizeof(size_t));
	}
};
	template<> struct IsZeroBits<1>{ static bool IsZero(LPCBYTE p){ return 0 == *p;							    		  } };
	template<> struct IsZeroBits<2>{ static bool IsZero(LPCBYTE p){ return 0 == *(WORD*)p;								  } };
	template<> struct IsZeroBits<3>{ static bool IsZero(LPCBYTE p){ return 0 == *(WORD*)p  && p[2] == 0;				  } };
	template<> struct IsZeroBits<4>{ static bool IsZero(LPCBYTE p){ return 0 == *(DWORD*)p;								  } };
	template<> struct IsZeroBits<5>{ static bool IsZero(LPCBYTE p){ return 0 == *(DWORD*)p && IsZeroBits<1>::IsZero(p+4); } };
	template<> struct IsZeroBits<6>{ static bool IsZero(LPCBYTE p){ return 0 == *(DWORD*)p && IsZeroBits<2>::IsZero(p+4); } };
	template<> struct IsZeroBits<7>{ static bool IsZero(LPCBYTE p){ return 0 == *(DWORD*)p && IsZeroBits<3>::IsZero(p+4); } };
	template<> struct IsZeroBits<8>{ static bool IsZero(LPCBYTE p)
	{
		return 0 == *(size_t*)p; 							  
	} };
	
namespace _details
{
	template<typename T, bool is_int = NumericTraits<T>::IsInteger, bool is_float = NumericTraits<T>::IsFloat>
	struct _IsZero
	{	static bool Is(const T& v){ return IsZeroBits<sizeof(T)>::IsZero((LPCBYTE)&v); }
	};
		template<typename T> struct _IsZero<T, true, false>{ static bool Is(T v){ return v == 0; } };
		template<typename T> struct _IsZero<T, false, true>{ static bool Is(T v){ return v<TypeTraits<T>::Epsilon() && v>-TypeTraits<T>::Epsilon(); } };
}

template<typename T>
FORCEINL bool IsZero(T v)
{
	return _details::_IsZero<T>::Is(v);
}


///////////////////////////////////////////////////////////
// Call bool/void Lambda function
namespace _details
{
template<typename default_type, typename return_type>
struct _CallLambda  // example: return rt::_details::_CallLambda<bool, decltype(func(arg))>(true, func, arg).retval;
{	return_type retval;
	template<typename function_type, typename... arg_types>
	_CallLambda(default_type,function_type&& func, arg_types&&... args)
	{	retval = func(std::forward<arg_types>(args)...);
	}
};
	template<typename default_type> struct _CallLambda<default_type, void>
	{	default_type retval;
		template<typename function_type, typename... arg_types>
		_CallLambda(default_type defval, function_type&& func, arg_types&&... args)
		{	func(std::forward<arg_types>(args)...);
			retval =  defval;
		}
	};
} // _details

template<typename return_type, typename func_type, typename... arg_types>
INLFUNC auto CallLambda(return_type _def_ret_val, func_type&& func, arg_types&&... args)
{
	return rt::_details::_CallLambda<return_type, decltype(func(std::forward<arg_types>(args)...))>(_def_ret_val, func, std::forward<arg_types>(args)...).retval;
}

////////////////////////////////////////////////////////
// Data traits
namespace _details
{
struct __PodRequired {};
template<typename pod_required, typename T, bool is_pod = rt::TypeTraits<T>::IsPOD>
struct _PodAssert {};
	template<typename T, bool is_pod>
	struct _PodAssert<__PodRequired, T, is_pod>
	{	_PodAssert(){ static_assert(is_pod, "Plain-old-data required"); }	};	// T should be a POD

struct _GetDataPtr
{	LPVOID	_p;
	template<typename T> auto _Begin(T* x) -> decltype(x->Begin()) { _p = (LPVOID)x->Begin(); return nullptr; }
	template<typename T> auto _Begin(const T* x) -> decltype(x->begin()) { _p = (LPVOID)x->begin(); return nullptr; }
	template<typename T> auto _Begin(const T* x) -> decltype(x->__IsVarSizePOD) { _p = (LPVOID)x; return nullptr; }
	template<typename T> auto _Begin(T* x) -> decltype(x->c_str()) { _p = (LPVOID)x->c_str(); return nullptr; }
				__PodRequired _Begin(...){ return __PodRequired(); }
	template<typename T>	  _GetDataPtr(T& x){ _p = (LPVOID)&x; }
};

struct _GetDataSize
{	SIZE_T	_s;
	SIZE_T	_s_ele;
	template<typename T> auto _SizeEle(T* x) -> decltype(sizeof((*x)[0])) { _s_ele = sizeof((*x)[0]); return 0; }
						 void _SizeEle(...){ _s_ele = 1; }

	template<typename T> auto _Size(T* x) -> decltype(x->Size()) { _s = x->Size(); return 0; }
	template<typename T> auto _Size(T* x) -> decltype(x->GetSize()) { _s = x->GetSize(); return 0; }
	template<typename T> auto _Size(T* x) -> decltype(x->size()) { _s = x->size(); return 0; }
	template<typename T> auto _Size(T* x) -> decltype(x->GetLength()) { _s = x->GetLength(); return 0; }
				__PodRequired _Size(...){ _s /= _s_ele; return __PodRequired(); }
	template<typename T>	  _GetDataSize(T& x){ _s = sizeof(x); }
};
} // namespace _details


#pragma pack(push, 1)
template<typename T>
struct ByteOrderSwapped
{	static_assert(TypeTraits<T>::IsPOD, "T must be POD");
	TYPETRAITS_DECLARE_POD;
	T	v;
	T operator = (const T a){ v = SwapByteOrder(a); return a; }
	operator T() const { return SwapByteOrder(v); }
};
#pragma pack(pop)

template<typename T>
INLFUNC LPVOID GetDataPtr(T& x)
{	_details::_GetDataPtr p(x);
	auto r = p._Begin(rt::_CastToNonconst(&x));
	_details::_PodAssert<decltype(r), T> _a;	_a = _a;
	return p._p; 
}

template<typename T>
INLFUNC SIZE_T GetDataSize(T& x)
{	_details::_GetDataSize s(x);
	s._SizeEle(&x);
	auto r = s._Size(&x);
	_details::_PodAssert<decltype(r), T> _a;	_a = _a;
	return s._s * s._s_ele;
}

////////////////////////////////////////////////////////
// Function traits
namespace _details
{

template <typename _ReturnType, typename... _Args>
struct FunctionTraitsDefs
{
    static constexpr size_t Arity = sizeof...(_Args);
    using ReturnType = _ReturnType;
    template <size_t i> struct Arg
    {
        using Type = typename std::tuple_element<i, std::tuple<_Args...>>::type;
    };
};

template<typename T>
struct FunctionTraitsImpl: public FunctionTraitsImpl<decltype(&T::operator())>
{};
	template <typename T> struct FunctionTraitsImpl<T&> : public FunctionTraitsImpl<T> {};
	template <typename T> struct FunctionTraitsImpl<const T&> : public FunctionTraitsImpl<T> {};
	template <typename T> struct FunctionTraitsImpl<volatile T&> : public FunctionTraitsImpl<T> {};
	template <typename T> struct FunctionTraitsImpl<const volatile T&> : public FunctionTraitsImpl<T> {};
	template <typename T> struct FunctionTraitsImpl<T&&> : public FunctionTraitsImpl<T> {};
	template <typename T> struct FunctionTraitsImpl<const T&&> : public FunctionTraitsImpl<T> {};
	template <typename T> struct FunctionTraitsImpl<volatile T&&> : public FunctionTraitsImpl<T> {};
	template <typename T> struct FunctionTraitsImpl<const volatile T&&> : public FunctionTraitsImpl<T> {};

	template <typename ReturnType, typename... Args>
		struct FunctionTraitsImpl<ReturnType(Args...)> : _details::FunctionTraitsDefs<ReturnType, Args...> {};
	template <typename ReturnType, typename... Args>
		struct FunctionTraitsImpl<ReturnType(&)(Args...)> : _details::FunctionTraitsDefs<ReturnType, Args...> {};
	template <typename ReturnType, typename... Args>
		struct FunctionTraitsImpl<ReturnType(&&)(Args...)> : _details::FunctionTraitsDefs<ReturnType, Args...> {};
	template <typename ReturnType, typename... Args>
		struct FunctionTraitsImpl<ReturnType(*)(Args...)> : _details::FunctionTraitsDefs<ReturnType, Args...> {};
	template <typename ClassType, typename ReturnType, typename... Args>
		struct FunctionTraitsImpl<ReturnType(ClassType::*)(Args...)> : _details::FunctionTraitsDefs<ReturnType, Args...> {};
	template <typename ClassType, typename ReturnType, typename... Args>
		struct FunctionTraitsImpl<ReturnType(ClassType::*)(Args...) const> : _details::FunctionTraitsDefs<ReturnType, Args...> {};
	template <typename ClassType, typename ReturnType, typename... Args>
		struct FunctionTraitsImpl<ReturnType(ClassType::*)(Args...) volatile> : _details::FunctionTraitsDefs<ReturnType, Args...> {};
	template <typename ClassType, typename ReturnType, typename... Args>
		struct FunctionTraitsImpl<ReturnType(ClassType::*)(Args...) const volatile> : _details::FunctionTraitsDefs<ReturnType, Args...> {};

} // _details

template <typename T>
struct FunctionTraits : _details::FunctionTraitsImpl<T> {};


/////////////////////////////////////////////////////
// class-neutral member function invocation
class __ThisCallMemberFunctionPointer
{
	typedef void (__thiscall __ThisCallMemberFunctionPointer::* __foo)();
	size_t	Data[sizeof(__foo)/sizeof(size_t)];
public:
	template<typename T>
	__ThisCallMemberFunctionPointer(const T& x){ ASSERT(sizeof(T) == sizeof(Data)); rt::Copy(Data, x); }
	__ThisCallMemberFunctionPointer(){ Zero(); }
	__ThisCallMemberFunctionPointer(decltype(nullptr) x){ ASSERT(x == nullptr); Zero(); }
	__ThisCallMemberFunctionPointer(const __ThisCallMemberFunctionPointer&x) = default;
	bool	IsNull() const { return rt::IsZero(*this); }
	void	Zero(){ rt::Zero(*this); }
	operator bool() const { return !IsNull(); }
};

namespace _details
{

template<class ThisCallPoly, bool is_void = rt::TypeTraits<typename ThisCallPoly::ReturnType>::Typeid == rt::_typeid_void>
struct _InvokeThisCall
{	template<typename... arg_types>
	static typename ThisCallPoly::ReturnType Invoke(LPCVOID This, const __ThisCallMemberFunctionPointer& Func, arg_types&&... args)
	{	ASSERT(This);
		return (((ThisCallPoly*)This)->*((typename ThisCallPoly::FUNC_CALL&)Func))(std::forward<arg_types>(args)...);
	}
};
	template<class ThisCallPoly>
	struct _InvokeThisCall<ThisCallPoly, true>
	{	template<typename... arg_types>
		static void Invoke(LPCVOID This, const __ThisCallMemberFunctionPointer& Func, arg_types&&... args)
		{	ASSERT(This);
			(((ThisCallPoly*)This)->*((typename ThisCallPoly::FUNC_CALL&)Func))(std::forward<arg_types>(args)...);
		}
	};

} // namespace _details

#define THISCALL_MFPTR rt::__ThisCallMemberFunctionPointer
#define THISCALL_POLYMORPHISM_DECLARE_(return_type, name, ...)												\
			struct __ThisCallPolymorphism_ ## name															\
			{	typedef return_type ReturnType;																\
				typedef ReturnType (__thiscall __ThisCallPolymorphism_ ## name::* FUNC_CALL)(__VA_ARGS__);	\
				ReturnType ThisCallFunction(__VA_ARGS__);

#define THISCALL_POLYMORPHISM_DECLARE(return_type, default_return, name, ...)								\
			THISCALL_POLYMORPHISM_DECLARE_(return_type, name, __VA_ARGS__)									\
				template<typename... arg_types>																\
				static ReturnType Invoke(LPCVOID This, const THISCALL_MFPTR& Func, arg_types&&... args)		\
				{ return This?rt::_details::_InvokeThisCall<__ThisCallPolymorphism_ ## name>::Invoke(This, Func, args...):default_return; }}

#define THISCALL_POLYMORPHISM_DECLARE_VOID(name, ...)														\
			THISCALL_POLYMORPHISM_DECLARE_(void, name, __VA_ARGS__)											\
				template<typename... arg_types>																\
				static void Invoke(LPCVOID This, const THISCALL_MFPTR& Func, arg_types&&... args)			\
				{ if(This)rt::_details::_InvokeThisCall<__ThisCallPolymorphism_ ## name>::Invoke(This, Func, args...); }}

#define THISCALL_POLYMORPHISM_INVOKE(name, This, Func, ...)	(__ThisCallPolymorphism_ ## name::Invoke(This, Func, ##__VA_ARGS__))

} // namespace rt


