#include "../../core/rt/string_type_ops.h"
#include "../../core/rt/buffer_type.h"
#include "../../core/rt/small_math.h"
#include "../../core/rt/xml_xhtml.h"
#include "../../core/rt/json.h"
#include "../../core/os/kernel.h"
#include "../../core/os/multi_thread.h"
#include "../../core/os/file_dir.h"
#include "../../core/os/file_zip.h"
#include "../../core/os/high_level.h"
#include "../../core/os/precompiler.h"
#include "../../core/inet/inet.h"
#include <string>
#include <algorithm>

#pragma warning(disable: 4838)

#include "test.h"

class _JArray;
void rt::UnitTests::json()
{
	{
		rt::JsonObject json = rt::SS("{\"aa:aa\": 3}");
		_LOG(json.GetValueAs<int>("aa:aa"));
		rt::JsonKeyValuePair kv;
		json.GetNextKeyValuePair(kv);
		_LOG(kv.GetKey()<<" = "<<kv.GetValue());
	}

	{
		rt::String str =
		(
			J_IF(false, J(cond_false) = "no"),
			J(abs) = 1.3,
			J_IF(1>0, J(cond_true) =
				(	
					J_IF(2>1, J(cond_true_nested) = JA(1,2,"eee")),
					J(qwe) = "nested",
					J_IF(2<1, J(cond_false_nested) = 1),
					J_IF(2<1, J(cond_false_nested) = 1),
					J(yue) = 3.1f,
					J_IF(2<1, J(cond_false_nested) = 1)
				)
			),
			J(empty) = (
					J_IF(2<1, J(cond_false_nested) = 2),
					J_IF(2<1, J(cond_false_nested) = 2)
				),
			J(fix) = (
					J(H)=0,
					J_IF(false, J(R) = 1),
					J_IF(true, J(R) = 2)
				),
			J_IF(true, J(cond_true) = 1234),
			J_IF(false, J(cond_false) = "no")
		);

		_LOG(JsonBeautified(str));
	}

	rt::_JArray<> a;
	a.Append((J(name) = "jake", J(age) = 12));
	a.Append((J(name) = "mary", J(age) = 7));
	a.Append(1);
	a.Append(false);
	a.Append("haha");
	a.Append('@');

	_LOG(JsonBeautified(rt::String(a)));

	std::string std_string("std::string");

	auto JsonObject = 
	(	J(Key:Complex) = "complex\\\" key",
		J(name) = "this is name", 
		J(sex) = "a female",
		J(empty) = rt::String_Ref(),
		J(alive) = true,
		J(age) = 20,
		J(char) = 'C',
		J(phone) =	(	J(number) = 5235251,
						J(ext) = 432
					),
		J(weight) = rt::String_Ref() + 123.5f + "kg",
		J(state) =	(	J(paid) = false,
						J(lastshow) = rt::tos::Date<>(os::Date32(2007,3,2))
					),
		J(children) = a,
		J(numbers) =	JA(	1,
							2,
							"hello",
							(J(c) = 1.2),
							JA(3,'c',false),
							JB("InArray"),
							4.5f
						),
		J(other) =	JA(	(J(f0) = 0),
						(J(f1) = 1)
					),
		J(std) = std_string,
		J(bin) = JB("1234567890ABCDEFGHIJK"),
		J(raw) = rt::_JVal(""),
		J(empty) = JA()
	);

	char buf[1024];
	int  len = JsonObject.CopyTo(buf);
	_LOG("Json Size: "<<len<<" = "<<JsonObject.GetLength());

	_LOG(JsonBeautified(rt::String_Ref(buf, len)));

	rt::JsonObject obj(rt::String_Ref(buf, len));
	_LOG(obj.GetValue("Key:Complex"));
	_LOG(obj.GetValue("name"));
	_LOG(obj.GetValue("bin"));
	_LOG(obj.GetValue("empty"));
	_LOG(obj.GetValue("phone.number"));
	_LOG(obj.GetValue("numbers[2]"));
	_LOG(obj.GetValue("numbers[3].c"));
	_LOG(obj.GetValue("numbers[4][2]"));
	_LOG(obj.GetValue("numbers[4][1]"));
	_LOG(obj.GetValue("numbers[5]"));
	_LOG(obj.GetValue("numbers[6]"));

	static const LPCSTR json_type_name[] = 
	{
		"string",
		"number",
		"bool",
		"null",
		"object",
		"array",
		"binary",
		"corrupted"
	};

	rt::JsonKeyValuePair kv;
	while(obj.GetNextKeyValuePair(kv))
	{
		_LOG(kv.GetKey()<<" ["<<json_type_name[kv.GetValueType()]<<"] = "<<kv.GetValue());
	}

	rt::JsonArray arr = obj.GetValue("children");
	rt::JsonObject child;
	while(arr.GetNextObjectRaw(child))
	{
		_LOG(child.GetValue("name"));
	}

	{
		rt::JsonObject json(__STRING(
			{
				c:1,
				b:[2,3,4],
				a:5
			}
		));

		rt::JsonObject json_sub(__STRING(
			{
				d: "Yes",
				c: 8.9
			}
		));

		rt::JsonArray b = json.GetValue("b");
		rt::JsonObject child;
		while(b.GetNextObjectRaw(child))
		{
			int val;
			child.GetString().ToNumber(val);
			_LOG(val);
		};
		rt::String out;
		rt::JsonObject::Override(json, json_sub, out);
		rt::JsonObject doc(out);
		for (const char *s : { "a", "b", "c", "d" })
			_LOG(s << ": " << doc.GetValue(s));
	}

	{
		rt::SS a("\x0\t\\ABC\'\"123");
		rt::JsonEscapeString e(a);
		_LOG("Escaped String: "<<e);

		rt::JsonUnescapeString u(e);

		_LOG("Unescaped Matches: "<<(u == a));
	}

	{
		auto str = rt::SS("A Json: ") + false +
							(
								J(var) = 1.3
							);
		_LOG(rt::String(str));	
	}

	{
		auto str = rt::SS("A Json: ") + ' ' + 13 +
							(
								J(var) = 1.3
							) + " EOF";
		_LOG(ALLOCA_C_STRING(str));
	}

	{
		rt::String a;
		a = (
			J(a) = rt::tos::Base16OnStack<>(123456),
			J(b) = rt::String("123456"),
			J(c) = JA("String",rt::String_Ref(), (J(a) = 12.f), rt::tos::Base16OnStack<>(123456), rt::String("123456"), rt::tos::Base32CrockfordLowercaseOnStack<>(0ULL))
		);

		_LOG(rt::JsonBeautified(a));
	}

	{
		rt::Json json;
		json.Object() << (
			J(key) = 32.1f
		);

		{	auto u = json.ScopeMergingObject();
			json.Object() << (
				J(key1) = "merged"
			);
		}

		{	json << (
				J(key2) = "merged_scope"
			);
		}

		{	auto scope = json.ScopeAppendingKey("array");
			json.Array();
			json << false << 1 << 2.1 << "ok" << (rt::SS("NN=") + 3);

			{	auto ele = json.ScopeAppendingElement();
				json.Object();
			}
		}

		json.ScopeWritingStringEscapedAtKey("str").String() += "1234";
		json.AppendKey("key3", (J(a) = false));
		json.AppendKey("key4", 13);
		json.AppendKey("key5", rt::SS("Hello ") + 1.0f);

		_LOG(rt::JsonBeautified(json));
	}

	{
		rt::String j_if;
		j_if = (
			J_IF(true, (J(a) = 1))
		);
		_LOG("J_IF: "<<rt::JsonBeautified(j_if));

		j_if = (
			J_IF(false, (J(a) = 2))
		);
		_LOG("J_IF: "<<rt::JsonBeautified(j_if));

		j_if = (
			J(b) = 3,
			J_IF(false, (J(a) = 2)),
			J(d) = 4
		);
		_LOG("J_IF: "<<rt::JsonBeautified(j_if));

		j_if = (
			J(b) = 3,
			J_IF(false, (J(a) = 2, J(c) = 3)),
			J(d) = 4,
			J_IF(true, (J(f) = 6, J(g) = 7)),
			J(e) = 5
		);
		_LOG("J_IF: "<<rt::JsonBeautified(j_if));
	}
}

