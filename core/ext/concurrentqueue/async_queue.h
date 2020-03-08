#pragma once
#include "../../../../CPCF/essentials.h"
#include "blockingconcurrentqueue.h"
#include "readerwriterqueue.h"

namespace ext
{

/////////////////////////////////////////////////////////////////////////////////////
// Lock-free queue based on moodycamel::ConcurrentQueue
// http://moodycamel.com/blog/2014/a-fast-general-purpose-lock-free-queue-for-c++
// 
// *** WARNING ***
// Unless `singleReaderWriter` is true, the Pop order of elements
// will not be the exact Push order of elements, which can be a little jittered 
// when there are multiple producing threads.
//
// Another alternative is https://github.com/max0x7ba/atomic_queue which support
// consistent total order across multiple producing threads but lacking blocking
// Push/Pop
/////////////////////////////////////////////////////////////////////////////////////

template<typename SEQNO>
class SeqNoDeJitter
{
#if defined(PLATFORM_DEBUG_BUILD)
	rt::hash_set<SEQNO>		_Dedup;
#endif
	SEQNO		_Min;	// consolidate on min
	SEQNO		_Max;	
	SIZE_T		_GapFilled;

public:
	SeqNoDeJitter(SEQNO init = 0){ Reset(init); }
	void Reset(SEQNO init)
	{	_GapFilled = 0;
		_Min = _Max = init;
#if defined(PLATFORM_DEBUG_BUILD)
		_Dedup.clear();
#endif
	}
	bool Insert(SEQNO s) // return true if consolidation updated
	{	ASSERT(s > _Min && s != _Max);
		ASSERT(_Dedup.find(s) == _Dedup.end());
		if(s > _Max)
		{	_GapFilled += s - _Max - 1;
			if(_GapFilled == 0)
			{	_Min = _Max = s;
				ASSERT(_Dedup.size() == 0);
				return true; 
			}
#if defined(PLATFORM_DEBUG_BUILD)
			_Dedup.insert(_Max);
#endif
			_Max = s;
		}
		else
		{	ASSERT(_GapFilled);
			_GapFilled--;
			if(_GapFilled == 0)
			{	_Min = _Max;
#if defined(PLATFORM_DEBUG_BUILD)
				_Dedup.clear();
#endif
				return true; 
			}
#if defined(PLATFORM_DEBUG_BUILD)
			_Dedup.insert(s);
#endif
		}
		return false;
	}
	SEQNO DeJittered() const { return _Min; }
	bool  IsJittering() const { return _Min != _Max; }
};
	
namespace _details
{

template<UINT PL_MAX>
struct AsyncData
{
	union
	{	UINT	Size24;	// 16M max
	struct{
		BYTE	__padding[3];
		BYTE	Type;
	};};
	BYTE		Data[PL_MAX];

