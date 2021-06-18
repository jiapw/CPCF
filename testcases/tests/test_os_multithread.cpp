#include "../../core/os/multi_thread.h"

#pragma warning(disable: 4838)

#include "test.h"

os::CriticalSection	test_multithread_ccs;
os::Event test_multithread_event;

void multithread_WaitForEnding()
{
	{	DWORD a = 0x0010;
	DWORD org = os::AtomicOr(0x1000, &a);
	_LOG("AtomicOr: " << rt::tos::HexNum<>(org) << " -> " << rt::tos::HexNum<>(a));
	}

	{
		volatile INT c = 0;
		bool ends = false;
		os::Thread a, b;
		auto th = [&c, &ends]()
		{	while (!ends)
		{
			int x = os::AtomicIncrement(&c);
			_LOGC("c = " << x << " TH=" << os::Thread::GetCurrentId());
			os::Sleep(100);
		}
		_LOGC("Exit TH=" << os::Thread::GetCurrentId());
		};
		a.Create(th);
		b.Create(th);
		os::Sleep(2000);
		ends = true;
		a.WaitForEnding();
		b.WaitForEnding();
	}
	{
		volatile INT c = 0;
		os::Thread a, b;
		auto th = [&c]()
		{
			for (;;)
			{
				int x = os::AtomicIncrement(&c);
				_LOGC("c = " << x << " TH=" << os::Thread::GetCurrentId());
				os::Sleep(100);
			}
			_LOGC("Exit TH=" << os::Thread::GetCurrentId());
		};
		a.Create(th);
		b.Create(th);
		a.WaitForEnding(2000, true);
		b.WaitForEnding(2000, true);
	}
}

