#include "kernel.h"
#include "multi_thread.h"
#include "file_dir.h"

namespace rt
{
namespace _details
{
LPCSTR _UnitTestOutputFilePrefix = nullptr;
}}


#if defined(PLATFORM_WIN)
#include <conio.h>
#endif

namespace os
{
namespace _details
{

#ifndef PLATFORM_DISABLE_LOG

static CriticalSection	_LogWriteCS;
static File				_LogFile;
static Timestamp		_LogTime;
static rt::String		_LogPrompt;

#endif

#if defined(PLATFORM_WIN) && !defined(PLATFORM_DISABLE_LOG)
void _CreateConsole()
{
#pragma warning(disable:4311)
	if(::GetConsoleWindow() == NULL)
	{
		// allocate a console for this app
		AllocConsole();
	}

	// set the screen buffer to be big enough to let us scroll text
    CONSOLE_SCREEN_BUFFER_INFO coninfo;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);
	coninfo.dwSize.Y = 9999;
	// How many lines do you want to have in the console buffer
	SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);
    // redirect unbuffered STDOUT to the console
    freopen("CONOUT$", "w", stdout);
	setvbuf(stdout, NULL, _IONBF, 0);
	// redirect unbuffered STDIN to the console
    freopen("CONIN$", "r", stdin);
	setvbuf(stdin, NULL, _IONBF, 0);
	// redirect unbuffered STDERR to the console
    freopen("CONOUT$", "w", stderr);
    setvbuf(stderr, NULL, _IONBF, 0);

	// make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog point to console as well
	std::ios::sync_with_stdio();
#pragma warning(default:4311)
}
#endif

os::Thread				_LogInputPump;
ConsoleInputHandler*	_pLogInputHandler = NULL;
rt::String				_ConsoleRepeatCommand;
UINT					_ConsoleRepeatCommandInterval;
LONGLONG				_ConsoleRepeatCommandLastTick;
DWORD					_LogInputPumpRoute(LPVOID p)
{
	static const int API_CMDLINE_BUFSIZE = 2048;
	os::CommandLine cmd;

#ifdef PLATFORM_WIN
	ASSERT(::GetConsoleWindow());
	// If thread start without console, _kbhit() will return 0 forever
	// Console will be created when EnableConsoleInput is called, or the first _LOG is printed
#endif // other platforms don't use _kbhit

	while(!_LogInputPump.WantExit())
	{
#ifdef PLATFORM_WIN
		char cmdbuf[API_CMDLINE_BUFSIZE*3/2];
#else
		char cmdbuf[API_CMDLINE_BUFSIZE];
#endif
		for(int i=0; !_LogInputPump.WantExit(); i++)
		{
#if defined(PLATFORM_WIN)
			if(_kbhit())break;
#else
			if(!feof(stdin))break;
#endif

			os::Sleep(20);
			if((i&15) == 0 && _ConsoleRepeatCommandInterval && os::Timestamp::Get() - _ConsoleRepeatCommandLastTick > _ConsoleRepeatCommandInterval)
			{
				_ConsoleRepeatCommandLastTick = os::Timestamp::Get();
				cmd.Parse(_ConsoleRepeatCommand);
				if(_pLogInputHandler)
					if(_pLogInputHandler->OnCommand(cmd))
						continue;

				_ConsoleRepeatCommandInterval = 0;
				_ConsoleRepeatCommand.SecureEmpty();
			}
		}

		cmdbuf[0] = 0;
		fgets(cmdbuf, API_CMDLINE_BUFSIZE, stdin);

#ifdef PLATFORM_WIN
		// take care CJK 
		int mb_len = (int)rt::String_Ref(cmdbuf).TrimRightSpace().GetLength();
		LPWSTR utf16 = (LPWSTR)alloca((1 + mb_len*2)*sizeof(WCHAR));
		int utf16_len = MultiByteToWideChar(CP_THREAD_ACP, 0, cmdbuf, mb_len, utf16, 1 + mb_len*2);
		mb_len = (int)os::UTF8Encode(utf16, utf16_len, cmdbuf);
		ASSERT(mb_len < API_CMDLINE_BUFSIZE*3/2);
		cmdbuf[mb_len] = 0;
#endif

		cmd.Parse(cmdbuf);

		if(cmd.GetTextCount() || cmd.GetOptionCount())
		{
			if(_pLogInputHandler && _pLogInputHandler->OnCommand(cmd))
			{
				int repeat = -1;
				if(cmd.HasOption("repeat"))repeat = cmd.GetOptionAs<int>("repeat", 2);
				if(cmd.HasOption("r"))repeat = cmd.GetOptionAs<int>("r", 2);

				if(repeat > 0)
				{
					_ConsoleRepeatCommand = cmdbuf;
					_ConsoleRepeatCommandInterval = repeat*1000;
					_ConsoleRepeatCommandLastTick = os::Timestamp::Get();
				}
				else
				{	_ConsoleRepeatCommand.SecureEmpty();
					_ConsoleRepeatCommandInterval = 0;
				}
			}
		}
		else
		{	
			_LOGC_PROMPT();
			// stop command repeating
			if(!_ConsoleRepeatCommand.IsEmpty())
			{
				_ConsoleRepeatCommand.Empty();
				_ConsoleRepeatCommandInterval = 0;
			}
		}

		cmd.SecureClear();
	}

	return 0;
}
} // namespace _details


