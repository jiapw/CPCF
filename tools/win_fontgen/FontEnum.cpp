#include "StdAfx.h"
#include "FontEnum.h"



int CALLBACK CFontEnum::_EnumFontFamExProc(	  ENUMLOGFONTEX *lpelfe,    // logical-font data
											  NEWTEXTMETRICEX *lpntme,  // physical-font data
											  DWORD FontType,           // type of font
											  LPARAM lParam             // application-defined data
											)
{
	if(	lpelfe->elfLogFont.lfFaceName[0] != _T('@') && FontType!=1 &&
		((CFontEnum*)lParam)->SearchFont(lpelfe->elfLogFont.lfFaceName)==0xffffffff )
	{
		_FontInfo fi;
		memcpy(fi.FaceName,	lpelfe->elfLogFont.lfFaceName,	sizeof(fi.FaceName));
		fi.Charset		= lpelfe->elfLogFont.lfCharSet;
		fi.FixedPitch	= (lpelfe->elfLogFont.lfPitchAndFamily&0x3)!=VARIABLE_PITCH;
		fi.Type			= FontType;

		((CFontEnum*)lParam)->m_FontList.push_back(fi);
	}

	return 1;
}

CFontEnum::CFontEnum()
{
	m_DesktopDC = GetDC(NULL);
}

CFontEnum::~CFontEnum()
{
	::ReleaseDC(NULL,m_DesktopDC);
}

void CFontEnum::EnumerateFont()
{
	LOGFONT lf;
	lf.lfCharSet = DEFAULT_CHARSET;
	lf.lfFaceName[0] = _T('\0');
	lf.lfPitchAndFamily = 0;
	::EnumFontFamiliesEx(m_DesktopDC,&lf,(FONTENUMPROC)CFontEnum::_EnumFontFamExProc,(LPARAM)this,0);
}

UINT CFontEnum::SearchFont(LPCTSTR facename)
{
	for(UINT i=0;i<GetSize();i++)
	{
		if(_tcsicmp(facename,m_FontList[i].FaceName)==0)
			return i;
	}

	return 0xffffffff;
}

LPCTSTR CFontEnum::GetCharsetName(UINT charset)
{
	switch(charset)
	{
	case ANSI_CHARSET:			return _T("ANSI");
	case BALTIC_CHARSET:		return _T("Baltic");
	case CHINESEBIG5_CHARSET:	return _T("BIG5");
	case DEFAULT_CHARSET:		return _T("Default");
	case EASTEUROPE_CHARSET:	return _T("East Europe");
	case GB2312_CHARSET:		return _T("GB2312");
	case GREEK_CHARSET:			return _T("Greek");
	case HANGUL_CHARSET:		return _T("Hangul");
	case MAC_CHARSET:			return _T("MAC");
	case OEM_CHARSET:			return _T("OEM");
	case RUSSIAN_CHARSET:		return _T("Russian");
	case SHIFTJIS_CHARSET:		return _T("Shift-JIS");
	case SYMBOL_CHARSET:		return _T("Symbol");
	case TURKISH_CHARSET:		return _T("Turkish");
	case VIETNAMESE_CHARSET:	return _T("Vietnamese");
	case JOHAB_CHARSET:			return _T("Johab");
	case ARABIC_CHARSET:		return _T("Arabic");
	case HEBREW_CHARSET:		return _T("Hebrew");
	case THAI_CHARSET:			return _T("Thai");
	default:					return _T("Undefined");
	}
}




















