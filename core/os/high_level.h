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

#include "predefines.h"
#include "../rt/string_type.h"
#include "multi_thread.h"
#include "file_dir.h"

namespace os
{

class Daemon	// singleton
{
#if defined(PLATFORM_WIN)
	volatile LPVOID			_hServiceHandler;
	os::Thread				_DispatchThread;
#else
#endif

protected:
	static Daemon*			_pDaemon;
	rt::String				_DaemonName;
public:
	Daemon();
	~Daemon();
	bool	InitializeDaemonController(LPCSTR name);	// NOT thread-safe
	void	ReportDaemonStatus(DWORD state = DAEMON_PAUSED);	// SERVICE_PAUSED
	virtual void OnDaemonControl(DWORD dwControl){};			// dwControl = SERVICE_CONTROL_STOP/SERVICE_CONTROL_SHUTDOWN/ ...
};

#if defined(PLATFORM_WIN)

class LaunchProcess
{
	HANDLE hChildStdinRd, hChildStdinWrDup, 
		   hChildStdoutWr,hChildStdoutRdDup;

public:
	typedef void	(*FUNC_HOOKEDOUTPUT)(char* p, UINT len, LPVOID cookie);

protected:
	FUNC_HOOKEDOUTPUT	_Callback;
	LPVOID				_Callback_Cookie;
	DWORD				_Flag;
	os::Thread			_OutputHookThread;
	void				_HookedOutput(char* p, UINT len);

protected:
	os::CriticalSection	_CCS;
	HANDLE				_hProcess;
	rt::String			_Output;
	int					_ExitCode;
	os::Timestamp		_ExecutionTime;		// in msec
	os::Timestamp		_ExitTime;
	void				ClearAll();
	static DWORD		OutputHookThread(LPVOID);

public:
	static void _RemoveCarriageReturn(rt::String& output, const rt::String_Ref& add);
	enum
	{	FLAG_ROUTE_OUTPUT	= 0x1,
		FLAG_SAVE_OUTPUT	= 0x2,	// retrieve by GetOutput/GetOutputLen
	};
	LaunchProcess();
	~LaunchProcess();
	void	SetOutputCallback(FUNC_HOOKEDOUTPUT func, LPVOID cookie);
	bool	Launch(LPCSTR cmdline, DWORD flag = FLAG_ROUTE_OUTPUT, DWORD window_show = SW_HIDE, LPCSTR pWorkDirectory = nullptr, LPCSTR pEnvVariableAddon = nullptr);  // pEnvVariableAddon is \0 seperated multiple strings (UTF8), ends with \0\0
	bool	WaitForEnding(DWORD timeout = INFINITE); // return false when timeout
	void	Terminate();
	bool	IsRunning();
	LPCSTR	GetOutput();
	UINT	GetOutputLen();
	void	CopyOutput(rt::String& out);
	bool	SendInput(const rt::String_Ref& str){ return str.IsEmpty()?true:SendInput(str.Begin(), (UINT)str.GetLength()); }
	bool	SendInput(LPCSTR p, UINT len);
	const os::Timestamp&	GetExecutionTime() const { return _ExecutionTime; }		// available after IsRunning() returns false!	
	const os::Timestamp&	GetExitTime() const { return _ExitTime; }		// available after IsRunning() returns false!	
	int		GetExitCode() const { return _ExitCode; }					// available after IsRunning() returns false!
	bool	SendToStdin(LPCVOID str, UINT len);
};

#endif

class ParallelFileWriter
{
protected:
	rt::Buffer<BYTE>	_WritePool;
	os::File			_File;
	double				_AvgAttenuation;

public:
	typedef bool (*FUNC_FileWriter)(LPCVOID data, UINT len, os::File& file, LPVOID cookie);
	
protected:
	FUNC_FileWriter		_pFileWriterCB;
	LPVOID				_FileWriterCookie;
	bool				_FilenameByLocalTime;

	LPCSTR				_FileOpenFlag;
	rt::String			_FilenameTemplate;
	bool				_FilenameChanged;
	os::CriticalSection	_FilenameTemplateCS;

protected:
	struct _Chunk
	{	UINT	size;		
		UINT	length;		// INFINITE indicates the chunk is not finalized
		BYTE	data[1];	// real size set is by SetLogEntrySize
		_Chunk*	GetNext(){ return (_Chunk*)(data + size); }
	};
	static const SIZE_T _ChunkHeaderSize = sizeof(UINT)*2;
	struct _WriteBuf
	{	LPBYTE			pBuf;
		volatile int	Used;
	};
	_WriteBuf			_WriteBuffers[2];

protected:
	_WriteBuf*			_FrontBuf;
	_WriteBuf*			_BackBuf;
	UINT				_WriteDownInterval;
	UINT				_WriteBufferSize;
	os::Thread			_WriteThread;

