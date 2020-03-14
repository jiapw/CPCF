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

class RandomNumberGenerator
{
protected:
	VSLStreamStatePtr	pStreamObj;
	int					BRNG_Id;
	void				_ReleaseRNStream()
	{	if( pStreamObj )
		{	vslDeleteStream( &pStreamObj );
			pStreamObj = nullptr;
	}	}
public:
	enum 
	{								// Properties:  Func	Test		Error	Period
		BRNG_MCG31 =	VSL_BRNG_MCG31,        //	SS		All Ok 		10-20%	2.10E+09    // Default
		BRNG_R250  =	VSL_BRNG_R250,		   //	NN		3 Fails		0-80%	1.80E+75
		BRNG_MRG32K3A = VSL_BRNG_MRG32K3A,	   //	SN		All Ok 		0-20%	3.10E+57    // Intel recommanded
		BRNG_MCG59 = 	VSL_BRNG_MCG59,		   //	SS		1.5 Fails 	0-45%	1.40E+17
		BRNG_WH    =	VSL_BRNG_WH			   //	SS		1 Fails		0-60%	1.20E+24
	};
	RandomNumberGenerator(int _BRNG_Id = BRNG_MCG31){ pStreamObj = nullptr; ResetBRGN(0,_BRNG_Id); }
	~RandomNumberGenerator(){ _ReleaseRNStream(); };
	RandomNumberGenerator(RandomNumberGenerator& in)  // the copy constructor
	{	BRNG_Id = in.BRNG_Id;
		if( in.pStreamObj )
			vslCopyStream(&pStreamObj,in.pStreamObj);
		else
			pStreamObj = nullptr;
	}

	void ResetBRGN(DWORD seed = 0, int New_BRNG_Id = -1 )
	{	_ReleaseRNStream();
		if( New_BRNG_Id >=0 )BRNG_Id = New_BRNG_Id;
		vslNewStream( &pStreamObj, BRNG_Id,seed?seed:((DWORD)time(NULL)) );
		ASSERT( pStreamObj );
	}
	
	INLFUNC void UniformSequence(float *r, MKL_SIZE len, float LowBoundary,float HighBoundary){ vsRngUniform(0,pStreamObj,len,r,LowBoundary,HighBoundary); }
	INLFUNC void GaussianSequence(float *r, MKL_SIZE len,float Mean,float Sigma){ vsRngGaussian(VSL_METHOD_SGAUSSIAN_BOXMULLER,pStreamObj,len,r,Mean,Sigma); }
	INLFUNC void LaplaceSequence(float *r, MKL_SIZE len,float A,float Beta){ vsRngLaplace(0,pStreamObj,len,r,A,Beta); }
	INLFUNC void WeibullSequence(float *r, MKL_SIZE len,float alpha, float A,float beta){ vsRngWeibull(0,pStreamObj,len,r,alpha,A,beta); }
	INLFUNC void CauchySequence(float *r, MKL_SIZE len,float A,float beta){ vsRngCauchy(0,pStreamObj,len,r,A,beta); }
	INLFUNC void RayleighSequence(float *r, MKL_SIZE len,float A,float beta){ vsRngRayleigh(0,pStreamObj,len,r,A,beta); }
	INLFUNC void LognormalSequence(float *r, MKL_SIZE len,float A,float sigma,float B,float beta){ vsRngLognormal(0,pStreamObj,len,r,A,sigma,B,beta); }
	INLFUNC void GumbelSequence(float *r, MKL_SIZE len,float A,float beta){ vsRngGumbel(0,pStreamObj,len,r,A,beta); }
	INLFUNC void UniformSequence(double *r, MKL_SIZE len,double LowBoundary,double HighBoundary){ vdRngUniform(0,pStreamObj,len,r,LowBoundary,HighBoundary); }
	INLFUNC void GaussianSequence(double *r, MKL_SIZE len,double Mean,double Sigma){ vdRngGaussian(VSL_METHOD_SGAUSSIAN_BOXMULLER,pStreamObj,len,r,Mean,Sigma); }
	INLFUNC void LaplaceSequence(double *r, MKL_SIZE len,double A,double Beta){ vdRngLaplace(0,pStreamObj,len,r,A,Beta); }
	INLFUNC void WeibullSequence(double *r, MKL_SIZE len,double alpha, double A,double beta){ vdRngWeibull(0,pStreamObj,len,r,alpha,A,beta); }
	INLFUNC void CauchySequence(double *r, MKL_SIZE len,double A,double beta){ vdRngCauchy(0,pStreamObj,len,r,A,beta); }
	INLFUNC void RayleighSequence(double *r, MKL_SIZE len,double A,double beta){ vdRngRayleigh(0,pStreamObj,len,r,A,beta); }
	INLFUNC void LognormalSequence(double *r, MKL_SIZE len,double A,double sigma,double B,double beta){ vdRngLognormal(0,pStreamObj,len,r,A,sigma,B,beta); }
	INLFUNC void GumbelSequence(double *r, MKL_SIZE len,double A,double beta){ vdRngGumbel(0,pStreamObj,len,r,A,beta); }
};




} // namespace mkl




