#include "../../../../CPCF/core/rt/runtime_base.h"
#include "../../../../CPCF/core/os/file_dir.h"
#include "wnd_app.h"

#include <Wincon.h>
#include <io.h>
#include <fcntl.h>
#include <Shellapi.h>

#pragma warning(disable: 4302)

namespace os
{

namespace _details
{	

enum _tagWindowClassType
{
	WCT_NORMAL = 0,
	WCT_LAYERED,

	WCT_MAX
};

class _WndBase_Class
{	friend class WndBase;
protected:
	static LRESULT CALLBACK _CommonWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{	
#pragma warning(disable:4312)
		WndBase * P_WND = (WndBase *)::GetWindowLongPtr(hWnd,GWLP_USERDATA);
#pragma warning(default:4312)
		if(P_WND)
		{
			ASSERT(P_WND != NULL);
			return P_WND->WndProc(message,wParam,lParam);
		}
		else
			return DefWindowProc(hWnd,message,wParam,lParam);
	}
	static INT_PTR CALLBACK _CommonDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{	
#pragma warning(disable:4312)
		WndBase * P_WND = (WndBase *)::GetWindowLongPtr(hWnd,DWLP_USER);
#pragma warning(default:4312)
		if(P_WND)
		{
			return P_WND->WndProc(message,wParam,lParam);
		}
		else
			return 0;
	}
	static void  _RegisterClass(UINT class_style, LPCTSTR prefix, _tagWindowClassType type)
	{
		ASSERT(type < WCT_MAX);
		HMODULE hInstance = ::GetModuleHandle(NULL);
#if defined(PLATFORM_64BIT)
		int len = _stprintf(Name[type],_T("%08X%08X"),(UINT)hInstance,(UINT)(((ULONGLONG)hInstance)>>32));
#else
		int len = _stprintf(Name[type],_T("%08X"),(UINT)hInstance);
#endif
		memcpy(&Name[type][len],prefix, rt::min((int)_tcslen(prefix),16));

		WNDCLASSEX wcex;
		wcex.cbSize = sizeof(WNDCLASSEX);

		wcex.style			= class_style;
		wcex.hbrBackground	= NULL;

		wcex.lpfnWndProc	= (WNDPROC)_CommonWndProc;
		wcex.cbClsExtra		= 4; // Hold this of WndBase
		wcex.cbWndExtra		= sizeof(LPVOID);  //Holds a pointer to CwglWnd Object
		wcex.hInstance		= hInstance;
		wcex.hIcon			= NULL;
		wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
		wcex.lpszMenuName	= NULL;
		wcex.lpszClassName	= Name[type];
		wcex.hIconSm		= NULL;
		VERIFY(RegisterClassEx(&wcex));
	}
	static TCHAR Name[WCT_MAX][16+1+sizeof(LPVOID)*2];
	
public:
	_WndBase_Class(){ ZeroMemory(Name,sizeof(Name)); }
	static LPCTSTR GetWindowClassName(_tagWindowClassType type = os::_details::WCT_NORMAL)
	{	
		ASSERT(type < WCT_MAX);
		if(Name[type][0]){}
		else
		{	switch(type)
			{
				case WCT_NORMAL: _RegisterClass(0, _T("os::WndBase"),type); break;
				case WCT_LAYERED: _RegisterClass(CS_HREDRAW|CS_VREDRAW, _T("os::CWndLayered"),type); break;
				default: ASSERT(0);
			}
		}
		return Name[type];
	}
	~_WndBase_Class()
	{	for(UINT i=0;i<WCT_MAX;i++)
			if(Name[0])::UnregisterClass(Name[0],::GetModuleHandle(NULL));
	}
};
_WndBase_Class _TheWndClass;
TCHAR _WndBase_Class::Name[WCT_MAX][16+1+sizeof(LPVOID)*2];

} // namespace _details

} // namespace os

os::WndBase* os::WndBase::_pMainWnd = NULL;
HMODULE		   os::WndBase::_hModuleHandle = ::GetModuleHandle(NULL);