void multithread_ATOM()
{
	static const int LOOPCOUNT = 500000;

	struct _call_atom
	{
		static DWORD _atom_inc(LPVOID pi)
		{
			volatile int* p = (volatile int*)pi;
			for (int i = 0; i < LOOPCOUNT; i++)
			{
				os::AtomicIncrement(p);
			}
			return 0;
		}
		static DWORD _atom_dec(LPVOID pi)
		{
			volatile int* p = (volatile int*)pi;
			for (int i = 0; i < LOOPCOUNT; i++)
			{
				os::AtomicDecrement(p);
			}
			return 0;
		}
		static DWORD _ccs_inc(LPVOID pi)
		{
			volatile int* p = (volatile int*)pi;
			for (int i = 0; i < LOOPCOUNT / 10; i++)
			{
				EnterCSBlock(test_multithread_ccs);
				(*p)++;
			}
			return 0;
		}
		static DWORD _ccs_dec(LPVOID pi)
		{
			volatile int* p = (volatile int*)pi;
			for (int i = 0; i < LOOPCOUNT / 10; i++)
			{
				EnterCSBlock(test_multithread_ccs);
				(*p)--;
			}
			return 0;
		}
		static DWORD _inc(LPVOID pi)
		{
			volatile int* p = (volatile int*)pi;
			for (int i = 0; i < LOOPCOUNT; i++)
			{
				(*p)++;
			}
			return 0;
		}
		static DWORD _dec(LPVOID pi)
		{
			volatile int* p = (volatile int*)pi;
			for (int i = 0; i < LOOPCOUNT; i++)
			{
				(*p)--;
			}
			return 0;
		}
	};

	double ops_atom, ops_ccs, ops_nosync;

	{	volatile int counter = 456789;
	os::Thread	thread_inc[4], thread_dec[4];
	for (int i = 0; i < sizeofArray(thread_inc); i++)
	{
		thread_inc[i].Create(_call_atom::_atom_inc, (LPVOID)&counter);
		thread_dec[i].Create(_call_atom::_atom_dec, (LPVOID)&counter);
	}
	for (int i = 0; i < sizeofArray(thread_inc); i++)
	{
		thread_inc[i].WaitForEnding();
		thread_dec[i].WaitForEnding();
	}
	_LOG("ATOM Result: " << counter);

	os::TickCount t;
	t.LoadCurrentTick();
	for (int i = 0; i < LOOPCOUNT; i++)
	{
		os::AtomicIncrement(&counter);
		os::AtomicIncrement(&counter);
		os::AtomicIncrement(&counter);
		os::AtomicIncrement(&counter);
		os::AtomicIncrement(&counter);
	}
	int cost = t.TimeLapse();
	ops_atom = (5.0 * LOOPCOUNT) / cost;
	}

	{	volatile int counter = 456789;
	os::Thread	thread_inc[4], thread_dec[4];
	for (int i = 0; i < sizeofArray(thread_inc); i++)
	{
		thread_inc[i].Create(_call_atom::_ccs_inc, (LPVOID)&counter);
		thread_dec[i].Create(_call_atom::_ccs_dec, (LPVOID)&counter);
	}
	for (int i = 0; i < sizeofArray(thread_inc); i++)
	{
		thread_inc[i].WaitForEnding();
		thread_dec[i].WaitForEnding();
	}
	_LOG("CriticalSection Result: " << counter);

	os::TickCount t;
	t.LoadCurrentTick();
	for (int i = 0; i < LOOPCOUNT; i++)
	{
		{	EnterCSBlock(test_multithread_ccs);	counter++; }
		{	EnterCSBlock(test_multithread_ccs);	counter++; }
		{	EnterCSBlock(test_multithread_ccs);	counter++; }
		{	EnterCSBlock(test_multithread_ccs);	counter++; }
		{	EnterCSBlock(test_multithread_ccs);	counter++; }
	}
	int cost = t.TimeLapse();
	ops_ccs = (5.0 * LOOPCOUNT) / cost;
	}
	{	volatile int counter = 456789;
	os::Thread	thread_inc[4], thread_dec[4];
	for (int i = 0; i < sizeofArray(thread_inc); i++)
	{
		thread_inc[i].Create(_call_atom::_inc, (LPVOID)&counter);
		thread_dec[i].Create(_call_atom::_dec, (LPVOID)&counter);
	}
	for (int i = 0; i < sizeofArray(thread_inc); i++)
	{
		thread_inc[i].WaitForEnding();
		thread_dec[i].WaitForEnding();
	}
	_LOGC("Async Result: " << counter);
	_LOGC("sync is required: " << (counter != 456789));

	os::TickCount t;
	t.LoadCurrentTick();
	for (int i = 0; i < LOOPCOUNT; i++)
	{
		counter++;
		counter++;
		counter++;
		counter++;
		counter++;
	}
	int cost = t.TimeLapse();
	ops_nosync = (5.0 * LOOPCOUNT) / cost;
	}

	_LOGC("// CriticalSection:  " << ops_ccs << " kop/s");//<<rt::tos::Number((float)LOOPCOUNT/cost)<<" kop/s");
	_LOGC("// ATOM           : " << ops_atom << " kop/s");//rt::tos::Number((float)10*LOOPCOUNT/cost)<<" kop/s");
	_LOGC("// Nosync         : " << ops_nosync << " kop/s");//rt::tos::Number((float)10*LOOPCOUNT/cost)<<" kop/s");

}
void multithread_Event()
{
	test_multithread_event.Reset();
	_LOG("Event Set: " << test_multithread_event.IsSignaled());
	test_multithread_event.Set();
	_LOG("Event Set: " << test_multithread_event.IsSignaled());

	struct _call_event
	{
		static DWORD _wait_thread(LPVOID pi)
		{
			os::TickCount t;
			t.LoadCurrentTick();
			test_multithread_event.WaitSignal(2000);
			_LOG("Event Timeout: " << 1000 * ((200 + t.TimeLapse()) / 1000));
			test_multithread_event.Reset();

			t.LoadCurrentTick();
			test_multithread_event.WaitSignal();
			test_multithread_event.WaitSignal();
			_LOG("Thread Waited: " << 1000 * ((200 + t.TimeLapse()) / 1000) << ", " << test_multithread_event.IsSignaled());

			return 0;
		}
	};

	{	test_multithread_event.Reset();

	os::Thread	th;
	th.Create(_call_event::_wait_thread, nullptr);
	_LOG("Thread Run: " << th.IsRunning());
	os::Sleep(4000);

	_LOG("Event Set");
	test_multithread_event.Set();

	th.WaitForEnding();
	_LOG("Thread Ended");
	}
}
void rt::UnitTests::multithread()
{
	multithread_WaitForEnding();
	multithread_ATOM();
	multithread_Event();
}

struct A
{
	os::TickCount tc;
	A() { tc.LoadCurrentTick(); }
	~A()
	{
		_LOG("killed, after " << tc.TimeLapse() << " msec");
	}
};

void rt::UnitTests::delayed_deletion()
{
	A* p = _New(A);
	_SafeDel_Delayed(p, 2000);
	p = _New(A);
	_SafeDel_Delayed(p, 5000);

	os::Sleep(9000);
}