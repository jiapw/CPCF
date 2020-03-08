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
//     * Neither the name of CPF.  nor the names of its
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

#include "../../core/os/predefines.h"
#include "../../core/rt/runtime_base.h"
#include "../../core/rt/buffer_type.h"
#include "../../core/os/kernel.h"

#include "inc\mkl_cpp.h"

typedef rt::TypeTraits<MKL_INT>::t_Unsigned		MKL_SIZE;
typedef rt::TypeTraits<MKL_INT>::t_Signed		MKL_SSIZE;

#if defined(PLATFORM_WIN)

#if defined(PLATFORM_DEBUG_BUILD)
#pragma comment(linker, "/NODEFAULTLIB:libcpmt.lib")
#endif

////////////////////////////////////////////////////////////////
// http://software.intel.com/sites/products/mkl/MKL_Link_Line_Advisor.html

//#define MKL_USE_OPENMP_DLL  
//requires libiomp5md.dll or https://software.intel.com/en-us/articles/redistributables-for-intel-parallel-studio-xe-2016-composer-edition-for-windows

#ifdef _WIN64
	#pragma comment(lib, "mkl_x64_core.lib")
	#ifdef MKL_ILP64
		#pragma comment(lib, "mkl_x64_intel_ilp64.lib")
	#else
		#pragma comment(lib, "mkl_x64_intel_lp64.lib")
	#endif
	#ifdef MKL_USE_OPENMP_DLL
		#pragma comment(lib, "mkl_x64_libiomp5md.lib")
		#pragma comment(lib, "mkl_x64_intel_thread.lib")
	#else
		#pragma comment(lib, "mkl_x64_sequential.lib")
	#endif
#else
		#pragma comment(lib, "mkl_w32_intel_c.lib")
		#pragma comment(lib, "mkl_w32_core.lib")
	#ifdef MKL_USE_OPENMP_DLL
		#pragma comment(lib, "mkl_w32_libiomp5md.lib")
		#pragma comment(lib, "mkl_w32_intel_thread.lib")
	#else
		#pragma comment(lib, "mkl_w32_sequential.lib")
	#endif

#endif

#endif // #if defined(PLATFORM_WIN)

#include "../../core/rt/buffer_type.h"

namespace mkl
{

INLFUNC void SetNumberOfThread(int num = -1)
{
	if(num<0)num = os::GetNumberOfProcessors();
	mkl_set_num_threads(num);
	_LOG("MKL Thread Number = "<<mkl_get_max_threads());
}

template<typename t_Val, bool t_NotTransposed>
class Matrix_Ref;
template<typename t_Val>
class Matrix;

namespace _details
{

template<typename t_Val>
class VectorCompact_Ref: public ::rt::Buffer_Ref<t_Val, MKL_SIZE>
{
	static_assert(	rt::TypeTraits< typename ::rt::Remove_Qualifer<t_Val>::t_Result >::Typeid == rt::_typeid_32f || 
					rt::TypeTraits< typename ::rt::Remove_Qualifer<t_Val>::t_Result >::Typeid == rt::_typeid_64f,
					"Only float/double Vector is supported"
	);
private:
	MKL_SIZE	__always_be_one;		// make class memory layout same to Stride Vector
public:
	void		_set_stride(SIZE_T inc){ ASSERT(inc==1); }

public:
	static const MKL_SIZE _inc = 1;
	static const bool IsCompactVector = true;
	typedef VectorCompact_Ref<t_Val>	Ref;

