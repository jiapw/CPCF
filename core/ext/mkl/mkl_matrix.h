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

#include "mkl_vector.h"

namespace mkl
{

////////////////////////////////////////////////////
//			Column major
//
//			3x4 matrix ( 3 row x 4 column )
//			Logic layout:
//			| --------- 4 -----------> j
//			| / a00, a01, a02, a03 \
//			3 | a10, a11, a12, a13 |
//			| \ a20, a21, a22, a23 /
//			V
//			i
//
//			mx1 is column vector
//			1xn is row vector
//
//			Physical layout:
//			a00, a10, a20, p1, p2 .. pn
//			a01, a11, a21, p1, p2 .. pn
//			...
//			a03, a13, a23, p1, p2 .. pn
//			( px is padding elements ), leading_dimension is row+padding

namespace _details
{
template<typename t_Val, bool t_NotTransposed>
struct CRTypes;
	template<typename t_Val> struct CRTypes<t_Val, false>
	{	typedef Vector_Ref<t_Val, mkl::_details::VectorCompact_Ref<t_Val> >	t_RowRef;
		typedef Vector_Ref<t_Val, mkl::_details::VectorStrided_Ref<t_Val> >	t_ColRef;
	};
	template<typename t_Val> struct CRTypes<t_Val, true>
	{	typedef Vector_Ref<t_Val, mkl::_details::VectorCompact_Ref<t_Val> >	t_ColRef;
		typedef Vector_Ref<t_Val, mkl::_details::VectorStrided_Ref<t_Val> >	t_RowRef;
	};
}

enum _tagGeneralSVD_Flag
{
	Replace_None = 0,
	Replace_U_in_This,
	Replace_V_in_This
};


template<typename t_Val, bool t_NotTransposed>
class Matrix_Ref
{
	template<typename t_Val2, bool t_NotTransposed2>
	friend class Matrix_Ref;
	static_assert(	rt::TypeTraits<t_Val>::Typeid == ::rt::_typeid_32f || 
					rt::TypeTraits<t_Val>::Typeid == ::rt::_typeid_64f, 
					"Only float/double Matrix is supported");
	static const int OPTI_WORKING_BLOCK_SIZE = 64;

protected:
	MKL_SIZE	col_count;
	MKL_SIZE	row_count;
	MKL_SIZE	Padding;
	MKL_SIZE	LeadingDimen; // LeadingDimen = row_count + Padding;
	t_Val*		lpData;

	INLFUNC CBLAS_ORDER		 _BlasMajor() const{ return t_NotTransposed?CblasColMajor:CblasRowMajor; }
	INLFUNC CBLAS_TRANSPOSE	 _BlasTranspose() const{ return t_NotTransposed?CblasNoTrans:CblasTrans; }
	INLFUNC char			 _LapackTranspose() const{ return t_NotTransposed?'N':'T'; }
	INLFUNC operator t_Val*(){ return lpData; }
	INLFUNC operator const t_Val*() const { return lpData; }

public:
	template<typename T,typename T2>
	INLFUNC t_Val& operator ()(const T i,const T2 j){ return t_NotTransposed?lpData[j*LeadingDimen+i]:lpData[i*LeadingDimen+j]; }
	template<typename T,typename T2>
	INLFUNC const t_Val& operator ()(const T i,const T2 j) const{ return  t_NotTransposed?lpData[j*LeadingDimen+i]:lpData[i*LeadingDimen+j]; }

	INLFUNC Matrix_Ref(t_Val* p, MKL_SIZE rc, MKL_SIZE cc, MKL_SIZE leading){ lpData=p; col_count=cc; row_count=rc; LeadingDimen=leading; Padding=row_count - (t_NotTransposed?row_count:col_count); }
	INLFUNC Matrix_Ref(){ lpData=nullptr; col_count=0; row_count=0; Padding=0; LeadingDimen = 0; }
	INLFUNC ~Matrix_Ref(){}
	INLFUNC MKL_SIZE GetSize() const{ return col_count*row_count; }
	INLFUNC MKL_SIZE GetRowCount() const { return t_NotTransposed?row_count:col_count; }
	INLFUNC MKL_SIZE GetColCount() const { return t_NotTransposed?col_count:row_count; }
	INLFUNC MKL_SIZE GetLeadingDim() const { return LeadingDimen; }
	INLFUNC bool	 IsSquare() const { return row_count==col_count; }

public:
	typedef Vector_Ref<t_Val, mkl::_details::VectorCompact_Ref<t_Val> >	t_VecComp_Ref;
	typedef Vector_Ref<t_Val, mkl::_details::VectorStrided_Ref<t_Val> >	t_VecStrd_Ref;
	typedef Matrix_Ref<t_Val, t_NotTransposed>  Ref;
	typedef Matrix_Ref<t_Val, !t_NotTransposed> Ref_Transposed;

	INLFUNC Ref_Transposed& operator !(){ return Transpose(); }
	INLFUNC const Ref_Transposed& operator !() const{ return Transpose(); }
	INLFUNC Ref_Transposed& Transpose(){ return *((Ref_Transposed*)this); }
	INLFUNC const Ref_Transposed& Transpose() const { return ::rt::_CastToNonconst(this)->Transpose(); }