void EnableConsoleInput(ConsoleInputHandler* input_handler, LPCSTR prompt)
{
#if defined(PLATFORM_WIN) && !defined(PLATFORM_DISABLE_LOG)
	_details::_CreateConsole();
#endif

	if(input_handler)
	{
		os::SetLogConsolePrompt(prompt);

		if(!_details::_LogInputPump.IsRunning())
			VERIFY(_details::_LogInputPump.Create(_details::_LogInputPumpRoute));
	}
	else
	{	
		os::SetLogConsolePrompt(NULL);

		_details::_LogInputPump.WantExit() = true;
		_details::_LogInputPump.WaitForEnding(1000, true);
	}

	_details::_pLogInputHandler = input_handler;
}

void SetLogConsolePrompt(LPCSTR prompt)
{
#ifndef PLATFORM_DISABLE_LOG
	_details::_LogPrompt = rt::String_Ref(prompt).SubStr(0, 64);
#endif
}

bool SetLogFile(LPCSTR filename_in, bool append)
{
#ifndef PLATFORM_DISABLE_LOG
	rt::String filename = filename_in;

	os::Timestamp::Fields f = os::Timestamp::Get().GetLocalDateTime();

	filename.Replace("%YEAR%", rt::tos::Number(f.Year));
	filename.Replace("%MONTH%", rt::tos::Number(f.Month).RightAlign(2,'0'));
	filename.Replace("%DAY%", rt::tos::Number(f.Day).RightAlign(2,'0'));

	EnterCSBlock(_details::_LogWriteCS);
	if(_details::_LogFile.IsOpen())_details::_LogFile.Close();
	if(_details::_LogFile.Open(filename, append?os::File::Normal_Append:os::File::Normal_Write, true))
	{
		if(!append || _details::_LogFile.GetFileSize() == 0)
			if(3 != _details::_LogFile.Write("\xef\xbb\xbf", 3))
			{	_details::_LogFile.Close();
				return false;
			}

		return true;
	}
#endif

	return false;
}

rt::String_Ref GetLogFilename()
{
#ifndef PLATFORM_DISABLE_LOG
	return _details::_LogFile.GetFilename();
#else
	return nullptr;
#endif
}

void LogWriteFlush()
{
#ifndef PLATFORM_DISABLE_LOG
	if(_details::_LogFile.IsOpen())
	{
		EnterCSBlock(_details::_LogWriteCS);
		_details::_LogFile.Flush();
		_details::_LogTime.LoadCurrentTime();
	}
#endif
}

LogPrefix& LogPrefix::operator << (LogPrefixItemCode code)
{
	_item& it = items.push_back();
    it.code = code;
    return *this;
}

LogPrefix& LogPrefix::operator << (char c)
{
	if (items.GetSize() && !items.last().code) // if last item is a string, append the string
    {   items.last().string += c;
    }
    else
    {   _item& it = items.push_back();
		auto x = rt::String_Ref(&c, 1);
        it.string = ALLOCA_C_STRING(x);
    }
    return *this;
}

LogPrefix& LogPrefix::operator << (LPCSTR s)
{
	if (items.GetSize() && !items.last().code) // if last item is a string, append the string
    {   items.last().string += s;
    }
    else
    {   _item& it = items.push_back();
        it.string = s;
    }
    return *this;
}

const LogPrefix& LogPrefix::operator = (const LogPrefix& x)
{   items.SetSize(x.items.GetSize());
    for (int i = 0; i < (int)x.items.GetSize(); i++)
    {   
        if (x.items[i].code)
        {   items[i].code = x.items[i].code;
        }
        else
        {   items[i].string = x.items[i].string;
        }
    }
    return *this;
}

} // namespace os



#if defined(PLATFORM_WIN)
//////////////////////////////////////////////////////////
// All Windows implementations
#include <windows.h>
#include <Rpc.h>
#include <iostream>
#include <io.h>
#pragma comment(lib,"Rpcrt4.lib")