void rt::UnitTests::xml()
{
	{	rt::String val;
		rt::XMLParser::_convert_xml_to_text("abcd & fff-&lt;-<cdnsion cnindi>!</v>??", val, true);
		_LOG(val);
	}

	rt::XMLComposer xml;
	xml.EnteringNode("root");
	xml.SetAttribute("name","testing");
	xml.SetAttribute("height",12.5f);
	xml.EnteringNodeDone();

	xml.EnteringNode("item");
	xml.SetAttribute("key","no");
	xml.EnteringNodeDone();
	xml.AddText("/*Some Inner Text*/");
	xml.ExitNode();

	xml.EnteringNode("item");
	xml.SetAttribute("key","<yes>");
	xml.SetAttribute("height",12);
	xml.EnteringNodeDone(true);

	xml.EnteringNode("item");
	xml.SetAttribute("key","no");
	xml.EnteringNodeDone();
	xml.AddText("Another Node");
	xml.ExitNode();

	xml.ExitNode();

	_LOG(xml.GetDocument());

	rt::XMLParser	xmlp;
	rt::String		val;
	xmlp.Load(xml.GetDocumentBuffer(), true, xml.GetDocumentLength());
	if(xmlp.EnterXPath("/root/item[@key='no']") && xmlp.GetInnerText(val))
		_LOG("EnterXPath: "<<val);

	rt::XMLParser node = xmlp.GetNodeDocument();
	node.GetAttribute("key",val);
	_LOG("key = "<<val);

	if(xmlp.EnterXPath("/root"))
	{
		xmlp.GetInnerText(val);
		_LOG("mixed inner text: "<<val);
	}

	xmlp.EnterRootNode();
	xmlp.TextDump();
}

void rt::UnitTests::html()
{
	os::FileRead<char>	file("category_page.htm");
	rt::XMLParser	html;

	if(html.LoadHTML(file, (UINT)file.GetSize()))
	{
		_LOG("Loaded");

		rt::String xml;
		html.EnterXPath("/html/head/meta");
		html.GetOuterXML(xml);

		_LOG(xml);

		os::File("xhtml.htm", os::File::Normal_Write).Write(html.GetConvertedXHTML());
	}
}

os::CriticalSection	test_multithread_ccs;
os::Event test_multithread_event;


void rt::UnitTests::multithread()
{
	{	DWORD a = 0x0010;
		DWORD org = os::AtomicOr(0x1000, &a);
		_LOG("AtomicOr: "<<rt::tos::HexNum<>(org)<<" -> "<<rt::tos::HexNum<>(a));
	}

	{
		volatile INT c = 0;
		bool ends = false;
		os::Thread a,b;
		auto th = [&c, &ends]()
		{	while(!ends)
			{	int x = os::AtomicIncrement(&c);
				_LOGC("c = "<<x<<" TH="<<os::Thread::GetCurrentId());
				os::Sleep(100);
			}
			_LOGC("Exit TH="<<os::Thread::GetCurrentId());
		};
		a.Create(th);
		b.Create(th);
		os::Sleep(2000);
		ends = true;
		a.WaitForEnding();
		b.WaitForEnding();
	}

	static const int LOOPCOUNT = 500000;

	struct _call_atom
	{	static DWORD _atom_inc(LPVOID pi)
		{	volatile int *p = (volatile int *)pi;
			for(int i=0;i<LOOPCOUNT;i++)
			{	os::AtomicIncrement(p);
			}
			return 0;
		}
		static DWORD _atom_dec(LPVOID pi)
		{	volatile int *p = (volatile int *)pi;
			for(int i=0;i<LOOPCOUNT;i++)
			{	os::AtomicDecrement(p);
			}
			return 0;
		}
		static DWORD _ccs_inc(LPVOID pi)
		{	volatile int *p = (volatile int *)pi;
			for(int i=0;i<LOOPCOUNT/10;i++)
			{	EnterCSBlock(test_multithread_ccs);
				(*p)++;
			}
			return 0;
		}
		static DWORD _ccs_dec(LPVOID pi)
		{	volatile int *p = (volatile int *)pi;
			for(int i=0;i<LOOPCOUNT/10;i++)
			{	EnterCSBlock(test_multithread_ccs);
				(*p)--;
			}
			return 0;
		}
		static DWORD _inc(LPVOID pi)
		{	volatile int *p = (volatile int *)pi;
			for(int i=0;i<LOOPCOUNT;i++)
			{	(*p)++;
			}
			return 0;
		}
		static DWORD _dec(LPVOID pi)
		{	volatile int *p = (volatile int *)pi;
			for(int i=0;i<LOOPCOUNT;i++)
			{	(*p)--;
			}
			return 0;
		}
	};

	double ops_atom, ops_ccs, ops_nosync;

	{	volatile int counter = 456789;
		os::Thread	thread_inc[4], thread_dec[4];
		for(int i=0;i<sizeofArray(thread_inc);i++)
		{	thread_inc[i].Create(_call_atom::_atom_inc, (LPVOID)&counter);
			thread_dec[i].Create(_call_atom::_atom_dec, (LPVOID)&counter);
		}
		for(int i=0;i<sizeofArray(thread_inc);i++)
		{	thread_inc[i].WaitForEnding();
			thread_dec[i].WaitForEnding();
		}
		_LOG("ATOM Result: "<<counter);

		os::TickCount t;
		t.LoadCurrentTick();
		for(int i=0;i<LOOPCOUNT;i++)
		{	os::AtomicIncrement(&counter);
			os::AtomicIncrement(&counter);
			os::AtomicIncrement(&counter);
			os::AtomicIncrement(&counter);
			os::AtomicIncrement(&counter);
		}
		int cost = t.TimeLapse();
		ops_atom = (5.0*LOOPCOUNT)/cost;
	}

	{	volatile int counter = 456789;
		os::Thread	thread_inc[4], thread_dec[4];
		for(int i=0;i<sizeofArray(thread_inc);i++)
		{	thread_inc[i].Create(_call_atom::_ccs_inc, (LPVOID)&counter);
			thread_dec[i].Create(_call_atom::_ccs_dec, (LPVOID)&counter);
		}
		for(int i=0;i<sizeofArray(thread_inc);i++)
		{	thread_inc[i].WaitForEnding();
			thread_dec[i].WaitForEnding();
		}
		_LOG("CriticalSection Result: "<<counter);
		
		os::TickCount t;
		t.LoadCurrentTick();
		for(int i=0;i<LOOPCOUNT;i++)
		{	
			{	EnterCSBlock(test_multithread_ccs);	counter++; }
			{	EnterCSBlock(test_multithread_ccs);	counter++; }
			{	EnterCSBlock(test_multithread_ccs);	counter++; }
			{	EnterCSBlock(test_multithread_ccs);	counter++; }
			{	EnterCSBlock(test_multithread_ccs);	counter++; }
		}
		int cost = t.TimeLapse();
		ops_ccs = (5.0*LOOPCOUNT)/cost;
	}
	{	volatile int counter = 456789;
		os::Thread	thread_inc[4], thread_dec[4];
		for(int i=0;i<sizeofArray(thread_inc);i++)
		{	thread_inc[i].Create(_call_atom::_inc, (LPVOID)&counter);
			thread_dec[i].Create(_call_atom::_dec, (LPVOID)&counter);
		}
		for(int i=0;i<sizeofArray(thread_inc);i++)
		{	thread_inc[i].WaitForEnding();
			thread_dec[i].WaitForEnding();
		}
		_LOGC("Async Result: "<<counter);
		_LOGC("sync is required: "<<(counter!=456789));

		os::TickCount t;
		t.LoadCurrentTick();
		for(int i=0;i<LOOPCOUNT;i++)
		{	counter++;
			counter++;
			counter++;
			counter++;
			counter++;
		}
		int cost = t.TimeLapse();
		ops_nosync = (5.0*LOOPCOUNT)/cost;
	}

	_LOGC("// CriticalSection:  "<<ops_ccs<<" kop/s");//<<rt::tos::Number((float)LOOPCOUNT/cost)<<" kop/s");
	_LOGC("// ATOM           : "<<ops_atom<<" kop/s");//rt::tos::Number((float)10*LOOPCOUNT/cost)<<" kop/s");
	_LOGC("// Nosync         : "<<ops_nosync<<" kop/s");//rt::tos::Number((float)10*LOOPCOUNT/cost)<<" kop/s");
	

	test_multithread_event.Reset();
	_LOG("Event Set: "<<test_multithread_event.IsSignaled());
	test_multithread_event.Set();
	_LOG("Event Set: "<<test_multithread_event.IsSignaled());

	struct _call_event
	{	static DWORD _wait_thread(LPVOID pi)
		{	
			os::TickCount t;
			t.LoadCurrentTick();
			test_multithread_event.WaitSignal(2000);
			_LOG("Event Timeout: "<<1000*((200 + t.TimeLapse())/1000));
			test_multithread_event.Reset();
			
			t.LoadCurrentTick();
			test_multithread_event.WaitSignal();
			test_multithread_event.WaitSignal();
			_LOG("Thread Waited: "<<1000*((200 + t.TimeLapse())/1000)<<", "<<test_multithread_event.IsSignaled());

			return 0;
		}
	};

	{	test_multithread_event.Reset();

		os::Thread	th;
		th.Create(_call_event::_wait_thread, nullptr);
		_LOG("Thread Run: "<<th.IsRunning());
		os::Sleep(4000);
		
		_LOG("Event Set");
		test_multithread_event.Set();

		th.WaitForEnding();
		_LOG("Thread Ended");
	}
}

