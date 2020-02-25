#pragma once

#include <vector>

class CFontEnum
{
	static int CALLBACK _EnumFontFamExProc(	  ENUMLOGFONTEX *lpelfe,    // logical-font data
											  NEWTEXTMETRICEX *lpntme,  // physical-font data
											  DWORD FontType,           // type of font
											  LPARAM lParam             // application-defined data
											);

	//static int CALLBACK _EnumFontFamExProc(	  ENUMLOGFONT *lpelfe,    // logical-font data
	//										  NEWTEXTMETRIC *lpntme,  // physical-font data
	//										  DWORD FontType,           // type of font
	//										  LPARAM lParam             // application-defined data
	//										);

protected:
	struct _FontInfo
	{	TCHAR	FaceName[LF_FACESIZE];
		UINT	Charset;
		bool	FixedPitch;
		UINT	Type;
	};

	HDC		m_DesktopDC;

protected:
	std::vector<_FontInfo>	m_FontList;

public:
	CFontEnum();
	~CFontEnum();
	UINT SearchFont(LPCTSTR facename);	// return 0xffffffff if not found
	void EnumerateFont();
	UINT GetSize()const{ return (UINT)m_FontList.size(); }
	LPCTSTR GetCharsetName(UINT charset);





	template<typename T>
	const _FontInfo&  operator [](T i)const{ ASSERT(i<GetSize()); return m_FontList[i]; }
};

