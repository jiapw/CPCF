#pragma once

//////////////////////////////////////////////////////////////////////
// Cross-Platform Core Foundation (CPCF)
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
//     * Neither the name of CPCF.  nor the names of its
//       contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//////////////////////////////////////////////////////////////////////

#include "string_type.h"

//////////////////////////////////////////////////////////////////////////////
// Composing Json Data
namespace rt
{
	
class _JObj
{
	rt::String		__Obj;
	rt::String_Ref	_Ref;
	bool			_AsBinary;
	char			_BinLen[10];
	UINT			_BinLenSize;
	void			_SetBinLen(){ _BinLenSize = rt::string_ops::itoa((UINT)_Ref.GetLength(), _BinLen); }
public:
	enum _tagVARTYPE
	{	VARTYPE_BUILTIN = 0,
		VARTYPE_STRING  = 1,
		VARTYPE_ARRAY   = 2,
		VARTYPE_OBJECT  = 3
	};
	FORCEINL			_JObj(){ static rt::SS n("null"); _Ref = n; }
	FORCEINL explicit	_JObj(LPCSTR str, bool bin = false)
						{	_AsBinary = bin;
							if(bin){ _Ref = rt::String_Ref(str); _SetBinLen(); }
							else _Ref = rt::String_Ref(str).TrimSpace();
						}
	FORCEINL explicit	_JObj(const rt::String_Ref& str, bool bin = false)
						{	_AsBinary = bin;
							if(bin){ _Ref = str; _SetBinLen(); }
							else _Ref = str.TrimSpace();
						}
	FORCEINL explicit	_JObj(const rt::String& str, bool bin = false)
						{	_AsBinary = bin;
							if(bin){ _Ref = str; _SetBinLen(); }
							else _Ref = str.TrimSpace();
						}
	FORCEINL explicit	_JObj(LPSTR str, bool bin = false)
						{	_AsBinary = bin;
							if(bin){ _Ref = rt::String_Ref(str); _SetBinLen(); }
							else _Ref = rt::String_Ref(str).TrimSpace();
						}
	template <template<class, class, class> class std_string, class _Traits, class _Alloc>
	FORCEINL explicit	_JObj(const std_string<char, _Traits, _Alloc>& str, bool bin = false)
						{	_AsBinary = bin;
							if(bin){ _Ref = str; _SetBinLen(); }
							else _Ref = rt::String_Ref(str).TrimSpace();
						}
	FORCEINL explicit	_JObj(rt::String_Ref& str, bool bin = false)
						{	_AsBinary = bin;
							if(bin){ _Ref = str; _SetBinLen(); }
							else _Ref = str.TrimSpace();
						}
	FORCEINL explicit	_JObj(rt::String& str, bool bin = false)
						{	_AsBinary = bin;
							if(bin){ _Ref = str; _SetBinLen(); }
							else _Ref = str.TrimSpace();
						}
	template<typename T>
	FORCEINL explicit	_JObj(T& j, bool bin = false)
						{	_AsBinary = bin;
							j.ToString(__Obj);
							if(bin){ _Ref = __Obj; _SetBinLen(); }
							else _Ref = __Obj.TrimSpace();
						}
	FORCEINL UINT		GetLength() const 
						{	return _AsBinary?(2 + _BinLenSize + (UINT)_Ref.GetLength() + 1):
											 (_Ref.IsEmpty()?4:(UINT)_Ref.GetLength());
						}
	FORCEINL UINT		CopyTo(LPSTR p) const 
						{	if(_AsBinary)
							{	ASSERT(_Ref.GetLength() <= 0xffffff);
								*p = '<';
								memcpy(p+1, _BinLen, _BinLenSize);
								p += 2 + _BinLenSize;
								p[-1] = '/';
								_Ref.CopyTo(p);
								p[_Ref.GetLength()] = '>';
								return 2 + _BinLenSize + (UINT)_Ref.GetLength() + 1;
							}
							else
							{	if(_Ref.IsEmpty())
								{	*(DWORD*)p = 0x6c6c756e;
									return 4;
								}
								else return (UINT)_Ref.CopyTo(p);
							}
						}
};

namespace _details
{
	template<typename T>
	FORCEINL void _json_GetLengthPrev(T& n, UINT& l){ n._GetLength(l); }
	FORCEINL UINT _json_GetLengthPrev(LPVOID, UINT&){ return 0; }

