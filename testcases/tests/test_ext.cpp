#include "../../core/ext/sparsehash/sparsehash.h"
#include "../../core/ext/rocksdb/rocksdb.h"
#include "../../core/ext/concurrentqueue/async_queue.h"
#include "test.h"

void rt::UnitTests::sparsehash()
{
	{
		ext::fast_set<int>	set;
		set.insert(1);
		set.insert(2);
		set.insert(3);
		set.insert(4);
		set.insert(5);
		set.insert(6);
		set.insert(7);
		set.insert(8);
		set.insert(9);
		set.insert(10);

		for(auto it = set.begin(); it != set.end(); it++)
			if((*it & 1) == 0)
				set.erase(it);

		rt::BufferEx<int>	a;
		for(auto it: set)
			a.push_back(it);

		a.Sort();
		_LOG(a);
	}

	{
		ext::fast_map<int, LPCSTR>	set;
		set.insert(std::make_pair(1,"1"));
		set.insert(std::make_pair(2,"2"));
		set.insert(std::make_pair(3,"3"));
		set.insert(std::make_pair(4,"4"));
		set.insert(std::make_pair(5,"5"));
		set.insert(std::make_pair(6,"6"));
		set.insert(std::make_pair(7,"7"));
		set.insert(std::make_pair(8,"8"));
		set.insert(std::make_pair(9,"9"));
		set.insert(std::make_pair(10, "10"));

		for(auto it = set.begin(); it != set.end(); it++)
			if((it->first & 1) == 1)
				set.erase(it);

		rt::BufferEx<int>	a;
		for(auto it: set)
			a.push_back(it.first);

		a.Sort();
		_LOG(a);
	}
}

