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

#include "runtime_base.h"
#include "type_traits.h"
#include "string_type_op.h"

#ifdef PLATFORM_LINUX
#include <cstring>
#endif

namespace rt
{
namespace _details
{
	template<typename T>
	INLFUNC T		_pow(T x, int y){ return (T)pow((double)x,y); }
	INLFUNC float	_pow(float x, int y){ return pow(x,y); }

	template<typename t_Left, typename t_Right>
	class String_AddExpr;

	template<int BASE, bool base_LE_10 = (BASE<=10)>
	struct _to_num
	{	static INLFUNC int call(int x){ return x>='0'?(x-'0'):BASE; }
	};
	template<int BASE>
	struct _to_num<BASE, false>
	{	static INLFUNC int call(int x)
		{	if(x<='9' && x>='0'){ return x-'0';	}
			else if(x<='A'+BASE && x>='A'){	return x-'A'+10; }
			else if(x<='a'+BASE && x>='a'){	return x-'a'+10; }
			else return BASE;
		}	
	};

} // namespace rt::_details
} // namespace rt


namespace rt
{

class CharacterSet
{
protected:
	char	_Set[127]; // model 1 to 127
public:
	FORCEINL void Init(LPCSTR p, LPCSTR mapped, UINT len)
	{	rt::Zero(_Set);
		for(UINT i=0;i<len;i++)
		{	ASSERT(p[i]>=1);
			_Set[p[i]-1] = mapped[i];
		}
	}
	FORCEINL void Init(LPCSTR p, UINT len){ Init(p,p,len); }
	FORCEINL CharacterSet(){ rt::Zero(_Set); }
	FORCEINL CharacterSet(char c){ rt::Zero(_Set); ASSERT(c>=1); _Set[c-1] = c; }
	FORCEINL CharacterSet(LPCSTR p){ Init(p, (UINT)strlen(p)); }
	template<typename T>
	FORCEINL CharacterSet(const T& s){ Init(s.Begin(), (UINT)s.GetLength()); }
	FORCEINL CharacterSet(LPCSTR p, LPCSTR mapped){ Init(p, mapped, (UINT)strlen(p)); }
	template<typename T>
	FORCEINL CharacterSet(const T& s, const T& mapped){ Init(s.Begin(), mapped.Begin(), (UINT)s.GetLength()); }
	FORCEINL bool Has(int c) const 
	{	if(c>=1)
			return _Set[c-1]; 
		else
			return false;
	}
	FORCEINL int Mapped(int c) const // return 0 for out of the set
	{	if(c>=1 && _Set[c-1])
			return _Set[c-1]; 
		else
			return c;
	}
};

struct CharacterSet_Escape: public CharacterSet	// Escape for C/Json string
{	CharacterSet_Escape():CharacterSet("\b\f\r\n\t\v\'\"\\","bfrntv\'\"\\"){}
};

struct CharacterSet_Unescape: public CharacterSet	// Unescape for C/Json string
{	CharacterSet_Unescape():CharacterSet("bfrntv\'\"\\","\b\f\r\n\t\v\'\"\\"){}
};


struct CharacterSet_ControlChar: public CharacterSet
{	FORCEINL CharacterSet_ControlChar(LPCSTR p)
		:CharacterSet(p)
	{	for(UINT i=1; i<' '; i++)_Set[i-1] = true;
	}
	FORCEINL CharacterSet_ControlChar()
	{	for(UINT i=1; i<' '; i++)_Set[i-1] = true;
	}
};

struct CharacterSet_Alphabet: public CharacterSet
{	FORCEINL CharacterSet_Alphabet(LPCSTR sep_additional = nullptr)
	{	for(int i='a'; i<='z'; i++)_Set[i-1] = true;
		for(int i='A'; i<='Z'; i++)_Set[i-1] = true;
		if(sep_additional)while(*sep_additional)_Set[((int)*sep_additional++)-1] = true;
	}
};

struct CharacterSet_LowerCase: public CharacterSet
{	FORCEINL CharacterSet_LowerCase(LPCSTR sep_additional = nullptr)
	{	for(int i='a'; i<='z'; i++)_Set[i-1] = true;
		if(sep_additional)while(*sep_additional)_Set[((int)*sep_additional++)-1] = true;
	}
};

struct CharacterSet_UpperCase: public CharacterSet
{	FORCEINL CharacterSet_UpperCase(LPCSTR sep_additional = nullptr)
	{	for(int i='A'; i<='Z'; i++)_Set[i-1] = true;
		if(sep_additional)while(*sep_additional)_Set[((int)*sep_additional++)-1] = true;
	}
};

	
struct CharacterSet_Digits: public CharacterSet
{	FORCEINL CharacterSet_Digits()
	{	for(int i='0'; i<='9'; i++)_Set[i-1] = true;
	}
};

struct CharacterSet_Number: public CharacterSet_Digits
{	FORCEINL CharacterSet_Number()
	{	_Set['.'-1] = true;
	}
};


struct CharacterSet_AlphabetDigits: public CharacterSet
{	FORCEINL CharacterSet_AlphabetDigits(LPCSTR sep_additional = nullptr)
	{	for(int i='0'; i<='9'; i++)_Set[i-1] = true;
		for(int i='a'; i<='z'; i++)_Set[i-1] = true;
		for(int i='A'; i<='Z'; i++)_Set[i-1] = true;
		if(sep_additional)while(*sep_additional)_Set[((int)*sep_additional++)-1] = true;
	}
};

struct CharacterSet_Symbol: public CharacterSet_AlphabetDigits		// C/C++ Symbol
{	FORCEINL CharacterSet_Symbol(LPCSTR sep_additional = nullptr):CharacterSet_AlphabetDigits(sep_additional)
	{	_Set[((int)'_')-1] = true;
	}
};

struct CharacterSet_Printable: public CharacterSet_Symbol
{	FORCEINL CharacterSet_Printable():CharacterSet_Symbol(".{}[]|\\\"';:,<>?/-=+`~!@#$%^&*()"){}
};

namespace _details
{
	struct _StringPtrStore
	{	char*	_p;
		SIZE_T	_len;
		FORCEINL bool			IsEmpty() const { return this == nullptr || _p==nullptr || _len<2; }
		FORCEINL void			Empty(){ _p = nullptr; _len = 0; }
	};
	template<SIZE_T SIZE_RESERVED>
	struct _StringArrayStore
	{	char	_p[SIZE_RESERVED+1];
		SIZE_T	_len;
		FORCEINL bool	IsEmpty() const { return this == nullptr || _len<2; }
		FORCEINL void	Empty(){ _len = 0; }
		static const SIZE_T	_len_reserved = SIZE_RESERVED;
	};
};

class String;

template<typename t_StringStore, class t_String_Ref>
class String_Base: public t_StringStore
{	typedef t_StringStore _SC;
public:
	FORCEINL SIZE_T			GetLength() const { return _SC::_len?_SC::_len-1:0; }
	FORCEINL SIZE_T			CopyTo(char* p) const { 
		memcpy(p,_SC::_p,GetLength()*sizeof(char)); 
		return GetLength(); 
	} // terminate-zero is not copied
	FORCEINL SIZE_T			CopyToZeroTerminated(char* p) const { 
		memcpy(p,_SC::_p,GetLength()*sizeof(char)); 
		p[GetLength()] = '\0';
		return GetLength(); 
	} // terminate-zero is copied

	FORCEINL const char*	Begin() const { return _SC::_p; }	// may NOT terminated by ZERO !!
	FORCEINL char*			Begin() { return _SC::_p; }	// may NOT terminated by ZERO !!
	FORCEINL const char*	GetString() const { if(_SC::IsEmpty())return ""; ASSERT(IsZeroTerminated()); return _SC::_p; }
	FORCEINL const char*	End() const { return _SC::_p+GetLength(); }
	FORCEINL char&			Last()		{ ASSERT(GetLength()); return _SC::_p[GetLength()-1]; }
	FORCEINL const char&	Last()const	{ ASSERT(GetLength()); return _SC::_p[GetLength()-1]; }
	FORCEINL char&			First()		{ ASSERT(GetLength()); return _SC::_p[0]; }
	FORCEINL const char&	First()const{ ASSERT(GetLength()); return _SC::_p[0]; }
	FORCEINL bool			IsZeroTerminated() const { return _SC::_p && _SC::_p[GetLength()] == 0; }
	FORCEINL bool			HasTrailingPathSeparator() const { return Last() == '\\' || Last() == '/'; }
	template<typename t_Str>
	FORCEINL void			ToString(t_Str& str){ str = *this; }

	template<typename T>
	FORCEINL const char&	operator [](T i) const { return _SC::_p[i]; }
	template<typename T>
	FORCEINL char&	operator [](T i){ return _SC::_p[i]; }

