
#pragma warning(disable: 4838)

#include "test.h"

void rt::UnitTests::binary_search()
{
	typedef WORD TT;

	rt::Buffer<TT>	a;
	a.SetSize(10);
	a.RandomBits(100);

	std::sort(a.Begin(), a.End());
	_LOG(a);

	_LOG(a[0] << " is at " << rt::LowerBound(a, a.GetSize(), a[0]) << "/" << rt::BinarySearch(a, a.GetSize(), a[0]));
	_LOG(a[0] + 10 << " is at " << rt::LowerBound(a, a.GetSize(), a[0] + 10) << "/" << rt::BinarySearch(a, a.GetSize(), a[0] + 10));
	_LOG(a[0] - 1 << " is at " << rt::LowerBound(a, a.GetSize(), a[0] - 1) << "/" << rt::BinarySearch(a, a.GetSize(), a[0] - 1));

	_LOG(a[3] << " is at " << rt::LowerBound(a, a.GetSize(), a[3]) << "/" << rt::BinarySearch(a, a.GetSize(), a[3]));
	_LOG(a[3] + 1 << " is at " << rt::LowerBound(a, a.GetSize(), a[3] + 1) << "/" << rt::BinarySearch(a, a.GetSize(), a[3] + 1));

	_LOG(a[a.GetSize() - 1] << " is at " << rt::LowerBound(a, a.GetSize(), a[a.GetSize() - 1]) << "/" << rt::BinarySearch(a, a.GetSize(), a[a.GetSize() - 1]));
	_LOG(a[a.GetSize() - 1] + 1 << " is at " << rt::LowerBound(a, a.GetSize(), a[a.GetSize() - 1] + 1) << "/" << rt::BinarySearch(a, a.GetSize(), a[a.GetSize() - 1] + 1));
}