void rt::UnitTests::binary_search()
{
	typedef WORD TT;

	rt::Buffer<TT>	a;
	a.SetSize(10);
	a.RandomBits(100);

	std::sort(a.Begin(),a.End());
	_LOG(a);

	_LOG(a[0]<<" is at "<<rt::LowerBound(a,a.GetSize(),a[0])<<"/"<<rt::BinarySearch(a,a.GetSize(),a[0]));
	_LOG(a[0]+10<<" is at "<<rt::LowerBound(a,a.GetSize(),a[0]+10)<<"/"<<rt::BinarySearch(a,a.GetSize(),a[0]+10));
	_LOG(a[0]-1<<" is at "<<rt::LowerBound(a,a.GetSize(),a[0]-1)<<"/"<<rt::BinarySearch(a,a.GetSize(),a[0]-1));

	_LOG(a[3]<<" is at "<<rt::LowerBound(a,a.GetSize(),a[3])<<"/"<<rt::BinarySearch(a,a.GetSize(),a[3]));
	_LOG(a[3]+1<<" is at "<<rt::LowerBound(a,a.GetSize(),a[3]+1)<<"/"<<rt::BinarySearch(a,a.GetSize(),a[3]+1));

	_LOG(a[a.GetSize()-1]<<" is at "<<rt::LowerBound(a,a.GetSize(),a[a.GetSize()-1])<<"/"<<rt::BinarySearch(a,a.GetSize(),a[a.GetSize()-1]));
	_LOG(a[a.GetSize()-1]+1<<" is at "<<rt::LowerBound(a,a.GetSize(),a[a.GetSize()-1]+1)<<"/"<<rt::BinarySearch(a,a.GetSize(),a[a.GetSize()-1]+1));
}

namespace rt
{
	enum TestEnum
	{
		TE_X = 0,
		TE_Y,
		TE_MAX,
	};

	enum TestEnumBitwise
	{
		TEB_A = 0x1,
		TEB_B = 0x2,
		TEB_C = 0x4,
		TEB_COM = TEB_B|TEB_C
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
	_LOG(rt::tos::Number(123LL));
	_LOG(rt::tos::Number(123.450000001f));
	_LOG(rt::tos::Number(123.450000001));
	_LOG(rt::tos::Number(1.23e45));
	_LOG(rt::tos::Number(1.23e45));
	_LOG(rt::tos::Number(1.0/a));

	_LOG(TE_X);
	_LOG(TEB_COM);

	_LOG((TEB_A|TEB_B|TEB_C));

	TestEnumBitwise val;
	rt::EnumParse("TEB_B|TEB_C", val);			    _LOG(val);
	rt::EnumParse("TEB_A|TEB_B|TEB_C", val);	    _LOG(val);
	