os::WndBase::WndBase()
{
	hWnd = NULL;
	Hosted_Child = NULL;
	hDC = NULL;
	_IsInFallScreen = false;
	_bAutoDeleteThis = false;
	_Next_AutoDelWnd = NULL;
	_bMoveWindowByClientRegion = false;
	_bAsDialog = false;
	_WndOldProc = NULL;
}

void os::WndBase::EnableWindowDragFromClientRegion(bool yes)
{
	_bMoveWindowByClientRegion = yes;
}

bool os::WndBase::IsChildWindow()
{   
    return GetStyle()&WS_CHILD;
}

bool os::WndBase::Attach(HWND hwnd)
{
	hWnd = hwnd;
	if(SetupAfterCreation())
	{
		return true;
	}
	else
	{	hWnd = NULL;
		return false;
	}
}

HWND os::WndBase::Detach()
{	ASSERT(hWnd);
	ASSERT(0);     // not implemeted !!!! 
	return NULL;
}


bool os::WndBase::SetupAfterCreation(bool attached)
{
	if(hWnd)
	{	
		if(!_bAsDialog)
		{
		#ifndef _WIN64
			::SetWindowLongPtr(hWnd,GWLP_USERDATA,(LONG)this);
		#else
			::SetWindowLongPtr(hWnd,GWLP_USERDATA,(LONG_PTR)this);
		#endif
			if(!attached)
				WndProc(WM_CREATE,0,0);
		}
		else
		{
		#ifndef _WIN64
			::SetWindowLongPtr(hWnd,DWLP_USER,(LONG)this);
		#else
			::SetWindowLongPtr(hWnd,DWLP_USER,(LONG_PTR)this);
		#endif
			if(!attached)
			{	WndProc(WM_CREATE,0,0);
				WndProc(WM_INITDIALOG,0,0);
			}
		}

		if(attached)
		{
			_WndOldProc = reinterpret_cast<WNDPROC>(GetWindowLongPtr(hWnd, GWLP_WNDPROC));
			SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(_details::_WndBase_Class::_CommonWndProc));
		}
		else
			if(!OnInitWnd())
			{	DestroyWindow();
				return false;
			}

		return true;
	}
	else
	{	
		return false;
	}
}


bool os::WndBase::Create(DWORD WinStyle,HWND Parent)
{
	ASSERT(hWnd==NULL);
	hWnd = CreateWindow(_details::_WndBase_Class::GetWindowClassName(os::_details::WCT_NORMAL)
						,_T(""), WinStyle,
						CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 
						Parent, NULL, ::GetModuleHandle(NULL), NULL);
	_bAsDialog = false;
	if( SetupAfterCreation() )
		return true;
	else
	{	hWnd = NULL;
		return false;
	}
}

bool os::WndBase::CreateControl(LPCTSTR ClassName, DWORD style, const RECT& rc, HWND parent)
{
	ASSERT(hWnd==NULL);
	hWnd = CreateWindow(ClassName, 0, style, 
						rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, 
						parent, NULL, ::GetModuleHandle(NULL), NULL);
	_bAsDialog = false;
	if( SetupAfterCreation(true) )
		return true;
	else
	{	hWnd = NULL;
		return false;
	}
}

bool os::WndBase::CreatePopupWindow(HWND Parent)
{
	ASSERT(hWnd==NULL);
	hWnd = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
                          _details::_WndBase_Class::GetWindowClassName(os::_details::WCT_NORMAL),
                          _T(""), WS_POPUP,
						  CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 
						  Parent, NULL, ::GetModuleHandle(NULL), NULL);
	_bAsDialog = false;
	if( SetupAfterCreation() )
		return true;
	else
	{	hWnd = NULL;
		return false;
	}
}

bool os::WndBase::CreateLayeredWindow(INT x, INT y, INT w, INT h, HWND Parent)
{
	ASSERT(hWnd==NULL);
	hWnd = CreateWindowEx(	WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE | WS_EX_LEFT,
							_details::_WndBase_Class::GetWindowClassName(os::_details::WCT_LAYERED),
							_T(""), 0,
							x,y,w,h,
							Parent, NULL, ::GetModuleHandle(NULL), NULL);
						
	_bAsDialog = false;
	if( SetupAfterCreation() )
		return true;
	else
	{	hWnd = NULL;
		return false;
	}
}


