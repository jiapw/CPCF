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
		ext::RocksStorage::Nuke(fn);
	#pragma pack(push, 1)
		struct TxnMetadata
		{	WORD	ExtraDataSize;
		};
	#pragma pack(pop)
		typedef ext::RocksDBStandalonePaged<UINT, TxnMetadata, 1024>	t_PagedTxnDB;

		t_PagedTxnDB db;
		db.Open(fn);
		db.SetPaged(100, {10}, "123", 3);

		std::string ws;
		auto* b = db.GetPaged(100, 0, ws);
		_LOG(b->TotalSize<<", "<<b->ExtraDataSize);
	}

	{
		ext::RocksStorage::Nuke(fn);
		ext::RocksStorage store;

		store.SetDBOpenOption("t2", ext::RocksDBOpenOption().SetKeyOrder<UINT>());

		store.Open(fn);

		ext::RocksDB db = store.Get("t");
		ext::RocksDB db2 = store.Get("t2");
			
		for(UINT i=0; i<100; i++)
		{
			db.Set(i*100,i*100);
			db2.Set(i*100,i*100 + 1);
		}

		{	rt::String str;
			auto it = db.First();
			while(it.IsValid()){ str += rt::tos::Number(it.Key<UINT>()) + ' '; it.Next(); }
			_LOG(str);

			str.Empty();
			it = db2.First();
			while(it.IsValid()){ str += rt::tos::Number(it.Key<UINT>()) + ' '; it.Next(); }
			_LOG(str);
		}
		store.Close();
	}

	{
		ext::RocksStorage store;
		store.SetDBOpenOption("t2", ext::RocksDBOpenOption().SetKeyOrder<UINT>());

		store.Open(fn);

		ext::RocksDB db = store.Get("t");
		ext::RocksDB db2 = store.Get("t2");

		_LOG(db.GetAs<int>(200));
		_LOG(db2.GetAs<int>(200));

		_LOG(db.Last().Key<int>());
		_LOG(db2.Last().Key<int>());

		_LOG(db.First().Key<int>());
		_LOG(db2.First().Key<int>());
		store.Close();
	}
	ext::RocksStorage::Nuke(fn);

	{
		ext::RocksDBStandalonePaged<UINT, void, 32>	paged;
		paged.Open(fn);

		rt::Buffer<BYTE> test, org;
		test.SetSize(1000 + paged.VALUE_PREFIX_SIZE);
		test.RandomBits();

		org.SetSize(1000);
		org.CopyFrom(test.Begin() + paged.VALUE_PREFIX_SIZE);

		paged.SetPaged(123, test.Begin() + paged.VALUE_PREFIX_SIZE, 1000);

		std::string ws;
		auto* d = paged.GetPaged(123, 0, ws);
		if(d)
		{
			rt::Buffer<BYTE> load;
			load.SetSize(1000);
			if(paged.LoadAllPages(123, d, load) && memcmp(org, load, 1000) == 0)
			{	_LOG("RocksDBStandalonePaged: Passed");
			}
			else
			{	_LOG("RocksDBStandalonePaged: Data mismatch");
			}
		}
		else _LOG("RocksDBStandalonePaged: Loading failed");
	}

	ext::RocksStorage::Nuke(fn);
	

	// Release Build Z440 Samsung SSD 960 (NVMe)				Release Build T4500 + WD1002								Release Build Mac Mini (2014) + Apple SSD (512GB)
	//ROCKSSTG_FASTEST - WriteOptionsFastRisky                ROCKSSTG_FASTEST - WriteOptionsFastRisky					ROCKSSTG_FASTEST - WriteOptionsFastRisky
	//Asce Order: 1754355     18971043        1536483			Asce Order: 1147158     12799680        1047964				Asce Order: 1384569    19037349    524771
	//Desc Order: 1523945     18397082        1560335			Desc Order: 928318      12968263        1048138				Desc Order: 1205159    18190861    549404
	//Desc+Compr: 2018609     23166896        2385684			Desc+Compr: 1165212     16218442        1688749				Desc+Compr: 1473572    23039712    768779
	//Rand Order: 805183      12741072        1532626			Rand Order: 548065      10026633        1039350				Rand Order: 443674     13509947    558604
	//Rand+Compr: 974097      9647362         2124415			Rand+Compr: 652878      12150552        1562221				Rand+Compr: 613971     13021859    824895
	//ROCKSSTG_DEFAULT - WriteOptionsFastRisky				ROCKSSTG_DEFAULT - WriteOptionsFastRisky					ROCKSSTG_DEFAULT - WriteOptionsFastRisky
	//Asce Order: 1933790     19395041        1666927			Asce Order: 1138739     12959728        952329				Asce Order: 1447145    18513161    596101
	//Desc Order: 1496179     19224631        1575680			Desc Order: 851233      11751339        962068				Desc Order: 1248247    8585274     507108
	//Desc+Compr: 1602880     20132908        2187711			Desc+Compr: 1159472     16235413        1649511				Desc+Compr: 1535761    21282786    848086
	//Rand Order: 842410      12935989        1535301			Rand Order: 544576      10037050        984916				Rand Order: 417861     13004660    512533
	//Rand+Compr: 904298      11940159        2118981			Rand+Compr: 656599      12084449        1577032				Rand+Compr: 565986     13773808    807828
	//ROCKSSTG_UNBUFFERED - WriteOptionsFastRisky				ROCKSSTG_UNBUFFERED - WriteOptionsFastRisky				ROCKSSTG_UNBUFFERED - WriteOptionsFastRisky
	//Asce Order: 1990591     19486203        1461333			Asce Order: 1144991     12863190        1073432				Asce Order: 594477     15504580    553609
	//Desc Order: 1509968     19231130        1623263			Desc Order: 935493      12405354        1032636				Desc Order: 741282     18691929    496552
	//Desc+Compr: 1961948     22855612        2385378			Desc+Compr: 1162328     16230266        1677291				Desc+Compr: 1537722    21467505    790474
	//Rand Order: 814625      8617715         1494277			Rand Order: 550593      10041775        1027449				Rand Order: 294542     13129551    534549
	//Rand+Compr: 941886      13580181        2197835			Rand+Compr: 646134      11056643        1609582				Rand+Compr: 617939     13524222    771488
	//ROCKSSTG_STRONG - WriteOptionsFastRisky					ROCKSSTG_STRONG - WriteOptionsFastRisky					ROCKSSTG_STRONG - WriteOptionsFastRisky
	//Asce Order: 1986189     19510336        1605609			Asce Order: 1105031     12174678        1048738				Asce Order: 1402182    18116839    488892
	//Desc Order: 1544122     17148981        1484828			Desc Order: 910659      12980098        1051921				Desc Order: 1198487    18238489    534471
	//Desc+Compr: 1988040     22878591        2366984			Desc+Compr: 1143022     16226666        1687404				Desc+Compr: 1466985    22705099    793701
	//Rand Order: 836567      13338543        1560799			Rand Order: 540766      10011634        1082208				Rand Order: 413760     12686613    460277
	//Rand+Compr: 948912      12566730        2116865			Rand+Compr: 603076      11247308        1474983				Rand+Compr: 592750     13157894    853167
	//ROCKSSTG_FASTEST - WriteOptionsDefault					ROCKSSTG_FASTEST - WriteOptionsDefault					ROCKSSTG_FASTEST - WriteOptionsDefault
	//Asce Order: 361997      17424152        1367176			Asce Order: 147521      12752338        1087254				Asce Order: 219073     15970305    489711
	//Desc Order: 343889      17396031        1595274			Desc Order: 138703      12717179        1019356				Desc Order: 215834     18141553    527876
	//Desc+Compr: 373537      22908789        2407751			Desc+Compr: 146023      16210739        1648791				Desc+Compr: 185060     13428979    671346
	//Rand Order: 279890      12283037        1495525			Rand Order: 124057      10029775        1033631				Rand Order: 152521     10909052    536152
	//Rand+Compr: 282770      12959236        2166622			Rand+Compr: 129771      12172941        1586925				Rand+Compr: 169758     9591247     742205
	//ROCKSSTG_DEFAULT - WriteOptionsDefault					ROCKSSTG_DEFAULT - WriteOptionsDefault					ROCKSSTG_DEFAULT - WriteOptionsDefault
	//Asce Order: 299899      12912662        1287069			Asce Order: 145513      12955301        1040197				Asce Order: 189913     17422373    540624
	//Desc Order: 351548      19312738        1518370			Desc Order: 140691      12912174        1035229				Desc Order: 205955     17479473    460539
	//Desc+Compr: 368541      22418284        2320322			Desc+Compr: 146304      16225380        1583901				Desc+Compr: 197788     17065813    766067
	//Rand Order: 280780      11558997        1519021			Rand Order: 124097      9767637 1024537						Rand Order: 152264     11569444    497687
	//Rand+Compr: 303718      13076903        2263038			Rand+Compr: 128670      12401598        1584423				Rand+Compr: 172546     13504602    657699
	//ROCKSSTG_UNBUFFERED - WriteOptionsDefault				ROCKSSTG_UNBUFFERED - WriteOptionsDefault					ROCKSSTG_UNBUFFERED - WriteOptionsDefault
	//Asce Order: 373988      19434427        1492578			Asce Order: 146721      12964322        1046979				Asce Order: 214872     16501224    498653
	//Desc Order: 317104      19371925        1570549			Desc Order: 140484      12824045        998004				Desc Order: 202687     17270166    467851
	//Desc+Compr: 342619      21583340        2410341			Desc+Compr: 144364      16088233        1670557				Desc+Compr: 192490     20801170    666498
	//Rand Order: 253407      10175891        1497531			Rand Order: 123405      10034198        1057974				Rand Order: 151901     13172281    428023
	//Rand+Compr: 288887      12736318        1979451			Rand+Compr: 129344      11053779        1596963				Rand+Compr: 178632     9668860     787233
	//ROCKSSTG_STRONG - WriteOptionsDefault					ROCKSSTG_STRONG - WriteOptionsDefault						ROCKSSTG_STRONG - WriteOptionsDefault
	//Asce Order: 374028      19315288        1408939			Asce Order: 142883      12139749        1031852				Asce Order: 218338     16905779    543460
	//Desc Order: 339521      18857153        1626483			Desc Order: 138219      12919179        1036582				Desc Order: 190053     18399396    523554
	//Desc+Compr: 379595      22998832        2427195			Desc+Compr: 143721      16117861        1676572				Desc+Compr: 182842     14763126    816510
	//Rand Order: 278343      10042169        1446764			Rand Order: 123227      9965257 1057139						Rand Order: 143805     11083330    472800
	//Rand+Compr: 294540      11438145        2196204			Rand+Compr: 129064      12240019        1636577				Rand+Compr: 172461     12502441    763272
	//ROCKSSTG_FASTEST - WriteOptionsRobust					ROCKSSTG_FASTEST - WriteOptionsRobust						ROCKSSTG_FASTEST - WriteOptionsRobust
	//Asce Order: 467         18517179        2051898			Asce Order: 56			12397094        1404760				Asce Order: 12704      20257171    228967
	//Desc Order: 459         18840846        2257246			Desc Order: 39			12389594        1457547				Desc Order: 10717      19068901    940225
	//Desc+Compr: 455         21950696        3448972			Desc+Compr: 46			15014662        2291340				Desc+Compr: 12987      22333696    1400820
	//Rand Order: 472         16939619        2272021			Rand Order: 46			12300300        1345774				Rand Order: 9895       20098135    980655
	//Rand+Compr: 466         20317460        3362337			Rand+Compr: 53			15069904        2162618				Rand+Compr: 12704      23486238    1298997
	//ROCKSSTG_DEFAULT - WriteOptionsRobust					ROCKSSTG_DEFAULT - WriteOptionsRobust						ROCKSSTG_DEFAULT - WriteOptionsRobust
	//Asce Order: 463         18500451        2137563			Asce Order: 39			12579852        1528244				Asce Order: 14415      20297324    969742
	//Desc Order: 466         18635122        2144951			Desc Order: 34			12300300        1514568				Desc Order: 14836      20439121    245525
	//Desc+Compr: 470         22285092        3185069			Desc+Compr: 38			15609756        2309427				Desc+Compr: 11661      23089064    1370633
	//Rand Order: 460         18285714        2253769			Rand Order: 38			12427184        1368618				Rand Order: 12041      20237154    827341
	//Rand+Compr: 455         20500500        3087126			Rand+Compr: 46			15136733        2211663				Rand+Compr: 13919      23115124    1281120
	//ROCKSSTG_UNBUFFERED - WriteOptionsRobust				ROCKSSTG_UNBUFFERED - WriteOptionsRobust					ROCKSSTG_UNBUFFERED - WriteOptionsRobust
	//Asce Order: 468         19140186        2314124			Asce Order: 46			11790443        1491624				Asce Order: 14830      20237154    857872
	//Desc Order: 461         15352323        2190140			Desc Order: 51			11544532        1543330				Desc Order: 14786      20157480    1043460
	//Desc+Compr: 457         21833688        3373414			Desc+Compr: 46			14755043        2032552				Desc+Compr: 14675      23115124    1470736
	//Rand Order: 457         18302055        2035380			Rand Order: 45			12322503        1481481				Rand Order: 14555      20257171    874690
	//Rand+Compr: 464         21222797        3471186			Rand+Compr: 44			13950953        2215251				Rand+Compr: 14001      23167420    1476461
	//ROCKSSTG_STRONG - WriteOptionsRobust					ROCKSSTG_STRONG - WriteOptionsRobust						ROCKSSTG_STRONG - WriteOptionsRobust
	//Asce Order: 460         18771769        2116577			Asce Order: 47			12427184        1567306				Asce Order: 14262      20078431    992103
	//Desc Order: 464         17731601        2326743			Desc Order: 46			12397094        1305955				Desc Order: 14912      20197238    913836
	//Desc+Compr: 458         22212581        3246670			Desc+Compr: 46			15329341        2310208				Desc+Compr: 14825      23193657    1533393
	//Rand Order: 457         17824194        2291340			Rand Order: 46			12263473        1556467				Rand Order: 14383      20277227    842174
	//Rand+Compr: 458         21178903        3389045			Rand+Compr: 46			15260804        2036797				Rand+Compr: 14668      23063063    1380519
    

	// Release Build Z440 Samsung SSD 960 (NVMe)
	// No PointLoopup Opti									PointLoopup (Cache=200MB)						PointLoopup (Cache=10MB)
	//ROCKSSTG_FASTEST - WriteOptionsFastRisky				ROCKSSTG_FASTEST - WriteOptionsFastRisky		ROCKSSTG_FASTEST - WriteOptionsFastRisky
	//Asce Order: 1708204     1486967						Asce Order: 1676736     1363134					Asce Order: 1572964     1469490
	//Desc Order: 1121933     1202555						Desc Order: 961520      1336709					Desc Order: 1411012     1530391
	//Rand Order: 680769      933293						Rand Order: 767087      987559					Rand Order: 796856      945181
	//ROCKSSTG_DEFAULT - WriteOptionsFastRisky				ROCKSSTG_DEFAULT - WriteOptionsFastRisky		ROCKSSTG_DEFAULT - WriteOptionsFastRisky
	//Asce Order: 1893036     1553421						Asce Order: 1916346     1466334					Asce Order: 1722977     1452214
	//Desc Order: 800281      1370853						Desc Order: 741539      1454049					Desc Order: 800481      1508411
	//Rand Order: 805215      823018						Rand Order: 810293      644811					Rand Order: 771915      844675
	//ROCKSSTG_UNBUFFERED - WriteOptionsFastRisky			ROCKSSTG_UNBUFFERED - WriteOptionsFastRisky		ROCKSSTG_UNBUFFERED - WriteOptionsFastRisky
	//Asce Order: 1995051     1360020						Asce Order: 1014032     897308					Asce Order: 1794822     1291298
	//Desc Order: 1532039     1418872						Desc Order: 911745      1249100					Desc Order: 1534726     1507611
	//Rand Order: 828197      914285						Rand Order: 521443      535254					Rand Order: 848159      991633
	//ROCKSSTG_STRONG - WriteOptionsFastRisky				ROCKSSTG_STRONG - WriteOptionsFastRisky			ROCKSSTG_STRONG - WriteOptionsFastRisky
	//Asce Order: 1954571     1499729						Asce Order: 1264384     1086333					Asce Order: 1965526     1419521
	//Desc Order: 1537375     1494650						Desc Order: 1428531     1283481					Desc Order: 1327749     1560285
	//Rand Order: 846973      993981						Rand Order: 757917      924304					Rand Order: 638280      886388
	//ROCKSSTG_FASTEST - WriteOptionsDefault				ROCKSSTG_FASTEST - WriteOptionsDefault			ROCKSSTG_FASTEST - WriteOptionsDefault
	//Asce Order: 385075      1560761						Asce Order: 318832      1442111					Asce Order: 364154      1064648
	//Desc Order: 360084      1505461						Desc Order: 341701      1468795					Desc Order: 345293      1561308
	//Rand Order: 290181      931688						Rand Order: 264201      741727					Rand Order: 253991      726648
	//ROCKSSTG_DEFAULT - WriteOptionsDefault				ROCKSSTG_DEFAULT - WriteOptionsDefault			ROCKSSTG_DEFAULT - WriteOptionsDefault
	//Asce Order: 385701      1442070						Asce Order: 366726      1480303					Asce Order: 344714      1498478
	//Desc Order: 364459      1431087						Desc Order: 329948      1491363					Desc Order: 311515      1410662
	//Rand Order: 276899      897796						Rand Order: 276056      1005419					Rand Order: 259307      813925
	//ROCKSSTG_UNBUFFERED - WriteOptionsDefault				ROCKSSTG_UNBUFFERED - WriteOptionsDefault		ROCKSSTG_UNBUFFERED - WriteOptionsDefault
	//Asce Order: 384924      1335176						Asce Order: 374653      1559169					Asce Order: 328215      1271449
	//Desc Order: 360400      1512577						Desc Order: 345278      1409226					Desc Order: 357693      1493516
	//Rand Order: 275960      791265						Rand Order: 220893      636578					Rand Order: 244743      745665
	//ROCKSSTG_STRONG - WriteOptionsDefault					ROCKSSTG_STRONG - WriteOptionsDefault			ROCKSSTG_STRONG - WriteOptionsDefault
	//Asce Order: 381612      1485629						Asce Order: 346872      1484854					Asce Order: 331075      1458689
	//Desc Order: 265403      1461541						Desc Order: 325386      1389888					Desc Order: 358666      1428850
	//Rand Order: 294612      928267						Rand Order: 261373      973541					Rand Order: 293455      930562
	//ROCKSSTG_FASTEST - WriteOptionsRobust					ROCKSSTG_FASTEST - WriteOptionsRobust			ROCKSSTG_FASTEST - WriteOptionsRobust
	//Asce Order: 526         1921200						Asce Order: 538			2021717					Asce Order: 509			1872029
	//Desc Order: 533         1663688						Desc Order: 532			2087665					Desc Order: 529			2060362
	//Rand Order: 529         1866909						Rand Order: 529			1956064					Rand Order: 531			1569348
	//ROCKSSTG_DEFAULT - WriteOptionsRobust					ROCKSSTG_DEFAULT - WriteOptionsRobust			ROCKSSTG_DEFAULT - WriteOptionsRobust
	//Asce Order: 539         1695364						Asce Order: 530			2133333					Asce Order: 533			1974927
	//Desc Order: 547         2003913						Desc Order: 539			1915809					Desc Order: 537			2019723
	//Rand Order: 532         1662337						Rand Order: 537			1967339					Rand Order: 535			1971126
	//ROCKSSTG_UNBUFFERED - WriteOptionsRobust				ROCKSSTG_UNBUFFERED - WriteOptionsRobust		ROCKSSTG_UNBUFFERED - WriteOptionsRobust
	//Asce Order: 538         2023715						Asce Order: 543			2087665					Asce Order: 521			1742978
	//Desc Order: 543         1994157						Desc Order: 535			1996101					Desc Order: 547			1933899
	//Rand Order: 528         1978743						Rand Order: 544			1954198					Rand Order: 531			1919400
	//ROCKSSTG_STRONG - WriteOptionsRobust					ROCKSSTG_STRONG - WriteOptionsRobust			ROCKSSTG_STRONG - WriteOptionsRobust
	//Asce Order: 532         2235807						Asce Order: 541			1954198					Asce Order: 533			1667752
	//Desc Order: 543         2089795						Desc Order: 534			1631872					Desc Order: 533			2111340
	//Rand Order: 541         2033763						Rand Order: 537			1923004					Rand Order: 533			1870319

	struct cmp: public ::rocksdb::Comparator
	{
		// Three-way comparison.  Returns value:
		//   < 0 iff "a" < "b",
		//   == 0 iff "a" == "b",
		//   > 0 iff "a" > "b"
		virtual int Compare(const ::rocksdb::Slice& a, const ::rocksdb::Slice& b) const override
		{
			auto x = *(UINT*)a.data();
			auto y = *(UINT*)b.data();
			if(x>y)return -1;
			else if(x<y)return +1;
			return 0;
		}

		virtual bool Equal(const ::rocksdb::Slice& a, const ::rocksdb::Slice& b) const override 
		{	
			return  *(UINT*)a.data() ==  *(UINT*)b.data();
		}

		virtual const char* Name() const { return "test_cmp"; }
		virtual void FindShortestSeparator(std::string* start, const ::rocksdb::Slice& limit) const {}
		virtual void FindShortSuccessor(std::string* key) const {}
	};


	os::SetProcessPriority(os::PROCPRIO_REALTIME);

	rt::Buffer<ULONGLONG>	keys;
	keys.SetSize(100*1024);

	/*
	{
		cmp	_cmp;
		::rocksdb::ColumnFamilyOptions opt;
		opt.comparator = &_cmp;

		auto test = [&keys, fn, &opt](ext::RocksStorageWriteRobustness wr, const ext::WriteOptions* w_opt)
		{
			for(UINT i=0; i<keys.GetSize(); i++)
			{	keys[i] = i;
				rt::SwapByteOrder(keys[i]);
			}

			os::HighPerformanceCounter hpc;
			hpc.SetOutputUnit(1000U);
		
			{	ext::RocksStorage::Nuke(fn);
				ext::RocksStorage store;
				store.Open(fn, wr);
				auto db = store.Get("q");

				hpc.LoadCurrentCount();
				for(UINT i=0; i<keys.GetSize(); i++)
					db.Set(keys[i], keys[i], w_opt);
				ULONGLONG insert = keys.GetSize()*1000000LL/rt::max(1LL, hpc.TimeLapse());

				hpc.LoadCurrentCount();
				for(UINT i=0; i<10; i++)
				{
					auto it = db.First();
					while(it.IsValid())it.Next();
				}
				ULONGLONG scan = keys.GetSize()*10000000LL/rt::max(1LL, hpc.TimeLapse());

				hpc.LoadCurrentCount();
				for(UINT i=0; i<10; i++)
				{
					auto it = db.Last();
					while(it.IsValid())it.Prev();
				}
				_LOG("Asce Order: "<<insert<<'\t'<<scan<<'\t'<<keys.GetSize()*10000000LL/rt::max(1LL, hpc.TimeLapse()));
			}

			{	ext::RocksStorage::Nuke(fn);
				ext::RocksStorage store;
				store.Open(fn, wr);
				auto db = store.Get("q");

				hpc.LoadCurrentCount();
				for(UINT i=0; i<keys.GetSize(); i++)
					db.Set(keys[keys.GetSize() - i - 1], keys[i], w_opt);
				ULONGLONG insert = keys.GetSize()*1000000LL/rt::max(1LL, hpc.TimeLapse());

				hpc.LoadCurrentCount();
				for(UINT i=0; i<10; i++)
				{
					auto it = db.First();
					while(it.IsValid())it.Next();
				}
				ULONGLONG scan = keys.GetSize()*10000000LL/rt::max(1LL, hpc.TimeLapse());

				hpc.LoadCurrentCount();
				for(UINT i=0; i<10; i++)
				{
					auto it = db.Last();
					while(it.IsValid())it.Prev();
				}
				_LOG("Desc Order: "<<insert<<'\t'<<scan<<'\t'<<keys.GetSize()*10000000LL/rt::max(1LL, hpc.TimeLapse()));
			}

			{	ext::RocksStorage::Nuke(fn);
				ext::RocksStorage store;

				for(UINT i=0; i<keys.GetSize(); i++)
					keys[i] = i;

				store.SetDBOpenOption("q", opt);
				store.Open(fn, wr);
				auto db = store.Get("q");

				hpc.LoadCurrentCount();
				for(UINT i=0; i<keys.GetSize(); i++)
					db.Set(keys[keys.GetSize() - i - 1], keys[i], w_opt);
				ULONGLONG insert = keys.GetSize()*1000000LL/rt::max(1LL, hpc.TimeLapse());

				hpc.LoadCurrentCount();
				for(UINT i=0; i<10; i++)
				{
					auto it = db.First();
					while(it.IsValid())it.Next();
				}
				ULONGLONG scan = keys.GetSize()*10000000LL/rt::max(1LL,hpc.TimeLapse());

				hpc.LoadCurrentCount();
				for(UINT i=0; i<10; i++)
				{
					auto it = db.Last();
					while(it.IsValid())it.Prev();
				}
				_LOG("Desc+Compr: "<<insert<<'\t'<<scan<<'\t'<<keys.GetSize()*10000000LL/rt::max(1LL, hpc.TimeLapse()));
			}

			{	ext::RocksStorage::Nuke(fn);
				ext::RocksStorage store;
				store.Open(fn, wr);
				auto db = store.Get("q");

				for(UINT i=0; i<keys.GetSize(); i++)
				{	keys[i] = i;
					rt::SwapByteOrder(keys[i]);
				}

				keys.Shuffle(5439);

				hpc.LoadCurrentCount();
				for(UINT i=0; i<keys.GetSize(); i++)
					db.Set(keys[i], keys[i], w_opt);
				ULONGLONG insert = keys.GetSize()*1000000LL/rt::max(1LL, hpc.TimeLapse());

				hpc.LoadCurrentCount();
				for(UINT i=0; i<10; i++)
				{
					auto it = db.First();
					while(it.IsValid())it.Next();
				}
				ULONGLONG scan = keys.GetSize()*10000000LL/rt::max(1LL, hpc.TimeLapse());

				hpc.LoadCurrentCount();
				for(UINT i=0; i<10; i++)
				{
					auto it = db.Last();
					while(it.IsValid())it.Prev();
				}
				_LOG("Rand Order: "<<insert<<'\t'<<scan<<'\t'<<keys.GetSize()*10000000LL/rt::max(1LL, hpc.TimeLapse()));
			}

			{	ext::RocksStorage::Nuke(fn);
				ext::RocksStorage store;

				store.SetDBOpenOption("q", opt);

				store.Open(fn, wr);
				auto db = store.Get("q");

				for(UINT i=0; i<keys.GetSize(); i++)
					keys[i] = i;
				keys.Shuffle(5439);

				hpc.LoadCurrentCount();
				for(UINT i=0; i<keys.GetSize(); i++)
					db.Set(keys[i], keys[i], w_opt);
				ULONGLONG insert = keys.GetSize()*1000000LL/rt::max(1LL, hpc.TimeLapse());

				hpc.LoadCurrentCount();
				for(UINT i=0; i<10; i++)
				{
					auto it = db.First();
					while(it.IsValid())it.Next();
				}
				ULONGLONG scan = keys.GetSize()*10000000LL/rt::max(1LL, hpc.TimeLapse());

				hpc.LoadCurrentCount();
				for(UINT i=0; i<10; i++)
				{
					auto it = db.Last();
					while(it.IsValid())it.Prev();
				}
				_LOG("Rand+Compr: "<<insert<<'\t'<<scan<<'\t'<<keys.GetSize()*10000000LL/rt::max(1LL, hpc.TimeLapse()));
			}
		};

		_LOG_HIGHLIGHT("ROCKSSTG_FASTEST - WriteOptionsFastRisky");
		test(ext::ROCKSSTG_FASTEST, ext::RocksDB::WriteOptionsFastRisky);

		_LOG_HIGHLIGHT("ROCKSSTG_DEFAULT - WriteOptionsFastRisky");
		test(ext::ROCKSSTG_DEFAULT, ext::RocksDB::WriteOptionsFastRisky);

		_LOG_HIGHLIGHT("ROCKSSTG_UNBUFFERED - WriteOptionsFastRisky");
		test(ext::ROCKSSTG_UNBUFFERED, ext::RocksDB::WriteOptionsFastRisky);

		_LOG_HIGHLIGHT("ROCKSSTG_STRONG - WriteOptionsFastRisky");
		test(ext::ROCKSSTG_STRONG, ext::RocksDB::WriteOptionsFastRisky);


		_LOG_HIGHLIGHT("ROCKSSTG_FASTEST - WriteOptionsDefault");
		test(ext::ROCKSSTG_FASTEST, ext::RocksDB::WriteOptionsDefault);

		_LOG_HIGHLIGHT("ROCKSSTG_DEFAULT - WriteOptionsDefault");
		test(ext::ROCKSSTG_DEFAULT, ext::RocksDB::WriteOptionsDefault);

		_LOG_HIGHLIGHT("ROCKSSTG_UNBUFFERED - WriteOptionsDefault");
		test(ext::ROCKSSTG_UNBUFFERED, ext::RocksDB::WriteOptionsDefault);

		_LOG_HIGHLIGHT("ROCKSSTG_STRONG - WriteOptionsDefault");
		test(ext::ROCKSSTG_STRONG, ext::RocksDB::WriteOptionsDefault);

		keys.ShrinkSize(2*1024);

		_LOG_HIGHLIGHT("ROCKSSTG_FASTEST - WriteOptionsRobust");
		test(ext::ROCKSSTG_FASTEST, ext::RocksDB::WriteOptionsRobust);

		_LOG_HIGHLIGHT("ROCKSSTG_DEFAULT - WriteOptionsRobust");
		test(ext::ROCKSSTG_DEFAULT, ext::RocksDB::WriteOptionsRobust);

		_LOG_HIGHLIGHT("ROCKSSTG_UNBUFFERED - WriteOptionsRobust");
		test(ext::ROCKSSTG_UNBUFFERED, ext::RocksDB::WriteOptionsRobust);

		_LOG_HIGHLIGHT("ROCKSSTG_STRONG - WriteOptionsRobust");
		test(ext::ROCKSSTG_STRONG, ext::RocksDB::WriteOptionsRobust);
	}
	*/
	/*
	{
		::rocksdb::ColumnFamilyOptions opt;
		opt.OptimizeForPointLookup(10);

		auto test = [&keys, fn, &opt](ext::RocksStorageWriteRobustness wr, const ext::WriteOptions* w_opt)
		{
			for(UINT i=0; i<keys.GetSize(); i++)
			{	keys[i] = i;
				rt::SwapByteOrder(keys[i]);
			}

			os::HighPerformanceCounter hpc;
			hpc.SetOutputUnit(1000U);
		
			{	ext::RocksStorage::Nuke(fn);
				ext::RocksStorage store;
				store.Open(fn, wr);
				auto db = store.Get("q");

				hpc.LoadCurrentCount();
				for(UINT i=0; i<keys.GetSize(); i++)
					db.Set(keys[i], keys[i], w_opt);
				ULONGLONG insert = keys.GetSize()*1000000LL/rt::max(1LL, hpc.TimeLapse());

				hpc.LoadCurrentCount();
				std::string str;
				for(UINT i=0; i<keys.GetSize(); i++)
					db.Get(keys[i], str);
				ULONGLONG lookup = keys.GetSize()*1000000LL/rt::max(1LL, hpc.TimeLapse());

				_LOG("Asce Order: "<<insert<<'\t'<<lookup);
			}

			{	ext::RocksStorage::Nuke(fn);
				ext::RocksStorage store;
				store.Open(fn, wr);
				auto db = store.Get("q");

				hpc.LoadCurrentCount();
				for(UINT i=0; i<keys.GetSize(); i++)
					db.Set(keys[keys.GetSize() - i - 1], keys[i], w_opt);
				ULONGLONG insert = keys.GetSize()*1000000LL/rt::max(1LL, hpc.TimeLapse());

				hpc.LoadCurrentCount();
				std::string str;
				for(UINT i=0; i<keys.GetSize(); i++)
					db.Get(keys[i], str);
				ULONGLONG lookup = keys.GetSize()*1000000LL/rt::max(1LL, hpc.TimeLapse());

				_LOG("Desc Order: "<<insert<<'\t'<<lookup);
			}

			{	ext::RocksStorage::Nuke(fn);
				ext::RocksStorage store;
				store.Open(fn, wr);
				auto db = store.Get("q");

				for(UINT i=0; i<keys.GetSize(); i++)
				{	keys[i] = i;
					rt::SwapByteOrder(keys[i]);
				}

				keys.Shuffle(5439);

				hpc.LoadCurrentCount();
				for(UINT i=0; i<keys.GetSize(); i++)
					db.Set(keys[i], keys[i], w_opt);
				ULONGLONG insert = keys.GetSize()*1000000LL/rt::max(1LL, hpc.TimeLapse());

				hpc.LoadCurrentCount();
				std::string str;
				for(UINT i=0; i<keys.GetSize(); i++)
					db.Get(keys[i], str);
				ULONGLONG lookup = keys.GetSize()*1000000LL/rt::max(1LL, hpc.TimeLapse());

				_LOG("Rand Order: "<<insert<<'\t'<<lookup);
			}
		};

		_LOG_HIGHLIGHT("ROCKSSTG_FASTEST - WriteOptionsFastRisky");
		test(ext::ROCKSSTG_FASTEST, ext::RocksDB::WriteOptionsFastRisky);

		_LOG_HIGHLIGHT("ROCKSSTG_DEFAULT - WriteOptionsFastRisky");
		test(ext::ROCKSSTG_DEFAULT, ext::RocksDB::WriteOptionsFastRisky);

		_LOG_HIGHLIGHT("ROCKSSTG_UNBUFFERED - WriteOptionsFastRisky");
		test(ext::ROCKSSTG_UNBUFFERED, ext::RocksDB::WriteOptionsFastRisky);

		_LOG_HIGHLIGHT("ROCKSSTG_STRONG - WriteOptionsFastRisky");
		test(ext::ROCKSSTG_STRONG, ext::RocksDB::WriteOptionsFastRisky);


		_LOG_HIGHLIGHT("ROCKSSTG_FASTEST - WriteOptionsDefault");
		test(ext::ROCKSSTG_FASTEST, ext::RocksDB::WriteOptionsDefault);

		_LOG_HIGHLIGHT("ROCKSSTG_DEFAULT - WriteOptionsDefault");
		test(ext::ROCKSSTG_DEFAULT, ext::RocksDB::WriteOptionsDefault);

		_LOG_HIGHLIGHT("ROCKSSTG_UNBUFFERED - WriteOptionsDefault");
		test(ext::ROCKSSTG_UNBUFFERED, ext::RocksDB::WriteOptionsDefault);

		_LOG_HIGHLIGHT("ROCKSSTG_STRONG - WriteOptionsDefault");
		test(ext::ROCKSSTG_STRONG, ext::RocksDB::WriteOptionsDefault);

		keys.ShrinkSize(2*1024);

		_LOG_HIGHLIGHT("ROCKSSTG_FASTEST - WriteOptionsRobust");
		test(ext::ROCKSSTG_FASTEST, ext::RocksDB::WriteOptionsRobust);

		_LOG_HIGHLIGHT("ROCKSSTG_DEFAULT - WriteOptionsRobust");
		test(ext::ROCKSSTG_DEFAULT, ext::RocksDB::WriteOptionsRobust);

		_LOG_HIGHLIGHT("ROCKSSTG_UNBUFFERED - WriteOptionsRobust");
		test(ext::ROCKSSTG_UNBUFFERED, ext::RocksDB::WriteOptionsRobust);

		_LOG_HIGHLIGHT("ROCKSSTG_STRONG - WriteOptionsRobust");
		test(ext::ROCKSSTG_STRONG, ext::RocksDB::WriteOptionsRobust);
	}
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