	template<typename T>
	FORCEINL void _json_CopyPrev(T& n, LPSTR p, UINT& l){ n._CopyTo(p, l); }
	FORCEINL UINT _json_CopyPrev(LPVOID, LPSTR, UINT&){ return 0; }
} // namespace _details

template<typename t_Prev, typename t_Val>
struct _JVar
{
	t_Prev			prev;
	rt::String_Ref	tagname;
	t_Val			value;
	DWORD			value_type;
	
	FORCEINL void	_GetLength(UINT& len) const
					{	_details::_json_GetLengthPrev(prev, len);
						if(!IsGhost())
						{	if(len)len++;
							len += (UINT)tagname.GetLength() + 3;
							len += (UINT)value.GetLength() + (_JObj::VARTYPE_STRING == value_type?2:0);
						}
					}
	FORCEINL void	_CopyTo(LPSTR p, UINT& len) const
					{	_details::_json_CopyPrev(prev, p, len);
						if(!IsGhost())
						{	if(len)
							{	len++;	p += len;
								*(p-1) = ',';
							}
							int l;
							*p++='"'; p += (l = (UINT)tagname.CopyTo(p)); *p++='"';
							*p++=':'; len += l + 3;
							if(_JObj::VARTYPE_STRING == value_type)
							{	*p++='"';
								p += (l = (UINT)value.CopyTo(p));
								*p++='"';
								len += l + 2;
							}
							else{ len += (UINT)value.CopyTo(p); }
						}
					}
	template<typename t_V>
	FORCEINL _JVar(const rt::String_Ref& name, t_V& v, DWORD vt):tagname(name), value(v), value_type(vt){}
	FORCEINL _JVar(){}
	template<typename Prev, typename t_V>
	FORCEINL _JVar(Prev& n, const rt::String_Ref& name,t_V& v, DWORD vt):tagname(name), value(v), value_type(vt), prev(n){}

	FORCEINL bool	IsGhost() const { return tagname.IsEmpty(); }
	FORCEINL UINT	GetLength() const
					{	UINT len = 0;	_GetLength(len);
						if(len)return len + 2;
						return 2;
					}
	FORCEINL UINT	CopyTo(LPSTR p) const
					{	*p++ = '{';
						UINT len = 0;
						_CopyTo(p, len);
						if(len){ p+=len; *p = '}'; return len + 2; }
						*p = '}';
						return 2;
					}
	FORCEINL void	ToString(rt::String& out) const
					{	VERIFY(out.SetLength(GetLength()));
						out.SetLength(CopyTo(out.Begin()));
					}
	// override operator , for connecting key-value pairs in declaring json object
	template<typename t_right>
	FORCEINL auto operator , (const _JVar<LPVOID, t_right>& right) const
	{	return _JVar< _JVar<t_Prev,t_Val>, t_right>((const _JVar<t_Prev,t_Val>&)*this, right.tagname, right.value, right.value_type);
	}
};

template<typename t_String = rt::StringFixed<1024>>  // or StringFixed<LEN>
class _JArray
{
	t_String	_buf;

	template<typename T>
	FORCEINL void	_Append(const T& obj, bool need_quote)
					{	if(_buf.GetLength() != 1){ _buf += ',';	}
						if(need_quote){	_buf += '"'; _buf += obj; _buf += '"'; }
						else{ _buf += obj; }
					}
public:
	FORCEINL _JArray(){ _buf = "["; }

#define JA_NUMERIC_ITEM(numeric_type)	FORCEINL void Append(numeric_type x){ _Append(rt::tos::Number(x), false); }
	JA_NUMERIC_ITEM(bool)
	JA_NUMERIC_ITEM(short)
	JA_NUMERIC_ITEM(unsigned short)
	JA_NUMERIC_ITEM(int)
	JA_NUMERIC_ITEM(UINT)
#if defined(PLATFORM_WIN)
	JA_NUMERIC_ITEM(long)
	JA_NUMERIC_ITEM(unsigned long)
#endif
	JA_NUMERIC_ITEM(LONGLONG)
	JA_NUMERIC_ITEM(ULONGLONG)
	JA_NUMERIC_ITEM(float)
	JA_NUMERIC_ITEM(double)
#undef JA_NUMERIC_ITEM
		
	FORCEINL void	Append(LPCSTR x){ _Append(rt::String_Ref(x), true); }
	FORCEINL void	Append(LPSTR x){ _Append(rt::String_Ref(x), true); }
	FORCEINL void	Append(char x){ _Append(rt::String_Ref(&x, 1), true); }
	template<typename T>
	FORCEINL void	Append(const T& json){ _Append(json, std::is_base_of<rt::String_Ref, T>::value); }