	void				_WriteDownBackBuffer();
	void				_WriteRoute();
	void				_RealizeFilename(rt::String& filename);

public: // statistic
	int					stat_FileError;
	int					stat_UnfinalizedChunk;
	float				stat_ClaimPerSecond;
	int					stat_BufferUsageAvg;			// percentage
	int					stat_BufferUsagePeek;			// percentage
	int					stat_FileIOUsageAvg;			// percentage
	int					stat_FileIOUsagePeek;			// percentage
	volatile int		stat_TotalClaimed;
	volatile int		stat_ClaimFailure;
	volatile int		stat_FinalizeFailure;

public:
	ParallelFileWriter();
	~ParallelFileWriter();
	static void RealizeFilename(const rt::String& file_template, const os::Timestamp::Fields& time_f, rt::String& filename);
	void	SetFileWritingCallback(FUNC_FileWriter cb, LPVOID cookie);
	bool	Open(LPCSTR filename, bool append_existing = true, UINT buffer_size = 1024*1024, bool filetime_by_localtime = false);  // filename may contain macros as %YEAR% %MONTH% %DAY% %HOUR% %MINUTE% %HALFHOUR% %QUARTER%
	void	Close();
	void	PreClose(); // call if you want ParallelFileWriter to close but will do something else before hanging on Close() or dtor
	void	LogAlert();
	bool	IsOpen() const { return _File.IsOpen(); }
	void	SwitchTo(LPCSTR filename, bool no_wait);
	UINT	GetWriteDownInterval() const { return _WriteDownInterval; }
	void	SetWriteDownInterval(UINT write_interval_msec);
	UINT	GetBufferSize() const { return _WriteBufferSize; }
	void	GetCurrentLogFilename(rt::String& filename);
	const rt::String& GetLogFilename() const { return _FilenameTemplate; }
	INLFUNC LPSTR ClaimWriting(UINT size)
	{	ASSERT(size);
		_WriteBuf* buf = _FrontBuf;
        int chunk = os::AtomicAdd(size + _ChunkHeaderSize, &buf->Used) - (size + _ChunkHeaderSize);
		os::AtomicIncrement(&stat_TotalClaimed);
		if(chunk + (int)size + _ChunkHeaderSize <= (int)_WriteBufferSize)
		{
			_Chunk* p = (_Chunk*)&buf->pBuf[chunk];
			p->size = size;
			p->length = INFINITE;
			return (LPSTR)p->data;
		}
		else
		{	// this may happen when the writer is much faster then the sync thread
			os::AtomicIncrement(&stat_ClaimFailure);
			return nullptr;
		}
	}

	INLFUNC void FinalizeWritten(LPSTR pBuf, UINT len)
	{	_Chunk* p = (_Chunk*)&pBuf[-((int)_ChunkHeaderSize)];

		if(	len <= p->size &&
			p->length == INFINITE && p->size < _WriteBufferSize)
		{	p->length = len;	}
		else 
			os::AtomicIncrement(&stat_FinalizeFailure);
			// This may happen when it takes too long time between a paired 
			// ClaimWriting/FinalizeWritten calling. e.g. longer than m_WriteDownInterval
			// other possiblity is the caller ruined the buffer (run down)  
	}

	template<class T>
	INLFUNC bool WriteString(const T& x)	// string expression or json definition
	{	
		UINT len = (UINT)x.GetLength();
		LPSTR p = ClaimWriting(len);
		if(p)
		{	VERIFY(len == x.CopyTo((char*)p));
			FinalizeWritten(p,len);
			return true;
		}else return false;
	}
	template<class T>
	INLFUNC bool WriteLine(const T& x)	// string expression or json definition
	{
		UINT len = (UINT)x.GetLength();
		LPSTR p = ClaimWriting(len + 2);
		if(p)
		{	VERIFY(len == x.CopyTo((char*)p));
			*((WORD*)(p + len)) = 0x0a0d;	// "\r\n"
			FinalizeWritten(p,len+2);
			return true;
		}else return false;
	}
	INLFUNC bool WriteString(const rt::String_Ref& str){ ASSERT(str.GetLength()<INT_MAX); return Write(str.Begin(),(UINT)str.GetLength()); }
	INLFUNC bool Write(LPCVOID str, UINT len)
	{	
		LPSTR p = ClaimWriting(len);
		if(p)
		{	memcpy(p,str,len);
			FinalizeWritten(p,len);
			return true;
		}else return false;
	}
};

} // namespace os