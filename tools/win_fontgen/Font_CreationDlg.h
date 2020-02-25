// Font_CreationDlg.h : header file
//

#pragma once

#include <atlimage.h>
#include <gdiplus.h>
#include "afxcmn.h"
#pragma comment(lib,"gdiplus.lib")

#include "unicode_chart.h"
using namespace unicode;

#include "CheckBoxTreeCtrl.h"
#include "fontenum.h"
#include "afxwin.h"

#include "../../core/gl/gl_gdi.h"


struct FontInfo
{
	FontInfo()
	{	
		ZeroMemory(this,sizeof(FontInfo)); 
	}
	bool IsAvailable()const{ return m_FaceName[0]; }

	WCHAR				m_FaceName[LF_FACESIZE];

	int					m_FontSize;
	UINT				m_FontQuality;
	UINT				m_FontAntiAliasMode;
	UINT				m_FontContrast;
	BOOL 				m_FontBold;
	BOOL 				m_FontItalic;
	BOOL 				m_FontUnderline;
	BOOL 				m_FontStrikeout;
	BOOL				m_FixedWidth;
	int					m_Baseline;
	int 				m_WidthShrink;
	int 				m_LeftMargin;

	void				CopyFontInfo(const FontInfo& x)
	{
		memcpy(this,&x,sizeof(FontInfo));
	}

};

// CFont_CreationDlg dialog
class CFont_CreationDlg : public CDialog
						, protected FontInfo
{
// Construction
public:
	CFont_CreationDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_FONT_CREATION_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

// Font data
	ATL::CImage			m_FontImage;

// Code Chart
	UINT				m_SelectedCodeArea;
	HTREEITEM			m_RegionEntry[UR_REGION_MAX];

	HTREEITEM			m_CodeAreaEntry[UNICODE_CHART_SIZE];
	FontInfo			m_CodeAreaFontInfo[UNICODE_CHART_SIZE];
	UINT				m_CodeAreaChecked[UNICODE_CHART_SIZE];  // length = m_CodeAreaCheckedCount. refer to m_CodeAreaEntry
	UINT				m_CodeAreaCheckedCount; 
	UINT				m_CharacterCount;

// Text Previewing
	bool				m_bShowGrid;
	UINT				m_FontCharSeed;
	CFontEnum			m_FontList;
	WCHAR				m_CodeMin,m_CodeMax;

	UINT				m_FontSelected;	// refer to m_FontList
	bool				m_IsFontListUpdated;
	UINT				m_FontHeight;



// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

	void UpdateFontList(bool filter_charset);
	bool SelectFontList(LPCWSTR facename);
	void RedrawPreviewImage();
	void RedrawPreivewText();
	void SetupUI();

public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButtonTest();
	afx_msg void OnBnClickedButtonTest2();
	CString m_sInput;
	CCheckBoxTreeCtrl m_wChartTree;
	CStatic m_wChartInfo;
	CComboBox m_wFontPicker;
	CComboBox m_wFontSize;
	CString m_sFontSize;
	CString m_sFontContrast;
	CComboBox m_wFontContrast;
	CComboBox m_wFontAA;
	CComboBox m_wFontQuality;
	CComboBox m_wFontWeight;
	CString m_sFontWeight;
	CStatic m_wTotalInfo;
	CButton m_wFixedWidth;
	BOOL m_bInvert;
	BOOL m_bOverallDisplay;
	CComboBox m_wPixelFormat;
	int m_PixelFormat;

	gl::gdiFont		m_gdiFont;

	afx_msg void OnTvnSelchangedCodeChart(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTvnCheckchangedCodeChart(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnCbnSelchangeFontlist();
	afx_msg void OnCbnEditchangeFontsize();
	afx_msg void OnBnClickedFontBold();
	afx_msg void OnBnClickedFontBold2();
	afx_msg void OnBnClickedFontBold3();
	afx_msg void OnBnClickedFontBold4();
	afx_msg void OnCbnSelchangeFontsize();
	afx_msg void OnCbnSelchangeFontsize2();
	afx_msg void OnCbnEditchangeFontsize2();
	afx_msg void OnCbnSelchangeFontAa();
	afx_msg void OnCbnSelchangeFontQuality();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnCbnSelchangeFontWeight();
	afx_msg void OnCbnEditchangeFontWeight();
	afx_msg void OnBnClickedButtonCopy();
	afx_msg void OnBnClickedButtonPaste();
	afx_msg void OnBnClickedButtonSave();
	afx_msg void OnCbnDropdownFontlist();
	afx_msg void OnBnClickedCheck1();
	afx_msg void OnBnClickedFontBold5();
	afx_msg void OnEnChangeEdit3();
	afx_msg void OnEnChangeEdit4();
	afx_msg void OnEnChangeEdit5();
	afx_msg void OnEnChangeEdit2();
	afx_msg void OnBnClickedDisplayInvert();
	afx_msg void OnBnClickedButtonImport();
	afx_msg void OnBnClickedButtonExport();
	afx_msg void OnBnClickedButtonSave2();
	afx_msg void OnCbnSelchangeCombo1();
	afx_msg void OnBnClickedButtonGenerate();
	afx_msg void OnBnClickedButtonTestUfg();
	CString m_sTestText;
	afx_msg void OnEnChangeTestText();
	int m_GlyphLineWidthIndex;
	int m_GlyphLineWidth;
	CComboBox m_wImageStep;
	BOOL m_bSwizzleMap;
	afx_msg void OnBnClickedButtonImg2array();
	afx_msg void OnBnClickedButtonCopycode();
};



//////////////////////////////////////////////////
// File Format for Unicode Glyph Bitmap (*.ufg)
#pragma pack(1)
struct _FontGlyphIndex
{	
	//DWORD	Width8_OffsetVBit24;
	void SetValue(UINT Width, UINT Offset_VBit)
	{
		GlyphWidth = Width;
		OffsetToGlyphBitmap = Offset_VBit;
		//Width8_OffsetVBit24 = Width<<24 | (0xffffff&Offset_VBit);
	}
	WORD	GlyphWidth;
	UINT	OffsetToGlyphBitmap; //offset relative to GlyphBitmap_Offset
};

struct _FontGroup
{
	UINT		CodeMin;
	UINT		CodeMax;
	int			GlyphWidth_IndexOffset;		// <0 is Fixed width font and >0 is Offset to GlyphIndexTable
	UINT		GlyphBitmap_PixelStart;		// in pixel of width
	int			Margin;
	int			Baseline;
};

struct _UFG_Header
{
	enum
	{	SIGN_MAGIC = 0x4d424655,
		Flag_Swizzled_GlyphLine = 0x80000000,
		Flag_NoAntialiasing		= 0x40000000,
	};

	DWORD		Sign; // "UFBM"
	DWORD		Flag_BPP; // low 16bit is 8BPP / 4BPP / 2BPP / 1BPP, high 16bit is flag
	UINT		GlyphHeight;
	UINT		GlyphStep;
	UINT		FontGroupCount;
	UINT		FontGroups_Offset;
	UINT		GlyphIndexTable_Size;   //in byte
	UINT		GlyphIndexTable_Offset;
	UINT		GlyphBitmap_Size;		//in byte
	UINT		GlyphBitmap_Offset;
};
#pragma pack()