	FORCEINL UINT	GetLength() const { return (UINT)(_buf.GetLength() + 1); }
	FORCEINL UINT	CopyTo(LPSTR p) const 
					{	_buf.CopyTo(p);
						p[_buf.GetLength()] = ']';
						return (UINT)(_buf.GetLength()+1);
					}
};

#define J_EXPR_CONNECT_OP(type, type_in, vt)					\
FORCEINL _JVar<LPVOID, type>									\
operator = (type_in p)											\
{	if(tagname.IsEmpty())										\
	{	static type __empty_p; 									\
		return _JVar<LPVOID, type>(tagname, __empty_p, vt); 	\
	} else { return _JVar<LPVOID, type>(tagname, p, vt); }		\
}																\

struct _JTag
{	
	template<typename t_Obj>
	struct _O
	{	const t_Obj* _Obj;
		FORCEINL UINT	GetLength() const { ASSERT(_Obj); return (UINT)_Obj->GetLength(); }
		FORCEINL UINT	CopyTo(LPSTR p) const { ASSERT(_Obj); return (UINT)_Obj->CopyTo(p); }
		FORCEINL		_O(const t_Obj& x):_Obj(&x){}
		FORCEINL		_O(){ _Obj = nullptr; }
	};

	rt::String_Ref	tagname;
	FORCEINL _JTag(const rt::String_Ref& name):tagname(name){}

	J_EXPR_CONNECT_OP(String_Ref,					const String_Ref&	, _JObj::VARTYPE_STRING)
	J_EXPR_CONNECT_OP(String_Ref,					const char*			, _JObj::VARTYPE_STRING)
	J_EXPR_CONNECT_OP(tos::S_<MARCO_CONCAT(1,2)>,	char				, _JObj::VARTYPE_STRING)
	J_EXPR_CONNECT_OP(tos::S_<MARCO_CONCAT(1,12)>,	int					, _JObj::VARTYPE_BUILTIN)
	J_EXPR_CONNECT_OP(tos::S_<MARCO_CONCAT(1,7)>,	bool				, _JObj::VARTYPE_BUILTIN)
	J_EXPR_CONNECT_OP(tos::S_<MARCO_CONCAT(1,12)>,	unsigned int		, _JObj::VARTYPE_BUILTIN)
	J_EXPR_CONNECT_OP(tos::S_<MARCO_CONCAT(1,25)>,	LONGLONG			, _JObj::VARTYPE_BUILTIN)
	J_EXPR_CONNECT_OP(tos::S_<MARCO_CONCAT(1,25)>,	ULONGLONG			, _JObj::VARTYPE_BUILTIN)
#if defined(PLATFORM_WIN)
	J_EXPR_CONNECT_OP(tos::S_<MARCO_CONCAT(1,12)>,	long				, _JObj::VARTYPE_BUILTIN)
	J_EXPR_CONNECT_OP(tos::S_<MARCO_CONCAT(1,12)>,	unsigned long		, _JObj::VARTYPE_BUILTIN)
#endif
	J_EXPR_CONNECT_OP(tos::S_<MARCO_CONCAT(1,32)>,	float				, _JObj::VARTYPE_BUILTIN)
	J_EXPR_CONNECT_OP(tos::S_<MARCO_CONCAT(1,64)>,	double				, _JObj::VARTYPE_BUILTIN)
	J_EXPR_CONNECT_OP(_O<_JObj>,					const _JObj& 		, _JObj::VARTYPE_OBJECT)

	template<typename prev, typename type>
	FORCEINL auto operator = (const _JVar<prev, type>& p)
	{	return _JVar<LPVOID, _O<const _JVar<prev, type>> >(tagname, p, _JObj::VARTYPE_OBJECT);
	}

	template<typename T>
	FORCEINL auto operator = (const _JArray<T>& p)
	{	return _JVar<LPVOID, _O<const _JArray<T>> >(tagname, p, _JObj::VARTYPE_ARRAY);
	}

