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
#include "buffer_type.h"


namespace rt
{

class XMLComposer
{
	struct _TagCache
	{	LPCSTR	tag;
		UINT	tag_pos;
	};
	LPCSTR					_EnteringNodeTag;
	void					_InsertPlainText(LPCSTR text);
	UINT					_DepthMax;

protected:
	rt::_File *             m_pStreamOut;
	SIZE_T					m_StreamOutOrgPos;
	rt::String				m_Output;
	UINT					m_WrittenLength;
	UINT					m_HeaderLength;

	rt::BufferEx<_TagCache>	m_NestLevelTag;

	void Linebreak(int vary = 0);
	void Output(char x){ if(m_pStreamOut){m_pStreamOut->Write(&x,1); m_WrittenLength++; }else{m_Output+=x;} }
	void Output(LPCSTR x){ if(m_pStreamOut){int l = (int)strlen(x); m_pStreamOut->Write(x,l); m_WrittenLength+=l; }else{m_Output+=x;} }

public:
	static const char		g_XML_Header[40];
	XMLComposer(rt::_File * out_stream = nullptr);	// if nullptr, output in m_Content, obtain by GetDocument/GetDocumentLength
	
	void ResetContent(LPCSTR customized_header = nullptr);
	void EnterNode(LPCSTR tag);
	void EnteringNode(LPCSTR tag);
	void EnteringNodeDone(bool compact = false);
	void SetAttribute(LPCSTR name, LPCSTR value = nullptr);  // call EnteringNode first, and call EnteringNodeDone when done
	void SetAttribute(LPCSTR name, int value);  // call EnteringNode first, and call EnteringNodeDone when done
	void SetAttribute(LPCSTR name, unsigned int value);  // call EnteringNode first, and call EnteringNodeDone when done
	void SetAttribute(LPCSTR name, long long value);  // call EnteringNode first, and call EnteringNodeDone when done
	void SetAttribute(LPCSTR name, unsigned long long value);  // call EnteringNode first, and call EnteringNodeDone when done
	void SetAttribute(LPCSTR name, float value);  // call EnteringNode first, and call EnteringNodeDone when done
	void SetAttribute(LPCSTR name, double value);  // call EnteringNode first, and call EnteringNodeDone when done

	void AddText(LPCSTR text);
	void AddCData(LPCSTR cdata);
	void ExitNode();
	void AppendTrail(LPCSTR text);
    void AddXML(const XMLComposer& another);

	LPCSTR		GetDocumentBuffer(bool withHeader = true) const;
	UINT		GetDocumentLength(bool withHeader = true) const;  // size of XML document in byte, not include teminator zero
	rt::String_Ref GetDocument(bool withHeader = true) const { return rt::String_Ref(GetDocumentBuffer(withHeader), GetDocumentLength(withHeader)); }
};

enum XMLParseError
{
	ERR_XML_OK							=  0,
	ERR_XML_HEADER_BAD_FORMAT			=  1,	// XML header should be <?xml version="1.0" encoding="UTF-8"?>
	ERR_XML_UNEXPECTED_ENDOFFILE		=  2,	// Unexpected end of file in parsing #
	ERR_XML_SYMBOL_UNEXPECTED			=  3,	// '#[0]' is not expected to appear here #
	ERR_XML_OUT_OF_MEMORY				=  4,	// Insufficient memory when paring #
	ERR_ATTRIBUTE_EQUALSIGN_NOT_FOUND	=  5,	// '=' not found after attrubute name #
	ERR_ATTRIBUTE_NAME_NOT_FOUND		=  6,	// attribute name not found, which is expected at #
	ERR_ATTRIBUTE_QUOTATION_NOT_MATCHED	=  7,	// '\'' in attribute value is not found to matched #
	ERR_NODE_CLOSURE_NOT_MATCHED		=  8,	// '</...>' is not found to match #
	ERR_TAG_CLOSURE_NOT_MATCHED			=  9,	// '>' is not found to match #
	ERR_CDATA_CLOSURE_NOT_MATCHED		= 10,	// ']]>' is not found to match #
	ERR_COMMENT_CLOSURE_NOT_MATCHED		= 11,	// '-->' is not found to match #
	ERR_PROC_INST_CLOSURE_NOT_MATCHED	= 12,	// '?>' or ']>' is not found to match #
	ERR_TAG_NAME_NOT_FOUND				= 13,	// tag name is not found after #
	ERR_XML_DEPTH_EXCEEDED				= 14,

