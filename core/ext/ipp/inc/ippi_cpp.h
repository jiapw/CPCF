namespace ipp
{
	namespace ipp_cpp
	{

#define IPPI_WRAP inline IppStatus

IPPI_WRAP ippiCopy_C1R( const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int 
                             dstStep, IppiSize roiSize) { 
     return IPPCALL(ippiCopy_8u_C1R)( pSrc, srcStep, pDst, dstStep, roiSize ); 
     } 
IPPI_WRAP ippiCopy_C3R( const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int 
                             dstStep, IppiSize roiSize) { 
     return IPPCALL(ippiCopy_8u_C3R)( pSrc, srcStep, pDst, dstStep, roiSize ); 
     } 
IPPI_WRAP ippiCopy_C4R( const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int 
                             dstStep, IppiSize roiSize) { 
     return IPPCALL(ippiCopy_8u_C4R)( pSrc, srcStep, pDst, dstStep, roiSize ); 
     } 
IPPI_WRAP ippiCopy_C1R(const Ipp16s* pSrc, int srcStep, Ipp16s* pDst,
	int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiCopy_16s_C1R)(pSrc, srcStep, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiCopy_C3R(const Ipp16s* pSrc, int srcStep, Ipp16s* pDst,
	int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiCopy_16s_C3R)(pSrc, srcStep, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiCopy_C4R(const Ipp16s* pSrc, int srcStep, Ipp16s* pDst,
	int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiCopy_16s_C4R)(pSrc, srcStep, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiCopy_C1R(const Ipp32f* pSrc, int srcStep, Ipp32f* pDst,
	int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiCopy_32f_C1R)(pSrc, srcStep, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiCopy_C3R(const Ipp32f* pSrc, int srcStep, Ipp32f* pDst,
	int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiCopy_32f_C3R)(pSrc, srcStep, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiCopy_C4R(const Ipp32f* pSrc, int srcStep, Ipp32f* pDst,
	int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiCopy_32f_C4R)(pSrc, srcStep, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiScale_C1R(const Ipp8u* pSrc, int srcStep, Ipp16u* pDst,
	int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiScale_8u16u_C1R)(pSrc, srcStep, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiScale_C1R(const Ipp8u* pSrc, int srcStep, Ipp16s* pDst,
	int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiScale_8u16s_C1R)(pSrc, srcStep, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiScale_C1R(const Ipp8u* pSrc, int srcStep, Ipp32s* pDst,
	int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiScale_8u32s_C1R)(pSrc, srcStep, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiScale_C1R(const Ipp8u* pSrc, int srcStep, Ipp32f* pDst,
	int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiScale_8u32f_C1R)(pSrc, srcStep, pDst, dstStep, roiSize, GetEnv()->FloatMin, GetEnv()->FloatMax);
}
IPPI_WRAP ippiScale_C1R(const Ipp16u* pSrc, int srcStep, Ipp8u* pDst,
	int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiScale_16u8u_C1R)(pSrc, srcStep, pDst, dstStep, roiSize, GetEnv()->HintAlgorithm);
}
IPPI_WRAP ippiScale_C1R(const Ipp16s* pSrc, int srcStep, Ipp8u* pDst,
	int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiScale_16s8u_C1R)(pSrc, srcStep, pDst, dstStep, roiSize, GetEnv()->HintAlgorithm);
}
IPPI_WRAP ippiScale_C1R(const Ipp32s* pSrc, int srcStep, Ipp8u* pDst,
	int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiScale_32s8u_C1R)(pSrc, srcStep, pDst, dstStep, roiSize, GetEnv()->HintAlgorithm);
}
IPPI_WRAP ippiScale_C1R(const Ipp32f* pSrc, int srcStep, Ipp8u* pDst,
	int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiScale_32f8u_C1R)(pSrc, srcStep, pDst, dstStep, roiSize, GetEnv()->FloatMin, GetEnv()->FloatMax);
}
IPPI_WRAP ippiScale_C3R(const Ipp8u* pSrc, int srcStep, Ipp16u* pDst,
	int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiScale_8u16u_C3R)(pSrc, srcStep, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiScale_C3R(const Ipp8u* pSrc, int srcStep, Ipp16s* pDst,
	int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiScale_8u16s_C3R)(pSrc, srcStep, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiScale_C3R(const Ipp8u* pSrc, int srcStep, Ipp32s* pDst,
	int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiScale_8u32s_C3R)(pSrc, srcStep, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiScale_C3R(const Ipp8u* pSrc, int srcStep, Ipp32f* pDst,
	int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiScale_8u32f_C3R)(pSrc, srcStep, pDst, dstStep, roiSize, GetEnv()->FloatMin, GetEnv()->FloatMax);
}
IPPI_WRAP ippiScale_C3R(const Ipp16u* pSrc, int srcStep, Ipp8u* pDst,
	int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiScale_16u8u_C3R)(pSrc, srcStep, pDst, dstStep, roiSize, GetEnv()->HintAlgorithm);
}
IPPI_WRAP ippiScale_C3R(const Ipp16s* pSrc, int srcStep, Ipp8u* pDst,
	int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiScale_16s8u_C3R)(pSrc, srcStep, pDst, dstStep, roiSize, GetEnv()->HintAlgorithm);
}
IPPI_WRAP ippiScale_C3R(const Ipp32s* pSrc, int srcStep, Ipp8u* pDst,
	int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiScale_32s8u_C3R)(pSrc, srcStep, pDst, dstStep, roiSize, GetEnv()->HintAlgorithm);
}
IPPI_WRAP ippiScale_C3R(const Ipp32f* pSrc, int srcStep, Ipp8u* pDst,
	int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiScale_32f8u_C3R)(pSrc, srcStep, pDst, dstStep, roiSize, GetEnv()->FloatMin, GetEnv()->FloatMax);
}
IPPI_WRAP ippiScale_C4R(const Ipp8u* pSrc, int srcStep, Ipp16u* pDst,
	int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiScale_8u16u_C4R)(pSrc, srcStep, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiScale_C4R(const Ipp8u* pSrc, int srcStep, Ipp16s* pDst,
	int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiScale_8u16s_C4R)(pSrc, srcStep, pDst, dstStep, roiSize);
}

IPPI_WRAP ippiScale_C4R(const Ipp8u* pSrc, int srcStep, Ipp32s* pDst,
	int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiScale_8u32s_C4R)(pSrc, srcStep, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiScale_C4R(const Ipp8u* pSrc, int srcStep, Ipp32f* pDst,
	int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiScale_8u32f_C4R)(pSrc, srcStep, pDst, dstStep, roiSize, GetEnv()->FloatMin, GetEnv()->FloatMax);
}
IPPI_WRAP ippiScale_C4R(const Ipp16u* pSrc, int srcStep, Ipp8u* pDst,
	int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiScale_16u8u_C4R)(pSrc, srcStep, pDst, dstStep, roiSize, GetEnv()->HintAlgorithm);
}
IPPI_WRAP ippiScale_C4R(const Ipp16s* pSrc, int srcStep, Ipp8u* pDst,
	int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiScale_16s8u_C4R)(pSrc, srcStep, pDst, dstStep, roiSize, GetEnv()->HintAlgorithm);
}
IPPI_WRAP ippiScale_C4R(const Ipp32s* pSrc, int srcStep, Ipp8u* pDst,
	int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiScale_32s8u_C4R)(pSrc, srcStep, pDst, dstStep, roiSize, GetEnv()->HintAlgorithm);
}
IPPI_WRAP ippiScale_C4R(const Ipp32f* pSrc, int srcStep, Ipp8u* pDst,
	int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiScale_32f8u_C4R)(pSrc, srcStep, pDst, dstStep, roiSize, GetEnv()->FloatMin, GetEnv()->FloatMax);
}
IPPI_WRAP ippiCopy_P3C3R(const Ipp8u* const pSrc[3], int srcStep,
	Ipp8u* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiCopy_8u_P3C3R)(pSrc, srcStep, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiCopy_P3C3R(const Ipp16s* const pSrc[3], int srcStep,
	Ipp16s* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiCopy_16s_P3C3R)(pSrc, srcStep, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiCopy_P3C3R(const Ipp32f* const pSrc[3], int srcStep,
	Ipp32f* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiCopy_32f_P3C3R)(pSrc, srcStep, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiCopy_P4C4R(const Ipp8u* const pSrc[4], int srcStep,
	Ipp8u* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiCopy_8u_P4C4R)(pSrc, srcStep, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiCopy_P4C4R(const Ipp16s* const pSrc[4], int srcStep,
	Ipp16s* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiCopy_16s_P4C4R)(pSrc, srcStep, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiCopy_P4C4R(const Ipp32f* const pSrc[4], int srcStep,
	Ipp32f* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiCopy_32f_P4C4R)(pSrc, srcStep, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiRGBToGray_C3C1R(const Ipp8u* pSrc, int srcStep, Ipp8u*
	pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiRGBToGray_8u_C3C1R)(pSrc, srcStep, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiRGBToGray_C3C1R(const Ipp16u* pSrc, int srcStep, Ipp16u*
	pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiRGBToGray_16u_C3C1R)(pSrc, srcStep, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiRGBToGray_C3C1R(const Ipp16s* pSrc, int srcStep, Ipp16s*
	pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiRGBToGray_16s_C3C1R)(pSrc, srcStep, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiRGBToGray_C3C1R(const Ipp32f* pSrc, int srcStep, Ipp32f*
	pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiRGBToGray_32f_C3C1R)(pSrc, srcStep, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiSet_C1R(Ipp8u value, Ipp8u* pDst, int dstStep, IppiSize
	roiSize) {
	return IPPCALL(ippiSet_8u_C1R)(value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiSet_C1R(Ipp16s value, Ipp16s* pDst, int dstStep, IppiSize
	roiSize) {
	return IPPCALL(ippiSet_16s_C1R)(value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiSet_C1R(Ipp32f value, Ipp32f* pDst, int dstStep, IppiSize
	roiSize) {
	return IPPCALL(ippiSet_32f_C1R)(value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiCopy_C3AC4R(const Ipp8u* pSrc, int srcStep, Ipp8u* pDst,
	int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiCopy_8u_C3AC4R)(pSrc, srcStep, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiCopy_C3AC4R(const Ipp16s* pSrc, int srcStep, Ipp16s*
	pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiCopy_16s_C3AC4R)(pSrc, srcStep, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiCopy_C3AC4R(const Ipp32f* pSrc, int srcStep, Ipp32f*
	pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiCopy_32f_C3AC4R)(pSrc, srcStep, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiRGBToGray_AC4C1R(const Ipp8u* pSrc, int srcStep, Ipp8u*
	pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiRGBToGray_8u_AC4C1R)(pSrc, srcStep, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiRGBToGray_AC4C1R(const Ipp16u* pSrc, int srcStep, Ipp16u*
	pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiRGBToGray_16u_AC4C1R)(pSrc, srcStep, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiRGBToGray_AC4C1R(const Ipp16s* pSrc, int srcStep, Ipp16s*
	pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiRGBToGray_16s_AC4C1R)(pSrc, srcStep, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiRGBToGray_AC4C1R(const Ipp32f* pSrc, int srcStep, Ipp32f*
	pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiRGBToGray_32f_AC4C1R)(pSrc, srcStep, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiCopy_AC4C3R(const Ipp8u* pSrc, int srcStep, Ipp8u* pDst,
	int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiCopy_8u_AC4C3R)(pSrc, srcStep, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiCopy_AC4C3R(const Ipp16s* pSrc, int srcStep, Ipp16s*
	pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiCopy_16s_AC4C3R)(pSrc, srcStep, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiCopy_AC4C3R(const Ipp32f* pSrc, int srcStep, Ipp32f*
	pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiCopy_32f_AC4C3R)(pSrc, srcStep, pDst, dstStep, roiSize);
}

/* ///////////////////////////////////////////////////////////////////////////
//             Linear Filters
/////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
// Name:   ippiFilterBox_8u_C1R
//         ippiFilterBox_8u_C3R
//         ippiFilterBox_8u_AC4R
//         ippiFilterBox_16s_C1R
//         ippiFilterBox_16s_C3R
//         ippiFilterBox_16s_AC4R
//         ippiFilterBox_32f_C1R
//         ippiFilterBox_32f_C3R
//         ippiFilterBox_32f_AC4R
//
// Purpose:    Applies simple neighborhood averaging filter to blur an image.
// Returns:             IppStatus
//      ippStsNoErr       No errors.
//      ippStsNullPtrErr  pSrc == NULL, or pDst == NULL.
//      ippStsSizeErr     dstRoiSize or roiSize has a field with zero or negative value
//      ippStsMemAllocErr Memory allocation error
//      ippStsStepErr     One of the step values is zero or negative
//      ippStsAnchorErr   Anchor is outside the mask
//      ippStsMaskSizeErr One of the maskSize fields is less than or equal to 1
// Parameters:
//   pSrc        Pointer to the source image
//   srcStep     Step through the source image
//   pDst        Pointer to the destination image
//   dstStep     Step through the destination image
//   pSrcDst     Pointer to the source/destination image (in-place flavors)
//   srcDstStep  Step through the source/destination image (in-place flavors)
//   dstRoiSize  Size of the destination ROI
//   roiSize     Size of the source/destination ROI (in-place flavors)
//   maskSize    Size of the mask in pixels
//   anchor      The [x,y] coordinates of the anchor cell in the kernel
//
*/
//IPPI_WRAP ippiFilterBox_C1R(const Ipp8u* pSrc, int srcStep, Ipp8u*
//	pDst, int dstStep, IppiSize dstRoiSize, IppiSize maskSize,
//	IppiPoint anchor) {
//	return IPPCALL(ippiFilterBox_8u_C1R)(pSrc, srcStep, pDst, dstStep, dstRoiSize, maskSize,
//		anchor);
//}
//IPPI_WRAP ippiFilterBox_C3R(const Ipp8u* pSrc, int srcStep, Ipp8u*
//	pDst, int dstStep, IppiSize dstRoiSize, IppiSize maskSize,
//	IppiPoint anchor) {
//	return IPPCALL(ippiFilterBox_8u_C3R)(pSrc, srcStep, pDst, dstStep, dstRoiSize, maskSize,
//		anchor);
//}
//IPPI_WRAP ippiFilterBox_AC4R(const Ipp8u* pSrc, int srcStep, Ipp8u*
//	pDst, int dstStep, IppiSize dstRoiSize, IppiSize maskSize,
//	IppiPoint anchor) {
//	return IPPCALL(ippiFilterBox_8u_AC4R)(pSrc, srcStep, pDst, dstStep, dstRoiSize, maskSize,
//		anchor);
//}
//IPPI_WRAP ippiFilterBox_C4R(const Ipp8u* pSrc, int srcStep, Ipp8u*
//	pDst, int dstStep, IppiSize dstRoiSize, IppiSize maskSize,
//	IppiPoint anchor) {
//	return IPPCALL(ippiFilterBox_8u_C4R)(pSrc, srcStep, pDst, dstStep, dstRoiSize, maskSize,
//		anchor);
//}
//
//
//IPPI_WRAP ippiFilterBox_C1R(const Ipp16s* pSrc, int srcStep, Ipp16s*
//	pDst, int dstStep, IppiSize dstRoiSize, IppiSize maskSize,
//	IppiPoint anchor) {
//	return IPPCALL(ippiFilterBox_16s_C1R)(pSrc, srcStep, pDst, dstStep, dstRoiSize, maskSize,
//		anchor);
//}
//IPPI_WRAP ippiFilterBox_C3R(const Ipp16s* pSrc, int srcStep, Ipp16s*
//	pDst, int dstStep, IppiSize dstRoiSize, IppiSize maskSize,
//	IppiPoint anchor) {
//	return IPPCALL(ippiFilterBox_16s_C3R)(pSrc, srcStep, pDst, dstStep, dstRoiSize, maskSize,
//		anchor);
//}
//IPPI_WRAP ippiFilterBox_AC4R(const Ipp16s* pSrc, int srcStep, Ipp16s*
//	pDst, int dstStep, IppiSize dstRoiSize, IppiSize maskSize,
//	IppiPoint anchor) {
//	return IPPCALL(ippiFilterBox_16s_AC4R)(pSrc, srcStep, pDst, dstStep, dstRoiSize, maskSize,
//		anchor);
//}
//IPPI_WRAP ippiFilterBox_C4R(const Ipp16s* pSrc, int srcStep, Ipp16s*
//	pDst, int dstStep, IppiSize dstRoiSize, IppiSize maskSize,
//	IppiPoint anchor) {
//	return IPPCALL(ippiFilterBox_16s_C4R)(pSrc, srcStep, pDst, dstStep, dstRoiSize, maskSize,
//		anchor);
//}
//IPPI_WRAP ippiFilterBox_C1R(const Ipp32f* pSrc, int srcStep, Ipp32f*
//	pDst, int dstStep, IppiSize dstRoiSize, IppiSize maskSize,
//	IppiPoint anchor) {
//	return IPPCALL(ippiFilterBox_32f_C1R)(pSrc, srcStep, pDst, dstStep, dstRoiSize, maskSize,
//		anchor);
//}
//IPPI_WRAP ippiFilterBox_C3R(const Ipp32f* pSrc, int srcStep, Ipp32f*
//	pDst, int dstStep, IppiSize dstRoiSize, IppiSize maskSize,
//	IppiPoint anchor) {
//	return IPPCALL(ippiFilterBox_32f_C3R)(pSrc, srcStep, pDst, dstStep, dstRoiSize, maskSize,
//		anchor);
//}
//IPPI_WRAP ippiFilterBox_AC4R(const Ipp32f* pSrc, int srcStep, Ipp32f*
//	pDst, int dstStep, IppiSize dstRoiSize, IppiSize maskSize,
//	IppiPoint anchor) {
//	return IPPCALL(ippiFilterBox_32f_AC4R)(pSrc, srcStep, pDst, dstStep, dstRoiSize, maskSize,
//		anchor);
//}
//IPPI_WRAP ippiFilterBox_C4R(const Ipp32f* pSrc, int srcStep, Ipp32f*
//	pDst, int dstStep, IppiSize dstRoiSize, IppiSize maskSize,
//	IppiPoint anchor) {
//	return IPPCALL(ippiFilterBox_32f_C4R)(pSrc, srcStep, pDst, dstStep, dstRoiSize, maskSize,
//		anchor);
//}
//
//IPPI_WRAP ippiFilterBox_C1R(Ipp8u* pSrcDst, int srcDstStep, IppiSize
//	roiSize, IppiSize maskSize, IppiPoint anchor) {
//	return IPPCALL(ippiFilterBox_8u_C1IR)(pSrcDst, srcDstStep, roiSize, maskSize, anchor);
//}
//IPPI_WRAP ippiFilterBox_C3R(Ipp8u* pSrcDst, int srcDstStep, IppiSize
//	roiSize, IppiSize maskSize, IppiPoint anchor) {
//	return IPPCALL(ippiFilterBox_8u_C3IR)(pSrcDst, srcDstStep, roiSize, maskSize, anchor);
//}
//IPPI_WRAP ippiFilterBox_AC4R(Ipp8u* pSrcDst, int srcDstStep, IppiSize
//	roiSize, IppiSize maskSize, IppiPoint anchor) {
//	return IPPCALL(ippiFilterBox_8u_AC4IR)(pSrcDst, srcDstStep, roiSize, maskSize, anchor);
//}
IPPI_WRAP ippiFilterBox_C4R(Ipp8u* pSrcDst, int srcDstStep, IppiSize
	roiSize, IppiSize maskSize, IppiPoint anchor) {
	return IPPCALL(ippiFilterBox_8u_C4IR)(pSrcDst, srcDstStep, roiSize, maskSize, anchor);
}

IPPI_WRAP ippiFilterBox_C1R(Ipp16s* pSrcDst, int srcDstStep, IppiSize
	roiSize, IppiSize maskSize, IppiPoint anchor) {
	return IPPCALL(ippiFilterBox_16s_C1IR)(pSrcDst, srcDstStep, roiSize, maskSize, anchor);
}
IPPI_WRAP ippiFilterBox_C3R(Ipp16s* pSrcDst, int srcDstStep, IppiSize
	roiSize, IppiSize maskSize, IppiPoint anchor) {
	return IPPCALL(ippiFilterBox_16s_C3IR)(pSrcDst, srcDstStep, roiSize, maskSize, anchor);
}
IPPI_WRAP ippiFilterBox_AC4R(Ipp16s* pSrc, int srcDstStep, IppiSize
	roiSize, IppiSize maskSize, IppiPoint anchor) {
	return IPPCALL(ippiFilterBox_16s_AC4IR)(pSrc, srcDstStep, roiSize, maskSize, anchor);
}
IPPI_WRAP ippiFilterBox_C4R(Ipp16s* pSrcDst, int srcDstStep, IppiSize
	roiSize, IppiSize maskSize, IppiPoint anchor) {
	return IPPCALL(ippiFilterBox_16s_C4IR)(pSrcDst, srcDstStep, roiSize, maskSize, anchor);
}

IPPI_WRAP ippiFilterBox_C1R(Ipp32f* pSrcDst, int srcDstStep, IppiSize
	roiSize, IppiSize maskSize, IppiPoint anchor) {
	return IPPCALL(ippiFilterBox_32f_C1IR)(pSrcDst, srcDstStep, roiSize, maskSize, anchor);
}
IPPI_WRAP ippiFilterBox_C3R(Ipp32f* pSrcDst, int srcDstStep, IppiSize
	roiSize, IppiSize maskSize, IppiPoint anchor) {
	return IPPCALL(ippiFilterBox_32f_C3IR)(pSrcDst, srcDstStep, roiSize, maskSize, anchor);
}
IPPI_WRAP ippiFilterBox_AC4R(Ipp32f* pSrcDst, int srcDstStep, IppiSize
	roiSize, IppiSize maskSize, IppiPoint anchor) {
	return IPPCALL(ippiFilterBox_32f_AC4IR)(pSrcDst, srcDstStep, roiSize, maskSize, anchor);
}
IPPI_WRAP ippiFilterBox_C4R(Ipp32f* pSrcDst, int srcDstStep, IppiSize
	roiSize, IppiSize maskSize, IppiPoint anchor) {
	return IPPCALL(ippiFilterBox_32f_C4IR)(pSrcDst, srcDstStep, roiSize, maskSize, anchor);
}
/* /////////////////////////////////////////////////////////////////////////////////////////////////
//                      Logical Operations and Shift Functions
///////////////////////////////////////////////////////////////////////////////////////////////// */
/*
//  Names:          ippiAnd, ippiAndC, ippiOr, ippiOrC, ippiXor, ippiXorC, ippiNot,
//  Purpose:        Performs corresponding bitwise logical operation between pixels of two image
//                  (AndC/OrC/XorC  - between pixel of the source image and a constant)
//
//  Names:          ippiLShiftC, ippiRShiftC
//  Purpose:        Shifts bits in each pixel value to the left and right
//  Parameters:
//   value         1) The constant value to be ANDed/ORed/XORed with each pixel of the source,
//                     constant vector for multi-channel images;
//                 2) The number of bits to shift, constant vector for multi-channel images.
//   pSrc          Pointer to the source image
//   srcStep       Step through the source image
//   pSrcDst       Pointer to the source/destination image (in-place flavors)
//   srcDstStep    Step through the source/destination image (in-place flavors)
//   pSrc1         Pointer to first source image
//   src1Step      Step through first source image
//   pSrc2         Pointer to second source image
//   src2Step      Step through second source image
//   pDst          Pointer to the destination image
//   dstStep       Step in destination image
//   roiSize       Size of the ROI
//
//  Returns:
//   ippStsNullPtrErr   One of the pointers is NULL
//   ippStsStepErr      One of the step values is less than or equal to zero
//   ippStsSizeErr      roiSize has a field with zero or negative value
//   ippStsShiftErr     Shift value is less than zero
//   ippStsNoErr        No errors
*/

IPPI_WRAP ippiAnd_C1R(const Ipp8u* pSrc1, int src1Step, const Ipp8u*
	pSrc2, int src2Step, Ipp8u* pDst, int dstStep, IppiSize
	roiSize) {
	return IPPCALL(ippiAnd_8u_C1R)(pSrc1, src1Step, pSrc2, src2Step, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiAnd_C3R(const Ipp8u* pSrc1, int src1Step, const Ipp8u*
	pSrc2, int src2Step, Ipp8u* pDst, int dstStep, IppiSize
	roiSize) {
	return IPPCALL(ippiAnd_8u_C3R)(pSrc1, src1Step, pSrc2, src2Step, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiAnd_C4R(const Ipp8u* pSrc1, int src1Step, const Ipp8u*
	pSrc2, int src2Step, Ipp8u* pDst, int dstStep, IppiSize
	roiSize) {
	return IPPCALL(ippiAnd_8u_C4R)(pSrc1, src1Step, pSrc2, src2Step, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiAnd_AC4R(const Ipp8u* pSrc1, int src1Step, const Ipp8u*
	pSrc2, int src2Step, Ipp8u* pDst, int dstStep, IppiSize
	roiSize) {
	return IPPCALL(ippiAnd_8u_AC4R)(pSrc1, src1Step, pSrc2, src2Step, pDst, dstStep, roiSize
		);
}
IPPI_WRAP ippiAnd_C1R(const Ipp8u* pSrc, int srcStep, Ipp8u* pSrcDst,
	int srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiAnd_8u_C1IR)(pSrc, srcStep, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiAnd_C3R(const Ipp8u* pSrc, int srcStep, Ipp8u* pSrcDst,
	int srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiAnd_8u_C3IR)(pSrc, srcStep, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiAnd_C4R(const Ipp8u* pSrc, int srcStep, Ipp8u* pSrcDst,
	int srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiAnd_8u_C4IR)(pSrc, srcStep, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiAnd_AC4R(const Ipp8u* pSrc, int srcStep, Ipp8u* pSrcDst,
	int srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiAnd_8u_AC4IR)(pSrc, srcStep, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiAnd_C1R(const Ipp8u* pSrc, int srcStep, Ipp8u value,
	Ipp8u* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiAndC_8u_C1R)(pSrc, srcStep, value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiAnd_C3R(const Ipp8u* pSrc, int srcStep, const Ipp8u
	value[3], Ipp8u* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiAndC_8u_C3R)(pSrc, srcStep, value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiAnd_C4R(const Ipp8u* pSrc, int srcStep, const Ipp8u
	value[4], Ipp8u* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiAndC_8u_C4R)(pSrc, srcStep, value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiAnd_AC4R(const Ipp8u* pSrc, int srcStep, const Ipp8u
	value[3], Ipp8u* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiAndC_8u_AC4R)(pSrc, srcStep, value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiAnd_C1R(Ipp8u value, Ipp8u* pSrcDst, int srcDstStep,
	IppiSize roiSize) {
	return IPPCALL(ippiAndC_8u_C1IR)(value, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiAnd_C3R(const Ipp8u value[3], Ipp8u* pSrcDst, int
	srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiAndC_8u_C3IR)(value, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiAnd_C4R(const Ipp8u value[4], Ipp8u* pSrcDst, int
	srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiAndC_8u_C4IR)(value, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiAnd_AC4R(const Ipp8u value[3], Ipp8u* pSrcDst, int
	srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiAndC_8u_AC4IR)(value, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiAnd_C1R(const Ipp16u* pSrc1, int src1Step, const Ipp16u*
	pSrc2, int src2Step, Ipp16u* pDst, int dstStep, IppiSize
	roiSize) {
	return IPPCALL(ippiAnd_16u_C1R)(pSrc1, src1Step, pSrc2, src2Step, pDst, dstStep, roiSize
		);
}
IPPI_WRAP ippiAnd_C3R(const Ipp16u* pSrc1, int src1Step, const Ipp16u*
	pSrc2, int src2Step, Ipp16u* pDst, int dstStep, IppiSize
	roiSize) {
	return IPPCALL(ippiAnd_16u_C3R)(pSrc1, src1Step, pSrc2, src2Step, pDst, dstStep, roiSize
		);
}
IPPI_WRAP ippiAnd_C4R(const Ipp16u* pSrc1, int src1Step, const Ipp16u*
	pSrc2, int src2Step, Ipp16u* pDst, int dstStep, IppiSize
	roiSize) {
	return IPPCALL(ippiAnd_16u_C4R)(pSrc1, src1Step, pSrc2, src2Step, pDst, dstStep, roiSize
		);
}
IPPI_WRAP ippiAnd_AC4R(const Ipp16u* pSrc1, int src1Step, const Ipp16u*
	pSrc2, int src2Step, Ipp16u* pDst, int dstStep, IppiSize
	roiSize) {
	return IPPCALL(ippiAnd_16u_AC4R)(pSrc1, src1Step, pSrc2, src2Step, pDst, dstStep, roiSize
		);
}
IPPI_WRAP ippiAnd_C1R(const Ipp16u* pSrc, int srcStep, Ipp16u* pSrcDst,
	int srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiAnd_16u_C1IR)(pSrc, srcStep, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiAnd_C3R(const Ipp16u* pSrc, int srcStep, Ipp16u* pSrcDst,
	int srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiAnd_16u_C3IR)(pSrc, srcStep, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiAnd_C4R(const Ipp16u* pSrc, int srcStep, Ipp16u* pSrcDst,
	int srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiAnd_16u_C4IR)(pSrc, srcStep, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiAnd_AC4R(const Ipp16u* pSrc, int srcStep, Ipp16u*
	pSrcDst, int srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiAnd_16u_AC4IR)(pSrc, srcStep, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiAnd_C1R(const Ipp16u* pSrc, int srcStep, Ipp16u value,
	Ipp16u* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiAndC_16u_C1R)(pSrc, srcStep, value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiAnd_C3R(const Ipp16u* pSrc, int srcStep, const Ipp16u
	value[3], Ipp16u* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiAndC_16u_C3R)(pSrc, srcStep, value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiAnd_C4R(const Ipp16u* pSrc, int srcStep, const Ipp16u
	value[4], Ipp16u* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiAndC_16u_C4R)(pSrc, srcStep, value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiAnd_AC4R(const Ipp16u* pSrc, int srcStep, const Ipp16u
	value[3], Ipp16u* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiAndC_16u_AC4R)(pSrc, srcStep, value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiAnd_C1R(Ipp16u value, Ipp16u* pSrcDst, int srcDstStep,
	IppiSize roiSize) {
	return IPPCALL(ippiAndC_16u_C1IR)(value, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiAnd_C3R(const Ipp16u value[3], Ipp16u* pSrcDst, int
	srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiAndC_16u_C3IR)(value, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiAnd_C4R(const Ipp16u value[4], Ipp16u* pSrcDst, int
	srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiAndC_16u_C4IR)(value, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiAnd_AC4R(const Ipp16u value[3], Ipp16u* pSrcDst, int
	srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiAndC_16u_AC4IR)(value, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiAnd_C1R(const Ipp32s* pSrc1, int src1Step, const Ipp32s*
	pSrc2, int src2Step, Ipp32s* pDst, int dstStep, IppiSize
	roiSize) {
	return IPPCALL(ippiAnd_32s_C1R)(pSrc1, src1Step, pSrc2, src2Step, pDst, dstStep, roiSize
		);
}
IPPI_WRAP ippiAnd_C3R(const Ipp32s* pSrc1, int src1Step, const Ipp32s*
	pSrc2, int src2Step, Ipp32s* pDst, int dstStep, IppiSize
	roiSize) {
	return IPPCALL(ippiAnd_32s_C3R)(pSrc1, src1Step, pSrc2, src2Step, pDst, dstStep, roiSize
		);
}
IPPI_WRAP ippiAnd_C4R(const Ipp32s* pSrc1, int src1Step, const Ipp32s*
	pSrc2, int src2Step, Ipp32s* pDst, int dstStep, IppiSize
	roiSize) {
	return IPPCALL(ippiAnd_32s_C4R)(pSrc1, src1Step, pSrc2, src2Step, pDst, dstStep, roiSize
		);
}
IPPI_WRAP ippiAnd_AC4R(const Ipp32s* pSrc1, int src1Step, const Ipp32s*
	pSrc2, int src2Step, Ipp32s* pDst, int dstStep, IppiSize
	roiSize) {
	return IPPCALL(ippiAnd_32s_AC4R)(pSrc1, src1Step, pSrc2, src2Step, pDst, dstStep, roiSize
		);
}
IPPI_WRAP ippiAnd_C1R(const Ipp32s* pSrc, int srcStep, Ipp32s* pSrcDst,
	int srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiAnd_32s_C1IR)(pSrc, srcStep, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiAnd_C3R(const Ipp32s* pSrc, int srcStep, Ipp32s* pSrcDst,
	int srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiAnd_32s_C3IR)(pSrc, srcStep, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiAnd_C4R(const Ipp32s* pSrc, int srcStep, Ipp32s* pSrcDst,
	int srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiAnd_32s_C4IR)(pSrc, srcStep, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiAnd_AC4R(const Ipp32s* pSrc, int srcStep, Ipp32s*
	pSrcDst, int srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiAnd_32s_AC4IR)(pSrc, srcStep, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiAnd_C1R(const Ipp32s* pSrc, int srcStep, Ipp32s value,
	Ipp32s* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiAndC_32s_C1R)(pSrc, srcStep, value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiAnd_C3R(const Ipp32s* pSrc, int srcStep, const Ipp32s
	value[3], Ipp32s* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiAndC_32s_C3R)(pSrc, srcStep, value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiAnd_C4R(const Ipp32s* pSrc, int srcStep, const Ipp32s
	value[4], Ipp32s* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiAndC_32s_C4R)(pSrc, srcStep, value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiAnd_AC4R(const Ipp32s* pSrc, int srcStep, const Ipp32s
	value[3], Ipp32s* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiAndC_32s_AC4R)(pSrc, srcStep, value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiAnd_C1R(Ipp32s value, Ipp32s* pSrcDst, int srcDstStep,
	IppiSize roiSize) {
	return IPPCALL(ippiAndC_32s_C1IR)(value, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiAnd_C3R(const Ipp32s value[3], Ipp32s* pSrcDst, int
	srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiAndC_32s_C3IR)(value, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiAnd_C4R(const Ipp32s value[4], Ipp32s* pSrcDst, int
	srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiAndC_32s_C4IR)(value, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiAnd_AC4R(const Ipp32s value[3], Ipp32s* pSrcDst, int
	srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiAndC_32s_AC4IR)(value, pSrcDst, srcDstStep, roiSize);
}

IPPI_WRAP ippiOr_C1R(const Ipp8u* pSrc1, int src1Step, const Ipp8u*
	pSrc2, int src2Step, Ipp8u* pDst, int dstStep, IppiSize
	roiSize) {
	return IPPCALL(ippiOr_8u_C1R)(pSrc1, src1Step, pSrc2, src2Step, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiOr_C3R(const Ipp8u* pSrc1, int src1Step, const Ipp8u*
	pSrc2, int src2Step, Ipp8u* pDst, int dstStep, IppiSize
	roiSize) {
	return IPPCALL(ippiOr_8u_C3R)(pSrc1, src1Step, pSrc2, src2Step, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiOr_C4R(const Ipp8u* pSrc1, int src1Step, const Ipp8u*
	pSrc2, int src2Step, Ipp8u* pDst, int dstStep, IppiSize
	roiSize) {
	return IPPCALL(ippiOr_8u_C4R)(pSrc1, src1Step, pSrc2, src2Step, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiOr_AC4R(const Ipp8u* pSrc1, int src1Step, const Ipp8u*
	pSrc2, int src2Step, Ipp8u* pDst, int dstStep, IppiSize
	roiSize) {
	return IPPCALL(ippiOr_8u_AC4R)(pSrc1, src1Step, pSrc2, src2Step, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiOr_C1R(const Ipp8u* pSrc, int srcStep, Ipp8u* pSrcDst,
	int srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiOr_8u_C1IR)(pSrc, srcStep, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiOr_C3R(const Ipp8u* pSrc, int srcStep, Ipp8u* pSrcDst,
	int srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiOr_8u_C3IR)(pSrc, srcStep, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiOr_C4R(const Ipp8u* pSrc, int srcStep, Ipp8u* pSrcDst,
	int srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiOr_8u_C4IR)(pSrc, srcStep, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiOr_AC4R(const Ipp8u* pSrc, int srcStep, Ipp8u* pSrcDst,
	int srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiOr_8u_AC4IR)(pSrc, srcStep, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiOr_C1R(const Ipp8u* pSrc, int srcStep, Ipp8u value,
	Ipp8u* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiOrC_8u_C1R)(pSrc, srcStep, value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiOr_C3R(const Ipp8u* pSrc, int srcStep, const Ipp8u
	value[3], Ipp8u* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiOrC_8u_C3R)(pSrc, srcStep, value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiOr_C4R(const Ipp8u* pSrc, int srcStep, const Ipp8u
	value[4], Ipp8u* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiOrC_8u_C4R)(pSrc, srcStep, value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiOr_AC4R(const Ipp8u* pSrc, int srcStep, const Ipp8u
	value[3], Ipp8u* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiOrC_8u_AC4R)(pSrc, srcStep, value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiOr_C1R(Ipp8u value, Ipp8u* pSrcDst, int srcDstStep,
	IppiSize roiSize) {
	return IPPCALL(ippiOrC_8u_C1IR)(value, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiOr_C3R(const Ipp8u value[3], Ipp8u* pSrcDst, int
	srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiOrC_8u_C3IR)(value, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiOr_C4R(const Ipp8u value[4], Ipp8u* pSrcDst, int
	srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiOrC_8u_C4IR)(value, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiOr_AC4R(const Ipp8u value[3], Ipp8u* pSrcDst, int
	srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiOrC_8u_AC4IR)(value, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiOr_C1R(const Ipp16u* pSrc1, int src1Step, const Ipp16u*
	pSrc2, int src2Step, Ipp16u* pDst, int dstStep, IppiSize
	roiSize) {
	return IPPCALL(ippiOr_16u_C1R)(pSrc1, src1Step, pSrc2, src2Step, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiOr_C3R(const Ipp16u* pSrc1, int src1Step, const Ipp16u*
	pSrc2, int src2Step, Ipp16u* pDst, int dstStep, IppiSize
	roiSize) {
	return IPPCALL(ippiOr_16u_C3R)(pSrc1, src1Step, pSrc2, src2Step, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiOr_C4R(const Ipp16u* pSrc1, int src1Step, const Ipp16u*
	pSrc2, int src2Step, Ipp16u* pDst, int dstStep, IppiSize
	roiSize) {
	return IPPCALL(ippiOr_16u_C4R)(pSrc1, src1Step, pSrc2, src2Step, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiOr_AC4R(const Ipp16u* pSrc1, int src1Step, const Ipp16u*
	pSrc2, int src2Step, Ipp16u* pDst, int dstStep, IppiSize
	roiSize) {
	return IPPCALL(ippiOr_16u_AC4R)(pSrc1, src1Step, pSrc2, src2Step, pDst, dstStep, roiSize
		);
}
IPPI_WRAP ippiOr_C1R(const Ipp16u* pSrc, int srcStep, Ipp16u* pSrcDst,
	int srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiOr_16u_C1IR)(pSrc, srcStep, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiOr_C3R(const Ipp16u* pSrc, int srcStep, Ipp16u* pSrcDst,
	int srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiOr_16u_C3IR)(pSrc, srcStep, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiOr_C4R(const Ipp16u* pSrc, int srcStep, Ipp16u* pSrcDst,
	int srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiOr_16u_C4IR)(pSrc, srcStep, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiOr_AC4R(const Ipp16u* pSrc, int srcStep, Ipp16u* pSrcDst,
	int srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiOr_16u_AC4IR)(pSrc, srcStep, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiOr_C1R(const Ipp16u* pSrc, int srcStep, Ipp16u value,
	Ipp16u* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiOrC_16u_C1R)(pSrc, srcStep, value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiOr_C3R(const Ipp16u* pSrc, int srcStep, const Ipp16u
	value[3], Ipp16u* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiOrC_16u_C3R)(pSrc, srcStep, value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiOr_C4R(const Ipp16u* pSrc, int srcStep, const Ipp16u
	value[4], Ipp16u* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiOrC_16u_C4R)(pSrc, srcStep, value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiOr_AC4R(const Ipp16u* pSrc, int srcStep, const Ipp16u
	value[3], Ipp16u* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiOrC_16u_AC4R)(pSrc, srcStep, value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiOr_C1R(Ipp16u value, Ipp16u* pSrcDst, int srcDstStep,
	IppiSize roiSize) {
	return IPPCALL(ippiOrC_16u_C1IR)(value, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiOr_C3R(const Ipp16u value[3], Ipp16u* pSrcDst, int
	srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiOrC_16u_C3IR)(value, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiOr_C4R(const Ipp16u value[4], Ipp16u* pSrcDst, int
	srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiOrC_16u_C4IR)(value, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiOr_AC4R(const Ipp16u value[3], Ipp16u* pSrcDst, int
	srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiOrC_16u_AC4IR)(value, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiOr_C1R(const Ipp32s* pSrc1, int src1Step, const Ipp32s*
	pSrc2, int src2Step, Ipp32s* pDst, int dstStep, IppiSize
	roiSize) {
	return IPPCALL(ippiOr_32s_C1R)(pSrc1, src1Step, pSrc2, src2Step, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiOr_C3R(const Ipp32s* pSrc1, int src1Step, const Ipp32s*
	pSrc2, int src2Step, Ipp32s* pDst, int dstStep, IppiSize
	roiSize) {
	return IPPCALL(ippiOr_32s_C3R)(pSrc1, src1Step, pSrc2, src2Step, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiOr_C4R(const Ipp32s* pSrc1, int src1Step, const Ipp32s*
	pSrc2, int src2Step, Ipp32s* pDst, int dstStep, IppiSize
	roiSize) {
	return IPPCALL(ippiOr_32s_C4R)(pSrc1, src1Step, pSrc2, src2Step, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiOr_AC4R(const Ipp32s* pSrc1, int src1Step, const Ipp32s*
	pSrc2, int src2Step, Ipp32s* pDst, int dstStep, IppiSize
	roiSize) {
	return IPPCALL(ippiOr_32s_AC4R)(pSrc1, src1Step, pSrc2, src2Step, pDst, dstStep, roiSize
		);
}
IPPI_WRAP ippiOr_C1R(const Ipp32s* pSrc, int srcStep, Ipp32s* pSrcDst,
	int srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiOr_32s_C1IR)(pSrc, srcStep, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiOr_C3R(const Ipp32s* pSrc, int srcStep, Ipp32s* pSrcDst,
	int srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiOr_32s_C3IR)(pSrc, srcStep, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiOr_C4R(const Ipp32s* pSrc, int srcStep, Ipp32s* pSrcDst,
	int srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiOr_32s_C4IR)(pSrc, srcStep, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiOr_AC4R(const Ipp32s* pSrc, int srcStep, Ipp32s* pSrcDst,
	int srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiOr_32s_AC4IR)(pSrc, srcStep, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiOr_C1R(const Ipp32s* pSrc, int srcStep, Ipp32s value,
	Ipp32s* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiOrC_32s_C1R)(pSrc, srcStep, value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiOr_C3R(const Ipp32s* pSrc, int srcStep, const Ipp32s
	value[3], Ipp32s* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiOrC_32s_C3R)(pSrc, srcStep, value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiOr_C4R(const Ipp32s* pSrc, int srcStep, const Ipp32s
	value[4], Ipp32s* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiOrC_32s_C4R)(pSrc, srcStep, value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiOr_AC4R(const Ipp32s* pSrc, int srcStep, const Ipp32s
	value[3], Ipp32s* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiOrC_32s_AC4R)(pSrc, srcStep, value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiOr_C1R(Ipp32s value, Ipp32s* pSrcDst, int srcDstStep,
	IppiSize roiSize) {
	return IPPCALL(ippiOrC_32s_C1IR)(value, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiOr_C3R(const Ipp32s value[3], Ipp32s* pSrcDst, int
	srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiOrC_32s_C3IR)(value, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiOr_C4R(const Ipp32s value[4], Ipp32s* pSrcDst, int
	srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiOrC_32s_C4IR)(value, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiOr_AC4R(const Ipp32s value[3], Ipp32s* pSrcDst, int
	srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiOrC_32s_AC4IR)(value, pSrcDst, srcDstStep, roiSize);
}

IPPI_WRAP ippiXor_C1R(const Ipp8u* pSrc1, int src1Step, const Ipp8u*
	pSrc2, int src2Step, Ipp8u* pDst, int dstStep, IppiSize
	roiSize) {
	return IPPCALL(ippiXor_8u_C1R)(pSrc1, src1Step, pSrc2, src2Step, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiXor_C3R(const Ipp8u* pSrc1, int src1Step, const Ipp8u*
	pSrc2, int src2Step, Ipp8u* pDst, int dstStep, IppiSize
	roiSize) {
	return IPPCALL(ippiXor_8u_C3R)(pSrc1, src1Step, pSrc2, src2Step, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiXor_C4R(const Ipp8u* pSrc1, int src1Step, const Ipp8u*
	pSrc2, int src2Step, Ipp8u* pDst, int dstStep, IppiSize
	roiSize) {
	return IPPCALL(ippiXor_8u_C4R)(pSrc1, src1Step, pSrc2, src2Step, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiXor_AC4R(const Ipp8u* pSrc1, int src1Step, const Ipp8u*
	pSrc2, int src2Step, Ipp8u* pDst, int dstStep, IppiSize
	roiSize) {
	return IPPCALL(ippiXor_8u_AC4R)(pSrc1, src1Step, pSrc2, src2Step, pDst, dstStep, roiSize
		);
}
IPPI_WRAP ippiXor_C1R(const Ipp8u* pSrc, int srcStep, Ipp8u* pSrcDst,
	int srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiXor_8u_C1IR)(pSrc, srcStep, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiXor_C3R(const Ipp8u* pSrc, int srcStep, Ipp8u* pSrcDst,
	int srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiXor_8u_C3IR)(pSrc, srcStep, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiXor_C4R(const Ipp8u* pSrc, int srcStep, Ipp8u* pSrcDst,
	int srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiXor_8u_C4IR)(pSrc, srcStep, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiXor_AC4R(const Ipp8u* pSrc, int srcStep, Ipp8u* pSrcDst,
	int srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiXor_8u_AC4IR)(pSrc, srcStep, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiXor_C1R(const Ipp8u* pSrc, int srcStep, Ipp8u value,
	Ipp8u* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiXorC_8u_C1R)(pSrc, srcStep, value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiXor_C3R(const Ipp8u* pSrc, int srcStep, const Ipp8u
	value[3], Ipp8u* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiXorC_8u_C3R)(pSrc, srcStep, value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiXor_C4R(const Ipp8u* pSrc, int srcStep, const Ipp8u
	value[4], Ipp8u* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiXorC_8u_C4R)(pSrc, srcStep, value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiXor_AC4R(const Ipp8u* pSrc, int srcStep, const Ipp8u
	value[3], Ipp8u* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiXorC_8u_AC4R)(pSrc, srcStep, value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiXor_C1R(Ipp8u value, Ipp8u* pSrcDst, int srcDstStep,
	IppiSize roiSize) {
	return IPPCALL(ippiXorC_8u_C1IR)(value, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiXor_C3R(const Ipp8u value[3], Ipp8u* pSrcDst, int
	srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiXorC_8u_C3IR)(value, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiXor_C4R(const Ipp8u value[4], Ipp8u* pSrcDst, int
	srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiXorC_8u_C4IR)(value, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiXor_AC4R(const Ipp8u value[3], Ipp8u* pSrcDst, int
	srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiXorC_8u_AC4IR)(value, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiXor_C1R(const Ipp16u* pSrc1, int src1Step, const Ipp16u*
	pSrc2, int src2Step, Ipp16u* pDst, int dstStep, IppiSize
	roiSize) {
	return IPPCALL(ippiXor_16u_C1R)(pSrc1, src1Step, pSrc2, src2Step, pDst, dstStep, roiSize
		);
}
IPPI_WRAP ippiXor_C3R(const Ipp16u* pSrc1, int src1Step, const Ipp16u*
	pSrc2, int src2Step, Ipp16u* pDst, int dstStep, IppiSize
	roiSize) {
	return IPPCALL(ippiXor_16u_C3R)(pSrc1, src1Step, pSrc2, src2Step, pDst, dstStep, roiSize
		);
}
IPPI_WRAP ippiXor_C4R(const Ipp16u* pSrc1, int src1Step, const Ipp16u*
	pSrc2, int src2Step, Ipp16u* pDst, int dstStep, IppiSize
	roiSize) {
	return IPPCALL(ippiXor_16u_C4R)(pSrc1, src1Step, pSrc2, src2Step, pDst, dstStep, roiSize
		);
}
IPPI_WRAP ippiXor_AC4R(const Ipp16u* pSrc1, int src1Step, const Ipp16u*
	pSrc2, int src2Step, Ipp16u* pDst, int dstStep, IppiSize
	roiSize) {
	return IPPCALL(ippiXor_16u_AC4R)(pSrc1, src1Step, pSrc2, src2Step, pDst, dstStep, roiSize
		);
}
IPPI_WRAP ippiXor_C1R(const Ipp16u* pSrc, int srcStep, Ipp16u* pSrcDst,
	int srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiXor_16u_C1IR)(pSrc, srcStep, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiXor_C3R(const Ipp16u* pSrc, int srcStep, Ipp16u* pSrcDst,
	int srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiXor_16u_C3IR)(pSrc, srcStep, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiXor_C4R(const Ipp16u* pSrc, int srcStep, Ipp16u* pSrcDst,
	int srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiXor_16u_C4IR)(pSrc, srcStep, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiXor_AC4R(const Ipp16u* pSrc, int srcStep, Ipp16u*
	pSrcDst, int srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiXor_16u_AC4IR)(pSrc, srcStep, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiXor_C1R(const Ipp16u* pSrc, int srcStep, Ipp16u value,
	Ipp16u* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiXorC_16u_C1R)(pSrc, srcStep, value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiXor_C3R(const Ipp16u* pSrc, int srcStep, const Ipp16u
	value[3], Ipp16u* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiXorC_16u_C3R)(pSrc, srcStep, value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiXor_C4R(const Ipp16u* pSrc, int srcStep, const Ipp16u
	value[4], Ipp16u* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiXorC_16u_C4R)(pSrc, srcStep, value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiXor_AC4R(const Ipp16u* pSrc, int srcStep, const Ipp16u
	value[3], Ipp16u* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiXorC_16u_AC4R)(pSrc, srcStep, value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiXor_C1R(Ipp16u value, Ipp16u* pSrcDst, int srcDstStep,
	IppiSize roiSize) {
	return IPPCALL(ippiXorC_16u_C1IR)(value, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiXor_C3R(const Ipp16u value[3], Ipp16u* pSrcDst, int
	srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiXorC_16u_C3IR)(value, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiXor_C4R(const Ipp16u value[4], Ipp16u* pSrcDst, int
	srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiXorC_16u_C4IR)(value, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiXor_AC4R(const Ipp16u value[3], Ipp16u* pSrcDst, int
	srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiXorC_16u_AC4IR)(value, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiXor_C1R(const Ipp32s* pSrc1, int src1Step, const Ipp32s*
	pSrc2, int src2Step, Ipp32s* pDst, int dstStep, IppiSize
	roiSize) {
	return IPPCALL(ippiXor_32s_C1R)(pSrc1, src1Step, pSrc2, src2Step, pDst, dstStep, roiSize
		);
}
IPPI_WRAP ippiXor_C3R(const Ipp32s* pSrc1, int src1Step, const Ipp32s*
	pSrc2, int src2Step, Ipp32s* pDst, int dstStep, IppiSize
	roiSize) {
	return IPPCALL(ippiXor_32s_C3R)(pSrc1, src1Step, pSrc2, src2Step, pDst, dstStep, roiSize
		);
}
IPPI_WRAP ippiXor_C4R(const Ipp32s* pSrc1, int src1Step, const Ipp32s*
	pSrc2, int src2Step, Ipp32s* pDst, int dstStep, IppiSize
	roiSize) {
	return IPPCALL(ippiXor_32s_C4R)(pSrc1, src1Step, pSrc2, src2Step, pDst, dstStep, roiSize
		);
}
IPPI_WRAP ippiXor_AC4R(const Ipp32s* pSrc1, int src1Step, const Ipp32s*
	pSrc2, int src2Step, Ipp32s* pDst, int dstStep, IppiSize
	roiSize) {
	return IPPCALL(ippiXor_32s_AC4R)(pSrc1, src1Step, pSrc2, src2Step, pDst, dstStep, roiSize
		);
}
IPPI_WRAP ippiXor_C1R(const Ipp32s* pSrc, int srcStep, Ipp32s* pSrcDst,
	int srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiXor_32s_C1IR)(pSrc, srcStep, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiXor_C3R(const Ipp32s* pSrc, int srcStep, Ipp32s* pSrcDst,
	int srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiXor_32s_C3IR)(pSrc, srcStep, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiXor_C4R(const Ipp32s* pSrc, int srcStep, Ipp32s* pSrcDst,
	int srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiXor_32s_C4IR)(pSrc, srcStep, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiXor_AC4R(const Ipp32s* pSrc, int srcStep, Ipp32s*
	pSrcDst, int srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiXor_32s_AC4IR)(pSrc, srcStep, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiXor_C1R(const Ipp32s* pSrc, int srcStep, Ipp32s value,
	Ipp32s* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiXorC_32s_C1R)(pSrc, srcStep, value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiXor_C3R(const Ipp32s* pSrc, int srcStep, const Ipp32s
	value[3], Ipp32s* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiXorC_32s_C3R)(pSrc, srcStep, value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiXor_C4R(const Ipp32s* pSrc, int srcStep, const Ipp32s
	value[4], Ipp32s* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiXorC_32s_C4R)(pSrc, srcStep, value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiXor_AC4R(const Ipp32s* pSrc, int srcStep, const Ipp32s
	value[3], Ipp32s* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiXorC_32s_AC4R)(pSrc, srcStep, value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiXor_C1R(Ipp32s value, Ipp32s* pSrcDst, int srcDstStep,
	IppiSize roiSize) {
	return IPPCALL(ippiXorC_32s_C1IR)(value, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiXor_C3R(const Ipp32s value[3], Ipp32s* pSrcDst, int
	srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiXorC_32s_C3IR)(value, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiXor_C4R(const Ipp32s value[4], Ipp32s* pSrcDst, int
	srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiXorC_32s_C4IR)(value, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiXor_AC4R(const Ipp32s value[3], Ipp32s* pSrcDst, int
	srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiXorC_32s_AC4IR)(value, pSrcDst, srcDstStep, roiSize);
}

IPPI_WRAP ippiNot_C1R(const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int
	dstStep, IppiSize roiSize) {
	return IPPCALL(ippiNot_8u_C1R)(pSrc, srcStep, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiNot_C3R(const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int
	dstStep, IppiSize roiSize) {
	return IPPCALL(ippiNot_8u_C3R)(pSrc, srcStep, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiNot_C4R(const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int
	dstStep, IppiSize roiSize) {
	return IPPCALL(ippiNot_8u_C4R)(pSrc, srcStep, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiNot_AC4R(const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int
	dstStep, IppiSize roiSize) {
	return IPPCALL(ippiNot_8u_AC4R)(pSrc, srcStep, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiNot_C1R(Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize)
{
	return IPPCALL(ippiNot_8u_C1IR)(pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiNot_C3R(Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize)
{
	return IPPCALL(ippiNot_8u_C3IR)(pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiNot_C4R(Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize)
{
	return IPPCALL(ippiNot_8u_C4IR)(pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiNot_AC4R(Ipp8u* pSrcDst, int srcDstStep, IppiSize
	roiSize) {
	return IPPCALL(ippiNot_8u_AC4IR)(pSrcDst, srcDstStep, roiSize);
}

IPPI_WRAP ippiLShift_C1R(const Ipp8u* pSrc, int srcStep, Ipp32u value,
	Ipp8u* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiLShiftC_8u_C1R)(pSrc, srcStep, value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiLShift_C3R(const Ipp8u* pSrc, int srcStep, const Ipp32u
	value[3], Ipp8u* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiLShiftC_8u_C3R)(pSrc, srcStep, value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiLShift_C4R(const Ipp8u* pSrc, int srcStep, const Ipp32u
	value[4], Ipp8u* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiLShiftC_8u_C4R)(pSrc, srcStep, value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiLShift_AC4R(const Ipp8u* pSrc, int srcStep, const Ipp32u
	value[3], Ipp8u* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiLShiftC_8u_AC4R)(pSrc, srcStep, value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiLShift_C1R(Ipp32u value, Ipp8u* pSrcDst, int srcDstStep,
	IppiSize roiSize) {
	return IPPCALL(ippiLShiftC_8u_C1IR)(value, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiLShift_C3R(const Ipp32u value[3], Ipp8u* pSrcDst, int
	srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiLShiftC_8u_C3IR)(value, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiLShift_C4R(const Ipp32u value[4], Ipp8u* pSrcDst, int
	srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiLShiftC_8u_C4IR)(value, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiLShift_AC4R(const Ipp32u value[3], Ipp8u* pSrcDst, int
	srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiLShiftC_8u_AC4IR)(value, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiLShift_C1R(const Ipp16u* pSrc, int srcStep, Ipp32u value,
	Ipp16u* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiLShiftC_16u_C1R)(pSrc, srcStep, value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiLShift_C3R(const Ipp16u* pSrc, int srcStep, const Ipp32u
	value[3], Ipp16u* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiLShiftC_16u_C3R)(pSrc, srcStep, value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiLShift_C4R(const Ipp16u* pSrc, int srcStep, const Ipp32u
	value[4], Ipp16u* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiLShiftC_16u_C4R)(pSrc, srcStep, value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiLShift_AC4R(const Ipp16u* pSrc, int srcStep, const Ipp32u
	value[3], Ipp16u* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiLShiftC_16u_AC4R)(pSrc, srcStep, value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiLShift_C1R(Ipp32u value, Ipp16u* pSrcDst, int srcDstStep,
	IppiSize roiSize) {
	return IPPCALL(ippiLShiftC_16u_C1IR)(value, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiLShift_C3R(const Ipp32u value[3], Ipp16u* pSrcDst, int
	srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiLShiftC_16u_C3IR)(value, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiLShift_C4R(const Ipp32u value[4], Ipp16u* pSrcDst, int
	srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiLShiftC_16u_C4IR)(value, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiLShift_AC4R(const Ipp32u value[3], Ipp16u* pSrcDst, int
	srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiLShiftC_16u_AC4IR)(value, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiLShift_C1R(const Ipp32s* pSrc, int srcStep, Ipp32u value,
	Ipp32s* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiLShiftC_32s_C1R)(pSrc, srcStep, value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiLShift_C3R(const Ipp32s* pSrc, int srcStep, const Ipp32u
	value[3], Ipp32s* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiLShiftC_32s_C3R)(pSrc, srcStep, value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiLShift_C4R(const Ipp32s* pSrc, int srcStep, const Ipp32u
	value[4], Ipp32s* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiLShiftC_32s_C4R)(pSrc, srcStep, value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiLShift_AC4R(const Ipp32s* pSrc, int srcStep, const Ipp32u
	value[3], Ipp32s* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiLShiftC_32s_AC4R)(pSrc, srcStep, value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiLShift_C1R(Ipp32u value, Ipp32s* pSrcDst, int srcDstStep,
	IppiSize roiSize) {
	return IPPCALL(ippiLShiftC_32s_C1IR)(value, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiLShift_C3R(const Ipp32u value[3], Ipp32s* pSrcDst, int
	srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiLShiftC_32s_C3IR)(value, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiLShift_C4R(const Ipp32u value[4], Ipp32s* pSrcDst, int
	srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiLShiftC_32s_C4IR)(value, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiLShift_AC4R(const Ipp32u value[3], Ipp32s* pSrcDst, int
	srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiLShiftC_32s_AC4IR)(value, pSrcDst, srcDstStep, roiSize);
}

IPPI_WRAP ippiRShift_C1R(const Ipp8u* pSrc, int srcStep, Ipp32u value,
	Ipp8u* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiRShiftC_8u_C1R)(pSrc, srcStep, value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiRShift_C3R(const Ipp8u* pSrc, int srcStep, const Ipp32u
	value[3], Ipp8u* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiRShiftC_8u_C3R)(pSrc, srcStep, value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiRShift_C4R(const Ipp8u* pSrc, int srcStep, const Ipp32u
	value[4], Ipp8u* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiRShiftC_8u_C4R)(pSrc, srcStep, value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiRShift_AC4R(const Ipp8u* pSrc, int srcStep, const Ipp32u
	value[3], Ipp8u* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiRShiftC_8u_AC4R)(pSrc, srcStep, value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiRShift_C1R(Ipp32u value, Ipp8u* pSrcDst, int srcDstStep,
	IppiSize roiSize) {
	return IPPCALL(ippiRShiftC_8u_C1IR)(value, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiRShift_C3R(const Ipp32u value[3], Ipp8u* pSrcDst, int
	srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiRShiftC_8u_C3IR)(value, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiRShift_C4R(const Ipp32u value[4], Ipp8u* pSrcDst, int
	srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiRShiftC_8u_C4IR)(value, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiRShift_AC4R(const Ipp32u value[3], Ipp8u* pSrcDst, int
	srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiRShiftC_8u_AC4IR)(value, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiRShift_C1R(const Ipp8s* pSrc, int srcStep, Ipp32u value,
	Ipp8s* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiRShiftC_8s_C1R)(pSrc, srcStep, value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiRShift_C3R(const Ipp8s* pSrc, int srcStep, const Ipp32u
	value[3], Ipp8s* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiRShiftC_8s_C3R)(pSrc, srcStep, value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiRShift_C4R(const Ipp8s* pSrc, int srcStep, const Ipp32u
	value[4], Ipp8s* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiRShiftC_8s_C4R)(pSrc, srcStep, value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiRShift_AC4R(const Ipp8s* pSrc, int srcStep, const Ipp32u
	value[3], Ipp8s* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiRShiftC_8s_AC4R)(pSrc, srcStep, value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiRShift_C1R(Ipp32u value, Ipp8s* pSrcDst, int srcDstStep,
	IppiSize roiSize) {
	return IPPCALL(ippiRShiftC_8s_C1IR)(value, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiRShift_C3R(const Ipp32u value[3], Ipp8s* pSrcDst, int
	srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiRShiftC_8s_C3IR)(value, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiRShift_C4R(const Ipp32u value[4], Ipp8s* pSrcDst, int
	srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiRShiftC_8s_C4IR)(value, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiRShift_AC4R(const Ipp32u value[3], Ipp8s* pSrcDst, int
	srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiRShiftC_8s_AC4IR)(value, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiRShift_C1R(const Ipp16u* pSrc, int srcStep, Ipp32u value,
	Ipp16u* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiRShiftC_16u_C1R)(pSrc, srcStep, value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiRShift_C3R(const Ipp16u* pSrc, int srcStep, const Ipp32u
	value[3], Ipp16u* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiRShiftC_16u_C3R)(pSrc, srcStep, value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiRShift_C4R(const Ipp16u* pSrc, int srcStep, const Ipp32u
	value[4], Ipp16u* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiRShiftC_16u_C4R)(pSrc, srcStep, value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiRShift_AC4R(const Ipp16u* pSrc, int srcStep, const Ipp32u
	value[3], Ipp16u* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiRShiftC_16u_AC4R)(pSrc, srcStep, value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiRShift_C1R(Ipp32u value, Ipp16u* pSrcDst, int srcDstStep,
	IppiSize roiSize) {
	return IPPCALL(ippiRShiftC_16u_C1IR)(value, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiRShift_C3R(const Ipp32u value[3], Ipp16u* pSrcDst, int
	srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiRShiftC_16u_C3IR)(value, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiRShift_C4R(const Ipp32u value[4], Ipp16u* pSrcDst, int
	srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiRShiftC_16u_C4IR)(value, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiRShift_AC4R(const Ipp32u value[3], Ipp16u* pSrcDst, int
	srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiRShiftC_16u_AC4IR)(value, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiRShift_C1R(const Ipp16s* pSrc, int srcStep, Ipp32u value,
	Ipp16s* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiRShiftC_16s_C1R)(pSrc, srcStep, value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiRShift_C3R(const Ipp16s* pSrc, int srcStep, const Ipp32u
	value[3], Ipp16s* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiRShiftC_16s_C3R)(pSrc, srcStep, value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiRShift_C4R(const Ipp16s* pSrc, int srcStep, const Ipp32u
	value[4], Ipp16s* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiRShiftC_16s_C4R)(pSrc, srcStep, value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiRShift_AC4R(const Ipp16s* pSrc, int srcStep, const Ipp32u
	value[3], Ipp16s* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiRShiftC_16s_AC4R)(pSrc, srcStep, value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiRShift_C1R(Ipp32u value, Ipp16s* pSrcDst, int srcDstStep,
	IppiSize roiSize) {
	return IPPCALL(ippiRShiftC_16s_C1IR)(value, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiRShift_C3R(const Ipp32u value[3], Ipp16s* pSrcDst, int
	srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiRShiftC_16s_C3IR)(value, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiRShift_C4R(const Ipp32u value[4], Ipp16s* pSrcDst, int
	srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiRShiftC_16s_C4IR)(value, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiRShift_AC4R(const Ipp32u value[3], Ipp16s* pSrcDst, int
	srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiRShiftC_16s_AC4IR)(value, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiRShift_C1R(const Ipp32s* pSrc, int srcStep, Ipp32u value,
	Ipp32s* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiRShiftC_32s_C1R)(pSrc, srcStep, value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiRShift_C3R(const Ipp32s* pSrc, int srcStep, const Ipp32u
	value[3], Ipp32s* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiRShiftC_32s_C3R)(pSrc, srcStep, value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiRShift_C4R(const Ipp32s* pSrc, int srcStep, const Ipp32u
	value[4], Ipp32s* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiRShiftC_32s_C4R)(pSrc, srcStep, value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiRShift_AC4R(const Ipp32s* pSrc, int srcStep, const Ipp32u
	value[3], Ipp32s* pDst, int dstStep, IppiSize roiSize) {
	return IPPCALL(ippiRShiftC_32s_AC4R)(pSrc, srcStep, value, pDst, dstStep, roiSize);
}
IPPI_WRAP ippiRShift_C1R(Ipp32u value, Ipp32s* pSrcDst, int srcDstStep,
	IppiSize roiSize) {
	return IPPCALL(ippiRShiftC_32s_C1IR)(value, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiRShift_C3R(const Ipp32u value[3], Ipp32s* pSrcDst, int
	srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiRShiftC_32s_C3IR)(value, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiRShift_C4R(const Ipp32u value[4], Ipp32s* pSrcDst, int
	srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiRShiftC_32s_C4IR)(value, pSrcDst, srcDstStep, roiSize);
}
IPPI_WRAP ippiRShift_AC4R(const Ipp32u value[3], Ipp32s* pSrcDst, int
	srcDstStep, IppiSize roiSize) {
	return IPPCALL(ippiRShiftC_32s_AC4IR)(value, pSrcDst, srcDstStep, roiSize);
}

	} // namespace ipp_cpp 
} // namespace ipp 
