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


#include "mkl_matrix.h"
#include <algorithm>

namespace mkl
{

template<typename t_Val = float, typename t_MatrixValue = float>
class LinearEquationSolver
{
protected:
	struct _NonZeroItem
	{	union
		{	ULONGLONG	_ordinal;
			struct{UINT i,j; };
		};
		t_Val	value;
		INLFUNC bool operator <(const _NonZeroItem& x)
		{	return _ordinal<x._ordinal;  
		}
	};
	struct _Value
	{	UINT	i;
		t_Val	value;
	};
protected:
	::rt::BufferEx<_NonZeroItem>	m_Entries;		// all non-zero entries

	UINT							m_VaribleCount;	// number of varibles to be solved
	UINT							m_EquationCount;// number of equation to be solved

	bool							m_StructIsDirty;
	::rt::Buffer<UINT>				m_ColumnIndex;	// refer to m_Entries, UINT[m_VaribleCount+1]
	::rt::Buffer<_Value>			m_Coefficients;	// all non-zero value

protected:
	void UpdateSparseStruct()
	{	ASSERT(m_StructIsDirty);
		if( m_Entries.GetSize() )
		{	if( !m_Coefficients.GetSize() ){}
			else // merge m_Coefficients to m_Entries
			{	SIZE_T NewEntry = m_Entries.GetSize();
				VERIFY(m_Entries.ChangeSize(NewEntry + m_Coefficients.GetSize()));
				_NonZeroItem* pEntry = &m_Entries[NewEntry];
				_Value* p = m_Coefficients;
				for(UINT j=0;j<m_ColumnIndex.GetSize();j++)
				{	_Value* pend = &m_Coefficients[m_ColumnIndex[j+1]];
					for(;p<pend;p++,pEntry++)
					{	pEntry->i = p->i;
						pEntry->value = p->value;
						pEntry->j = j;
					}
				}
			}
			std::sort(m_Entries.Begin(),m_Entries.End());
			UINT open = 0;
			for(UINT i=1;i<m_Entries.GetSize();i++)
			{	if(m_Entries[open]._ordinal == m_Entries[i]._ordinal)
				{
					m_Entries[open].value += m_Entries[i].value;
					continue;
				}

				open++;
				m_Entries[open] = m_Entries[i];
			}
			m_Entries.ChangeSize(open+1);

			m_Coefficients.SetSize(m_Entries.GetSize());
			_NonZeroItem*	pEntry = m_Entries;
			_NonZeroItem*	pEntryEnd = m_Entries.End();
			{	// update value
				_Value*			pValue = m_Coefficients;
				for(;pEntry<pEntryEnd;pEntry++,pValue++)
				{	pValue->i = pEntry->i;
					pValue->value = pEntry->value;
				}
			}
			{	// update m_ColumnIndex
				ASSERT(m_ColumnIndex[0] == 0);
				pEntry = m_Entries;
				for(UINT offset=1;pEntry<pEntryEnd;pEntry++,offset++)
					m_ColumnIndex[pEntry->j+1] = offset;
			}
		}
		m_Entries.SetSize();
		m_StructIsDirty = false;
	}

public:
	LinearEquationSolver()
	{	m_VaribleCount=m_EquationCount=0; 
		m_StructIsDirty  =true;
	}
	void DefineProblem(UINT equ_co, UINT x_co, UINT estimated_nonzeros = 100)
	{	ASSERT(x_co);
		ASSERT(equ_co);
		m_EquationCount = equ_co;
		m_VaribleCount = x_co;
		m_StructIsDirty = true;

		m_Entries.SetSize();
		m_Entries.reserve(rt::max(estimated_nonzeros,10U));

		m_Coefficients.SetSize();
		m_ColumnIndex.SetSize(x_co+1);
		m_ColumnIndex.Zero();	
	}

	//__forceinline LinearEquationSolver& AddCoefficient(UINT equation_i,UINT varible_j)
	//{	ASSERT( equation_i< m_EquationCount );
	//	ASSERT( varible_j< m_VaribleCount );
	//	_NonZeroItem& item = m_Entries.push_back();
	//	item.i = equation_i;
	//	item.j = varible_j;
	//	m_StructIsDirty = true;
	//	return *this;
	//}

	__forceinline LinearEquationSolver& AddCoefficient(UINT equation_i,UINT varible_j,const t_Val & value)
	{	ASSERT( equation_i< m_EquationCount );
		ASSERT( varible_j< m_VaribleCount );
		_NonZeroItem& item = m_Entries.push_back();
		item.i = equation_i;
		item.j = varible_j;
		item.value = value;
		m_StructIsDirty = true;
		return *this;
	}

	LinearEquationSolver& SetCoefficient(UINT equation_i,UINT varible_j,const t_Val & value)
	{	ASSERT( !m_StructIsDirty ); // call only after solve once
		_Value* p = &m_Coefficients[m_ColumnIndex[varible_j]];
		_Value* pend = &m_Coefficients[m_ColumnIndex[varible_j+1]]-1;
		for(;p<=pend;)
		{	if(p->i == equation_i)
			{	p->value=value; return *this; }
		}
		ASSERT(0); // not exist, you can not change the sparse struct via this function
	}