	INLFUNC Ref GetSub(MKL_SIZE i,MKL_SIZE j,MKL_SIZE row_co,MKL_SIZE col_co)
	{	ASSERT(i+row_co<=GetRowCount());
		ASSERT(j+col_co<=GetColCount());
		return Ref(&(*this)(i,j),row_co,col_co,LeadingDimen);
	}
	INLFUNC const Ref GetSub(MKL_SIZE i,MKL_SIZE j,MKL_SIZE row_co,MKL_SIZE col_co) const {	return rt::_CastToNonconst(this)->GetSub(i,j,row_co,col_co); }

public:
	typedef typename _details::CRTypes<t_Val, t_NotTransposed>::t_ColRef	t_ColRef;
	typedef typename _details::CRTypes<t_Val, t_NotTransposed>::t_RowRef	t_RowRef;

	INLFUNC t_ColRef GetCol(UINT j)
	{ ASSERT(j<GetColCount()); return t_ColRef(&(*this)(0,j),GetRowCount(),t_NotTransposed?1:GetLeadingDim()); }
	INLFUNC const t_ColRef GetCol(UINT j) const { return ::rt::_CastToNonconst(this)->GetCol(j); }

	INLFUNC t_RowRef GetRow(UINT i)
	{ ASSERT(i<GetRowCount()); return t_RowRef(&(*this)(i,0),GetColCount(),t_NotTransposed?GetLeadingDim():1); }
	INLFUNC const t_RowRef GetRow(UINT i) const	{ return ::rt::_CastToNonconst(this)->GetRow(i); }

	INLFUNC Vector_Ref<t_Val> GetDiag(){ return Vector_Ref<t_Val>(lpData,rt::min(GetRowCount(),GetColCount()),GetLeadingDim()+1); }
	INLFUNC const Vector_Ref<t_Val> GetDiag() const { return ::rt::_CastToNonconst(this)->GetDiag(); }
	
public:
	void CopyFrom(const Matrix_Ref<t_Val,t_NotTransposed>& x)
	{	ASSERT(GetRowCount() == x.GetRowCount());
		ASSERT(GetColCount() == x.GetColCount());
		if(t_NotTransposed)
		{	for(UINT j=0;j<GetColCount();j++)
				GetCol(j) = x.GetCol(j);
		}
		else
		{	for(UINT j=0;j<GetRowCount();j++)
				GetRow(j) = x.GetRow(j);
		}
	}
	void CopyFrom(const Matrix_Ref<t_Val,!t_NotTransposed>& x)
	{	ASSERT(GetRowCount() == x.GetRowCount());
		ASSERT(GetColCount() == x.GetColCount());
		if(&(*this)(0,0)!=&x(0,0))	// a = !b;
		{	for(UINT j=0;j<GetColCount();j++)
				GetCol(j) = x.GetCol(j);
		}
		else // a = !a;
		{	if(GetColCount()==GetRowCount())
			{	for(UINT i=0;i<GetColCount();i++)
				for(UINT j=0;j<i;j++)
					rt::Swap((*this)(i,j),(*this)(j,i));
			}
			else ASSERT(0);
		}
	}

	template<bool transpose> 
	INLFUNC const Matrix_Ref<t_Val, transpose>& operator = (const Matrix_Ref<t_Val, transpose>& x){ CloneFrom(x); return x; }
	INLFUNC t_Val operator = (t_Val x)
	{	if(t_NotTransposed)
			for(UINT i=0;i<GetColCount();i++)GetCol(i) = x;
		else
			for(UINT i=0;i<GetRowCount();i++)GetRow(i) = x;
		return x;
	}

