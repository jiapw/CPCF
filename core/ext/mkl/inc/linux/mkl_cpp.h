#pragma once

//////////////////////////////////////////////////////////////////////
// Copyright 2012 the Cicada Project Dev Team. All rights reserved.
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
//     * Neither the name of Cicada.  nor the names of its
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

#include "mkl.h"
#define __forceinline inline 
namespace mkl
{
namespace mkl_cpp
{
////////////////////////////////////////////////////
// C++ overrides for MKL functions
////////////////////////////////////////////////////

////////////////////////////////////////////////////
// BLAS Level 1
__forceinline float mkl_asum(MKL_INT N, const float *X, MKL_INT incX)
{ return cblas_sasum(N,X,incX); }
__forceinline double mkl_asum(MKL_INT N, const double *X, MKL_INT incX)
{ return cblas_dasum(N,X,incX); }

__forceinline float mkl_nrm2(MKL_INT N, const float *X, MKL_INT incX)
{ return cblas_snrm2(N,X,incX); }
__forceinline double mkl_nrm2(MKL_INT N, const double *X, MKL_INT incX)
{ return cblas_dnrm2(N,X,incX); }

__forceinline void mkl_axpy(MKL_INT N, float alpha, const float *X,MKL_INT incX, float *Y,MKL_INT incY)
{ cblas_saxpy(N,alpha,X,incX,Y,incY); }
__forceinline void mkl_axpy(MKL_INT N, double alpha, const double *X,MKL_INT incX, double *Y,MKL_INT incY)
{ cblas_daxpy(N,alpha,X,incX,Y,incY); }

__forceinline float mkl_dot(MKL_INT N, const float  *X,MKL_INT incX, const float  *Y,MKL_INT incY)
{ return cblas_sdot(N,X,incX,Y,incY); }
__forceinline double mkl_dot(MKL_INT N, const double  *X,MKL_INT incX, const double  *Y,MKL_INT incY)
{ return cblas_ddot(N,X,incX,Y,incY); }

__forceinline void mkl_swap(MKL_INT N, float *X, MKL_INT incX, float *Y, MKL_INT incY)
{ cblas_sswap(N,X,incX,Y,incY); }
__forceinline void mkl_swap(MKL_INT N, double *X, MKL_INT incX, double *Y, MKL_INT incY)
{ cblas_dswap(N,X,incX,Y,incY); }

__forceinline size_t mkl_amax(MKL_INT N, const float  *X, MKL_INT incX)
{ return cblas_isamax(N,X,incX); }
__forceinline size_t mkl_amax(MKL_INT N, const double  *X, MKL_INT incX)
{ return cblas_idamax(N,X,incX); }

__forceinline size_t mkl_amin(MKL_INT N, const float  *X, MKL_INT incX)
{ return cblas_isamin(N,X,incX); }
__forceinline size_t mkl_amin(MKL_INT N, const double  *X, MKL_INT incX)
{ return cblas_idamin(N,X,incX); }

__forceinline void mkl_copy(MKL_INT N, const float *X, MKL_INT incX,float *Y, MKL_INT incY)
{ return cblas_scopy(N,X,incX,Y,incY); }
__forceinline void mkl_copy(MKL_INT N, const double *X, MKL_INT incX,double *Y, MKL_INT incY)
{ return cblas_dcopy(N,X,incX,Y,incY); }

__forceinline void mkl_scal(MKL_INT N, float alpha, float *X, MKL_INT incX)
{ cblas_sscal(N,alpha,X,incX); }
__forceinline void mkl_scal(MKL_INT N, double alpha, double *X, MKL_INT incX)
{ cblas_dscal(N,alpha,X,incX); }


////////////////////////////////////////////////////
// BLAS Level 2
__forceinline void mkl_gemv(CBLAS_ORDER order,
							CBLAS_TRANSPOSE TransA, MKL_INT M, MKL_INT N,
							float alpha, const float *A, MKL_INT lda,
							const float *X, MKL_INT incX, float beta,
							float *Y, MKL_INT incY)
{
	cblas_sgemv(order,TransA,M,N,alpha,A,lda,X,incX,beta,Y,incY);
}
__forceinline void mkl_gemv(CBLAS_ORDER order,
							CBLAS_TRANSPOSE TransA, MKL_INT M, MKL_INT N,
							double alpha, const double *A, MKL_INT lda,
							const double *X, MKL_INT incX, double beta,
							double *Y, MKL_INT incY)
{
	cblas_dgemv(order,TransA,M,N,alpha,A,lda,X,incX,beta,Y,incY);
}


////////////////////////////////////////////////////
// BLAS Level 3
__forceinline void mkl_gemm(	const  CBLAS_ORDER Order, const  CBLAS_TRANSPOSE TransA,
								const  CBLAS_TRANSPOSE TransB, const MKL_INT M, const MKL_INT N,
								const MKL_INT K, const float alpha, const float *A,
								const MKL_INT lda, const float *B, const MKL_INT ldb,
								const float beta, float *C, const MKL_INT ldc)
{
	cblas_sgemm(Order,TransA,TransB,M,N,K,alpha,A,lda,B,ldb,beta,C,ldc);
}

__forceinline void mkl_gemm(	const  CBLAS_ORDER Order, const  CBLAS_TRANSPOSE TransA,
								const  CBLAS_TRANSPOSE TransB, const MKL_INT M, const MKL_INT N,
								const MKL_INT K, const double alpha, const double *A,
								const MKL_INT lda, const double *B, const MKL_INT ldb,
								const double beta, double *C, const MKL_INT ldc)
{
	cblas_dgemm(Order,TransA,TransB,M,N,K,alpha,A,lda,B,ldb,beta,C,ldc);
}


////////////////////////////////////////////////////
// LAPACK
__forceinline MKL_INT mkl_getrf(MKL_INT m,MKL_INT n,float *a,MKL_INT lda,MKL_INT *ipiv)
{	MKL_INT ret;
	sgetrf(&m,&n,a,&lda,ipiv,&ret);
	return ret;
}
__forceinline MKL_INT mkl_getrf(MKL_INT m,MKL_INT n,double *a,MKL_INT lda,MKL_INT *ipiv)
{	MKL_INT ret;
	dgetrf(&m,&n,a,&lda,ipiv,&ret);
	return ret;
}
__forceinline MKL_INT mkl_getri(MKL_INT n,float *a,MKL_INT lda,MKL_INT *ipiv,float *work,MKL_INT lwork)
{	MKL_INT ret;
	sgetri(&n,a,&lda,ipiv,work,&lwork,&ret);
	return ret;
}
__forceinline MKL_INT mkl_getri(MKL_INT n,double *a,MKL_INT lda,MKL_INT *ipiv,double *work,MKL_INT lwork)
{	MKL_INT ret;
	dgetri(&n,a,&lda,ipiv,work,&lwork,&ret);
	return ret;
}
__forceinline MKL_INT mkl_gesv(MKL_INT n,MKL_INT nrhs,float *a,MKL_INT lda,MKL_INT *ipiv,float *b,MKL_INT ldb)
{	MKL_INT ret;
	sgesv(&n,&nrhs,a,&lda,ipiv,b,&ldb,&ret);
	return ret;
}
__forceinline MKL_INT mkl_gesv(MKL_INT n,MKL_INT nrhs,double *a,MKL_INT lda,MKL_INT *ipiv,double *b,MKL_INT ldb)
{	MKL_INT ret;
	dgesv(&n,&nrhs,a,&lda,ipiv,b,&ldb,&ret);
	return ret;
}
__forceinline MKL_INT mkl_gels(char trans,MKL_INT m,MKL_INT n,MKL_INT nrhs,float *a,MKL_INT lda,float *b,MKL_INT ldb,float *work,MKL_INT lwork)
{	MKL_INT ret;
	sgels(&trans,&m,&n,&nrhs,a,&lda,b,&ldb,work,&lwork,&ret);
	return ret;
}
__forceinline MKL_INT mkl_gels(char trans,MKL_INT m,MKL_INT n,MKL_INT nrhs,double *a,MKL_INT lda,double *b,MKL_INT ldb,double *work,MKL_INT lwork)
{	MKL_INT ret;
	dgels(&trans,&m,&n,&nrhs,a,&lda,b,&ldb,work,&lwork,&ret);
	return ret;
}
__forceinline MKL_INT mkl_gelss(MKL_INT m,MKL_INT n,MKL_INT nrhs,float *a,MKL_INT lda,float *b,MKL_INT ldb,float *s,float rcond,MKL_INT *rank,float *work,MKL_INT lwork)
{	MKL_INT ret;
	sgelss(&m,&n,&nrhs,a,&lda,b,&ldb,s,&rcond,rank,work,&lwork,&ret);
	return ret;
}
__forceinline MKL_INT mkl_gelss(MKL_INT m,MKL_INT n,MKL_INT nrhs,double *a,MKL_INT lda,double *b,MKL_INT ldb,double *s,double rcond,MKL_INT *rank,double *work,MKL_INT lwork)
{	MKL_INT ret;
	dgelss(&m,&n,&nrhs,a,&lda,b,&ldb,s,&rcond,rank,work,&lwork,&ret);
	return ret;
}
__forceinline MKL_INT mkl_syev(char jobz,char uplo,MKL_INT n,float *a,MKL_INT lda,float *w,float *work,MKL_INT lwork)
{	MKL_INT ret;
	ssyev(&jobz,&uplo,&n,a,&lda,w,work,&lwork,&ret);
	return ret;
}
__forceinline MKL_INT mkl_syev(char jobz,char uplo,MKL_INT n,double *a,MKL_INT lda,double *w,double *work,MKL_INT lwork)
{	MKL_INT ret;
	dsyev(&jobz,&uplo,&n,a,&lda,w,work,&lwork,&ret);
	return ret;
}
__forceinline MKL_INT mkl_syevx(char jobz,char range,char uplo,MKL_INT n,float *a,MKL_INT lda,float vl,float vu,MKL_INT il,MKL_INT iu,float abstol,MKL_INT* m,float *w,float *z,MKL_INT ldz,float *work,MKL_INT lwork,MKL_INT *iwork,MKL_INT *ifail)
{	MKL_INT ret;
	ssyevx(&jobz,&range,&uplo,&n,a,&lda,&vl,&vu,&il,&iu,&abstol,m,w,z,&ldz,work,&lwork,iwork,ifail,&ret);
	return ret;
}
__forceinline MKL_INT mkl_syevx(char jobz,char range,char uplo,MKL_INT n,double *a,MKL_INT lda,double vl,double vu,MKL_INT il,MKL_INT iu,double abstol,MKL_INT* m,double *w,double *z,MKL_INT ldz,double *work,MKL_INT lwork,MKL_INT *iwork,MKL_INT *ifail)
{	MKL_INT ret;
	dsyevx(&jobz,&range,&uplo,&n,a,&lda,&vl,&vu,&il,&iu,&abstol,m,w,z,&ldz,work,&lwork,iwork,ifail,&ret);
	return ret;
}
__forceinline MKL_INT mkl_syevr(char jobz,char range,char uplo,MKL_INT n,float *a,MKL_INT lda,float vl,float vu,MKL_INT il,MKL_INT iu,float abstol,MKL_INT *m,float *w,float *z,MKL_INT ldz,MKL_INT *isuppz,float *work,MKL_INT lwork,MKL_INT *iwork,MKL_INT liwork)
{	MKL_INT ret;
	ssyevr(&jobz,&range,&uplo,&n,a,&lda,&vl,&vu,&il,&iu,&abstol,m,w,z,&ldz,isuppz,work,&lwork,iwork,&liwork,&ret);
	return ret;
}
__forceinline MKL_INT mkl_syevr(char jobz,char range,char uplo,MKL_INT n,double *a,MKL_INT lda,double vl,double vu,MKL_INT il,MKL_INT iu,double abstol,MKL_INT *m,double *w,double *z,MKL_INT ldz,MKL_INT *isuppz,double *work,MKL_INT lwork,MKL_INT *iwork,MKL_INT liwork)
{	MKL_INT ret;
	dsyevr(&jobz,&range,&uplo,&n,a,&lda,&vl,&vu,&il,&iu,&abstol,m,w,z,&ldz,isuppz,work,&lwork,iwork,&liwork,&ret);
	return ret;
}
__forceinline MKL_INT mkl_gesvd(char jobu,char jobvt,MKL_INT m,MKL_INT n,float *a,MKL_INT lda,float *s,float *u,MKL_INT ldu,float *vt,MKL_INT ldvt,float *work,MKL_INT lwork)
{	MKL_INT ret;
	sgesvd(&jobu,&jobvt,&m,&n,a,&lda,s,u,&ldu,vt,&ldvt,work,&lwork,&ret);
	return ret;
}
__forceinline MKL_INT mkl_gesvd(char jobu,char jobvt,MKL_INT m,MKL_INT n,double *a,MKL_INT lda,double *s,double *u,MKL_INT ldu,double *vt,MKL_INT ldvt,double *work,MKL_INT lwork)
{	MKL_INT ret;
	dgesvd(&jobu,&jobvt,&m,&n,a,&lda,s,u,&ldu,vt,&ldvt,work,&lwork,&ret);
	return ret;
}
//__forceinline MKL_INT mkl_gees(char jobvs,char sort,MKL_INT *select,MKL_INT *n,float *a,MKL_INT lda,MKL_INT *sdim,float *wr,float *wi,float *vs,MKL_INT ldvs,float *work,MKL_INT lwork,MKL_INT *bwork)
//{	MKL_INT ret;
//	sgees(&jobvs,&sort,static_cast<MKL_INT*>(select),n,a,&lda,sdim,wr,wi,vs,&ldvs,work,&lwork,static_cast<MKL_INT*>(bwork),&ret);
//	return ret;
//}
//__forceinline MKL_INT mkl_gees(char jobvs,char sort,MKL_INT *select,MKL_INT *n,double *a,MKL_INT lda,MKL_INT *sdim,double *wr,double *wi,double *vs,MKL_INT ldvs,double *work,MKL_INT lwork,MKL_INT *bwork)
//{	MKL_INT ret;
//	dgees(&jobvs,&sort,static_cast<MKL_INT*>(select),n,a,&lda,sdim,wr,wi,vs,&ldvs,work,&lwork,static_cast<MKL_INT*>(bwork),&ret);
//	return ret;
//}






////////////////////////////////////////////////////
// VML functions
#define VML_DECL_2(name)																\
	__forceinline void vml##name(MKL_INT n, const float* a, float* r) { vs##name(n,a,r); }	\
	__forceinline void vml##name(MKL_INT n, const double* a, double* r) { vd##name(n,a,r); }\

VML_DECL_2(Inv)
VML_DECL_2(Sqrt)
VML_DECL_2(InvSqrt)
VML_DECL_2(Cbrt)
VML_DECL_2(InvCbrt)
VML_DECL_2(Exp)
VML_DECL_2(Ln)
VML_DECL_2(Log10)
VML_DECL_2(Sin)
VML_DECL_2(Cos)
VML_DECL_2(Tan)
VML_DECL_2(Sinh)
VML_DECL_2(Cosh)
VML_DECL_2(Tanh)
VML_DECL_2(Acos)
VML_DECL_2(Asin)
VML_DECL_2(Atan)
VML_DECL_2(Asinh)
VML_DECL_2(Acosh)
VML_DECL_2(Atanh)
VML_DECL_2(Erf)
VML_DECL_2(Erfc)

#undef VML_DECL_2

__forceinline void vmlAtan2(MKL_INT n, const float* a, const float* b, float* r){ vsAtan2(n,a,b,r); }
__forceinline void vmlAtan2(MKL_INT n, const double* a, const double* b, double* r){ vdAtan2(n,a,b,r); }

__forceinline void vmlDiv(MKL_INT n, const float* a, const float* b, float* r){ vsDiv(n,a,b,r); }
__forceinline void vmlDiv(MKL_INT n, const double* a, const double* b, double* r){ vdDiv(n,a,b,r); }

__forceinline void vmlPow(MKL_INT n, const float* a, const float* b, float* r){ vsPow(n,a,b,r); }
__forceinline void vmlPow(MKL_INT n, const double* a, const double* b, double* r){ vdPow(n,a,b,r); }

__forceinline void vmlPowx(MKL_INT n, const float* a, float b, float* r){ vsPowx(n,a,b,r); }
__forceinline void vmlPowx(MKL_INT n, const double* a, double b, double* r){ vdPowx(n,a,b,r); }

__forceinline void vmlSinCos(MKL_INT n, const float* a, float* r1, float* r2){ vsSinCos(n,a,r1,r2); }
__forceinline void vmlSinCos(MKL_INT n, const double* a, double* r1, double* r2){ vdSinCos(n,a,r1,r2); }





} // namespace mkl_cpp

} // namespace mkl