	template<typename t_Left, typename t_Right>
	FORCEINL auto operator = (const _SE<t_Left,t_Right>& p)
	{	return _JVar<LPVOID, _O<const _SE<t_Left,t_Right>> >(tagname, p, _JObj::VARTYPE_STRING);
	}
};

#undef J_EXPR_CONNECT_OP


class _JObject
{
	rt::String	__Temp;
	rt::String& _Save;
public:
	_JObject():_Save(__Temp){ __Temp = "{}"; }
	_JObject(rt::String& out):_Save(out)
	{	if(out.IsEmpty()){ out = "{}"; }
		_Save = _Save.TrimSpace();
		ASSERT(_Save.Last() == '}');
	}
	template<typename t_left, typename t_right>
	void operator << (const _JVar<t_left,t_right>& json)
	{
		if(_Save.GetLength() <= 2)
		{
			_Save = json;
		}
		else
		{	_Save._len--;
			SIZE_T e = _Save.GetLength();
			_Save += json;
			_Save.Begin()[e] = ',';
		}
	}
	const rt::String_Ref& GetString() const { return _Save; }
	operator const rt::String_Ref&() const { return _Save; }
};

template<typename LEFT, typename T>
FORCEINL _SE<String_Ref, _JVar<LEFT, T>>  operator + (const String_Ref& left, const _JVar<LEFT, T>& right)
{	return _SE<String_Ref, _JVar<LEFT, T>> ( (left), (right) );
}
template<typename t_Left, typename t_Right, typename LEFT, typename T>
FORCEINL _SE<_JVar<LEFT, T>, _SE<t_Left,t_Right> >								
operator + (const _JVar<LEFT, T>& p, const _SE<t_Left,t_Right>& x)	
{	return _SE<_JVar<LEFT, T>, _SE<t_Left,t_Right>>(p,x);
}
template<typename t_Left, typename t_Right, typename LEFT, typename T>
FORCEINL _SE<_SE<t_Left,t_Right> , _JVar<LEFT, T>>
operator + (const _SE<t_Left,t_Right>& x, const _JVar<LEFT, T>& p)	
{	return _SE<_SE<t_Left,t_Right>, _JVar<LEFT, T>>(x, p);
}

} // namespace rt

#define J(x)					rt::_JTag(#x)
#define J_IF(cond, definition)	((cond) ? (definition) : decltype(definition)())
#define JB(x)					(rt::_JObj((x), true))
#define JKSTR(x)				(rt::SS("\"" #x "\":"))

template<typename T>
FORCEINL void _JA(rt::_JArray<>& a, T&& t) {
	a.Append(t);
}
template<typename T, typename... Ts>
FORCEINL void _JA(rt::_JArray<>& a, T&& t, Ts&& ... args) {
	a.Append(t);
	_JA(a, args...);
}

template<typename... Ts>
FORCEINL rt::_JArray<> JA(Ts&& ... args) {
	rt::_JArray<> a;
	_JA(a, args...);
	return a;
}
template<> rt::_JArray<> FORCEINL JA() { return {}; }


//////////////////////////////////////////////////////////////////////////////
// Parsing Json Data
namespace rt
{

typedef enum _tagJsonType
{
	JSON_STRING,
	JSON_NUMBER,
	JSON_BOOL,
	JSON_NULL,
	JSON_OBJECT,
	JSON_ARRAY,
	JSON_BINARY,
	JSON_CORRUPTED
} JsonType;


class JsonKeyValuePair
{
	friend class JsonObject;
	rt::String_Ref	_Key;
	rt::String_Ref	_Value;

public:
	static JsonType GetValueType(const rt::String_Ref& Value);

public:
	const rt::String_Ref&	GetKey() const { return _Key; }
	const rt::String_Ref&	GetValue() const { return _Value; }
	rt::String_Ref			GetValueRaw() const;
	JsonType				GetValueType() const { return GetValueType(_Value); }
	void					Empty(){ _Key.Empty(); _Value.Empty(); }
	bool					IsEmpty() const { return _Key.IsEmpty() || _Value.IsEmpty(); }