	INLFUNC void ExchangeColumn(UINT j1,UINT j2)
	{	ASSERT(j1<GetColCount());	ASSERT(j2<GetColCount());
		if(j1!=j2){ ::mkl::mkl_cpp::mkl_swap(GetRowCount(),&(*this)(0,j1),t_NotTransposed?1:GetLeadingDim(),
														   &(*this)(0,j2),t_NotTransposed?1:GetLeadingDim()); 	
	}	}
	INLFUNC void ExchangeRow(UINT i1,UINT i2)
	{	ASSERT(i1<GetRowCount());	ASSERT(i2<GetRowCount());
		if(i1!=i2){ ::mkl::mkl_cpp::mkl_swap(GetRowCount(),&(*this)(i1,0),t_NotTransposed?GetLeadingDim():1,
														   &(*this)(i2,0),t_NotTransposed?GetLeadingDim():1); 	
	}	}
	INLFUNC void FlipColumns(){	for(UINT i=0;i<GetColCount()/2;i++)ExchangeColumn(i,GetColCount()-i-1); }
	INLFUNC void FlipRows(){ for(UINT i=0;i<GetRowCount()/2;i++) ExchangeRow(i,GetRowCount()-1-i); }
public:
	// Matrix as vector
	// disturb values
	INLFUNC void Perturb(t_Val power = EPSILON)
	{	if(t_NotTransposed)
			for(UINT i=0;i<GetColCount();i++)GetCol(i).Perturb(power);
		else
			for(UINT i=0;i<GetRowCount();i++)GetRow(i).Perturb(power);
	}
	INLFUNC void PerturbPositive(t_Val power = EPSILON)
	{	if(t_NotTransposed)
			for(UINT i=0;i<GetColCount();i++)GetCol(i).PerturbPositive(power);
		else
			for(UINT i=0;i<GetRowCount();i++)GetRow(i).PerturbPositive(power);
	}	
	INLFUNC void Zero()
	{	if(t_NotTransposed)
			for(UINT i=0;i<GetColCount();i++)GetCol(i).Zero();
		else
			for(UINT i=0;i<GetRowCount();i++)GetRow(i).Zero();
	}
	INLFUNC t_Val Sum() const
	{	t_Val sum = 0;
		if(t_NotTransposed)
			for(UINT i=0;i<GetColCount();i++)sum += GetCol(i).Sum();
		else
			for(UINT i=0;i<GetRowCount();i++)sum += GetRow(i).Sum();
		return sum;
	}
	INLFUNC t_Val SumSqrt() const
	{	t_Val sum = 0;
		if(t_NotTransposed)
			for(UINT i=0;i<GetColCount();i++)sum += GetCol(i).SumSqrt();
		else
			for(UINT i=0;i<GetRowCount();i++)sum += GetRow(i).SumSqrt();
		return sum;
	}
	INLFUNC t_Val MaxAbs() const
	{	t_Val maxval = 0;
		if(t_NotTransposed)
			for(UINT i=0;i<GetColCount();i++)maxval = rt::max(maxval, GetCol(i).MaxAbs());
		else
			for(UINT i=0;i<GetRowCount();i++)maxval = rt::max(maxval, GetRow(i).MaxAbs());
		return maxval;
	}
	INLFUNC t_Val MinAbs() const
	{	t_Val minval = 0;
		if(t_NotTransposed)
			for(UINT i=0;i<GetColCount();i++)minval = rt::max(minval, GetCol(i).MinAbs());
		else
			for(UINT i=0;i<GetRowCount();i++)minval = rt::max(minval, GetRow(i).MinAbs());
		return minval;
	}
	INLFUNC t_Val operator *= (t_Val s)
	{	if(t_NotTransposed)
			for(UINT i=0;i<GetColCount();i++)GetCol(i) *= s;
		else
			for(UINT i=0;i<GetRowCount();i++)GetRow(i) *= s;
		return s;
	}
	template<typename t_Val2, bool transpose>
	INLFUNC const Matrix_Ref<t_Val2, transpose>& operator = (const Matrix_Ref<t_Val2, transpose>& x){ CopyFrom(x); return *this; }
	INLFUNC const Matrix_Ref& operator = (const Matrix_Ref& x){ CopyFrom(x); return *this; }
public:
	//BLAS Level 2
	//y = alpha*This*x + beta*x
	template<class t_Vec1, class t_Vec2>
	void VectorProduct(const t_Vec1& x,t_Vec2& y,t_Val alpha=1,t_Val beta=0) const
	{
		ASSERT(x.GetSize() == GetColCount());
		ASSERT(y.GetSize() == GetRowCount());
		::mkl::mkl_cpp::mkl_gemv(_BlasMajor(),CblasNoTrans,(MKL_INT)GetRowCount(),(MKL_INT)GetColCount(),alpha,
								 (const t_Val*)*this,(MKL_INT)GetLeadingDim(),
								 (const t_Val*)x,(MKL_INT)x.GetStride(),beta,
								 (t_Val*)y,(MKL_INT)y.GetStride());
	}

	//BLAS Level 3
	//This = alpha*A*B + beta*This
	template<bool _a,bool _b>
	void Product(const Matrix_Ref<t_Val,_a>& a,const Matrix_Ref<t_Val,_b>& b,t_Val alpha=1,t_Val beta=0)
	{	ASSERT(((size_t)this)!=((size_t)&a));	//a should not be the same object of This
		ASSERT(((size_t)this)!=((size_t)&b));	//b should not be the same object of This
		ASSERT(a.GetColCount() == b.GetRowCount());
		ASSERT(GetRowCount() == a.GetRowCount());
		ASSERT(GetColCount() == b.GetColCount());

		::mkl::mkl_cpp::mkl_gemm(CblasColMajor,a._BlasTranspose(),b._BlasTranspose(),
								 GetRowCount(),GetColCount(),a.GetColCount(),
								 alpha,a,a.GetLeadingDim(),b,b.GetLeadingDim(),
								 beta,*this,GetLeadingDim());
	}

	//LAPACK
	//Matrix Inversion
	//This = This^-1
	bool Inverse()
	{	ASSERT(IsSquare()); 
		rt::Buffer<int> Ipiv;
		if(Ipiv.SetSize(GetColCount())){}
		else return false;
		// LU factorization
		int ret;
		ret = ::mkl::mkl_cpp::mkl_getrf(GetRowCount(),GetColCount(),*this,GetLeadingDim(),Ipiv);

		if( !ret )
		{	mkl::_meta_::CVector_Compact<t_Val> Workspc;
			if(Workspc.SetSize(GetColCount()*OPTI_WORKING_BLOCK_SIZE)){}
			else return false;
			
			ret = ::mkl::mkl_cpp::mkl_getri(GetColCount(),*this,GetLeadingDim(),Ipiv,Workspc,Workspc.GetSize());
		}
		return ret == 0;
	}