void rt::UnitTests::rocks_db()
{
	LPCSTR fn = "test.db";

	{
		ext::RocksDB db;
		ext::RocksDB::Nuke(fn);
		db.Open(fn);

		for(UINT i=0; i<100; i++)
			db.Set(i*2,i*2);
		{
			auto it = db.First();
			_LOG(it.Value<int>());
			it.Next(4);
			_LOG(it.Value<int>());

			// ext::RocksCursor a;   // not allowed
			// ext::RocksCursor b = it;  // not allowed

			it = db.Find(33);
			_LOG(it.Value<int>());
			it.Prev(4);
			_LOG(it.Value<int>());
		}

		db.Close();

		ext::RocksDB::Nuke(fn);
	}

	// Release Build Z440 Samsung SSD 960 (NVMe)				Release Build T4500 + WD1002
	//DBWR_LEVEL_FASTEST - WriteOptionsFastRisky				DBWR_LEVEL_FASTEST - WriteOptionsFastRisky
	//Asce Order: 1280000     18962962        1712374			Asce Order: 489952      13653333        1036437
	//Desc Order: 142618      20897959        1759450			Desc Order: 66797       13473684        1047034
	//Rand Order: 136351      13298701        1667752			Rand Order: 69189       9570093 1042769
	//DBWR_LEVEL_DEFAULT - WriteOptionsFastRisky				DBWR_LEVEL_DEFAULT - WriteOptionsFastRisky
	//Asce Order: 1484057     21787234        1738539			Asce Order: 467579      13653333        1087048
	//Desc Order: 144632      21333333        1777777			Desc Order: 72061       13298701        1090521
	//Rand Order: 137081      15283582        1732656			Rand Order: 65473       10039215        1041709
	//DBWR_LEVEL_UNBUFFERED - WriteOptionsFastRisky				DBWR_LEVEL_UNBUFFERED - WriteOptionsFastRisky
	//Asce Order: 1365333     21333333        1689768			Asce Order: 404743      13298701        1072251
	//Desc Order: 139509      20480000        1783972			Desc Order: 59122       13298701        1066666
	//Rand Order: 135989      15283582        1706666			Rand Order: 61207       9481481 1038539
	//DBWR_LEVEL_STRONG - WriteOptionsFastRisky					DBWR_LEVEL_STRONG - WriteOptionsFastRisky
	//Asce Order: 1505882     21333333        1812389			Asce Order: 395366      12800000        1026052
	//Desc Order: 147338      21333333        1695364			Desc Order: 59813       13128205        1077894
	//Rand Order: 130612      14840579        1726812			Rand Order: 55172       9941747 1056759
	//DBWR_LEVEL_FASTEST - WriteOptionsDefault					DBWR_LEVEL_FASTEST - WriteOptionsDefault
	//Asce Order: 358041      21787234        1855072			Asce Order: 120046      13298701        989371
	//Desc Order: 119069      20897959        1585139			Desc Order: 51225       13298701        1072251
	//Rand Order: 100293      14628571        1735593			Rand Order: 50443       9846153 1035389
	//DBWR_LEVEL_DEFAULT - WriteOptionsDefault					DBWR_LEVEL_DEFAULT - WriteOptionsDefault
	//Asce Order: 347118      21787234        1851717			Asce Order: 114413      13298701        1014866
	//Desc Order: 118381      21333333        1768566			Desc Order: 51534       13298701        1036437
	//Rand Order: 105785      14840579        1718120			Rand Order: 49708       9846153			1052415
	//DBWR_LEVEL_UNBUFFERED - WriteOptionsDefault				DBWR_LEVEL_UNBUFFERED - WriteOptionsDefault
	//Asce Order: 345945      21787234        1882352			Asce Order: 102605      12641975        1024000
	//Desc Order: 118655      20897959        1868613			Desc Order: 51900       13298701        1015873
	//Rand Order: 111183      14840579        1865209			Rand Order: 48995       9752380			1079030
	//DBWR_LEVEL_STRONG - WriteOptionsDefault					DBWR_LEVEL_STRONG - WriteOptionsDefault
	//Asce Order: 339072      21787234        1747440			Asce Order: 115445      13128205        1080168
	//Desc Order: 113024      21333333        1910447			Desc Order: 52485       13128205        1077894
	//Rand Order: 110344      14628571        1692561			Rand Order: 50667       9846153			1026052
	//DBWR_LEVEL_FASTEST - WriteOptionsRobust					DBWR_LEVEL_FASTEST - WriteOptionsRobust
	//Asce Order: 577		20480000        2275555				Asce Order: 46			20480000        1462857
	//Desc Order: 563		20480000        2560000				Desc Order: 39			10240000        1462857
	//Rand Order: 578		20480000        2275555				Rand Order: 49			10240000        1575384
	//DBWR_LEVEL_DEFAULT - WriteOptionsRobust					DBWR_LEVEL_DEFAULT - WriteOptionsRobust
	//Asce Order: 576		20480000        2560000				Asce Order: 46			20480000        1462857
	//Desc Order: 571		20480000        2560000				Desc Order: 46			20480000        1462857
	//Rand Order: 567		20480000        2560000				Rand Order: 46			20480000        1365333
	//DBWR_LEVEL_UNBUFFERED - WriteOptionsRobust				DBWR_LEVEL_UNBUFFERED - WriteOptionsRobust
	//Asce Order: 572		20480000        2560000				Asce Order: 46			20480000        1365333
	//Desc Order: 573		20480000        2275555				Desc Order: 38			10240000        1462857
	//Rand Order: 574		20480000        2560000				Rand Order: 55			20480000        1365333
	//DBWR_LEVEL_STRONG - WriteOptionsRobust					DBWR_LEVEL_STRONG - WriteOptionsRobust
	//Asce Order: 575		20480000        2560000				Asce Order: 39			10240000        1365333
	//Desc Order: 560		20480000        2925714				Desc Order: 54			10240000        1462857
	//Rand Order: 576		20480000        2048000				Rand Order: 48			10240000        1204705

	/*
	rt::Buffer<ULONGLONG>	keys;
	keys.SetSize(100*1024);

	auto test = [&keys, fn](ext::RocksDB::DBScopeWriteRobustness wr, const ext::WriteOptions* w_opt)
	{
		for(UINT i=0; i<keys.GetSize(); i++)
		{	keys[i] = i;
			rt::SwitchByteOrder(keys[i]);
		}

		os::HighPerformanceCounter hpc;
		hpc.SetOutputMillisecond();
		hpc.LoadCurrentCount();

		{	ext::RocksDB::Nuke(fn);
			ext::RocksDB db;
			db.Open(fn, wr);
			for(UINT i=0; i<keys.GetSize(); i++)
				db.Set(keys[i], keys[i], w_opt);
			ULONGLONG insert = keys.GetSize()*1000/hpc.TimeLapse();

			hpc.LoadCurrentCount();
			for(UINT i=0; i<10; i++)
			{
				auto it = db.First();
				while(it.IsValid())it.Next();
			}
			ULONGLONG scan = keys.GetSize()*10000/hpc.TimeLapse();

			hpc.LoadCurrentCount();
			for(UINT i=0; i<10; i++)
			{
				auto it = db.Last();
				while(it.IsValid())it.Prev();
			}
			_LOG("Asce Order: "<<insert<<'\t'<<scan<<'\t'<<keys.GetSize()*10000/hpc.TimeLapse());
		}

		{	ext::RocksDB::Nuke(fn);
			ext::RocksDB db;
			db.Open(fn, wr);
			for(UINT i=0; i<keys.GetSize(); i++)
				db.Set(keys[keys.GetSize() - i - 1], keys[i], w_opt);
			ULONGLONG insert = keys.GetSize()*1000/hpc.TimeLapse();

			hpc.LoadCurrentCount();
			for(UINT i=0; i<10; i++)
			{
				auto it = db.First();
				while(it.IsValid())it.Next();
			}
			ULONGLONG scan = keys.GetSize()*10000/hpc.TimeLapse();

			hpc.LoadCurrentCount();
			for(UINT i=0; i<10; i++)
			{
				auto it = db.Last();
				while(it.IsValid())it.Prev();
			}
			_LOG("Desc Order: "<<insert<<'\t'<<scan<<'\t'<<keys.GetSize()*10000/hpc.TimeLapse());
		}

		{	ext::RocksDB::Nuke(fn);
			ext::RocksDB db;
			db.Open(fn, wr);
			keys.Shuffle(5439);
			for(UINT i=0; i<keys.GetSize(); i++)
				db.Set(keys[i], keys[i], w_opt);
			ULONGLONG insert = keys.GetSize()*1000/hpc.TimeLapse();

			hpc.LoadCurrentCount();
			for(UINT i=0; i<10; i++)
			{
				auto it = db.First();
				while(it.IsValid())it.Next();
			}
			ULONGLONG scan = keys.GetSize()*10000/hpc.TimeLapse();

			hpc.LoadCurrentCount();
			for(UINT i=0; i<10; i++)
			{
				auto it = db.Last();
				while(it.IsValid())it.Prev();
			}
			_LOG("Rand Order: "<<insert<<'\t'<<scan<<'\t'<<keys.GetSize()*10000/hpc.TimeLapse());
		}
	};

	_LOG_HIGHLIGHT("DBWR_LEVEL_FASTEST - WriteOptionsFastRisky");
	test(ext::RocksDB::DBWR_LEVEL_FASTEST, ext::RocksDB::WriteOptionsFastRisky);

	_LOG_HIGHLIGHT("DBWR_LEVEL_DEFAULT - WriteOptionsFastRisky");
	test(ext::RocksDB::DBWR_LEVEL_DEFAULT, ext::RocksDB::WriteOptionsFastRisky);

	_LOG_HIGHLIGHT("DBWR_LEVEL_UNBUFFERED - WriteOptionsFastRisky");
	test(ext::RocksDB::DBWR_LEVEL_UNBUFFERED, ext::RocksDB::WriteOptionsFastRisky);

	_LOG_HIGHLIGHT("DBWR_LEVEL_STRONG - WriteOptionsFastRisky");
	test(ext::RocksDB::DBWR_LEVEL_STRONG, ext::RocksDB::WriteOptionsFastRisky);


	_LOG_HIGHLIGHT("DBWR_LEVEL_FASTEST - WriteOptionsDefault");
	test(ext::RocksDB::DBWR_LEVEL_FASTEST, ext::RocksDB::WriteOptionsDefault);

	_LOG_HIGHLIGHT("DBWR_LEVEL_DEFAULT - WriteOptionsDefault");
	test(ext::RocksDB::DBWR_LEVEL_DEFAULT, ext::RocksDB::WriteOptionsDefault);

	_LOG_HIGHLIGHT("DBWR_LEVEL_UNBUFFERED - WriteOptionsDefault");
	test(ext::RocksDB::DBWR_LEVEL_UNBUFFERED, ext::RocksDB::WriteOptionsDefault);

	_LOG_HIGHLIGHT("DBWR_LEVEL_STRONG - WriteOptionsDefault");
	test(ext::RocksDB::DBWR_LEVEL_STRONG, ext::RocksDB::WriteOptionsDefault);

	keys.ShrinkSize(2*1024);

	_LOG_HIGHLIGHT("DBWR_LEVEL_FASTEST - WriteOptionsRobust");
	test(ext::RocksDB::DBWR_LEVEL_FASTEST, ext::RocksDB::WriteOptionsRobust);

	_LOG_HIGHLIGHT("DBWR_LEVEL_DEFAULT - WriteOptionsRobust");
	test(ext::RocksDB::DBWR_LEVEL_DEFAULT, ext::RocksDB::WriteOptionsRobust);

	_LOG_HIGHLIGHT("DBWR_LEVEL_UNBUFFERED - WriteOptionsRobust");
	test(ext::RocksDB::DBWR_LEVEL_UNBUFFERED, ext::RocksDB::WriteOptionsRobust);

	_LOG_HIGHLIGHT("DBWR_LEVEL_STRONG - WriteOptionsRobust");
	test(ext::RocksDB::DBWR_LEVEL_STRONG, ext::RocksDB::WriteOptionsRobust);

	*/
}

