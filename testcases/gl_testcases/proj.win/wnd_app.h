#pragma once

//////////////////////////////////////////////////////////////////////
// Copyright 2012 the Cicada Project Dev Team. All rights reserved.
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
//     * Neither the name of Cicada.  nor the names of its
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

#include "../../../../CPCF/core/os/predefines.h"
#include "../../../../CPCF/core/rt/string_type.h"

#ifdef CreateDialog
#undef CreateDialog
#endif

namespace os
{

namespace _details{ class _WndBase_Class; }

void		SetResourceModule(HANDLE DLL);
HINSTANCE	GetResourceModule();
bool		OpenDefaultBrowser(LPCTSTR url, DWORD show_window);
void		PumpMessages();
bool		LoadResource(rt::String& out, LPCTSTR res_name_id, LPCTSTR res_type, HINSTANCE module = NULL);

class WndBase
{	
	friend class _details::_WndBase_Class;
	friend void SetResourceModule(HANDLE);
	friend HINSTANCE GetResourceModule();
private:
	static HMODULE	 _hModuleHandle;
	static WndBase* _pMainWnd;

public:
	enum
	{	ALIGN_X = 1,
		ALIGN_Y = 2,
		ALIGN_WIDTH = 4,
		ALIGN_HEIGHT = 8,
	};

	WndBase();
	virtual ~WndBase();
	void CenterWindow();

	bool Create(DWORD WinStyle = WS_OVERLAPPEDWINDOW,HWND Parent = NULL);
    bool CreatePopupWindow(HWND Parent = NULL);
	bool CreateLayeredWindow(INT x, INT y, INT w, INT h, HWND Parent = NULL);
	bool CreateDialog(LPCTSTR lpDlgTemplate, HWND Parent = NULL);
	bool CreateControl(LPCTSTR ClassName, DWORD style, const RECT& rc, HWND parent);
	bool CreateToolWnd(HWND Parent = NULL);
	void EnableWindowDragFromClientRegion(bool yes = true);
	void DestroyWindow();
	bool IsEmpty() const { return hWnd == NULL; } 
	HWND GetSafeHwnd(){ return hWnd; }
	operator HWND(){ return GetSafeHwnd(); }
	operator HDC(){ return GetDC(); }

	bool	Attach(HWND hwnd);
	HWND	Detach();

	int		MessageBox(LPCTSTR lpText,LPCTSTR lpCaption = _T("Information"),UINT uType = MB_OK);
	DWORD	GetStyle(){ return ::GetWindowLong(*this,GWL_STYLE); } 
	DWORD   GetStyleEx(){ return ::GetWindowLong(*this,GWL_EXSTYLE); } 
	void	SetStyle(DWORD style){ ::SetWindowLong(*this,GWL_STYLE,style); }
	void	SetStyleEx(DWORD style){ ::SetWindowLong(*this,GWL_EXSTYLE,style); }
	void	SetTopMost(bool is = true){ ::SetWindowPos(*this,is?HWND_TOPMOST:HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE); }
	void	ResizeWindowByClientArea(int Width,int Height);
	void	SwitchFullScreen();
	bool	IsInFullScreen(){ return _IsInFallScreen; }
    bool    IsChildWindow();
	void	AlignTo(HWND Reference,DWORD AlignFlag = WndBase::ALIGN_X|WndBase::ALIGN_Y|WndBase::ALIGN_WIDTH|WndBase::ALIGN_HEIGHT);