	template<typename T>
	INLFUNC T				GetValueAs() const { return GetValueAs<T>(0); }
	template<typename T>
	INLFUNC T				GetValueAs(T default_val) const
							{	rt::String_Ref s = GetValue();
								if(!s.IsEmpty())s.ToNumber(default_val); 
								return default_val;
							}
};
	
template<>
INLFUNC bool JsonKeyValuePair::GetValueAs<bool>(bool default_val) const
{	rt::String_Ref s = GetValue();
	if(!s.IsEmpty()){ return s[0] == 'T' || s[0] == 't'; }
	return default_val;
}

class JsonObject
{
	friend class JsonArray;
	friend class JsonBeautified;
	rt::String_Ref	_Doc;
protected:
	static const INLFUNC LPCSTR _skip_whitespace(LPCSTR p, LPCSTR end){ while(p < end && *p<=' ' && *p>=0)p++; return p; }
	static const INLFUNC LPCSTR _scan_text(LPCSTR p, LPCSTR end){ while(p < end && _is_text(*p))p++; return p; }
	static const INLFUNC LPCSTR _seek_char(LPCSTR p, LPCSTR end, char c){ while(p < end && *p!=c)p++; return p; }
	static const INLFUNC void	_cook_raw_value(LPCSTR& p, LPCSTR& tail)
								{	if(*p == '"' || *p == '\''){ p++; tail--; } // string
									else if(*p == '<'){	p+=2; while(*p != '/'){ p++; ASSERT(p<tail); } p++;	tail--; }  // binary
								}
	static const INLFUNC bool   _is_text(int c)
								{	static const rt::CharacterSet sep(rt::SS("\x7\x9\xa\xb\xc\xd '\",:{}[]"));
									return !sep.Has(c); // c<0 || ( c>' ' && c!='\'' && c!='"' && c!=',' && c!=':' && c!='{' && c!='}'); 
								}
	static const INLFUNC LPCSTR _seek_char_escape(LPCSTR p, LPCSTR end, char c)
								{	for(;p < end;p++)
									{	if(*p==c)
										{	LPCSTR s = p-1;
											for(;*s == '\\';s--);
											if(((int)(s-p))&1)return p;
										}
									}
									return end;
								}
	static const INLFUNC int   _match_closure(int c)
								{	if(c == '"' || c == '\'')return c;
									else if(c == '[')return ']';
									else if(c == '{')return '}';
									else return 0;
								}
	static int					_count_seps(const rt::String_Ref& doc);
	static LPCSTR				_seek_json_object_closure(LPCSTR p, LPCSTR end);
	LPCSTR						_LocateValue(const rt::String_Ref& xpath, bool bDoNotSplitDot = false) const;

public:
	JsonObject(){};
	JsonObject(LPCSTR str, UINT length){ Attach(str, length); }
	JsonObject(const rt::String_Ref& doc){ Attach(doc); }

	const rt::String_Ref& GetString() const { return _Doc; }
	operator const rt::String_Ref () const { return _Doc; }

	void			Attach(LPCSTR str, UINT length){ Attach(rt::String_Ref(str, length)); }
	void			Attach(const rt::String_Ref& doc){	SSIZE_T s = doc.FindCharacter('{');	if(s>=0)_Doc = doc.SubStr(s); }
	rt::String_Ref	GetValue(const rt::String_Ref& xpath, const rt::String_Ref& default_val = rt::String_Ref(), bool bDoNotSplitDot = false) const;	// xxx.yyy.zzz
	rt::String_Ref	GetValue(const rt::String_Ref& xpath, bool& p_exist, bool bDoNotSplitDot = false) const;	// xxx.yyy.zzz
	bool			IsEmpty() const { return _Doc.IsEmpty(); }
	bool			IsEmptyObject() const;
	bool			GetNextKeyValuePair(JsonKeyValuePair& kvp) const;

	template<typename T>
	T				GetValueAs(const rt::String_Ref& xpath, T default_val) const
					{	rt::String_Ref s = GetValue(xpath);
						if(!s.IsEmpty())s.ToNumber(default_val);
						return default_val;
					}
	template<typename T>
	T				GetValueAs(const rt::String_Ref& xpath) const { return GetValueAs<T>(xpath, 0); }

	template<typename T1,typename T2>
	void			Derive(const rt::_JVar<T1,T2>& sub, rt::String& derived, bool append = false) const { Override(_Doc, sub, derived, append); }
	void			Derive(const rt::String_Ref& sub, rt::String& derived, bool append = false) const { Override(_Doc, sub, derived, append); }
	void			Derive(const rt::String_Ref& key, const rt::String_Ref& val_raw, rt::String& derived, bool append = false) const { Override(_Doc, key, val_raw, derived, append); }

	template<typename T1,typename T2>
	static void		Override(const rt::String_Ref& base, const rt::_JVar<T1,T2>& sub, rt::String& derived, bool append = false){ Override(base, rt::String(sub), derived, append); }
	static void		Override(const rt::String_Ref& base, const rt::String_Ref& sub, rt::String& derived, bool append = false);
	static void		Override(const rt::String_Ref& base, const rt::String_Ref& key, const rt::String_Ref& val_raw, rt::String& derived, bool append = false);
	static void		RemoveKeys(const rt::String_Ref& source, const rt::String_Ref& keys_to_exclude, rt::String& removed);

