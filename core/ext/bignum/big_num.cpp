#include "big_num.h"
#include "../../os/kernel.h"


#ifdef PLATFORM_MAC
#include <immintrin.h>
#endif

namespace ext
{

#define SIGNS_A_B(a,b)	((((int)a.GetSign())<<1) + (int)b.GetSign())
#define SIGNS_POS_POS	0
#define SIGNS_POS_NEG	1
#define SIGNS_NEG_POS	2
#define SIGNS_NEG_NEG	3

namespace _details
{

// no considering on sign
// assert no carry
void BN_LeftShift64(BN_Dyn& b, UINT s)
{
	if(0 == s || b.GetLength() == 0)
		return;

	ASSERT(s < 64);
	ASSERT(0 == ((~0ULL << (64 - s)) & b._Data[b.GetLength() - 1]));

	UINT i = b.GetLength() - 1;
	for (; 0 < i; i--) {
		b._Data[i] = (b._Data[i] << s) | (b._Data[i - 1] >> (64 - s));
	}
	b._Data[i] <<= s;
}

// no considering on sign
void BN_RightShift64(BN_Dyn& b, UINT s)
{
	if(0 == s || b.GetLength() == 0)
		return;

	ASSERT(s < 64);
	UINT i = 0;
	for (; i < b.GetLength() - 1; i++) {
		b._Data[i] = (b._Data[i] >> s) | (b._Data[i + 1] << (64 - s));
	}
	b._Data[i] >>= s;
}


bool BN_AbsLess(const BN_Unsigned& a, const BN_Unsigned& b) // a < b
{
	if(a.Data() == b.Data())return false;

	ASSERT(a.IsLeadingZeroTrimmed());
	ASSERT(b.IsLeadingZeroTrimmed());

	if(a.Len < b.Len)return true;
	if(a.Len == b.Len)
	{
		for(int i=a.Len - 1; i>=0; i--)
		{	
			if(a.pData[i] < b.pData[i])return true;
			if(a.pData[i] > b.pData[i])return false;
		}
	}

	return false;
}

bool BN_AbsLessOrEqual(const BN_Unsigned& a, const BN_Unsigned& b) // a < b
{
	if(a.Data() == b.Data())return true;

	ASSERT(a.IsLeadingZeroTrimmed());
	ASSERT(b.IsLeadingZeroTrimmed());

	if(a.Len < b.Len)return true;
	if(a.Len == b.Len)
	{
		for(int i=a.Len - 1; i>=0; i--)
		{	
			if(a.pData[i] < b.pData[i])return true;
			if(a.pData[i] > b.pData[i])return false;
		}

		return true;
	}

	return false;
}

bool BN_AbsEqual(const BN_Unsigned& a, const BN_Unsigned& b) // a == b
{
	if(a.Data() == b.Data())return true;

	ASSERT(a.IsLeadingZeroTrimmed());
	ASSERT(b.IsLeadingZeroTrimmed());

	if(a.Len != b.Len)return false;
	for(UINT i=0; i<a.Len; i++)
		if(a.pData[i] != b.pData[i])return false;

	return true;
}

int  BN_AbsCompare(const BN_Unsigned& a, const BN_Unsigned& b) // a < b: -1, a == b:0, a > b:1
{
	if(a.Data() == b.Data())return 0;

	ASSERT(a.IsLeadingZeroTrimmed());
	ASSERT(b.IsLeadingZeroTrimmed());

	if(a.Len > b.Len)return 1;
	if(a.Len < b.Len)return -1;

	for(int i=a.Len - 1; i>=0; i--)
	{	
		if(a.pData[i] < b.pData[i])return -1;
		if(a.pData[i] > b.pData[i])return 1;
	}

	return 0;
}

bool BN_Less(const BN_Ref& a, const BN_Ref& b) // a < b
{
	if(&a == &b)return false;

	ASSERT(a.IsLeadingZeroTrimmed());
	ASSERT(b.IsLeadingZeroTrimmed());

	switch(SIGNS_A_B(a,b))
	{
	case SIGNS_POS_POS: return BN_AbsLess(a, b);
	case SIGNS_POS_NEG: return false;
	case SIGNS_NEG_POS: return true;
	case SIGNS_NEG_NEG: return BN_AbsLessOrEqual(b, a);
	}

	return false;
}

bool BN_Equal(const BN_Ref& a, const BN_Ref& b) // a == b
{
	if(&a == &b)return true;

	ASSERT(a.IsLeadingZeroTrimmed());
	ASSERT(b.IsLeadingZeroTrimmed());

	return	(a.GetSign() == b.GetSign() && BN_AbsEqual(a,b)) ||
			(a.GetLength() == 0 && b.GetLength() == 0);
}

bool BN_LessOrEqual(const BN_Ref& a, const BN_Ref& b) // a <= b
{
	if(&a == &b)return true;

	ASSERT(a.IsLeadingZeroTrimmed());
	ASSERT(b.IsLeadingZeroTrimmed());

	switch(SIGNS_A_B(a,b))
	{
	case SIGNS_POS_POS: return BN_AbsLessOrEqual(a, b);
	case SIGNS_POS_NEG: return a.GetLength() == 0 && b.GetLength() == 0;
	case SIGNS_NEG_POS: return true;
	case SIGNS_NEG_NEG: return BN_AbsLess(b, a);
	}

	return false;
}

int BN_Compare(const BN_Ref& a, const BN_Ref& b) // a < b: -1, a == b:0, a > b:1
{
	if(&a == &b)return 0;

	ASSERT(a.IsLeadingZeroTrimmed());
	ASSERT(b.IsLeadingZeroTrimmed());

	if(a.GetLength() == 0 && b.GetLength() == 0)return 0;

	switch(SIGNS_A_B(a,b))
	{
	case SIGNS_POS_POS: return BN_AbsCompare(a, b);
	case SIGNS_POS_NEG: return 1;
	case SIGNS_NEG_POS: return -1;
	case SIGNS_NEG_NEG: return BN_AbsCompare(b, a);
	}

	return 0;
}

template<bool Inplace>
bool BN_Increase(const BN_Unsigned& b, const BN_Unsigned& ret) // return carry
{
	ASSERT(b.Len == ret.Len);
	if(b.Len == 0)return true;

	UINT i=0;
	for(; i<b.Len; i++)
		if(b.pData[i] != 0xffffffffffffffffULL)
		{	ret.pData[i] = b.pData[i] + 1;
			if(!Inplace)
			{	i++;
				memcpy(ret.pData + i, b.pData + i, (b.Len - i)*BN_BLK_SIZE);
			}
			return false;
		}
		else
		{	ret.pData[i] = 0;
		}
	
	return true;
}

template<bool Inplace>
bool BN_Decrease(const BN_Unsigned& b, const BN_Unsigned& ret) // return borrow
{
	ASSERT(b.Len == ret.Len);
	if(b.Len == 0)return true;

	UINT i=0;
	for(; i<b.Len; i++)
		if(b.pData[i])
		{	ret.pData[i] = b.pData[i] - 1;
			if(!Inplace)
			{	i++;
				memcpy(ret.pData + i, b.pData + i, (b.Len - i)*BN_BLK_SIZE);
			}
			return false;
		}
		else
		{	ret.pData[i] = ~(BN_BLK)0;
		}
	
	return true;
}

void BN_Dyn::TrimLeadingZero()
{
	UINT len = (UINT)_Data.GetSize();
	while(len && _Data[len-1] == 0)
		len--;

	_Data.ShrinkSize(len);
}

void BN_Dyn::LeftShift(UINT a)
{
	ASSERT(_Data.Last());
	if(GetLength() == 0)return;

	int s = a%BN_BLK_BITSIZE;
	int Exponent = a/BN_BLK_BITSIZE;

	if(s)
	{
		_Data.push_both(0, Exponent, (_Data.Last() & (~(0ULL)<<(BN_BLK_BITSIZE - s)))?1:0);
		BN_LeftShift64(*this, s);
		TrimLeadingZero();
	}
	else
	{
		if(Exponent)
			_Data.push_front(0, Exponent);
	}
}

void BN_Dyn::RightShift(UINT a)
{
	ASSERT(_Data.Last());
	if(GetLength() == 0)return;

	int s = a%BN_BLK_BITSIZE;
	UINT Exponent = a/BN_BLK_BITSIZE;

	if(Exponent)
		_Data.erase(0, Exponent);

	if(s)
	{
		BN_RightShift64(*this, s);
		TrimLeadingZero();
	}
}

void BN_ToString(const BN_Ref& a, rt::String& append, int base)
{
	
	if(base == 16)
	{
		if(a.GetLength())
		{
			UINT org_len = (UINT)append.GetLength();

			rt::tos::Base16 x(a.pData, a.Len*BN_BLK_SIZE);

			UINT co = (UINT)x.GetLength()/2;
			WORD* s = (WORD*)(x.End()-2);
			while(co && *s == 0x3030)
			{
				co--;
				s--;
			}

			WORD* p;
			if(a.GetSign())
			{
				append.SetLength(3 + co*2 + org_len);
				*(DWORD*)(append.Begin() + org_len) = 0x78302d;
				p = (WORD*)(append.Begin()+3 + org_len);
			}
			else
			{	
				append.SetLength(2 + co*2 + org_len);
				*(WORD*)(append.Begin() + org_len) = 0x7830;
				p = (WORD*)(append.Begin()+2 + org_len);
			}
		
			for(UINT i=0; i<co; i++, p++, s--)
				*p = *s;
		}
		else
		{
			append += rt::SS("0x00");
		}
	}
	else
	{	
		ASSERT(base == 10);

		if(a.GetLength())
		{
			ext::BigNumMutable w, b;
			if(a.GetSign())append += '-';

			UINT org_len = (UINT)append.GetLength();

			UINT r = 0;
			BN_Div(a, 10U, &r, w);

			append += (char)('0' + r);
			while(w.GetLength())
			{
				b.Div(w, 10U, &r);
				append += (char)('0' + r);
				rt::Swap(w, b);
			}

			append.TrimLeft(org_len).Flip();
		}
		else
		{
			append += rt::SS("0");
		}
	}
}

void BN_AbsAdd(const BN_Unsigned& a, const BN_Unsigned& b, ext::BigNumMutable& ret)
{
	if(a.Len < b.Len)
		rt::Swap(a, b);

	ret.SetLength(a.Len);

	BYTE carry = 0;
	UINT i=0;
	for(; i<b.Len; i++)
		carry = rt::AddCarry(carry, a.pData[i], b.pData[i], &ret._Data[i]);

	if(carry)
	{
		carry = BN_Increase<false>(a.SubVal(b.Len), ret.SubVal(b.Len, a.Len - b.Len));
		if(carry)ret.AppendLast() = 1;
	}
	else
	{
		memcpy(&ret._Data[b.Len], &a.pData[b.Len], (a.Len - b.Len)*BN_BLK_SIZE);
		ret.TrimLeadingZero();
	}
}

void BN_AbsAdd(const BN_Unsigned& b, ext::BigNumMutable& ret)
{
	ret.ExtendLength(b.Len);

	BN_BLK* p = ret._Data;

	BYTE carry = 0;
	UINT i=0;
	for(; i<b.Len; i++)
		carry = rt::AddCarry(carry, p[i], b.pData[i], p + i);

	if(carry)
	{
		carry = BN_Increase<true>(ret.SubVal(b.Len, ret.GetLength() - b.Len), ret.SubVal(b.Len, ret.GetLength() - b.Len));
		if(carry)ret.AppendLast() = 1;
	}
	else
	{
		ret.TrimLeadingZero();
	}
}

void BN_AbsSub(const BN_Unsigned& a, const BN_Unsigned& b, ext::BigNumMutable& ret)
{
	ASSERT(BN_AbsLessOrEqual(b, a));
	ret.SetLength(a.Len);

	BN_BLK* p = ret._Data;

	BYTE borrow = 0;
	UINT i=0;
	for(; i<b.Len; i++)
		borrow = rt::SubBorrow(borrow, a.pData[i], b.pData[i], p + i);

	if(borrow)
	{
		VERIFY(0 == BN_Decrease<false>(a.SubVal(b.Len), ret.SubVal(b.Len)));
	}
	else
	{
		memcpy(&ret._Data[b.Len], &a.pData[b.Len], (a.Len - b.Len)*BN_BLK_SIZE);
	}

	ret.TrimLeadingZero();
}

void BN_AbsSub(const BN_Unsigned& b, ext::BigNumMutable& ret)
{
	if(BN_AbsLessOrEqual(b, ret))
	{
		BN_BLK* p = ret._Data;

		BYTE borrow = 0;
		UINT i=0;
		for(; i<b.Len; i++)
			borrow = rt::SubBorrow(borrow, p[i], b.pData[i], p + i);

		if(borrow)
		{
			VERIFY(0 == BN_Decrease<true>(ret.SubVal(b.Len, ret.GetLength() - b.Len), ret.SubVal(b.Len, ret.GetLength() - b.Len)));
		}
	}
	else
	{	
		ret.ExtendLength(b.Len);
		BN_BLK* p = ret._Data;

		BYTE borrow = 0;
		UINT i=0;
		for(; i<b.Len; i++)
			borrow = rt::SubBorrow(borrow, b.pData[i], p[i], p + i);

		ASSERT(borrow == 0);
	}

	ret.TrimLeadingZero();
}

void BN_Add(const BN_Ref& b, ext::BigNumMutable& ret) // ret += b
{
	switch(SIGNS_A_B(ret,b))
	{	
	case SIGNS_POS_POS:	
		_details::BN_AbsAdd(b, ret);
		ret.SetSign(false);
		break;
	case SIGNS_POS_NEG: BN_Sub(b.Abs(), ret); break;
	case SIGNS_NEG_POS: BN_Sub(BN_Ref(true, b.pData, b.Len), ret); break;
		break;
	case SIGNS_NEG_NEG: 
		_details::BN_AbsAdd(b, ret);
		ret.SetSign(true);
		break;
	}
}

void BN_Sub(const BN_Ref& b, ext::BigNumMutable& ret) // ret -= b
{
	switch(SIGNS_A_B(ret,b))
	{	
	case SIGNS_POS_POS:	
		{
			bool sign = BN_AbsLess(ret, b);
			_details::BN_AbsSub(b, ret);
			ret.SetSign(sign);
		}
		break;
	case SIGNS_POS_NEG:
		BN_AbsAdd(b, ret);
		ret.SetSign(false);
		break;
	case SIGNS_NEG_POS:
		BN_AbsAdd(b, ret);
		ret.SetSign(true); break;
		break;
	case SIGNS_NEG_NEG:
		{	
			bool sign = !BN_AbsLess(ret, b);
			_details::BN_AbsSub(b, ret);
			ret.SetSign(sign);
		}
		break;
	}
}

void BN_Add(const BN_Ref& a, const BN_Ref& b, ext::BigNumMutable& ret) // ret = a + b
{
	switch(SIGNS_A_B(a,b))
	{	
	case SIGNS_POS_POS:	
		_details::BN_AbsAdd(a, b, ret);
		ret.SetSign(false);
		break;
	case SIGNS_POS_NEG: BN_Sub(a, b.Abs(), ret); break;
	case SIGNS_NEG_POS: BN_Sub(a.Abs(), b, ret); break;
	case SIGNS_NEG_NEG: 
		_details::BN_AbsAdd(b, a, ret);
		ret.SetSign(true);
		break;
	}
}

void BN_Sub(const BN_Ref& a, const BN_Ref& b, ext::BigNumMutable& ret) // ret = a - b
{
	switch(SIGNS_A_B(a,b))
	{	
	case SIGNS_POS_POS:	
		if(BN_Less(a, b))
		{	ret.SetSign(true);
			_details::BN_AbsSub(b, a, ret);
		}
		else
		{	ret.SetSign(false);
			_details::BN_AbsSub(a, b, ret);
		}
		break;
	case SIGNS_POS_NEG:
		ret.SetSign(false);
		_details::BN_AbsAdd(a, b, ret);
		break;
	case SIGNS_NEG_POS:
		ret.SetSign(false);
		_details::BN_AbsAdd(a, b, ret);
		break;
	case SIGNS_NEG_NEG:	
		if(BN_AbsLess(a, b))
		{	
			ret.SetSign(false);
			_details::BN_AbsSub(b, a, ret);
		}
		else
		{	ret.SetSign(false);
			_details::BN_AbsSub(a, b, ret);
		}
		break;
	}
}

UINT BN_Mantissa32(const BN_Unsigned& b, int* exp)
{
	if(b.GetLength() == 0)
	{
		*exp = 0;
		return 0;
	}

	auto* p = (const UINT*)b.Data();
	int  len = (int)b.GetLength()*sizeof(BN_BLK)/sizeof(UINT);
	if(sizeof(BN_BLK) == 8 && p[len-1] == 0)len--;

	int lb = rt::LeadingZeroBits(p[len-1]);
	UINT ret = p[len-1]<<lb;
	if(len>1)ret |= p[len-2]>>(32 - lb);

	*exp = (len-1)*32 - lb;
	return ret;
}

ULONGLONG BN_Mantissa64(const BN_Unsigned& b, int* exp)
{
	if(b.GetLength() == 0)
	{
		*exp = 0;
		return 0;
	}

	auto* p = (const UINT*)b.Data();
	int   len = (int)b.GetLength()*sizeof(BN_BLK)/sizeof(UINT);
	if(sizeof(BN_BLK) == 8 && p[len-1] == 0)len--;

	int lb = rt::LeadingZeroBits(p[len-1]);
	ULONGLONG ret = ((ULONGLONG)p[len-1])<<(lb + 32);
	if(len>1)ret |= ((ULONGLONG)p[len-2])<<lb;
	if(len>2)ret |= ((ULONGLONG)p[len-3])>>(32 - lb);

	*exp = (len-2)*32 - lb;
	return ret;
}

void BN_AbsMul(const BN_Unsigned& a, UINT b_in, ext::BigNumMutable& ret)
{
	if(b_in == 0)
	{	ret.SetZero();
		return;
	}

	ret.SetLength(a.Len + 1);
	ret.FillZero();

	auto* c = (UINT*)ret._Data.Begin();
	auto* pa = (UINT*)a.pData;
	UINT a_len = a.Len*(sizeof(BN_BLK)/sizeof(UINT));

	ULONGLONG b = b_in;

	for(UINT i=0; i<a_len; i++)
	{
		auto* v = (ULONGLONG *)(c + i);
		v[1] += rt::AddCarry(0, *v, pa[i]*b, v);
	}

	ret.TrimLeadingZero();
}

void BN_AbsMul(const BN_Unsigned& a, ULONGLONG b_in, ext::BigNumMutable& ret) // ret = a*b
{
	if(b_in == 0)
	{	ret.SetZero();
		return;
	}

	if(b_in <= 0xffffffffULL)
	{	BN_AbsMul(a, (UINT)b_in, ret);
		return;
	}

	ret.SetLength(a.Len + sizeof(ULONGLONG)/sizeof(UINT));
	ret.FillZero();

	auto* c = (UINT*)ret._Data.Begin();
	auto* pa = (UINT*)a.pData;
	UINT a_len = a.Len*(sizeof(BN_BLK)/sizeof(UINT));

	ULONGLONG b1 = (UINT)b_in;
	ULONGLONG b2 = (UINT)(b_in>>32);

	for(UINT i=0; i<a_len; i++)
	{
		auto* v = (ULONGLONG *)(c + i);
		v[1] += rt::AddCarry(0, *v, pa[i]*b1, v);

		v = (ULONGLONG *)(c + i + 1);
		v[1] += rt::AddCarry(0, *v, pa[i]*b2, v);
	}

	ret.TrimLeadingZero();
}

void BN_AbsMul(UINT b_in, ext::BigNumMutable& ret)
{
	if(b_in == 0)
	{	ret.SetZero();
		return;
	}

	ret.AppendLast() = 0;
	UINT a_len = ret.GetLength()*(sizeof(BN_BLK)/sizeof(UINT));

	auto* c = (UINT*)ret._Data.Begin();

	ULONGLONG b = b_in;
	ULONGLONG carry = 0;
	for(UINT i=0; i<a_len; i++)
	{
		ULONGLONG muli;
		carry = ((ULONGLONG)rt::AddCarry(0, carry, c[i]*b, &muli))<<32;
		carry += (muli >> 32);

		c[i] = (UINT)muli;
	}

	ASSERT(carry == 0);
	ret.TrimLeadingZero();
}

void BN_Mul(const BN_Ref& a, const BN_Ref& b, ext::BigNumMutable& ret)
{
	ret.SetLength(a.Len + b.Len + 1);
	ret.FillZero();

	UINT* c = (UINT*)ret._Data.Begin();
	UINT* pa = (UINT*)a.pData;
	UINT a_len = a.Len*(sizeof(BN_BLK)/sizeof(UINT));

	UINT* pb = (UINT*)b.pData;
	UINT b_len = b.Len*(sizeof(BN_BLK)/sizeof(UINT));

	auto carries = (UINT*)_alloca(sizeof(UINT)*(b_len+a_len+2));
	rt::Zero(carries, sizeof(UINT)*(b_len+a_len+2));

	for(UINT i=0; i<a_len; i++)
	{
		ULONGLONG pa_i = pa[i];
		for(UINT j=0; j<b_len; j++)
		{
			auto* v = (ULONGLONG*)(c + i + j);
			carries[i + j + 2] += rt::AddCarry(0, *v, pa_i*pb[j], v);
		}
	}

	{	int carry = 0;
		for(UINT i=0; i<b_len+a_len+2; i++)
		{
			carry = rt::AddCarry(carry, carries[i], c[i], c + i);
		}
		ASSERT(carry == 0);
	}

	ret.SetSign(a.GetSign() != b.GetSign());
	ret.TrimLeadingZero();
}


void BN_Mul(const BN_Ref& a, const NativeFloat& b, ext::BigNumMutable& ret)
{
	if(b.Mantissa == 0 || a.GetLength()*BN_BLK_BITSIZE + b.Exponent + 32 <= 0)
	{
		ret.SetZero();
		return;
	}

	int lz = rt::LeadingZeroBits(b.Mantissa);
	if(lz >= 32)
	{
		BN_AbsMul(a, (UINT)b.Mantissa, ret);
		ret <<= b.Exponent;
	}
	else
	{
		int shift = 32 - lz;
		BN_AbsMul(a, (UINT)(b.Mantissa>>shift), ret);
		ret <<= (b.Exponent + shift);
	}
	
	ret.SetSign(a.GetSign() != b.Sign);
}

void BN_Mul(const NativeFloat& b, ext::BigNumMutable& ret)
{
	if(b.Mantissa == 0 || ret.GetLength()*BN_BLK_BITSIZE + b.Exponent + 32 <= 0)
	{
		ret.SetZero();
		return;
	}

	int lz = rt::LeadingZeroBits(b.Mantissa);
	if(lz >= 32)
	{
		BN_AbsMul((UINT)b.Mantissa, ret);
		ret <<= b.Exponent;
	}
	else
	{
		int shift = 32 - lz;
		BN_AbsMul((UINT)(b.Mantissa>>shift), ret);
		ret <<= (b.Exponent + shift);
	}
	
	ret.SetSign(ret.GetSign() != b.Sign);
}


bool BN_Div(const BN_Unsigned& a, UINT b_in, UINT* reminder, ext::BigNumMutable& ret) // ret = (a - *reminder)/b
{
	if(b_in == 0)return false;

	ret.SetLength(a.Len);

	auto* r = (UINT*)ret.Data();
	auto* s = (const UINT*)a.pData;

	ULONGLONG carried = 0;
	ULONGLONG b = b_in;
	for(int i=a.Len*(sizeof(BN_BLK)/sizeof(UINT))-1; i>=0; i--)
	{
		ULONGLONG v = s[i] | carried;
		r[i] = (UINT)(v/b);
		((UINT*)&carried)[1] = (UINT)(v%b);
	}

	ret.TrimLeadingZero();
	if(reminder)*reminder = ((UINT*)&carried)[1];

	return true;
}

float BN_2_float(const BN_Ref& x)
{
	if(x.GetLength() == 0)return 0;

	int exp;
	UINT m = BN_Mantissa32(x, &exp);
	ASSERT(m);

	DWORD y = (((UINT)x.GetSign())<<31) | (0x007fffff&(m>>8)) | (0x7f800000&((exp + 127 + 32 - 1) << 23));
	return (float&)y;
}

double BN_2_double(const BN_Ref& x)
{
	if(x.GetLength() == 0)return 0;
	
	int exp;
	ULONGLONG m = BN_Mantissa64(x, &exp);
	ASSERT(m);

	ULONGLONG y = (((ULONGLONG)x.GetSign())<<63) | (0xfffffffffffffULL&(m>>11)) | (0x7ff0000000000000ULL&(((ULONGLONG)(exp + 1023 + 64 - 1)) << 52));
	return (double&)y;
}

NativeFloat::NativeFloat(float x_in)
{	// https://en.wikipedia.org/wiki/Single-precision_floating-point_format
	DWORD& x = *(DWORD*)&x_in;
	Mantissa = (x&0x007fffff)|0x800000;
	Exponent = ((x&0x7f800000)>>23) - 127 - 23;
	Sign = x&0x80000000;
}

NativeFloat::NativeFloat(double x_in)
{	// https://en.wikipedia.org/wiki/Double-precision_floating-point_format
	ULONGLONG& x = *(ULONGLONG*)&x_in;

	Mantissa = (x&0xfffffffffffffULL)|0x10000000000000ULL;
	Exponent = ((x&0x7ff0000000000000ULL)>>52) - 1023 - 52;
	Sign = x&0x8000000000000000ULL;
}

} // namespace _details


void BigNumMutable::_FromNativeFloat(const _details::NativeFloat& x)
{
	if(x.Mantissa)
	{
		SetSign(x.Sign);

		SetLength(sizeof(ULONGLONG)/sizeof(_details::BN_BLK));
		*(ULONGLONG*)&_Data[0] = x.Mantissa;
		if(sizeof(ULONGLONG)/sizeof(_details::BN_BLK)>1 && _Data[1] == 0)
			_Data.ShrinkSize(_Data.GetSize()-1);

		*this <<= x.Exponent;
	}
	else
	{
		SetSign(false);
		SetLength(0);
	}
}

void BigNumMutable::FlipSign()
{
	SetSign(!GetSign());
}

void BigNumMutable::DivFast(const BigNumRef& a, const BigNumRef& b) // this ~= a/b
{
	int exp;
	UINT m = _details::BN_Mantissa32(b, &exp);
	ASSERT(m);

	if(exp <= 0)
	{
		Div(a, *(UINT*)b.Data());
		return;
	}

	Div(a, m);
	*this >>= exp;
}

void BigNumMutable::DivRough(const BigNumRef& a, const BigNumRef& b) // this ~= a/b
{
	int exp;
	UINT m = _details::BN_Mantissa32(b, &exp);
	ASSERT(m);

	if(exp <= 0)
	{
		Div(a, *(UINT*)b.Data());
		return;
	}

	thread_local BigNumMutable wei;
	wei.CopyLowBits(exp, b);
	if(wei.IsZero())
	{
		Div(a, m);
		*this >>= exp;
		return;
	}

	thread_local BigNumMutable up, weighted;
	up.Div(a, m);	up >>= exp;		//_LOG("y0\t="<<up);// y0	
	Div(a, m+1);	*this >>= exp;	//_LOG("y1\t="<<*this);// y1
									//_LOG("wei\t="<<wei);
	weighted.Mul(*this, wei);		//_LOG("y1*wei\t="<<weighted);// y1*wei
	SetToPowerOfTwo(exp);
	*this -= wei;
	SetSign(false);					//_LOG("1-wei\t="<<*this); // (1-wei)

	wei.Mul(*this, up);				//_LOG("y0*(1-w)="<<up_weight);// y0*(1-wei)
	Add(wei, weighted);

	*this >>= exp;
}


void BigNumMutable::CopyLowBits(UINT w, const BigNumRef& a)
{
	typedef _details::BN_BLK	BN_BLK;

	ASSERT(a.GetLength()*sizeof(BN_BLK)*8 >= w);

	UINT len = (w + sizeof(BN_BLK)*8 - 1)/(sizeof(BN_BLK)*8);
	*this = BigNumRef(false, a.Data(), len);

	UINT remain = w%(sizeof(BN_BLK)*8);
	BN_BLK mask = (~((BN_BLK)0)) >> (sizeof(BN_BLK)*8 - remain);
	_Data[len-1] &= mask;

	TrimLeadingZero();
}

void BigNumMutable::SetToPowerOfTwo(UINT exp)
{
	typedef _details::BN_BLK	BN_BLK;

	UINT len = (exp + sizeof(_details::BN_BLK)*8)/(sizeof(BN_BLK)*8);
	SetLength(len);
	FillZero();
	UINT remain = (exp%(sizeof(BN_BLK)*8));

	_Data[len-1] = ((BN_BLK)1)<<remain;
}

bool BigNumMutable::FromString(const rt::String_Ref& s)
{
	struct _Hex
	{	bool error;
		int ToVal(char a)
		{
			if('0'<=a && a<='9')
			{	return a -= '0';
			}
			else if(a>='A' && a<='F')
			{	return a + 10 - 'A';
			}
			else if(a>='a' && a<='f')
			{	return a + 10 - 'a';
			}
			error = true;
			return 0;
		}
		_Hex(){ error = false; }
	};

	_Hex h;
	bool sign = false;
	LPCSTR p = s.Begin();
	if(*p == '-')
	{	sign = true;
		p++;
	}
	else if(*p == '+')
		p++;

	if(*(WORD*)p == 0x7830 || *(WORD*)p == 0x5830)
	{	// base16
		p+=2;
		UINT co = (UINT)(s.End() - p);
		UINT exp = 0;
		{	WORD* t = (WORD*)(s.End() - 2);
			UINT t_co = co/2;
			while(t_co && *t == 0x3030)
			{
				co -= 2;
				t_co--;
				t--;
				exp++;
			}

			SetLengthInByte((co+1)/2);
			FillZero();
			LPBYTE p = (LPBYTE)Data();
			for(UINT i=0; i<t_co; i++, t--, p++)
			{
				*p = (h.ToVal(((LPCSTR)t)[0]) << 4) | 
					 (h.ToVal(((LPCSTR)t)[1]));
				if(h.error)return false;
			}
			if(co&1)
			{	*p = h.ToVal(((LPCSTR)t)[1]);
				if(h.error)return false;
			}

			TrimLeadingZero();
			SetSign(sign);
			return true;
		}
	}
	else
	{	// base10
		int d = s.Last() - '0';
		if(d<0 || d>9)return false;

		*this = d;
		BigNumMutable mag = 10, val;

		for(int i = (int)(s.GetLength() - 2); i>=0 && s[i] >= '0' && s[i] <= '9'; i--)
		{
			UINT dig = (UINT)(s[i] - '0');
			if(dig)
			{
				val.Mul(mag, dig);
				*this += val;
			}
			mag *= 10;
		}

		TrimLeadingZero();
		SetSign(sign);
		return true;
	}

	return false;
}

} // namespace oxd