	template< class StrT >
	FORCEINL bool	operator < (const StrT& x) const
	{	int ret = memcmp(_SC::_p,x.Begin(),min(GetLength(),x.GetLength())*sizeof(char));
		return (ret < 0) || (ret==0 && (GetLength() < x.GetLength()));
	}
	template< class StrT >
	FORCEINL bool	IsReferring(const StrT& x)  // (*this) is a part of x
	{	return Begin() >= x.Begin() && (End() <= x.End());
	}
public:
	FORCEINL t_String_Ref SubStrTail(SIZE_T len) const { return len > GetLength() ? *this : t_String_Ref(_SC::_p+GetLength()-len, len); }
	FORCEINL t_String_Ref SubStrHead(SIZE_T len) const { return t_String_Ref(_SC::_p,rt::min(len,GetLength())); }
	FORCEINL t_String_Ref SubStr(SIZE_T start, SIZE_T len) const { return start < GetLength() ? t_String_Ref(_SC::_p+start,rt::min(len,GetLength()-start)) : nullptr; }
	FORCEINL t_String_Ref SubStr(SIZE_T start) const { return start < GetLength() ? t_String_Ref(_SC::_p+start, GetLength()-start) : nullptr; }
	FORCEINL bool	StartsWith(const t_String_Ref& prefix) const
	{	return	(GetLength() >= prefix.GetLength()) &&
				SubStr(0,prefix.GetLength()) == prefix;
	}
	FORCEINL bool	EndsWith(const t_String_Ref& suffix) const
	{	return	(GetLength() >= suffix.GetLength()) &&
				SubStr(GetLength() - suffix.GetLength(),suffix.GetLength()) == suffix;
	}
	template<typename T>
	FORCEINL SSIZE_T FindString(const T& x_in, SIZE_T start_offset = 0) const
	{	t_String_Ref x(x_in);
		if(x.GetLength())
		{	char ch = x[0];	SSIZE_T i = start_offset;
			while((i = FindCharacter(ch,i)) >=0)
			{	if(x.GetLength() > GetLength() - i)return -1;
				if(memcmp(x.Begin(),_SC::_p + i,x.GetLength()*sizeof(char)) == 0)
					return i;
				else
					i++;
			}
			return -1;
		}
		else return -1;
	}
	FORCEINL t_String_Ref	MakeUpper()
	{	for(SIZE_T i=0;i<GetLength();i++)
			if(_SC::_p[i] >='a' && _SC::_p[i] <='z') _SC::_p[i] -= ('a' - 'A');
		return *this;
	}
	FORCEINL t_String_Ref	MakeLower()
	{	for(SIZE_T i=0;i<GetLength();i++)
			if(_SC::_p[i] >='A' && _SC::_p[i] <='Z') _SC::_p[i] += ('a' - 'A');
		return *this;
	}
	FORCEINL SSIZE_T FindCharacter(char ch, SIZE_T start_offset = 0) const
	{	for(;start_offset<GetLength();start_offset++)
			if(_SC::_p[start_offset] == ch)return start_offset;
		return -1;
	}
	FORCEINL SSIZE_T FindCharacter(const CharacterSet& seps, SIZE_T start_offset = 0) const
	{	for(;start_offset<GetLength();start_offset++)
			if(seps.Has(_SC::_p[start_offset]))return start_offset;
		return -1;
	}
	FORCEINL SSIZE_T FindCharacterReverse(char ch, SIZE_T start_offset = 0) const
	{	SIZE_T l=GetLength(), i=l-1;
		for(;l>start_offset;l--,i--)
			if(_SC::_p[i] == ch)return i;
		return -1;
	}
	FORCEINL t_String_Ref Replace(char a, char b) // replace all a with b in the current string
	{	for(SIZE_T i=0; i<GetLength(); ++i)
			if(_SC::_p[i] == a)_SC::_p[i] = b;
		return *this;
	}
	FORCEINL t_String_Ref Replace(const CharacterSet& seps, char b) // replace all a with b in the current string
	{	for(SIZE_T i=0; i<GetLength(); ++i)
			if(seps.Has(_SC::_p[i]))_SC::_p[i] = b;
		return *this;
	}
	FORCEINL bool HasNonPrintableAscii() const
	{	for(SIZE_T i=0;i<GetLength();i++)
			if(_SC::_p[i] < ' ' || _SC::_p[i] > '~')return true;
		return false;
	}
	FORCEINL bool HasOnlyNumbers() const
	{	for(SIZE_T i=0;i<GetLength();i++)
			if(_SC::_p[i] < '0' || _SC::_p[i] > '9')return false;
		return true;
	}
	FORCEINL void ReplaceIllegalFilenameCharacters(CHAR with = '_') // illegal character for Windows Filename
	{	static const CharacterSet illegal(t_String_Ref("\\/:*?\"><|", 9));
		for(SIZE_T i=0;i<GetLength();i++)
			if(illegal.Has(_SC::_p[i]))_SC::_p[i] = with;
	}
	FORCEINL void ReplaceControlCharacters(CHAR with = '_') // control character like '\r' '\n' '\t'
	{	for(SIZE_T i=0;i<GetLength();i++)
			if((_SC::_p[i] > 0 && _SC::_p[i] < ' ') || _SC::_p[i] == '\xff')_SC::_p[i] = with;
	}
	FORCEINL void NormalizePathSeparator(CHAR with = '_') // replace '\\' with '/'
	{	for(SIZE_T i=0;i<GetLength();i++)
			if(_SC::_p[i] == '\\' || _SC::_p[i] == '/')_SC::_p[i] = with;
	}
	FORCEINL t_String_Ref TrimTrailingPathSeparator() const // remove if last character is '\\' or '/'
	{	if(!_SC::IsEmpty() && (Last() == '\\' || Last() == '/'))return t_String_Ref(_SC::_p, GetLength()-1);
		return *this;
	}
	FORCEINL t_String_Ref RemoveCharacters(const rt::CharacterSet& s)
	{	SIZE_T open = 0;
		LPCSTR p = Begin();	LPCSTR end = End();
		for(;p<end;p++)
			if(!s.Has(*p))_SC::_p[open++] = *p;
		_SC::_len = open+1;
		return *this;
	}
	FORCEINL t_String_Ref Fill(CHAR c = 0)
	{	memset(_SC::_p, c, GetLength());
		return *this;
	}
	FORCEINL bool	operator == (const char* in) const { return *this == t_String_Ref(in); }
	FORCEINL bool	operator == (const t_String_Ref & in) const
	{	if(_SC::IsEmpty() && in.IsEmpty())return true;
		return GetLength() == in.GetLength() && (memcmp(_SC::_p,in.Begin(),GetLength()*sizeof(char))==0);
	}
	template< class StrT >
	FORCEINL bool	operator != (const StrT & in) const { return !(*this == in); }
	FORCEINL t_String_Ref Replace(const t_String_Ref& a, const t_String_Ref& b) // replace all a with b in the current string. a and b should have the same length
	{	SIZE_T len = a.GetLength();
		ASSERT(len == b.GetLength());
        if (len == 0) return *this;
        for (SIZE_T i = 0; i < GetLength() - len; i++)
        {	SIZE_T j = 0;
            for (j = 0; j < len && _SC::_p[i + j] == a[j]; j++);
            if (j == len)
            {
                for (j = 0; j < len; j++)
                {
                    _SC::_p[i + j] = b[j];
                }
                i += len - 1;
	}	}   return *this; }
	FORCEINL int ToNumber(bool& x) const // handles 0,1,T,F,t,f,true,false,TRUE,FALSE
    {	if(_SC::IsEmpty())return 0;
		if(_SC::_p[0] == '1'){ x = true; return 1; }
		if(_SC::_p[0] == '0'){ x = false; return 1; }
		if(_SC::_p[0] == 't'){ x = true; return (0x65757274 == *(DWORD*)_SC::_p)?4:1; }
		if(_SC::_p[0] == 'f'){ x = false; return (0x65736c61 == *(DWORD*)(_SC::_p+1))?5:1; }
		if(_SC::_p[0] == 'T'){ x = true; return (0x45555254 == *(DWORD*)_SC::_p)?4:1; }
		if(_SC::_p[0] == 'F'){ x = false; return (0x45534c41 == *(DWORD*)(_SC::_p+1))?5:1; }
		return 0;
	}
	template<typename T>
	FORCEINL int ToNumber(T& x) const {	return ToNumber<T,10,true,'.',','>(x); }
	template<typename T, int BASE>
	FORCEINL int ToNumber(T& x) const {	return ToNumber<T,BASE,true,'.',','>(x); }
    template<typename T, int BASE, bool ALLOW_LEADING_NON_NUMERIC, char DECIMAL_POINT, char DIGIT_GROUP_SEPARATOR>
	INLFUNC int ToNumber(T& x) const  // [...][+/-/ ]12.34, return the offset where parsing stopped, return -1 if no number found
	{	x = (T)0;
		char *s, *d, *end = _SC::_p + GetLength();
        bool neg = false;
		if(_SC::_p == end) return -1; // string is empty

        // find the first digit
		for (d = _SC::_p; _details::_to_num<BASE>::call(*d)>=BASE && d<end; d++){};
        if (d == end) return -1; // no digit found

        // (if is float) look for possible decimal point before the first digit
		if(rt::NumericTraits<T>::IsFloat)
		{
            if ((d-_SC::_p)>=1 && *(d-1)==(char)DECIMAL_POINT) --d; // set 'd' to decimal point so that x will be 0 and fragment parse will start at this point
		}
        s = d;

        // (if is signed) look for possible signs (+/-)
        if(rt::NumericTraits<T>::IsSigned || rt::NumericTraits<T>::IsFloat)
        {
            if ((s-_SC::_p)>=1 && (*(s-1)==(char)'+' || *(s-1)==(char)'-'))
            {
                if (*(--s)==(char)'-') neg = true;
            }
        }

        // check if there is leading non-numeric
        if(!ALLOW_LEADING_NON_NUMERIC)
        {
            if (s != _SC::_p) return -1; // number doesn't start at position 0
        }

        // parse integral 'x'
		int val = 0;
		while((*d==DIGIT_GROUP_SEPARATOR || (val = _details::_to_num<BASE>::call(*d))<BASE) && d<end)
		{
            if (*d++!=DIGIT_GROUP_SEPARATOR) x = (T)(x*BASE + val);
		}

        // (if is float) parse fragment
		if(rt::NumericTraits<T>::IsFloat)
		{
			if(*d == (char)DECIMAL_POINT && d<end)
			{	d++;
                double frag = 0, div = 1;
				while((*d==DIGIT_GROUP_SEPARATOR || (val = _details::_to_num<BASE>::call(*d))<BASE) && d<end)
				{
                    if (*d++!=DIGIT_GROUP_SEPARATOR)
                    {
                        div /= BASE;
					    frag += div * val;
                    }
				}
				x = (T)(x + frag);
			}
			// parse scientific
			if(*d == 'e' || *d == 'E')
			{	d++; int index;
				d += t_String_Ref(d,End()).ToNumber(index);
				if(index)x *= _details::_pow((T)10,index);
			}
		}

        // apply sign
		if(rt::NumericTraits<T>::IsSigned || rt::NumericTraits<T>::IsFloat)
		{
#pragma warning(disable:4146)
            if(neg)x = -x; 
#pragma warning(default:4146)
		}
        return (int)(d - _SC::_p);
	}
private:
	INLFUNC void GetNextToken(const CharacterSet& token_charset, rt::String& token, rt::String& non_token) const { ASSERT(0); } // not supported
	INLFUNC void GetNextToken(const CharacterSet& token_charset, rt::String& token) const { ASSERT(0); } // not supported
	INLFUNC void GetNextLine(rt::String& x, bool skip_empty_line = true) const { ASSERT(0); } // not supported
public:
	INLFUNC bool GetNextLineWithQuote(t_String_Ref& x, bool skip_empty_line = true, char QUOTE = '"')
	{	int quote_co = 0;
		t_String_Ref seg = x;
		if(GetNextLine(seg))
		{
			LPCSTR begin = seg.Begin();
			LPCSTR end;

			do
			{	end = seg.End();
				for(SIZE_T i=0; i<seg.GetLength(); i++)
					if(seg[i] == QUOTE)quote_co++;

				if(0 == (quote_co&1))break;
			}while(GetNextLine(seg));

			x = t_String_Ref(begin, end);
			return true;
		}
		return false;
	}
	INLFUNC bool GetNextLine(t_String_Ref& x, bool skip_empty_line = true) const // empty line will be skipped
	{	LPCSTR start = _SC::_p;		const char* tail = End();
		if(x.End() >= _SC::_p && x.End() <= tail)start = (char*)x.End();
		LPCSTR end;
		if(skip_empty_line)
		{	while(start<tail && (*start==0xd || *start==0xa))start++;
			end = start+1;
		}
		else if(start<tail && (*start==0xd || *start==0xa))
		{	start++;
			if(start<tail && start[-1]==0xd && *start==0xa)start++;
			end = start;
		}
		else
		{	end = start+1;
		}
		if(start>=tail)return false;
		
		while(end<tail && *end!=0xa && *end!=0xd)end++;
		x = t_String_Ref(start,end);
		return true;
	}
	INLFUNC bool GetNextToken(const CharacterSet& token_charset, t_String_Ref& token, t_String_Ref& non_token) const
	{	
		LPCSTR start = _SC::_p;		LPCSTR tail = End();
		if(non_token.End() >= (LPCSTR)_SC::_p && non_token.End() <= tail){ start = non_token.End(); }
		else if(token.End() >= (LPCSTR)_SC::_p && token.End() <= tail){ start = token.End(); }
		if(start>=tail)return false;

		LPCSTR nontoken_start = start;			
		while(token_charset.Has(*nontoken_start))
		{	
			if(nontoken_start<tail){ nontoken_start++; }
			else{ token = t_String_Ref(start, tail); non_token = t_String_Ref(); return start < tail; }
		}
		token = t_String_Ref(start, nontoken_start);

		LPCSTR nontoken_end = nontoken_start+1;
		while(!token_charset.Has(*nontoken_end))
		{	if(nontoken_end<tail){ nontoken_end++; }
			else{ non_token = t_String_Ref(nontoken_start, tail); return true; }
		}
		non_token = t_String_Ref(nontoken_start, nontoken_end);
		return true;
	}
	INLFUNC UINT GetTokens(const CharacterSet& token_charset, t_String_Ref* token, UINT token_count) const
	{	t_String_Ref v;	UINT co = 0;
		while(GetNextToken(token_charset, v))
		{	token[co] = v;
			co++;
			if(co == token_count)return co;
		}
		return co;
	}
	INLFUNC bool GetNextToken(const CharacterSet& token_charset, t_String_Ref& token) const
	{	
		LPCSTR start = _SC::_p;
		LPCSTR tail = End();
		if(token.End() >= (LPCSTR)_SC::_p && token.End() <= tail){ start = token.End(); }
		if(start>=tail)return false;

		while(!token_charset.Has(*start))
		{	if(start<tail){	start++; }
			else return false;
		}
		LPCSTR token_end = start+1;
		while(token_charset.Has(*token_end) && token_end<tail)
			token_end++;

		token = t_String_Ref(start, token_end);
		return !token.IsEmpty();
	}
	template<bool merge_adjecent_sep, char quote1, char quote2, typename T>
	INLFUNC UINT Split(T* fields, UINT fields_count, const CharacterSet& seps)	const // return count of field actually parsed
	{	if(_SC::IsEmpty())return 0;
		UINT i=0;	const char* start = _SC::_p;	const char* tail = End();
		const char* p = start;
		int quote_cur = 0;
		for(;;)
		{	
			if(p<tail)
			{
				if(quote_cur)
				{	if(quote_cur == *p && p>start && p[-1] != '\\')
					{	quote_cur = 0;
					}
					p++;
					continue;
				}
				else
				{	if(*p == quote1 || quote2 == *p)
					{	quote_cur = *p;
						p++;
						continue;
					}
					else if(!seps.Has(*p))
					{	p++;
						continue;
					}
				}
			}

			t_String_Ref v(start,p);
			if(v.GetLength()>=2 && v[0] == v.Last() && (v[0] == quote1 || v[0] == quote2))
				v = v.SubStr(1,v.GetLength()-2);
			fields[i++] = v;
			if(i>=fields_count || p==tail)
			{
				for(UINT j=i+1;j<fields_count;j++)
					fields[j].Empty();
				return i;
			}
			p++;
			if(merge_adjecent_sep)
				while(seps.Has(*p) && p<tail)p++;
			start = p;
		}
	}
	template<bool merge_adjecent_sep, typename T>
	FORCEINL UINT Split(T* fields, UINT fields_count, const CharacterSet& seps)	const // return count of field actually parsed
	{	return Split<merge_adjecent_sep, '"', '\'', T>(fields, fields_count, seps);
	}
	template<typename T>
	FORCEINL UINT Split(T* fields, UINT fields_count, const CharacterSet& seps)	const // return count of field actually parsed
	{	return Split<false, '"', '\'', T>(fields, fields_count, seps);
	}
	t_String_Ref UnescapeCharacters(char escape_sign = '\\')
	{	LPCSTR s = Begin();	LPCSTR end = End();
		LPSTR p = Begin();
		for(; s<end-1; s++)
		{	if(*s == escape_sign){ *p++ = s[1]; s++; }
			else{ *p++ = *s; }
		}
		if(*s != escape_sign)*p++ = *s;
		_SC::_len = (SIZE_T)(p-_SC::_p)+1;
		return *this;
	}
	t_String_Ref UnescapeCharacters(const CharacterSet& unescape, char escape_sign = '\\')
	{	LPCSTR s = Begin();	LPCSTR end = End();
		LPSTR p = Begin();
		for(; s<end-1; s++)
		{	if(*s == escape_sign){ *p++ = unescape.Mapped(s[1]); s++; }
			else{ *p++ = *s; }
		}
		if(*s != escape_sign)*p++ = *s;
		_SC::_len = (SIZE_T)(p-_SC::_p)+1;
		return *this;
	}
	//FORCEINL t_String_Ref TrimBy(CHAR c) const
	//{	SIZE_T i=0;
	//	for(;i<_SC::_len;i++)
	//	{	if(_SC::_p[i] == (char)'.')return t_String_Ref(_SC::_p, &_SC::_p[i]);
	//	}
	//	return *this;
	//}
	FORCEINL t_String_Ref TrimExtName() const
	{	SSIZE_T i = (SSIZE_T)GetLength();
		for(;i>=0;i--)
		{	if(_SC::_p[i] == (char)'.')goto FIND_EXT;
			if(_SC::_p[i] == (char)'\\' || _SC::_p[i] == (char)'/')return *this;
		}
		return *this;
	FIND_EXT:
		return SubStr(0,i);
	}
	FORCEINL t_String_Ref GetExtName() const // return ".xxx"
	{	SSIZE_T i=GetLength()-1;
		for(;i>=0;i--)
		{	if(_SC::_p[i] == '.')return t_String_Ref(_SC::_p+i, End());
			if(_SC::_p[i] == '\\' || _SC::_p[i] == '/')return t_String_Ref();
		}
		return t_String_Ref();
	}
	FORCEINL t_String_Ref GetFilename() const // return "aaa.bbb"
	{	SSIZE_T i=GetLength()-1;
		for(;i>=0;i--)
		{	if(_SC::_p[i] == '\\' || _SC::_p[i] == '/')return t_String_Ref(_SC::_p+i+1,End());
		}
		return *this;
	}
	FORCEINL t_String_Ref DecodedURL()	// inplace precent-decoding
	{	SIZE_T c = 0;
		for(SIZE_T i=0; i<GetLength(); i++)
        {	if(_SC::_p[i] != '%')
            {	_SC::_p[c++] = _SC::_p[i];
			}
			else
			{	_SC::_p[c++] =	((_SC::_p[i+1]>='A'?(_SC::_p[i+1] - 'A' + 10):(_SC::_p[i+1] - '0')) << 4 ) |
                                ((_SC::_p[i+2]>='A'?(_SC::_p[i+2] - 'A' + 10):(_SC::_p[i+2] - '0')) );
				i += 2;
			}
		}
		_SC::_len = c+1;
		return *this;
	}
	FORCEINL t_String_Ref GetFilenameURL() const // return "aaa.bbb"
	{	t_String_Ref fn = GetFilename();
		for(SIZE_T i=0;i<fn.GetLength();i++)
			if(fn[i] == '?' || fn[i] == '#')
				return t_String_Ref(fn.Begin(), &fn[i]).GetFilename();
		return GetFilename();
	}
	INLFUNC void SplitURL(t_String_Ref& protocal, t_String_Ref& host, t_String_Ref& path) const
	{	int s = (int)FindCharacter(':');
		if(s<0 || ((int)GetLength()) < s+4 || _SC::_p[s+1] != '/' || _SC::_p[s+2] != '/')
		{	protocal.Empty(); host.Empty();
			path = *this;
		}
		else
		{	protocal = t_String_Ref(_SC::_p, &_SC::_p[s]);
			s+=3;
			int h = (int)FindCharacter('/', s);
			if(h>=s)
			{	host = t_String_Ref(&_SC::_p[s], &_SC::_p[h]);
				path = t_String_Ref(&_SC::_p[h], End());
			}
			else
			{	host = t_String_Ref(&_SC::_p[s], End());
				path = t_String_Ref("/",1);
			}
		}
	}
	INLFUNC void SplitURL(t_String_Ref& protocal, t_String_Ref& host, t_String_Ref& file_path, t_String_Ref& cgi_param) const
	{	SplitURL(protocal, host, file_path);
		int off = (int)file_path.FindCharacter('?');
		if(off>=0)
		{	cgi_param = file_path.TrimLeft(off+1);
			file_path.SetLength(off);
		}
		else cgi_param.Empty();
	}
	INLFUNC t_String_Ref ResolveRelativePath()
	{
		t_String_Ref d[256];
#ifdef PLATFORM_WIN
		UINT co = Split<true>(d, 256, "/\\");
#else
		UINT co = Split<true>(d, 256, '/');
#endif
		UINT open = 0;
		for(UINT q=0;q<co;q++)
		{
			//if(d[i].IsEmpty())continue;
			if(d[q].GetLength() == 1 && d[q][0] == '.')continue;
			if(d[q].GetLength() == 2 && d[q][0] == '.' && d[q][0] == '.'){ if(open)open--; continue; }
			d[open++] = d[q];
		}
		LPSTR p = _SC::_p;		UINT i = 0;
		while(p == d[i].Begin() && i<open)
		{	p += d[i].GetLength()+1;
			i++;
		}
		while(i<open)
		{	memcpy(p, d[i].Begin(), d[i].GetLength());
			p += d[i].GetLength()+1;
			if(i<open-1)p[-1] = '/';
			i++;
		}
		_SC::_len = ((SIZE_T)(p-_SC::_p));
		return *this;
	}
	FORCEINL t_String_Ref TrimBefore(const CharacterSet& seps) const // separator will also be trimmed
	{	SIZE_T pos = 0;
		for(;pos<_SC::_len-1;pos++)
			if(seps.Has(_SC::_p[pos]))return t_String_Ref(&_SC::_p[pos+1], End());
		return *this;
	}
	FORCEINL t_String_Ref TrimAfter(const CharacterSet& seps) const // separator will also be trimmed
	{	SIZE_T pos = 0;
		for(;pos<_SC::_len-1;pos++)
			if(seps.Has(_SC::_p[pos]))return t_String_Ref(Begin(), &_SC::_p[pos]);
		return *this;
	}
	FORCEINL t_String_Ref TrimBeforeReverse(const CharacterSet& seps) const // separator will also be trimmed
	{	SSIZE_T pos = GetLength()-1;
		for(;pos>=0;pos--)
			if(seps.Has(_SC::_p[pos]))return t_String_Ref(&_SC::_p[pos+1], End());
		return *this;
	}
	FORCEINL t_String_Ref TrimAfterReverse(const CharacterSet& seps) const // separator will also be trimmed
	{	SSIZE_T pos = GetLength()-1;
		for(;pos>=0;pos--)
			if(seps.Has(_SC::_p[pos]))return t_String_Ref(Begin(), &_SC::_p[pos]);
		return *this;
	}
	FORCEINL t_String_Ref GetDirectoryName() const // include the terminate '/'
	{	
		SSIZE_T i=GetLength()-1;
		for(;i>=0;i--)
		{	if(_SC::_p[i] == '\\' || _SC::_p[i] == '/')return t_String_Ref(_SC::_p,&_SC::_p[i+1]);
		}
		return t_String_Ref("./",2);
	}
	FORCEINL t_String_Ref TrimLeft(SIZE_T length) const 
	{	if(length >= GetLength())return t_String_Ref();
		return t_String_Ref(_SC::_p+length, _SC::_len-length-1);
	}
	FORCEINL t_String_Ref	TrimRight(SIZE_T length) const
	{	if(length >= GetLength())return t_String_Ref();
		return t_String_Ref(_SC::_p, _SC::_len-length-1);
	}
	FORCEINL t_String_Ref	TrimLeftSpace() const 
	{	if(_SC::IsEmpty())return *this;
		char* p=_SC::_p, *end = _SC::_p+_SC::_len-1; for(;p<end && *p<=0x20 && *p>=0;p++){}; return t_String_Ref(p,end);
	}
	FORCEINL t_String_Ref	TrimRightSpace() const
	{	if(_SC::IsEmpty())return *this;
		SIZE_T l = _SC::_len-1; for(;l>1 && _SC::_p[l-1]<=0x20 && _SC::_p[l-1]>=0;l--){}; return t_String_Ref(_SC::_p,l);
	}
	FORCEINL t_String_Ref	TrimSpace() const 
	{	if(_SC::IsEmpty())return *this;
		return TrimLeftSpace().TrimRightSpace(); 
	}
	FORCEINL t_String_Ref	RegularizeUTF8()
	{	LPSTR p = _SC::_p;
		for(SIZE_T i=0;i<_SC::_len-1;i++)
		{	int c = ((LPCBYTE)_SC::_p)[i];
			if((c&0x80) == 0){ *p++ = c; continue; }
			if((c&0xe0) == 0xc0){ *p++ = c; *p++ = 0x80 | (0x3f&_SC::_p[++i]); continue; }
			if((c&0xf0) == 0xe0){ *p++ = c; *p++ = 0x80 | (0x3f&_SC::_p[++i]); *p++ = 0x80 | (0x3f&_SC::_p[++i]); continue; }
		}
		_SC::_len = p - _SC::_p + 1;
		return *this;
	}
	FORCEINL SIZE_T		Occurrence(const rt::CharacterSet& set) const
	{	SIZE_T c = 0;
		for(SIZE_T i=0;i<GetLength();i++)
			if(set.Has(_SC::_p[i]))c++;
		return c;
	}
};

class String_Ref: public String_Base<_details::_StringPtrStore, String_Ref> // String_Ref represent a string with length, the string is not necessary zero-terminated
{	
	typedef String_Base<_details::_StringPtrStore, String_Ref> _SC;
	template<typename t_Left, typename t_Right>
	friend class _details::String_AddExpr;