	//A*A'=E
	bool AccurateInverse()
	{	ASSERT(IsSquare()); 
		rt::Buffer<int> Ipiv;
		if(Ipiv.SetSize(GetColCount())){}
		else return false;

		mkl::Matrix<t_Val>	origSolveLinear(*this);
		// LU factorization
		int ret;
		ret = ::mkl::mkl_cpp::mkl_getrf(GetRowCount(),GetColCount(),*this,GetLeadingDim(),Ipiv);

		if( !ret )
		{	mkl::_meta_::Vector_Compact<t_Val> Workspc;
			if(Workspc.SetSize(GetColCount()*OPTI_WORKING_BLOCK_SIZE)){}
			else{ return false; }
			::rt::Buffer<int> intWorkspc;
			if(intWorkspc.SetSize(GetColCount()*OPTI_WORKING_BLOCK_SIZE)){}
			else{ return false; }
			
			mkl::CMatrix<t_Val>		identity, X;
			identity.SetSizeAs(*this);
			X.SetSizeAs(identity);
			X.Zero();
			identity.Zero();

			for( UINT i=0; i<identity.GetRowCount(); i++ )
			{
				X(i,i) = static_cast<t_Val>(1.0);
				identity.At(i,i) = static_cast<t_Val>(1.0);
			}
			
			ret = ::mkl::mkl_cpp::mkl_getrs(_LapackTranspose(), GetColCount(), GetRowCount(),origSolveLinear,
				origSolveLinear.GetLeadingDim(), Ipiv, X, X.GetLeadingDim() );

			mkl::CMatrix<t_Val>		temp;
			temp.SetSizeAs(origSolveLinear);
			temp.Product(!!origSolveLinear,!!X);

			if(!ret)
			{
				mkl::_meta_::CVector_Compact<t_Val> ferr, berr;
				if(ferr.SetSize(GetColCount()) && berr.SetSize(GetColCount()) )  {}
				else return false;
				//refine the solution
				ret = ::mkl::mkl_cpp::mkl_gerfs(_LapackTranspose(), GetColCount(), GetRowCount(), 
					origSolveLinear, origSolveLinear.GetLeadingDim(), *this, GetLeadingDim(), Ipiv, identity, identity.GetLeadingDim(), 
					X, X.GetLeadingDim(), ferr, berr, Workspc, intWorkspc); 
			}

			temp.Product(!!origSolveLinear, !!X);
			
			if(ret)
				return false;
			else
			{
				*this = X;
				return true;
			}
		}
		else
			return false;
	}

	//Linear Equations
	//This*x=B  --> A change to LU, B change to solution
	bool SolveLinearEquations( Matrix_Ref<t_Val,true>&B )
	{	static_assert(t_NotTransposed, "row major matrix is not supported");
		ASSERT(IsSquare());
		ASSERT(GetRowCount() == B.GetRowCount());
		::rt::Buffer_32BIT<int> Ipiv;
		if(Ipiv.SetSize(GetColCount())){}
		else{ return false; }

		int ret = ::mkl::mkl_cpp::mkl_gesv(GetRowCount(),B.GetColCount(),*this,GetLeadingDim(),Ipiv,B,B.GetLeadingDim());
		return 0 == ret;
	}
	INLFUNC bool SolveLinearEquations( t_VecComp_Ref&b )
	{	ASSERT(GetRowCount() == b.GetSize());
		return SolveLinearEquations( CMatrix_Ref<t_Val,true>(b,b.GetSize(),1,b.GetSize()));
	}

	//Linear Least Squares
	//This*x=B  --> This change to LU, B change to solution
	bool SolveLinearLeastSquares_LQR(Matrix_Ref<t_Val,true>& B)
	{	ASSERT(rt::max(GetRowCount(),GetColCount()) == B.GetRowCount());
		Vector<t_Val> Workspc;
		if(Workspc.SetSize(rt::min(GetColCount(),GetRowCount())+rt::max(B.GetColCount(),rt::max(GetColCount(),GetRowCount()))*OPTI_WORKING_BLOCK_SIZE)){}
		else{ return false; }

		return 0 == 
		::mkl::mkl_cpp::mkl_gels(_LapackTranspose(),GetRowCount(),GetColCount(),B.GetColCount(),*this,
									GetLeadingDim(),B,B.GetLeadingDim(),Workspc,Workspc.GetSize());
	}
	INLFUNC bool SolveLinearLeastSquares_LQR(t_VecComp_Ref&b)
	{	return SolveLinearLeastSquares_LQR( Matrix_Ref<t_Val,true>(b,b.GetSize(),1,b.GetSize())); }