	static void		UnescapeStringValue(const rt::String_Ref& in, rt::String& val_out);
	void			GetValueUnescaped(rt::String& val_out, const rt::String_Ref& xpath, const rt::String_Ref& default_val = rt::String_Ref(), bool bDoNotSplitDot = false) const	// xxx.yyy.zzz
					{	rt::String_Ref in = GetValue(xpath, default_val, bDoNotSplitDot);
						UnescapeStringValue(in, val_out);
					}
};
template<>
INLFUNC bool JsonObject::GetValueAs<bool>(const rt::String_Ref& xpath, bool default_val) const
{	rt::String_Ref s = GetValue(xpath);
	if(!s.IsEmpty()){ return s[0] != 'F' && s[0] != 'f' && s[0] != '0'; }
	return default_val;
}

class JsonArray
{
	rt::String_Ref	_Doc;
public:
	JsonArray(const rt::String_Ref& doc);
	bool IsEmpty() const { return _Doc.IsEmpty(); }
	bool GetNextObjectRaw(JsonObject& obj) const { return GetNextObjectRaw(obj._Doc); }
	bool GetNextObjectRaw(rt::String_Ref& obj) const;
	bool GetNextObject(rt::String_Ref& obj) const;
	SIZE_T GetSize() const;

	template<typename t_Num>
	INLFUNC UINT CopyNumbers(t_Num* pDest, SIZE_T DestLen) const // return actual number of numbers copied
	{	
		SIZE_T i = 0;
		rt::String_Ref item;
		for(; i<DestLen; i++)
		{	if(GetNextObject(item))
			{	item.ToNumber(pDest[i]);
			}else break;
		}
		return (UINT)i;
	}
};

class JsonEscapeString: public rt::String
{
public:
	JsonEscapeString(const rt::String_Ref& c_string, bool add_quote = false);
	static void Concat(const rt::String_Ref& input, rt::String& out);
};

class JsonUnescapeString: public rt::String
{
public:
	JsonUnescapeString(const rt::String_Ref& json_string)
	{	
		rt::JsonObject::UnescapeStringValue(json_string, *this);
	}
};

// increamental json composing
class Json
{
	rt::String		_String;

protected:
	char&	Last(){ return _String.Last(); }
	char	Last() const { return _String.Last(); }

public:
	class _Appending
	{	protected:	Json* _pJson;
		public:		_Appending(Json& j):_pJson(&j){}
					operator Json& (){ return *_pJson; }
					template<typename T>
					void operator << (const T& json_str){ *_pJson << json_str; }
	};
	struct _AppendingKeyedValue: public _Appending
	{	friend class Json;
		Json& operator ->(){ return *_pJson; }
		_AppendingKeyedValue(_AppendingKeyedValue&& x):_Appending(x){ x._pJson = nullptr; }
		~_AppendingKeyedValue()
		{
			if(_pJson)_pJson->_String += '}';
		}
	protected:
		_AppendingKeyedValue(Json& j, const rt::String_Ref& key):_Appending(j)
		{	ASSERT(j.Last() == '}');
			j._String.Shorten(1);
			if(j.Last() != '{')j._String += ',';
			j._String += '"'; JsonEscapeString::Concat(key, j._String); j._String+= '"'; j._String+=':';
		}
	};
	struct _MergingObject: public _Appending
	{	friend class Json;
		Json& operator ->(){ return *_pJson; }
		SIZE_T	_StartPos;
		_MergingObject(_MergingObject&& x):_Appending(x){ _StartPos = x._StartPos; x._pJson = nullptr; }
		~_MergingObject()
		{	if(_pJson)
			{	if(_StartPos != INFINITE){ ASSERT(_pJson->_String[_StartPos] == '{'); _pJson->_String[_StartPos] = ','; }
				ASSERT(_pJson->Last() == '}');
			}
		}
	protected:
		_MergingObject(Json& j):_Appending(j)
		{	ASSERT(j.Last() == '}');
			j._String.Shorten(1);
			if(j.Last() == '{'){ _StartPos = INFINITE; j._String.Shorten(1); }
			else _StartPos = j._String.GetLength();
		}
	};
	struct _AppendingElement: public _Appending
	{	friend class Json;
		Json& operator ->(){ return *_pJson; }
		_AppendingElement(_AppendingElement&& x):_Appending(x){ x._pJson = nullptr; }
		~_AppendingElement()
		{
			if(_pJson)_pJson->_String += ']';
		}
	protected:
		_AppendingElement(Json& j):_Appending(j)
		{	ASSERT(j.Last() == ']');
			j._String.Shorten(1);
			if(j._String.Last() != '[')j._String += ',';
		}
	};
	struct _AppendingArray: public _Appending
	{	friend class Json;
		Json& operator ->(){ return *_pJson; }
		SIZE_T	_StartPos;
		_AppendingArray(_AppendingArray&& x):_Appending(x){ _StartPos = x._StartPos; x._pJson = nullptr; }
		~_AppendingArray()
		{	if(_pJson)
			{	if(_StartPos != INFINITE){ ASSERT(_pJson->_String[_StartPos] == '['); _pJson->_String[_StartPos] = ','; }
				ASSERT(_pJson->Last() == ']');
			}
		}
	protected:
		_AppendingArray(Json& j):_Appending(j)
		{	ASSERT(j.Last() == ']');
			j._String.Shorten(1);
			if(j.Last() == '['){ _StartPos = INFINITE; j._String.Shorten(1); }
			else _StartPos = j._String.GetLength();
		}
	};
	struct _WritingStringEscaped: public _Appending
	{	friend class Json;
		_WritingStringEscaped(_AppendingArray&& x):_Appending(x){ x._pJson = nullptr; }
		~_WritingStringEscaped()
		{
			if(_pJson)_pJson->_String += '"';
		}
		operator	rt::String& (){ return _pJson->_String; }
		auto&		String(){ return _pJson->_String; }
	protected:
		_WritingStringEscaped(Json& j):_Appending(j)
		{	
			if(j.Last() == '"'){ j._String.Shorten(1); }
			else j._String += '"';
		}
	};
	struct _WritingStringAtKey: public _Appending
	{	friend class Json;
		Json& operator ->(){ return *_pJson; }
		_WritingStringAtKey(_AppendingKeyedValue&& x):_Appending(x){ x._pJson = nullptr; }
		~_WritingStringAtKey()
		{
			if(_pJson){ _pJson->_String += '"'; _pJson->_String += '}'; }
		}
		operator	rt::String& (){ return _pJson->_String; }
		auto&		String(){ return _pJson->_String; }
	protected:
		_WritingStringAtKey(Json& j, const rt::String_Ref& key):_Appending(j)
		{	ASSERT(j.Last() == '}');
			j._String.Shorten(1);
			if(j.Last() != '{')j._String += ',';
			j._String += '"'; JsonEscapeString::Concat(key, j._String); j._String+= '"'; j._String+=':'; j._String+='"';
		}
	};

public:
	operator const rt::String&(){ return _String; }
	SIZE_T	GetLength() const { return _String.GetLength(); }
	SIZE_T	CopyTo(char* p) const { return _String.CopyTo(p); }
	LPCSTR	Begin() const { return _String.Begin(); }