	INLFUNC t_Val		at(MKL_SIZE i) const { return _p[i]; }
	INLFUNC t_Val&		at(MKL_SIZE i){ return _p[i]; }

public:
	VectorCompact_Ref(){ __always_be_one = 1; }
	VectorCompact_Ref(const VectorCompact_Ref& x):Buffer_Ref<t_Val>(x){}

public:
	// MKL VML functions
	//y = 1.0/x
	INLFUNC void vml_Reciprocal(VectorCompact_Ref&y) const
	{ ASSERT(GetSize()==y.GetSize()); ::mkl::mkl_cpp::vmlInv(y.GetSize(),*this,y); }
	INLFUNC void vml_Reciprocal(){ vml_Reciprocal(*this); }
	//y = x^0.5
	INLFUNC void vml_SquareRoot(VectorCompact_Ref&y) const
	{ ASSERT(GetSize()==y.GetSize()); ::mkl::mkl_cpp::vmlSqrt(y.GetSize(),*this,y); }
	INLFUNC void vml_SquareRoot(){ vml_SquareRoot(*this); }
	//y = 1/x^0.5
	INLFUNC void vml_ReciprocalSquareRoot(VectorCompact_Ref&y) const
	{ ASSERT(GetSize()==y.GetSize()); ::mkl::mkl_cpp::vmlInvSqrt(y.GetSize(),*this,y); }
	INLFUNC void vml_ReciprocalSquareRoot(){ vml_ReciprocalSquareRoot(*this); }
	//y = x^0.3333333
	INLFUNC void vml_CubeRoot(VectorCompact_Ref&y) const
	{ ASSERT(GetSize()==y.GetSize()); ::mkl::mkl_cpp::vmlCbrt(y.GetSize(),*this,y); }
	INLFUNC void vml_CubeRoot(){ vml_CubeRoot(*this); }
	//y = 1/x^0.333333333
	INLFUNC void vml_ReciprocalCubeRoot(VectorCompact_Ref&y) const
	{ ASSERT(GetSize()==y.GetSize()); ::mkl::mkl_cpp::vmlInvCbrt(y.GetSize(),*this,y); }
	INLFUNC void vml_ReciprocalCubeRoot(){ vml_ReciprocalCubeRoot(*this); }
	//y = e^x
	INLFUNC void vml_Exponential(VectorCompact_Ref&y) const
	{ ASSERT(GetSize()==y.GetSize()); ::mkl::mkl_cpp::vmlExp(y.GetSize(),*this,y); }
	INLFUNC void vml_Exponential(){ vml_Exponential(*this); }
	//y = ln(x)
	INLFUNC void vml_LogarithmNatural(VectorCompact_Ref&y) const
	{ ASSERT(GetSize()==y.GetSize()); ::mkl::mkl_cpp::vmlLn(y.GetSize(),*this,y); }
	INLFUNC void vml_LogarithmNatural(){ LogarithmNatural(*this); }
	//y = log(x)
	INLFUNC void vml_LogarithmDecimal(VectorCompact_Ref&y) const
	{ ASSERT(GetSize()==y.GetSize()); vmlLog10(y.GetSize(),*this,y); }
	INLFUNC void vml_LogarithmDecimal(){ vml_LogarithmDecimal(*this); }
	//y = 2/(sqrt(Pi))* Int(0,x){ e^(-t^2) } dt
	INLFUNC void vml_ErrorFunc(VectorCompact_Ref&y) const
	{ ASSERT(GetSize()==y.GetSize()); ::mkl::mkl_cpp::vmlErf(y.GetSize(),*this,y); }
	INLFUNC void vml_ErrorFunc(){ vml_ErrorFunc(*this); }
	//y = 1 - 2/(sqrt(Pi))* Int(0,x){ e^(-t^2) } dt
	INLFUNC void vml_ErrorFuncComple(VectorCompact_Ref&y) const
	{ ASSERT(GetSize()==y.GetSize()); ::mkl::mkl_cpp::vmlErfc(y.GetSize(),*this,y); }
	INLFUNC void vml_ErrorFuncComple(){ vml_ErrorFuncComple(*this); }
	//y = sin(x)
	INLFUNC void vml_Sin(VectorCompact_Ref&y) const
	{ ASSERT(GetSize()==y.GetSize()); ::mkl::mkl_cpp::vmlSin(y.GetSize(),*this,y); }
	INLFUNC void vml_Sin(){ Sin(*this); }
	//y = cos(x)
	INLFUNC void vml_Cos(VectorCompact_Ref&y) const
	{ ASSERT(GetSize()==y.GetSize()); ::mkl::mkl_cpp::vmlCos(y.GetSize(),*this,y); }
	INLFUNC void vml_Cos(){ vml_Cos(*this); }
	//y = tan(x)
	INLFUNC void vml_Tan(VectorCompact_Ref&y) const
	{ ASSERT(GetSize()==y.GetSize()); ::mkl::mkl_cpp::vmlTan(y.GetSize(),*this,y); }
	INLFUNC void vml_Tan(){ vml_Tan(*this); }
	//y = sh(x)
	INLFUNC void vml_Sinh(VectorCompact_Ref&y) const
	{ ASSERT(GetSize()==y.GetSize()); ::mkl::mkl_cpp::vmlSinh(y.GetSize(),*this,y); }
	INLFUNC void vml_Sinh(){ vml_Sinh(*this); }
	//y = ch(x)
	INLFUNC void vml_Cosh(VectorCompact_Ref&y) const
	{ ASSERT(GetSize()==y.GetSize()); ::mkl::mkl_cpp::vmlCosh(y.GetSize(),*this,y); }
	INLFUNC void vml_Cosh(){ vml_Cosh(*this); }
	//y = th(x)
	INLFUNC void vml_Tanh(VectorCompact_Ref&y) const
	{ ASSERT(GetSize()==y.GetSize()); ::mkl::mkl_cpp::vmlTanh(y.GetSize(),*this,y); }
	INLFUNC void vml_Tanh(){ vml_Tanh(*this); }
	//y = arcsin(x)
	INLFUNC void vml_ArcSin(VectorCompact_Ref&y) const
	{ ASSERT(GetSize()==y.GetSize()); ::mkl::mkl_cpp::vmlAsin(y.GetSize(),*this,y); }
	INLFUNC void vml_ArcSin(){ vml_ArcSin(*this); }
	//y = arccos(x)
	INLFUNC void vml_ArcCos(VectorCompact_Ref&y) const
	{ ASSERT(GetSize()==y.GetSize()); ::mkl::mkl_cpp::vmlAcos(y.GetSize(),*this,y); }
	INLFUNC void vml_ArcCos(){ vml_ArcCos(*this); }
	//y = arctan(x)
	INLFUNC void vml_ArcTan(VectorCompact_Ref&y) const
	{ ASSERT(GetSize()==y.GetSize()); ::mkl::mkl_cpp::vmlAtan(y.GetSize(),*this,y); }
	INLFUNC void vml_ArcTan(){ vml_ArcTan(*this); }
	//y = arcsh(x)
	INLFUNC void vml_ArcSinh(VectorCompact_Ref&y) const
	{ ASSERT(GetSize()==y.GetSize()); ::mkl::mkl_cpp::vmlAsinh(y.GetSize(),*this,y); }
	INLFUNC void vml_ArcSinh(){ vml_ArcSinh(*this); }
	//y = arcch(x)
	INLFUNC void vml_ArcCosh(VectorCompact_Ref&y) const
	{ ASSERT(GetSize()==y.GetSize()); ::mkl::mkl_cpp::vmlAcosh(y.GetSize(),*this,y); }
	INLFUNC void vml_ArcCosh(){ vml_ArcCosh(*this); }
	//y = arcth(x)
	INLFUNC void vml_ArcTanh(VectorCompact_Ref&y) const
	{ ASSERT(GetSize()==y.GetSize()); ::mkl::mkl_cpp::vmlAtanh(y.GetSize(),*this,y); }
	INLFUNC void vml_ArcTanh(){ vml_ArcTanh(*this); }
	// y = x^b
	INLFUNC void vml_Power(const VectorCompact_Ref&b, VectorCompact_Ref&y) const
	{ ASSERT(GetSize()==y.GetSize()); ASSERT(GetSize()==b.GetSize()); ::mkl::mkl_cpp::vmlPow(y.GetSize(),*this,b,y); }
	INLFUNC void vml_Power(t_Val b,VectorCompact_Ref&y) const
	{ ASSERT(GetSize()==y.GetSize()); ::mkl::mkl_cpp::vmlPowx(y.GetSize(),*this,b,y); }
	INLFUNC void vml_Power(const VectorCompact_Ref&b){ Power(b,*this); }
	INLFUNC void vml_Power(t_Val b){ vml_Power(b,*this); }
	// a = sin(x) b=cos(x)
	INLFUNC void vml_SinCos(VectorCompact_Ref&a, VectorCompact_Ref&b) const
	{ ASSERT(GetSize() == a.GetSize()); ASSERT(GetSize() == b.GetSize()); ::mkl::mkl_cpp::vmlSinCos(GetSize(),*this,a,b); }
	// y = arctan(x/b)
	INLFUNC void vml_ArctanDivision(const VectorCompact_Ref&b, VectorCompact_Ref&y) const
	{ ASSERT(GetSize()==y.GetSize()); ASSERT(GetSize()==b.GetSize()); ::mkl::mkl_cpp::vmlAtan2(y.GetSize(),*this,b,y); }
	// x = x^2
	INLFUNC void vml_Square()
	{ t_Val* p = *this; t_Val* end = &p[GetSize()]; for(;p<end;p++)*p = ::rt::Sqr(*p); }
};

template<typename t_Val>
class VectorStrided_Ref
{
	static_assert(	rt::TypeTraits< typename ::rt::Remove_Qualifer<t_Val>::t_Result >::Typeid == rt::_typeid_32f || 
					rt::TypeTraits< typename ::rt::Remove_Qualifer<t_Val>::t_Result >::Typeid == rt::_typeid_64f,
					"Only float/double Vector is supported"
	);
protected:
	t_Val*		_p;
	MKL_SIZE	_len;
	MKL_SIZE	_inc;
	void	_set_stride(MKL_SIZE inc){ _inc = inc; }
public:
	static const bool IsCompactVector = false;