	//Linear Least Squares
	//This*x=B  --> This change to singlar vectors, B change to solution
	bool SolveLinearLeastSquares_SVD( Matrix_Ref<t_Val,true>& B , UINT* pRank = nullptr, t_Val rank_condition = ((t_Val)0.0001) )
	{	static_assert(t_NotTransposed, "row major matrix is not supported");
		ASSERT(rt::max(GetColCount(),GetRowCount()) == B.GetRowCount());
		UINT s_len = rt::min(GetColCount(),GetRowCount());
		Vector<t_Val> Workspc; //first [s_len] elements used to output eigenvalue
		if(Workspc.SetSize(	3*s_len + s_len + 
							rt::max(rt::max(B.GetColCount(),rt::max(GetColCount(),GetRowCount())),2*s_len))
		  ){}
		else{ return false; }
		UINT determined_rank;
		if(!pRank)pRank = &determined_rank;
		return 0 == 
		::mkl::mkl_cpp::mkl_gelss(	GetRowCount(),GetColCount(),B.GetColCount(),
									*this,GetLeadingDim(),B,B.GetLeadingDim(),
									Workspc,rank_condition,(int*)pRank,&Workspc[s_len],Workspc.GetSize()-s_len);
	}
	INLFUNC bool SolveLinearLeastSquares_SVD(t_VecComp_Ref&b, UINT* pRank = nullptr, t_Val rank_condition = ((t_Val)0.0001))
	{	return SolveLinearLeastSquares_SVD(	Matrix_Ref<t_Val,true>(b,b.GetSize(),1,b.GetSize()),
											pRank, rank_condition); 
	}

	//Singular Value Decomposition of a general rectangular matrix.
	//This = USV'
	bool GeneralSVD( Vector<t_Val>& EigenValue, DWORD ReplaceFlag = mkl::Replace_U_in_This, Matrix<t_Val>* pU_Matrix=nullptr, Matrix<t_Val>* pV_Matrix=nullptr)
	{	UINT m = GetRowCount();
		UINT n = GetColCount();
		UINT order = rt::min(m,n);

		if(ReplaceFlag==Replace_U_in_This)ASSERT(pU_Matrix==nullptr);
		if(ReplaceFlag==Replace_V_in_This)ASSERT(pV_Matrix==nullptr);

		if(EigenValue.SetSize(order)){}else{ return false; }
		if(pU_Matrix)
			if(pU_Matrix->SetSize(m,order)){}else{ return false; }
		if(pV_Matrix)
			if(pV_Matrix->SetSize(order,n)){}else{ return false; }
		
		int info;
		mkl::Vector<t_Val> workspc;
		{	t_Val opti_work_size = 0;
			info = ::mkl::mkl_cpp::mkl_gesvd((ReplaceFlag==Replace_U_in_This)?'O':(pU_Matrix?'S':'N'),
                                             (ReplaceFlag==Replace_V_in_This)?'O':(pV_Matrix?'S':'N'),
											 m,n,*this,GetLeadingDim(),EigenValue,
											 pU_Matrix?(pU_Matrix->GetCol(0).Begin()):nullptr,
											 pU_Matrix?(pU_Matrix->GetLeadingDim()):1,
											 pV_Matrix?(pV_Matrix->GetCol(0).Begin()):nullptr,
											 pV_Matrix?(pV_Matrix->GetLeadingDim()):1,
											 &opti_work_size,-1);
			if(info){ return false; }
			if(workspc.SetSize((UINT)opti_work_size)){}else{ return false; }
		}

		info = ::mkl::mkl_cpp::mkl_gesvd((ReplaceFlag==Replace_U_in_This)?'O':(pU_Matrix?'S':'N'),
                                         (ReplaceFlag==Replace_V_in_This)?'O':(pV_Matrix?'S':'N'),
										 m,n,*this,GetLeadingDim(),EigenValue,
										 pU_Matrix?(pU_Matrix->GetCol(0).Begin()):nullptr,
										 pU_Matrix?(pU_Matrix->GetLeadingDim()):1,
										 pV_Matrix?(pV_Matrix->GetCol(0).Begin()):nullptr,
										 pV_Matrix?(pV_Matrix->GetLeadingDim()):1,
										 workspc,workspc.GetSize());
		if(info)
			return false;
		else
			return true;
	}

	//Symmetric Eigen Problem
	//This turn to EigenVectors (in columns)
	bool SolveSymmetricEigen(Vector<t_Val>& EigenValue, bool NeedEigenVectors = false)
	{	ASSERT(IsSquare());
		if(EigenValue.SetSize(GetColCount())){}
		else{ return false; }

		Vector<t_Val> Workspc;
		if(Workspc.SetSize(max(3*GetColCount()-1,GetColCount()*(OPTI_WORKING_BLOCK_SIZE+2)))){}
		else{ return false; }

		int ret;
		ret = ::mkl::mkl_cpp::mkl_syev(	NeedEigenVectors?'V':'N','U',
										GetColCount(),*this,GetLeadingDim(),EigenValue,Workspc,Workspc.GetSize());
		return 0 == ret;
	}