	void	MainMenu_SetCheck(UINT Id,bool Check);
	void	MainMenu_Enable(UINT Id,bool Enable);
	void	MainMenu_SetText(UINT Id,LPCTSTR);
	void	SetMenu(HMENU hMenu){ ::SetMenu(*this,hMenu); }
	void	SetIcon(HICON hIcon, bool bBigIcon){ SendMessage(WM_SETICON, bBigIcon, (LPARAM)hIcon); }
	void	SetMenu(LPCTSTR ResId);
	void	SetIcon(LPCTSTR ResId, bool bBigIcon);
	void	SetMenu(UINT ResId){ SetMenu(MAKEINTRESOURCE(ResId)); }
	void	SetIcon(UINT ResId, bool bBigIcon){ SetIcon(MAKEINTRESOURCE(ResId),bBigIcon); }
	void	SetTimer(SIZE_T timer_id, UINT interval){ ::SetTimer(hWnd, (UINT_PTR)timer_id, interval, NULL); }
	void	KillTimer(SIZE_T timer_id){ ::KillTimer(hWnd, timer_id); }  
	// WIN32 API wrapper
	HWND	GetDlgItem(int nCtrlID){ return ::GetDlgItem(*this,nCtrlID); }
	bool	GetClientRect(LPRECT lpRect){ return ::GetClientRect(*this,lpRect); }
	bool	GetWindowRect(LPRECT lpRect){ return ::GetWindowRect(*this,lpRect); }
	bool	ScreenToClient(LPPOINT lpPoint){ return ::ScreenToClient(*this,lpPoint); }
	bool	ClientToScreen(LPPOINT lpPoint){ return ::ClientToScreen(*this,lpPoint); }
	bool	EnableWindow(bool bEnable = true){ return ::EnableWindow(*this,bEnable); }
	bool	SetWindowText(LPCTSTR str){ return ::SetWindowText(*this,str); }
	bool	GetWindowText(LPTSTR str, int count){ return ::GetWindowText(*this,str,count); }
	HWND	GetDlgItem(UINT item_id){ return ::GetDlgItem(*this, item_id); }
#ifdef UNICODE
	bool	SetWindowTextA(LPCSTR str){ return ::SetWindowTextA(*this,str); }
	bool	GetWindowTextA(LPSTR str, int count){ return ::GetWindowTextA(*this,str,count); }
#else
	bool	SetWindowTextW(LPCWSTR str){ return ::SetWindowTextW(*this,str); }
	bool	GetWindowTextW(LPWSTR str, int count){ return ::GetWindowTextW(*this,str,count); }
#endif
	bool	ShowWindow(int nCmdShow=SW_SHOW){ return ::ShowWindow(*this,nCmdShow); }
	bool	GetWindowPlacement(WINDOWPLACEMENT* plc){ return ::GetWindowPlacement(*this,plc); }
	UINT	GetShowWindowState(){ WINDOWPLACEMENT plc; return GetWindowPlacement(&plc)?plc.showCmd:INFINITE; }
	HWND	SetFoucs(){ return ::SetFocus(*this); }
	HWND	SetCapture(){ return ::SetCapture(*this); }
	bool	ReleaseCapture(){ return ::ReleaseCapture(); }
	LRESULT SendMessage(UINT Msg, WPARAM wParam, LPARAM lParam){ return ::SendMessage(hWnd,Msg,wParam,lParam); }  
	bool	SetWindowPos(HWND hWndAfterZ, int x, int y, int cx, int cy, UINT nFlags){ return ::SetWindowPos(hWnd,hWndAfterZ,x,y,cx,cy,nFlags); }
	bool	RedrawWindow(const RECT *lprcUpdate = NULL, HRGN hrgnUpdate = NULL,UINT flags = 0){ return ::RedrawWindow(hWnd,lprcUpdate,hrgnUpdate,flags); }

	void	EnableResizing(bool is = false);
	void	HostChildWindow(HWND hChild = NULL);
	void	BringWindowToTop(){ ShowWindow(); ::BringWindowToTop(*this); }
	void	SetAsMainWnd(){ _pMainWnd = this; }
	HDC		GetDC(){ if(hDC){}else{hDC = ::GetDC(hWnd);} return hDC; }
	void	ReleaseDC(){ if(hDC){ ::ReleaseDC(hWnd,hDC); hDC = NULL; } }

protected:
	HDC		hDC;   // obtained by first call of GetDC();
	HWND	hWnd;
	HWND	Hosted_Child;
	bool	SetupAfterCreation(bool attached = false);
	virtual LRESULT WndProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual bool	OnInitWnd(){ return true; };

private:
	bool		_bAsDialog;
	bool		_bMoveWindowByClientRegion;
	bool		_bAutoDeleteThis;
	WndBase*	_Next_AutoDelWnd;

	bool		_IsInFallScreen; // following available only when _IsInFallScreen==true
	RECT		_OrgLayout;
	DWORD		_OrgStyle;
	DWORD		_OrgStyleEx;
	HWND		_OrgParentWnd;
	WNDPROC		_WndOldProc;
};

//bool WaitForEnteringCriticalSection_UI(w32::CCriticalSection& ccs, UINT timeout = INFINITE);  // return false when WM_QUIT received or timeout
//bool WaitForThreadEnding_UI(HANDLE hThread, UINT time_wait_ms = INFINITE);
//bool WaitForProcessEnding_UI(HANDLE Process, UINT time_wait_ms);

extern bool SwitchToWindow(HWND hWnd, DWORD timeout = INFINITE);

} // namespace os




