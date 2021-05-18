#include "../../core/rt/string_type_ops.h"
#pragma warning(disable: 4838)
#include <string>
#include "test.h"

namespace rt
{
	enum TestEnum
	{
		TE_X = 0,
		TE_Y,
		TE_MAX,
		TE_Z,
	};

	enum TestEnumBitwise
	{
		TEB_A = 0x1,
		TEB_B = 0x2,
		TEB_C = 0x4,
		TEB_COM = TEB_B | TEB_C
	};
}

STRINGIFY_ENUM_BEGIN(TestEnum, rt)
STRINGIFY_ENUM(TE_X)
STRINGIFY_ENUM(TE_Y)
STRINGIFY_ENUM(TE_MAX)
STRINGIFY_ENUM_END(TestEnum, rt)

STRINGIFY_ENUM_BEGIN(TestEnumBitwise, rt)
STRINGIFY_ENUM_BIT(TEB_COM)
STRINGIFY_ENUM_BIT(TEB_A)
STRINGIFY_ENUM_BIT(TEB_B)
STRINGIFY_ENUM_BIT(TEB_C)
STRINGIFY_ENUM_END(TestEnumBitwise, rt)

void rt::UnitTests::string_conv()
{
	double a = 0;

	_LOG(rt::tos::Number((BYTE)12));
	_LOG(rt::tos::Number((SHORT)12));
	_LOG(rt::tos::Number(123));
	_LOG(rt::tos::Number((LONGLONG)123LL));
	_LOG(rt::tos::Number(123.450000001f));
	_LOG(rt::tos::Number(123.450000001));
	_LOG(rt::tos::Number(1.23e45));
	_LOG(rt::tos::Number(1.23e45));
	_LOG(rt::tos::Number(1.0 / a));

	_LOG(TE_X);
	_LOG(TE_Z); // will log 3 
	_LOG(TEB_COM);

	_LOG((TEB_A | TEB_B | TEB_C));
	_LOG((TEB_B | TEB_A | TEB_C));
	_LOG((TEB_C | TEB_B | TEB_A));

	TestEnumBitwise val;
	rt::EnumParse("TEB_B|TEB_C", val);			    _LOG(val);
	rt::EnumParse("TEB_A|TEB_B|TEB_C", val);	    _LOG(val);

	TestEnum eval;
	rt::EnumParse("TE_MAX", eval);					_LOG(eval);
}

void String_RegularizeUTF8()
{
	rt::String s = "123\xF0\x9F\x93\xA2\xe5\xa5\xbd\xe7\x9a\x84 456";
	_LOG(s); 
	s.RegularizeUTF8();
	_LOG(s);
}