	INLFUNC VectorStrided_Ref(){ _inc = 1; }
	INLFUNC VectorStrided_Ref(const VectorStrided_Ref& x):Buffer_Ref<t_Val>(x){ _inc = x._inc; }
	
	INLFUNC t_Val		operator [](MKL_SIZE i) const { return _p[_inc*i]; }
	INLFUNC t_Val&		operator [](MKL_SIZE i){ return _p[_inc*i]; }
	INLFUNC t_Val		at(MKL_SIZE i) const { return _p[_inc*i]; }
	INLFUNC t_Val&		at(MKL_SIZE i){ return _p[_inc*i]; }
	INLFUNC MKL_SIZE	GetSize() const { return _len; }
};

} // namespace _details
} // namespace mkl

INLFUNC void vml_LowAccuracy(){ vmlSetMode( VML_LA ); }
INLFUNC void vml_HighAccuracy(){ vmlSetMode( VML_HA ); }
INLFUNC void vml_OptimizeForFloat(){ vmlSetMode(VML_FLOAT_CONSISTENT); }
INLFUNC void vml_OptimizeForDouble(){ vmlSetMode(VML_DOUBLE_CONSISTENT); }

//namespace _meta_
//{

//////////////////////////////////////////////////////////////////////
//Vector reference type for all linear vectors
//template<typename t_Val>
//class _BaseStrideVecRefRoot
//{
//protected:
//	INLFUNC _BaseStrideVecRefRoot(const _BaseStrideVecRefRoot &x){ ASSERT_STATIC(0); }
//	INLFUNC _BaseStrideVecRefRoot(){}
//public:
//	INLFUNC bool SetSize(UINT co=0){ return (co==_len)?true:false; }
//	INLFUNC bool ChangeSize(UINT new_size){ if(_len >= new_size){ _len=new_size; return true;} return false; }
//	static const bool IsRef = true;
//	typedef UINT t_SizeType;
//	//TypeTraits
//	TYPETRAITS_DECL_LENGTH(TYPETRAITS_SIZE_UNKNOWN)	
//	TYPETRAITS_DECL_IS_AGGREGATE(false)
//
//	template<typename T>
//	INLFUNC t_Val & At(const T index){ return _p[(int)index*_inc]; }
//	template<typename T>
//	INLFUNC const t_Val & At(const T index)const { return _p[(int)index*_inc]; }
//	INLFUNC UINT GetSize() const{ return _len; }
//	static const bool IsCompactLinear = false;
//protected:
//	t_Val*	_p;
//	UINT	_len;
//	UINT	_inc;
//};
//class _BaseStrideVecRef
//{public:	template<typename t_Ele> class SpecifyItemType
//			{public: typedef ::mkl::_meta_::_BaseStrideVecRefRoot<t_Ele> t_BaseVec; };
//			static const bool IsCompactLinear = false;
//};
//}// namespace _meta_
//
//#define MKL_VEC_ASSIGN_OPERATOR(clsname)																\
//		ASSIGN_OPERATOR_VEC(clsname)																	\
//		template<typename t_BaseVec2>																	\
//		INLFUNC const clsname& operator = (const ::mkl::_meta_::CVector_Base<t_Val,t_BaseVec2>& x)	\
//		{ ASSERT(GetSize()==x.GetSize()); CopyFrom(x); return *this; }									\
										