	ERR_HTML_COMMENT_CLOSURE_NOT_MATCHED = 100,
	ERR_HTML_SPECIAL_NODE_UNKNONW,
	ERR_HTML_TAG_NAME_NOT_FOUND,
	ERR_HTML_ATTRIBUTE_NAME_NOT_FOUND,
	ERR_HTML_TAG_CLOSURE_NOT_MATCHED,
	ERR_HTML_QUOTATION_NOT_MATCHED,
	ERR_HTML_NODE_CLOSURE_NOT_MATCHED,
	ERR_HTML_UNEXPECTED_SYMBOL,
	ERR_HTML_CSS_OPERATOR_NOT_SUPPORTED		// only # and . CSS qualifer is supported in EnterXPathByCSSPath
};

enum XPathParseError
{
	ERR_XPATH_OK = 0,
	ERR_XPATH_DESCENDANT_IN_HALFWAY,				// descendants seletor '//' can only appear before last node qualifier
	ERR_SELECTOR_ATTRIBUTE_SET_NOT_SUPPORTED,		// Selects attributes is not supported
	ERR_SELECTOR_NODENAME_NOT_FOUND,				// nodename should appears after #
	ERR_SELECTOR_UNEXPECTED_SYMBOL,					// # is not expected here
	ERR_QUALIFIER_CONDITION_CLOSURE_NOT_MATCHED,	// ']' is not to match #
	ERR_QUALIFIER_CONDITION_NOT_SUPPORTED,			// only attributes conditions are supported
	ERR_QUALIFIER_VALUE_NOT_SUPPORTED,				// only string values are supported
	ERR_QUALIFIER_BAD_ORDINAL,						// non-number character found in [n] qualifier
	ERR_QUALIFIER_QUOTATION_NOT_MATCHED,			// '\'' in string value is not found to matched #
	ERR_QUALIFIER_OPERATOR_NOT_SUPPORTED,			// only = and != are supported
};

namespace _details
{
	struct _XML_Tag_Filter
	{	virtual bool _TagTest(const rt::String_Ref& tag_name, LPCSTR attributes, int DescendantLevel) = 0;	// true for pass
	};
};


/*////////////////////////////////////////////////////////////////////////////////////////////////////////////
A subset of xpatn syntex is supported:
Node Selection	like ../aa/bb
	nodename  		Selects all nodes as the named
	/ 				Selects from the root node
	// 				Selects nodes in the document from the current node that match the selection no matter where they are
					// should follows only one node, // * , //nodename , //xx[@att] are legal. //aa/cc is illegal.
	. 				Selects the current node
	.. 				Selects the parent of the current node
	*				Selects all child nodes (Wildcard)
	@ 				Selects attributes [** IS NOT SUPPORTED **]
Predicates suffix like /aa/bb[n]/ccc.
	[@att]			Selects child node by having specific attributes
	[@att='xx']		Selects child node by having specific attributes with specific value
	[@att!='xx']	Selects child node by not having specific attributes with specific value
	[@att:='xx']	Selects child node by having specific attributes with value starts with 'xx' [** An extension **]
	[@att?='xx']	Selects child node by having specific attributes with value ends with 'xx' [** An extension **]
	[@att~='xx']	Selects child node by having specific attributes with value containing with 'xx' [** An extension **]
	[@att^='xx']	Selects child node by having specific attributes with value containing with 'xx' as a whole word [** An extension **]

	[n]				Selects child node by order (zero-based)
Operators (not done yet)
	=				Equal, between strings
	!=				Inequal, between strings
	or				Logic or, between logic expression
	and				Logic and, between logic expression
////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
class XMLParser;
class XPathParser: public _details::_XML_Tag_Filter  // This class is not done yet
{	
	friend class XMLParser;
	XMLParser&			_XmlParser;
	LPCSTR				_pXPath;
	rt::String			_conv_value_temp;
	bool				_LastNodeSatificated;

protected:
	XPathParseError		m_XPathParseError;
	UINT				m_XPathParseErrorPosition;	// offset to _pXPath
	LPCSTR				SetLastSyntaxError(XPathParseError errnum, LPCSTR pos);

protected:

	enum _QualifierType	// grouped bit defined
	{	QT_NAME_NOT_PARSED		= 0x000,
		QT_NAME_ANY				= 0x001,		//  		*
		QT_NAME_EXACT			= 0x002,		//			nodename
		QT_ATTRIBUTE_ANY		= 0x010,		//			(not specified)
		QT_ATTRIBUTE_HAVE		= 0x020,		//			[@lang]
		QT_ATTRIBUTE_EQUAL		= 0x030,		//			[@lang='xx']
		QT_ATTRIBUTE_NOTEQUAL	= 0x040,		//			[@lang!='xx']
		QT_ATTRIBUTE_STARTWITH  = 0x050,		//			[@lang:='xx']
		QT_ATTRIBUTE_ENDWITH	= 0x060,		//			[@lang?='xx']
		QT_ATTRIBUTE_CONTAIN	= 0x070,		//			[@lang~='xx']
		QT_ATTRIBUTE_HAVE_WORD	= 0x080,		//			[@lang^='xx']
		QT_ATTRIBUTE_EXPRESSION = 0x090,		//			[@lang='xx' or @lang='yy'] or things more complicated [** not done yet **]
		QT_ORDINAL				= 0x100,		//			[n]
	};	

	struct _Qualifier
	{
		_QualifierType		qt_TagName;
		rt::String			TagName;
	
		_QualifierType		qt_Attribute;
		rt::String			Attribute;
		rt::String			Value;

		int					qt_Ordinal;		// -1 for none

		void	Load(LPCSTR qualify, UINT length);

		// runtime information
		int					_LastOrdinalTestPast;
	};
	rt::BufferEx<_Qualifier>	m_Qualifiers;

	bool						m_bRelativePath;
	bool						m_bIncludeDescendants;
	_Qualifier					m_FinalQualifier;
	int							m_UpLevelCount;
	int							m_QualifierShifts;
	_details::_XML_Tag_Filter*	m_pUserTagFilter;

	bool				ParseQualifier(_Qualifier& q);
	bool				_TagTest(const rt::String_Ref& tag_name, LPCSTR attributes, int DescendantLevel);
	bool				QualifierTest(_Qualifier& q, const rt::String_Ref& tag_name, LPCSTR attributes);
	void				_ClearLastOrdinalTestPast(int level);

public:
						XPathParser(XMLParser& xml_parser);
	bool				Load(LPCSTR xpath);
	void				Clear();
	XPathParseError		GetLastSyntaxError() const { return m_XPathParseError; }
	UINT				GetLastSyntaxErrorPostion() const { return m_XPathParseErrorPosition; }
};


class XMLParser		// no error handle of the content
{
	friend class XPathParser;
	rt::String	_content_copy;
	LPCSTR		_attribute_cursor;
	LPCSTR		_root_node_xml_start;
	LPCSTR		_root_node_xml_end;

	LPCSTR		_seek_next_attrib(LPCSTR start, rt::String_Ref& attrib_name, rt::String_Ref& value) const;
	// succeeded if both attrib_name !IsEmpty

	LPCSTR		_html_check_node_close(const rt::String_Ref& tagname, LPCSTR p, bool just_next);
	LPCSTR		_search_control_close(LPCSTR start) const ; // search '>' for '<!'
	LPCSTR		_search_node_close(LPCSTR start_inner, const rt::String_Ref& tag_name, LPCSTR* pInnerXML_End = nullptr) const;	// search for matched '</tag_name>', return pointer to '>' OuterXML_End, pInnerXML_End to '<' InnerXML_End
	LPCSTR		_search_special_node_close(LPCSTR start_outer) const;	// search ending for <!--,<?,<!. return pointer to '>'
	bool		_search_attrib(LPCSTR start, LPCSTR attrib_name, rt::String_Ref& value) const ; // if found, return pointer where parsing stopped, not found or error return nullptr

public:
	static  void	_convert_xml_to_text(const rt::String_Ref& string, rt::String& text, bool TrimXMLCode = false, bool TrimSubnodes = false, char sep = ' ');

	static	LPCSTR	_seek_tag_open(LPCSTR start) { return strchr(start,'<'); }	// nullptr for not found, or a pointer to '<'
	static	LPCSTR	_seek_tag_close(LPCSTR start) { return strchr(start,'>'); }  // nullptr for not found, or a pointer to '>'
	static	LPCSTR	_search_node_close(XMLParseError* pErrorCode, LPCSTR start_inner, const rt::String_Ref& tag_name, LPCSTR* pInnerXML_End = nullptr);	// search for matched '</tag_name>', return pointer to '>' OuterXML_End, pInnerXML_End to '<' InnerXML_End
	static	LPCSTR	_search_special_node_close(XMLParseError* pErrorCode, LPCSTR start_outer);	// search ending for <!--,<?,<!. return pointer to '>'
	template<typename t_Val> static INLFUNC  t_Val*	_seek_symbol_end(t_Val* start)
	{ for(;(*start>=0x2d || *start=='#' || *start<0) && *start!='/' && *start!='<' && *start!='>' && *start!='=' && *start!='[' && *start!=']' ;start++){};  return start; }
	// Be sure extended-ascii, ':', '.', '#' is not a symbol_end !!

protected:
	XMLParseError		m_XMLParseError;
	UINT				m_XMLParseErrorPosition;	// offset to m_pDocument
	LPCSTR				SetLastSyntaxError(XMLParseError errnum, LPCSTR pos);

	XPathParser					m_XPathParser;
	_details::_XML_Tag_Filter*	m_pUserTagFilter;
	_details::_XML_Tag_Filter*	m_pCurTagFilter;
	bool						m_bTrySkipError;
	void						ClearXPathFilter();

protected:
	struct _node
	{
		rt::String_Ref	TagName;
		rt::String_Ref	Attributes;
		LPCSTR			OuterXML_Start;			// pointing to a '<'
		LPCSTR			InnerXML_Start;			// (InnerXML_Start-1) pointing to a '>', or 0
		bool			IsCompactNode;			// no child node, if true, InnerXML_Start is actaully OuterXML_End
	};
	LPCSTR				_search_node_close(_node& node, LPCSTR* pInnerXML_End = nullptr) const ;	// search for matched '</tag_name>', return pointer to '>' OuterXML_End, pInnerXML_End to '<' InnerXML_End

	LPCSTR				m_pDocument;
	rt::BufferEx<_node>	m_NodePath;

	bool				_ForceWellformed(rt::String& out_in);		// false if error found and fixed
	bool				_EnterNextNode(LPCSTR start, _details::_XML_Tag_Filter* pFilter = nullptr, bool replace_node = false);
	const _node&		_CurNode() const { ASSERT(m_NodePath.GetSize()); return m_NodePath[m_NodePath.GetSize()-1]; }
	void				ClearSyntaxError();
	
public:
	XMLParser();
	XMLParser(const XMLParser& xml);
	const XMLParser& operator = (const XMLParser& xml);
	bool			LoadHTML(const rt::String_Ref& doc){ return LoadHTML(doc.Begin(),(UINT)doc.GetLength()); }
	bool			LoadHTML(LPCSTR pHTML, UINT len = INFINITE);	// HTML will be convert to XHTML, p will not be held, p[] will be modified slightly
	bool			Load(LPCSTR text, bool Keep_referring, SSIZE_T text_len = -1);	// if Keep_referring, the text pointer will be kept for document accessing
	bool			IsLoaded() const { return (bool)m_NodePath.GetSize(); }

	void			SetUserTagFilter(_details::_XML_Tag_Filter* pFilter = nullptr){ m_pUserTagFilter = m_XPathParser.m_pUserTagFilter = pFilter; }
	UINT			GetDescendantLevel(){ return (UINT)m_NodePath.GetSize(); }	// starting from 1, 0 indicate error

	static void		ConvertExtendedCSSPathToXPath(LPCSTR extended_css_path, rt::String& xpath);
	bool			EnterXPathByCSSPath(const rt::String_Ref& css_path);
	bool			EnterXPath(LPCSTR xpath);	// Enter first node of the xpath selected set, call EnterSucceedNode for more nodes
	void			EscapeXPath();				// call this to remove XPath filtering

	void			EnterRootNode();	// equivalent to xpath: //* 
	bool			EnterParentNode();	// equivalent to xpath: ../*

	bool			EnterChildNode(UINT n_th, LPCSTR tag_name = nullptr);	// n_th starts from 1
	bool			EnterFirstChildNode(LPCSTR tag_name = nullptr);
	bool			EnterNextSiblingNode();
	bool			EnterNextSiblingNode(UINT n_th);
	bool			EnterSucceedNode();	// go succeed node as depth-first traveling order

	bool			HasAttribute(LPCSTR name) const;
	bool			GetAttribute(LPCSTR name, rt::String& value) const;
	bool			GetAttribute(LPCSTR name, rt::String& value, LPCSTR default_value) const;
	bool			GetAttribute_Bool(LPCSTR name, bool default_value = false) const;
	bool			GetAttribute_Path(LPCSTR name, rt::String& value) const;
	INT				GetAttribute_Int(LPCSTR name, INT default_value = 0) const;
	LONGLONG		GetAttribute_Int64(LPCSTR name, LONGLONG default_value = 0) const;
	ULONGLONG		GetAttribute_FileSize(LPCSTR name, ULONGLONG default_value = 0) const;
	double			GetAttribute_Float(LPCSTR name, double default_value = 0.0) const;
	int				GetAttribute_Timespan(LPCSTR name, int default_value_msec) const;  // msec

    bool            GetAttribute_BoolRef(LPCSTR name, bool& value) const;
	bool            GetAttribute_BoolRef(LPCSTR name, bool& value, bool default_value) const;
    bool            GetAttribute_IntRef(LPCSTR name, INT& value) const;
	bool            GetAttribute_IntRef(LPCSTR name, INT& value, INT default_value) const;
	bool			GetAttribute_FloatRef(LPCSTR name, double& value) const;
	bool			GetAttribute_FloatRef(LPCSTR name, double& value, double default_value) const;

    bool			GetFirstAttribute(rt::String& name, rt::String& value) const;
	bool			GetNextAttribute(rt::String& name, rt::String& value) const;

	rt::String_Ref	GetInternalDocument() const;
	bool			GetNodeDocument(rt::String& doc_out);
	XMLParser		GetNodeDocument(int nth_parent = 0) const;
	void			GetNodeName(rt::String& name) const;
	rt::String_Ref	GetNodeName() const;			// return the tag name
	bool			GetOuterXML(rt::String& text) const;
	rt::String_Ref	GetOuterXML() const;
	bool			GetInnerXML(rt::String& text) const;
	rt::String_Ref	GetInnerXML() const;
	bool			GetAttributesXML(rt::String& text) const;
	bool			GetInnerText(rt::String& text, bool no_text_from_subnodes = false) const;
	static void		ExtractInnerText(const rt::String_Ref& doc, rt::String& out, char sep = '\t');
	bool			GetInnerRtfHtml(rt::String& text) const;
	rt::String_Ref	GetInnerCDATA() const;
	bool			GetInnerCDATA(rt::String& out) const { out = GetInnerCDATA(); return !out.IsEmpty(); }
	ULONGLONG		GetNodeDocumentOffset() const;	// the begining of outer xml of the current node, related to the begining of the input buffer;

	XMLParseError	GetLastSyntaxError() const { return m_XMLParseError; }
	UINT			GetLastSyntaxErrorPostion() const { return m_XMLParseErrorPosition; }
	void			Clear();
	void			EnableSyntaxFaultTolerance(bool enable = true){ m_bTrySkipError = enable; }
	void			TextDump();
	ULONGLONG		GetNodeFileOffset() const;
	rt::String_Ref	GetConvertedXHTML() const;

	void			GetXPathAsCSSPath(rt::String& out);
};

} // namespace rt