bool os::WndBase::CreateDialog(LPCTSTR lpDlgTemplate, HWND Parent)
{
	ASSERT(hWnd==NULL);

#ifdef _UNICODE
	hWnd = CreateDialogW(os::GetResourceModule(),lpDlgTemplate,Parent,_details::_WndBase_Class::_CommonDlgProc);
#else
	hWnd = CreateDialogA(os::GetResourceModule(),lpDlgTemplate,Parent,_details::_WndBase_Class::_CommonDlgProc);
#endif

	_bAsDialog = true;
	if(SetupAfterCreation())
		return true;
	else
	{	hWnd = NULL;
		return false;
	}
}


bool os::WndBase::CreateToolWnd(HWND Parent)
{	
	bool ret = Create(WS_CAPTION|WS_SYSMENU|WS_THICKFRAME,Parent);
	if(ret)
	{
		SetStyleEx(WS_EX_TOOLWINDOW | GetStyleEx());
		AlignTo(NULL,ALIGN_X);
	}
	return ret;
}

void os::WndBase::EnableResizing(bool is)
{ 
	if(is)
	{
		SetStyle(GetStyle()|WS_THICKFRAME);
		HMENU Menu= ::GetSystemMenu(*this,false);
		if(Menu)
		{
			::InsertMenu(Menu,4,MF_BYPOSITION|MF_STRING|MF_ENABLED,SC_MAXIMIZE,_T("Ma&ximize"));
		}
	}
	else
	{
		SetStyle((GetStyle()&(~WS_THICKFRAME)));
		HMENU Menu= ::GetSystemMenu(*this,false);
		if(Menu)
		{
			::ModifyMenu(Menu,SC_MAXIMIZE,MF_BYCOMMAND|MF_DISABLED,NULL,NULL);
		}
	}

	::RedrawWindow(*this,NULL,NULL,RDW_FRAME|RDW_INVALIDATE);
}

void os::WndBase::HostChildWindow(HWND hChild)
{
	if(Hosted_Child)
	{
		::ShowWindow(Hosted_Child,SW_HIDE);
		Hosted_Child = NULL;
	}

	Hosted_Child = hChild;

	if(Hosted_Child)
	{
		::SetParent(Hosted_Child,*this);
		RECT rc;
		::GetClientRect(*this,&rc);
		::SetWindowPos(Hosted_Child,NULL,rc.left,rc.top,(rc.right-rc.left),(rc.bottom-rc.top),SWP_NOZORDER);
		::ShowWindow(Hosted_Child,SW_SHOW);
	}
}

void os::WndBase::AlignTo(HWND Reference,DWORD AlignFlag)
{
	HWND MyParent = ::GetParent(*this);
	RECT rc,org_rc;
	::GetWindowRect( *this, &org_rc );

	if(Reference)
	{
		::GetWindowRect( Reference, &rc );
	}
	else
	{
		rc.left = 0;
		rc.top = 0;
		rc.right = ::GetSystemMetrics(SM_CXFULLSCREEN);
		rc.bottom = ::GetSystemMetrics(SM_CYFULLSCREEN);
	}

	ASSERT(rc.right >= rc.left);
	ASSERT(rc.bottom >= rc.top);
	ASSERT(org_rc.right >= org_rc.left);
	ASSERT(org_rc.bottom >= org_rc.top);

	POINT	p;
	int w,h;
	p.x = AlignFlag&ALIGN_X?rc.left:org_rc.left;
	p.y = AlignFlag&ALIGN_Y?rc.top:org_rc.top;
	w = AlignFlag&ALIGN_WIDTH?(rc.right - rc.left):(org_rc.right - org_rc.left);
	h = AlignFlag&ALIGN_HEIGHT?(rc.bottom - rc.top):(org_rc.bottom - org_rc.top);

	if( MyParent )::ScreenToClient(MyParent,&p);
	
	::SetWindowPos(*this,NULL,p.x,p.y,w,h,SWP_NOZORDER);
}