namespace mkl
{

template<typename t_Val, class Base_Ref = _details::VectorStrided_Ref<t_Val> >
class Vector_Ref: public Base_Ref
{
public:
	INLFUNC MKL_SIZE GetStride() const{ return _inc; }

public:
	INLFUNC void  Flip(){ MKL_SIZE c=_len/2; for(MKL_SIZE i=0;i<c;i++)rt::Swap(at(i), at(_len-i-1)); }
	INLFUNC t_Val Sum() const { t_Val s=0; for(MKL_SIZE i=0;i<_len;i++)s+=at(i); return s; }
	INLFUNC t_Val SumSqrt() const { t_Val s=0; for(MKL_SIZE i=0;i<_len;i++)s+=sqrt(at(i)); return s; }
	// BLAS Level 1
	// |P1|+|P2|+.....+|Pn|
	INLFUNC t_Val SumAbs() const{ return mkl_asum(_len,_p,_inc); }
	// Pi += scale*Xi
	template<class t_BaseVec2>
	INLFUNC void  Add_Scaled(t_Val scale,const Vector_Ref<t_Val,t_BaseVec2> &x)
	{ ASSERT(GetSize() == x.GetSize());	::mkl::mkl_cpp::mkl_axpy(_len,scale,x.Begin(),x.GetStride(),_p,_inc); }
	// P1*X1+P2*X2+...Pn*Xn
	template<class t_BaseVec2>
	INLFUNC t_Val Dot(const Vector_Ref<t_Val,t_BaseVec2>& x) const
	{ ASSERT(GetSize() == x.GetSize()); return ::mkl::mkl_cpp::mkl_dot(_len,x.Begin(),x.GetStride(),_p,_inc); }
	template<class t_BaseVec2>
	INLFUNC t_Val L2Norm_Sqr(const Vector_Ref<t_Val,t_BaseVec2>& x) const
	{	ASSERT(GetSize() == x.GetSize()); t_Val acc = 0;
		for(MKL_SIZE i=0;i<GetSize();i++)acc += rt::Sqr((*this)[i] - x[i]);
		return acc;
	}
	template<class t_BaseVec2>
	INLFUNC t_Val L2Norm(const Vector_Ref<t_Val,t_BaseVec2>& x) const { sqrt(L2Norm_Sqr(x)); }
	// P1*P1+P2*P2+...Pn*Pn
	INLFUNC t_Val L2Norm_Sqr() const{ return ::mkl::mkl_cpp::mkl_dot(_len,_p,_inc,_p,_inc); }
	// || P ||
	INLFUNC t_Val L2Norm() const{ return ::mkl::mkl_cpp::mkl_nrm2(_len,_p,_inc); }
	// let || P || = 1
	INLFUNC void Normalize(){ t_Val invl2Norm = 1/L2Norm(); *this *= invl2Norm; }
	// get the index of max/min ABS(value),the MKL library is 1-based
	INLFUNC int MaxAbsIndex() const{ return ::mkl::mkl_cpp::mkl_amax(_len,_p,_inc)-1; }
	INLFUNC int MinAbsIndex() const{ return ::mkl::mkl_cpp::mkl_amax(_len,_p,_inc)-1; }
	// get the value of max/min ABS(value),the MKL library is 1-based
	INLFUNC t_Val MinAbs() const{ return (*this)[MinAbsIndex()]; }
	INLFUNC t_Val MaxAbs() const{ return (*this)[MaxAbsIndex()]; }
	// swap content
	template<class t_BaseVec2>
	INLFUNC void Swap(Vector_Ref<t_Val,t_BaseVec2>& x)
	{ ASSERT(GetSize()==x.GetSize()); ::mkl::mkl_cpp::mkl_swap((MKL_SIZE)_len,_p,_inc,x.Begin(),(MKL_SIZE)x._len); }
	// Perturb values
	INLFUNC void Perturb(t_Val power = EPSILON)
	{ for(UINT i=0;i<GetSize();i++)at(i)+=power*(::rand()-(RAND_MAX/2))/((t_Val)(RAND_MAX/2)); }
	INLFUNC void PerturbPositive(t_Val power = EPSILON)
	{ for(UINT i=0;i<GetSize();i++)at(i)+=power*::rand()/((t_Val)RAND_MAX); }