	FORCEINL SIZE_T _ExprFill(char* p) const	// return number of char filled
	{	if(!IsEmpty())
		{	memcpy(p,_p,GetLength()*sizeof(char));
			return GetLength();
		}
		else return 0;
	}

public:
	typedef char t_Char;
	typedef _details::hash_compare_str<String_Ref> hash_compare;

public:
    FORCEINL ~String_Ref(){}
	template<typename SS, typename SR>
	FORCEINL String_Ref(const String_Base<SS,SR>& x){ _SC::_p = (LPSTR)x._p; _SC::_len = x._len; }
	FORCEINL String_Ref(){ _SC::_p = nullptr; _len = 0; }
	FORCEINL String_Ref(const String_Ref& x){ *this = x; }
	FORCEINL String_Ref(const char* x){ operator = (x); }
	FORCEINL String_Ref(const char* p, SIZE_T len){ _SC::_p = (char*)p; _SC::_len = len+1; }
	FORCEINL String_Ref(const char* p, const char* end)
	{	if(end){ _p = (char*)p; ASSERT(end>=p); _SC::_len = (int)(end-p+1); }
		else{ *this = p; }
	}
	template <template<class, class, class> class std_string, class _Traits, class _Alloc>
	FORCEINL String_Ref(const std_string<char, _Traits, _Alloc>& str)
	{	if(str.size()){	_SC::_p = (LPSTR)str.c_str(); _SC::_len = str.size()+1; }
		else{ _SC::_p = nullptr; _SC::_len = 0; }
	}
	FORCEINL const String_Ref& operator = (const char* x)
	{	if(x)
		{	_SC::_p = (char*)x;
			_SC::_len = 1 + strlen(x);
		}
		else Empty();
		return *this;
	}
	FORCEINL static const String_Ref& EmptyString(){ static const rt::String_Ref _empty; return _empty; }
	FORCEINL bool SetLength(SIZE_T len){ if(len<=GetLength()){ _SC::_len = len + 1; return true; } return false; }
public:
	FORCEINL String_Ref& Replace(char a, char b){ ((_SC*)this)->Replace(a,b); return *this; }
	FORCEINL String_Ref& Replace(const CharacterSet& a, char b){ ((_SC*)this)->Replace(a,b); return *this; }
	template<typename T1, typename TString>
	FORCEINL UINT ReplaceTo(const T1& a, const String_Ref& b, TString& replaced) const // replace all a with b in the current string, low performance, return # of replacement
	{	SIZE_T a_len = rt::String_Ref(a).GetLength();
		replaced = *this;
        SIZE_T s = 0;
		UINT replaced_co = 0;
        for (;;)
        {   SSIZE_T p = replaced.FindString(a, s);
            if (p < 0) break;
            replaced = replaced.SubStrHead(p) + b + replaced.SubStr(p + a_len);
            s = p + b.GetLength();
			replaced_co++;
        }
		return replaced_co;
	}
	FORCEINL bool ParseNextNumber(UINT& out)	// parsed text will be removed
	{	SIZE_T start = 0;
		for(;start < GetLength();start++)
			if(_SC::_p[start]>=(char)'0' && _SC::_p[start]<=(char)'9')goto PARSE_UINT;
		return false;
	PARSE_UINT:
		UINT ret = 0;
		for(;start < GetLength();start++)
			if(_SC::_p[start]>=(char)'0' && _SC::_p[start]<=(char)'9')
				ret = ret*10 + (_SC::_p[start] - (char)'0');
			else break;
		_SC::_p += start; _SC::_len -= start;	out = ret;
		return true;
	}
	FORCEINL String_Ref GetLengthRecalculated() const
	{	if(_p)
		{	for(SIZE_T i=0;i<GetLength();i++)
				if(_p[i] == 0)
				{	return rt::String_Ref(_p, i);
				}
		}
		return *this;
	}
	FORCEINL String_Ref Flip()
	{	for(SIZE_T i=0; i<GetLength()/2; i++)
			rt::Swap(_SC::_p[i], _SC::_p[GetLength() - i - 1]);
		return *this;
	}
};
template<class t_Ostream, typename t_StringStore, class t_String_Ref>
FORCEINL t_Ostream& operator << (t_Ostream& Ostream, const String_Base<t_StringStore, t_String_Ref> & vec)
{	
	if(vec.GetLength())
	{
		if(vec.IsZeroTerminated())
		{	Ostream<<vec.Begin();	}
		else
		{	for(SIZE_T i=0;i<vec.GetLength();i++)Ostream<<vec[i];	}
	}
	return Ostream;
}

FORCEINL String_Ref operator || (const String_Ref& x, const String_Ref& y)
{
	if(x.IsEmpty())return y;
	return x;
}

} // namespace rt