os::WndBase::~WndBase(void)
{
	DestroyWindow();
	hWnd = NULL;
#ifdef __AFXWIN_H__
	ASSERT(_pMfcWnd);
	_pMfcWnd->Detach();
	_SafeDel(_pMfcWnd);
#endif
}

void os::WndBase::DestroyWindow(void)
{
	if(hWnd)
	{	ReleaseDC();
		HWND h = hWnd;
		hWnd = NULL;
		if(::DestroyWindow(h)){}
		else
		{
#pragma warning(disable: 4244 )
			::SetWindowLongPtr(hWnd,GWLP_WNDPROC,(LONG_PTR)DefWindowProc);  // just in case window is not completely destroyed, maybe in multithreading env. 
#pragma warning(default: 4244 )
		}
	}
}


void os::WndBase::ResizeWindowByClientArea(int Width,int Height)
{	RECT rc_win,rc_client;
	::GetWindowRect(*this,&rc_win);
	::GetClientRect(*this,&rc_client);

	::SetWindowPos(*this,NULL,0,0,
					Width +( rc_win.right - rc_win.left - rc_client.right),
					Height+( rc_win.bottom - rc_win.top - rc_client.bottom),
					SWP_NOZORDER|SWP_NOMOVE|SWP_NOACTIVATE);
	::PostMessage(*this,WM_EXITSIZEMOVE,0,0);
}

void os::WndBase::SwitchFullScreen()
{
	static const DWORD style_to_remove = WS_CHILD|WS_CAPTION|WS_BORDER|WS_THICKFRAME|WS_MAXIMIZEBOX|WS_MINIMIZEBOX|WS_DLGFRAME|WS_OVERLAPPED|WS_SYSMENU;
	static const DWORD styleEx_to_remove = WS_EX_WINDOWEDGE|WS_EX_APPWINDOW|WS_EX_CLIENTEDGE|WS_EX_DLGMODALFRAME|WS_EX_MDICHILD|WS_EX_STATICEDGE|WS_EX_TOOLWINDOW;

	if( !IsInFullScreen() )
	{	// go full srceen
		_IsInFallScreen = true;
		_OrgParentWnd = ::GetParent(*this);
		::GetWindowRect(*this,&_OrgLayout);
		::SetParent(*this,NULL);

		_OrgStyle = GetStyle();
		_OrgStyleEx = GetStyleEx();

		SetStyle( (_OrgStyle& (~style_to_remove)) | WS_POPUP );
		SetStyleEx( _OrgStyleEx& (~styleEx_to_remove) );

		if(_OrgParentWnd)
		{	POINT p = {0,0};
			::ScreenToClient(_OrgParentWnd,&p);
			_OrgLayout.left += p.x;
			_OrgLayout.right += p.x;
			_OrgLayout.bottom += p.y;
			_OrgLayout.top += p.y;
		}

		// just in case that we are using multiple monitor in DualView mode
		int scrw = GetSystemMetrics(SM_CXSCREEN);
		int scrh = GetSystemMetrics(SM_CYSCREEN);
		int x,y;
		{	POINT p = {_OrgLayout.left,_OrgLayout.top};
			::ClientToScreen(_OrgParentWnd,&p);
			
			x = ((int)(p.x/(float)scrw + 0.5f))*scrw;
			y = ((int)(p.y/(float)scrh + 0.5f))*scrh;
			if(p.x<0)x-=scrw;
			if(p.y<0)y-=scrh;

			int vw = GetSystemMetrics(SM_CXVIRTUALSCREEN);
			int vh = GetSystemMetrics(SM_CYVIRTUALSCREEN);
			int vx = GetSystemMetrics(SM_XVIRTUALSCREEN);
			int vy = GetSystemMetrics(SM_YVIRTUALSCREEN);

			x = rt::min(rt::max(x,vx),vw+vx-scrw);
			y = rt::min(rt::max(y,vy),vh+vy-scrh);
		}
		
		::SetWindowPos(*this,HWND_TOPMOST,x,y,scrw,scrh,SWP_NOZORDER);
		::PostMessage(*this,WM_EXITSIZEMOVE,0,0);
	}
	else
	{	_IsInFallScreen = false;
		::SetParent(*this,_OrgParentWnd);

		SetStyle(_OrgStyle);
		SetStyleEx(_OrgStyleEx);

		::SetWindowPos(*this,HWND_NOTOPMOST,_OrgLayout.left,_OrgLayout.top,
											_OrgLayout.right-_OrgLayout.left,
											_OrgLayout.bottom-_OrgLayout.top,SWP_NOZORDER);
		::PostMessage(*this,WM_EXITSIZEMOVE,0,0);
		HWND ancestor = ::GetAncestor(*this,GA_ROOT);
		if(ancestor!=hWnd)::BringWindowToTop(ancestor);
	}
}