	//assignment
	template<class t_BaseVec2>
	INLFUNC void CopyTo(Vector_Ref<t_Val,t_BaseVec2>& x) const
	{ ASSERT(x.GetSize() == GetSize()); ::mkl::mkl_cpp::mkl_copy((MKL_SIZE)_len,_p,_inc,x._p,x._inc); }

	template<class t_BaseVec2>
	INLFUNC void CopyFrom(const Vector_Ref<t_Val,t_BaseVec2>& x)
	{ ASSERT(x.GetSize() == GetSize()); ::mkl::mkl_cpp::mkl_copy((MKL_SIZE)_len,_p,x.GetStride(),_p,_inc); }

	template<typename T, typename BaseVec2>
	INLFUNC void CopyFrom(const Vector_Ref<T,BaseVec2> & x)
	{ ASSERT(GetSize()==x.GetSize());	for(MKL_SIZE i=0;i<GetSize();i++)(*this)[i] = x[i]; }

	template<class t_BaseVec2>
	INLFUNC const Vector_Ref<t_Val,t_BaseVec2>& operator = (const Vector_Ref<t_Val,t_BaseVec2>& x){ CopyFrom(x); return x; }
	INLFUNC t_Val operator = (t_Val x){ for(MKL_SIZE i=0;i<GetSize();i++)(*this)[i] = x; return x; }
	INLFUNC const Vector_Ref& operator = (const Vector_Ref& x){ CopyFrom(x); return x; }

public:
	typedef Vector_Ref Ref;

