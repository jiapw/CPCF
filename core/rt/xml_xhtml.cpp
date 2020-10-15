#include "xml_xhtml.h"

#define MAKE_FOURCC(a,b,c,d)	(a|(b<<8)|(c<<16)|(d<<24))

namespace rt
{

namespace _details
{
	template<typename t_Val>
	static	t_Val*	_skip_whitespace(t_Val* start) { while(*start>0 && *start <=' ')start++; return start; }
	template<typename t_Val>
	static	t_Val*	_skip_attribute_noise(t_Val* start)
	{	while(	(*start>0 && *start<'A' && *start!='/' && *start!='>') ||
				(*start>'Z' && *start<'a') ||
				(*start>'z')
		)start++; return start;
	}
}

const char XMLComposer::g_XML_Header[40] = "<?xml version=\"1.0\" encoding=\"utf-8\" ?>";

XMLComposer::XMLComposer(rt::_File * out_stream)
{
	if(out_stream)
	{
		m_StreamOutOrgPos = out_stream->Seek(0,rt::_File::Seek_Current);
		m_pStreamOut = out_stream;
	}
	else
	{
		m_StreamOutOrgPos = 0;
		m_pStreamOut = nullptr;
	}
	ResetContent();
}

void XMLComposer::Linebreak(int vary)
{
	Output('\15');
	Output('\12');

	for(UINT i = 0; i<m_NestLevelTag.GetSize() + vary; i++)
	{
		Output(' ');
		Output(' ');
		Output(' ');
		Output(' ');
	}
}

void XMLComposer::ResetContent(LPCSTR customized_header)
{
	if(m_pStreamOut)
	{
		m_pStreamOut->Seek(m_StreamOutOrgPos);
	}
	else m_Output.Empty();

	LPCSTR header = customized_header ? customized_header : g_XML_Header;
	Output(header);
	m_HeaderLength = (UINT)rt::String_Ref(header).GetLength();
	
	m_NestLevelTag.ShrinkSize(0);
	_EnteringNodeTag = nullptr;
}

void XMLComposer::EnterNode(LPCSTR tag)
{
	EnteringNode(tag);
	EnteringNodeDone();
}

void XMLComposer::EnteringNode(LPCSTR tag)
{
	ASSERT(!_EnteringNodeTag);

	Linebreak();
	Output('<');
	Output(tag);
	_EnteringNodeTag = tag;
}
 
void XMLComposer::EnteringNodeDone(bool compact)
{
	ASSERT(_EnteringNodeTag); 

	if(compact)
	{
		_DepthMax = 1 + (UINT)m_NestLevelTag.GetSize();
    	Output(' ');
		Output('/');
		Output('>');
	//	Linebreak(-1);
	}
	else
	{
		_TagCache& tag = m_NestLevelTag.push_back();
		tag.tag = _EnteringNodeTag;
		tag.tag_pos = GetDocumentLength();
		_DepthMax = (UINT)m_NestLevelTag.GetSize();
		Output('>');
	}
	
	_EnteringNodeTag = nullptr;
}

void XMLComposer::SetAttribute(LPCSTR name, long long value)
{
	SetAttribute(name, rt::tos::Number((LONGLONG)value).Begin());
}

void XMLComposer::SetAttribute(LPCSTR name, int value)
{
	SetAttribute(name, rt::tos::Number(value).Begin());
}

void XMLComposer::SetAttribute(LPCSTR name, unsigned long long value)
{
	SetAttribute(name, rt::tos::Number((ULONGLONG)value).Begin());
}

void XMLComposer::SetAttribute(LPCSTR name, unsigned int value)
{
	SetAttribute(name, (unsigned long long)value);
}

void XMLComposer::SetAttribute(LPCSTR name, float value)
{	
	SetAttribute(name, rt::tos::Number(value).Begin());
}

void XMLComposer::SetAttribute(LPCSTR name, double value)
{
	SetAttribute(name, rt::tos::Number(value).Begin());
}

void XMLComposer::SetAttribute(LPCSTR name, LPCSTR value)
{
	ASSERT(_EnteringNodeTag);
	ASSERT(name);

	if (_EnteringNodeTag && name)
	{	Output(' ');
		Output(name);
		Output('=');
		Output('"');
		if(value)_InsertPlainText(value);
		Output('"');
	}
}

void XMLComposer::AddCData(LPCSTR cdata)
{
	ASSERT(cdata);
	ASSERT(!_EnteringNodeTag);

	if (cdata && !_EnteringNodeTag)
	{	Linebreak();
		Output("<![CDATA[");
		Output(cdata);
		Output("]]>");
	}
}

void XMLComposer::AddText(LPCSTR text)
{	
	ASSERT(text);
	ASSERT(!_EnteringNodeTag);

	if (text && !_EnteringNodeTag)
	{	_InsertPlainText(text);
	}
}

void XMLComposer::AddXML(const XMLComposer& another)
{
	LPCSTR xml = another.GetDocumentBuffer(false);
	LPCSTR xmlEnd = xml + another.GetDocumentLength(false);

	// see how many leading spaces are there
	int leadingSpaces = 0;
	while (xml[leadingSpaces] == ' ') leadingSpaces++;

	if (!xml[leadingSpaces]) return; // 'another' only has spaces

	LPCSTR p = xml, nextp;
	do
	{	// skip leading spaces
		for (int i = 0; i < leadingSpaces; i++, p++) ASSERT(*p == ' ');

		int q = (int)rt::String_Ref(p).FindCharacter('\r');
		if (q == -1)
		{	q = (int)(xmlEnd - p);
			nextp = xmlEnd;
		}
		else
		{	nextp = p + q + 2;
		}

		Linebreak();
		Output(ALLOCA_C_STRING(rt::String_Ref(p, q)));

		p = nextp;
	}
	while (p != xmlEnd);

	_DepthMax = 1 + (UINT)m_NestLevelTag.GetSize();
}


void XMLComposer::_InsertPlainText(LPCSTR text)
{
	while(*text)
	{
		switch(*text)
		{
		case '<':	Output("&lt;");		break;
		case '&':	Output("&amp;");	break;
		case '>':	Output("&gt;");		break;
		case '\"':	Output("&quot;");	break;
		case '\'':	Output("&apos;");	break;
		default:	Output(*text);
		}
		text++;
	}
}

void XMLComposer::ExitNode()
{	
	ASSERT(m_NestLevelTag.GetSize());
	UINT level = (UINT)(m_NestLevelTag.GetSize()-1);
	if(m_NestLevelTag[level].tag_pos == GetDocumentLength())
	{	//empty node, end with "/>"
		if(m_pStreamOut)
		{
			m_pStreamOut->Seek(-1,rt::_File::Seek_Current);
			m_WrittenLength--;
			Output(' ');
			Output('/');
			Output('>');
		}
		else
		{
			m_Output += ' ';
			m_Output += '/';
			m_Output += '>';
		}
	}
	else
	{
		if(_DepthMax > m_NestLevelTag.GetSize())
			Linebreak(-1);

		Output('<');
		Output('/');
		Output(m_NestLevelTag[level].tag);
		Output('>');
	}
	m_NestLevelTag.pop_back();
}

void XMLComposer::AppendTrail(LPCSTR text)
{	
	ASSERT(m_NestLevelTag.GetSize()==0);
	Output(text);
}

LPCSTR XMLComposer::GetDocumentBuffer(bool withHeader) const
{
	ASSERT(m_pStreamOut == nullptr);
	ASSERT(m_NestLevelTag.GetSize()==0);
	LPCSTR buf = m_Output.Begin();
	LPCSTR buf2 = buf + m_HeaderLength + 2; // +2 for \r\n
	return withHeader ? buf : buf2;
}

UINT XMLComposer::GetDocumentLength(bool withHeader) const
{
	UINT ret = m_pStreamOut
			 ? m_WrittenLength
			 : (UINT)m_Output.GetLength();
	if (!withHeader) ret -= m_HeaderLength + 2; // +2 for \r\n
	return ret;
}

void XMLParser::ClearSyntaxError()
{
	m_XMLParseError = ERR_XML_OK;
	m_XMLParseErrorPosition = 0;
}

#pragma warning(disable:4355)

XMLParser::XMLParser():m_XPathParser(*this)
{
	SetUserTagFilter();
	m_pCurTagFilter = nullptr;
	_attribute_cursor = nullptr;
	_root_node_xml_start = nullptr;
	m_pDocument = nullptr;
	m_bTrySkipError = false;
	ClearSyntaxError();
}

XMLParser::XMLParser(const XMLParser& xml):m_XPathParser(*this)
{
	ClearSyntaxError();
	_attribute_cursor = nullptr;

	*this = xml;
}

#pragma warning(default:4355)

bool XMLParser::GetNodeDocument(rt::String& doc_out)
{
	LPCSTR p = _CurNode().OuterXML_Start;
	while(p > m_pDocument && (p[-1] == '\t' || p[-1] == ' ') )p--;

	LPCSTR pend;
	if(_CurNode().IsCompactNode)
		pend = _seek_tag_close(_CurNode().OuterXML_Start);
	else
		pend = _search_node_close(_CurNode().InnerXML_Start,_CurNode().TagName);

	if(pend)
	{
		doc_out = rt::String_Ref(XMLComposer::g_XML_Header,sizeof(XMLComposer::g_XML_Header)-1) + 
				  '\r' + '\n' + rt::String_Ref(p, pend + 1);
        return true;
	}
	else
	{	m_XMLParseError = ERR_NODE_CLOSURE_NOT_MATCHED;
		m_XMLParseErrorPosition = (int)(p-_root_node_xml_start);
		return false;
	}
}

rt::String_Ref XMLParser::GetInternalDocument() const
{
	return rt::String_Ref(m_pDocument, _root_node_xml_end);
}

XMLParser XMLParser::GetNodeDocument(int nth_parent) const
{
	XMLParser ret;
	ret.SetUserTagFilter(m_pUserTagFilter);
	ret.m_pCurTagFilter = m_pUserTagFilter;

	ASSERT(nth_parent < (int)m_NodePath.GetSize());
	const _node& out = m_NodePath[m_NodePath.GetSize() - 1 - nth_parent];

	ret._root_node_xml_start = out.OuterXML_Start;
	if(out.IsCompactNode)
		ret._root_node_xml_end = _seek_tag_close(out.OuterXML_Start);
	else
		ret._root_node_xml_end = _search_node_close(out.InnerXML_Start,out.TagName);

	ret.m_pDocument = out.OuterXML_Start;
	ret.m_NodePath.SetSize(1);
	ret.m_NodePath[0] = out;

	return ret;
}


const XMLParser& XMLParser::operator = (const XMLParser& xml)
{
	_content_copy.Empty();
	ClearSyntaxError();

	LONGLONG ptr_shift = 0;
	if(!xml._content_copy.IsEmpty())
	{	_content_copy = xml._content_copy;
		ptr_shift = _content_copy.Begin() - xml._content_copy.Begin();
	}

	_attribute_cursor = nullptr;
	_root_node_xml_start = xml._root_node_xml_start + ptr_shift;

#if defined(PLATFORM_32BIT)
	if(xml._root_node_xml_end == (LPCSTR)0xffffffff)
		_root_node_xml_end = (LPCSTR)0xffffffff;
	else
		_root_node_xml_end = xml._root_node_xml_end + ptr_shift;
#elif defined(PLATFORM_64BIT)
	if(xml._root_node_xml_end == (LPCSTR)0xffffffffffffffff)
		_root_node_xml_end = (LPCSTR)0xffffffffffffffff;
	else
		_root_node_xml_end = xml._root_node_xml_end + ptr_shift;
#else
	ASSERT(0);
#endif
	
	SetUserTagFilter(xml.m_pUserTagFilter);
	if(xml.m_pCurTagFilter == &xml.m_XPathParser)
	{
		m_XPathParser._pXPath = nullptr;
		m_XPathParser.m_bRelativePath = xml.m_XPathParser.m_bRelativePath;
		m_XPathParser.m_bIncludeDescendants = xml.m_XPathParser.m_bIncludeDescendants;
		m_XPathParser.m_FinalQualifier = xml.m_XPathParser.m_FinalQualifier;
		m_XPathParser.m_UpLevelCount = xml.m_XPathParser.m_UpLevelCount;
		m_XPathParser.m_QualifierShifts = xml.m_XPathParser.m_QualifierShifts;

		m_XPathParser.m_Qualifiers.SetSize(xml.m_XPathParser.m_Qualifiers.GetSize());
		m_XPathParser.m_Qualifiers = xml.m_XPathParser.m_Qualifiers;

		m_pCurTagFilter = &m_XPathParser;
	}
	else
		m_pCurTagFilter = xml.m_pCurTagFilter;

	m_pDocument = xml.m_pDocument + ptr_shift;
	m_NodePath.SetSize(xml.m_NodePath.GetSize());
	for(UINT i=0;i<m_NodePath.GetSize();i++)
	{
		if(!xml.m_NodePath[i].TagName.IsEmpty())
			m_NodePath[i].TagName = rt::String_Ref(xml.m_NodePath[i].TagName.Begin() + ptr_shift, xml.m_NodePath[i].TagName.GetLength());
		if(!xml.m_NodePath[i].Attributes.IsEmpty())
			m_NodePath[i].Attributes = rt::String_Ref(xml.m_NodePath[i].Attributes.Begin() + ptr_shift, xml.m_NodePath[i].Attributes.GetLength());
		m_NodePath[i].OuterXML_Start = xml.m_NodePath[i].OuterXML_Start + ptr_shift;
		m_NodePath[i].InnerXML_Start = xml.m_NodePath[i].InnerXML_Start + ptr_shift;
		m_NodePath[i].IsCompactNode = xml.m_NodePath[i].IsCompactNode;
	}

	return xml;
}

LPCSTR XMLParser::SetLastSyntaxError(XMLParseError errnum, LPCSTR pos)
{
	m_XMLParseError = errnum;
	if(pos)
	{
		ASSERT(pos >= m_pDocument);
		m_XMLParseErrorPosition = (UINT)(pos - m_pDocument);
	}
	else
		m_XMLParseErrorPosition = 0;

	//ASSERT(0);
	//_LOG("XML Parse Error: "<<errnum<<'\n');
	//_asm int 3
	return pos;
}

LPCSTR XMLParser::_search_control_close(LPCSTR start) const
{
	int enclosure = 0;
	for(;*start;start++)
	{
		if(*start != '<' && *start != '>'){}
		else
		{	enclosure -= *start - 0x3d;	// '<' = \x3c, '>' = \x3e
			if(enclosure == -1)
				return start;
		}
	}

	return nullptr;
}

LPCSTR XMLParser::_html_check_node_close(const rt::String_Ref& tagname, LPCSTR p, bool just_next)
{
SEARCH_AGAIN:
	LPCSTR c = strstr(p,"</");

	if(c)
	{	c+=2;

		UINT i=0;
		for(;i<tagname.GetLength();i++)
		{
			if(	tagname[i] == c[i] ||
				tagname[i] == c[i] - 'A' + 'a'
			){}
			else
			{
				if(!just_next)
				{
					p = c+2;
					goto SEARCH_AGAIN;
				}
				else
					return nullptr;
			}
		}

		if(c[i] == '>' || c[i] <=' ')
			return c-2;
	}

	return nullptr;
}


LPCSTR XMLParser::_search_special_node_close(XMLParseError* pErrorCode, LPCSTR start)
{
	*pErrorCode = ERR_XML_OK;

	ASSERT(*start == '<'); // must pointing to outer xml
	// special nodes
	LPCSTR ending_symbol;
	int ending_symbol_length;
	LPCSTR start_pos;
	XMLParseError errnum;

	if( *((DWORD*)(start)) == 0x2d2d213c )
	{	// <!-- , comments
		start_pos = start + 4;
		ending_symbol = "-->";
		ending_symbol_length = 3;
		errnum = ERR_COMMENT_CLOSURE_NOT_MATCHED;
	}
	else if(*((WORD*)(start)) == 0x3f3c)
	{	// <?, processing instruction
		start_pos = start + 2;
		ending_symbol = "?>";
		ending_symbol_length = 2;
		errnum = ERR_PROC_INST_CLOSURE_NOT_MATCHED;
	}
	else if(*((DWORD*)(start+1)) == 0x44435b21 && *((DWORD*)(start+5)) == 0x5b415441)
	{	// <![CDATA
		start_pos = start + 9;
		ending_symbol = "]]>";
		ending_symbol_length = 3;
		errnum = ERR_CDATA_CLOSURE_NOT_MATCHED;
	}
	else
	{	// <! 
		ASSERT(start[1] == '!');
		LPCSTR p = start + 1;
		for(;*p && *p!='>' && *p!='[';p++)
			if(*p == '<')
			{	
				*pErrorCode = ERR_XML_SYMBOL_UNEXPECTED;
				return p;
			}

		if(*p)
		{	if(*p == '>'){ return p; }	// compact <!
			else
			{	ASSERT(*p == '[');
				start_pos = start + 2;
				ending_symbol = "]>";
				ending_symbol_length = 2;
				errnum = ERR_PROC_INST_CLOSURE_NOT_MATCHED;
			}
		}
		else
		{	*pErrorCode = ERR_XML_UNEXPECTED_ENDOFFILE;
			return start;
		}
	}

	LPCSTR p = strstr(start_pos,ending_symbol);
	if(p){ return p + ending_symbol_length - 1; }
	else
	{	*pErrorCode = errnum;
		return start;
	}

	ASSERT(0);
}

void XMLParser::_convert_xml_to_text(const rt::String_Ref& string, rt::String& text, bool TrimXMLCode, bool TrimSubnodes, char sep)
{
	if(TrimSubnodes)TrimXMLCode = true;

	VERIFY(text.SetLength(string.GetLength()));
	if(string.IsEmpty())return;	

	LPCSTR p = string.Begin();
	LPSTR d = text.Begin();
	LPCSTR pend = string.Begin() + string.GetLength();

	while(p<pend)
	{
		LPCSTR s;

		if(TrimXMLCode)
			for(s=p;s<pend && *s!='&' && *s!='<';s++)
			{	if(*s>' ' || (*s==' ' && d[-1]!=' ' && d[-1]!=sep)){ *d = *s;	d++; }
			}
		else
			for(s=p;s<pend && *s!='&';s++)
			{	if(*s>' ' || (*s==' ' && d[-1]!=' ' && d[-1]!=sep)){ *d = *s;	d++; }
			}

		if(s == pend)break;

		if(*s == '&')
		{	if(s[1] == '#')
			{	
				if(s[2]>='0' && s[2]<='9')
				{
					s+=2;
					int a = 0; // a is a uft16 character
					while(*s>='0' && *s<='9')
					{	a = a*10 + (*s - '0');
						s++;
					}
					if(*s==';')s++;

					if(a <= 0x7f)	// translate to utf8 character
					{	*d = (char)a;
						a++;
					}
					else if(a > 0x7ff)	// 1110xxxx 	10xxxxxx 	10xxxxxx
					{	*((DWORD*)d) = 0x8080e0 | ((a>>12)&0xf) | ((a<<2)&0x3f00) | ((0x3f&a)<<16);
						p+=3;
					}
					else	// 110xxxxx 	10xxxxxx
					{	*((WORD*)d) = 0x80c0 | ((a>>6)&0x1f) | ((0x3f&a)<<8);
						d+=2;
					}
				}
				else
				{	d[0] = '&';	d[1] = '#';
					d+=2;
					s+=2;
				}
			}
			else
			{
				switch(*((DWORD*)s))
				{	
				case 0x3b746c26: *d = '<'; d++; s+=4; break;  // &lt;
				case 0x706d6126: *d = '&'; d++; s+=5; break;  // &amp; 
				case 0x3b746726: *d = '>'; d++; s+=4; break;  // &gt;
				case 0x6f757126: *d = '\"'; d++; s+=6; break; // &quot; 
				case 0x6f706126: *d = '\''; d++; s+=6; break; // &apos; 
				default: *d = *s; d++; s++;
				}
			}
		}
		else if(*s == '<')
		{	
			while(d[-1] == ' ')d--;
			
			LPCSTR close = _seek_tag_close(s+1);
			if(close)
			{	
				if(	((s[1] == 'b' || s[1] == 'B') && (s[2] == 'r' || s[2] == 'R') && s[3]<'@') ||
					((s[1] == 'p' || s[1] == 'P') && s[2]<'@')
				)
				{	d[0] = '\r'; d[1] = '\n'; d+=2;
				}
				else
					if(d[-1] != sep){ d[0] = sep; d++; };

				if(TrimSubnodes && close[-1] != '/')
				{	
					LPCSTR tag_end = _seek_symbol_end(s+1);

					XMLParseError err;
					LPCSTR close_tag = _search_node_close(&err, close, rt::String_Ref(s+1, tag_end));
					if(err == ERR_XML_OK)
					{	p = close_tag+1;
						continue;
					}
				}

				p = close+1;
				continue;
			}
			else break;
		}
		else break;

		p = s;
	}

	text.SetLength((UINT)(d - text.Begin()));

	if(TrimXMLCode)
		text = text.TrimSpace();
}

void XMLParser::GetXPathAsCSSPath(rt::String& out)
{
	out.Empty();
	for(UINT i=0;i<m_NodePath.GetSize();i++)
	{
		out += m_NodePath[i].TagName;

		if(i>=2)
		{	// skip html and body
			rt::String_Ref val;
			if(_search_attrib(m_NodePath[i].Attributes.Begin(),"id",val))
			{
				out += '#';
				out += val;
			}
			else if(_search_attrib(m_NodePath[i].Attributes.Begin(),"class",val))
			{
				rt::String_Ref c[256];
				UINT co = val.Split<true>(c,256,rt::CharacterSet_ControlChar(" "));
				for(UINT q=0;q<co;q++)
				{	out += '.';
					out += c[q];
				}
			}
		}

		out += ' ';
	}

	out.SetLength(out.GetLength()-1);
}

/*
LPCSTR XMLParser::_search_node_close(LPCSTR start_inner, const rt::String_Ref& tag_name, LPCSTR* pInnerXML_End) const
{
	int enclosure = 1;
	rt::String_Ref tagname_stack[_SEARCH_DEPTH_MAX];
	tagname_stack[0] = tag_name;

	LPCSTR start = start_inner;
	while(start = _details::_seek_tag_open(start))
	{
		if(start[1] != '!' && start[1] != '?')
		{	LPCSTR pend = _seek_tag_close(start+1);
			if(pend)
			{	if(start[1] == '/')
				{	if(pend[-1] == '/' && !m_bTrySkipError)
					{	rt::_CastToNonconst(this)->SetLastSyntaxError(ERR_XML_SYMBOL_UNEXPECTED,pend-1);
						return nullptr;
					}
					// check tag match
					LPCSTR p = _details::_skip_whitespace(start+2);
					rt::String_Ref tag(p,_seek_symbol_end(p));
					int encls = enclosure;
					int encls_stop = max(0,enclosure - _SEARCH_FAULT_TOLERANCE_SKIP_DEPTH_MAX);
					while(encls>encls_stop)
					{	encls--;
						if(tag==tagname_stack[encls])
						{	enclosure = encls;
							break;
						}
						if(!m_bTrySkipError)
						{	rt::_CastToNonconst(this)->SetLastSyntaxError(ERR_NODE_CLOSURE_NOT_MATCHED,pend-1);
							return nullptr;
						}
					}
					if(enclosure == 0)	// get it
					{	if(pInnerXML_End)*pInnerXML_End = start;
						return pend;
					}
				}
				else if(pend[-1] != '/')
				{	
					ASSERT(enclosure>=0);
					if(enclosure<_SEARCH_DEPTH_MAX)
					{	LPCSTR p = _details::_skip_whitespace(start+1);
						tagname_stack[enclosure] = rt::String_Ref(p,_seek_symbol_end(p));	}
					else
					{	rt::_CastToNonconst(this)->SetLastSyntaxError(ERR_XML_DEPTH_EXCEEDED,pend-1);
						return nullptr;
					}
					enclosure++;					
				}

				start = pend + 1;
				continue;
			}
			else
			{	rt::_CastToNonconst(this)->SetLastSyntaxError(ERR_TAG_CLOSURE_NOT_MATCHED,start);
				return nullptr;
			}
		}
		else
		{	
			LPCSTR p = _search_special_node_close(start);
			if(!p)return nullptr;
			start = p + 1;
		}
	}

	rt::_CastToNonconst(this)->SetLastSyntaxError(ERR_XML_UNEXPECTED_ENDOFFILE,start_inner);
	return nullptr;
}
*/

LPCSTR XMLParser::_search_node_close(XMLParseError* pErrorCode, LPCSTR start_inner, const rt::String_Ref& tag_name, LPCSTR* pInnerXML_End)
{
	*pErrorCode = ERR_XML_OK;
	LPCSTR start = start_inner;
	int enclosure = 0;
	while((start = _seek_tag_open(start)))
	{
		if(start[1] != '!' && start[1] != '?')
		{	LPCSTR pend = _seek_tag_close(start+1);
			if(pend)
			{	if(start[1] == '/')
				{	if(pend[-1] == '/')
					{	*pErrorCode = ERR_XML_SYMBOL_UNEXPECTED;
						return pend-1;
					}
					enclosure--;
					if(enclosure == -1)
					{	// get it
						if(memcmp(tag_name.Begin(),start+2,tag_name.GetLength()) == 0)
						{
							if(pInnerXML_End)*pInnerXML_End = start;
							return pend;
						}
					}
				}
				else if(pend[-1] != '/')
				{	enclosure++;
				}

				start = pend + 1;
				continue;
			}
			else
			{	*pErrorCode = ERR_TAG_CLOSURE_NOT_MATCHED;
				return start;
			}
		}
		else
		{	
			LPCSTR p = _search_special_node_close(pErrorCode, start);
			if(*pErrorCode != ERR_XML_OK)return p;
			start = p + 1;
		}
	}

	*pErrorCode = ERR_XML_UNEXPECTED_ENDOFFILE;
	return start_inner;
}

LPCSTR XMLParser::_seek_next_attrib(LPCSTR start, rt::String_Ref& attrib_name, rt::String_Ref& value) const
{
	for(;*start && *start != '>' && !(start[0] == '/' && start[1] == '>');start++)
	{
		if(start[-1] <'\x2d' && (start[0] >='\x2d' || start[0]<0))
		{	
			// determine attribute name
			LPCSTR _String = start;
			start = _seek_symbol_end(start);
			UINT _Length = (UINT)(start - _String);

			attrib_name = rt::String_Ref(_String,_Length);

			if(attrib_name.IsEmpty())
			{	rt::_CastToNonconst(this)->SetLastSyntaxError(ERR_ATTRIBUTE_NAME_NOT_FOUND,start);
				break;
			}
			
			// seek '='
			LPCSTR p = strchr(start, '=');
			if(p == nullptr)
			{	rt::_CastToNonconst(this)->SetLastSyntaxError(ERR_ATTRIBUTE_EQUALSIGN_NOT_FOUND,start);
				break;
			}

			start = p+1;
			while(*start && *start != '>')
			{
				if(*start == '"' || *start == '\'')
				{
					LPCSTR _String = start+1;
					p = strchr(start+1, *start);
					if(p == nullptr)
					{	rt::_CastToNonconst(this)->SetLastSyntaxError(ERR_ATTRIBUTE_QUOTATION_NOT_MATCHED,start);
						break;
					}
					UINT _Length = (UINT)(p - start - 1);

					value = rt::String_Ref(_String, _Length);
					return p+1;
				}
				start++;
			}
		}
	}

	attrib_name.Empty();
	value.Empty();
	return nullptr;
}

bool XMLParser::_search_attrib(LPCSTR start, LPCSTR attrib_name,  rt::String_Ref& value) const
{
	rt::String_Ref name;
		
	while((start = _seek_next_attrib(start,name,value)) && !name.IsEmpty())
	{
		if(	name == attrib_name)
			return true;
	}

	value.Empty();
	return false;
}

ULONGLONG XMLParser::GetNodeDocumentOffset() const
{
	return (ULONGLONG)(_CurNode().OuterXML_Start - m_pDocument);
}

bool XMLParser::_EnterNextNode(LPCSTR start, _details::_XML_Tag_Filter* pFilter, bool replace_node)
{
	rt::String_Ref tag;
	LPCSTR ptag, patt, pend;
	int descendant_offset = !replace_node;

	if(start >= _root_node_xml_end)return false;

	while((ptag = _seek_tag_open(start)) && ptag[1]!='/')
	{
		if(ptag >= _root_node_xml_end)return false;

		if(ptag[1] == '!' || ptag[1] == '?')
		{
			ptag = _search_special_node_close(ptag);
			if(ptag == nullptr)return false;
			start = ptag + 1;
			continue;
		}

		LPCSTR _String = ptag+1;
		patt = _seek_symbol_end(ptag+1);
		ASSERT(patt);
		UINT _Length = (UINT)(patt - _String);

		tag = rt::String_Ref(_String,_Length);

		if(tag.IsEmpty())
		{
			SetLastSyntaxError(ERR_TAG_NAME_NOT_FOUND,ptag+1);
			return false;
		}
		
		pend = _seek_tag_close(patt);
		if(pend == nullptr)
		{
			SetLastSyntaxError(ERR_TAG_CLOSURE_NOT_MATCHED,ptag);
			return false;
		}

		if(pFilter && !pFilter->_TagTest(tag,patt,GetDescendantLevel()+descendant_offset))
		{	// skip this node
			if(pend[-1] == '/') // compact node
			{	start = pend + 1; continue;	}
			else
			{	start = _search_node_close(pend + 1, tag);
				if(start){ start++; continue; }
				else
				{	SetLastSyntaxError(ERR_NODE_CLOSURE_NOT_MATCHED,ptag);
					return false;
				}
			}
		}
		else
		{	// enter this node
			_node& node = replace_node?m_NodePath[m_NodePath.GetSize()-1]:m_NodePath.push_back();

			node.InnerXML_Start = pend+1;
			node.OuterXML_Start = ptag;
			node.TagName = tag;
			if(pend[-1] == '/')
			{
				node.Attributes = rt::String_Ref(patt, (UINT)(pend - patt) - 1);
				node.IsCompactNode = true;
			}
			else
			{
				node.Attributes = rt::String_Ref(patt, (UINT)(pend - patt));
				node.IsCompactNode = false;
			}
			node.Attributes = node.Attributes.TrimSpace();
			return true;
		}
	}

	return false;
}


void XMLParser::Clear()
{
	_content_copy.Empty();
	_attribute_cursor = _root_node_xml_start = _root_node_xml_end = nullptr;
	m_XMLParseError = ERR_XML_OK;
	m_XMLParseErrorPosition = 0;
	ClearXPathFilter();
	m_pDocument = nullptr;
	m_NodePath.ShrinkSize(0);
}

ULONGLONG XMLParser::GetNodeFileOffset() const
{
	LPCSTR p = _CurNode().OuterXML_Start;
	return p - m_pDocument;
}

namespace _details
{
	struct _tag
	{	rt::String_Ref		tagname;
		LPSTR				tagclose;
		int					insert_index;
		_tag(){ insert_index = -1; }
	};
	struct _html_insert
	{	int	offset;
		rt::String	content;
		bool	removed;
		_html_insert(){ removed = false; }
	};
};

bool XMLParser::_ForceWellformed(rt::String& xml)
{
	rt::BufferEx<_details::_tag>			tagname_stack;
	rt::BufferEx<_details::_html_insert>	insertions;

	bool ret = true;
	bool FirstOpen = true;

	m_pDocument = xml.Begin();

	LPSTR start = xml.Begin();
	while((start = (LPSTR)_seek_tag_open(start)))
	{
		if(start[1] != '!' && start[1] != '?')
		{
			LPSTR pend = (LPSTR)_seek_tag_close(start+1);
			if(pend && start+1<pend)
			{	if(start[1] == '/')
				{	// close tag
					if(pend[-1] == '/'){ pend[-1] = ' '; ret = false; }
					// check tag match
					LPCSTR p = _details::_skip_whitespace(start+2);
					LPCSTR tagend = _seek_symbol_end(p);
					ASSERT(tagend);
					rt::String_Ref tag(p,tagend);
					int encls = (int)tagname_stack.GetSize();
					while(encls-->0)
					{
						if(tag==tagname_stack[encls].tagname)
						{	int sz = (int)tagname_stack.GetSize();
							// earse all unmatched open tag
							for(int i=encls+1;i<sz;i++)
							{	
								ret = false;
								if(tagname_stack[i].insert_index == -1)
								{
									LPSTR h = tagname_stack[i].tagname.Begin();
									while(*h!='<')h--;
									*h = ' ';
									ASSERT(*tagname_stack[i].tagclose == '>');
									*tagname_stack[i].tagclose = ' ';
									if(tagname_stack[i].tagname.GetLength() <= 2)
										memset(tagname_stack[i].tagname.Begin(),' ',tagname_stack[i].tagname.GetLength());
								}
								else
								{	insertions[tagname_stack[i].insert_index].removed = true;
								}
							}
							tagname_stack.erase(encls,sz);
							goto NEXT_TAG;
						}
					}
					// earse the close tag
					memset(start,' ',tagend - start+1);
				}
				else if(pend[-1] != '/')
				{	// open tag
					LPCSTR p = _details::_skip_whitespace(start+1);

					rt::String_Ref tagname(p,_seek_symbol_end(p));
					if(FirstOpen)
					{	if(tagname != "html")
						{	// insert <html>
							_details::_html_insert& ins = insertions.push_back();
							ins.offset = (int)(start - m_pDocument);
							ins.content = "<html xmlns=\"http://www.w3.org/1999/xhtml\">";
							_details::_tag& t = tagname_stack.push_back();
							t.tagname = "html";
							t.tagclose = start;
							t.insert_index = (int)(insertions.GetSize()-1);
							ret = false;
						}
						FirstOpen = false;
					}

					_details::_tag& t = tagname_stack.push_back();
					t.tagname = tagname;
					t.tagclose = pend;
				}
NEXT_TAG:
				start = pend + 1;
				continue;
			}
		}
		else
		{	// special tag
			LPSTR p = (LPSTR)_search_special_node_close(start);
			if(p)
			{	start = p + 1;
				continue;
			}
		}

		ret = false;
		*start++ = ' ';	// earse the open tag
	}

	if(tagname_stack.GetSize())
	{	// add all unmatched open tags
		ret = false;
		for(int i=((int)tagname_stack.GetSize())-1;i>=0;i--)
		{	xml += '<';
			xml += '/';
			xml += tagname_stack[i].tagname;
			xml += '>';
		}
	}

	if(insertions.GetSize())
	{
		rt::String out;
		int last = 0;
		for(UINT i=0; i<insertions.GetSize(); i++)
		{
			if(!insertions[i].removed)
			{
				out += rt::String_Ref(&xml[last], &xml[insertions[i].offset]);
				out += insertions[i].content;
				last = insertions[i].offset;
			}
		}

		out += rt::String_Ref(&xml[last], xml.End());
		rt::Swap(xml,out);
	}

	return ret;
}


rt::String_Ref XMLParser::GetConvertedXHTML() const
{
	return rt::String_Ref(_content_copy);
}

bool XMLParser::LoadHTML(LPCSTR in, UINT len)
{
	static const rt::String_Ref _szlt("&lt;",4);
	static const rt::String_Ref _szgt("&gt;",4);
	static const rt::String_Ref _szquot("&quot;",6);
	static const rt::String_Ref _szapos("&apos;",6);

	struct _ect 
	{	static bool check(DWORD tagid)
		{	return 	tagid == MAKE_FOURCC('b','a','s','e')
				||	tagid == MAKE_FOURCC('m','e','t','a')
				||	tagid == MAKE_FOURCC('l','i','n','k')
				||	tagid == MAKE_FOURCC('a','r','e','a')
				||	tagid == MAKE_FOURCC('r','e','l',0)
				||	tagid == MAKE_FOURCC('i','m','g',0)
				||	tagid == MAKE_FOURCC('h','r',0,0)
				||	tagid == MAKE_FOURCC('b','r',0,0);
		}	
		static bool becompact(rt::String_Ref& tagname)
		{
			return (tagname.GetLength()<=4 && _ect::check((*((DWORD*)tagname.Begin())) & (0xffffffffU >> (8*(4 - tagname.GetLength()))))) ||
					tagname == rt::String_Ref("input",5) ||
					tagname == rt::String_Ref("button",6);
		}
		static void appendXHTML(rt::String& out, const rt::String_Ref& in)
		{	if(in.IsEmpty())return;
			LPCSTR open = in.Begin();
			LPCSTR p = in.Begin();
			LPCSTR end = in.End();
			rt::String_Ref	sztag;
			for(;;p++)
			{	
				if(p==end){}
				else if(*p == '<'){ sztag = _szlt; }
				else if(*p == '>'){ sztag = _szgt; }
				else if(*p == '"'){ sztag = _szquot; }
				else if(*p == '\''){ sztag = _szapos; }
				else continue;

				if(open!=p)out += rt::String_Ref(open,p);
				if(p==end)break;
				open = p+1;
				out += sztag;
			}
		}
	};

	static const rt::String_Ref _szsslash(" /",2);

	if(len == INFINITE)len = (int)strlen(in);
	rt::String input;
	{	input.SetLength(len);
		LPSTR p = input.Begin();
		LPCSTR inend = in + len;
		for(;in < inend;in++)
		{
			if(*in>=' ' || *in<0 || *in=='\t' || *in=='\r' || *in=='\n')
			{	*p++ = *in;	}
			else
			{
				p++; p--; 
			}
		}
		len = (int)(p - input.Begin());
		input.SetLength(len);
	}
	
	_content_copy.SetLength((UINT)(len*1.2f + 100));
	_content_copy.SetLength(0);

	m_pDocument = input.Begin();
	LPSTR p = input.Begin();
	LPSTR pend = p+len;
	LPSTR open;
	
	while(p<pend && (p = strchr(open = p, '<')))
	{
START_OF_CLOSURE_PARSE:
		ASSERT(*p = '<');

		/////////////////////////////////////////////////////////
		// special nodes
		if(p[1] == '!')
		{
			if(p[2] == '-' && p[3] == '-')	// comment
			{	
				p = strstr(p+3, "-->");
				if(p)
				{	p += 3;
					continue;
				}
				else
				{	SetLastSyntaxError(ERR_HTML_COMMENT_CLOSURE_NOT_MATCHED, nullptr);
					return false;
				}
			}
			else
			{	p = strchr(p+1,'>');
				if(p)
				{	p++;
					_content_copy += rt::String_Ref(open,p);
				}
				else
				{	p = open+1;
				}
				continue;
			}
		}
		// text in between
		if(open != p)
		{	_ect::appendXHTML(_content_copy,rt::String_Ref(open,p));
			open = p;
		}

		int orgoutlen = (int)_content_copy.GetLength();	// restart closure parsing for not well formated tag

		//////////////////////////////////////////////////
		// some tag
		LPSTR tag = _details::_skip_whitespace(p+1);
		bool closingtag;
		if(*tag == '/')
		{	
			closingtag = true;
			tag = _details::_skip_whitespace(tag + 1);
		}
		else
		{	closingtag = false;
		}

		LPSTR tagend = _seek_symbol_end(tag);
		if(!tagend)
		{	SetLastSyntaxError(ERR_HTML_TAG_CLOSURE_NOT_MATCHED, nullptr);
			return false;
		}

		if(tagend == tag)
		{
			if(m_bTrySkipError){ p++; continue; }
			else
			{	SetLastSyntaxError(ERR_HTML_TAG_NAME_NOT_FOUND, nullptr);
				return false;
			}
		}

		rt::String_Ref tagname(tag,tagend);
		tagname.MakeLower();

		bool shouldbecompact;
		if((shouldbecompact = _ect::becompact(tagname)) && closingtag)
		{	// skip this node
			p = strchr(tagend,'>');
			if(*p)p++;
			continue;
		}
		_content_copy += rt::String_Ref("</",1+(int)closingtag);
		_content_copy += tagname;

		if(closingtag)
		{	LPCSTR cl = _details::_skip_whitespace(tagname.End());
			if(*cl == '>'){ cl++; }
			else
			{	if(!m_bTrySkipError)
				{	SetLastSyntaxError(ERR_HTML_TAG_CLOSURE_NOT_MATCHED, nullptr);
					return false;
				}
			}
			p = (LPSTR)cl;
			_content_copy += '>';
			continue;
		}
		else // clear attributes until the closure
		{	
			p = tagend;
			for(;;)
			{	
				p = _details::_skip_attribute_noise(p);
				//p = _details::_skip_whitespace(p);

				if(*p == '<')		// not wellformed
				{
					_content_copy.SetLength(orgoutlen);	// remove content has been processed
					goto START_OF_CLOSURE_PARSE;
				}
				else if(*p == '>')	// reach the end of tag
				{	// enforce compact tag
					if(shouldbecompact)
					{	_content_copy += _szsslash;
					}
					else
					if(tagname == rt::String_Ref("script",6) || tagname == rt::String_Ref("style",5))
					{	//make all content CDATA
						_content_copy += rt::String_Ref("><![CDATA[",10);
						LPCSTR cl = _html_check_node_close(tagname, p+1, false);
						if(cl)
						{	_content_copy += rt::String_Ref(p+1,cl);
							_content_copy += rt::String_Ref("]]></",5);
							_content_copy += tagname;
							p = (LPSTR)_details::_skip_whitespace(cl+1+tagname.GetLength()+1);
							if(*p != '>')
							{	SetLastSyntaxError(ERR_HTML_NODE_CLOSURE_NOT_MATCHED, nullptr);
								return false;
							}
						}
						else
						{	SetLastSyntaxError(ERR_HTML_NODE_CLOSURE_NOT_MATCHED, nullptr);
							return false;
						}
					}
					goto GO_NEXT;
				}
				else if(p[0] == '/' && p[1] == '>')
				{	
					_content_copy += _szsslash;
					p++;
					goto GO_NEXT;
				}
				else if(p[0] == '\0')
				{	
					p--;
					goto GO_NEXT;
				}

				// start of an attribute
				_content_copy += ' ';

				LPSTR patt = p;
				LPSTR pattend = _seek_symbol_end(patt);

				if(patt == pattend)
				{
					if(m_bTrySkipError){ p++; continue; }
					else
					{	SetLastSyntaxError(ERR_HTML_ATTRIBUTE_NAME_NOT_FOUND, nullptr);
						return false;
					}
				}
				
				rt::String_Ref att(patt, pattend);
				att.MakeLower();

				_content_copy += att;

				p = _details::_skip_whitespace(pattend);
				if(*p != '=')
				{	// attribute without value
					_content_copy += rt::String_Ref("=\"\"",3);
					continue;
				}
				_content_copy += '=';

				p = _details::_skip_whitespace(p+1);
				if(*p == '"' || *p == '\'')
				{
					_content_copy += '"';
					LPSTR end = strchr(p+1,*p);
					if(end)
					{	_ect::appendXHTML(_content_copy,rt::String_Ref(p+1,end));
						_content_copy += '"';
						p = end+1;
					}
					else
					{	SetLastSyntaxError(ERR_HTML_QUOTATION_NOT_MATCHED, nullptr);
						return false;
					}
				}
				else
				{	// attribute value may not enclosed by "" even for things like url
					_content_copy += '"';
					tagend = p;
					while((*tagend < 0 || *tagend > ' ') && *tagend != '>' && *tagend != '<' && *tagend != '"' && *tagend != '\'')
						tagend++;
					//tagend = _seek_symbol_end(p);
					_content_copy += rt::String_Ref(p,tagend);
					_content_copy += '"';
					p = tagend;
				}
			}
		}
GO_NEXT:
		_content_copy += '>';
		p++;
	}

    _ForceWellformed(_content_copy);
	return Load(_content_copy,true,_content_copy.GetLength());
}

bool XMLParser::Load(LPCSTR text, bool Keep_referring, SSIZE_T len)
{
	if(Keep_referring)
	{	m_pDocument = text;	}
	else
	{	_content_copy = text;
		m_pDocument = _content_copy;
	}

	if(len < 0)
		_root_node_xml_end = text + strlen(text);
	else
		_root_node_xml_end = m_pDocument + len;

	ASSERT(text);
	_attribute_cursor = nullptr;
	m_pCurTagFilter = nullptr;
	ClearSyntaxError();
	
	m_NodePath.SetSize();

	if(memcmp(m_pDocument,"\xef\xbb\xbf",3) == 0)m_pDocument += 3;

	// parse header
	//LPCSTR pxml, pend, perr;

	//if( (pxml = _details::_seek_tag_open(perr = m_pDocument)) &&
	//	(*((DWORD*)(perr = pxml+1)) == 0x6c6d783f) &&	// ?xml
	//	(pend = _seek_tag_close(perr = pxml+5)) &&
	//	(pend[-1] == '?')
	//)
	{	//LPCSTR start = pend+1;
		LPCSTR start = m_pDocument;
		while((start = _seek_tag_open(start)))
		{
			if(start[1] == '!' || start[1] == '?')
			{
				start = _search_special_node_close(start);
				if(start){ start++; }
				else return false;
			}
			else break;
		}
		
		if(start)
		{
			_root_node_xml_start = start;
			return _EnterNextNode(start);
		}
	}

	//SetLastSyntaxError(ERR_XML_HEADER_BAD_FORMAT,perr);
	return false;
}

void XMLParser::EnterRootNode()
{
	_attribute_cursor = nullptr;
	ASSERT(m_NodePath.GetSize() > 0);
	m_NodePath.ChangeSize(1);
	m_pCurTagFilter = m_pUserTagFilter;
}

bool XMLParser::EnterParentNode()
{
	_attribute_cursor = nullptr;
	if(GetDescendantLevel()>1)
	{
		m_NodePath.ShrinkSize(m_NodePath.GetSize()-1);
		return true;
	}
	else
		return false;
}

bool XMLParser::EnterNextSiblingNode()
{
	if(m_NodePath.GetSize())
	{
		_attribute_cursor = nullptr;
		LPCSTR close = _CurNode().IsCompactNode?(_CurNode().InnerXML_Start-1):
												_search_node_close(_CurNode().InnerXML_Start,_CurNode().TagName);

		return close && _EnterNextNode(close+1, m_pCurTagFilter, true);
	}
	else return false;
}

bool XMLParser::EnterFirstChildNode(LPCSTR tag_name)
{
	if( m_NodePath.GetSize() && _CurNode().IsCompactNode)return false;

	_attribute_cursor = nullptr;
	_details::_XML_Tag_Filter* pfilter;

	struct _name_filter: public _details::_XML_Tag_Filter
	{	
		_XML_Tag_Filter*	CurFilter;
		LPCSTR				Name;

		virtual bool _TagTest(const rt::String_Ref& tag_name, LPCSTR attributes, int DescendantLevel)
		{
			if(tag_name!=Name )return false;
			if(CurFilter)return CurFilter->_TagTest(tag_name, attributes,DescendantLevel);
			return true;
		}
	};

	_name_filter filter;

	if(tag_name)
	{
		filter.Name = tag_name;
		filter.CurFilter = m_pCurTagFilter;
		
		pfilter = &filter;
	}
	else
		pfilter = m_pCurTagFilter;

	if(m_NodePath.GetSize())
		return _EnterNextNode(_CurNode().InnerXML_Start,pfilter);
	else
		return _EnterNextNode(_root_node_xml_start,pfilter);
}

bool XMLParser::EnterChildNode(UINT n_th, LPCSTR tag_name)
{
	ASSERT(n_th);
	if(EnterFirstChildNode(tag_name))
		return EnterNextSiblingNode(n_th-1);		
	
	return false;
}

bool XMLParser::EnterNextSiblingNode(UINT n_th)
{
	while(n_th)
	{
		if(!EnterNextSiblingNode())return false;
		n_th--;
	}
	return true;
}

void XMLParser::ClearXPathFilter()
{
	m_pCurTagFilter = m_pUserTagFilter;
	m_XPathParser.Clear();
}

void XMLParser::EscapeXPath()				// call this to remove XPath filtering
{
	ClearXPathFilter();
}


void XMLParser::ConvertExtendedCSSPathToXPath(LPCSTR extended_css_path, rt::String& xpath)
{
	xpath.SetLength(0);
	LPCSTR tagbegin = _details::_skip_whitespace(extended_css_path);

	while(*tagbegin)
	{
		int z = (int)rt::String_Ref(tagbegin).FindCharacter(' ');
		LPCSTR tagend = tagbegin + (z < 0 ? rt::String_Ref(tagbegin).GetLength() : z);

		rt::String_Ref tag(tagbegin, tagend);
		char* currentStopChar = nullptr;
		int lastFilterPos = -1;
		int lastFilterPriority = -1;

		do
		{
			int p = (int)tag.FindCharacter('#'); if (p == -1) p = (int)tag.GetLength();
			int q = (int)tag.FindCharacter('.'); if (q == -1) q = (int)tag.GetLength();
			int r = (int)tag.FindCharacter('~'); if (r == -1) r = (int)tag.GetLength();
			int s = (int)tag.FindCharacter('['); if (s == -1) s = (int)tag.GetLength();

			int nextStop = min(min(min(p, q), r), s);
			char* nextStopChar = &tag[nextStop];
			rt::String_Ref seg = tag.SubStrHead(nextStop);

			if (!currentStopChar)
			{	xpath += '/' + seg;
				xpath.SubStrTail(nextStop).MakeLower();
			}
			else
			{
				rt::String filter;
				int filterPriority;
				if (*currentStopChar == '.')
				{	filter = rt::String_Ref("@class:='") + seg + '\'';
					filterPriority = 1;
				}
				else if (*currentStopChar == '~')
				{	filter = rt::String_Ref("@class~='") + seg + '\'';
					filterPriority = 1;
				}
				else if (*currentStopChar == '#')
				{	filter = rt::String_Ref("@id='") + seg + '\'';
					filterPriority = 0;
				}
				else if (*currentStopChar == '[')
				{	//ASSERT(tag[nextStop - 1] == ']');
                    nextStop = (int)tag.FindCharacter(']') + 1;
                    ASSERT(nextStop); // ']' must be found in tag
                    nextStopChar = &tag[nextStop];
                    seg = tag.SubStrHead(nextStop);

					filter = seg.TrimRight(1);
					filterPriority = 2;
				}
				else ASSERT(false);

				if (lastFilterPos == -1)
				{	lastFilterPos = (int)xpath.GetLength();
					lastFilterPriority = filterPriority;
					xpath += '[' + filter + ']';
				}
				else if (filterPriority < lastFilterPriority)
				{	lastFilterPriority = filterPriority;
					xpath = xpath.SubStrHead(lastFilterPos) + '[' + filter + ']';
				}
			}
			
			tag = tag.SubStr(nextStop + 1); // SubStr() is okay when pos > GetLength()
			currentStopChar = nextStopChar;
		}
		while (currentStopChar != tagend);

		tagbegin = _details::_skip_whitespace(tagend);
	}
}

bool XMLParser::EnterXPathByCSSPath(const rt::String_Ref& css_path)
{
	if(css_path.IsEmpty())return true;

	static const rt::CharacterSet_ControlChar sep(" >/\\");

	rt::String_Ref seg[256];
	UINT co = css_path.Split<true>(seg, 256, sep);
	
	rt::String xpath;
	for(UINT i=0;i<co;i++)
	{
		if(seg[i].IsEmpty())continue;
		rt::String_Ref f[2];
		UINT s = seg[i].Split(f,2,"#.");
		xpath += '/';
		xpath += f[0];
		if(s==2)
		{
			switch(f[1].Begin()[-1])
			{
			case '#':
				xpath += rt::SS("[@id='") + f[1] + rt::SS("']");
				break;
			case '.':
				xpath += rt::SS("[@class^='") + f[1] + rt::SS("']");
				break;
			default:
				SetLastSyntaxError(ERR_HTML_CSS_OPERATOR_NOT_SUPPORTED, f[1].Begin()-1);
				return false;
			}
		}
	}

	return EnterXPath(xpath);
}

bool XMLParser::EnterXPath(LPCSTR xpath)
{
	if(xpath==nullptr)return false;

	_attribute_cursor = nullptr;
	ClearXPathFilter();
	ClearSyntaxError();

	if(m_XPathParser.Load(xpath))
	{
		if(m_XPathParser.m_bRelativePath)
		{	// translate to fixed path
			if(m_XPathParser.m_UpLevelCount == GetDescendantLevel())
			{	m_XPathParser.m_UpLevelCount = 0;
				m_XPathParser.m_QualifierShifts = 0;
				m_XPathParser.m_bRelativePath = false;
				m_NodePath.SetSize();
			}
			else
			{
				while(m_XPathParser.m_UpLevelCount > 0)
				{	m_XPathParser.m_UpLevelCount--;
					if(!EnterParentNode())goto XPATH_ERROR;
				}
				m_XPathParser.m_QualifierShifts = GetDescendantLevel();
				_root_node_xml_start = _CurNode().OuterXML_Start;
			}
		}
		else
		{	m_XPathParser.m_QualifierShifts = 0;
			m_NodePath.SetSize();
		}

		m_pCurTagFilter = &m_XPathParser;
		return EnterSucceedNode();
	}
	
	return false;
XPATH_ERROR:
	EnterRootNode();

	return false;
}

void XPathParser::_ClearLastOrdinalTestPast(int level)
{
	if(level < (int)m_Qualifiers.GetSize())
		m_Qualifiers[level]._LastOrdinalTestPast = 0;
	else
		m_FinalQualifier._LastOrdinalTestPast = 0;
}

bool XPathParser::_TagTest(const rt::String_Ref& tag_name, LPCSTR attributes, int DescendantLevel)
{
	bool ret = false;
	ASSERT(DescendantLevel > m_QualifierShifts);
	_Qualifier* pQ = nullptr;
	if(DescendantLevel <= (int)(m_QualifierShifts + m_Qualifiers.GetSize()))
	{	
		_ClearLastOrdinalTestPast(DescendantLevel - m_QualifierShifts); // clear _LastOrdinalTestPast
		// stepping into path halfway
		_Qualifier& qu = m_Qualifiers[DescendantLevel - m_QualifierShifts - 1];

		if(qu.qt_Ordinal>=0 && qu.qt_Ordinal<qu._LastOrdinalTestPast)
			return false;

		pQ = &qu;
		_LastNodeSatificated = false;	// desired level yet reached
		ret =	QualifierTest(qu, tag_name, attributes) &&
				( !m_pUserTagFilter || m_pUserTagFilter->_TagTest(tag_name, attributes, DescendantLevel));
	}
	else
	{	// checking last node
		if(!m_bIncludeDescendants && DescendantLevel > (int)(m_QualifierShifts + m_Qualifiers.GetSize() + 1))
		{
			ret = _LastNodeSatificated = false;
		}
		else
		{	pQ = &m_FinalQualifier;
			if(m_FinalQualifier.qt_Ordinal>=0 && m_FinalQualifier.qt_Ordinal<m_FinalQualifier._LastOrdinalTestPast)
				return false;

			_LastNodeSatificated =	QualifierTest(m_FinalQualifier, tag_name, attributes) &&
									( !m_pUserTagFilter || m_pUserTagFilter->_TagTest(tag_name, attributes, DescendantLevel));

			ret = m_bIncludeDescendants || _LastNodeSatificated;
		}
	}

	if(ret && pQ->qt_Ordinal>=0)
	{	ret = (pQ->_LastOrdinalTestPast == pQ->qt_Ordinal);
		pQ->_LastOrdinalTestPast++;
	}

	return ret;
}



XPathParser::XPathParser(XMLParser& xml_parser)
 :_XmlParser(xml_parser)
{
	_pXPath = nullptr;
	m_XPathParseError = ERR_XPATH_OK;
	m_XPathParseErrorPosition = 0;
	m_UpLevelCount = m_QualifierShifts = 0;
	m_pUserTagFilter = nullptr;
	_LastNodeSatificated = true;
}

bool XPathParser::QualifierTest(_Qualifier& q, const rt::String_Ref& tag_name, LPCSTR attributes)
{
	ASSERT(QT_NAME_NOT_PARSED != q.qt_TagName);

	if((q.qt_TagName == QT_NAME_EXACT) && tag_name!=q.TagName)return false;

	rt::String_Ref name, value;
	switch(q.qt_Attribute)
	{
	case QT_ATTRIBUTE_ANY: return true;
	case QT_ATTRIBUTE_HAVE:
		while(attributes)
		{
			attributes = _XmlParser._seek_next_attrib(attributes, name, value);
			if(attributes)
			{	if(name == q.Attribute)return true;
			}
			else
			{	if(GetLastSyntaxError()!=ERR_XPATH_OK)return false;
			}
		}
		return false;
	case QT_ATTRIBUTE_EQUAL:
		while(attributes)
		{
			attributes = _XmlParser._seek_next_attrib(attributes, name, value);
			if(attributes)
			{	if(name == q.Attribute && q.Value.GetLength()<=value.GetLength())
				{	
					XMLParser::_convert_xml_to_text(value,_conv_value_temp);
					return _conv_value_temp == q.Value;
				}
			}
			else
			{	if(GetLastSyntaxError()!=ERR_XPATH_OK)return false;
			}
		}
		return false;
	case QT_ATTRIBUTE_NOTEQUAL:
		while(attributes)
		{
			attributes = _XmlParser._seek_next_attrib(attributes, name, value);
			if(attributes)
			{	if(name == q.Attribute)
				{	
					XMLParser::_convert_xml_to_text(value,_conv_value_temp);
					return !(_conv_value_temp == q.Value);
				}
			}
			else
			{	if(GetLastSyntaxError()!=ERR_XPATH_OK)return false;
			}
		}
		return true;
	case QT_ATTRIBUTE_STARTWITH:
		while(attributes)
		{
			attributes = _XmlParser._seek_next_attrib(attributes, name, value);
			if(attributes)
			{	if(name == q.Attribute && q.Value.GetLength()<=value.GetLength())
				{	
					XMLParser::_convert_xml_to_text(value,_conv_value_temp);
					return	_conv_value_temp.StartsWith(q.Value);
				}
			}
			else
			{	if(GetLastSyntaxError()!=ERR_XPATH_OK)return false;
			}
		}
		return false;
	case QT_ATTRIBUTE_ENDWITH:
		while(attributes)
		{
			attributes = _XmlParser._seek_next_attrib(attributes, name, value);
			if(attributes)
			{	if(name == q.Attribute && q.Value.GetLength()<=value.GetLength())
				{	
					XMLParser::_convert_xml_to_text(value,_conv_value_temp);
					return	_conv_value_temp.EndsWith(q.Value);
				}
			}
			else
			{	if(GetLastSyntaxError()!=ERR_XPATH_OK)return false;
			}
		}
		return false;
	case QT_ATTRIBUTE_CONTAIN:
		while(attributes)
		{
			attributes = _XmlParser._seek_next_attrib(attributes, name, value);
			if(attributes)
			{	if(name == q.Attribute && q.Value.GetLength()<=value.GetLength())
				{	
					XMLParser::_convert_xml_to_text(value,_conv_value_temp);
					return	q.Value.GetLength() <= _conv_value_temp.GetLength() &&
							strstr(_conv_value_temp, q.Value);
				}
			}
			else
			{	if(GetLastSyntaxError()!=ERR_XPATH_OK)return false;
			}
		}
		return false;
	case QT_ATTRIBUTE_HAVE_WORD:
		while(attributes)
		{
			attributes = _XmlParser._seek_next_attrib(attributes, name, value);
			if(attributes)
			{	if(name == q.Attribute && q.Value.GetLength()<=value.GetLength())
				{	
					static const rt::CharacterSet_ControlChar sep(" ");
					XMLParser::_convert_xml_to_text(value,_conv_value_temp);
					rt::String_Ref f[256];
					UINT co = _conv_value_temp.Split(f, 256, sep);
					for(UINT i=0;i<co;i++)
						if(f[i] == q.Value)return true;
					return false;
				}
			}
			else
			{	if(GetLastSyntaxError()!=ERR_XPATH_OK)return false;
			}
		}
		return false;
	default:	ASSERT(0);
	}

	return false;
}

bool XMLParser::EnterSucceedNode()
{
	_attribute_cursor = nullptr;

	if(m_pCurTagFilter!= &m_XPathParser)
	{
		if(EnterFirstChildNode())return true;
		if(GetLastSyntaxError() != ERR_XML_OK)return false;

		if(EnterNextSiblingNode()){ return true; }
		else
		{	do
			{	if(GetLastSyntaxError() != ERR_XML_OK)return false;
				if(!EnterParentNode())return false;
			}while(!EnterNextSiblingNode());
			return true;
		}
	}
	else
	{
		for(;;)
		{
			if(	m_XPathParser.m_bIncludeDescendants || 
				m_NodePath.GetSize() < (int)(m_XPathParser.m_QualifierShifts + m_XPathParser.m_Qualifiers.GetSize() + 1)
			)
			{	if(EnterFirstChildNode())goto CHECK_THIS_NODE;
				if(GetLastSyntaxError() != ERR_XML_OK)return false;
			}

			if(EnterNextSiblingNode()){ goto CHECK_THIS_NODE; }
			else
			{	
				while( ((int)GetDescendantLevel()) > m_XPathParser.m_QualifierShifts+1 )
				{
					if(GetLastSyntaxError() != ERR_XML_OK)return false;
					if(!EnterParentNode())return false;
					if(EnterNextSiblingNode())goto CHECK_THIS_NODE;
				}

				return false;
			}

CHECK_THIS_NODE:
			if(m_XPathParser._LastNodeSatificated)break;
		}

		return true;
	}

	ASSERT(0);
}

void XMLParser::TextDump()
{
	if(GetDescendantLevel() == 0)return;

	char spaces[256];
	memset(spaces, ' ', sizeof(spaces));

	rt::String att;
	do
	{	if(GetLastSyntaxError() != ERR_XML_OK)
		{	_LOG(rt::String_Ref("XML Parse Error = ")<<GetLastSyntaxError());
			break;
		}

		if(GetAttributesXML(att))
		{
			_LOG(	rt::String_Ref(spaces, (GetDescendantLevel()-1)*2)<<'<'<<GetNodeName()<<'>'<<
					rt::SS(" [ ")<<att<<rt::SS(" ]")
			);
		}
		else
		{	_LOG(rt::String_Ref(spaces, GetDescendantLevel()*2)<<'<'<<GetNodeName()<<'>');
		}
	}while(EnterSucceedNode());
}

bool XMLParser::GetInnerXML(rt::String& text) const
{
	if(_CurNode().IsCompactNode)
		text.Empty();
	else
	{   LPCSTR inner_start = _CurNode().InnerXML_Start;
        LPCSTR inner_end = nullptr;
		if(!_search_node_close(inner_start, _CurNode().TagName, &inner_end))
		{	text.Empty();
			return false;
		}
		rt::String_Ref out(inner_start, inner_end);
		text = out.TrimSpace();
	}

	return true;
}

bool XMLParser::GetInnerRtfHtml(rt::String& out) const
{
	if(_CurNode().IsCompactNode)
	{	out.Empty();
		return false;
	}

	rt::String_Ref xml = GetInnerXML();
	if(!out.SetLength(xml.GetLength()))
		return false;
	
	LPSTR p = out.Begin();
	LPCSTR s = xml.Begin();
	LPCSTR end = xml.End();

	while(s<end)
	{
		LPCSTR close;
		if(*s == '<' && (close = rt::XMLParser::_seek_tag_close(s+1)))
		{	
			LPCSTR tag_start = s+1;
			if(*tag_start == '/')tag_start++;
			LPCSTR tag_end = rt::XMLParser::_seek_symbol_end(tag_start);
			
			rt::String_Ref tag(tag_start, tag_end);
			LPCSTR js_close;
			rt::XMLParseError err;

			if(tag.GetLength() == 2 && ((*(WORD*)&tag[0] == *(WORD*)"br") || (*(WORD*)&tag[0] == *(WORD*)"hr")))
			{
				*p++ = '<';
				*p++ = tag[0];		*p++ = 'r';
				*p++ = ' ';			*p++ = '/';		*p++ = '>';
			}
			else
			if(	tag[0] == 'a' && tag.GetLength() == 1)
			{	
				if(tag[-1] == '/'){ memcpy(p, "</a>", 4); p+=4; }
				else
				{	rt::String_Ref url;
					_search_attrib(&tag[1], "href", url);
					memcpy(p, "<a href=\"", 9);					p+=9;
					memcpy(p, url.Begin(), url.GetLength());	p+=url.GetLength();
					memcpy(p, "\">", 2);						p+=2;
				}
			}
			else 
			if(	(tag.GetLength() == 1 && ((tag[0] == 'p') ||(tag[0] == 'b') ||(tag[0] == 'i') ||(tag[0] == 'q') ||(tag[0] == 's') ||(tag[0] == 'u'))) || 
				(tag.GetLength() == 2 && (	(*(WORD*)&tag[0] == *(WORD*)"em") || 
											
											(*(WORD*)&tag[0] == *(WORD*)"ol") ||
											(*(WORD*)&tag[0] == *(WORD*)"ul") ||
											(*(WORD*)&tag[0] == *(WORD*)"li") ||
											(*(WORD*)&tag[0] == *(WORD*)"hr") ||
											(tag[0] == 'h' && tag[1] >= '1' && tag[1] <= '6')
										 )
				) ||
				tag == rt::SS("strong") ||
				tag == rt::SS("code") ||
				tag == rt::SS("big") ||
				tag == rt::SS("pre") ||
				tag == rt::SS("sub") ||
				tag == rt::SS("sup") ||
				tag == rt::SS("small")
			)
			{	*p++ = '<';
				if(tag_start[-1] == '/')*p++ = '/';
				memcpy(p, tag_start, tag.GetLength());	p+=tag.GetLength();
				*p++ = '>';
			}
			else
			if(	tag == rt::SS("script") && tag_start[-1] == '<' && 
				(js_close = rt::XMLParser::_search_node_close(&err,close+1,tag)) &&
				err == rt::ERR_XML_OK
			)
			{	close = js_close;
			}
			else
			{	if(p == out.Begin() || p[-1] < 0 || p[-1] > ' ' || p[-1] != '>')
					*p++ = ' ';
			}
			s = close + 1;
		}
		else
		{
			if(*s > 0 && *s <= ' ' && p > out.Begin() && ((p[-1] >0 && p[-1] <=' ') || p[-1] == '>')){ s++; }
			else
			{	*p++ = *s++;
			}
		}
	}

	out.SetLength((UINT)(p-out.Begin()));
	return true;
}


rt::String_Ref XMLParser::GetInnerXML() const
{
	if(_CurNode().IsCompactNode)
		return nullptr;
	else
	{   LPCSTR inner_start = _CurNode().InnerXML_Start;
        LPCSTR inner_end = nullptr;
		if(!_search_node_close(inner_start, _CurNode().TagName, &inner_end))
		{	return nullptr;
		}
		return rt::String_Ref(inner_start, inner_end).TrimSpace();
	}
}

bool XMLParser::GetInnerText(rt::String& text, bool no_text_from_subnodes) const
{
	if(_CurNode().IsCompactNode)
		text.Empty();
	else
	{   LPCSTR inner_start = _CurNode().InnerXML_Start;
        LPCSTR inner_end = nullptr;
		if(!_search_node_close(inner_start,_CurNode().TagName,&inner_end))
		{
			text.Empty();
			return false;
		}
		rt::String_Ref out(inner_start, inner_end);
		out = out.TrimSpace();
		if(!out.IsEmpty())
		{	text.SetLength(out.GetLength());
			_convert_xml_to_text(rt::String_Ref(inner_start,inner_end),text,true,no_text_from_subnodes);
		}
		else 
		{
			text.Empty();
		}
		return true;
	}

	return true;
}

void XMLParser::ExtractInnerText(const rt::String_Ref& doc, rt::String& out, char sep)
{
	_convert_xml_to_text(doc,out,true,false,sep);
}


rt::String_Ref XMLParser::GetInnerCDATA() const
{
	if(_CurNode().IsCompactNode)
		return nullptr;
	else
	{   LPCSTR inner_start = _CurNode().InnerXML_Start;
        LPCSTR inner_end = nullptr;
		if(!_search_node_close(inner_start,_CurNode().TagName,&inner_end))
		{
			return nullptr;
		}

		rt::String_Ref a = rt::String_Ref(inner_start, inner_end).TrimSpace();
		if (a.StartsWith(rt::SS("<![CDATA[")) && a.EndsWith(rt::SS("]]>")))
		{
			return a.TrimLeft(9).TrimRight(3);
		}
		else
		{   return nullptr;
		}
	}
}


bool XMLParser::GetAttributesXML(rt::String& text) const
{
	if(!_CurNode().Attributes.IsEmpty())
	{	
		text.SetLength(_CurNode().Attributes.GetLength());
		memcpy(text.Begin(),_CurNode().Attributes.Begin(),_CurNode().Attributes.GetLength());
		return true;
	}
	else
	{	text.Empty();
		return false;
	}
}


bool XMLParser::GetOuterXML(rt::String& text) const
{
	LPCSTR end;
	if(_CurNode().IsCompactNode)
	{	end = _CurNode().InnerXML_Start - 1;	}
	else
	{	end = _search_node_close(_CurNode().InnerXML_Start,_CurNode().TagName);
		if(end == nullptr)
		{	text.Empty();
			return false;
		}
	}

	end++;
	if(!text.SetLength((int)(end - _CurNode().OuterXML_Start)))
	{
		text.Empty();
		rt::_CastToNonconst(this)->SetLastSyntaxError(ERR_XML_OUT_OF_MEMORY, _CurNode().OuterXML_Start);
		return false;
	}
			
	memcpy(text.Begin(),_CurNode().OuterXML_Start,text.GetLength());
	return true;
}

rt::String_Ref XMLParser::GetOuterXML() const
{
	LPCSTR end;
	if(_CurNode().IsCompactNode)
	{	end = _CurNode().InnerXML_Start - 1;	}
	else
	{	end = _search_node_close(_CurNode().InnerXML_Start,_CurNode().TagName);
		if(end == nullptr)
		{	return rt::String_Ref();
		}
	}

	end++;
	return rt::String_Ref(_CurNode().OuterXML_Start, (int)(end - _CurNode().OuterXML_Start));
}

bool XMLParser::HasAttribute(LPCSTR name) const
{
	rt::String_Ref val;
	if(!_CurNode().Attributes.IsEmpty() && _search_attrib(_CurNode().Attributes.Begin(),name,val))
	{	
		return true;
	}

	return false;
}

bool XMLParser::GetAttribute(LPCSTR name, rt::String& value) const
{
	rt::String_Ref val;
	if(!_CurNode().Attributes.IsEmpty() && _search_attrib(_CurNode().Attributes.Begin(),name,val))
	{	
		_convert_xml_to_text(val,value);
		return true;
	}

	value.Empty();
	return false;
}

bool XMLParser::GetAttribute(LPCSTR name, rt::String& value, LPCSTR default_value) const
{
	rt::String_Ref val;
	if(!_CurNode().Attributes.IsEmpty() && _search_attrib(_CurNode().Attributes.Begin(),name,val))
	{	
		_convert_xml_to_text(val,value);
		return true;
	}

	value = default_value;
	return false;
}


bool XMLParser::GetAttribute_Path(LPCSTR name, rt::String& value) const
{
	if(GetAttribute(name,value))
	{
		if(value.End()[-1] == '\\' || value.End()[-1] == '/')
			value.SetLength(value.GetLength()-1);
		return true;
	}
	return false;
}


INT XMLParser::GetAttribute_Int(LPCSTR name, int dd) const
{
	rt::String value;
	if(GetAttribute(name,value))
	{
		return atoi(value);
	}
	else return dd;
}

bool XMLParser::GetAttribute_Bool(LPCSTR name, bool default_value) const
{
	rt::String value;
	if(GetAttribute(name,value))
	{
		return	atoi(value) != 0 ||
				value[0] == 't' || value[0] == 'T' || value[0] == 'y' || value[0] == 'Y';
	}
	else return default_value;
}

LONGLONG XMLParser::GetAttribute_Int64(LPCSTR name, LONGLONG dd) const
{
	rt::String value;
	if(GetAttribute(name,value))
	{
		LONGLONG x;
		value.ToNumber(x);
		return x;
	}
	else return dd;
}

LPCSTR XMLParser::_search_node_close(LPCSTR start_inner, const rt::String_Ref& tag_name, LPCSTR* pInnerXML_End) const
{
	XMLParseError err;
	LPCSTR ret = _search_node_close(&err, start_inner, tag_name, pInnerXML_End);
	if(err!=ERR_XML_OK)
	{	rt::_CastToNonconst(this)->SetLastSyntaxError(err, ret);
		return nullptr;
	}
	return ret;
}

LPCSTR XMLParser::_search_special_node_close(LPCSTR start_outer) const
{
	XMLParseError err;
	LPCSTR ret = _search_special_node_close(&err, start_outer);
	if(err!=ERR_XML_OK)
	{	rt::_CastToNonconst(this)->SetLastSyntaxError(err, ret);
		return nullptr;
	}
	return ret;
}


int	XMLParser::GetAttribute_Timespan(LPCSTR name, int default_value_msec = 0.0) const
{
	rt::String value;
	if(GetAttribute(name,value))
	{
		double v;
		UINT i = value.ToNumber(v);

		for(;i<value.GetLength();i++)	// skip whitespace
			if(value[i] > ' ')break;

		if(value[i] == 's' || value[i] == 'S')
			return (int)(1000*v + 0.5);
		else if(value[i] == 'm' || value[i] == 'M')
			return (int)(1000*v*60 + 0.5);
		else if(value[i] == 'h' || value[i] == 'H')
			return (int)(1000*v*3600 + 0.5);
		else if(value[i] == 'd' || value[i] == 'D')
			return (int)(1000*v*3600*24 + 0.5);
		else if(value[i] == 'w' || value[i] == 'W')
			return (int)(1000*v*3600*24*7 + 0.5);
		else
			return (int)(v + 0.5);	// default msec
	}
	
	return default_value_msec;
}


ULONGLONG XMLParser::GetAttribute_FileSize(LPCSTR name, ULONGLONG dd) const
{
	rt::String value;
	if(GetAttribute(name,value))
	{
		ULONGLONG sz = 0;
		UINT i = value.ToNumber(sz);

		for(;i<value.GetLength();i++)	// skip whitespace
			if(value[i] > ' ')break;

		if(value[i] == 'k' || value[i] == 'K')
			sz *= 1024;
		else if(value[i] == 'm' || value[i] == 'M')
			sz *= 1024*1024;
		else if(value[i] == 'g' || value[i] == 'G')
			sz *= 1024LL*1024*1024;
		else if(value[i] == 't' || value[i] == 'T')
			sz *= 1024LL*1024*1024*1024;

		return sz;
	}
	else return dd;
}

double XMLParser::GetAttribute_Float(LPCSTR name, double dd) const
{
	rt::String value;
	if(GetAttribute(name,value))
	{
		double ret;
		value.ToNumber(ret);
		return ret;
	}
	else return dd;
}


bool XMLParser::GetAttribute_BoolRef(LPCSTR name, bool& value) const
{
	rt::String v;
	if(GetAttribute(name,v))
	{   value = (atoi(v) != 0);
        return true;
	}
	else
    {   return false;
    }
}

bool XMLParser::GetAttribute_BoolRef(LPCSTR name, bool& value, bool default_value) const
{
	rt::String v;
	if(GetAttribute(name,v))
	{   value = (atoi(v) != 0);
        return true;
	}
	else
    {   value = default_value;
        return false;
    }
}

bool XMLParser::GetAttribute_IntRef(LPCSTR name, INT& value) const
{
	rt::String v;
	if(GetAttribute(name,v))
	{   value = atoi(v);
        return true;
	}
	else
    {   return false;
    }
}

bool XMLParser::GetAttribute_IntRef(LPCSTR name, INT& value, INT default_value) const
{
	rt::String v;
	if(GetAttribute(name,v))
	{   value = atoi(v);
        return true;
	}
	else
    {   value = default_value;
        return false;
    }
}

bool XMLParser::GetAttribute_FloatRef(LPCSTR name, double& value) const
{
	rt::String v;
	if(GetAttribute(name,v))
	{   value = atof(v);
        return true;
	}
	else
    {   return false;
    }
}

bool XMLParser::GetAttribute_FloatRef(LPCSTR name, double& value, double default_value) const
{
	rt::String v;
	if(GetAttribute(name,v))
	{   value = atof(v);
        return true;
	}
	else
    {   value = default_value;
        return false;
    }
}


bool XMLParser::GetFirstAttribute(rt::String& name, rt::String& value) const
{
	if(_CurNode().Attributes.IsEmpty())return false;
	rt::_CastToNonconst(this)->_attribute_cursor = _CurNode().Attributes.Begin();
	return GetNextAttribute(name, value);
}

bool XMLParser::GetNextAttribute(rt::String& name, rt::String& value) const
{
	ASSERT(_attribute_cursor >= _CurNode().Attributes.Begin());
	rt::String_Ref n, v;
	LPCSTR p = _seek_next_attrib(_attribute_cursor, n, v);
	if(p && !n.IsEmpty())
	{	rt::_CastToNonconst(this)->_attribute_cursor = p;
		name = n;	value = v;
		return true;
	}
	else return false;
}


LPCSTR XPathParser::SetLastSyntaxError(XPathParseError errnum, LPCSTR pos)
{
	ASSERT(pos >= _pXPath);
	m_XPathParseError = errnum;
	m_XPathParseErrorPosition = (UINT)(pos - _pXPath);

	ASSERT(0);
	return pos;
}


void XPathParser::_Qualifier::Load(LPCSTR qualify, UINT length)
{
	qt_TagName = QT_NAME_NOT_PARSED;
	TagName = rt::String_Ref(qualify, length);
}

bool XPathParser::ParseQualifier(_Qualifier& q)
{
	q._LastOrdinalTestPast = 0;
	ASSERT(q.qt_TagName == QT_NAME_NOT_PARSED);
	rt::String qt = q.TagName;
	//rt::Swap(qt,q.TagName);

	LPSTR qualify = qt.Begin();
	if(qt.IsEmpty())
	{
		SetLastSyntaxError(ERR_SELECTOR_NODENAME_NOT_FOUND,qualify);
		return false;
	}

	qualify = _details::_skip_whitespace(qualify);
	if(*qualify == '@')
	{
		SetLastSyntaxError(ERR_SELECTOR_ATTRIBUTE_SET_NOT_SUPPORTED,qualify);
		return false;
	}
	else
	{	
		LPSTR patt = nullptr;

		if(*qualify == '*')
		{	
			patt = qualify + 1;
			q.qt_TagName = QT_NAME_ANY;
		}
		else
		{	patt = (LPSTR)XMLParser::_seek_symbol_end(qualify);
			if(patt == qualify)
			{
				SetLastSyntaxError(ERR_SELECTOR_NODENAME_NOT_FOUND,qualify);
				return false;
			}
			q.qt_TagName = QT_NAME_EXACT;
			q.TagName = rt::String_Ref(qualify, patt);
			//q.TagName.SetLength((UINT)(patt - qualify));
			//memcpy(q.TagName.GetBuffer(),qualify,(UINT)(patt - qualify));
		}

		patt = _details::_skip_whitespace(patt);

		if(*patt == '[')
		{	
			patt = _details::_skip_whitespace(patt+1);

			if(*patt >= '0' && *patt <= '9')
			{
				q.qt_Attribute = QT_ATTRIBUTE_ANY;
				LPSTR tail = XMLParser::_seek_symbol_end(patt);
				if(*tail == ']')
				{
					rt::String_Ref(patt,tail).ToNumber(q.qt_Ordinal);
					return true;
				}
				else
				{	SetLastSyntaxError(ERR_QUALIFIER_BAD_ORDINAL,tail);
					return false;
				}
			}
			else if(*patt == '@')
			{
				q.qt_Ordinal = -1;
				q.qt_Attribute = QT_ATTRIBUTE_HAVE;

				patt = _details::_skip_whitespace(patt+1);
				LPCSTR pend = XMLParser::_seek_symbol_end(patt);
				q.Attribute.SetLength((UINT)(pend - patt));
				memcpy(q.Attribute.Begin(),patt,(UINT)(pend - patt));

				patt = (LPSTR)_details::_skip_whitespace(pend);

				if(*patt == '=')
				{	if(patt[-1] == ':')
					{	q.qt_Attribute = QT_ATTRIBUTE_STARTWITH;
						q.Attribute.SetLength(q.Attribute.GetLength()-1);
					}
					else if(patt[-1] == '?')
					{	q.qt_Attribute = QT_ATTRIBUTE_ENDWITH;
						q.Attribute.SetLength(q.Attribute.GetLength()-1);
					}
					else if(patt[-1] == '~')
					{	q.qt_Attribute = QT_ATTRIBUTE_CONTAIN;
						q.Attribute.SetLength(q.Attribute.GetLength()-1);
					}
					else if(patt[-1] == '^')
					{	q.qt_Attribute = QT_ATTRIBUTE_HAVE_WORD;
						q.Attribute.SetLength(q.Attribute.GetLength()-1);
					}
					else
					{
						q.qt_Attribute = QT_ATTRIBUTE_EQUAL;
					}
					patt++;
				}
				else if(patt[0] == '!' && patt[1] == '=')
				{	q.qt_Attribute = QT_ATTRIBUTE_NOTEQUAL;
					patt+=2;
				}
				else if(patt[0] == ']')
				{	return true;
				}
				else
				{
					SetLastSyntaxError(ERR_QUALIFIER_OPERATOR_NOT_SUPPORTED,patt);
					return false;
				}

				patt = (LPSTR)_details::_skip_whitespace(patt);
				if(*patt == '\'' || *patt == '\"')
				{
					pend = strchr(patt+1,*patt);
					if(pend)
					{
						XMLParser::_convert_xml_to_text(rt::String_Ref(patt+1,(UINT)(pend - patt) -1),q.Value);

						patt = (LPSTR)_details::_skip_whitespace(pend+1);
						if(*patt == ']')return true;
						else
						{
							SetLastSyntaxError(ERR_QUALIFIER_CONDITION_NOT_SUPPORTED,patt);
							return false;
						}
					}
					else
					{
						SetLastSyntaxError(ERR_QUALIFIER_QUOTATION_NOT_MATCHED,patt);
						return false;
					}
				}
				else
				{
					SetLastSyntaxError(ERR_QUALIFIER_VALUE_NOT_SUPPORTED,patt);
					return false;
				}
			}
			else
			{
				SetLastSyntaxError(ERR_QUALIFIER_CONDITION_NOT_SUPPORTED,patt);
				return false;
			}
		}
		else if(*patt == '\0')
		{
			q.qt_Attribute = QT_ATTRIBUTE_ANY;
			q.qt_Ordinal = -1;
			return true;
		}
		else
		{	
			SetLastSyntaxError(ERR_SELECTOR_UNEXPECTED_SYMBOL,patt);
			return false;
		}
	}
	
	ASSERT(0);
}

void XPathParser::Clear()
{
	m_bRelativePath = false;
	m_bIncludeDescendants = false;
	m_FinalQualifier.qt_TagName = QT_NAME_NOT_PARSED;
	_LastNodeSatificated = true;

	m_UpLevelCount = m_QualifierShifts = 0;
	m_Qualifiers.SetSize();
}

bool XPathParser::Load(LPCSTR xpath)
{
	m_Qualifiers.ShrinkSize(0);
	m_UpLevelCount = 0;
	_LastNodeSatificated = true;
	
	_pXPath = xpath;
	xpath = _details::_skip_whitespace(xpath);

	m_UpLevelCount = 0;

	LPCSTR pLastSep = strrchr(xpath,'/');
	if(pLastSep == nullptr)
	{	m_bRelativePath = true;
		m_bIncludeDescendants = false;
		m_FinalQualifier.Load(xpath,(UINT)strlen(xpath));
		return ParseQualifier(m_FinalQualifier);
	}
	else
	{	LPCSTR pend;
		if(pLastSep[-1] == '/')
		{	m_bIncludeDescendants = true;
			pend = pLastSep - 1;
		}
		else
		{	m_bIncludeDescendants = false;
			pend = pLastSep;
		}
		pLastSep++;
		m_FinalQualifier.Load(pLastSep,(UINT)strlen(pLastSep));
		if( ParseQualifier(m_FinalQualifier) )
		{
			if(*xpath == '/')
			{	m_bRelativePath = false;
				xpath++;
			}else m_bRelativePath = true;

			while(xpath < pend)
			{
				LPCSTR next = strchr(xpath,'/');
				ASSERT(next);

				if(xpath[0] == '/')
				{	SetLastSyntaxError(ERR_XPATH_DESCENDANT_IN_HALFWAY,xpath);
					return false;
				}

				if(xpath[0] == '.' && xpath+1 == next){}
				else if(xpath[0] == '.' && xpath[1] == '.' && xpath+2 == next)
				{	
					if(m_Qualifiers.GetSize())
					{	m_Qualifiers.ShrinkSize(m_Qualifiers.GetSize()-1);
					}
					else m_UpLevelCount++;
				}
				else
				{	m_Qualifiers.push_back().Load(xpath,(UINT)(next - xpath));
				}

				xpath = next+1;
			}
		}

		for(UINT i=0;i<m_Qualifiers.GetSize();i++)
			if(!ParseQualifier(m_Qualifiers[i]))return false;

		return true;
	}

	return false;
}

rt::String_Ref	XMLParser::GetNodeName() const
{
	return _CurNode().TagName;
}

void XMLParser::GetNodeName(rt::String& name) const
{
	name = _CurNode().TagName;
}



} // namespace w32
