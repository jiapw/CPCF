#pragma once
/** \addtogroup ConCurrentQueue
 * @ingroup ext
 *  @{
 */
/**
 * @file async_queue.h
 * @author JP Wang (wangjiaping@idea.edu.cn)
 * @brief 
 * @version 1.0
 * @date 2021-05-08
 * 
 * @copyright  
 * Cross-Platform Core Foundation (CPCF)
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *      * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *      * Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials provided
 *        with the distribution.
 *      * Neither the name of CPCF.  nor the names of its
 *        contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *  
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.   
 */
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
	/**
	 * @brief return true if consolidation updated
	 * 
	 * @param s 
	 * @return true 
	 * @return false 
	 */
	bool Insert(SEQNO s) 
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
	SEQNO	DeJittered() const { return _Min; }
	bool	IsJittering() const { return _Min != _Max; }
	SIZE_T	GetMissingCount() const { ASSERT(_Max - _Min >= _GapFilled); return _Max - _Min - _GapFilled; }
	auto	SeqNoMax() const { return _Min; }
	auto	SeqNoMin() const { return _Max; }
};
	
namespace _details
{

template<UINT PL_MAX>
struct AsyncData
{
	union
	{	UINT	Size24;	///< 16M max
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
	template<bool HasCounter>
	class _QueueCounter
	{	protected:
			INT		_InitReservedSize;
			INLFUNC bool _PostPop(bool yes){ return yes; }
			INLFUNC bool _PostPush(bool yes){ return yes; }
			_QueueCounter(int reserved):_InitReservedSize(reserved){}
	};	template<>
		class _QueueCounter<true>
		{	protected:
				INT				_InitReservedSize;
				volatile INT	_Size;
				INLFUNC bool _PostPop(bool yes){ if(yes)os::AtomicDecrement(&_Size); return yes; }
				INLFUNC bool _PostPush(bool yes){ if(yes)os::AtomicIncrement(&_Size); return yes; }
				_QueueCounter(int reserved):_InitReservedSize(reserved){ _Size = 0; }
			public:
				INLFUNC INT GetSize() const { return _Size; }
		};
} // _details

template<typename T, bool BLOCKING = true, UINT BLOCK_SIZE = 32, bool HAS_GETSIZE = false, bool SINGLE_READER_WRITER = false>
class AsyncDataQueueInfinite: public _details::_QueueCounter<HAS_GETSIZE>
{	
	typedef _details::_QueueCounter<HAS_GETSIZE> _SC;
	typedef _details::_ConcurrentQueue<T, BLOCKING, SINGLE_READER_WRITER, BLOCK_SIZE> QueueType;
protected:
	QueueType	_Q;
public:
	typedef T TYPE;
	template<typename T2>
	using WithType = AsyncDataQueueInfinite<T2, BLOCKING, BLOCK_SIZE, HAS_GETSIZE, SINGLE_READER_WRITER>;

	AsyncDataQueueInfinite(INT reserve_size = 32):_Q(reserve_size),_details::_QueueCounter<HAS_GETSIZE>(reserve_size){}
	void	Push(const T& t){ VERIFY((_SC::_PostPush(_Q.enqueue(t)))); }
	bool	Pop(T& t, UINT timeout = 0){ return _SC::_PostPop(_Q.Pop(t,timeout)); }
	void	Empty(){ _Q.~QueueType(); new (&_Q) QueueType(_SC::_InitReservedSize); }
};
/**
 * @brief the reserved size in the ctor will be round to multiple of BLOCK_SIZE
 * 
 * @tparam T 
 * @tparam BLOCKING 
 * @tparam BLOCK_SIZE 
 * @tparam SINGLE_READER_WRITER 
 */
template<typename T, bool BLOCKING = true, UINT BLOCK_SIZE = 32, bool SINGLE_READER_WRITER = false>
class AsyncDataQueue: public AsyncDataQueueInfinite<T, BLOCKING, BLOCK_SIZE, true, SINGLE_READER_WRITER>
{
	typedef AsyncDataQueueInfinite<T, BLOCKING, BLOCK_SIZE, true, SINGLE_READER_WRITER> _SC;
	INT	__SizeMax;
public:
	typedef T TYPE;
	template<typename T2>
	using WithType = AsyncDataQueue<T2, BLOCKING, BLOCK_SIZE, SINGLE_READER_WRITER>;

	AsyncDataQueue(INT max_size = 0x7fffffff, INT reserve_size = 32):_SC(reserve_size){ __SizeMax = max_size; }
	INLFUNC bool Push(const T& t, bool force_grow = false)
	{	if(force_grow || _SC::GetSize()<__SizeMax)
		{	_SC::Push(t);
			return true; 
		}
		else return false;
	}
};
/**
 * @brief only for single reader
 * 
 * @tparam T_QUEUE 
 * @tparam JITTER_BUFSIZE_MAX 
 */
template<class T_QUEUE, UINT JITTER_BUFSIZE_MAX = 1024U>
class DeJitteredQueue 
{
	static_assert((JITTER_BUFSIZE_MAX >= 4) && !(JITTER_BUFSIZE_MAX & (JITTER_BUFSIZE_MAX - 1)), "JITTER_BUFSIZE_MAX must be a power of 2 (and at least 4)");
	typedef typename T_QUEUE::TYPE	TYPE;
protected:
	static const UINT SEQ_RANGESIZE			= 0x80000000U;
	static const UINT SEQ_BITMASK			= SEQ_RANGESIZE - 1U;
	static const UINT FLAG_NOTNULL			= 0x80000000U;
	
	struct SeqT
	{	UINT		SeqFlag; ///< [Seq:31b][Null:1b]
		TYPE		Obj;
		SeqT(){ SeqFlag = 0; }
		SeqT(UINT seqflag, const TYPE& obj):Obj(obj){ SeqFlag = seqflag; ASSERT(IsNotNull()); }
		UINT		Seq() const { return SeqFlag&SEQ_BITMASK; }
		bool		IsNotNull() const { return SeqFlag&FLAG_NOTNULL; }
	};

	typename T_QUEUE::template WithType<SeqT>	_Queue;
	
	volatile UINT		_NextSeq; ///< writer state
	
	UINT				_LastReadSeq;///< reader state (single thread)
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
	auto Push(const TYPE& t, ARGS... args){ return _Queue.Push(SeqT(_WriterNextSeqFlag(),t), args...); }
	template<typename ...ARGS>
	bool Pop(TYPE* t, ARGS... args)
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
/** @}*/