#pragma pack(1)
struct MyType
{
	ULONGLONG	a;
};
struct MyTypeTR : public MyType
{
	static const bool   __IsPOD = false;
	typedef BYTE        __TypeVal[100];
};
#pragma pack()
void rt::UnitTests::rt()
{
	{
		int					a[4];
		LONGLONG			b;
		rt::BufferEx<int>	c;		c.SetSize(10);
		std::string			d;		d = "1234";
		rt::String_Ref		e;		e = "89014444";

		_LOG("a = " << rt::GetDataSize(a) << " PTR=" << (a == rt::GetDataPtr(a)));
		_LOG("b = " << rt::GetDataSize(b) << " PTR=" << (&b == rt::GetDataPtr(b)));
		_LOG("c = " << rt::GetDataSize(c) << " PTR=" << (c.Begin() == rt::GetDataPtr(c)));
		_LOG("d = " << rt::GetDataSize(d) << " PTR=" << (d.c_str() == rt::GetDataPtr(d)));
		_LOG("e = " << rt::GetDataSize(e) << " PTR=" << (e.Begin() == rt::GetDataPtr(e)));
	}

	{	_LOG("sizeof(rt::TypeTraits<MyType>::t_Val) = " << sizeof(rt::TypeTraits<MyType>::t_Val)); //can not understand
		_LOG("rt::TypeTraits<MyType>::IsPOD = " << rt::TypeTraits<MyType>::IsPOD);
		_LOG("sizeof(rt::TypeTraits<MyTypeTR>::t_Val) = " << sizeof(rt::TypeTraits<MyTypeTR>::t_Val));
		_LOG("rt::TypeTraits<MyTypeTR>::IsPOD = " << rt::TypeTraits<MyTypeTR>::IsPOD);
	}

	_LOG("sizeof(BYTE)	=" << sizeof(BYTE));
	_LOG("sizeof(WORD)	=" << sizeof(WORD));
	_LOG("sizeof(DWORD)	=" << sizeof(DWORD));
	_LOG("sizeof(UINT)	=" << sizeof(UINT));

	_LOG("sizeof(QWORD)		=" << sizeof(QWORD));
	_LOG("sizeof(LONGLONG)	=" << sizeof(LONGLONG));
	_LOG("sizeof(ULONGLONG)	=" << sizeof(ULONGLONG));
	_LOG("sizeof(QWORD)		=" << sizeof(QWORD));


	{	LPBYTE p = _Malloc32AL(BYTE, 2047);
	rt::Zero(p, 2047);
	_SafeFree32AL(p);
	}

	_LOG("rt::MaxPower2Within(2049): " << rt::MaxPower2Within(2049));

	{	short a = 1, b = 2;
	rt::Swap(a, b);
	_LOG("rt::Swap: <a, b> = <" << a << ',' << b << '>');
	}

	{	ULONGLONG a = 0xbabeface, b = 0xbabeface;
	rt::SwapByteOrder(b);
	_LOG("rt::SwapByteOrder(" << rt::tos::Binary<>(&a, sizeof(ULONGLONG)) <<
		") = " << rt::tos::Binary<>(&b, sizeof(ULONGLONG)));
	}

	_LOG("int is POD: " << rt::TypeTraits<int>::IsPOD);
	_LOG("int[24] is POD: " << rt::TypeTraits<int[24]>::IsPOD);

	{	bool ret = rt::IsTypeSame<int, int>::Result;
	_LOG("rt::IsTypeSame<int,int>: " << ret);
	ret = rt::IsTypeSame<int, float>::Result;
	_LOG("rt::IsTypeSame<int,float>: " << ret);
	ret = rt::IsTypeSame<const int, int>::Result;
	_LOG("rt::IsTypeSame<const int,int>: " << ret);
	ret = rt::IsTypeSame<rt::Remove_QualiferAndRef<const int>::t_Result, int>::Result;
	_LOG("rt::Remove_QualiferAndRef<const int>: " << ret);
	ret = rt::IsTypeSame<rt::TypeTraits<int[23]>::t_Element, int>::Result;
	_LOG("rt::TypeTraits<int[23]>::t_Element: " << ret);
	ret = rt::NumericTraits<double>::IsFloat;
	_LOG("rt::NumericTraits<double>::IsFloat: " << ret);
	ret = rt::NumericTraits<float>::IsUnsigned;
	_LOG("rt::NumericTraits<float>::IsUnsigned: " << ret);
	}

	{	try
	{
		LPCVOID p;
		os::EnableMemoryExceptionInThread(false);
		p = _Malloc32AL(BYTE, -1);
		_LOG("Allocate: " << (ULONGLONG)p);

		os::EnableMemoryExceptionInThread(true);
		p = _Malloc32AL(BYTE, -1);
		_LOG(p);
	}
	catch (std::bad_alloc x)
	{
		_LOG("catch std::bad_alloc");
	};
	}

	_LOG("17 ~= 2^" << Log2(17U));

	{
		_LOG(rt::TypeNameToString<int>());
		_LOG(rt::TypeNameToString<os::Timestamp>());
		_LOG(rt::TypeNameToString<os::_tagProcessPriority>());

		struct a { int __; };
		_LOG(rt::TypeNameToString<a>() << " (nested type in function scope)");
	}
}

void rt::UnitTests::sortedpush()
{
	rt::Randomizer a(100);

	{	_LOG("BufferEx::SortedPush");
	rt::BufferEx<WORD>	sorted;

	for (UINT i = 0; i < 20; i++)
	{
		WORD new_v = (BYTE)a.GetNext();
		SSIZE_T pos = sorted.PushSorted(new_v);
		_LOG(sorted << " Add:" << new_v << " at " << pos);
	}
	}

	{	_LOG("Buffer::SortedPush");
	rt::Buffer<WORD>	sorted;
	sorted.SetSize(8);
	sorted.Void();

	for (UINT i = 0; i < 20; i++)
	{
		WORD new_v = (BYTE)a.GetNext();
		SSIZE_T pos = sorted.PushSorted(new_v);
		_LOG(sorted << " Add:" << new_v << " at " << pos);
	}
	}
}