void rt::UnitTests::async_queue()
{
	{	_LOG("Multiple Reader/Writer");		
		typedef ext::AsyncDataQueue<DWORD, true, 2> CQ;
		
		UINT capacity = 4;
		DWORD t = 0;
		
		{	CQ cq(capacity);
			for(UINT i=0;i<5;i++)
			{
				_LOG("Push["<<i<<"]: "<<cq.Push(i));
			}

			for(UINT i=0;i<4;i++)
			{	
				int ret = cq.Pop(t);
				_LOG("Pop["<<i<<"]: "<<ret<<" VAL="<<t);
			}

			os::Timestamp tm;
			tm.LoadCurrentTime();
			_LOG("Pop[4]: "<<cq.Pop(t, 1100));
			_LOG("WAIT="<<(int)(tm.TimeLapse()/1000));
		}

		{	CQ cq(capacity);
			for(UINT i=0;i<5;i++)
			{
				_LOG("Push["<<i<<"]: "<<cq.Push(i, true));
			}
		}
	}
	
	{	_LOG("Single Reader/Writer");
		typedef ext::AsyncDataQueue<DWORD, true, 2, true> CQ;

		UINT capacity = 7;
		DWORD t = 0;

		{	CQ cq(capacity);
			for(UINT i=0;i<8;i++)
			{
				_LOG("Push["<<i<<"]: "<<cq.Push(i));
			}

			for(UINT i=0;i<7;i++)
			{	
				int ret = cq.Pop(t);
				_LOG("Pop["<<i<<"]: "<<ret<<" VAL="<<t);
			}

			_LOG("Pop[7]: "<<cq.Pop(t));
		}

		{	CQ cq(capacity);
			for(UINT i=0;i<8;i++)
			{
				_LOG("Push["<<i<<"]: "<<cq.Push(i, true));
			}
		}
	}

#if defined(PLATFORM_RELEASE_BUILD)
	{
		typedef ext::AsyncDataQueueInfinite<ULONGLONG, true, 16, false> CQ;
		CQ cq;

		os::HighPerformanceCounter tm;
		static const int steps = 1000000;

		tm.LoadCurrentCount();
		for(UINT i=0; i<steps; i++)
		{
			cq.Push(10000);	cq.Push(10000); cq.Push(10000); cq.Push(10000); cq.Push(10000);
			cq.Push(10000);	cq.Push(10000); cq.Push(10000); cq.Push(10000); cq.Push(10000);
			cq.Push(10000);	cq.Push(10000); cq.Push(10000); cq.Push(10000); cq.Push(10000);
			cq.Push(10000);	cq.Push(10000); cq.Push(10000); cq.Push(10000); cq.Push(10000);
			cq.Push(10000);	cq.Push(10000); cq.Push(10000); cq.Push(10000); cq.Push(10000);
		}

		_LOGC("CQ Push: "<<10e9*steps*25/tm.TimeLapse()<<" qps");

		tm.LoadCurrentCount();
		for(UINT i=0; i<steps; i++)
		{
			cq.Push(10000);	cq.Push(10000); cq.Push(10000); cq.Push(10000); cq.Push(10000);
			cq.Push(10000);	cq.Push(10000); cq.Push(10000); cq.Push(10000); cq.Push(10000);
			cq.Push(10000);	cq.Push(10000); cq.Push(10000); cq.Push(10000); cq.Push(10000);
			cq.Push(10000);	cq.Push(10000); cq.Push(10000); cq.Push(10000); cq.Push(10000);
			cq.Push(10000);	cq.Push(10000); cq.Push(10000); cq.Push(10000); cq.Push(10000);
		}

		_LOGC("CQ Pop: "<<10e9*steps*25/tm.TimeLapse()<<" qps");

		tm.LoadCurrentCount();
		for(UINT i=0; i<steps; i++)
		{
			cq.Push(10000);	cq.Push(10000); cq.Push(10000); cq.Push(10000); cq.Push(10000);
			cq.Push(10000);	cq.Push(10000); cq.Push(10000); cq.Push(10000); cq.Push(10000);
			cq.Push(10000);	cq.Push(10000); cq.Push(10000); cq.Push(10000); cq.Push(10000);
			cq.Push(10000);	cq.Push(10000); cq.Push(10000); cq.Push(10000); cq.Push(10000);
			cq.Push(10000);	cq.Push(10000); cq.Push(10000); cq.Push(10000); cq.Push(10000);
		}

		_LOGC("CQ Push: "<<10e9*steps*25/tm.TimeLapse()<<" qps");
	}
#endif

	_LOG("DeJitter Reader/Writer");
	struct DeJitter: public ext::DeJitteredQueue<ext::AsyncDataQueue<UINT, true>>
	{
		UINT seq_base;

		bool PushWithSeq(UINT x, UINT seq)
		{
			_NextSeq = (seq_base+seq-1)&SEQ_BITMASK;
			return Push(x);
		}

		UINT SEQ_BOUNDARY = SEQ_RANGESIZE - 10;
		void SetBoundryTest(bool yes)
		{	if(yes)
			{	_LastReadSeq = SEQ_BOUNDARY; 
				seq_base = SEQ_BOUNDARY;
			}
			else
			{	_LastReadSeq = 0;
				seq_base = 0;
			}
		}
		
	};

	ext::AsyncDataQueue<UINT, false> queue;
	DeJitter dejittered_queue;
	DeJitter dejittered_queue_boundry;

	dejittered_queue.SetBoundryTest(false);
	dejittered_queue_boundry.SetBoundryTest(true);

	rt::Buffer<UINT> pushes;
	pushes.SetSize(20);
	for(UINT i=0; i<pushes.GetSize(); i++)
		pushes[i] = i+1;

	pushes.Shuffle(3532893);

	for(UINT i: pushes)
	{
		queue.Push(i);
		dejittered_queue.PushWithSeq(i, i);
		dejittered_queue_boundry.PushWithSeq(i, i);
	}

	rt::String out;

	for(;;)
	{
		UINT val;
		if(!queue.Pop(val))break;
		out += rt::SS(",") + val;
	}
	_LOG("Jittered: "<<out.SubStr(1));

	out.Empty();
	for(;;)
	{
		UINT val;
		if(!dejittered_queue.Pop(&val))break;
		out += rt::SS(",") + val;
	}
	_LOG("DeJittered: "<<out.SubStr(1));

	out.Empty();
	for(;;)
	{
		UINT val;
		if(!dejittered_queue_boundry.Pop(&val))break;
		out += rt::SS(",") + val;
	}
	_LOG("DeJittered at Boundary: "<<out.SubStr(1));
}
