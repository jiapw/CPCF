#pragma once
#include "../../rt/type_traits.h"


namespace ext
{
#pragma pack(push, 4)
namespace _details
{
template<int bit_width>
class PrecisionFloat
{
	TYPETRAITS_DECLARE_POD;
	BYTE		_Data[bit_width/8 + 4];
public:
	PrecisionFloat(){ rt::Zero(_Data); }
	PrecisionFloat(ULONGLONG x);
	PrecisionFloat(UINT x):PrecisionFloat((ULONGLONG)x){}
	PrecisionFloat(LONGLONG x);
	PrecisionFloat(INT x):PrecisionFloat((LONGLONG)x){}
	PrecisionFloat(double x);
	PrecisionFloat(LPCSTR val_str);

	bool	Pow(ULONGLONG idx);
	bool	Pow(UINT idx){ return Pow((ULONGLONG)idx); }
	bool	Pow(const PrecisionFloat& idx);
	bool	Log(const PrecisionFloat& f, const PrecisionFloat& base);

	bool	Add(const PrecisionFloat& f);
	bool	Sub(const PrecisionFloat& f);
	bool	Mul(const PrecisionFloat& f);
	bool	Div(const PrecisionFloat& f);

//	void	BN_Mul(const oxd::BigNumRef& a, oxd::BigNumMutable& b) const;	// b = a*this

	std::string	ToString() const;
	bool	ToNumber(SIZE_T& x) const;
	bool	ToNumber(double& x) const;

	template<class t_Ostream>
	friend t_Ostream& operator<<(t_Ostream& Ostream, const PrecisionFloat& x){	Ostream << x.ToString(); return Ostream; }
};
} // namespace _details

typedef _details::PrecisionFloat<2048>	Float2048;
typedef _details::PrecisionFloat<1024>	Float1024;
typedef _details::PrecisionFloat<512>	Float512;
typedef _details::PrecisionFloat<256>	Float256;

#pragma pack(pop)
} // namespace ext