namespace rt
{
//////////////////////////////////////////
// to string
namespace tos
{
	template<int DIMEN = 1, int LEN = (21*DIMEN + (DIMEN-1)*2) >
	class S_:public String_Ref
	{	
		template<int _DIMEN, int _LEN> friend class S_;
		template<typename t_Left, typename t_Right> friend class String_AddExpr;
	protected:
        char _string[LEN];
		FORCEINL S_(){ _p = _string; _len = 0; }
		template<typename T>
		FORCEINL static int __toS_vec(LPSTR p, const T& x)
		{	int len = 1 + S_<DIMEN-1, LEN>::__toS_vec(p, x);
			p[len-1] = ',';
			return len + S_<1, LEN>::__toS(p+len, x[DIMEN-1]);
		}
	public:
		template<typename T>
		FORCEINL S_(const T& x)
		{	_len = 1 + __toS_vec(_string, x);
			_p = _string;
			ASSERT(_len <= LEN);
		}
		FORCEINL operator LPCSTR() const { return _p; }
	};
	template<int LEN>
	class S_<1, LEN>:public String_Ref
	{	template<int _DIMEN, int _LEN> friend class S_;
		template<typename t_Left, typename t_Right>	friend class String_AddExpr;
	protected:
        char _string[LEN];
		template<typename T>
		FORCEINL static int __toS_vec(LPSTR p, const T& x)
		{	return S_<1, LEN>::__toS(p, x[0]);
		}
	public:
		FORCEINL operator LPCSTR() const { return _p; }
		FORCEINL String_Ref&	RightAlign(UINT w, char pad = ' ')
		{	ASSERT(w < sizeofArray(_string));
			if(w > String_Ref::GetLength())
			{	memmove(&_string[w - String_Ref::GetLength()], _string, String_Ref::GetLength());
				for(SIZE_T i=0; i<w - GetLength(); i++)_string[i] = pad;
				_len = w + 1;
				_string[w] = 0;
			}
			return *this;
		}
		FORCEINL String_Ref&	LeftAlign(UINT w, char pad = ' ')
		{	ASSERT(w <= sizeofArray(_string));
			if(w > GetLength())
			{	for(SIZE_T i=GetLength(); i<w; i++)_string[i] = pad;
				_len = w + 1;
				_string[w] = 0;
			}
			return *this;
		}
		FORCEINL static int __toS(LPSTR p, char x){ p[0] = x; return 1; }
		FORCEINL static int __toS(LPSTR p, int x){ return string_ops::itoa(x,p); }// sprintf(p,"%d",x); }
		FORCEINL static int __toS(LPSTR p, unsigned int x){ return string_ops::itoa(x,p);; } //sprintf(p,"%u",x); }
		FORCEINL static int __toS(LPSTR p, LONGLONG x){ return string_ops::itoa(x,p); } //sprintf(p,"%lld",x); }
		FORCEINL static int __toS(LPSTR p, ULONGLONG x){ return string_ops::itoa(x,p); } //sprintf(p,"%llu",x); }
#if defined(PLATFORM_WIN) || defined(PLATFORM_MAC)
		FORCEINL static int __toS(LPSTR p, long x){ return string_ops::itoa((int)x,p); }// sprintf(p,"%d",x); }
		FORCEINL static int __toS(LPSTR p, unsigned long x){ return string_ops::itoa((UINT)x,p);; } //sprintf(p,"%u",x); }
#endif
		FORCEINL static int __toS(LPSTR p, LPCVOID x)
		{	p[0] = (char)'0';			p[1] = (char)'x';
#if   defined(PLATFORM_64BIT)
			return 2 + sprintf(&p[2],"%016llx",(ULONGLONG)x);
#elif defined(PLATFORM_32BIT)
			return 2 + sprintf(&p[2],"%08x",(DWORD)x);
#endif
		}
		FORCEINL static int __toS(LPSTR p, float x){ return string_ops::ftoa<float>(x, p, 2); }
		FORCEINL static int __toS(LPSTR p, double x){ return string_ops::ftoa<double>(x, p, 4); }
		FORCEINL static int __toS(LPSTR p, bool x)
		{	if(x){ *((DWORD*)p) = 0x65757274; return 4; }
			else{ *((DWORD*)p) = 0x736c6166; p[4] = 'e'; return 5; }
		}
		FORCEINL S_(const S_& x){ _p = _string; _len = x._len; memcpy(_p, x._p, _len); }
		FORCEINL S_(){ _p = _string; _len = 0; }
		FORCEINL S_(bool x){ _len = 1 + __toS(_string,x); ASSERT(_len <= LEN); _string[_len] = 0; _p = _string; }
		FORCEINL S_(char x){ _len = 1 + __toS(_string,x); ASSERT(_len <= LEN); _string[_len] = 0; _p = _string; }
		FORCEINL S_(int x){ _len = 1 + __toS(_string,x); ASSERT(_len <= LEN); _string[_len] = 0; _p = _string; }
		FORCEINL S_(unsigned int x){ _len = 1 + __toS(_string,x); ASSERT(_len <= LEN); _string[_len] = 0; _p = _string; }
		FORCEINL S_(LONGLONG x){ _len = 1 + __toS(_string,x); ASSERT(_len <= LEN); _string[_len] = 0; _p = _string; }
		FORCEINL S_(ULONGLONG x){ _len = 1 + __toS(_string,x); ASSERT(_len <= LEN); _string[_len] = 0; _p = _string; }
#if defined(PLATFORM_WIN) || defined(PLATFORM_MAC)
		FORCEINL S_(long x){ _len = 1 + __toS(_string,x); ASSERT(_len <= LEN); _string[_len] = 0; _p = _string; }
		FORCEINL S_(unsigned long x){ _len = 1 + __toS(_string,x); ASSERT(_len <= LEN); _string[_len] = 0; _p = _string; }
#endif
		FORCEINL S_(LPCVOID x){ _len = 1 + __toS(_string,x); ASSERT(_len <= LEN); _string[_len] = 0; _p = _string; }
		FORCEINL S_(float x){ _len = 1 + __toS(_string,x); ASSERT(_len <= LEN); _string[_len] = 0; _p = _string; }
		FORCEINL S_(double x){ _len = 1 + __toS(_string,x); ASSERT(_len <= LEN); _string[_len] = 0; _p = _string; }
	};

} // namespace tos



//////////////////////////////////////////
// String Expression
namespace _details
{
template<typename t_Left, typename t_Right>
class String_AddExpr
{	template<typename t_Left_, typename t_Right_>
	friend class String_AddExpr;
	char*		_pString;
	SSIZE_T		_len;

public:
	typedef char t_Char;
	t_Left		_left;
	t_Right		_right;

public:
	template<typename T1, typename T2>
	FORCEINL String_AddExpr(T1& l, T2& r)
		:_left(l),_right(r),_pString(nullptr),_len(-1)
	{}
	FORCEINL ~String_AddExpr(){ _SafeFree32AL(_pString); }
	FORCEINL const char* Begin() const { return rt::_CastToNonconst(this)->GetSafeString(); }
	FORCEINL const char* GetSafeString() 
	{	if(_pString == nullptr)
		{	GetLength();
			_pString = _Malloc32AL(char,_len+1);
			ASSERT(_pString);
			VERIFY(_len == (SSIZE_T)CopyTo(_pString));
			_pString[_len] = '\0';
		}
		return _pString;
	}
	FORCEINL const char* Begin(){ return GetSafeString(); }
	FORCEINL String_Ref GetRef()
	{	if(_pString == nullptr)
		{	SIZE_T len = GetLength();
			_pString = _Malloc32AL(char,len+1);
			ASSERT(_pString);
			VERIFY(len == CopyTo(_pString));
			_pString[len] = '\0';
			return String_Ref(_pString, len);
		}
		return String_Ref(_pString);
	}
	FORCEINL operator const char* () const { return ((String_AddExpr*)this)->GetSafeString(); }
	FORCEINL operator const rt::String_Ref () const { ((String_AddExpr*)this)->GetSafeString(); return rt::String_Ref(_pString,_len); }
	FORCEINL SIZE_T GetLength() const
	{	if(_len < 0)((String_AddExpr*)this)->_len = _left.GetLength() + _right.GetLength();
		return _len;
	}
	FORCEINL SIZE_T CopyTo(char* p) const 
	{	SIZE_T len = _left.CopyTo(p);
		return len + _right.CopyTo(p+len);
	}  // terminate-zero is not copied
	template<typename t_Str>
	FORCEINL void	ToString(t_Str& str) const { VERIFY(str.SetLength(GetLength())); VERIFY(str.GetLength() == CopyTo(str)); }
};
template<class t_Ostream, typename t_Left, typename t_Right>
FORCEINL t_Ostream& operator<<(t_Ostream& Ostream, const rt::_details::String_AddExpr<t_Left,t_Right> & str_expr)
{	return Ostream<<str_expr._left<<str_expr._right;
}

} // namespace _details

template<typename t_Left, typename t_Right>
struct _SE:public rt::_details::String_AddExpr<t_Left,t_Right>
{	template<typename T1, typename T2>
	FORCEINL _SE(T1& l, T2& r):rt::_details::String_AddExpr<t_Left,t_Right>(l,r){}
    //FORCEINL ~_SE(){}  // This is an unnecessary code, And even worse, it makes release build never ends in VS2010 SP1
};

#define CPF_STRING_CONNECT_OP_(t_Left, t_Right, t_Left_in, t_Right_in)						\
FORCEINL _SE<t_Left, t_Right >  operator + (const t_Left_in left, const t_Right_in right)	\
{ return _SE<t_Left, t_Right > ( (left), (right) ); }										\

#define CPF_STRING_CONNECT_OP(t_Left, t_Right, t_Left_in, t_Right_in)						\
		CPF_STRING_CONNECT_OP_(t_Right, t_Left, t_Right_in, t_Left_in)						\
		CPF_STRING_CONNECT_OP_(t_Left, t_Right, t_Left_in, t_Right_in)						\

CPF_STRING_CONNECT_OP_(String_Ref,String_Ref,	String_Ref&, String_Ref&	)
CPF_STRING_CONNECT_OP(String_Ref, String_Ref,	String_Ref&, char*			)
CPF_STRING_CONNECT_OP(String_Ref, tos::S_<>,	String_Ref&, char			)
CPF_STRING_CONNECT_OP(String_Ref, tos::S_<>,	String_Ref&, bool			)
CPF_STRING_CONNECT_OP(String_Ref, tos::S_<>,	String_Ref&, int			)
CPF_STRING_CONNECT_OP(String_Ref, tos::S_<>,	String_Ref&, unsigned int	)
#if defined(PLATFORM_WIN) || defined(PLATFORM_MAC)
CPF_STRING_CONNECT_OP(String_Ref, tos::S_<>,	String_Ref&, long			)
CPF_STRING_CONNECT_OP(String_Ref, tos::S_<>,	String_Ref&, unsigned long	)
#endif
CPF_STRING_CONNECT_OP(String_Ref, tos::S_<>,	String_Ref&, LONGLONG		)
CPF_STRING_CONNECT_OP(String_Ref, tos::S_<>,	String_Ref&, ULONGLONG		)
CPF_STRING_CONNECT_OP(String_Ref, tos::S_<>,	String_Ref&, float			)
CPF_STRING_CONNECT_OP(String_Ref, tos::S_<>,	String_Ref&, double			)



#define CPF_STRING_EXPR_CONNECT_OP(type, type_in)			\
template<typename t_Left, typename t_Right> FORCEINL		\
_SE<type, _SE<t_Left,t_Right> >								\
operator + (const type_in p, const _SE<t_Left,t_Right>& x)	\
{ return _SE<type, _SE<t_Left,t_Right> >((p),x); }			\
template<typename t_Left, typename t_Right> FORCEINL		\
_SE<_SE<t_Left,t_Right> , type>								\
operator + (const _SE<t_Left,t_Right>& x, const type_in p)	\
{ return _SE<_SE<t_Left,t_Right>, type>(x, (p)); }			\


CPF_STRING_EXPR_CONNECT_OP(String_Ref,	String_Ref&		)
CPF_STRING_EXPR_CONNECT_OP(String_Ref,	char*			)
CPF_STRING_EXPR_CONNECT_OP(tos::S_<>,	char			)
CPF_STRING_EXPR_CONNECT_OP(tos::S_<>,	int				)
CPF_STRING_EXPR_CONNECT_OP(tos::S_<>,	bool			)
CPF_STRING_EXPR_CONNECT_OP(tos::S_<>,	unsigned int	)
#if defined(PLATFORM_WIN) || defined(PLATFORM_MAC)
CPF_STRING_EXPR_CONNECT_OP(tos::S_<>,	long			)
CPF_STRING_EXPR_CONNECT_OP(tos::S_<>,	unsigned long	)
#endif
CPF_STRING_EXPR_CONNECT_OP(tos::S_<>,	LONGLONG		)
CPF_STRING_EXPR_CONNECT_OP(tos::S_<>,	ULONGLONG		)
CPF_STRING_EXPR_CONNECT_OP(tos::S_<>,	float			)
CPF_STRING_EXPR_CONNECT_OP(tos::S_<>,	double			)


template<typename t_LL, typename t_LR, typename t_RL, typename t_RR > INLFUNC 
_SE<_SE<t_LL,t_LR>, _SE<t_RL,t_RR> >	
operator + (const _SE<t_LL,t_LR>& xl, const _SE<t_RL,t_RR>& xr)					
{ return _SE<_SE<t_LL,t_LR>, _SE<t_RL,t_RR> >(xl,xr); }

template<SIZE_T LEN>
class StringFixed: public String_Base<_details::_StringArrayStore<LEN+1>, String_Ref>
{	typedef String_Base<_details::_StringArrayStore<LEN+1>, String_Ref> _SC;
public:
	FORCEINL StringFixed(){ _SC::_len = 0; }
	template<typename T>
	FORCEINL StringFixed(const T& x)
	{	if(SetLength(x.GetLength()))
			VERIFY(x.CopyTo(_SC::_p) == x.GetLength());
	}
	FORCEINL bool SetLength(SIZE_T sz)
	{	if(sz <= LEN){ _SC::_len = sz+1; _SC::_p[sz] = 0; return true; }
		else return false;
	}
	FORCEINL const StringFixed& operator = (const char* x){ *this = String_Ref(x); return *this; }
	FORCEINL const StringFixed& operator = (char* x){ *this = String_Ref(x); return *this; }
	FORCEINL const StringFixed& operator = (const String_Ref& x)
	{	if(!x.IsEmpty())
		{	if(SetLength(x.GetLength()))	// handle the case of x = substring of x
				memcpy(_SC::_p,x.Begin(),_SC::GetLength());
		}else{ _SC::Empty(); }
		return *this;
	}
	template<typename T>
	FORCEINL const StringFixed& operator = (const T& string_expr)
	{	SIZE_T len = string_expr.GetLength();
		if(SetLength(len))
			VERIFY(len == string_expr.CopyTo(_SC::_p));
		return *this;
	}
	template<typename T>
	FORCEINL const T& operator += (const T& string_expr)
	{	SIZE_T len = string_expr.GetLength();
		if(len + _SC::_len < LEN)
		{	VERIFY(len == string_expr.CopyTo(_SC::_p+_SC::GetLength()));
			_SC::_p[len+_SC::GetLength()] = '\0';
			_SC::_len = len + _SC::GetLength() + 1;
		}
		return string_expr;
	}
	FORCEINL LPCSTR operator += (LPCSTR str){ (*this) += rt::String_Ref(str); return str; }
	FORCEINL void operator += (char x){ SIZE_T pos = _SC::GetLength(); if(SetLength(pos+1))_SC::_p[pos] = x; }
};

class String: public String_Ref
{
protected:
	SIZE_T		_len_reserved;
	INLFUNC bool _ChangeSize(SIZE_T new_size) //Orignal data at front is preserved
	{	if( new_size <= _len_reserved){ _len = new_size; return true; }
		_len_reserved = rt::max(rt::max((SIZE_T)16,new_size),_len_reserved*2);
		LPBYTE pNewBuffer = _Malloc32AL(BYTE,_len_reserved);
		if(pNewBuffer){}else{ return false; }
		// copy old elements
		memcpy(pNewBuffer,_p,sizeof(char)*_len);
		_SafeFree32AL(_p);
		_p = (char*)pNewBuffer;
		_len = new_size;
		return true;
	}
public:
	FORCEINL bool SetLength(SIZE_T sz)
	{	if(sz == 0)
		{	if(_p && _len>1){ _p[0] = 0; _len = 1; }
			return true;
		}
		if(_ChangeSize(sz+1)){ _p[sz] = 0; return true; }
		else return false;
	}
	FORCEINL ~String(){ _SafeFree32AL(_p); }
	FORCEINL LPSTR DetachBuffer(){ LPSTR p = _p; _len_reserved = _len = 0; _p = nullptr; return p; }
	FORCEINL String& Empty(){ SetLength(0); return *this; }
	FORCEINL String& SecureEmpty(){ _len = 0; rt::Zero(_p, _len_reserved); return *this; }
public:
	FORCEINL String(){ _len_reserved = 0; _p = nullptr; _len = 0; }
	FORCEINL String(const String& x){ _len_reserved = 0; _p = nullptr; _len = 0; *this = (String_Ref&)x; }
	FORCEINL String(const String_Ref& x){ _len_reserved = 0; _p = nullptr; _len = 0; *this = x; }
	FORCEINL String(const char* x){ _len_reserved = 0; _p = nullptr; _len = 0; operator = (x); }
	FORCEINL String(const char* p, SIZE_T len){ _len_reserved = 0; _p = nullptr; _len = 0; *this = String_Ref(p,len); }
	FORCEINL String(const char* p, const char* end){ _len_reserved = 0; _p = nullptr; _len = 0; *this = String_Ref(p,end); }
    FORCEINL String(const char c, int count){ _len_reserved = 0; _p = nullptr; _len = 0; for (int i = 0; i < count; i++) *this += c; }
	template<typename T>
	FORCEINL String(const T& string_expr){ _p = nullptr; _len_reserved = _len = 0; (*this) = string_expr; }