namespace os
{
namespace _details
{
	void __ConsoleLogWriteDefault(LPCSTR log, int type, LPVOID)
	{	
#ifndef PLATFORM_DISABLE_LOG
		_details::_CreateConsole();

		int color[] = { 8, 8, 7, 10, 14, 12 };
		SetConsoleTextAttribute(GetStdHandle( STD_OUTPUT_HANDLE ), color[type&rt::LOGTYPE_LEVEL_MASK]);
		int len = (int)strlen(log);
		
		os::LPU16CHAR utf16;
		rt::Buffer<os::U16CHAR> temp;
		if(len < 4*1024)
			utf16 = (os::LPU16CHAR)alloca(2*len);
		else
		{
			temp.ChangeSize(len);
			utf16 = temp.Begin();
		}

		int len_utf16 = (int)os::UTF8Decode(log, len, utf16);

		char* mb;
		rt::String	temp_str;
		if(3*len_utf16 < 4*1024)
			mb = (char*)alloca(3*len_utf16 + 1);
		else
		{
			temp_str.SetLength(3*len_utf16);
			mb = temp_str;
		}

		int mb_len = WideCharToMultiByte(CP_THREAD_ACP, 0, utf16, len_utf16, mb, 3*len_utf16, NULL, NULL);
		mb[mb_len] = 0;

		thread_local bool _last_updating = false;
		static const char clear_len[] = "\r                                                                               \r";

		if(_last_updating)
			fputs(clear_len, stdout);

		if((type&rt::LOGTYPE_LEVEL_MASK) != rt::LOGTYPE_UPDATING)
		{
			if(!_details::_LogPrompt.IsEmpty())
			{	putchar('\r');
				fputs(&clear_len[sizeof(clear_len) - 3 - _details::_LogPrompt.GetLength()], stdout);
			}

			if((type&rt::LOGTYPE_IN_CONSOLE_PROMPT) == 0)
				puts(mb);

			SetConsoleTextAttribute(GetStdHandle( STD_OUTPUT_HANDLE ), color[2]);

			if(!_details::_LogPrompt.IsEmpty())
				fputs(_details::_LogPrompt, stdout);

			_last_updating = false;
		}
		else
		{
			fputs(mb, stdout);
			_last_updating = true;
			fflush(stdout);
			SetConsoleTextAttribute(GetStdHandle( STD_OUTPUT_HANDLE ), color[2]);
		}
#endif // #ifndef PLATFORM_DISABLE_LOG
	}
}

void SetLogConsoleTitle(LPCSTR title)
{
	HWND wnd = ::GetConsoleWindow();
	if(wnd)
		::SetWindowTextW(wnd, os::__UTF16(title));
}

} //  namespace os::_details

#elif defined(PLATFORM_ANDROID)
#include <android/log.h>

namespace os
{
namespace _details
{
	void __ConsoleLogWriteDefault(LPCSTR log, int type, LPVOID)
	{	
		ASSERT(type >=0 && type<rt::LOGTYPE_MAX);
		int cat[] = { ANDROID_LOG_VERBOSE, ANDROID_LOG_DEBUG, ANDROID_LOG_INFO, ANDROID_LOG_WARN, ANDROID_LOG_ERROR };
		__android_log_write(cat[type], "CPF", log);
	}
}} //  namespace os::_details

#elif defined (PLATFORM_IOS)

namespace os
{
namespace _details
{
	void __ConsoleLogWriteDefault(LPCSTR log, int type, LPVOID)
	{	
        static os::CriticalSection _LogCS;
        static rt::String _prev;
        
        rt::String_Ref logstr(log);
        EnterCSBlock(_LogCS);
        if(_prev == logstr)return;
        puts(log);
        _prev = logstr;
	}
}} //  namespace os::_details

#elif defined(PLATFORM_LINUX) || defined (PLATFORM_MAC)
#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>

namespace os
{
namespace _details
{
	void __ConsoleLogWriteTerminal(LPCSTR mb, int type, LPVOID)
	{	
		setlocale(LC_ALL, "en_US.UTF-8");
		
		LPCSTR color[] = { "\033[0;37m", "\033[0;37m", "\033[0m", "\033[1;32m", "\033[1;33m", "\033[1;31m" };
        
		thread_local bool _last_is_updating = false;
		
		if(_last_is_updating)
			fputs("\033[2K\r", stdout);
		
		fputs(color[type&rt::LOGTYPE_LEVEL_MASK], stdout);
		if((type&rt::LOGTYPE_LEVEL_MASK) != rt::LOGTYPE_UPDATING)
		{
#ifndef PLATFORM_DISABLE_LOG
			if(!_details::_LogPrompt.IsEmpty())
                fputs("\r                                                                \r", stdout);
#endif 

			if((type&rt::LOGTYPE_IN_CONSOLE_PROMPT) == 0)
				puts(mb);
			
			fputs("\033[0m", stdout);
			
#ifndef PLATFORM_DISABLE_LOG
			if(!_details::_LogPrompt.IsEmpty())
				fputs(_details::_LogPrompt, stdout);
#endif 
				
			fflush(stdout);
			_last_is_updating = false;
		}
		else
		{	
			fputs(mb, stdout);
			_last_is_updating = true;
			fputs("\033[0m", stdout);
			fflush(stdout);
		}
	}
    
