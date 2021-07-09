#include "../../core/ext/sparsehash/sparsehash.h"
#include "../../core/ext/rocksdb/rocksdb.h"
#include "../../core/ext/rocksdb/rocksdb_serving.h"
#include "../../core/ext/concurrentqueue/async_queue.h"
#include "../../core/os/kernel.h"
#include "../../core/ext/exprtk/exprtk.h"
#include "../../core/ext/bignum/precision_num.h"
#include "../../core/ext/bignum/big_num.h"
#include "test.h"



void rt::UnitTests::express_tk()
{
    typedef double T;
    
    rt::Expression<T> Expr;
    T x[100];
    
    Expr.BindVariable("x", x, 100);
    
    for(UINT i=0;i<sizeofArray(x);i++)x[i] = 0.1f;
    
    rt::String_Ref source = __STRING(
                                     for(var i=1;i<100;i++)
                                     {    x[i-1] = x[i] - 0.1;
                                     }
                                     );
    
    if(Expr.Compile(source))
    {
        os::Timestamp tm;
        tm.LoadCurrentTime();
        
        for(UINT i=0;i<100000;i++)
            Expr.Compute();
        
        _LOGC("ExprTk: "<<rt::tos::TimeSpan<false>(tm.TimeLapse()));
        _LOG(x[0]);
        
        for(UINT i=0;i<sizeofArray(x);i++)x[i] = 0.1f;
        tm.LoadCurrentTime();
        
        for(UINT i=0;i<100000;i++)
            for(UINT i=1;i<sizeofArray(x);i++)
            {    x[i-1] = x[i] - 0.1;
            }
        
        _LOGC("Native: "<<rt::tos::TimeSpan<false>(tm.TimeLapse()));
        _LOG(x[0]);
    }
    else
    {    _LOG_ERROR(Expr.GetLastError());
    }
}


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


void rt::UnitTests::rocksdb_serve()
{
	ext::RocksDBStandalone	db;
	db.Open("serv.db");

	for(UINT i=0; i<1000; i++)
	{
		db.Set(rt::tos::Number(i), rt::String(rt::SS("value ") + rt::tos::Number(i)));
	}

	ext::RocksDBServe server;
	server.RocksMap(&db, "/test", ext::RocksDBServe::KF_STRING, inet::TinyHttpd::MIME_STRING_TEXT);
	server.Start(1230);

	_LOG("RocksServing at http://"<<rt::tos::ip(server.GetBindedAddress()));

	os::Sleep();
}