	// Symmetric Eigen Problem
	// solve eigens of [TermStart,TermEnd] zero-based, eigenvalue is in ascending order
	// EigenVectors in rows
	HRESULT PartialSolveSymmetricEigen(	UINT TermStart,UINT TermEnd, 
										Vector<t_Val>&  EigenValue, 
										Matrix<t_Val>* EigenVectors = nullptr)
	{	ASSERT(IsSquare());
		ASSERT( TermStart>=0 && TermStart<=TermEnd && TermEnd<GetColCount() );
		TermStart++;   TermEnd++;   // MKL has 1-based index

		if(EigenValue.SetSize(TermEnd-TermStart+1)){}
		else{ return false; }
		
		if( EigenVectors )
		{	if(EigenVectors->SetSize(GetColCount(),TermEnd-TermStart+1)){}
			else{ return false; }
		}

		rt::Buffer<int> ifail_iWork;
		if(ifail_iWork.SetSize(6*GetColCount()+2)){}
		else{ return false; }

		t_Val abstol = 0;//2*slamch("S");
		int  m=0, info;

		mkl::_meta_::CVector_Compact<t_Val> Workspc;
		// query optimized working space
		{
			t_Val opti_workspace_size = 0;
			info = ::mkl::mkl_cpp::mkl_syevx(	(EigenVectors!=nullptr)?'V':'N','I','U',
												GetColCount(),*this,GetLeadingDim(),0,0,
												TermStart,TermEnd,abstol,&m,EigenValue,
												EigenVectors?EigenVectors->GetVec().Begin():nullptr,
												EigenVectors?EigenVectors->GetLeadingDim():0,
												&opti_workspace_size,-1,&ifail_iWork[GetColCount()+1],ifail_iWork);
			if(info){ return false; }
			if(Workspc.SetSize((UINT)opti_workspace_size)){}
			else{ return false; }
		}
	
		info = ::mkl::mkl_cpp::mkl_syevx(	(EigenVectors!=nullptr)?'V':'N','I','U',
											GetColCount(),*this,GetLeadingDim(),0,0,
											TermStart,TermEnd,abstol,&m,EigenValue,
											*EigenVectors,EigenVectors?EigenVectors->GetLeadingDim():0,
											Workspc,Workspc.GetSize(),&ifail_iWork[GetColCount()+1],ifail_iWork);
		if(info)
		{
			if(info>0)
			{	_CheckDump("::mkl::CMatrix::PartialSolveSymmetricEigen: Total "<<info<<" eigenvalues failed to converge:");
				for(int i=0;i<info-1;i++)
					_CheckDump(ifail_iWork[i]<<", ");
				_CheckDump(ifail_iWork[info-1]<<"\n");
			}
			return false;
		}
		else
			return true;
	}

	// Symmetric Eigen Problem based on Relatively Robust Representations.
	// solve eigens of [TermStart,TermEnd] zero-based, eigenvalue is in ascending order
	// EigenVectors in rows
	bool PartialSolveSymmetricEigen_Robust(	UINT TermStart,UINT TermEnd, 
												Vector<t_Val>& EigenValue, 
												Matrix<t_Val>* EigenVectors = nullptr)
	{	ASSERT(IsSquare());
		ASSERT( TermStart>=0 && TermStart<=TermEnd && TermEnd<GetColCount() );
		TermStart++;   TermEnd++;   // MKL has 1-based index

		if(EigenValue.SetSize(GetColCount())){}
		else{ return false; }
		
		if( EigenVectors )
		{	if(EigenVectors->SetSize(GetColCount(),TermEnd-TermStart+1)){}
			else{ return false; }
		}

		::rt::Buffer<int> isuppz;
		if(isuppz.SetSize(2*GetColCount())){}else{ return false; }

		t_Val abstol = 2*slamch("S");
		int  m=0, info;

		rt::Buffer<t_Val> Workspc;
		rt::Buffer<int> iwork;
		// query optimized working space
		{
			t_Val opti_workspace_size = 0;
			int   opti_iwork_size = 0;
			info = ::mkl::mkl_cpp::mkl_syevr(	(EigenVectors!=nullptr)?'V':'N','I','U',
												GetColCount(),*this,GetLeadingDim(),0,0,
												TermStart,TermEnd,abstol,&m,EigenValue,
												*EigenVectors,EigenVectors?EigenVectors->GetLeadingDim():0,isuppz,
												&opti_workspace_size,-1,&opti_iwork_size,-1);
			if(info){ return false; }
			if(Workspc.SetSize((UINT)opti_workspace_size)){}else{ return false; }
			if(iwork.SetSize((UINT)opti_iwork_size)){}else{ return false; }
		}

		info = ::mkl::mkl_cpp::mkl_syevr(	(EigenVectors!=nullptr)?'V':'N','I','U',
											GetColCount(),*this,GetLeadingDim(),0,0,
											TermStart,TermEnd,abstol,&m,EigenValue,
											*EigenVectors,EigenVectors?EigenVectors->GetLeadingDim():0,isuppz,
											Workspc,(MKL_INT)Workspc.GetSize(),iwork,(MKL_INT)iwork.GetSize());
		if(info)
			return false;
		else
		{	EigenValue.ChangeSize(m);
			return true;
		}
	}