	TestEnum eval;
	rt::EnumParse("TE_MAX", eval);					_LOG(eval);
}


void rt::UnitTests::string()
{
	{	rt::String s	= "123\xF0\x9F\x93\xA2\xe5\xa5\xbd\xe7\x9a\x84 456";
		_LOG(s);
		s.RegularizeUTF8();
		_LOG(s);
	}

	{
		std::string a("1234!!");
		rt::String_Ref b;
		b = a;
		_LOG("std::string = "<<b);
	}

	{	rt::SS url("https://aaa.bb.ccc:99/?a=232=11&b=&c=val_c");
		_LOG("a="<<url.GetUrlParam("a"));
		_LOG("b="<<url.GetUrlParam("b"));
		_LOG("c="<<url.GetUrlParam("c"));
	}

	{	LPCSTR code = 
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

	{	rt::String	org = "--\'\"--\\\\*\\9";
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

		d.EscapeCharacters("\'\"",'\\');
		_LOG(d);
	}

	{	rt::String str;
		str = "/123/456///7/.//./../89/";
		str.ResolveRelativePath();
		_LOG(str);
		str = "/123/456///7/.//./../89";
		str.ResolveRelativePath();
		_LOG(str);

		rt::String_Ref p,h,path;

		str = "ftp://www.xxx.com/124/rty.jpg";
		str.SplitURL(p,h,path);
		_LOG("P:"<<p<<" , H:"<<h<<" , PATH:"<<path);

		str = "124/rty.jpg";
		str.SplitURL(p,h,path);
		_LOG("P:"<<p<<" , H:"<<h<<" , PATH:"<<path);

		str = "rty.jpg";
		str.SplitURL(p,h,path);
		_LOG("P:"<<p<<" , H:"<<h<<" , PATH:"<<path);

		str = "/124/rty.jpg";
		str.SplitURL(p,h,path);
		_LOG("P:"<<p<<" , H:"<<h<<" , PATH:"<<path);

		str = "http://www.google.com";
		str.SplitURL(p,h,path);
		_LOG("P:"<<p<<" , H:"<<h<<" , PATH:"<<path);
	}

	{	rt::String s = "123456";
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

	{	rt::String_Ref token;
		rt::CharacterSet_Symbol symbol;

		rt::SS code("c->a*trim_comments.TrimCodeComments(code);80");
		while(code.GetNextToken(symbol, token))
		{
			_LOG(token);
		}

		_LOG(code.TrimAfter('*'));
		_LOG(code.TrimBefore('.'));

		token.Empty();
		rt::String_Ref nontoken;
		while(code.GetNextToken(symbol, token, nontoken))
		{
			_LOG(token<<'\t'<<nontoken);
		}
	}

	{	rt::SS code("1\n\r\n2\n\n\r\n\n");
		rt::String_Ref line;
		_LOG("GetNextLine(line, true)");
		while(code.GetNextLine(line, true))
		{	_LOG('"'<<line<<'"');
		}
		line.Empty();
		_LOG("GetNextLine(line, false)");
		while(code.GetNextLine(line, false))
		{	_LOG('"'<<line<<'"');
		}
	}

	{
		rt::String a;
		//a.Empty();
		a += "fdasf";
	}

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


	// String_Ref
	{	rt::String_Ref a = "abcde";
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
	}
	_LOGNL;

	{	rt::String t(" abcd");
		_LOG(t.TrimLeftSpace());
		t = t.TrimLeftSpace();
		_LOG(t);
		
		rt::String_Ref a("hello world");
		_LOG(a);
		_LOG("a[2]: "<<a[2]);

		a = "123456";
		int num;
		a.ToNumber(num);	
		_LOG("ToNumber: "<<num);

		a.TrimLeft(2).ToNumber(num);	_LOG("TrimLeft(2).ToNumber(num): "<<num);
		a.SubStrHead(4).ToNumber(num);	_LOG("SubStrHead(4).ToNumber(num): "<<num);
		

		a = __STRING(123,456,789 ,123$ddd 'yes "\'no');
		rt::String_Ref f[8];

		rt::Zero(f);
		a.Split(f,8,',');
		_LOG("Split<>(','): "<<f[0]<<','<<f[1]<<','<<f[2]<<','<<f[3]<<','<<f[4]<<','<<f[5]<<','<<f[6]<<','<<f[7]);

		rt::Zero(f);
		a.Split(f,8,rt::SS(" ,"));
		_LOG("Split<>(\" ,\"): "<<f[0]<<','<<f[1]<<','<<f[2]<<','<<f[3]<<','<<f[4]<<','<<f[5]<<','<<f[6]<<','<<f[7]);

		rt::Zero(f);
		a.Split<true>(f,8,rt::SS(" ,$"));
		_LOG("Split<true>(\" ,$\"): "<<f[0]<<','<<f[1]<<','<<f[2]<<','<<f[3]<<','<<f[4]<<','<<f[5]<<','<<f[6]<<','<<f[7]);

		rt::SS("345;\"453;546\";4543;\"444\"").Split(f, 8, ';');
		_LOG("Split<>(...): "<<f[0]<<','<<f[1]<<','<<f[2]<<','<<f[3]<<','<<f[4]<<','<<f[5]<<','<<f[6]<<','<<f[7]);
	}

	{	_LOG((LPCSTR)(rt::String_Ref("hello world") + ' ' + 1233 + rt::String_Ref() + rt::String(" !!")));
		rt::String a = (LPCSTR)(rt::String_Ref("hello world") + ' ' + 1233);
		a = a + " !!";
		a += " ..";
		_LOG("String Expression 1: "<<a);

		a = rt::String_Ref("hello worlld").TrimLeft(6) + " !!";
		_LOG("String Expression 2: "<<a);

		a.Replace("ll", "===");
		_LOG("Replace: "<<a);

		bool is = a.StartsWith("wo");
		_LOG("IsPrefixOf: "<<is);
	}

	{	_LOG("BinaryCString: "<<rt::tos::BinaryCString<>("123456",6));
		_LOG("Binary: "<<rt::tos::Binary<>("123456",6));
	}

	{	rt::hash_map<rt::String, int, rt::String::hash_compare>	map;
		map["123"] = 456;
		_LOG("map[\"123\"]: "<<map["123"]);

		char buf[100];
		int  len = (int)(rt::String_Ref("123",3) + rt::String_Ref("\x0\x1\x3",3)).CopyTo(buf);
		_LOG("Length of \"123\" + \"\\x0\\x1\\x3\" is "<<len);
	}

	{	
		{ rt::tos::FileSize<false> t(41513);		_LOG("FileSize<false,false>: "<<t); }
		{ rt::tos::FileSize<true> t(4151351);		_LOG("FileSize<false,true>: "<<t); }
		{ rt::tos::FileSize<false> t(41513510000);	_LOG("FileSize<false,false>: "<<t); }
		{ rt::tos::FileSize<true> t(4151351000000000); _LOG("FileSize<false,true>: "<<t); }

		{ rt::tos::TimeSpan<true>	t(10);			_LOG("TimeSpan<true>: "<<t); }
		{ rt::tos::TimeSpan<true>	t(10000);		_LOG("TimeSpan<true>: "<<t); }
		{ rt::tos::TimeSpan<true>	t(100000000);	_LOG("TimeSpan<true>: "<<t); }
		{ rt::tos::TimeSpan<true>	t((int)(10000000000U-5000));	_LOG("TimeSpan<true>: "<<t); }
		{ rt::tos::TimeSpan<false>	t((int)(10000000000U-5000));	_LOG("TimeSpan<false>: "<<t); }
	}
}

void rt::UnitTests::precompiler()
{
	rt::String source3 = rt::SS(
		"#if VAL == 1\n"
		"#if true\n"
		"val is 1\n"
		"#endif\n"
		"val is 1 for sure\n"
		"#elif VAL == 2\n"
		"val is 2\n"
		"#elif VAL == 3\n"
		"val is 3\n"
		"#else\n"
		"val > 3\n"
		"#endif\n"
	);

	for(UINT i=1; i<=4; i++)
	{
		os::PrecompilerPredefinedMacros predefined;
		predefined.Set("VAL", rt::tos::Number(i));

		os::Precompiler s;
		s.SetEnvVars(&predefined);

		s.Compile("val.cpp", source3);
		//_LOG(source);
		_LOG("\nPrecompiled:");
		_LOG(s.GetCompiled());
	}


	rt::String source = rt::SS(
		"#define TT 234\n"
		"#define IT \"this is the value of IT\"\n"
		"#define IT2 Values\n"
		"#define MultiLine	Line1 \\\n"
		"                   Line2 \\\n"
		"                   Line3 \n"
		"#define Max(a,b)   a>b?a:b\n"
		"#define MERGED     %IT% + %IT2%\n"
		"\n"
		"%Max(1,2)%\n"
		"IT = %IT%%IT2%;\n"
		"MultiLine = %MultiLine%;\n"
		"MultiLine is Line1 \\\n"
		"             Line2 \\\n"
		"             %COMPUTERNAME%\n"
		"\n"
		"ComSpec = %ComSpec%%HOMEDRIVE%%SomeMissing%\n"
		"%NotClosed %HOMEPATH%%%\n"
		"%Max(%COMPUTERNAME%,12)%\n"
		"Merged = %MERGED%\n"
		"#ifdef TT\n"
		"TT = %TT%\n"
		"#ifdef TT2\n"
		"TT2 = %TT2%\n"
		"#endif\n"
		"#endif\n"
		"#ifndef TT\n"
		"TT not found\n"
		"#else\n"
		"TT exists\n"
		"#endif\n"
		"End of File\n\n"
	);

	rt::String source2 = rt::SS(
		"#define TT 234\n"
		"#define IT \"this is the value of it\"\n"
		"#define IT2 Values\n"
		"#define MultiLine	Line1 \\\n"
		"                   Line2 \\\n"
		"                   Line3 \n"
		"#define Max(a,b)   a>b?a:b\n"
		"#define MERGED     IT + IT2 + HAHA##TT##XIXI##Max(2,3)##WIWI\n"
		"\n"
		"Max(1,2)\n"
		"it = IT IT2;\n"
		"multiLine = MultiLine;\n"
		"multiLine is Line1 \\\n"
		"             Line2 \\\n"
		"             COMPUTERNAME\n"
		"\n"
		"comSpec = ComSpec HOMEDRIVE SomeMissing\n"
		"Max(COMPUTERNAME,12)\n"
		"Merged = MERGED\n"
		"#ifdef TT\n"
		"tt = TT\n"
		"#ifdef TT2\n"
		"tt2 = TT2\n"
		"#endif\n"
		"#endif\n"
		"#ifndef TT\n"
		"tt not found\n"
		"#else\n"
		"tt exists\n"
		"#endif\n"
		"#if defined(IT) || defined(TTTT)\n"
		"it is defined\n"
		"#endif\n"
		"#if TT > 100 && TT <= 300\n"
		"tt is fit\n"
		"#endif\n"
		"#if !defined(IT)\n"
		"it is not defined\n"
		"#endif\n"
		"#if IT == \"this is the value of it\"\n"
		"it is correct\n"
		"#endif\n"
		"#if IT != \"this is the value of it\"\n"
		"it is not correct\n"
		"#endif\n"
		"#define TXID(a, b)		(a<<16 | b)\n"
		"TXID(tx.Contract, tx.Op)\n"
		"End of File\n\n"
	);
	
	os::CommandLine cmd;
	cmd.LoadEnvironmentVariablesAsOptions();
	
	os::PrecompilerPredefinedMacros predefined;
	predefined.ImportCmdLineOptions(cmd);
	predefined.Set("COMPUTERNAME", "MOOD-PC");
	predefined.Set("ComSpec", "cmd.exe");
	predefined.Set("HOMEDRIVE", "C:");
	predefined.Set("HOMEPATH", "/var/usr/home");

	{	os::Precompiler s("%", "%");
		s.SetEnvVars(&predefined);
		s.Compile("test.bat", source);
		//_LOG(source);
		_LOG("\nPrecompiled:");
		_LOG(s.GetCompiled());
	}

	{	os::Precompiler s;
		s.SetEnvVars(&predefined);
		s.Compile("test.cpp", source2);
		//_LOG(source);
		_LOG("\nPrecompiled:");
		_LOG(s.GetCompiled());
	}

	//os::File("out.cpp", os::File::Normal_WriteText).Write(os::Precompiler("main.cpp").GetCompiled());
}

void rt::UnitTests::buffer()
{
	{	rt::TopWeightedValues<int, 5, float> votes;
		votes.Sample(1, 0.1f);
		votes.Sample(3, 0.4f);
		_LOG(votes);

		votes.Sample(4, 0.2f);
		votes.Sample(5, 0.02f);
		votes.Sample(2, 0.01f);
		_LOG(votes);

		votes.Sample(4, 0.3f);
		_LOG(votes);

		votes.Sample(2, 1.f);
		_LOG(votes);
	}

	rt::BufferEx<int>	buf;
	buf.SetSize(3);
	buf.Set(0);
	buf[2] = 123;
	_LOG("Init: "<<buf);

	{	rt::Buffer<int> buf2(buf);
		_LOG("Copy: "<<buf2);
	}

	buf.push_back() = 456;
	buf.push_back(789);
	buf.push_front(12);
	_LOG("push: "<<buf);

	buf.erase(1,3);
	_LOG("erase: "<<buf);

	buf.insert(1,3,100);
	_LOG("insert: "<<buf);
	buf.insert(1) = 200;
	_LOG("insert: "<<buf);

	buf.Sort();
	//std::sort(buf.Begin(), buf.End());
	_LOG("sort: "<<buf);

	// sort
	{
		struct _item
		{	rt::String_Ref	a;
			int				v;
			_item(LPCSTR aa, int vv){ a = aa; v = vv; }
			bool operator < (const _item&x) const { return a < x.a; }
		};

		rt::BufferEx<_item>	buf;
		buf.push_back(_item("hello", 3));
		buf.push_back(_item("world", 4));
		buf.push_back(_item("is", 2));
		buf.push_back(_item("shit", 1));

		buf.Sort();

		for(UINT i=0;i<buf.GetSize();i++)
			_LOG(buf[i].a);
	}

	rt::String str("1234567890");
	rt::CircularBuffer cyc;
	cyc.SetSize(1024);
	LPBYTE p;

	p = cyc.Push(237);
	memcpy(p,str,str.GetLength());
	cyc.SetBlockSize(p, str.GetLength());
	cyc.Peek();

	p = cyc.Push(499);
	memcpy(p,str,str.GetLength());
	cyc.SetBlockSize(p, str.GetLength()-1);

	if(cyc.Push(277) == nullptr)_LOG("Overflow");

	_LOG(rt::String_Ref((LPCSTR)cyc.Peek()->Data, (UINT)cyc.Peek()->Length));
	cyc.Pop();

	p = cyc.Push(199);
	memcpy(p,str,str.GetLength());
	cyc.SetBlockSize(p, str.GetLength()-2);

	_LOG(rt::String_Ref((LPCSTR)cyc.Peek()->Data, (UINT)cyc.Peek()->Length));
	cyc.Pop();

	_LOG(rt::String_Ref((LPCSTR)cyc.Peek()->Data, (UINT)cyc.Peek()->Length));

	rt::BufferEx<rt::String>	non_pod;
	non_pod.push_back() = "string1";
	non_pod.push_back("string2");
	non_pod.push_front("string0");
	_LOG("Strings: "<<non_pod);
	rt::BufferEx<rt::String>	non_pod_copy(non_pod);
	non_pod_copy.push_back("string3");
	_LOG("Strings copied: "<<non_pod_copy);

	rt::Buffer<WORD> a;
	a.SetSize(10);
	a.RandomBits(0);
	_LOG(a);
	_LOG("Top-1: "<<a.FindTopKth(1));
	_LOG("Top-3: "<<a.FindTopKth(3));
	_LOG("Top-6: "<<a.FindTopKth(6));
	_LOG("Top-10: "<<a.FindTopKth(10));

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
}

void rt::UnitTests::encoding()
{
	rt::Randomizer r; 

	for(UINT i=0;i<100000;i++)
	{
		char buf[20];
		for(UINT b=1;b<20;b++)
		{
			r.Randomize(buf, b);
			rt::tos::Base32CrockfordOnStack<> base32(buf, b);
			if(i==0)
			{	_LOG(rt::tos::Binary<>(buf,b)<<" = "<<base32);
			}

			char buf_dec[20];
			int dec_b = (int)os::Base32DecodeLength(base32.GetLength());
			os::Base32CrockfordDecode(buf_dec, dec_b, base32, base32.GetLength());
			if(dec_b != b || memcmp(buf_dec, buf, b) != 0)
			{	_LOG_ERROR(rt::tos::Binary<>(buf,b)<<" = "<<base32<<" => "<<rt::tos::Binary<>(buf_dec, dec_b));
				return;
			}
		}
	}

	for(UINT i=0;i<100000;i++)
	{
		char buf[20];
		for(UINT b=1;b<20;b++)
		{
			r.Randomize(buf, b);
			rt::tos::Base32CrockfordLowercaseOnStack<> base32(buf, b);
			if(i==0)
			{	_LOG(rt::tos::Binary<>(buf,b)<<" = "<<base32);
			}

			char buf_dec[20];
			int dec_b = (int)os::Base32DecodeLength(base32.GetLength());
			os::Base32CrockfordDecode(buf_dec, dec_b, base32, base32.GetLength());
			if(dec_b != b || memcmp(buf_dec, buf, b) != 0)
			{	_LOG_ERROR(rt::tos::Binary<>(buf,b)<<" = "<<base32<<" => "<<rt::tos::Binary<>(buf_dec, dec_b));
				return;
			}
		}
	}

	for(UINT i=0;i<100000;i++)
	{
		char buf[20];
		for(UINT b=1;b<20;b++)
		{
			r.Randomize(buf, b);
			rt::tos::Base32OnStack<> base32(buf, b);
			if(i==0)
			{	_LOG(rt::tos::Binary<>(buf,b)<<" = "<<base32);
			}

			char buf_dec[20];
			int dec_b = (int)os::Base32DecodeLength(base32.GetLength());
			os::Base32Decode(buf_dec, dec_b, base32, base32.GetLength());
			if(dec_b != b || memcmp(buf_dec, buf, b) != 0)
			{	_LOG_ERROR(rt::tos::Binary<>(buf,b)<<" = "<<base32<<" => "<<rt::tos::Binary<>(buf_dec, dec_b));
				return;
			}
		}
	}

	for(UINT i=0;i<100000;i++)
	{
		char buf[20];
		for(UINT b=1;b<20;b++)
		{
			r.Randomize(buf, b);
			rt::tos::Base32LowercaseOnStack<> base32(buf, b);
			if(i==0)
			{	_LOG(rt::tos::Binary<>(buf,b)<<" = "<<base32);
			}

			char buf_dec[20];
			int dec_b = (int)os::Base32DecodeLength(base32.GetLength());
			os::Base32Decode(buf_dec, dec_b, base32, base32.GetLength());
			if(dec_b != b || memcmp(buf_dec, buf, b) != 0)
			{	_LOG_ERROR(rt::tos::Binary<>(buf,b)<<" = "<<base32<<" => "<<rt::tos::Binary<>(buf_dec, dec_b));
				return;
			}
		}
	}

	for(UINT i=0;i<100000;i++)
	{
		char buf[20];
		for(UINT b=1;b<20;b++)
		{
			r.Randomize(buf, b);
			rt::tos::Base32CrockfordFavCharLowercaseOnStack<> base32(buf, b);
			if(i==0)
			{	_LOG(rt::tos::Binary<>(buf,b)<<" = "<<base32);
			}

			char buf_dec[20];
			int dec_b = (int)os::Base32DecodeLength(base32.GetLength());
			os::Base32CrockfordDecode(buf_dec, dec_b, base32, base32.GetLength());
			if(dec_b != b || memcmp(buf_dec, buf, b) != 0)
			{	_LOG_ERROR(rt::tos::Binary<>(buf,b)<<" = "<<base32<<" => "<<rt::tos::Binary<>(buf_dec, dec_b));
				return;
			}
		}
	}
}

#pragma pack(1)
	struct MyType
	{
		ULONGLONG	a;
	};
	struct MyTypeTR: public MyType
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

		_LOG("a = "<<rt::GetDataSize(a)<<" PTR="<<(a == rt::GetDataPtr(a)));
		_LOG("b = "<<rt::GetDataSize(b)<<" PTR="<<(&b == rt::GetDataPtr(b)));
		_LOG("c = "<<rt::GetDataSize(c)<<" PTR="<<(c.Begin() == rt::GetDataPtr(c)));
		_LOG("d = "<<rt::GetDataSize(d)<<" PTR="<<(d.c_str() == rt::GetDataPtr(d)));
		_LOG("e = "<<rt::GetDataSize(e)<<" PTR="<<(e.Begin() == rt::GetDataPtr(e)));
	}
	
	{	
		_LOG("sizeof(rt::TypeTraits<MyType>::t_Val) = "<<sizeof(rt::TypeTraits<MyType>::t_Val));
		_LOG("rt::TypeTraits<MyType>::IsPOD = "<<rt::TypeTraits<MyType>::IsPOD);
		_LOG("sizeof(rt::TypeTraits<MyTypeTR>::t_Val) = "<<sizeof(rt::TypeTraits<MyTypeTR>::t_Val));
		_LOG("rt::TypeTraits<MyTypeTR>::IsPOD = "<<rt::TypeTraits<MyTypeTR>::IsPOD);
	}
	
	_LOG("sizeof(BYTE)	="<<sizeof(BYTE)	);
	_LOG("sizeof(WORD)	="<<sizeof(WORD)	);
	_LOG("sizeof(DWORD)	="<<sizeof(DWORD)	);
	_LOG("sizeof(UINT)	="<<sizeof(UINT)	);
	
	_LOG("sizeof(QWORD)		="<<sizeof(QWORD)		);
	_LOG("sizeof(LONGLONG)	="<<sizeof(LONGLONG)	);
	_LOG("sizeof(ULONGLONG)	="<<sizeof(ULONGLONG)	);
	_LOG("sizeof(QWORD)		="<<sizeof(QWORD)		);


	{	LPBYTE p = _Malloc32AL(BYTE,2047);
		rt::Zero(p,2047);
		_SafeFree32AL(p);
	}

	_LOG("rt::MaxPower2Within(2049): "<<rt::MaxPower2Within(2049));

	{	short a = 1,b = 2;
		rt::Swap(a,b);
		_LOG("rt::Swap: <a, b> = <"<<a<<','<<b<<'>');
	}

	{	ULONGLONG a = 0xbabeface,b = 0xbabeface;
		rt::SwapByteOrder(b);
		_LOG(	"rt::SwapByteOrder("<<rt::tos::Binary<>(&a,sizeof(ULONGLONG))<<
				") = "<<rt::tos::Binary<>(&b,sizeof(ULONGLONG)));
	}

	_LOG("int is POD: "<<rt::TypeTraits<int>::IsPOD);
	_LOG("int[24] is POD: "<<rt::TypeTraits<int[24]>::IsPOD);

	{	bool ret = rt::IsTypeSame<int,int>::Result;
		_LOG("rt::IsTypeSame<int,int>: "<<ret);
		ret = rt::IsTypeSame<int,float>::Result;
		_LOG("rt::IsTypeSame<int,float>: "<<ret);
		ret = rt::IsTypeSame<const int,int>::Result;
		_LOG("rt::IsTypeSame<const int,int>: "<<ret);
		ret = rt::IsTypeSame<rt::Remove_QualiferAndRef<const int>::t_Result,int>::Result;
		_LOG("rt::Remove_QualiferAndRef<const int>: "<<ret);
		ret = rt::IsTypeSame<rt::TypeTraits<int[23]>::t_Element, int>::Result;
		_LOG("rt::TypeTraits<int[23]>::t_Element: "<<ret);
		ret = rt::NumericTraits<double>::IsFloat;
		_LOG("rt::NumericTraits<double>::IsFloat: "<<ret);
		ret = rt::NumericTraits<float>::IsUnsigned;
		_LOG("rt::NumericTraits<float>::IsUnsigned: "<<ret);
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
		catch(std::bad_alloc x)
		{
			_LOG("catch std::bad_alloc");
		};
	}

	_LOG("17 ~= 2^"<<Log2(17U));

	{
		_LOG(rt::TypeNameToString<int>());
		_LOG(rt::TypeNameToString<os::Timestamp>());
		_LOG(rt::TypeNameToString<os::_tagProcessPriority>());
		
		struct a { int __; };
		_LOG(rt::TypeNameToString<a>()<<" (nested type in function scope)");
    }
}


void rt::UnitTests::timedate()
{
	{	os::Timestamp	tm;
		tm.LoadCurrentTime();
		_LOGC(rt::tos::Timestamp<>(tm));
		os::Sleep(10);
		tm.LoadCurrentTime();
		_LOGC(rt::tos::Timestamp<>(tm));
		os::Sleep(10);
		tm.LoadCurrentTime();
		_LOGC(rt::tos::Timestamp<>(tm));

		os::Timestamp	stm;
		stm.SetDateTime("1980/3/2 12:21:33.321");	_LOG((stm.GetDateTime()));
		stm.SetDateTime("1980/3/2 12:21:33");	_LOG((stm.GetDateTime()));
		stm.SetDateTime("1980/3/2 12:21");	_LOG((stm.GetDateTime()));
		stm.SetDateTime("12:21:33.321");	_LOG((stm.GetDateTime()));
		stm.SetDateTime("12:21:33");	_LOG((stm.GetDateTime()));
		stm.SetDateTime("1980/3/2");	_LOG((stm.GetDateTime()));
		stm.SetDateTime("12:21");	_LOG((stm.GetDateTime()));
		stm.SetDateTime(rt::tos::Number(tm._Timestamp));	_LOG((stm.GetDateTime()));
	}

	{	os::Timestamp tm;
		tm.SetDateTime(1980,3,2);
		_LOG("Day: "<<tm.GetDateTime().DayOfWeek<<" == "<<os::Timestamp::GetDayOfWeek(1980,3,2));
		tm.SetDateTime(1980,3,4);
		_LOG("Day: "<<tm.GetDateTime().DayOfWeek<<" == "<<os::Timestamp::GetDayOfWeek(1980,3,4));
		//_LOG(os::Timestamp::DaysSince0000(1970));
	}

	{	os::Timestamp tm;
		tm.LoadCurrentTime();
		os::Sleep(50);
		LONGLONG time_lapse = tm.TimeLapse();
		_LOG("Timelapse is between [50, 400]: " << (50 <= time_lapse && time_lapse <= 400));
	}

	{	os::HighPerformanceCounter	hpc;
		hpc.LoadCurrentCount();
		os::Sleep(1000);
		_LOGC(hpc.TimeLapse() << " nano-sec");
		os::Sleep(1500);
		_LOGC(hpc.TimeLapse() << " nano-sec");

		hpc.SetOutputMillisecond();
		hpc.LoadCurrentCount();
		os::Sleep(1000);
		_LOGC(hpc.TimeLapse() << " msec");
		os::Sleep(1500);
		_LOGC(hpc.TimeLapse() << " msec");
	}

	{	os::Timestamp tm;
		tm.LoadCurrentTime();
		_LOGC(rt::tos::Timestamp<>(tm));

		tm.SetLocalDateTime(os::Timestamp::Fields("2001-4-3 13:45:3"));
		_LOG(rt::tos::Timestamp<>(tm));

		tm.SetLocalDateTime(os::Timestamp::Fields("2011-7-13"));
		_LOG(rt::tos::Timestamp<>(tm));
	}
	
	{	os::Timestamp	tt;
		os::Timestamp::Fields f;
		f.FromInternetTimeFormat("Tue, 15 Nov 1994 12:45:26 GMT");
		tt.SetDateTime(f);
		_LOG("FromInternetTimeFormat: "<<rt::tos::Timestamp<>(tt, false));

		char ttt[30];
		tt.GetDateTime().ToInternetTimeFormat(ttt);
		_LOG("ToInternetTimeFormat: "<<ttt);
	}

	{	os::Date32 dd;
		dd.SetDate(2012,1,20);
		_LOG("LastMonth: "<<rt::tos::Date<>(dd.LastMonth()));

		os::Date32 dd2 = dd;
		_LOG("Date32 diff 1: "<<dd2 - dd<<','<<dd - dd2);

		dd2++;
		_LOG("Date32 diff 1: "<<dd2 - dd<<','<<dd - dd2);

		dd2++;
		_LOG("Date32 diff 1: "<<dd2 - dd<<','<<dd - dd2);
	}

	{	_LOG("Waiting for 2 seconds ...");
		os::Timestamp	t1,t2;
		os::TickCount	tick;
		t1.LoadCurrentTime();
		tick.LoadCurrentTick();
		os::Sleep(2100);
		t2.LoadCurrentTime();
		_LOG("os::TickCount in sec: " << tick.TimeLapse() / 1000);
	}
}

void rt::UnitTests::commandline()
{
	os::CommandLine cmd;
	cmd.SetOptionDefault("TEST_OPT", "SomeValue");
	cmd.LoadEnvironmentVariablesAsOptions();

	rt::String text = "Shell: %ComSpec%\nTest: [%TEST_OPT%]";
	cmd.SubstituteOptions(text);

	_LOG(text);
}

void rt::UnitTests::async_write()
{
	rt::Buffer<BYTE>	data;
	data.SetSize(23*1024*1024);
	data.RandomBits(94784);

	LPCSTR fn = "async_write.test";

	{
		os::FileWrite f;
		if(f.Open(fn, os::FileWrite::FW_TRUNCATE|os::FileWrite::FW_ASYNC, 8))
		{
			for(UINT i=0; i<data.GetSize(); i+=23*1024)
			{
				if(!f.Write(&data[i], 23*1024))
					return;
			}
		}

		f.WriteHeader(0x12345678deadbeefULL);
		f.Close();

		os::FileBuffer<BYTE> load;
		load.Open(fn);

		_LOG("Size Header: "<<load.GetSize() - data.GetSize());
		_LOG("Header = "<<rt::tos::HexNum<>(*(ULONGLONG*)load.Begin()));
		_LOG("Content Match: "<<(0==memcmp(load.Begin() + 8, data.Begin(), data.GetSize())));
	}

	{
		os::FileWrite f;
		if(f.Open(fn, os::FileWrite::FW_TRUNCATE|os::FileWrite::FW_UTF8SIGN))
		{
			for(UINT i=0; i<data.GetSize(); i+=23*1024)
			{
				if(!f.Write(&data[i], 23*1024))
					return;
			}
		}

		f.Close();

		os::FileBuffer<BYTE> load;
		load.Open(fn);

		_LOG("Size Header: "<<load.GetSize() - data.GetSize());
		_LOG("Header = "<<rt::tos::Base16OnStack<>(load.Begin(), 3));
		_LOG("Content Match: "<<(0==memcmp(load.Begin() + 3, data.Begin(), data.GetSize())));
	}

	os::File::RemovePath(fn);
}

void rt::UnitTests::file()
{
	{	os::FileBuffer<BYTE>	file("D:/ArtSq/Coin-FE/Dev/proj/unit_test/captcha_bg.jpg");
		os::File out;
		out.Open("data_define.hpp", os::File::Normal_WriteText);
		out.Write(rt::SS("static const BYTE _Data[") + (ULONGLONG)file.GetSize() + "] = {\n");
		LPCBYTE p = file;
		UINT sz = (UINT)file.GetSize();
		while(sz)
		{
			UINT block = rt::min(32U, sz);
			out.Write("\t",1);
			for(UINT i=0;i<block;i++)
			{
				char buf[10];
				sprintf(buf,"0x%02X,",p[i]);
				if(sz == block && i == block-1)
					out.Write(buf, 4);
				else
					out.Write(buf, 5);
			}
			sz-=block;
			p+=block;
			out.Write("\n",1);
		}
		out.Write("};\n",3);
		out.Close();
	}


	{	rt::String	fn;
		for(int i=0;i<10;i++)
		{
			LPCSTR filename = "test.txt";
			if(os::File::ProbeAvailableFilename(filename, fn))
				filename = fn;
			os::File(filename, os::File::Normal_Write).Write("123",3);
			_LOG(fn);
		}

		os::File::Remove("test.txt");
		os::FileList fl;
		fl.Populate(".", ").txt");
		for(UINT i=0; i<fl.GetCount(); i++)
			os::File::Remove(fl.GetFilename(i).Begin() + 1);
	}

	LPCSTR fn = "test_\xe4\xbd\xa0\xe5\xa5\xbd.txt";
	
	os::File	file;
	if(file.Open(fn, os::File::Normal_Write))
	{
		file.Write(rt::String_Ref("Testing String wrote to file !!"));
		file.Close();
	}

	rt::String val;
	if(	file.Open(fn) &&
		val.SetLength((SIZE_T)file.GetLength()) &&
		(SIZE_T)(file.Read(val,(UINT)val.GetLength()) == val.GetLength())
	)
	{	_LOG("File Content: "<<val);
		file.Close();
		_LOG("File Remove: "<<os::File::Remove(fn));
		_LOG("File Remove: "<<os::File::Remove(fn));
	}

	{	os::FilePacked_Writer f;
		f.Open("fpack.ckdp");
		f.Write(0x1001, "1234567890" ,8);
		f.Write(0x1002, "1234567890" ,7);
		f.Write(2, "1234567890" ,6);
		f.Write(1, "1234567890" ,5);
	}

	{	os::FilePacked_Reader f;
		f.Open("fpack.ckdp");
		rt::String str;
		UINT idx = f.FindFirstFile(2);
		if(idx != INFINITE)
		{	str.SetLength(f.GetFileSize(idx));
			f.Read(idx,str);
			_LOG("2 => "<<str);
		}

		if((idx = f.FindFirstFile(0x1000,0xf000)) != INFINITE)
		{
			do
			{	str.SetLength(f.GetFileSize(idx));
				f.Read(idx,str);
				_LOG(f.GetFileName(idx)<<" => "<<str);
			}while((idx = f.FindNextFile()) != INFINITE);
		}
	}
}

void rt::UnitTests::pfw()
{
	{	os::ParallelFileWriter	pfw;

		pfw.Open("pfw_test.txt", false);
		pfw.SetWriteDownInterval(1000);


		struct _call
		{
			static DWORD _func(LPVOID p)
			{
				os::ParallelFileWriter* w = (os::ParallelFileWriter*)p;
				for(UINT i=0;i<5000;i++)
				{
					w->WriteString(rt::String_Ref("ParallelFileWriter Test ") + rt::tos::Timestamp<>(os::Timestamp::Get()) + rt::SS(" \r\n"));
					os::Sleep(10);
				}
				return 0;
			}
		};

		rt::Buffer<os::Thread>	threads;
		threads.SetSize(100);
		for(UINT i=0;i<threads.GetSize();i++)
		{
			threads[i].Create(_call::_func, &pfw);
		}

		for(UINT i=0;i<threads.GetSize();i++)
			threads[i].WaitForEnding();

		pfw.LogAlert();
		pfw.Close();

		os::FileReadLine file;
		file.Open("pfw_test.txt");

		rt::String_Ref line;
		int lines = 0;
		int len = -1;

		while(file.GetNextLine(line))
		{
			if(len == -1)
			{	len = (int)line.GetLength();
			}
			else
			{	ASSERT(len == line.GetLength());
			}
		
			lines++;
		}

		_LOG("Total lines: "<<lines);
	}

	{	os::ParallelFileWriter	pfw;
		pfw.Open("pfw_test_%HOUR%.txt");
		pfw.SetWriteDownInterval(500);

		for(UINT i=0;i<100;i++)
		{	pfw.WriteLine(rt::SS("Some thing appended") + rt::tos::Timestamp<>(os::Timestamp::Get()));
			os::Sleep(100);
		}
	}

	{	os::ParallelFileWriter	pfw;
		pfw.Open("pfw_test_%HOUR%.txt");
		pfw.SetWriteDownInterval(500);

		for(UINT i=0;i<100;i++)
		{	pfw.WriteLine(rt::SS("Some thing appended again") + rt::tos::Timestamp<>(os::Timestamp::Get()));
			os::Sleep(100);
		}
	}
}

void rt::UnitTests::plog()
{
	os::ParallelLog log(500);

	for(;;)
	{	_LOG("Tick: "<<os::TickCount::Get());
		os::Sleep(100);
	}

	//log.Write("test", "test_file", 0, "test_func", rt::LOGTYPE_IN_CONSOLE|rt::LOGTYPE_INFORMATIONAL);

}

/*
void test_ipp_zip()
{
	os::FileZip	zip;
	zip.SetCompressionMode();

	zip.Open("TestOut.zip", os::File::Normal_Write,false) &&
	zip.AddZeroSizedEntry("test") &&
	zip.AddFile("1.txt", "Content of 1.txtContent of 1.txtContent of 1.txtContent of 1.txtContent of 1.txtContent of 1.txtContent of 1.txtContent of 1.txtContent of 1.txtContent of 1.txt", 160) &&
	zip.AddFile("test/2.txt", "Content of 2.txtContent of 2.txtContent of 2.txtContent of 2.txtContent of 2.txtContent of 2.txtContent of 2.txtContent of 2.txtContent of 2.txtContent of 2.txt", 160) &&
	zip.Save();

//#ifdef PLATFORM_WIN
//	os::EnableCrashDump("shared_test", true, true);
//	_asm int 3;
//#endif
}
*/

void rt::UnitTests::inet_encoding()
{
	{	static const LPCSTR testcase[][2] = 
		{
			{ "1", "MQ==" },
			{ "12", "MTI=" },
			{ "123", "MTIz" },
			{ "1234", "MTIzNA=="},
			{ "12345", "MTIzNDU=" }
		};

		for(UINT i=0;i<sizeofArray(testcase);i++)
		{
			rt::String_Ref text(testcase[i][0]);
			rt::String_Ref b64(testcase[i][1]);

			char buf[100];
			ASSERT(os::Base64EncodeLength(text.GetLength())==b64.GetLength());
			os::Base64Encode(buf, text.Begin(), text.GetLength());
			_LOG(rt::String_Ref(buf, b64.GetLength())<<" = "<<b64);
			
			ASSERT(os::Base64DecodeLength(b64.Begin(), b64.GetLength())==text.GetLength());
			SIZE_T len;
			_LOG("Full Data: "<<os::Base64Decode(buf, &len, b64.Begin(), b64.GetLength()));
			_LOG(rt::String_Ref(buf, len)<<" = "<<text);
		}

		{	// robust decoding
			char buf[100];
			rt::String_Ref b64("MTIzNDU\n\rew");
			SIZE_T  len;
			_LOG("Full Data: "<<os::Base64Decode(buf, &len, b64.Begin(), b64.GetLength()));
			_LOG(rt::String_Ref(buf, len));
		}

		_LOG(rt::tos::Base64OnStack<256>("123456", 6));
	}

	return;
}

inet::Socket	a,b,c;

void rt::UnitTests::socket()
{
	struct _call
	{
		static DWORD _func(LPVOID p)
		{
			LPCSTR name = (LPCSTR)p;
			inet::SocketEvent	sevt;
			sevt.Add(a);
			sevt.Add(b);

			for(;;)
			{
				_LOG(name<<": on call");
				int ret = sevt.WaitForEvents();
				if(ret > 0)
				{
					for(int i=0;i<ret;i++)
					{
						inet::SOCKET s = sevt.GetNextSocketEvent_Read();
						char message[512];
						inet::InetAddr addr;
						UINT l = 0;
						((inet::Socket&)s).RecvFrom(message, sizeof(message), l, addr);
						if(l <= 0)return 0;
						_LOG(name<<": "<<rt::String_Ref(message, l)<<"\ton "<<addr.GetPort());
					}

					_LOG(name<<": Sleep 2 sec");
					os::Sleep(2000);
				}
				else return 0;
			}
		}
	};



	inet::InetAddr addr;
	addr.SetAsLocal();
		
	addr.SetPort(10000);
	a.Create(addr, SOCK_DGRAM);

	addr.SetPort(10001);
	b.Create(addr, SOCK_DGRAM);

	addr.SetPort(10002);
	c.Create(addr, SOCK_DGRAM);

	os::Thread th1,th2,th3,th4;
	th1.Create(_call::_func, (LPVOID)"Thread 1");
	th2.Create(_call::_func, (LPVOID)"Thread 2");
	th3.Create(_call::_func, (LPVOID)"Thread 3");
	th4.Create(_call::_func, (LPVOID)"Thread 4");

	os::Sleep(1500);
	addr.SetPort(10000);
	c.SendTo("Message 1",9, addr);

	os::Sleep(500);
	c.SendTo("Message 2",9, addr);
	addr.SetPort(10001);
	b.SendTo("Message 3",9, addr);
	b.SendTo("Message 4",9, addr);
	b.SendTo("Message 5",9, addr);
	b.SendTo("Message 6",9, addr);
	b.SendTo("Message 7",9, addr);

	os::Sleep(8000);

	a.Close();
	b.Close();
	c.Close();

	th1.WaitForEnding();
	th2.WaitForEnding();
	th3.WaitForEnding();
	th4.WaitForEnding();
}

struct A
{
	os::TickCount tc;
	A(){ tc.LoadCurrentTick(); }
	~A()
	{	_LOG("killed, after "<<tc.TimeLapse()<<" msec");
	}
};

void rt::UnitTests::delayed_deletion()
{
	A* p = new A;
	_SafeDel_Delayed(p, 2000);
	p = new A;
	_SafeDel_Delayed(p, 5000);

	os::Sleep(9000);
}

void rt::UnitTests::sysinfo()
{
	rt::String exe, host, user;
	os::GetExecutableFilename(exe);
	os::GetHostName(host);
	os::GetLogonUserName(user);

	_LOGC("Executable: "<<exe);
	_LOGC("User: "<<user<<'@'<<host);

	_LOGC("#CPU: "<<os::GetNumberOfProcessors());
	_LOGC("#CPU (Physical): "<<os::GetNumberOfPhysicalProcessors());
	
	rt::String os_name, os_version;
	os::GetOSVersion(os_version, false);
	os::GetOSVersion(os_name, true);
	_LOGC("OS: "<<os_name<<"  "<<os_version);
	
	ULONGLONG busy[2], total[2];
	{
		os::GetProcessorTimes(busy, total);
		os::Sleep(1000);
		os::GetProcessorTimes(busy+1, total+1);
		_LOGC("INV: "<<(total[1] - total[0]));
	}
	_LOGC("CPU USAGE: "<<100*(busy[1]-busy[0])/(total[1]-total[0])<<'%');
	{	ULONGLONG free, total;
		os::GetSystemMemoryInfo(&free, &total);
		_LOGC("MEMORY: " << rt::tos::FileSize<>(free) << '/' << rt::tos::FileSize<>(total));
	}
	{	SIZE_T free, total;
		os::GetProcessMemoryLoad(&free, &total);
		_LOGC("MEMORY Load: "<<rt::tos::FileSize<>(free)<<'/'<<rt::tos::FileSize<>(total));
	}
	
	_LOGC("FREE DISK: "<<rt::tos::FileSize<>(os::GetFreeDiskSpace("./")));
	_LOGC("Power State: "<<os::GetPowerState());

	rt::String uid;
	os::GetDeviceUID(uid);
	_LOGC("DevUID: "<<uid);
}


void test_socket_io(bool recv)
{
	if(recv)
	{
		inet::Socket	s;
		//s.Create(inet::InetAddr(INADDR_ANY, 2000), SOCK_DGRAM);
		if(!s.Create(inet::InetAddr("10.0.0.15", 2000), SOCK_DGRAM))return;
		//if(!s.Create(inet::InetAddr("192.168.1.111", 2000), SOCK_DGRAM))return;
		s.EnableNonblockingIO();

		inet::SocketEvent	sevt;
		sevt.Add(s);

		int q = 0;
		for(;;)
		{
			int ret = sevt.WaitForEvents();
			if(ret > 0)
			{
				for(int i=0;i<ret;i++)
				{
					inet::SOCKET s = sevt.GetNextSocketEvent_Read();
					char message[512];
					inet::InetAddr addr;
					UINT l = 0;
					((inet::Socket&)s).RecvFrom(message, sizeof(message), l, addr);
					if(l <= 0)return;
					_LOG(l<<"\tfrom "<<rt::tos::ip(addr)<<'\t'<<q++);
				}
			}
			else return;
		}
	}
	else
	{
		inet::Socket	s;
		s.Create(inet::InetAddr(INADDR_ANY, 10001), SOCK_DGRAM);
		s.EnableNonblockingIO();

		for(;;)
		{
			s.SendTo("123",3, inet::InetAddr("10.0.0.12",10000));
			os::Sleep(1000);
		}
	}
}

void rt::UnitTests::socket_io()
{
	os::Thread recv;
	recv.Create(
		[](){
			test_socket_io(true);
		}
	);

	test_socket_io(false);
}

void rt::UnitTests::socket_socket_event()
{
	inet::Socket recv[2];
	recv[0].Create(inet::InetAddr("11.1.1.22", 20001), SOCK_DGRAM);
	recv[1].Create(inet::InetAddr("11.1.1.22", 20002), SOCK_DGRAM);

	inet::SocketEvent se;
	se.Add(recv[0]);
	se.Add(recv[1]);

	os::Thread sender;
	sender.Create([](){

		inet::InetAddr target[2] = {
			{"11.1.1.22", 20001},
			{"11.1.1.22", 20002}
		};

		inet::Socket send;
		send.Create(inet::InetAddr("11.1.1.22", 20000), SOCK_DGRAM);

		for(UINT i=0;;i++)
		{
			char buf[1024];
			os::Sleep(1000);

			UINT len = (UINT)(rt::SS("Msg ") + i).CopyTo(buf);
			UINT idx = i%sizeofArray(target);
			send.SendTo(buf, len, target[idx]);
			_LOG("To "<<idx<<": "<<rt::String_Ref(buf, len));
		}
	});

	for(;;)
	{
		UINT count = se.WaitForEvents();
		if(count>0)
		{
			char buf[1024];
			UINT read = 0;
			inet::InetAddr from;

			for(UINT i=0; i<count; i++)
			{
				inet::Socket sock;
				sock.Attach(se.GetNextSocketEvent_Read());
				sock.RecvFrom(buf, sizeof(buf), read, from);
				sock.Detach();  // don't miss that

				_LOG(rt::String_Ref(buf, read));
			}		
		}
	}
}

void rt::UnitTests::filelist()
{
	rt::String fp;
	os::FileList	fl;
	fl.Populate("../tests", ".cpp");
	fl.Sort();
	for(UINT i=0;i<fl.GetCount();i++)
	{
		_LOG(fl.GetFilename(i));
	}
    
    rt::Buffer<os::Process::Info>   list;
    os::Process::Populate(list);

	_LOG("\nProcess Listed "<<!!list.GetSize());
	for(UINT i=0;i<list.GetSize();i++)
	{
		_LOGC(list[i].Name.GetFileName()<<'('<<list[i].PID<<"), "<<rt::tos::Timestamp<>(list[i].StartTime));
	}
}

void rt::UnitTests::sockettimed()
{
	inet::SocketTimed s;
	inet::InetAddr addr;
	addr.SetAsLocal();
	s.Create(addr);
	if(s.ConnectTo(inet::InetAddr("i-funbox.com",80)))
	{
		_LOG("connected");
	}
	else
	{
		_LOG("failed");
	}

}


void rt::UnitTests::smallmath()
{
	{
		rt::Vec4d	c;
		c.x = 1e-100;
		c.y = 1e100;
		c.z = 100.4;
		c.w = 0;
		_LOG(c);
	}

	{	rt::Vec3f	c;
		c.x = 1e-10f;
		c.y = 1e10f;
		c.z = 100.4f;
		_LOG(c);
	}

	rt::Vec3d	a({ 1.2, 3.4, 5.6 }), b({ -6.5, -4.3, 2.1 });
	rt::String s = rt::SS("STR-EXP: a=(") + a + ')';
	_LOG(s);
	a.Normalize();
	_LOG(a);
	_LOG(a<<" dot "<<b<<" = "<<a.Dot(b));

	rt::Randomizer r(7749);
	rt::Mat3x3d	m;
	for (double& x : m._p)
		x = r.GetNext() * 123. / INT_MAX - 88.;
	_LOG(m);
	m.Transpose();
	_LOG(m);

	rt::Quaterniond  quat;
	quat.ImportRotationMatrix(m,a);
	quat.ExportRotationMatrix(m);
	_LOG(m);
	_LOG("scale = "<<a);
}


void rt::UnitTests::vm()
{
	LPVOID p = os::VMAlloc(1024*1024*1024);
	_LOG("1G VM: "<<!!p);

	((LPBYTE)p)[1024*1024*1024 - 1] = 0;
	os::VMFree(p, 1024*1024*1024);

	os::FileMapping map;
	
	os::File::Remove("filemapping.data");
	map.Open("filemapping.data", 100*1024*1024, false);
	_LOG("100M File Mapping: "<<!!map.GetBasePtr());

	_LOG("Write Head");
	((LPBYTE)map.GetBasePtr())[0] = 0;
	_LOG("Write Tail");
	((LPBYTE)map.GetBasePtr())[100*1024*1024 - 1] = 0;
	map.Close();

	os::File::Remove("filemapping.data");
}

void rt::UnitTests::sortedpush()
{
	rt::Randomizer a(100);

	{	_LOG("BufferEx::SortedPush");
		rt::BufferEx<WORD>	sorted;

		for(UINT i=0; i<20; i++)
		{
			WORD new_v = (BYTE)a.GetNext();
			SSIZE_T pos = sorted.SortedPush(new_v);
			_LOG(sorted << " Add:" <<new_v<<" at "<<pos);
		}
	}

	{	_LOG("Buffer::SortedPush");
		rt::Buffer<WORD>	sorted;
		sorted.SetSize(8);
		sorted.Void();

		for(UINT i=0; i<20; i++)
		{
			WORD new_v = (BYTE)a.GetNext();
			SSIZE_T pos = sorted.SortedPush(new_v);
			_LOG(sorted << " Add:" <<new_v<<" at "<<pos);
		}
	}
}