	INLFUNC void SetSize(UINT size, UINT type)
	{	ASSERT(size <= PL_MAX);
		ASSERT(type <= 0xff);
		Size24 = size;
		Type = type;
	}
	INLFUNC UINT GetSize() const { return Size24&0xffffff; }
	template<typename T>
	T& Val(){ return *(T*)Data; }
};

template<UINT block_size>
struct CQ_Traits: public moodycamel::ConcurrentQueueDefaultTraits
{	static const size_t BLOCK_SIZE = block_size;
};

template<typename T, bool blocking, bool singleReaderWriter, UINT block_size = 32>
struct _ConcurrentQueue;
	template<typename T, UINT block_size> 
	struct _ConcurrentQueue<T, true, false, block_size>
		:public moodycamel::BlockingConcurrentQueue<T, CQ_Traits<block_size>>
    {   typedef moodycamel::BlockingConcurrentQueue<T, CQ_Traits<block_size>> _SC;
        INLFUNC _ConcurrentQueue(UINT reserved_size)
            :moodycamel::BlockingConcurrentQueue<T, CQ_Traits<block_size>>(reserved_size){ ASSERT(_SC::is_lock_free()); }
		INLFUNC bool Pop(T& t, UINT timeout = 0)
        {	return timeout?_SC::wait_dequeue_timed(t, timeout*1000LL):_SC::try_dequeue(t);
		}
	};
	template<typename T, UINT block_size>
	struct _ConcurrentQueue<T, false, false, block_size>
		:public moodycamel::ConcurrentQueue<T, CQ_Traits<block_size>>
    {   typedef moodycamel::ConcurrentQueue<T, CQ_Traits<block_size>> _SC;
        INLFUNC _ConcurrentQueue(UINT reserved_size)
            :moodycamel::ConcurrentQueue<T, CQ_Traits<block_size>>(reserved_size){ ASSERT(_SC::is_lock_free()); }
		INLFUNC bool Pop(T& t, UINT timeout = 0)
		{	ASSERT(timeout==0);
            return _SC::try_dequeue(t);
		}
	};
	template<typename T, UINT block_size>
	struct _ConcurrentQueue<T, true, true, block_size>
		:public moodycamel::BlockingReaderWriterQueue<T, block_size>
    {   typedef moodycamel::BlockingReaderWriterQueue<T, block_size> _SC;
        INLFUNC _ConcurrentQueue(UINT reserved_size):moodycamel::BlockingReaderWriterQueue<T, block_size>(reserved_size){}
		INLFUNC bool Pop(T& t, UINT timeout = 0)
        {	return timeout?_SC::wait_dequeue_timed(t, timeout*1000LL): _SC::try_dequeue(t);
		}
	};
	template<typename T, UINT block_size>
	struct _ConcurrentQueue<T, false, true, block_size>
		:public moodycamel::ReaderWriterQueue<T, block_size>
	{	INLFUNC _ConcurrentQueue(UINT reserved_size):moodycamel::ReaderWriterQueue<T, block_size>(reserved_size){}
		INLFUNC bool Pop(T& t, UINT timeout = 0)
		{	ASSERT(timeout==0);
			return try_dequeue(t);
		}
	};
} // _details

template<typename T, bool BLOCKING = true, UINT BLOCK_SIZE = 32, bool SINGLE_READER_WRITER = false>
// the reserved size in the ctor will be round to multiple of BLOCK_SIZE
class AsyncDataQueueInfinite
{
	typedef _details::_ConcurrentQueue<T, BLOCKING, SINGLE_READER_WRITER, BLOCK_SIZE> QueueType;
protected:
	QueueType	_Q;
	INT			_InitReservedSize;

public:
	AsyncDataQueueInfinite(INT ReservedSize = 32):_Q(_InitReservedSize = ReservedSize){}

	void Push(const T& t){ VERIFY(_Q.enqueue(t)); }
	bool Pop(T& t, UINT timeout = 0){ return _Q.Pop(t,timeout); }
	void Empty(){ _Q.~QueueType(); new (&_Q) QueueType(_InitReservedSize); }
};

template<typename T, bool BLOCKING = true, UINT BLOCK_SIZE = 32, bool SINGLE_READER_WRITER = false>
// the reserved size in the ctor will be round to multiple of BLOCK_SIZE
class AsyncDataQueue: public AsyncDataQueueInfinite<T, BLOCKING, BLOCK_SIZE, SINGLE_READER_WRITER>
{
	typedef AsyncDataQueueInfinite<T, BLOCKING,  BLOCK_SIZE,SINGLE_READER_WRITER> _SC;
	volatile INT	__Size;
	INT				__SizeMax;
public:
	AsyncDataQueue(INT MaxSize = 0x7fffffff, INT ReservedSize = 32):_SC(ReservedSize){ __Size = 0; __SizeMax = MaxSize; }
	INLFUNC bool Push(const T& t, bool force_grow = false)
	{	if(force_grow || __Size<__SizeMax)
		{	
			os::AtomicIncrement(&__Size); 
			VERIFY(_SC::_Q.enqueue(t));
			return true; 
		}
		else return false;
	}
	INLFUNC bool Pop(T& t, UINT timeout = 0)
	{	if(_SC::_Q.Pop(t,timeout))
		{	os::AtomicDecrement(&__Size);
			return true;
		}
		else return false;
	}
	INLFUNC INT GetSize() const { return __Size; }
};

template<template<typename T, bool BLOCKING, UINT BLOCK_SIZE, bool SINGLE_READER_WRITER> class T_QUEUE, typename T, UINT JITTER_BUFSIZE_MAX = 1024U, bool BLOCKING = true, UINT BLOCK_SIZE = 32, bool SINGLE_READER_WRITER = false>
class DeJitteredQueue // only for single reader
{
protected:
	static const UINT SEQ_RANGESIZE			= 0x80000000U;
	static const UINT SEQ_BITMASK			= SEQ_RANGESIZE - 1U;
	static const UINT FLAG_NOTNULL			= 0x80000000U;