	template<typename t_Val2>
	void ComputeLeftHand(::mkl::Matrix<t_Val2>& AT_A)	// A'*A
	{	if(m_StructIsDirty)UpdateSparseStruct();
		VERIFY(AT_A.SetSize(m_VaribleCount,m_VaribleCount));
		AT_A.Zero();

		for(UINT i=0;i<m_VaribleCount;i++)
		{	UINT count = m_ColumnIndex[i+1] - m_ColumnIndex[i];
			if(count)
			{	// col[i]*col[i]
				{	t_Val2& dst = AT_A(i,i);
					const _Value* p = &m_Coefficients[m_ColumnIndex[i]];
					const _Value* pend = &m_Coefficients[m_ColumnIndex[i+1]];
					for(;p<pend;p++)
						dst += ::rt::Sqr(p->value);
				}

				for(UINT j=i+1;j<m_VaribleCount;j++)
				{	const _Value* pi = &m_Coefficients[m_ColumnIndex[i]];
					const _Value* piend = &m_Coefficients[m_ColumnIndex[i+1]];
					const _Value* pj = &m_Coefficients[m_ColumnIndex[j]];
					const _Value* pjend = &m_Coefficients[m_ColumnIndex[j+1]];
					t_Val2& dst = AT_A(i,j);

					for(;;)
					{	if(pj->i < pi->i)
						{	pj++;	if(pj<pjend){}else break; }
						else if(pj->i > pi->i)
						{	pi++;	if(pi<piend){}else break; }
						else // matched
						{	dst += pi->value*pj->value;
							pi++; pj++;
							if(pi<piend && pj<pjend){}
							else break;
						}
					}

					AT_A(j,i) = dst;
				}
			}
		}
	}
	template<typename t_Val2, typename t_Val3>
	void ComputeRightHand(const ::mkl::Vector<t_Val2>& B, ::mkl::Vector<t_Val3>& AT_B)	// A'*B
	{	ASSERT(B.GetSize() == m_EquationCount);
		VERIFY(AT_B.SetSize(m_VaribleCount));
		AT_B.Zero();

		if(m_StructIsDirty)UpdateSparseStruct();

		_Value* p = m_Coefficients;
		for(UINT j=0;j<m_VaribleCount;j++)
		{	_Value* pend = &m_Coefficients[m_ColumnIndex[j+1]];
			t_Val3 & dst = AT_B[j];
			for(;p<pend;p++)
				dst += p->value*B[p->i];
		}
	}
	template<typename t_Val2, typename t_Val3>
	void ComputeRightHand(const ::mkl::Matrix<t_Val2>& B, ::mkl::Matrix<t_Val3>& AT_B)	// A'*B
	{	ASSERT( B.GetSize_Col());
		ASSERT(	B.GetSize_Row() == m_EquationCount );
		AT_B.SetSize(m_VaribleCount,B.GetSize_Col());
		AT_B.Zero();

		if(m_StructIsDirty)UpdateSparseStruct();

		_Value* p = m_Coefficients;
		for(UINT j=0;j<m_VaribleCount;j++)
		{	typename ::mkl::CMatrix<t_Val3,t_BaseVec3>::t_RowRef& dst_row = AT_B.GetRow(j);
			_Value* pend = m_Coefficients[m_ColumnIndex[j+1]];
			for(;p<pend;p++)
				dst_row.Add_Scaled( p->value, B.GetRow(p->i) );
		}
	}

	template<typename t_Val2, typename t_Val3>
	bool Solve(const ::mkl::Vector<t_Val2>& B, ::mkl::Vector<t_Val3>& X)	// A'A*X = A'B
	{	if( !X.SetSize(m_VaribleCount) ){ return false; }
		ComputeRightHand(B,X);
		
		::mkl::Matrix<t_MatrixValue>	AT_A;
		if( !AT_A.SetSize(m_VaribleCount,m_VaribleCount) ){ return false; }
		ComputeLeftHand(AT_A);

		return AT_A.SolveLinearLeastSquares_LQR(X);
	}

	template<typename t_Val2, typename t_Val3>
	bool Solve(const ::mkl::Matrix<t_Val2>& B, ::mkl::Matrix<t_Val3>& AT_B) // A'A*X = A'B
	{	if( !X.SetSize(m_VaribleCount,B.GetSize_Col()) ){ return false; }
		ComputeRightHand(B,X);
		
		::mkl::CMatrix<t_MatrixValue>	AT_A;
		if( !AT_A.SetSize(m_VaribleCount,m_VaribleCount) ){ return false; }
		ComputeLeftHand(AT_A);

		return AT_A.SolveLinearLeastSquares_LQR(X);
	}
};

} // namespace mkl