void os::WndBase::MainMenu_SetCheck(UINT Id,bool Check)
{
	HMENU m = GetMenu(*this);
	if(m)CheckMenuItem(m,Id,Check?MF_CHECKED:MF_UNCHECKED);
}

void os::WndBase::MainMenu_Enable(UINT Id,bool Enable)
{
	HMENU m = GetMenu(*this);
	if(m)EnableMenuItem(m,Id,Enable?MF_ENABLED:MF_GRAYED);
}

void os::WndBase::MainMenu_SetText(UINT Id,LPCTSTR text)
{	ASSERT(text);
	HMENU m = GetMenu(*this);
	if(m)ModifyMenu(m,Id,MF_BYCOMMAND|MF_STRING,Id,text);
}

int os::WndBase::MessageBox(LPCTSTR lpText,LPCTSTR lpCaption,UINT uType)
{
	return ::MessageBox((*this),lpText,lpCaption,uType);
}

LRESULT os::WndBase::WndProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) 
	{
	case WM_SIZE:
		if(Hosted_Child)
		{
			int width = rt::max(1,(int)LOWORD(lParam));
			int height = rt::max(1,(int)HIWORD(lParam));
			::SetWindowPos(Hosted_Child,NULL,0,0,width,height,SWP_NOZORDER|SWP_NOMOVE);
			return 0;
		}
        break;
	//case WM_CLOSE:
	//	if(GetStyleEx()&WS_EX_TOOLWINDOW)
	//	{
	//		//::ShowWindow(*this,SW_HIDE);
	//		return 0;
	//	}
	//	break;
	case WM_LBUTTONDOWN:
		SetFoucs();
		if(_bMoveWindowByClientRegion)
			::SendMessage(hWnd,WM_NCLBUTTONDOWN,HTCAPTION,lParam);
		break;
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_ACTIVATE:
		SetFoucs();
		break;
	case WM_NCDESTROY:
		#ifdef __AFXWIN_H__
			ASSERT(_pMfcWnd);
			_pMfcWnd->Detach();
		#endif
		hWnd = NULL;
		if(this == _pMainWnd)
		{
			PostQuitMessage(0);
		}
		return 0;
		break;
	}

	if(_WndOldProc)
		return _WndOldProc(hWnd, message, wParam, lParam);

	if(!_bAsDialog)
		return DefWindowProc(hWnd, message, wParam, lParam);
	else
		return 0;
}

void os::WndBase::CenterWindow()
{
	RECT rc;
	::GetWindowRect(hWnd,&rc);

	RECT rcw;
	::SystemParametersInfo(SPI_GETWORKAREA,0,&rcw,0);

	::SetWindowPos( *this,NULL,
					(rcw.left+((rcw.right - rcw.left) - (rc.right - rc.left))/2),
					(rcw.top+((rcw.bottom - rcw.top) - (rc.bottom - rc.top))/2),
					0,0,SWP_NOZORDER|SWP_NOSIZE|SWP_NOACTIVATE);
}

void os::WndBase::SetMenu(LPCTSTR ResId)
{	HMENU	hMenu = LoadMenu(_hModuleHandle,ResId);
	ASSERT(hMenu);
	SetMenu(hMenu);
}

void os::WndBase::SetIcon(LPCTSTR ResId, bool bBigIcon)
{	HICON	hIcon = LoadIcon(_hModuleHandle,ResId);
	ASSERT(hIcon);
	SetIcon(hIcon, bBigIcon);
}