	static_assert((JITTER_BUFSIZE_MAX >= 4) && !(JITTER_BUFSIZE_MAX & (JITTER_BUFSIZE_MAX - 1)), "JITTER_BUFSIZE_MAX must be a power of 2 (and at least 4)");
	
	struct SeqT
	{	UINT	SeqFlag; // [Seq:31b][Null:1b]
		T		Obj;
		SeqT(){ SeqFlag = 0; }
		SeqT(UINT seqflag, const T& obj):Obj(obj){ SeqFlag = seqflag; ASSERT(IsNotNull()); }
		UINT	Seq() const { return SeqFlag&SEQ_BITMASK; }
		bool	IsNotNull() const { return SeqFlag&FLAG_NOTNULL; }
	};

	T_QUEUE<SeqT, BLOCKING, BLOCK_SIZE, SINGLE_READER_WRITER>	_Queue;
	// writer state
	volatile UINT		_NextSeq;
	// reader state (single thread)
	UINT				_LastReadSeq;
	rt::BufferEx<SeqT>	_JitterBuf;
	UINT				_JitterBufBaseSeq;

	static UINT			_SeqMinus(UINT large, UINT small)
						{	ASSERT(large<SEQ_RANGESIZE && small<SEQ_RANGESIZE);
							if(large >= small){ return large-small; }
							else{	ASSERT(large < 0x40000000U && small > 0x40000000U);
									return large + 0x80000000U - small;
						}		}
	static UINT			_SeqInc(UINT s){ return (s+1)&SEQ_BITMASK; }
	UINT				_WriterNextSeqFlag(){ return FLAG_NOTNULL|os::AtomicIncrement((volatile int*)&_NextSeq); }

public:
	DeJitteredQueue(){ _NextSeq = -1; _LastReadSeq = -1; _JitterBufBaseSeq = 0; }
	template<typename ...ARGS>
	auto Push(const T& t, ARGS... args){ return _Queue.Push(SeqT(_WriterNextSeqFlag(),t), args...); }
	template<typename ...ARGS>
	bool Pop(T* t, ARGS... args)
	{
		UINT expected_seq = _SeqInc(_LastReadSeq);
		if(_JitterBuf.GetSize())
		{	// pop from jitter buffer if available
			UINT idx = _SeqMinus(expected_seq, _JitterBufBaseSeq);
			if(idx < _JitterBuf.GetSize() && _JitterBuf[idx].IsNotNull())
			{	
				_LastReadSeq = expected_seq;
				*t = std::move(_JitterBuf[idx].Obj);
				// shrink jitter buffer
				if(idx + 1 == _JitterBuf.GetSize()){ _JitterBuf.ShrinkSize(0); }
				else if(_JitterBuf.GetSize() > JITTER_BUFSIZE_MAX/2 && idx > _JitterBuf.GetSize()/2)
				{	_JitterBuf.erase(0, idx+1);
					_JitterBufBaseSeq = (idx + 1 + _JitterBufBaseSeq)&SEQ_BITMASK;
				}

				return true;
			}
		}

		SeqT pop;
		while(_Queue.Pop(pop, args...))
		{
			if(pop.Seq() == expected_seq) // no jitter
			{
				_LastReadSeq = expected_seq;
				*t = std::move(pop.Obj);
				return true;
			}
			// jittered
			ASSERT(pop.IsNotNull());
			UINT idx;
			if(_JitterBuf.GetSize())
			{
				idx = _SeqMinus(pop.Seq(), _JitterBufBaseSeq);
				ASSERT(idx < JITTER_BUFSIZE_MAX);
				if(idx >= _JitterBuf.GetSize())VERIFY(_JitterBuf.ChangeSize(idx+1));
			}
			else
			{
				_JitterBufBaseSeq = expected_seq;
				idx = _SeqMinus(pop.Seq(), _JitterBufBaseSeq);
				ASSERT(idx < JITTER_BUFSIZE_MAX);
				_JitterBuf.ChangeSize(idx+1);
			}
			_JitterBuf[idx] = pop;
		}

		return false;
	}
};

} // ext