	FORCEINL const String& operator = (const char* x){ *this = String_Ref(x); return *this; }
	FORCEINL const String& operator = (char* x){ *this = String_Ref(x); return *this; }
	FORCEINL const String& operator = (char x){ SetLength(1); _p[0] = x; return *this; }
	FORCEINL const String& operator = (const String_Ref& x)
	{	if(!x.IsEmpty())
		{	if(_ChangeSize(x.GetLength()+1))	// handle the case of x = substring of x
			{	memcpy(_p,x.Begin(),GetLength());
				_p[x.GetLength()] = 0;
			}
		}else{ Empty(); }
		return *this;
	}
	FORCEINL const String& operator = (const String& x){ return *this = (const String_Ref&)x; }
	//INLFUNC operator const String_Ref& () const { return *this; }
	//INLFUNC operator const String_Ref& (){ return *this; }
	template<typename T>
	FORCEINL const T& operator = (const T& string_expr)
	{	SIZE_T len = string_expr.GetLength();
		LPSTR p = _Malloc32AL(char, len+1);
		p[len] = '\0';
		VERIFY(len == string_expr.CopyTo(p));
		_len_reserved = _len = len + 1;
		rt::Swap(p, _p);
		_SafeFree32AL(p);
		return string_expr;
	}
	template<typename T>
	FORCEINL const T& operator += (const T& string_expr)
	{	SIZE_T len = string_expr.GetLength();
		if(len + GetLength() + 1 > _len_reserved)
		{	LPSTR p = _Malloc32AL(char, len + GetLength()+1);
			memcpy(p, _p, GetLength());
			VERIFY(len == string_expr.CopyTo(p+GetLength()));
			p[len+GetLength()] = '\0';
			_len_reserved = _len = len + GetLength() + 1;
			rt::Swap(p, _p);
			_SafeFree32AL(p);
		}
		else
		{	VERIFY(len == string_expr.CopyTo(_p+GetLength()));
			_p[len+GetLength()] = '\0';
			_len = len + GetLength() + 1;
		}
		return string_expr;
	}
	FORCEINL LPCSTR operator += (LPCSTR str){ (*this) += rt::String_Ref(str); return str; }
	FORCEINL void operator += (char x){ SIZE_T pos = GetLength(); SetLength(pos+1); _p[pos] = x; }
private:
	FORCEINL bool ParseNextNumber(UINT& out){ ASSERT(0); return false; } // ParseNextNumber cannot be used on a String
public:
	FORCEINL operator LPCSTR () const { ASSERT(!_p || !_p[GetLength()]); return _p; }
	FORCEINL operator LPSTR () { if(_p){ ASSERT(!_p[GetLength()]); } return _p; }
	FORCEINL String_Ref RemoveCharacters(const rt::CharacterSet& s){ String_Ref::RemoveCharacters(s); if(!IsEmpty())_p[_len-1] = 0; return *this; }
	FORCEINL void		RecalculateLength(){ if(_p){ _len = strlen(_p) + 1; }else{ _len = 0; } }
	FORCEINL SSIZE_T	FindString(const char* pSub, SIZE_T skip_first = 0) const // return -1 if not found
	{	return (skip_first<GetLength() && (pSub=strstr(Begin()+skip_first,pSub)))?(int)(pSub-Begin()):-1;
	}
	FORCEINL String& Replace(char a, char b){ ((String_Base<_details::_StringPtrStore, String_Ref>*)(this))->Replace(a,b); return *this; } // replace all a with b
	FORCEINL String& Replace(char a, const String_Ref& b) // replace all a with b
	{	rt::String tmp;
        tmp.SetLength(this->GetLength() * b.GetLength());
        tmp.SetLength(0);
        for (int i=0; i<(int)this->GetLength(); i++)
        {   if (_p[i] == a) tmp += b;
            else tmp += _p[i];
        }
        rt::Swap(*this, tmp);
		return *this;
	}
	template<typename T1>
	FORCEINL UINT Replace(const T1& a, const String_Ref& b) // replace all a with b in the current string, low performance, return # of replacement
	{	SIZE_T a_len = rt::String_Ref(a).GetLength();
		if(a_len == 0)return 0;
		rt::String tmp = *this;
        SIZE_T s = 0;
		UINT replaced = 0;
        for (;;)
        {   SSIZE_T p = tmp.FindString(a, s);
            if (p < 0) break;
            tmp = tmp.SubStrHead(p) + b + tmp.SubStr(p + a_len);
            s = p + b.GetLength();
			replaced++;
        }
        rt::Swap(*this, tmp);
		return replaced;
	}
	String& UnescapeCharacters(char escape_sign = '\\')
	{	if(!IsEmpty())
		{	String_Ref::UnescapeCharacters(escape_sign);
			_p[_len-1] = 0;
		}
		return *this;
	}
	String& UnescapeCharacters(const rt::String_Ref& input, char escape_sign = '\\')
	{	VERIFY(SetLength(input.GetLength()));
		LPCSTR s = input.Begin();	LPCSTR end = input.End();
		LPSTR p = Begin();
		for(; s<end-1; s++)
		{	if(*s == escape_sign){ *p++ = s[1]; s++; }
			else{ *p++ = *s; }
		}
		if(*s != escape_sign)*p++ = *s;
		_len = (SIZE_T)(p-_p)+1;
		_p[_len-1] = 0;
		return *this;
	}
	String& UnescapeCharacters(const rt::String_Ref& input, const CharacterSet& escape, char escape_sign = '\\')
	{	VERIFY(SetLength(input.GetLength()));
		LPCSTR s = input.Begin();	LPCSTR end = input.End();
		LPSTR p = Begin();
		for(; s<end-1; s++)
		{	if(*s == escape_sign){ *p++ = escape.Mapped(s[1]); s++; }
			else{ *p++ = *s; }
		}
		if(*s != escape_sign)*p++ = *s;
		_len = (SIZE_T)(p-_p)+1;
		_p[_len-1] = 0;
		return *this;
	}
	String& EscapeCharacters(const rt::String_Ref& input, const CharacterSet& characters_to_be_escaped, char escape_sign = '\\')
	{	LPCSTR s = input.Begin();
		LPCSTR end = input.End();
		SIZE_T replace_count = 0;
		for(;s<end;s++)
			if(*s == escape_sign || characters_to_be_escaped.Has(*s))replace_count++;
		if(replace_count)
		{
			VERIFY(SetLength(input.GetLength()+replace_count));
			LPSTR p = Begin();
			s = input.Begin();
			for(;s<end;s++)
			{	if(*s == escape_sign || characters_to_be_escaped.Has(*s))
				{	*p++ = escape_sign;
					*p++ = characters_to_be_escaped.Mapped(*s);
				}
				else *p++ = *s;
			}
		}
		else *this = input;
		return *this;
	}
	String& EscapeCharacters(const CharacterSet& characters_to_be_escaped, char escape_sign = '\\')
	{	LPCSTR s = Begin();
		LPCSTR end = End();
		SIZE_T replace_count = 0;
		for(;s<end;s++)
			if(*s == escape_sign || characters_to_be_escaped.Has(*s))replace_count++;
		if(replace_count)
		{	VERIFY(SetLength(GetLength()+replace_count));
			s = End()-1-replace_count; end = Begin()-1;
			LPSTR p = &Last();
			for(;s > end;s--)
			{	if(*s == escape_sign || characters_to_be_escaped.Has(*s))
				{	*p-- = characters_to_be_escaped.Mapped(*s);
					*p-- = escape_sign;
				}
				else *p-- = *s;
			}
			ASSERT(p == _p-1);
		}
		return *this;
	}
	const String& TrimCodeComments(const rt::String_Ref& code) // for C/C++/Java/Javascript/Json code
	{
		if(code.IsEmpty()){	Empty(); return *this; }
		rt::String output;
		output.SetLength(code.GetLength());
		LPSTR out = output;
		LPCSTR p = code.Begin();
		LPCSTR end = code.End();
		while(p<end-1)
		{	
			if(p[0] == '"' || p[0] == '\'')	// copy 
			{	int quotation = p[0];
				*out++ = quotation;
				for(p++;p<end;p++)
				{	*out++ = *p;
					if(p[0] == quotation)
					{
						LPCSTR s = p-1;
						for(;*s == '\\';s--);
						if(((int)(s-p))&1)goto NEXT_SEGMENT;
					}
				}
				break;
			}
			if(p[0] == '/' && p[1] == '/')
			{	p+=2;
				while(p<end && (p[0]!='\r' && p[0]!='\n'))p++;
				if(out[-1] == '\r' || out[-1] == '\n')
					while(p<end && (p[0]=='\r' || p[0]=='\n'))p++;
				continue;
			}
			if(p[0] == '/' && p[1] == '*')
			{	p+=2;
				while(p<end && !(p[-1]=='*' && p[0]=='/'))p++;
				p++;
			}
			*out++ = *p;
		NEXT_SEGMENT:
			p++;
		}
		if(p<end) *out++ = *p;
		*this = rt::String_Ref(output, out);
		return *this;
	}
	INLFUNC void Insert(SIZE_T pos, char c)
	{	SIZE_T org_len = GetLength();
		ASSERT(pos<=GetLength());
		SetLength(GetLength() + 1);
		memmove(_p + pos + 1, _p + pos, org_len - pos);
		_p[pos] = c;
	}
	INLFUNC void Insert(SIZE_T pos, const rt::String_Ref& sub_str)
	{	SIZE_T org_len = GetLength();
		ASSERT(pos<=org_len);
		VERIFY(SetLength(org_len + sub_str.GetLength()));
		memmove(_p + pos + sub_str.GetLength(), _p + pos, org_len - pos);
		memcpy(_p + pos, sub_str.Begin(), sub_str.GetLength());
	}
	INLFUNC void SetDefaultExtName(const rt::String_Ref& extname)
	{	if(GetExtName().IsEmpty())
		{	if(extname[0] != '.')*this += '.';
			*this += extname;
		}
	}
	INLFUNC String& RegularizeUTF8()
	{	String_Ref::RegularizeUTF8();
		if(_len)_p[_len-1] = 0;
		return *this;
	}
	INLFUNC String& DecodedURL()	// inplace precent-decoding
	{	rt::String_Ref::DecodedURL();
		if(_len)_p[_len-1] = 0;
        return *this;
	}
	INLFUNC String& Shorten(UINT n = 1)
	{	SetLength(rt::max<SSIZE_T>(0, ((SSIZE_T)GetLength()) - n));
		return *this;
	}
	INLFUNC bool EndClosure(char closure_symb = ']') // false indicate the closure is empty
	{	ASSERT(!IsEmpty());
		if(Last() == ',' || Last() == '.' || Last() == ';'){ Last() = closure_symb; return true; }
		*this += closure_symb; return false;
	}
};
//template<class t_Ostream>
//t_Ostream& operator<<(t_Ostream& Ostream, const rt::String & vec)
//{	Ostream << (vec.GetSafeString());
//	return Ostream;
//}

#undef CPF_STRING_CONNECT_OP
#undef CPF_STRING_CONNECT_OP_
#undef CPF_STRING_EXPR_CONNECT_OP

template<> class TypeTraits<String>
{
public:	
	typedef String t_Val;
	typedef char t_Element;
	typedef String t_Signed;
	typedef String t_Unsigned;
	static const int Typeid = _typeid_string;
	static const bool IsPOD = false;
	static const bool IsNumeric = false;
};

template<> class TypeTraits<String_Ref>
{
public:	
	typedef String_Ref t_Val;
	typedef char t_Element;
	typedef String_Ref t_Signed;
	typedef String_Ref t_Unsigned;
	static const int Typeid = _typeid_string;
	static const bool IsPOD = false;
	static const bool IsNumeric = false;
};

}