	INLFUNC Ref GetRef(){ return Ref(*this); }
	INLFUNC Ref GetSub(MKL_SIZE offset, MKL_SIZE len){ return Ref(&at(offset),len,_inc); }

	INLFUNC const Ref GetRef() const { return ::rt::_CastToNonconst(this)->GetRef(); }
	INLFUNC const Ref GetSub(MKL_SIZE x, MKL_SIZE len) const{ return ::rt::_CastToNonconst(this)->GetRef(x,len); }

	INLFUNC operator Ref& (){ return *((Ref*)this); }
	INLFUNC operator const Ref& () const { return ::rt::_CastToNonconst(this)->operator Ref& (); }

	template<typename T>
	INLFUNC const t_Val&  operator ()(T offset) const { return at(offset); }
	template<typename T>
	INLFUNC t_Val& operator ()(T offset){ return at(offset); }

	INLFUNC void Zero(){ *this = 0; }
	//////////////////////////////////////////////////////////
	// overrided operators
	INLFUNC void operator *= (t_Val a){ ::mkl::mkl_cpp::mkl_scal((MKL_SIZE)_len,a,_p,_inc); }
	template<typename T>
	INLFUNC void operator -= (const T& a){ ASSERT(GetSize() == a.GetSize()); for(MKL_SIZE i=0; i<GetSize(); i++)(*this)[i] -= a[i]; }
	template<typename T>
	INLFUNC void operator += (const T& a){ ASSERT(GetSize() == a.GetSize()); for(MKL_SIZE i=0; i<GetSize(); i++)(*this)[i] += a[i]; }
	INLFUNC void operator -= (t_Val a){ for(MKL_SIZE i=0; i<GetSize(); i++)(*this)[i] -= a; }
	INLFUNC void operator += (t_Val a){ for(MKL_SIZE i=0; i<GetSize(); i++)(*this)[i] += a; }

public:
	INLFUNC Vector_Ref(){}
	INLFUNC Vector_Ref(const t_Val* ptr, MKL_SIZE len, MKL_SIZE inc = 1)
	{
		_p=(t_Val*)ptr; _len=len; _set_stride(inc); 
	}
	INLFUNC Vector_Ref(const Vector_Ref& x):Base_Ref(x){}
	template<class _BaseVec>
	INLFUNC Vector_Ref(Vector_Ref<t_Val,_BaseVec> &x){ _p=x; _len=x.GetSize(); _set_stride(x.GetStride()); }
};
template<class t_Ostream, typename t_Ele, class t_Base>
t_Ostream& operator<<(t_Ostream& Ostream, const ::mkl::Vector_Ref<t_Ele, t_Base> & km)
{
	UINT ele_limite = 8; //::rt::_meta_::IsStreamStandard(Ostream)?8:UINT_MAX;
	Ostream<<'(';
	if(km.GetSize())
	{	
		if(km.GetSize()<=ele_limite)
		{//print all
			for(UINT i=0;i<km.GetSize();i++)
			{	if(i)Ostream<<',';
				Ostream<<rt::tos::Number(km[i]).RightAlign(8,' ');
			}
		}
		else
		{//print head and tail
			for(UINT i=0;i<=6;i++)
			{	if(i)Ostream<<',';
				Ostream<<rt::tos::Number(km[i]).RightAlign(8,' ');
			}
			Ostream<<" ...,";
			Ostream<<rt::tos::Number(km[km.GetSize()-1]).RightAlign(8,' ');
		}
	}
	Ostream<<')';
	return Ostream;
}


template< typename t_Val>
class Vector:public Vector_Ref<t_Val, mkl::_details::VectorCompact_Ref<t_Val>>
{	
	typedef Vector_Ref<t_Val, mkl::_details::VectorCompact_Ref<t_Val>> _SC;
//public:
//	TYPETRAITS_DECL_EXTENDTYPEID((rt::_typeid_codelib<<16)+10)
//	COMMON_CONSTRUCTOR_VEC(CVector)
//#pragma warning(disable:4244)
//	MKL_VEC_ASSIGN_OPERATOR(CVector)
//#pragma warning(default:4244)
//	DEFAULT_TYPENAME(CVector)
	INLFUNC void __SafeFree(){ _SafeFree32AL(((LPVOID&)_SC::_p)); }
public:
	typedef Vector_Ref<t_Val, mkl::_details::VectorCompact_Ref<t_Val>> Ref;
	INLFUNC ~Vector(){ __SafeFree(); }
	INLFUNC bool SetSize(MKL_SIZE co=0) //zero for clear
	{	if(co == _SC::_len){ return true; }
		else
		{	__SafeFree();
			if(co)
			{	_SC::_len = co;
				_SC::_p = _Malloc32AL(t_Val,co);
				return nullptr != _SC::_p;
			}
		}
		return true;
	}
	template<class T>
	INLFUNC bool SetSizeAs(const T& x){ return SetSize(x.GetSize()); }
	INLFUNC bool ChangeSize(MKL_SIZE new_size) //Orignal data at front is preserved
	{	
		if( new_size == _SC::_len )return true;
		if( new_size<_SC::_len ){ _SC::_len = new_size; return true; }
		else	//expand buffer
		{	t_Val* new_p = _Malloc32AL(t_Val,new_size);
			if(new_p)
			{	memcpy(new_p,_SC::_p,_SC::_len*sizeof(t_Val));
				_SafeFree32AL(_SC::_p);
				_SC::_p = new_p;		
				_SC::_len = new_size;
				return true;
			}
			return false;
		}
	}
	template<class t_BaseVec2>
	INLFUNC const Vector_Ref<t_Val,t_BaseVec2>& operator = (const Vector_Ref<t_Val,t_BaseVec2>& x){ VERIFY(SetSizeAs(x)); CopyFrom(x); return x; }
	INLFUNC t_Val operator = (t_Val x){ for(SIZE_T i=0;i<GetSize();i++)(*this)[i] = x; return x; }
	INLFUNC const Vector& operator = (const Vector& x){ VERIFY(SetSizeAs(x)); CopyFrom(x); return x; }
};

typedef Vector<float>		Vector32;
typedef Vector<double>		Vector64;

typedef Vector_Ref<float>	Vector32_Ref;
typedef Vector_Ref<double>	Vector64_Ref;

typedef Vector_Ref<float, mkl::_details::VectorCompact_Ref<float> >		Vector32_RefCompact;
typedef Vector_Ref<double, mkl::_details::VectorCompact_Ref<double> >	Vector64_RefCompact;

} // namespace mkl




