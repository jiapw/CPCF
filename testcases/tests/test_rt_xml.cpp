#include "../../core/rt/xml_xhtml.h"

#pragma warning(disable: 4838)

#include "test.h"
void rt::UnitTests::xml()
{
	{	rt::String val;
	rt::XMLParser::_convert_xml_to_text("abcd & fff-&lt;-<cdnsion cnindi>!</v>??", val, true);
	_LOG(val);
	}

	rt::XMLComposer xml;
	xml.EnteringNode("root");
	xml.SetAttribute("name", "testing");
	xml.SetAttribute("height", 12.5f);
	xml.EnteringNodeDone();

	xml.EnteringNode("item");
	xml.SetAttribute("key", "no");
	xml.EnteringNodeDone();
	xml.AddText("/*Some Inner Text*/");
	xml.ExitNode();

	xml.EnteringNode("item");
	xml.SetAttribute("key", "<yes>");
	xml.SetAttribute("height", 12);
	xml.EnteringNodeDone(true);

	xml.EnteringNode("item");
	xml.SetAttribute("key", "no");
	xml.EnteringNodeDone();
	xml.AddText("Another Node");
	xml.ExitNode();

	xml.ExitNode();

	_LOG(xml.GetDocument());

	rt::XMLParser	xmlp;
	rt::String		val;
	xmlp.Load(xml.GetDocumentBuffer(), true, xml.GetDocumentLength());
	if (xmlp.EnterXPath("/root/item[@key='no']") && xmlp.GetInnerText(val))
		_LOG("EnterXPath: " << val);

	rt::XMLParser node = xmlp.GetNodeDocument();
	node.GetAttribute("key", val);
	_LOG("key = " << val);

	if (xmlp.EnterXPath("/root"))
	{
		xmlp.GetInnerText(val);
		_LOG("mixed inner text: " << val);
	}

	xmlp.EnterRootNode();
	xmlp.TextDump();
}

void rt::UnitTests::html()
{
	os::FileRead<char>	file("category_page.htm");
	rt::XMLParser	html;

	if (html.LoadHTML(file, (UINT)file.GetSize()))
	{
		_LOG("Loaded");

		rt::String xml;
		html.EnterXPath("/html/head/meta");
		html.GetOuterXML(xml);

		_LOG(xml);

		os::File("xhtml.htm", os::File::Normal_Write).Write(html.GetConvertedXHTML());
	}
}