	// Symmetric Eigen Problem
	// solve eigens of (EigenvalueMin,EigenvalueMax], eigenvalue is in ascending order
	// EigenVectors in rows
	bool PartialSolveSymmetricEigen_Robust(	Vector<t_Val>& EigenValue, 
												t_Val EigenvalueMin = EPSILON,
												Matrix<t_Val>* EigenVectors = nullptr,
												t_Val EigenvalueMax = FLT_MAX)
	{	ASSERT(IsSquare());
		ASSERT( EigenvalueMin<EigenvalueMax );
		UINT order = GetColCount();
		
		if(!EigenValue.SetSize(order))return false;
		if( EigenVectors )
		{	if(EigenVectors->SetSize(order,order)){}
			else{ return false; }
		}

		rt::Buffer<int> isuppz;
		if(isuppz.SetSize(2*GetColCount())){}else{ return false; }

		t_Val abstol = 2*slamch("S");
		int  m=0, info;

		rt::Buffer<t_Val> Workspc;
		rt::Buffer<int> iwork;
		// query optimized working space
		{
			t_Val opti_workspace_size = 0;
			int   opti_iwork_size = 0;
			info = ::mkl::mkl_cpp::mkl_syevr(	(EigenVectors!=nullptr)?'V':'N','V','U',
												GetColCount(),*this,GetLeadingDim(),EigenvalueMin,EigenvalueMax,
												0,0,abstol,&m,EigenValue,
												EigenVectors?((t_Val*)(*EigenVectors)):nullptr,
												EigenVectors?EigenVectors->GetLeadingDim():1,isuppz,
												&opti_workspace_size,-1,&opti_iwork_size,-1);
			if(info){ return false; }
			if(Workspc.SetSize((UINT)opti_workspace_size)){}else{ return false; }
			if(iwork.SetSize((UINT)opti_iwork_size)){}else{ return false; }
		}
	
		info = ::mkl::mkl_cpp::mkl_syevr(	(EigenVectors!=nullptr)?'V':'N','V','U',
											GetColCount(),*this,GetLeadingDim(),EigenvalueMin,EigenvalueMax,
											0,0,abstol,&m,EigenValue,
											EigenVectors?((t_Val*)(*EigenVectors)):nullptr,
											EigenVectors?EigenVectors->GetLeadingDim():1,isuppz,
											Workspc,(MKL_INT)Workspc.GetSize(),iwork,(MKL_INT)iwork.GetSize());
		if(info)
			return false;
		else
		{	EigenValue.ChangeSize(m);
			return true;
		}
	}
};
template<class t_Ostream, typename t_Ele, bool IsNotTransposed>
t_Ostream& operator<<(t_Ostream& Ostream, const ::mkl::Matrix_Ref<t_Ele, IsNotTransposed> & km)
{
	UINT ele_limite = 8; //::rt::_meta_::IsStreamStandard(Ostream)?8:UINT_MAX;
	if(km.GetSize() == 0)
	{	Ostream<<"{ -NULL- }";
		return Ostream;
	}
	Ostream<<"{ // "<<km.GetRowCount()<<'x'<<km.GetColCount()<<" Matrix";

	if(IsNotTransposed){ Ostream<<'\n'; }
	else{ Ostream<<" (transposed)\n"; }
	if(km.GetRowCount()<=ele_limite)
	{	//print all
		for(UINT i=0;i<km.GetRowCount()-1;i++)
		{
			Ostream<<' '<<km.GetRow(i)<<','<<'\n';
		}
		Ostream<<' '<<km.GetRow(km.GetRowCount()-1)<<'\n';
	}
	else
	{	//print head and tail
		for(UINT i=0;i<=4;i++)
			Ostream<<' '<<km.GetRow(i)<<','<<'\n';
		Ostream<<"   ... ...\n";
		Ostream<<' '<<km.GetRow(km.GetRowCount()-2)<<','<<'\n';
		Ostream<<' '<<km.GetRow(km.GetRowCount()-1)<<'\n';
	}
	Ostream<<'}'<<'\n';
	return Ostream;
}

template<typename t_Val>
class Matrix: public Matrix_Ref<t_Val, true>
{
protected:
	void	__SafeFree(){ _SafeFree32AL(lpData); }
public:
	~Matrix(){ __SafeFree(); }
	INLFUNC bool SetSize(MKL_SIZE row=0,MKL_SIZE col=0)
	{	if(row==row_count && col==col_count){ return true; }
		else
		{	__SafeFree();
			if(row&&col)
			{
				LeadingDimen = (MKL_INT)_EnlargeTo32AL(row);
				lpData = _Malloc32AL(t_Val,LeadingDimen*col);
			}
			if(lpData)
			{	row_count=row; col_count=col; 
				Padding = LeadingDimen - row;
				return true;
			}
			else{ row_count=col_count=0; }
		}
		return false;
	}
	template<class T>
	INLFUNC bool SetSizeAs(const T& x){ return SetSize(x.GetRowCount(),x.GetColCount()); }
	template<typename t_MA, typename t_MB>
	INLFUNC bool SetSizeAsProductOf(const t_MA& a,const t_MB& b){ return SetSize(a.GetRowCount(),b.GetColCount()); }
public:
	template<typename T> 
	void CloneFrom(const T& x)
	{	if(GetRowCount() == x.GetRowCount() && GetColCount() == x.GetColCount())
		{	CopyFrom(x);
		}
		else
		{	Matrix<t_Val> tmp;
			tmp.SetSizeAs(x);
			tmp.CopyFrom(x);
			rt::Swap((*this), tmp);
		}
	}