	auto&	Empty(){ _String.Empty(); return *this; }
	auto&	Object(){ _String += '{'; _String += '}'; return *this; }
	auto&	Array(){ _String += '['; _String += ']'; return *this; }

	template<typename T>
	void	Object(const T& json_str){ _String += json_str; }

	// Object Operation
	auto	ScopeAppendingKey(const rt::String_Ref& key){ return _AppendingKeyedValue(*this, key); }
	auto	ScopeMergingObject(){ return _MergingObject(*this); }
	template<typename P, typename V>
	auto&	AppendKey(const rt::String_Ref& key, const _JVar<P,V>& json_str){ _AppendingKeyedValue(*this, key)._pJson->_String += json_str;	return *this; }
	template<typename P, typename V>
	auto&	MergeObject(const _JVar<P,V>& json_str){ _MergingObject(*this)._pJson->_String += json_str; return *this; }

	// Array Operation
	auto	ScopeAppendingElement(){ return _AppendingElement(*this); }
	auto	ScopeMergingArray(){ return _AppendingArray(*this); }

	template<typename P, typename V>
	auto&	AppendElement(const _JVar<P,V>& json){ ASSERT(Last() == ']'); _AppendingElement(*this)._pJson->_String += json; return *this; }
	auto&	AppendElement(const rt::tos::Number& str){ ASSERT(Last() == ']'); _AppendingElement(*this)._pJson->_String += str; return *this; }
	auto&	AppendElement(const rt::String_Ref& str)
			{	ASSERT(Last() == ']');
				auto scope = _AppendingElement(*this);
				_String += '"'; _String += str; _String += '"';
				return *this; 
			}
	template<typename L, typename R>
	auto&	AppendElement(const _SE<L, R>& str)
			{	ASSERT(Last() == ']');
				auto scope = _AppendingElement(*this);
				_String += '"'; _String += str; _String += '"';
				return *this; 
			}