namespace rt
{
namespace _details
{
template<typename T>
String_Ref __alloca_string_ref(LPSTR p, const T& x)
{	UINT len;
	len = (UINT)x.CopyTo(p);
	return String_Ref(p, len);
}
}} // namespace rt::_details


namespace rt
{

namespace tos
{

typedef ::rt::tos::S_<> Number;

struct WhiteSpace:public ::rt::tos::S_<>
{	INLFUNC WhiteSpace(int len)
	{	ASSERT(len+1 < sizeof(_string));
		for(int i=0;i<len;i++)_p[i] = ' ';
		_p[len] = '\0';
		_len = len+1;
	}
};

struct character:public ::rt::tos::S_<>
{	INLFUNC character(char c)
	{	_p[0] = c;	_p[1] = '\0';
		_len = 2;
	}
};

struct StaticString: public ::rt::String_Ref
{
	INLFUNC StaticString(){}
	template<SIZE_T LEN>
	INLFUNC StaticString(char Str[LEN])
		: ::rt::String_Ref(Str, ((SIZE_T)LEN-1))
	{}
	template<SIZE_T LEN>
	INLFUNC StaticString(const char Str[LEN])
		: ::rt::String_Ref(Str, ((SIZE_T)LEN-1))
	{}
	INLFUNC StaticString(const char* p, SIZE_T len): ::rt::String_Ref(p, len){}
	//INLFUNC StaticString(LPCSTR x)    // don't define those, which will always be hit
	//INLFUNC StaticString(LPSTR x)
	template<typename T>
	INLFUNC StaticString(T& x)
		: ::rt::String_Ref((LPCSTR)&x, ((SIZE_T)(sizeof(x)-1)))
	{}
	operator LPCSTR () const { return _p; }
};

struct DataAsString: public ::rt::String_Ref
{
	template<typename T>
	INLFUNC DataAsString(const T& x)
		: ::rt::String_Ref((LPCSTR)&x, (SIZE_T)(sizeof(x)))
	{}
	INLFUNC DataAsString(LPCVOID x, UINT bytes)
		: ::rt::String_Ref((LPCSTR)x, bytes)
	{}
};

struct StdStringAsString: public ::rt::String_Ref
{
	template<typename std_string>
	INLFUNC StdStringAsString(const std_string& str)
		: ::rt::String_Ref(str.data(), str.size())
	{}
};

template<UINT LEN = 256>
struct StringOnStack:public ::rt::tos::S_<1,LEN>
{	typedef ::rt::tos::S_<1,LEN> _SC;
	template<class T>
    INLFUNC StringOnStack(const T& x)
	{	_SC::_len = x.CopyTo(_SC::_p);
		ASSERT(LEN>=_SC::_len);
		_SC::_p[_SC::_len] = 0;
		_SC::_len++;
	}
	INLFUNC operator LPCSTR() const { return _SC::_p; }
	INLFUNC StringOnStack<LEN>& operator += (const rt::String_Ref& x)
	{	ASSERT(x.GetLength() + _SC::GetLength() < LEN - 1);
		memcpy(&_SC::_p[_SC::_len-1], x.Begin(), x.GetLength());
		_SC::_len+=x.GetLength();
		_SC::_p[_SC::_len-1] = 0;
		return *this;
	}
};

template<bool upper_case = true, bool compact = false, char unit = 'B'>
struct FileSize:public ::rt::tos::S_<>
{	
	INLFUNC FileSize(ULONGLONG filesize)
	{
		int up = upper_case?0:('a' - 'A');
		rt::String_Ref sep;
		if(!compact)sep = " ";

		if(filesize < 1024)
			SetString(rt::String_Ref() + (UINT)(filesize) + sep + (char)(unit));
		else if(filesize < 1024*1024)
			SetString(rt::String_Ref() + (float)(filesize/1024.f) + sep + (char)('K'+up) + (char)(unit));
		else if(filesize < 1024*1024*1024)
			SetString(rt::String_Ref() + (float)(filesize/1024/1024.f) + sep + (char)('M'+up) + (char)(unit));
		else if(filesize < 1024LL*1024*1024*1024)
			SetString(rt::String_Ref() + (float)(filesize/1024/1024/1024.f) + sep + (char)('G'+up) + (char)(unit));
		else
			SetString(rt::String_Ref() + (float)(filesize/1024/1024/1024/1024.f) + sep + (char)('T'+up) + (char)(unit));
	}
private:
	template<class T>
	INLFUNC void SetString(const T& x)
	{	ASSERT(x.GetLength() < sizeofArray(_string)-1);
		_len = x.CopyTo(_string);
		_string[_len] = '\0';
		_len++;
	}
};

template<bool _smart = true>
struct TimeSpan:public ::rt::tos::S_<>
{	template<typename time_type>
	INLFUNC TimeSpan(time_type msec)
	{	char* p = _string;
		if(msec < 0){ p[0]='-'; p++; msec = (time_type)(-(typename rt::TypeTraits<time_type>::t_Signed)msec); }  // just for making compiler happy 
		if(msec >= 10000)
		{	msec = (time_type)(msec/1000.0f + 0.5f);
			if(msec >= 3600*24)
			{	p+=sprintf(p,"%d",(int)(msec/(3600*24)));
				p[0]='d';	p++;
				if(_smart && (msec%(3600*24)) == 0)
				{	p[-1] = ' '; p[0] = 'd';	p[1] = 'a';	p[2] = 'y';	p+=3;
					if(msec >= 3600*48){p[0] = 's';	p++;  }
					goto END_OF_PRINT;
				}
				p[0]=':';	p++;
			}
			if(msec >= 3600)
			{	p+=sprintf(p,"%d",(int)((msec%(3600*24))/3600));
				p[0]='h';	p++;
				if(_smart && (msec%3600) == 0)
				{	if(msec < 3600*24)
					{	p[-1] = ' '; p[0] = 'h';	p[1] = 'o';	p[2] = 'u';	p[3] = 'r';	p+=4;
						if(msec >= 7200){   p[0] = 's';	p++; }
					}
					goto END_OF_PRINT;
				}
				p[0]=':';	p++;
			}
			if(msec >= 60)
			{	p+=sprintf(p,"%d",(int)((msec%(3600))/60));
				p[0]='m';	p++;
				if(_smart && (msec%60) == 0)
				{	if(msec < 3600)
					{	p[-1] = ' '; p[0] = 'm';	p[1] = 'i';	p[2] = 'n';
						p[3] = 'u';	p[4] = 't';	p[5] = 'e';	p+=6;
						if(msec >= 120){ p[0] = 's'; p++; }
					}
					goto END_OF_PRINT;
				}
				p[0]=':';	p++;
			}
			p+=sprintf(p,"%d",(int)(msec%60));
			p[0]='s';				p++;
			if(_smart && msec < 60)
			{	p[-1]=' '; p[0]='s'; p[1]='e'; p[2]='c'; p[3]='o'; p[4]='n'; p[5]='d'; p+=6;
				if(msec > 1){ p[0]='s'; p++; }
			}
		}
		else
		{	
			p[0] = ('0' + (int)(msec/1000));	p++;
			int frac = (int)(msec%1000);
			if(frac)
			{	p[0] = '.';	p++;
				p[0] = ('0' + (int)(msec%1000)/100);
				p[1] = ('0' + (int)(msec%100)/10);
				p[2] = ('0' + (int)(msec%10));
				p+=3;
			}
			if(!_smart){ p[0] = 's'; p++; }
			else
			{	p[0]=' '; p[1]='s'; p[2]='e'; p[3]='c'; p[4]='o'; p[5]='n'; p[6]='d'; p+=7;
				if(msec >= 2000){ p[0]='s'; p++; }
			}
		}
END_OF_PRINT:
		p[0] = '\0';
		_len = 1+(int)(p-_string);
	}
};


template<int SEP = '/',bool no_year = false>
struct Date:public ::rt::tos::S_<>
{
	template<typename t_os_Date32>
	INLFUNC Date(const t_os_Date32& x)
	{	LPCSTR print_template = "%04d/%02d/%02d";
		if(!no_year)
		{
			_len = 1+sprintf(_string,print_template, x.GetYear(), x.GetMonth(), x.GetDay());
			if(SEP != '/')
			{	_string[4] = _string[7] = SEP;
			}
		}
		if(no_year)
		{
			_len = 1+sprintf(_string,print_template+5, x.GetMonth(), x.GetDay());
			if(SEP != '/')
			{	_string[2] = SEP;
			}
		}
	}
};

template<SIZE_T LEN = 65, bool uppercase = true>
struct HexNum:public ::rt::tos::S_<1,LEN>
{
	template<typename T>
	INLFUNC HexNum(const T& x){ new (this) HexNum(&x, sizeof(x)); }
	INLFUNC HexNum(LPCVOID pbyte, int len)
	{	
		const char char_base = uppercase?'A':'a';
		typedef ::rt::tos::S_<1,LEN> _SC;
		ASSERT(len < (int)sizeofArray( _SC::_string)/2);
		LPSTR p =  _SC::_string;
		int i=0;
		for(;i<len-1 && ((LPCBYTE)pbyte)[len - i - 1] == 0; i++);
		for(;i<len;i++,p+=2)
		{
			BYTE v = ((LPCBYTE)pbyte)[len - i - 1];
			p[0] = (v>=0xa0?char_base+((v-0xa0)>>4):'0'+(v>>4));
			v = v&0xf;
			p[1] = (v>=0xa?char_base+(v-0xa):'0'+v);
		}
		*p = 0;
		 _SC::_len = 1+(p-_SC::_string);
		 _SC::_p[ _SC::_len-1] = 0;
	}
};

template<SIZE_T LEN = 65, bool uppercase = true>
struct Binary:public ::rt::tos::S_<1,LEN>
{
	template<typename T>
	INLFUNC Binary(const T& x){ new (this) Binary(&x, sizeof(x)); }
	INLFUNC Binary(LPCVOID pbyte, int len)
	{	
		const char char_base = uppercase?'A':'a';
		typedef ::rt::tos::S_<1,LEN> _SC;
		ASSERT(len < (int)sizeofArray( _SC::_string)/2);
		LPSTR p =  _SC::_string;
		for(int i=0;i<len;i++,p+=2)
		{
			BYTE v = ((LPCBYTE)pbyte)[i];
			p[0] = (v>=0xa0?char_base+((v-0xa0)>>4):'0'+(v>>4));
			v = v&0xf;
			p[1] = (v>=0xa?char_base+(v-0xa):'0'+v);
		}
		 _SC::_len = 1+len*2;
		 _SC::_p[ _SC::_len-1] = 0;
	}
};

template<SIZE_T LEN = 65>
struct BinaryCString:public ::rt::tos::S_<1,LEN>
{
	template<typename T>
	INLFUNC BinaryCString(const T& x){ new (this) BinaryCString(&x, sizeof(x)); }
	INLFUNC BinaryCString(LPCVOID pbyte, int len)
	{	
		typedef ::rt::tos::S_<1,LEN> _SC;
		ASSERT(len < (int)sizeofArray( _SC::_string)/2);
		LPSTR p =  _SC::_string;
		for(int i=0;i<len;i++,p+=4)
		{
			p[0] = '\\';
			p[1] = 'x';
			BYTE v = ((LPCBYTE)pbyte)[i];
			p[2] = (v>=0xa0?'a'+((v-0xa0)>>4):'0'+(v>>4));
			v = v&0xf;
			p[3] = (v>=0xa?'a'+(v-0xa):'0'+v);
		}
		 _SC::_len = 1+len*4;
		 _SC::_p[ _SC::_len-1] = 0;
	}
};

struct GUID:public ::rt::tos::S_<>
{
	INLFUNC GUID(const ::GUID& guid)
	{
		if((&guid) != nullptr)
        {	LPCSTR print_template = "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x";
			_len = 1+sprintf(_string,print_template, guid.Data1,guid.Data2,guid.Data3,
							 guid.Data4[0],guid.Data4[1],guid.Data4[2],guid.Data4[3],
							 guid.Data4[4],guid.Data4[5],guid.Data4[6],guid.Data4[7]);
		}
		else
		{	memcpy(_string,"NULL",5);
			_len = 5;
		}
	}
};

} // tos

typedef tos::StaticString SS;
typedef tos::DataAsString DS;
typedef tos::StdStringAsString	StdStr;

// typename to string
namespace _details
{

template<typename __TNAS_T>
struct __TNAS
{	static LPCSTR __TNAS_END()
	{
#if defined(PLATFORM_WIN)
		return __FUNCTION__; 
#else
		return __PRETTY_FUNCTION__;
#endif
	}
};

} // namespace _details

template<typename T>
INLFUNC rt::String_Ref TypeNameToString()
{
	LPCSTR raw_name = _details::__TNAS<T>::__TNAS_END();
	raw_name = strstr(raw_name, "rt::_details::__TNAS<");			ASSERT(raw_name);
	raw_name += 21;
	LPCSTR end = strstr(raw_name, ">::__TNAS_END");					ASSERT(end);
	if(rt::String_Ref(raw_name, end) == rt::SS("__TNAS_T"))
	{	raw_name = strstr(end, "__TNAS_T = ");						ASSERT(raw_name);
		raw_name += 11;
		end = strchr(raw_name,';');
	}
	if(memcmp(raw_name, "struct ", 7) == 0){ raw_name += 7; }
	else if(memcmp(raw_name, "class ", 6) == 0){ raw_name += 6; }
	else if(memcmp(raw_name, "enum ", 5) == 0){ raw_name += 5; }
	rt::String_Ref ret(raw_name, end);
	// check if nested type defined within function scope, and fullback to clang's behavior 
	return ret.Occurrence("()") >= 2?ret.TrimBeforeReverse("):"):ret;
}

namespace _details
{
template<typename T>
LPSTR __alloca_string(LPSTR p, const T& x)
{	p[x.CopyTo(p)] = 0;
	return p;
}
} // namespace rt::_details

} // namespace rt

#define __SS(...)						(rt::SS(#__VA_ARGS__))
#define ALLOCA_STRING_BUFFER(size)		(rt::String_Ref((LPSTR)alloca(size), size))
#define ALLOCA_STRING_REF(x)			(rt::_details::__alloca_string_ref((LPSTR)alloca((x).GetLength()), (x)))	// x should be a varible, instead of a expression. use auto x = ..... if it need to be an expression
#define ALLOCA_C_STRING(x)				(rt::_details::__alloca_string((LPSTR)alloca((x).GetLength() + 1), (x)))	// x should be a varible, instead of a expression. use auto x = ..... if it need to be an expression
