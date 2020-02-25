// Font_Creation.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "Font_Creation.h"
#include "Font_CreationDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CFont_CreationApp
BEGIN_MESSAGE_MAP(CFont_CreationApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CFont_CreationApp construction
CFont_CreationApp::CFont_CreationApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CFont_CreationApp object
CFont_CreationApp theApp;


// CFont_CreationApp initialization
BOOL CFont_CreationApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	{
		CFont_CreationDlg dlg;
		m_pMainWnd = &dlg;
		INT_PTR nResponse = dlg.DoModal();
	}

	Gdiplus::GdiplusShutdown(gdiplusToken);

	return false;
}