void rt::UnitTests::big_num()
{
	
	/*
	 2's power
	 28    268435456
	 29    536870912
	 30    1073741824
	 31    2147483648
	 32    4294967296
	 33    8589934592
	 62    4611686018427387904
	 63    9223372036854775808
	 64    18446744073709551616
	 65    36893488147419103232
	 66    73786976294838206464
	 128   340282366920938463463374607431768211456
	 192   6277101735386680763835789423207666416102355444464034512896
	 4000  13182040934309431001038897942365913631840191610932727690928034502417569281128344551079752123172122033140940756480716823038446817694240581281731062452512184038544674444386888956328970642771993930036586552924249514488832183389415832375620009284922608946111038578754077913265440918583125586050431647284603636490823850007826811672468900210689104488089485347192152708820119765006125944858397761874669301278745233504796586994514054435217053803732703240283400815926169348364799472716094576894007243168662568886603065832486830606125017643356469732407252874567217733694824236675323341755681839221954693820456072020253884371226826844858636194212875139566587445390068014747975813971748114770439248826688667129237954128555841874460665729630492658600179338272579
	 8000  173766203193809456599982445949435627061939786100117250547173286503262376022458008465094333630120854338003194362163007597987225472483598640843335685441710193966274131338557192586399006789292714554767500194796127964596906605976605873665859580600161998556511368530960400907199253450604168622770350228527124626728538626805418833470107651091641919900725415994689920112219170907023561354484047025713734651608777544579846111001059482132180956689444108315785401642188044178788629853592228467331730519810763559577944882016286493908631503101121166109571682295769470379514531105239965209245314082665518579335511291525230373316486697786532335206274149240813489201828773854353041855598709390675430960381072270432383913542702130202430186637321862331068861776780211
	 */
	
	/*
	 if it is an early out case, quotient and remainder will both be 0.
	 format:
	 //correct answer
	 //output quotient
	 //output remainder
	 */
	using namespace ext;
	
	BigNumMutable remainder = 0;
	BigNumMutable u, v;
	BigNumMutable q = 0;
	
	//Correct answer: 399.99999999999999411508
	//399
	//368934881474188859280
	u.FromString("147573952589676410001249");
	v.FromString("368934881474191030431");
	BN_Div(u, v, &remainder, q);
	_LOG(q);
	_LOG(remainder);
	
	//-399
	//-368934881474188859280
		
	u.FromString("-147573952589676410001249");
	v.FromString("368934881474191030431");
	BN_Div(u, v, &remainder, q);
	_LOG(q);
	_LOG(remainder);
	
	//-399
	//368934881474188859280
	u.FromString("147573952589676410001249");
	v.FromString("-368934881474191030431");
	BN_Div(u, v, &remainder, q);
	_LOG(q);
	_LOG(remainder);
	
	//399
	//-368934881474188859280
	u.FromString("-147573952589676410001249");
	v.FromString("-368934881474191030431");
	BN_Div(u, v, &remainder, q);
	_LOG(q);
	_LOG(remainder);
	
	//2
	//0
	u.FromString("2");
	v.FromString("1");
	BN_Div(u, v, &remainder, q);
	_LOG(q);
	_LOG(remainder);
	
	//correct answer: 20,000,000,000,000,000,102.45289519125589431657, b ~ 2^132 (3 blocks), a ~ 68, Bad
	//20000000000000000101
	//167088833706224447654
	u.FromString("7378697629483820646418446744073709551616");
	v.FromString("368934881474191030431");
	BN_Div(u, v, &remainder, q);
	_LOG(q);
	_LOG(remainder);
	
	//correct answer: 139,248,928,633,429,192,196,931.51549477934584516444, b ~ 2^175 (3 blocks), a ~ 98, also the special case from the book
	//139248928633429192196931
	//190184005302496392418143744355
	u.FromString("51373786976452895191329483820646418446744073709551616");
	v.FromString("368934881443100023474191030431");
	BN_Div(u, v, &remainder, q);
	_LOG(q);
	_LOG(remainder);
	
	//correct answer: 5106324723863291849166480709076251125153902953460815077695792418627975612684228900723432001147710626581013673144599360242442907941844522541928464957468785962437.15063829409088928096, b ~ 2^640 (10 blocks), a ~ 109
	//5106324723863291849166480709076251125153902953460815077695792418627975612684228900723432001147710626581013673144599360242442907941844522541928464957468785962437
	//134593530316970653367838552104058
	u.FromString("4562440617622195218641171605700291324893228507248559930579192517347675167208677386505912811317371399778642309573594407310688704721375437998252661319722214188251994674360264950082874192246603776");
	v.FromString("893488147414641844674407389348814");
	BN_Div(u, v, &remainder, q);
	_LOG(q);
	_LOG(remainder);
	
	//correct answer: 272368421052631578947368421052631578947.3684210540650969529, b ~ 283, a ~ 155
	//272368421052631578947368421052631578947
	//31111111232163742690058479532163742690058479539
	u.FromString("23000000000000000000000000000000000000000000000000000000000000000000000000000000000007");
	v.FromString("84444444444444444444444444444444444444444444444");
	BN_Div(u, v, &remainder, q);
	_LOG(q);
	_LOG(remainder);
	
	//correct answer: 2723684210526315789473684210526315789473.68421054065096952908, b ~ 286, a ~ 155
	//2723684210526315789473684210526315789473
	//57777778988304093567251461988304093567251461995
	u.FromString("230000000000000000000000000000000000000000000000000000000000000000000000000000000000007");
	v.FromString("84444444444444444444444444444444444444444444444");
	BN_Div(u, v, &remainder, q);
	_LOG(q);
	_LOG(remainder);
	
	//correct answer: 431359146674410236714672241630210905942867727272214328073781831763641.00000000000007235718, b ~ 342, a ~ 114, diff = 128
	//431359146674410236714672241630210905942867727272214328073781831763641
	//1502799842158564253441
	u.FromString("8958978968711216842229769122273777112486581988938598139599956403855167484720643781523488428731772010670");
	v.FromString("20769187434139310514121985305426069");
	BN_Div(u, v, &remainder, q);
	_LOG(q);
	_LOG(remainder);
	
	//correct answer: 107,839,786,668,602,559,178,668,060,407,552,726,485,716,931,818,053,582,018,445,457,940,910.25000000000001808929
	//107839786668602559178668060407552726485716931818053582018445457940910
	//20769187434140813313964143869679510
	u.FromString("8958978968711216842229769122273777112486581988938598139599956403855167484720643781523488428731772010670");
	v.FromString("83076749736557242056487941221704276");
	BN_Div(u, v, &remainder, q);
	_LOG(q);
	_LOG(remainder);
	
	//correct answer: 18,446,744,073,709,551,616, b ~ 255, a ~ 191, diff = 126
	//18446744073709551616
	//0
	u.FromString("57896044618658097705508390768957273162799202909612615603626436559492530307072");
	v.FromString("3138550867693340381577612344682894744587803114800249044992");
	BN_Div(u, v, &remainder, q);
	_LOG(q);
	_LOG(remainder);
	
	//correct answer: 55365106547251187979686850736174759588956853304879202009049268523840669787352347279472705278372566769864758105890971829737223256827255515010930910433219532001736482266227175758322609122416905443347738284523965467765091350045486387150050073456354064882262455296626801147894627536448407815835060751939243782776076531010759921010725497696875933100911336561671801564967841007210383549221764785643872137873204518419782313700546399494990571937447419040965882886702010145067406415421926189224296322929556295751795766570945679521448780084877474272563958448580796034737590793477225980246701467244969486534908877489247387329176401471931142260175929557014377713846677185315299440589884013953328024192707.70250284824299847133, b ~ 8000, a ~ 191, diff = 126
	//55365106547251187979686850736174759588956853304879202009049268523840669787352347279472705278372566769864758105890971829737223256827255515010930910433219532001736482266227175758322609122416905443347738284523965467765091350045486387150050073456354064882262455296626801147894627536448407815835060751939243782776076531010759921010725497696875933100911336561671801564967841007210383549221764785643872137873204518419782313700546399494990571937447419040965882886702010145067406415421926189224296322929556295751795766570945679521448780084877474272563958448580796034737590793477225980246701467244969486534908877489247387329176401471931142260175929557014377713846677185315299440589884013953328024192707
	//2204840923910105871759158965546619174997821146813255506866
	u.FromString("173766203193809456599982445949435627061939786100117250547173286503262376022458008465094333630120854338003194362163007597987225472483598640843335685441710193966274131338557192586399006789292714554767500194796127964596906605976605873665859580600161998556511368530960400907199253450604168622770350228527124626728538626805418833470107651091641919900725415994689920112219170907023561354484047025713734651608777544579846111001059482132180956689444108315785401642188044178788629853592228467331730519810763559577944882016286493908631503101121166109571682295769470379514531105239965209245314082665518579335511291525230373316486697786532335206274149240813489201828773854353041855598709390675430960381072270432383913542702130202430186637321862331068861776780210");
	v.FromString("3138550867693340381577612344682894744587803114800249044992");
	BN_Div(u, v, &remainder, q);
	_LOG(q);
	_LOG(remainder);
	
	//correct answer: 3.2, b ~ 8000, a ~ 7995, diff = 5
	//3
	//10860387699613091037498902871839726691371236631257328159198330406453898501403625529068395851882553396125199647635187974874201592030224915052708480340106887122892133208659824536649937924330794659672968762174757997787306662873537867104116223787510124909781960533185025056699953340662760538923146889282945289170533664175338677091881728193227619993795338499668120007013698181688972584655252939107108415725548596536240381937566217633261309793090256769736587602636752761174289365849514279208233157488172722473621555126017905869289468943820072881848230143485591898719658194077497825577832130166594911208469455720326898332280418611658270950392134327550843075114298365897065115974919336917214435023817016902023994596418883137651886664832616395691803861048765
	u.FromString("173766203193809456599982445949435627061939786100117250547173286503262376022458008465094333630120854338003194362163007597987225472483598640843335685441710193966274131338557192586399006789292714554767500194796127964596906605976605873665859580600161998556511368530960400907199253450604168622770350228527124626728538626805418833470107651091641919900725415994689920112219170907023561354484047025713734651608777544579846111001059482132180956689444108315785401642188044178788629853592228467331730519810763559577944882016286493908631503101121166109571682295769470379514531105239965209245314082665518579335511291525230373316486697786532335206274149240813489201828773854353041855598709390675430960381072270432383913542702130202430186637321862331068861776780210");
	v.FromString("54301938498065455187494514359198633456856183156286640795991652032269492507018127645341979259412766980625998238175939874371007960151124575263542401700534435614460666043299122683249689621653973298364843810873789988936533314367689335520581118937550624548909802665925125283499766703313802694615734446414726445852668320876693385459408640966138099968976692498340600035068490908444862923276264695535542078627742982681201909687831088166306548965451283848682938013183763805871446829247571396041165787440863612368107775630089529346447344719100364409241150717427959493598290970387489127889160650832974556042347278601634491661402093058291354751960671637754215375571491829485325579874596684586072175119085084510119972982094415688259433324163081978459019305243815");
	BN_Div(u, v, &remainder, q);
	_LOG(q);
	_LOG(remainder);
	
	//correct answer: 4709942375182145298276880012525051054447110875461494985601937707265845059046761836066099659749564687995400227690784625484127220314885650601049171962546871819532400865329906336086287501276577875869159742455177622850367075352213374436810999675846625737034110518481222943136165508200442303984150364677574999807951564099730202549461188871715066128849418560157035626700634370036818627670525327187605296611386163665412428962888440068238148514729736805104869970284277633047705684016211464055463672577569977613374718718009797591363073461434258747928499499789744370745044312955321461751468565644447458396824469795963413366654036229718163319987098116236654639328887749426023679930278638109716214589426954958809726658171131125791029324257054.05568806636653863894
	//4709942375182145298276880012525051054447110875461494985601937707265845059046761836066099659749564687995400227690784625484127220314885650601049171962546871819532400865329906336086287501276577875869159742455177622850367075352213374436810999675846625737034110518481222943136165508200442303984150364677574999807951564099730202549461188871715066128849418560157035626700634370036818627670525327187605296611386163665412428962888440068238148514729736805104869970284277633047705684016211464055463672577569977613374718718009797591363073461434258747928499499789744370745044312955321461751468565644447458396824469795963413366654036229718163319987098116236654639328887749426023679930278638109716214589426954958809726658171131125791029324257054
	//2054527016446581682
	u.FromString("173766203193809456599982445949435627061939786100117250547173286503262376022458008465094333630120854338003194362163007597987225472483598640843335685441710193966274131338557192586399006789292714554767500194796127964596906605976605873665859580600161998556511368530960400907199253450604168622770350228527124626728538626805418833470107651091641919900725415994689920112219170907023561354484047025713734651608777544579846111001059482132180956689444108315785401642188044178788629853592228467331730519810763559577944882016286493908631503101121166109571682295769470379514531105239965209245314082665518579335511291525230373316486697786532335206274149240813489201828773854353041855598709390675430960381072270432383913542702130202430186637321862331068861776780210");
	v.FromString("36893488147419103232");
	BN_Div(u, v, &remainder, q);
	_LOG(q);
	_LOG(remainder);

	//correct answer: 132457.39695849051846594959, b ~ 57, a ~ 40
	//8577251
	//19876
	u.FromString("288230376151558121");
	v.FromString("2176023255552");
	BN_Div(u, v, &remainder, q);
	_LOG(q);
	_LOG(remainder);
	
	//correct answer: 132457.39695849051846594959
	//8577251
	//19876
	u.FromString("1375319331471");
	v.FromString("160345");
	BN_Div(u, v, &remainder, q);
	_LOG(q);
	_LOG(remainder);
	
	//correct answer: 132457.39695849051846594959
	//8577251
	//-19876
	u.FromString("-1375319331471");
	v.FromString("-160345");
	BN_Div(u, v, &remainder, q);
	_LOG(q);
	_LOG(remainder);
	
	//correct answer: 132457.39695849051846594959
	//-8577251
	//-19876
	u.FromString("-1375319331471");
	v.FromString("160345");
	BN_Div(u, v, &remainder, q);
	_LOG(q);
	_LOG(remainder);
	
	//correct answer: 132457.39695849051846594959
	//-8577251
	//19876
	u.FromString("1375319331471");
	v.FromString("-160345");
	BN_Div(u, v, &remainder, q);
	_LOG(q);
	_LOG(remainder);
	
	//correct answer: 241615780097735011624260892830535199701244446623002250532788947617386337880789906984991787400888860622598937632320807467890311064322341210098299858508685112525131442493933670084983824430594555155241913327877327588762968316837701997911326698870055505345657534832477364495315876674959632893349059807402729519467255426713356850917300233447409730890461155969390197935742690197534374171956059959222861815069269539616907649504242978336810825448625259065531771621988314782849215018120854283926965171375842009246513940822218644892866781103474446191199584940390039794426087211673181284963672832912743580690770778445057694993891268140743307979980296058885471870660045071015264321228933611809260162046252795435359954535559926531221754985263663166962643463.67579089634919575518
	//241615780097735011624260892830535199701244446623002250532788947617386337880789906984991787400888860622598937632320807467890311064322341210098299858508685112525131442493933670084983824430594555155241913327877327588762968316837701997911326698870055505345657534832477364495315876674959632893349059807402729519467255426713356850917300233447409730890461155969390197935742690197534374171956059959222861815069269539616907649504242978336810825448625259065531771621988314782849215018120854283926965171375842009246513940822218644892866781103474446191199584940390039794426087211673181284963672832912743580690770778445057694993891268140743307979980296058885471870660045071015264321228933611809260162046252795435359954535559926531221754985263663166962643463
	//486018
	u.FromString("173766203193809456599982445949435627061939786100117250547173286503262376022458008465094333630120854338003194362163007597987225472483598640843335685441710193966274131338557192586399006789292714554767500194796127964596906605976605873665859580600161998556511368530960400907199253450604168622770350228527124626728538626805418833470107651091641919900725415994689920112219170907023561354484047025713734651608777544579846111001059482132180956689444108315785401642188044178788629853592228467331730519810763559577944882016286493908631503101121166109571682295769470379514531105239965209245314082665518579335511291525230373316486697786532335206274149240813489201828773854353041855598709390675430960381072270432383913542702130202430186637321862331068861776780210");
	v.FromString("719184");
	BN_Div(u, v, &remainder, q);
	_LOG(q);
	_LOG(remainder);
	
	//correct answer: 9.33702779817126076219
	//9
	//242385
	u.FromString("6715041");
	v.FromString("719184");
	BN_Div(u, v, &remainder, q);
	_LOG(q);
	_LOG(remainder);
	
	//correct answer: 173766203193809456599982445949435627061939786100117250547173286503262376022458008465094333630120854338003194362163007597987225472483598640843335685441710193966274131338557192586399006789292714554767500194796127964596906605976605873665859580600161998556511368530960400907199253450604168622770350228527124626728538626805418833470107651091641919900725415994689920112219170907023561354484047025713734651608777544579846111001059482132180956689444108315785401642188044178788629853592228467331730519810763559577944882016286493908631503101121166109571682295769470379514531105239965209245314082665518579335511291525230373316486697786532335206274149240813489201828773854353041855598709390675430960381072270432383913542702130202430186637321862331068861776780210
	//173766203193809456599982445949435627061939786100117250547173286503262376022458008465094333630120854338003194362163007597987225472483598640843335685441710193966274131338557192586399006789292714554767500194796127964596906605976605873665859580600161998556511368530960400907199253450604168622770350228527124626728538626805418833470107651091641919900725415994689920112219170907023561354484047025713734651608777544579846111001059482132180956689444108315785401642188044178788629853592228467331730519810763559577944882016286493908631503101121166109571682295769470379514531105239965209245314082665518579335511291525230373316486697786532335206274149240813489201828773854353041855598709390675430960381072270432383913542702130202430186637321862331068861776780210
	//0
	u.FromString("173766203193809456599982445949435627061939786100117250547173286503262376022458008465094333630120854338003194362163007597987225472483598640843335685441710193966274131338557192586399006789292714554767500194796127964596906605976605873665859580600161998556511368530960400907199253450604168622770350228527124626728538626805418833470107651091641919900725415994689920112219170907023561354484047025713734651608777544579846111001059482132180956689444108315785401642188044178788629853592228467331730519810763559577944882016286493908631503101121166109571682295769470379514531105239965209245314082665518579335511291525230373316486697786532335206274149240813489201828773854353041855598709390675430960381072270432383913542702130202430186637321862331068861776780210");
	v.FromString("1");
	BN_Div(u, v, &remainder, q);
	_LOG(q);
	_LOG(remainder);
	
	//early out test case
	u.FromString("73465160877754457984611100105");
	v.FromString("2714554767500194796127964596906605976605873665859580600161998556511368530960400907199253450604168622770350228527124626728538626805418833470107651091641919900725415994689920112219170907023561354484047025713");
	BN_Div(u, v, &remainder, q);
	_LOG(q);
	_LOG(remainder);
	
	//early out test case
	u.FromString("-73465160877754457984611100105");
	v.FromString("2714554767500194796127964596906605976605873665859580600161998556511368530960400907199253450604168622770350228527124626728538626805418833470107651091641919900725415994689920112219170907023561354484047025713");
	BN_Div(u, v, &remainder, q);
	_LOG(q);
	_LOG(remainder);

	//early out test case
	u.FromString("73465160877754457984611100105");
	v.FromString("-2714554767500194796127964596906605976605873665859580600161998556511368530960400907199253450604168622770350228527124626728538626805418833470107651091641919900725415994689920112219170907023561354484047025713");
	BN_Div(u, v, &remainder, q);
	_LOG(q);
	_LOG(remainder);
	
	//early out test case
	u.FromString("-73465160877754457984611100105");
	v.FromString("-2714554767500194796127964596906605976605873665859580600161998556511368530960400907199253450604168622770350228527124626728538626805418833470107651091641919900725415994689920112219170907023561354484047025713");
	BN_Div(u, v, &remainder, q);
	_LOG(q);
	_LOG(remainder);
	
    //early out test case
    u.FromString("73465160877754457984611100105");
    v.FromString("0");
	BN_Div(u, v, &remainder, q);
    _LOG(q);
    _LOG(remainder);

    //early out test case
    u.FromString("0");
    v.FromString("73465160877754457984611100105");
	BN_Div(u, v, &remainder, q);
    _LOG(q);
    _LOG(remainder);
	
	//correct answer: 8,576,756,358.81833278971357075972
	//8576756358
	//131223141516161
	BigNumMutable k;
	u.FromString("1375319341434165787331471");
	v.FromString("160354250942445");
	k.Div(u,v,&remainder);
	_LOG(k);
	_LOG(remainder);

	
	{
		Float256 a("1.23456789987654321123456789987654321123456789987654321");
		a.Pow(100U);
		_LOG(a);
	}

	{
		Float2048 a("1.23456789987654321123456789987654321123456789987654321");
		a.Pow(100U);
		_LOG(a);
	}

	{
		Float1024 a("123456789987654321123456789987654321123456789987654321");
		Float1024 b(a);
		b.Add(a);
		_LOG("Float2048     = " << b);
	}

	_LOG("\nString");
	BigNumMutable a;
	a = (ULONGLONG)0x1234567890abcdefULL;
	_LOG(a);
	
	a = (LONGLONG)-0x567890abcdefLL;
	_LOG(a);

	a.SetZero();
	_LOG(a);

	a.FromString("10000000000000000000000000");
	_LOG(a);

	a.FromString("0x8111222333444555666777888999000aaabbbcccdddeeefff");
	_LOG(a);

	rt::String out;
	{	_LOG("\nGet Mantissa");
		a <<= 1;

		out.Empty();
		a.ToString(out, 16);
		_LOG(out);
		BigNumMutable b;
		int exp;

		UINT m = ext::_details::BN_Mantissa32(a, &exp);
		b = m;	b <<= exp;	out.Empty(); b.ToString(out, 16);	_LOG(rt::tos::HexNum<>(m)<<"*2^"<<exp<<" =\n"<<out);

		ULONGLONG mm = ext::_details::BN_Mantissa64(a, &exp);
		b = mm;	b <<= exp;	out.Empty(); b.ToString(out, 16);	_LOG(rt::tos::HexNum<>(mm)<<"*2^"<<exp<<" =\n"<<out);
		a >>= 1;
	}

	out.Empty();
	a.ToString(out, 10);

	_LOG("\nSimple Math");
	BigNumMutable b,c,d;

	b.FromString(out);
	_LOG(b);

	b.Add(a, 1);
	_LOG(b);
	a.Sub(b, 1);
	_LOG(a<<" Check:"<<(a == b));

	b.Add(a,a);
	_LOG(b);
	c.Sub(b, a);
	_LOG(c<<" Check:"<<(a == c));
	a *= 2;
	_LOG(a);

	b.Mul(a, 2);
	_LOG(b);
	c.Sub(b, a);
	_LOG(c<<" Check:"<<(a == c));

	_LOG("\nBit Ops");
	c >>= 40;
	_LOG(c);

	c <<= 40;
	_LOG(c);

	c >>= 103;
	_LOG(c);

	c <<= 103;
	_LOG(c);

	BigNumMutable e;
	e.CopyLowBits(79, a);
	_LOG(e<<" <- "<<a);
	e.SetToPowerOfTwo(79);
	_LOG(e);
	e.CopyLowBits(64, a);
	_LOG(e<<" <- "<<a);
	e.SetToPowerOfTwo(64);
	_LOG(e);

	_LOG("\nFloat");
	_LOG(b);
	c.Mul(b, 0.25f);
	_LOG(c);
	c.Mul(b, 0.25);
	_LOG(c);
	a.Mul(c, 4);
	_LOG(a);

	_LOG("\nInplace");
	b.Mul(a, 2);
	_LOG(b);
	a += a;
	_LOG(a<<" Check:"<<(a == b));
	a *= 2;
	_LOG(a);
	a *= -0.5;
	_LOG(a);

	a *= -1;
	b *= 0.5;
	b -= a;
	_LOG(b);
	b *= 0.5;
	_LOG(b);
	b -= a;
	_LOG(b);

	UINT reminder;
	b.Div(a, 16, &reminder);
	_LOG(b<<" R="<<rt::tos::Base16OnStack<>(reminder));

	_LOG("\nMultiplication");
	b.Mul(a, (ULONGLONG)0x123456789abcdULL);
	_LOG(b);
	d.Mul(a, BigNumMutable((ULONGLONG)0x123456789abcdULL));
	_LOG(d<<" Check:"<<(b == d));

	b.Mul(a, a);
	_LOG('('<<a<<")^2\n = "<<b);

	_LOG("\nDivision");
	_LOG(a);
	c.DivRough(b, a);
	d.Sub(a,c);
	_LOG(c<<" ERR/Rough="<<d);
	c.DivFast(b, a);
	d.Sub(a,c);
	_LOG(c<<" ERR/Fast ="<<d);

	_LOG("\nTo floating point");
	b = 3.14156789e30f;
	_LOG(b);
	_LOG(b.ToFloat());

	b = -3.14156789e20f;
	_LOG(b);
	_LOG(b.ToFloat());

	b = 3.14156789e10f;
	_LOG(b);
	_LOG(b.ToFloat());

	b = 3.14156789e70;
	_LOG(b);
	_LOG(b.ToDouble());

	b = -3.14156789e40;
	_LOG(b);
	_LOG(b.ToDouble());

	b = 3.14156789e25;
	_LOG(b);
	_LOG(b.ToDouble());

	b = -3.14156789e10;
	_LOG(b);
	_LOG(b.ToDouble());

	b = (ULONGLONG)0x0fedcba987654321ULL;
	b <<= 4;
	_LOG(b);
	b <<= 1;
	_LOG(b);
	b >>= 1;
	_LOG(b);
	b <<= 234;
	_LOG(b);
}