void os::SetResourceModule(HANDLE DLL)
{
	os::WndBase::_hModuleHandle = (HMODULE)DLL;
}

HINSTANCE os::GetResourceModule()
{
	return (HINSTANCE)os::WndBase::_hModuleHandle;
}

void os::PumpMessages()
{
	MSG Msg;

	while(GetMessage(&Msg, NULL, 0, 0) > 0)
	{
	    TranslateMessage(&Msg);
	    DispatchMessage(&Msg);
	}
}

/*
bool os::WaitForEnteringCriticalSection_UI(os::CCriticalSection& ccs, UINT timeout)
{
	double deadline = timeout==INFINITE?DBL_MAX:timeout;
	os::CTimeMeasure tm;
	tm.Start();
	while(tm.Snapshot()<deadline)
	{
#ifdef __AFXWIN_H__
		_AFX_THREAD_STATE *pState = AfxGetThreadState();

		if(::PeekMessage(&(pState->m_msgCur), NULL, NULL, NULL,true))
		{	if(pState->m_msgCur.message == WM_QUIT)
			{	::PostThreadMessage(GetCurrentThreadId(),WM_QUIT,0,0);
				return false;
		}	}
		else
		{	if(ccs.TryLock())return true;
			Sleep(100);
			continue;
		}

		if (pState->m_msgCur.message != WM_KICKIDLE && !AfxPreTranslateMessage(&(pState->m_msgCur)))
		{
			::TranslateMessage(&(pState->m_msgCur));
			::DispatchMessage(&(pState->m_msgCur));
		}
#else
		MSG	msgCur;
		if(::PeekMessage(&msgCur, NULL, NULL, NULL,true))
		{	if(msgCur.message == WM_QUIT)
			{	::PostThreadMessage(GetCurrentThreadId(),WM_QUIT,0,0);
				return false;
		}	}
		else
		{	if(ccs.TryLock())return true;
			Sleep(100);
			continue;
		}

		::TranslateMessage(&msgCur);
		::DispatchMessage(&msgCur);
#endif
	}

	return false;
}


bool os::WaitForThreadEnding_UI(HANDLE hThread, UINT time_wait_ms)
{
	double deadline = (time_wait_ms==INFINITE)?DBL_MAX:time_wait_ms;
	os::CTimeMeasure tm;
	tm.Start();
	while(tm.Snapshot()<deadline)
	{
#ifdef __AFXWIN_H__
		_AFX_THREAD_STATE *pState = AfxGetThreadState();

		if(::PeekMessage(&(pState->m_msgCur), NULL, NULL, NULL,true))
		{	if(pState->m_msgCur.message == WM_QUIT)
			{	::PostThreadMessage(GetCurrentThreadId(),WM_QUIT,0,0);
				return false;
		}	}
		else
		{	DWORD exitcode;
			if(!::GetExitCodeThread(hThread,&exitcode) || exitcode!=STILL_ACTIVE)
				return true;
			Sleep(100);
			continue;
		}

		if (pState->m_msgCur.message != WM_KICKIDLE && !AfxPreTranslateMessage(&(pState->m_msgCur)))
		{
			::TranslateMessage(&(pState->m_msgCur));
			::DispatchMessage(&(pState->m_msgCur));
		}
#else
		MSG	msgCur;
		if(::PeekMessage(&msgCur, NULL, NULL, NULL,true))
		{	if(msgCur.message == WM_QUIT)
			{	::PostThreadMessage(GetCurrentThreadId(),WM_QUIT,0,0);
				return false;
		}	}
		else
		{	DWORD exitcode;
			if(::GetExitCodeThread(hThread,&exitcode) && exitcode!=STILL_ACTIVE)
				return true;
			Sleep(100);
			continue;
		}

		::TranslateMessage(&msgCur);
		::DispatchMessage(&msgCur);
#endif
	}

	return false;
}


bool os::WaitForProcessEnding_UI(HANDLE Process, UINT time_wait_ms)
{
	double deadline = (time_wait_ms==INFINITE)?DBL_MAX:time_wait_ms;
	os::CTimeMeasure tm;
	tm.Start();
	while(tm.Snapshot()<deadline)
	{
#ifdef __AFXWIN_H__
		_AFX_THREAD_STATE *pState = AfxGetThreadState();

		if(::PeekMessage(&(pState->m_msgCur), NULL, NULL, NULL,true))
		{	if(pState->m_msgCur.message == WM_QUIT)
			{	::PostThreadMessage(GetCurrentThreadId(),WM_QUIT,0,0);
				return false;
		}	}
		else
		{	DWORD exitcode;
			if(!::GetExitCodeProcess(Process,&exitcode) || exitcode!=STILL_ACTIVE)
				return true;
			Sleep(100);
			continue;
		}

		if (pState->m_msgCur.message != WM_KICKIDLE && !AfxPreTranslateMessage(&(pState->m_msgCur)))
		{
			::TranslateMessage(&(pState->m_msgCur));
			::DispatchMessage(&(pState->m_msgCur));
		}
#else
		MSG	msgCur;
		if(::PeekMessage(&msgCur, NULL, NULL, NULL,true))
		{	if(msgCur.message == WM_QUIT)
			{	::PostThreadMessage(GetCurrentThreadId(),WM_QUIT,0,0);
				return false;
		}	}
		else
		{	DWORD exitcode;
			if(::GetExitCodeProcess(Process,&exitcode) && exitcode!=STILL_ACTIVE)
				return true;
			Sleep(100);
			continue;
		}

		::TranslateMessage(&msgCur);
		::DispatchMessage(&msgCur);
#endif
	}

	return false;
}
*/



