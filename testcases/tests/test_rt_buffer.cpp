#include "../../core/rt/buffer_type.h"
#pragma warning(disable: 4838)

#include "test.h"
void buffer_TopWeightedValues()
{
	int ret{ 0 };
	
	rt::TopWeightedValues<int, 5, float> votes;
	
	ret = votes.Sample(1, 0.1f);
	ret = votes.Sample(3, 0.4f);
	_LOG(votes);
	
	ret = votes.Sample(4, 0.2f);
	ret = votes.Sample(5, 0.02f);
	ret = votes.Sample(2, 0.01f);
	_LOG(votes);

	ret = votes.Sample(4, 0.3f);
	_LOG(votes);

	ret = votes.Sample(2, 1.f);
	_LOG(votes);
			
	// keep_latest_value may useless
	// if sumweight bigger than now ,what happened
	_LOG("\n");
	rt::TopWeightedValues<int, 5, float,true> votes2;

	ret = votes2.Sample(1, 0.1f);
	ret = votes2.Sample(3, 0.4f);
	_LOG(votes2);

	ret = votes2.Sample(4, 0.2f);
	ret = votes2.Sample(5, 0.02f);
	ret = votes2.Sample(2, 0.01f);
	_LOG(votes2);

	ret = votes2.Sample(4, 0.3f);
	_LOG(votes2);

	ret = votes2.Sample(2, 1.f);
	_LOG(votes2);
	//UN_MATCHED
	ret = votes2.Sample(12, 1.0f);
	_LOG(votes2);
	// bigger than rt::TypeTraits<T_WEIGHT>::MaxVal()/8*7
	// may div 2 too much times?
	_LOG("\n");
	constexpr auto maxnum = rt::TypeTraits<float>::MaxVal();
	_LOG("maxnum " << maxnum);
	rt::TopWeightedValues<int, 5, float> votes3;
	ret = votes3.Sample(1, maxnum-0.1f);
	ret = votes3.Sample(3, maxnum-0.4f);
	_LOG("maxnum " << votes3);

	// other functions
	_LOG("GetSize "<<votes.GetSize());
	_LOG("GetCapacity " << votes.GetCapacity());
	_LOG("GetWeight " << votes.GetWeight());
	_LOG("IsEmpty " << votes.IsEmpty());
	_LOG("GetCount " << votes.GetCount()<<"\t" <<votes.GetCount(1) << "\t" << votes.GetCount(2) << "\t" << votes.GetCount(3) << "\t" << votes.GetCount(4));
	
	_LOG("GetSignificantRatio " << votes.GetSignificantRatio());
	_LOG("IsSignificant " << votes.IsSignificant());
	ret = votes.Sample(2, 1.f);
	_LOG("GetSignificantRatio " << votes.GetSignificantRatio());
	_LOG("IsSignificant " << votes.IsSignificant())
	// is this val right?
	auto votes4 = votes;
	_LOG(votes4);
	votes4.Remove(0);
	_LOG("Remove " << votes4);
	votes4.Remove(1);
	_LOG("Remove " << votes4);
	
	votes.ClampWeight(0.02f);
	_LOG("ClampWeight " << votes);
	_LOG("GetCount " << votes.GetCount() << "\t" << votes.GetCount(1) << "\t" << votes.GetCount(2) << "\t" << votes.GetCount(3) << "\t" << votes.GetCount(4));
	_LOG("IsEmpty " << votes.IsEmpty()); // not empty?
	// too much get(i) (const)

}
void buffer_BufferEx()
{
	rt::BufferEx<int>	buf;
	buf.SetSize(3);
	buf.Set(0);
	buf[2] = 123;
	_LOG("Init: " << buf);

	{	rt::Buffer<int> buf2(buf);
	_LOG("Copy: " << buf2);
	}

	buf.push_back() = 456;
	buf.push_back(789);
	buf.push_front(12);
	_LOG("push: " << buf);

	// is this reasonable
	buf.erase(1, 3);
	_LOG("erase: " << buf);

	buf.insert_n(1, 3, 100);
	_LOG("insert: " << buf);
	buf.insert(1) = 200;
	_LOG("insert: " << buf);

	//since we has used std library,why not use all of it.
	buf.Sort();
	//std::sort(buf.Begin(), buf.End());
	_LOG("sort: " << buf);

	// sort
	{
		struct _item
		{
			rt::String_Ref	a;
			int				v;
			_item(LPCSTR aa, int vv) { a = aa; v = vv; }
			bool operator < (const _item& x) const { return a < x.a; }
		};

		rt::BufferEx<_item>	buf;
		buf.push_back(_item("hello", 3));
		buf.push_back(_item("world", 4));
		buf.push_back(_item("is", 2));
		buf.push_back(_item("shit", 1));

		buf.Sort();

		for (UINT i = 0; i < buf.GetSize(); i++)
			_LOG(buf[i].a);
	}

	rt::BufferEx<rt::String>	non_pod;
	_LOG("Reserver len: " << non_pod.GetReservedSize());
	_LOG("Size: " << non_pod.GetSize());
	non_pod.push_back() = "string1";
	_LOG("Reserver len: " << non_pod.GetReservedSize());
	_LOG("Size: " << non_pod.GetSize());

	non_pod.push_back("string2");
	non_pod.push_front("string0");
	_LOG("Reserver len: " << non_pod.GetReservedSize());
	_LOG("Size: " << non_pod.GetSize());
	_LOG("Strings: " << non_pod);

	// should handle assignment to self in operator
	rt::BufferEx<rt::String>	non_pod_wrongcopy(non_pod);
	non_pod_wrongcopy = non_pod_wrongcopy;
	_LOG("Strings: " << non_pod_wrongcopy);

	rt::BufferEx<rt::String>	non_pod_copy(non_pod);
	non_pod_copy.push_back("string3");
	_LOG("Strings copied: " << non_pod_copy);

	rt::Buffer<WORD> a;
	a.SetSize(10);
	a.RandomBits(0);
	_LOG(a);
	_LOG("Top-1: " << a.FindTopKth(1));
	_LOG("Top-3: " << a.FindTopKth(3));
	_LOG("Top-6: " << a.FindTopKth(6));
	_LOG("Top-10: " << a.FindTopKth(10));

	// other functions
	_LOG("Size: " << non_pod_copy.GetSize());
	auto ret = non_pod_copy.ChangeSize(5);
	_LOG("Changed Size: " << non_pod_copy.GetSize());
	//can not compile non_pod_copy.compact_memory();

	_LOG(non_pod_copy.GetReservedSize());

	ret=non_pod_copy.reserve(7);
	_LOG("reserved: " << non_pod_copy);
	
	// need sorted before this operation
	auto ret2 = non_pod_copy.PushSorted("string1");
	_LOG("PushSorted: " << non_pod_copy);
	ret2 = non_pod_copy.PushUnique("string1");
	_LOG("PushUnique: " << non_pod_copy);
}
void buffer_CircularBuffer()
{
	//too hard to understand
	rt::String str("1234567890");
	rt::CircularBuffer cyc;
	cyc.SetSize(1024);
	LPBYTE p;

	p = cyc.Push(237);
	memcpy(p, str, str.GetLength());
	cyc.SetBlockSize(p, str.GetLength());
	cyc.Peek();

	p = cyc.Push(499);
	memcpy(p, str, str.GetLength());
	cyc.SetBlockSize(p, str.GetLength() - 1);

	if (cyc.Push(277) == nullptr)_LOG("Overflow");

	_LOG(rt::String_Ref((LPCSTR)cyc.Peek()->Data, (UINT)cyc.Peek()->Length));
	cyc.Pop();

	p = cyc.Push(199);
	memcpy(p, str, str.GetLength());
	cyc.SetBlockSize(p, str.GetLength() - 2);

	_LOG(rt::String_Ref((LPCSTR)cyc.Peek()->Data, (UINT)cyc.Peek()->Length));
	cyc.Pop();

	_LOG(rt::String_Ref((LPCSTR)cyc.Peek()->Data, (UINT)cyc.Peek()->Length));
}
void buffer_BooleanArray()
{
	rt::BooleanArray<89>	boolvec;

	boolvec.SetAll();

	rt::String out;
	boolvec.ToString<>(out);	_LOG(out);	out.Empty();

	boolvec.ResetAll();
	boolvec.Set(1);
	boolvec.Set(5);
	boolvec.Set(63);
	boolvec.Set(80);
	boolvec.Set(88);

	rt::SS szpc(" PopCount:");

	boolvec.ToString<>(out); out += szpc + boolvec.PopCount();	_LOG(out);	out.Empty();
	boolvec.ToStringWithIndex<>(out);	_LOG(out);	out.Empty();

	boolvec.Shift(1);
	boolvec.ToString<>(out); out += szpc + boolvec.PopCount();	_LOG(out);	out.Empty();
	boolvec.ToStringWithIndex<>(out);	_LOG(out);	out.Empty();

	boolvec.Shift(2);
	boolvec.ToString<>(out); out += szpc + boolvec.PopCount();	_LOG(out);	out.Empty();
	boolvec.ToStringWithIndex<>(out);	_LOG(out);	out.Empty();

	boolvec.Shift(-9);
	boolvec.ToString<>(out); out += szpc + boolvec.PopCount();	_LOG(out);	out.Empty();
	boolvec.ToStringWithIndex<>(out);	_LOG(out);	out.Empty();

	boolvec.Shift(16);
	boolvec.ToString<>(out); out += szpc + boolvec.PopCount();	_LOG(out);	out.Empty();
	boolvec.ToStringWithIndex<>(out);	_LOG(out);	out.Empty();

	boolvec.Shift(-65);
	boolvec.ToString<>(out); out += szpc + boolvec.PopCount();	_LOG(out);	out.Empty();
	boolvec.ToStringWithIndex<>(out);	_LOG(out);	out.Empty();

	// other functions
	_LOG("Get 8: " << boolvec.Get(8));
	boolvec.Reset(8);
	_LOG("Get 8 after reset: " << boolvec.Get(8));
	_LOG("IsAllReset: " << boolvec.IsAllReset());
	_LOG("PopCount: " << boolvec.PopCount());
	// ^= && |= donot need test

}
void rt::UnitTests::buffer()
{
	//buffer_TopWeightedValues();
	//buffer_BufferEx();	
	//buffer_CircularBuffer();
	buffer_BooleanArray();
}