#pragma once
#include "../../rt/runtime_base.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "./inc/7z.h"
#include "./inc/7zAlloc.h"
#include "./inc/7zCrc.h"
#include "./inc/7zFile.h"
#include "./inc/7zVersion.h"
#include "./inc/LzmaEnc.h"
#include "./inc/LzmaDec.h"
	
#ifdef __cplusplus
}
#endif

namespace rt
{

namespace _details
{
	struct lamz_alloc:public ISzAlloc
	{
		static void *SzAlloc(void *p, size_t size) { (void)(p); return _Malloc32AL(BYTE, size); }
		static void SzFree(void *p, void *address) { (void)(p); _SafeFree32AL(address); }
		lamz_alloc()
		{	Alloc = SzAlloc;
			Free = SzFree;
		}
	};
};

INLFUNC bool lzma_encode(LPCVOID pSrc, size_t SrcLen, LPVOID pDst, size_t& DstLen, int Compression_Level = -1)
{
	if(DstLen <= LZMA_PROPS_SIZE)return 0;

	size_t propsSize = LZMA_PROPS_SIZE;
	size_t dlen = DstLen - LZMA_PROPS_SIZE;
	
	CLzmaEncProps props;
	LzmaEncProps_Init(&props);
	props.level = Compression_Level>=0?rt::min(9, Compression_Level):9;
	props.dictSize = 512*1024; // 64 KB
	props.algo = 1;
	props.btMode = 1;
	props.numThreads = 1;
	props.writeEndMark = 0; // 0 or 1

	_details::lamz_alloc _Alloc;

	int res = LzmaEncode(
		((LPBYTE)pDst) + LZMA_PROPS_SIZE, &dlen,
		(LPCBYTE)pSrc, SrcLen,
		&props, (LPBYTE)pDst, &propsSize, props.writeEndMark,
		NULL, &_Alloc, &_Alloc //&SzAllocForLzma, &SzAllocForLzma
	);
	if(res == SZ_OK && propsSize == LZMA_PROPS_SIZE)
	{
		DstLen = LZMA_PROPS_SIZE + dlen;
		return true;
	}
  
	return 0;
}

INLFUNC bool lzma_decode(LPCVOID pSrc, size_t SrcLen, LPVOID pDst, size_t& DstLen)
{
	if(SrcLen < LZMA_PROPS_SIZE)return false;

	_details::lamz_alloc _Alloc;
	ELzmaStatus s;
	return SZ_OK == LzmaDecode((LPBYTE)pDst, &DstLen, ((LPCBYTE)pSrc) + LZMA_PROPS_SIZE, &SrcLen, (LPCBYTE)pSrc, LZMA_PROPS_SIZE, LZMA_FINISH_ANY, &s, &_Alloc);
//SRes LzmaDecode(Byte *dest, SizeT *destLen, const Byte *src, SizeT *srcLen,
//    const Byte *propData, unsigned propSize, ELzmaFinishMode finishMode,
//    ELzmaStatus *status, ISzAlloc *alloc)
}

}