void performanceTest()
{
	LPCSTR fn = "test.db";
	{
		struct cmp : public ::rocksdb::Comparator
		{
			// Three-way comparison.  Returns value:
			//   < 0 iff "a" < "b",
			//   == 0 iff "a" == "b",
			//   > 0 iff "a" > "b"
			virtual int Compare(const ::rocksdb::Slice& a, const ::rocksdb::Slice& b) const override
			{
				auto x = *(UINT*)a.data();
				auto y = *(UINT*)b.data();
				if (x > y)return -1;
				else if (x < y)return +1;
				return 0;
			}

			virtual bool Equal(const ::rocksdb::Slice& a, const ::rocksdb::Slice& b) const override
			{
				return  *(UINT*)a.data() == *(UINT*)b.data();
			}

			virtual const char* Name() const { return "test_cmp"; }
			virtual void FindShortestSeparator(std::string* start, const ::rocksdb::Slice& limit) const {}
			virtual void FindShortSuccessor(std::string* key) const {}
		};

		os::SetProcessPriority(os::PROCPRIO_REALTIME);

		rt::Buffer<ULONGLONG>	keys;
		keys.SetSize(100 * 1024);
		{
			cmp	_cmp;
			::rocksdb::ColumnFamilyOptions opt;
			opt.comparator = &_cmp;

			::rocksdb::ColumnFamilyOptions opt3;

			auto test = [&keys, fn, &opt, &opt3](ext::RocksStorageWriteRobustness wr, const ext::WriteOptions* w_opt)
			{
				os::HighPerformanceCounter hpc;
				hpc.SetOutputUnit(1000U);

				{
					ext::RocksStorage::Nuke(fn);
					ext::RocksStorage store;

					for (UINT i = 0; i < keys.GetSize(); i++)
						keys[i] = i;

					store.SetDBOpenOption("q", opt);
					store.Open(fn, wr);
					auto db = store.Get("q");

					hpc.LoadCurrentCount();
					for (UINT i = 0; i < keys.GetSize(); i++)
						db.Set(keys[keys.GetSize() - i - 1], keys[i], w_opt);
					ULONGLONG insert = keys.GetSize() * 1000000LL / rt::max(1LL, hpc.TimeLapse());

					hpc.LoadCurrentCount();
					for (UINT i = 0; i < 10; i++)
					{
						auto it = db.First();
						while (it.IsValid())it.Next();
					}
					ULONGLONG scan = keys.GetSize() * 10000000LL / rt::max(1LL, hpc.TimeLapse());

					hpc.LoadCurrentCount();
					for (UINT i = 0; i < 10; i++)
					{
						auto it = db.Last();
						while (it.IsValid())it.Prev();
					}
					_LOG("Desc+Compr: " << insert << '\t' << scan << '\t' << keys.GetSize() * 10000000LL / rt::max(1LL, hpc.TimeLapse()));
				}

				{
					int count = 0;
					ext::RocksStorage::Nuke(fn);
					ext::RocksStorage store;

					for (UINT i = 0; i < keys.GetSize(); i++)
						keys[i] = i;

					store.SetDBOpenOption("q", opt3);
					store.Open(fn, wr);
					auto db = store.Get("q");

					hpc.LoadCurrentCount();
					for (UINT i = 0; i < keys.GetSize(); i++)
					{
						ext::ToRightByteOrder<false, ULONGLONG> test(keys[keys.GetSize() - i - 1]);
						::rocksdb::Slice x((LPCSTR)&test, sizeof(test));
						db.Set(x, keys[i], w_opt);
					}
					ULONGLONG insert = keys.GetSize() * 1000000LL / rt::max(1LL, hpc.TimeLapse());

					hpc.LoadCurrentCount();
					for (UINT i = 0; i < 10; i++)
					{
						auto it = db.First();
						while (it.IsValid()) { it.Next(); count++; }
					}
					ULONGLONG scan = keys.GetSize() * 10000000LL / rt::max(1LL, hpc.TimeLapse());
					count = 0;
					hpc.LoadCurrentCount();
					for (UINT i = 0; i < 10; i++)
					{
						auto it = db.Last();
						while (it.IsValid()) { it.Prev(); count++; }
					}
					_LOG("ToRightByteOrder: " << insert << '\t' << scan << '\t' << keys.GetSize() * 10000000LL / rt::max(1LL, hpc.TimeLapse()) << '\t' << count);
				}

			};


			_LOG_HIGHLIGHT("ROCKSSTG_DEFAULT - WriteOptionsDefault");
			test(ext::ROCKSSTG_DEFAULT, ext::RocksDB::WriteOptionsDefault);
		}
	}
}

