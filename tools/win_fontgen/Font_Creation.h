// Font_Creation.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CFont_CreationApp:
// See Font_Creation.cpp for the implementation of this class
//

class CFont_CreationApp : public CWinApp
{
public:
	CFont_CreationApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CFont_CreationApp theApp;