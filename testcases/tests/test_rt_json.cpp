#include "../../core/rt/json.h"
#pragma warning(disable: 4838)

#include "test.h"

class _JArray;
void json_GetKeyValue()
{
	rt::JsonObject json = rt::SS("{\"aa:aa\": 3}");
	_LOG(json.GetValueAs<int>("aa:aa"));
	rt::JsonKeyValuePair kv;
	json.GetNextKeyValuePair(kv);
	_LOG(kv.GetKey() << " = " << kv.GetValue());
}
void json_JsonBeautified() 
{
	rt::String str =
		(
			J_IF(false, J(cond_false) = "no"),
			J(abs) = 1.3,
			J_IF(1 > 0, J(cond_true) =
				(
					J_IF(2 > 1, J(cond_true_nested) = JA(1, 2, "eee")),
					J(qwe) = "nested",
					J_IF(2 < 1, J(cond_false_nested) = 1),
					J_IF(2 < 1, J(cond_false_nested) = 1),
					J(yue) = 3.1f,
					J_IF(2 < 1, J(cond_false_nested) = 1)
					)
			),
			J(empty) = (
				J_IF(2 < 1, J(cond_false_nested) = 2),
				J_IF(2 < 1, J(cond_false_nested) = 2)
				),
			J(fix) = (
				J(H) = 0,
				J_IF(false, J(R) = 1),
				J_IF(true, J(R) = 2)
				),
			J_IF(true, J(cond_true) = 1234),
			J_IF(false, J(cond_false) = "no")
			);

	_LOG(rt::JsonBeautified(str));
}
void json_JArray()
{
	rt::_JArray<> a;
	a.Append((J(name) = "jake", J(age) = 12));
	a.Append((J(name) = "mary", J(age) = 7));
	a.Append(1);
	a.Append(false);
	a.Append("haha");
	a.Append('@');

	_LOG(rt::JsonBeautified(rt::String(a)));
	std::string std_string("std::string");

	auto JsonObject =
		(J(Key:Complex) = "complex\\\" key",
			J(name) = "this is name",
			J(sex) = "a female",
			J(empty) = rt::String_Ref(),
			J(alive) = true,
			J(age) = 20,
			J(char) = 'C',
			J(phone) = (J(number) = 5235251,
				J(ext) = 432
				),
			J(weight) = rt::String_Ref() + 123.5f + "kg",
			J(state) = (J(paid) = false,
				J(lastshow) = rt::tos::Date<>(os::Date32(2007, 3, 2))
				),
			J(children) = a,
			J(numbers) = JA(1,
				2,
				"hello",
				(J(c) = 1.2),
				JA(3, 'c', false),
				JB("InArray"),
				4.5f
			),
			J(other) = JA((J(f0) = 0),
				(J(f1) = 1)
			),
			J(std) = std_string,
			J(bin) = JB("1234567890ABCDEFGHIJK"),
			J(raw) = rt::_JVal(""),
			J(empty) = JA()
			);

	char buf[1024];
	int  len = JsonObject.CopyTo(buf);
	_LOG("Json Size: " << len << " = " << JsonObject.GetLength());

	_LOG(rt::JsonBeautified(rt::String_Ref(buf, len)));
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
	while (obj.GetNextKeyValuePair(kv))
	{
		_LOG(kv.GetKey() << " [" << json_type_name[kv.GetValueType()] << "] = " << kv.GetValue());
	}

	rt::JsonArray arr = obj.GetValue("children");
	rt::JsonObject child;
	while (arr.GetNextObjectRaw(child))
	{
		_LOG(child.GetValue("name"));
	}
}

void json_GetNextObjectRaw()
{
	rt::JsonObject json(__STRING(
		{
			c:1,
			b : [2,3,4] ,
			a : 5
		}
	));

	rt::JsonObject json_sub(__STRING(
		{
			d: "Yes",
			c : 8.9
		}
	));

	rt::JsonArray b = json.GetValue("b");
	rt::JsonObject child;
	while (b.GetNextObjectRaw(child))
	{
		int val;
		child.GetString().ToNumber(val);
		_LOG(val);
	};
	rt::String out;
	rt::JsonObject::Override(json, json_sub, out);
	rt::JsonObject doc(out);
	for (const char* s : { "a", "b", "c", "d" })
		_LOG(s << ": " << doc.GetValue(s));
}
void json_EscapeString()
{
	rt::SS a("\x0\t\\ABC\'\"123");
	rt::JsonEscapeString e(a);
	_LOG("Escaped String: " << e);

	rt::JsonUnescapeString u(e);

	_LOG("Unescaped Matches: " << (u == a));
}
void json_Output()
{
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
			J(c) = JA("String", rt::String_Ref(), (J(a) = 12.f), rt::tos::Base16OnStack<>(123456), rt::String("123456"), rt::tos::Base32CrockfordLowercaseOnStack<>(0ULL))
			);

		_LOG(rt::JsonBeautified(a));
	}
}
void json_AppendKey()
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
void json_J_IF()
{
	rt::String j_if;
	j_if = (
		J_IF(true, (J(a) = 1))
		);
	_LOG("J_IF: " << rt::JsonBeautified(j_if));

	j_if = (
		J_IF(false, (J(a) = 2))
		);
	_LOG("J_IF: " << rt::JsonBeautified(j_if));

	j_if = (
		J(b) = 3,
		J_IF(false, (J(a) = 2)),
		J(d) = 4
		);
	_LOG("J_IF: " << rt::JsonBeautified(j_if));

	j_if = (
		J(b) = 3,
		J_IF(false, (J(a) = 2, J(c) = 3)),
		J(d) = 4,
		J_IF(true, (J(f) = 6, J(g) = 7)),
		J(e) = 5
		);
	_LOG("J_IF: " << rt::JsonBeautified(j_if));
}
void rt::UnitTests::json()
{
	json_GetKeyValue();
	json_JsonBeautified();
	json_JArray();
	json_GetNextObjectRaw();
	json_EscapeString();
	json_Output();
	json_AppendKey();
	json_J_IF();
}