bool os::OpenDefaultBrowser(LPCTSTR url, DWORD show_window)
{
	struct _func
	{	LPCTSTR url;
		DWORD	show_window;
		static DWORD WINAPI _call(LPVOID p)
		{	
			::ShellExecute(NULL,_T("open"),((_func*)p)->url,NULL,NULL,((_func*)p)->show_window);
			delete (_func*)p;
			return 0;
		};
	};

	_func& c = *new _func;
	c.url = url;
	c.show_window = show_window;

	::CloseHandle(::CreateThread(NULL,0,_func::_call,&c,0,NULL));
	return true;
}


bool os::SwitchToWindow(HWND hWnd, DWORD timeout)
{
	if(!::IsWindow(hWnd)) return false;
 
	BYTE keyState[256] = {0};
	//to unlock SetForegroundWindow we need to imitate Alt pressing
	if(::GetKeyboardState((LPBYTE)&keyState))
	{
		if(!(keyState[VK_MENU] & 0x80))
		{
			::keybd_event(VK_MENU, 0, KEYEVENTF_EXTENDEDKEY | 0, 0);
		}
	}
 
	::SetForegroundWindow(hWnd);
 
	if(::GetKeyboardState((LPBYTE)&keyState))
	{
		if(!(keyState[VK_MENU] & 0x80))
		{
			::keybd_event(VK_MENU, 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
		}
	}

	if(timeout == INFINITE)
	{	::ShowWindow(hWnd, SW_SHOWNORMAL);
	}
	else
	{
		if(SendMessageTimeout(hWnd, WM_SYSCOMMAND, SC_RESTORE, 0, 0, timeout, NULL) == 0)
			return false;
	}

	::SetWindowPos(hWnd, HWND_TOP, 0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
	return true;
}


bool os::LoadResource(rt::String& out, LPCTSTR res_name_id, LPCTSTR res_type, HINSTANCE module)
{
	HRSRC myResource = NULL;
    UINT myResourceSize = 0;
    HGLOBAL myResourceData = NULL;
	void* pMyBinaryData = NULL;

	if(	(myResource = ::FindResource(module, res_name_id, res_type)) &&
		(myResourceSize = ::SizeofResource(module, myResource)) &&
		(myResourceData = ::LoadResource(module, myResource)) &&
		out.SetLength(myResourceSize)
	)
	{	memcpy(out.Begin(), ::LockResource(myResourceData), myResourceSize);
		return true;
	}

	return false;
}