    void __ConsoleLogWriteDebugger(LPCSTR log, int type, LPVOID)
    {
        static os::CriticalSection _LogCS;
        static rt::String _prev;
        
        rt::String_Ref logstr(log);
        if(!logstr.IsEmpty() && logstr.Last() == '\r')
        {   EnterCSBlock(_LogCS);
            if(_prev == logstr)return;
            _prev = logstr;
        }
        
        puts(log);
    }
    
    void __ConsoleLogWriteDefault(LPCSTR mb, int type, LPVOID)
    {
        struct winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        if(w.ws_row && w.ws_col)  // detect terminal/debugger
        {
            SetConsoleLogWriteFunction(__ConsoleLogWriteTerminal, NULL);
            __ConsoleLogWriteTerminal(mb, type, NULL);
        }
        else
        {
            SetConsoleLogWriteFunction(__ConsoleLogWriteDebugger, NULL);
            __ConsoleLogWriteDebugger(mb, type, NULL);
        }
    }
}} //  namespace os::_details

#else
#endif


namespace os
{
namespace _details
{

/////////////////////////////////////////////////////////////////
// Platform independent implementations but depend on some platform dependent functions
void LogWriteDefault(LPCSTR log, LPCSTR file, int line_num, LPCSTR func, int type, LPVOID)
{
#ifndef PLATFORM_DISABLE_LOG
	EnterCSBlock(_LogWriteCS);

	if(type&(rt::LOGTYPE_IN_CONSOLE|rt::LOGTYPE_IN_CONSOLE_FORCE|rt::LOGTYPE_IN_CONSOLE_PROMPT))os::_details::__ConsoleLogWrite(log, type);
	if((type&rt::LOGTYPE_IN_LOGFILE) && _LogFile.IsOpen())
	{
		_LogFile.Write(rt::String_Ref(log));
		_LogFile.Write("\x0d\x0a", 2);
		_LogFile.Flush();
	}

#endif
}

static FUNC_LOG_WRITE	__LogWrtieFunc = LogWriteDefault;
static LPVOID			__LogWrtieFuncCookie = nullptr;
static bool				__LogWrtieNoConsoleDisplay = false;

void SetLogWriteFunction(FUNC_LOG_WRITE func, LPVOID cookie)
{
	if(func)
	{	__LogWrtieFunc = func;
		__LogWrtieFuncCookie = cookie;
	}
	else
	{	__LogWrtieFunc = LogWriteDefault;
		__LogWrtieFuncCookie = nullptr;
	}
}

static LPVOID					__ConsoleLogWriteCookie = nullptr;
static FUNC_CONSOLE_LOG_WRITE	__ConsoleLogWriteFunc = __ConsoleLogWriteDefault;

void __ConsoleLogWrite(LPCSTR log, int type){ __ConsoleLogWriteFunc(log, type, __ConsoleLogWriteCookie); }

void SetConsoleLogWriteFunction(FUNC_CONSOLE_LOG_WRITE func, LPVOID cookie)
{
	if(func)
	{	__ConsoleLogWriteFunc = func;
		__ConsoleLogWriteCookie = cookie;
	}
	else
	{	__ConsoleLogWriteCookie = nullptr;
		__ConsoleLogWriteFunc = __ConsoleLogWriteDefault;
	}
}

} // namespace _details

void LogDisplayInConsole(bool yes)
{
	_details::__LogWrtieNoConsoleDisplay = !yes;
}

bool LogIsDisplayInConsole()
{
	return !_details::__LogWrtieNoConsoleDisplay;
}

void LogWrite(LPCSTR log, LPCSTR file, int line_num, LPCSTR func, int type)
{
	if(_details::__LogWrtieNoConsoleDisplay)type = type&(~rt::LOGTYPE_IN_CONSOLE);
	_details::__LogWrtieFunc(log, file, line_num, func, type, _details::__LogWrtieFuncCookie);
}


} // namespace os