	template<typename T>
	auto&	MergeArray(){ _AppendingArray(*this)._pJson->_String += json_str; return *this; }

	template<typename P, typename V>
	auto&	operator << (const _JVar<P,V>& json)	// merge object or append array element
			{	
				if(Last() == '}') // merge with another object
				{	
					MergeObject(json);
				}
				else // append an element
				{
					AppendElement(json);
				}
				return *this;
			}

	auto&	operator << (const rt::String_Ref& s){ return AppendElement(s); }
	auto&	operator << (LPCSTR s){ return AppendElement(rt::String_Ref(s)); }
	auto&	operator << (char c){ return AppendElement(rt::String_Ref(&c, 1)); }
	auto&	operator << (short x){ return AppendElement(rt::tos::Number(x)); }
	auto&	operator << (WORD x){ return AppendElement(rt::tos::Number(x)); }
	auto&	operator << (int x){ return AppendElement(rt::tos::Number(x)); }
	auto&	operator << (UINT x){ return AppendElement(rt::tos::Number(x)); }
	auto&	operator << (LONGLONG x){ return AppendElement(rt::tos::Number(x)); }
	auto&	operator << (ULONGLONG x){ return AppendElement(rt::tos::Number(x)); }
	auto&	operator << (bool x){ return AppendElement(rt::tos::Number(x)); }
	auto&	operator << (float x){ return AppendElement(rt::tos::Number(x)); }
	auto&	operator << (double x){ return AppendElement(rt::tos::Number(x)); }
	template<typename L, typename R>
	auto&	operator <<(const _SE<L, R>& str){ return AppendElement(str); }

	template<typename T>
	void	Number(T n){ _String += rt::tos::Number(n); }
	void	Null(){ _String += rt::SS("null"); }
	void	Boolean(bool b){ _String += b?rt::SS("true"):rt::SS("false"); }

	void	String(const rt::String_Ref& s){ _String += '"'; JsonEscapeString::Concat(s, _String); _String += '"'; }
	void	StringEscaped(const rt::String_Ref& s){ _String += '"'; _String += s; _String += '"'; }
	auto	ScopeWritingStringEscaped(){ return _WritingStringEscaped(*this); }
	auto	ScopeWritingStringEscapedAtKey(const rt::String_Ref& key){ return _WritingStringAtKey(*this, key); }
	LPSTR	WriteStringEscaped(SIZE_T len)
			{	_String += '"';
				SIZE_T org = _String.GetLength();
				_String.SetLength(org + len + 1);
				_String.Last() = '"';
				return &_String[org];
			}
};

class JsonBeautified: public rt::String
{
	int _Indent;
	int _Line_max;

	void _AppendSpace(UINT count);
	void _AppendAsSingleLine(const rt::String_Ref& doc);
	void _Beautify(const rt::String_Ref& json_string, bool newline, int depth, int line_remain);

public:
	JsonBeautified(){}
	JsonBeautified(const rt::String_Ref& json_string, int indent = 3, int line_remain = 80){ Beautify(json_string, indent, line_remain); }
	void Beautify(const rt::String_Ref& json_string, int indent = 3, int line_remain = 80);
};


template<typename TJSON>
INLFUNC rt::String& JSON_OBJECT_APPEND(rt::String& x, const rt::String_Ref& key, TJSON&& value)
{	static const rt::SS  _sep("\":");
	if(x.IsEmpty()){ x = rt::SS("{\"") + key + _sep + value + '}'; }
	else
	{	ASSERT(x.Last() == '}');
		x.Last() = ',';
		x += rt::SS() + '"' + key + _sep + value + '}';
	}

	return x;
}

template<typename TJSON>
INLFUNC rt::String& JSON_OBJECT_MERGE(rt::String& x, TJSON&& json)
{	static const rt::SS  _sep("\":");
	if(x.GetLength()<5 || x.TrimSpace().TrimLeft(1).TrimRight(1).TrimSpace().IsEmpty()){ x = json; }
	else
	{	ASSERT(x.Last() == '}');
		x.Shorten(1);
		UINT len = (UINT)x.GetLength();
		x += json;
		if(x[len+1] != '}'){ x[len] = ','; }
		else { x[len] = '}'; x.SetLength(len+1); }
	}

	return x;
}


} // namespace rt