void StringRef_basic()
{
	std::string a("1234!!");
	rt::String_Ref b;
	b = a;
	_LOG("std::string = " << b);
}
void StringBase_GetUrlParam()
{
	rt::SS url("https://aaa.bb.ccc:99/?a=232=11&b=&c=val_c");
	_LOG("a=" << url.GetUrlParam("a"));
	_LOG("b=" << url.GetUrlParam("b"));
	_LOG("c=" << url.GetUrlParam("c"));
}
void String_TrimCodeComments()
{
	LPCSTR code =
		"int i=110; i/=2/*34*/;\r\n"
		"_LOG(\"/*some comment*/\");\r\n"
		"//i += 2342;\r\n"
		"i += 32;\r\n"
		"i++; //haha\r\n"
		"_LOG(\"//some conmment\");\r\n"
		"//\n"
		"i+=222;"
		;

	rt::String trim_comments;
	trim_comments.TrimCodeComments(code);
	_LOG(trim_comments);
}
void String_EscapeCharacters()
{
	rt::String	org = "--\'\"--\\\\*\\9";
	rt::String	str;
	str.EscapeCharacters(org, "\'\"");
	_LOG(str);
	org.EscapeCharacters("\'\"");
	_LOG(org);
	str.UnescapeCharacters();
	_LOG(str);
	str.UnescapeCharacters(org);
	_LOG(str);

	rt::String d =
		"<div class=\"tabBody3 tabContent\"><ul><li>"
		"<a id=\"viewItemsMeasurements\" title=\"View this items measurements\" href=\"/am/pssizechart.nap?productID=511413\">View this item's measurements</a><br>"
		"</li></ul><span><ul><li> Fits true to size, take your normal size</li><li> Cut for a loose fit</li><li> Mid-weight, slightly stretchy fabric</li><li>"
		"Model is 177cm/ 5'10\" and is wearing a FR size 36 </li></ul></span></div>";

	d.EscapeCharacters("\'\"", '\\');
	_LOG(d);
	d.UnescapeCharacters("\'\"", '\\');
	_LOG(d);
}
void Stringbase_ResolveRelativePath()
{
	rt::String str;
	str = "/123/456///7/.//./../89/";
	str.ResolveRelativePath();
	_LOG(str);
	str = "/123/456///7/.//./../89";
	str.ResolveRelativePath();
	_LOG(str);
}
void Stringbase_SplitURL()
{
	rt::String str;
	rt::String_Ref p, h, path;

	str = "ftp://www.xxx.com/124/rty.jpg";
	str.SplitURL(p, h, path);
	_LOG("P:" << p << " , H:" << h << " , PATH:" << path);

	str = "124/rty.jpg";
	str.SplitURL(p, h, path);
	_LOG("P:" << p << " , H:" << h << " , PATH:" << path);

	str = "rty.jpg";
	str.SplitURL(p, h, path);
	_LOG("P:" << p << " , H:" << h << " , PATH:" << path);

	str = "/124/rty.jpg";
	str.SplitURL(p, h, path);
	_LOG("P:" << p << " , H:" << h << " , PATH:" << path);

	str = "http://www.google.com";
	str.SplitURL(p, h, path);
	_LOG("P:" << p << " , H:" << h << " , PATH:" << path);

	str = "www.xxx.com/124/rty.jpg";
	str.SplitURL(p, h, path);
	_LOG("P:" << p << " , H:" << h << " , PATH:" << path);
}
void String_Insert()
{
	rt::String s = "123456";
	s.Insert(0, '!');
	s.Insert(7, '~');
	s.Insert(1, '?');
	_LOG(s);
	s.Insert(0, "[-");
	s.Insert(s.GetLength(), "-]");
	s.Insert(4, "****");
	_LOG(s);
	_LOG(s[0]);
}
void Stringbase_Token()
{
	rt::String_Ref token;
	rt::CharacterSet_Symbol symbol;

	rt::SS code("c->a*trim_comments.TrimCodeComments(code);80");
	while (code.GetNextToken(symbol, token))
	{
		_LOG(token);
	}

	_LOG(code.TrimAfter('*'));
	_LOG(code.TrimBefore('.'));

	_LOG(code.TrimAfterReverse('*'));
	_LOG(code.TrimBeforeReverse('.'));
	token.Empty();
	rt::String_Ref nontoken;
	while (code.GetNextToken(symbol, token, nontoken))
	{
		_LOG(token << '\t' << nontoken);
	}
}
void Stringbase_GetNextLine()
{
	rt::SS code("1\n\r\n2\n\n\r\n\n");
	rt::String_Ref line;
	_LOG("GetNextLine(line, true)");
	while (code.GetNextLine(line, true))
	{
		_LOG('"' << line << '"');
	}
	line.Empty();
	_LOG("GetNextLine(line, false)");
	while (code.GetNextLine(line, false))
	{
		_LOG('"' << line << '"');
	}
}
void String_Basic()
{
	rt::String a;
	//a.Empty();
	a += "fdasf";
}
void Stringbase_IsSimilarTo()
{
	_LOG(rt::SS("adress").IsSimilarTo(rt::SS("address")));
	_LOG(rt::SS("adresss").IsSimilarTo(rt::SS("address")));
	_LOG(rt::SS("addresss").IsSimilarTo(rt::SS("address")));
}
void _something()
{
	//	if(0)
	//	{	os::FileBuffer<char>	file;
	//		file.Open("list.txt"); // http://theiphonewiki.com/wiki/Models
	//		rt::String whole = rt::String_Ref(file, file.GetSize());
	//
	//		whole.Replace("8 GB", "8GB");
	//		whole.Replace("16 GB", "16GB");
	//		whole.Replace("32 GB", "32GB");
	//		whole.Replace("64 GB", "64GB");
	//		whole.Replace("128 GB", "128GB");
	//		whole.Replace("Space Gray", "SpaceGray");
	//		whole.Replace("Black & Silver", "Black&Silver");
	//
	//		whole.Replace("iPad ", "iPad_");
	//		whole.Replace("iPad mini ", "iPad_Mini_");
	//		whole.Replace("iPhone ", "iPhone_");
	//		whole.Replace("iPod touch ", "iPod_touch_");
	//
	//		whole.Replace('\t',' ');
	//		whole.Replace('\r',' ');
	//		whole.Replace('\n',' ');
	//
	//		static const LPCSTR color[] = {
	//		"Silver",
	//		"Black",
	//		"White",
	//		"Gold",
	//		"SpaceGray",
	//		"Green",
	//		"Blue",
	//		"Yellow",
	//		"Pink",
	//		"Red",
	//		"Black&Silver"
	//		};
	//
	//		static const LPCSTR size[] = 
	//		{
	//			"8GB",
	//			"16GB",
	//			"32GB",
	//			"64GB",
	//			"128GB"
	//		};
	//
	//		static const LPCSTR ipad_model[] = 
	//		{
	//			"iPad_1G",
	//			"iPad_2",
	//			"iPad_3",
	//			"iPad_4",
	//			"iPad_Air",
	//			"iPad_mini_1G",
	//			"iPad_mini_2G"
	//		};
	//
	//		static const LPCSTR iphone_model[] = 
	//		{
	//			"iPhone_2G",		
	//			"iPhone_3G", 
	//			"iPhone_3GS", 
	//			"iPhone_4",
	//			"iPhone_4S",
	//			"iPhone_5",
	//			"iPhone_5c",
	//			"iPhone_5s"
	//		};
	//
	//		static const LPCSTR ipod_model[] = 
	//		{
	//			"iPod_touch_1G",		
	//			"iPod_touch_2G", 
	//			"iPod_touch_3G", 
	//			"iPod_touch_4G",
	//			"iPod_touch_5G"
	//		};
	//
	//		rt::Buffer<rt::String_Ref>	words;
	//		words.SetSize(whole.GetLength()/3);
	//
	//		UINT co = whole.Split(words.Begin(), words.GetSize(), ' ');
	//
	//		int cur_color = 0;
	//		int cur_size = 0;
	//		int cur_model = 0;
	//
	//		for(UINT i=0;i<co;i++)
	//		{
	//			const rt::String_Ref& w = words[i];
	//			if(	w.GetLength() >= 5 &&
	//				w[0] >= 'A' && w[0] <= 'Z' &&
	//				w[1] >= 'A' && w[1] <= 'Z' &&
	//				w[2] >= '0' && w[2] <= '9' &&
	//				w[3] >= '0' && w[3] <= '9' &&
	//				w[4] >= '0' && w[4] <= '9'
	//			)
	//			{	int num;
	//				w.SubStr(2,3).ToNumber(num);
	//				_LOG(" { "<<num<<", "<<cur_color<<", "<<cur_size<<", "<<cur_model<<" },");
	//				continue;
	//			}
	//			else
	//			{
	//				for(UINT i=0;i<sizeofArray(color);i++)
	//					if(w.TrimSpace() == color[i])
	//					{	cur_color = i+1;
	//						goto NEXT_WORD;
	//					}
	//
	//				for(UINT i=0;i<sizeofArray(size);i++)
	//					if(w.TrimSpace() == size[i])
	//					{	cur_size = i+1;
	//						goto NEXT_WORD;
	//					}
	//
	//				for(UINT i=0;i<sizeofArray(ipad_model);i++)
	//					if(w.TrimSpace() == ipad_model[i])
	//					{	cur_model = i+0x300;
	//						goto NEXT_WORD;
	//					}
	//				for(UINT i=0;i<sizeofArray(iphone_model);i++)
	//					if(w.TrimSpace() == iphone_model[i])
	//					{	cur_model = i+0x100;
	//						goto NEXT_WORD;
	//					}
	//				for(UINT i=0;i<sizeofArray(ipod_model);i++)
	//					if(w.TrimSpace() == ipod_model[i])
	//					{	cur_model = i+0x200;
	//						goto NEXT_WORD;
	//					}
	//			}
	//			
	//NEXT_WORD:
	//			continue;
	//		}
	//
	//		return ;
	//	}
}
void Stringbase_SubStr()
{
	rt::String_Ref a = "abcde";
	_LOG(a.SubStr(0, 3)); // abc
	_LOG(a.SubStr(1, 3)); // bcd
	_LOG(a.SubStr(2, 3)); // cde
	_LOG(a.SubStr(3, 3)); // de
	_LOG(a.SubStr(0, 6)); // abcde
	_LOG(a.SubStr(6, 6)); // 
	_LOG(a.SubStr(3, 0)); // 
	_LOGNL;
	_LOG(a.SubStr(0)); // abcde
	_LOG(a.SubStr(2)); // cde
	_LOG(a.SubStr(4)); // e
	_LOG(a.SubStr(6)); // 
	_LOGNL;
	_LOG(a.SubStrHead(0)); // 
	_LOG(a.SubStrHead(2)); // ab
	_LOG(a.SubStrHead(5)); // abcde
	_LOG(a.SubStrHead(7)); // abcde
	_LOGNL;
	_LOG(a.SubStrTail(0)); // 
	_LOG(a.SubStrTail(2)); // de
	_LOG(a.SubStrTail(5)); // abcde
	_LOG(a.SubStrTail(7)); // abcde
	_LOGNL;
}
void Stringbase_TrimLeftSpace()
{
	rt::String t(" abcd");
	_LOG(t.TrimLeftSpace());
	t = t.TrimLeftSpace();
	_LOG(t);
}
void Stringbase_ToNumber()
{
	rt::String_Ref a("hello world");
	_LOG(a);
	_LOG("a[2]: " << a[2]);

	a = "123456";
	int num;
	a.ToNumber(num);
	_LOG("ToNumber: " << num);

	a = "1024";
	a.ToNumber<int, 2>(num);
	_LOG("ToNumber: " << num);

	a.TrimLeft(2).ToNumber(num);	_LOG("TrimLeft(2).ToNumber(num): " << num);
	a.SubStrHead(4).ToNumber(num);	_LOG("SubStrHead(4).ToNumber(num): " << num);
}
void Stringbase_Split()
{
	rt::String_Ref a("hello world");

	a = __STRING(123, 456, 789, 123$ddd 'yes "\'no');
	rt::String_Ref f[8];

	rt::Zero(f);
	a.Split(f, 8, ',');
	_LOG("Split<>(','): " << f[0] << ',' << f[1] << ',' << f[2] << ',' << f[3] << ',' << f[4] << ',' << f[5] << ',' << f[6] << ',' << f[7]);

	rt::Zero(f);
	a.Split(f, 8, rt::SS(" ,"));
	_LOG("Split<>(\" ,\"): " << f[0] << ',' << f[1] << ',' << f[2] << ',' << f[3] << ',' << f[4] << ',' << f[5] << ',' << f[6] << ',' << f[7]);

	rt::Zero(f);
	a.Split<true>(f, 8, rt::SS(" ,$"));
	_LOG("Split<true>(\" ,$\"): " << f[0] << ',' << f[1] << ',' << f[2] << ',' << f[3] << ',' << f[4] << ',' << f[5] << ',' << f[6] << ',' << f[7]);
		
	rt::SS("345;\"453;546\";4543;\"444\"").Split(f, 8, ';');
	_LOG("Split<>(...): " << f[0] << ',' << f[1] << ',' << f[2] << ',' << f[3] << ',' << f[4] << ',' << f[5] << ',' << f[6] << ',' << f[7]);
}
void String_Replace()
{
	_LOG((LPCSTR)(rt::String_Ref("hello world") + ' ' + 1233 + rt::String_Ref() + rt::String(" !!")));
	rt::String a = (LPCSTR)(rt::String_Ref("hello world") + ' ' + 1233);
	a = a + " !!";
	a += " ..";
	_LOG("String Expression 1: " << a);

	a = rt::String_Ref("hello worlld").TrimLeft(6) + " !!";
	_LOG("String Expression 2: " << a);

	a.Replace("ll", "===");
	_LOG("Replace: " << a);

	bool is = a.StartsWith("wo");
	_LOG("IsPrefixOf: " << is);
}
void BinaryCString_basic()
{
	_LOG("BinaryCString: " << rt::tos::BinaryCString<>("123456", 6));
	_LOG("Binary: " << rt::tos::Binary<>("123456", 6));
}
void StringRef_HashCompare()
{
	rt::hash_map<rt::String, int, rt::String::hash_compare>	map;
	map["123"] = 456;
	_LOG("map[\"123\"]: " << map["123"]);

	char buf[100];
	int  len = (int)(rt::String_Ref("123", 3) + rt::String_Ref("\x0\x1\x3", 3)).CopyTo(buf);
	_LOG("Length of \"123\" + \"\\x0\\x1\\x3\" is " << len);
}
void String_TOS()
{
	{ rt::tos::FileSize<false> t(41513);		_LOG("FileSize<false,false>: " << t); }
	{ rt::tos::FileSize<true> t(4151351);		_LOG("FileSize<false,true>: " << t); }
	{ rt::tos::FileSize<false> t(41513510000);	_LOG("FileSize<false,false>: " << t); }
	{ rt::tos::FileSize<true> t(4151351000000000); _LOG("FileSize<false,true>: " << t); }

	{ rt::tos::TimeSpan<true>	t(10);			_LOG("TimeSpan<true>: " << t); }
	{ rt::tos::TimeSpan<true>	t(10000);		_LOG("TimeSpan<true>: " << t); }
	{ rt::tos::TimeSpan<true>	t(100000000);	_LOG("TimeSpan<true>: " << t); }
	{ rt::tos::TimeSpan<true>	t((int)(10000000000U - 5000));	_LOG("TimeSpan<true>: " << t); }
	{ rt::tos::TimeSpan<false>	t((int)(10000000000U - 5000));	_LOG("TimeSpan<false>: " << t); }
}
void rt::UnitTests::string()
{
	String_RegularizeUTF8();
	StringRef_basic();
	StringBase_GetUrlParam();
	String_TrimCodeComments();
	String_EscapeCharacters();
	Stringbase_ResolveRelativePath();
	Stringbase_SplitURL();
	String_Insert();
	Stringbase_Token();
	Stringbase_GetNextLine();
	String_Basic();
	Stringbase_IsSimilarTo();
	_something();
	Stringbase_SubStr();
	Stringbase_TrimLeftSpace();
	Stringbase_ToNumber();
	Stringbase_Split();
	String_Replace();
	BinaryCString_basic();
	StringRef_HashCompare();
	String_TOS();
}