void accurucyTest()
{

	LPCSTR fn = "test.db";

	{
		ext::RocksStorage::Nuke(fn);
#pragma pack(push, 1)
		struct TxnMetadata
		{
			WORD	ExtraDataSize;
		};
#pragma pack(pop)
		typedef ext::RocksDBStandalonePaged<UINT, TxnMetadata, 1024>	t_PagedTxnDB;

		t_PagedTxnDB db;
		db.Open(fn);
		db.SetPaged(100, { 10 }, "123", 3);

		std::string ws;
		auto* b = db.GetPaged(100, 0, ws);
		_LOG(b->TotalSize << ", " << b->ExtraDataSize);
	}

	{
		ext::RocksStorage::Nuke(fn);
		ext::RocksStorage store;

		store.SetDBOpenOption("t2", ext::RocksDBOpenOption().SetKeyOrder<INT>());

		store.Open(fn);

		ext::RocksDB db = store.Get("t");
		ext::RocksDB db2 = store.Get("t2");
		ext::RocksDB db3 = store.Get("t3");
		using structType = ext::ToRightByteOrder<true, int, int>;
		for (INT i = -100; i < 100; i++)
		{
			db.Set(i * 100, i * 100);
			db2.Set(i * 100, i * 100 + 1);
			structType test(i * 100, -i * 100);
			if (i == 99)
			{
				ext::setRightByteOrderElement<1>(99900, test);
			}
			::rocksdb::Slice x((LPCSTR)&test, sizeof(test));
			db3.Set(x, i * 100);
		}

		{
			rt::String str;
			auto it = db.First();
			while (it.IsValid()) { str += rt::tos::Number(it.Key<INT>()) + ' '; it.Next(); }
			_LOG(str);
			_LOG("");

			str.Empty();
			it = db2.First();
			while (it.IsValid()) { str += rt::tos::Number(it.Key<INT>()) + ' '; it.Next(); }
			_LOG(str);
			_LOG("");
			str.Empty();
			it = db3.First();
			while (it.IsValid())
			{
				auto t = it.Key<structType>();
				str += rt::tos::Number(ext::getRightByteOrderElement<0>(t)) + '(' + rt::tos::Number(ext::getRightByteOrderElement<1>(t)) + ')' + ' ';
				it.Next();
			}
			_LOG(str);
		}
		store.Close();
	}
}
void rt::UnitTests::rocks_db_ByteOrder()
{
	accurucyTest();
	performanceTest();
}