	template<bool transpose> 
	INLFUNC const Matrix_Ref<t_Val, transpose>& operator = (const Matrix_Ref<t_Val, transpose>& x){ CloneFrom(x); return x; }
	INLFUNC const Matrix& operator = (const Matrix& x){ CloneFrom(x); return x; }
	INLFUNC t_Val operator = (t_Val x){ for(UINT i=0;i<GetColCount();i++)GetCol(i) = x; return x; }

	// SVD (Driver function for PartialSolveSymmetricEigen)
	// [*this] turn to EigenVectors after solving in descending order
	bool SolveEigen_ByTerm(Vector<t_Val>& EigenValue,UINT FeatureDimenMax,UINT FeatureDimenMin = 1,bool NeedEigenVectors = true)
	{	UINT dimen = GetColCount();
		ASSERT( dimen == GetRowCount() );
		ASSERT(FeatureDimenMax >= FeatureDimenMin);
		ASSERT(dimen >= FeatureDimenMax);
		
		Matrix<t_Val> MatTemp;

		if(!PartialSolveSymmetricEigen_Robust(	dimen-FeatureDimenMax,dimen-1,
												EigenValue,NeedEigenVectors?&MatTemp:nullptr)
		)return false;

		UINT org_terms = EigenValue.GetSize();
		EigenValue.Flip();

		{	UINT i=0;
			t_Val EigenMax = EigenValue[0];
			for(;i<EigenValue.GetSize();i++)
			{
				if(	rt::IsNumberOk(EigenValue[i]) && 
					(EigenValue[i]/EigenMax)>::rt::TypeTraits<t_Val>::Epsilon()*10 ){ continue; }
				else{ break; }
			}
			EigenValue.ChangeSize(rt::max(FeatureDimenMin,i));
		}

		//reserves eigenvectors
		if( NeedEigenVectors )
		{
			if(EigenValue.GetSize() == MatTemp.GetColCount()){ ::rt::Swap(*this,MatTemp); }
			else
			{	if(SetSize(MatTemp.GetRowCount(),EigenValue.GetSize()))
				{	*this = MatTemp.GetSub(0,org_terms-EigenValue.GetSize(),MatTemp.GetRowCount(),EigenValue.GetSize());
				}
				else{ return false; }
			}
			FlipColumns();
		}

		return true;
	}
	// SVD (Driver function for PartialSolveSymmetricEigen)
	// [*this] turn to EigenVectors (in rows) after solving in descending order
	HRESULT SolveEigen_ByValue(Vector<t_Val>& EigenValue,t_Val EigenvalueMin = EPSILON, bool NeedEigenVectors = true)
	{	int dimen = GetColCount();
		ASSERT( dimen == GetRowCount() );
		ASSERT(EigenvalueMin>0);

		Matrix<t_Val> MatTemp;

		if(!PartialSolveSymmetricEigen_Robust(EigenValue,EigenvalueMin,NeedEigenVectors?&MatTemp:nullptr))
			return false;
		EigenValue.Flip();
		//reserves eigenvectors
		if( NeedEigenVectors )
		{
			if(SetSize(MatTemp.GetRowCount(),EigenValue.GetSize()))
			{	*this = MatTemp.GetSub(0,0,MatTemp.GetRowCount(),EigenValue.GetSize());
			}
			else{ return false; }
			FlipColumns();
		}

		return true;
	}
	// SVD (Driver function for PartialSolveSymmetricEigen)
	// [*this] turn to EigenVectors after solving in descending order
	bool SolveEigen_ByEnerge(Vector<t_Val>& EigenValue,float EnergePreservationRate = 1.0f,UINT dimen_max = UINT_MAX, bool ignore_first_eige_in_energy = false)
	{	UINT term;
		if(EnergePreservationRate < 0.999999999f)
		{	Matrix<t_Val>	mat_temp;
			mat_temp.SetSizeAs(*this);
			mat_temp = *this;
			if(mat_temp.SolveEigen_ByValue(EigenValue,FLT_EPSILON,false))
			{	if(ignore_first_eige_in_energy)EigenValue[0] = (t_Val)2.0 * ::rt::TypeTraits<t_Val>::Epsilon();
				t_Val tot_energe = EigenValue.Sum()*EnergePreservationRate;
				UINT i=0;
				for(;i<EigenValue.GetSize();i++)
					if(	tot_energe > ::rt::TypeTraits<t_Val>::Epsilon() )
					{	tot_energe -= EigenValue[i]; 
					}
					else{ break; }
				term = rt::min(dimen_max,i);
			}else{ return false; }
		}
		else term = rt::min(dimen_max,GetColCount());
		return SolveEigen_ByTerm(EigenValue,term);
	}
};


typedef Matrix<float>			Matrix32;
typedef Matrix<double>			Matrix64;

typedef Matrix_Ref<float, true>		Matrix32_Ref;
typedef Matrix_Ref<double, true>	Matrix64_Ref;

typedef Matrix_Ref<float, false>	Matrix32_TransposdRef;
typedef Matrix_Ref<double, false>	Matrix64_TransposdRef;



} // namespace mkl

