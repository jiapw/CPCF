#include "../rt/string_type_ops.h"
#include "kernel.h"


namespace os
{

DWORD crc32c(LPCVOID data_in, SIZE_T length, DWORD crc)
{
	static const DWORD crc32c_table[256] = {
		0x00000000L, 0xF26B8303L, 0xE13B70F7L, 0x1350F3F4L,	0xC79A971FL, 0x35F1141CL, 0x26A1E7E8L, 0xD4CA64EBL,	0x8AD958CFL, 0x78B2DBCCL, 0x6BE22838L, 0x9989AB3BL,
		0x4D43CFD0L, 0xBF284CD3L, 0xAC78BF27L, 0x5E133C24L,	0x105EC76FL, 0xE235446CL, 0xF165B798L, 0x030E349BL,	0xD7C45070L, 0x25AFD373L, 0x36FF2087L, 0xC494A384L,
		0x9A879FA0L, 0x68EC1CA3L, 0x7BBCEF57L, 0x89D76C54L,	0x5D1D08BFL, 0xAF768BBCL, 0xBC267848L, 0x4E4DFB4BL,	0x20BD8EDEL, 0xD2D60DDDL, 0xC186FE29L, 0x33ED7D2AL,
		0xE72719C1L, 0x154C9AC2L, 0x061C6936L, 0xF477EA35L,	0xAA64D611L, 0x580F5512L, 0x4B5FA6E6L, 0xB93425E5L,	0x6DFE410EL, 0x9F95C20DL, 0x8CC531F9L, 0x7EAEB2FAL,
		0x30E349B1L, 0xC288CAB2L, 0xD1D83946L, 0x23B3BA45L,	0xF779DEAEL, 0x05125DADL, 0x1642AE59L, 0xE4292D5AL,	0xBA3A117EL, 0x4851927DL, 0x5B016189L, 0xA96AE28AL,
		0x7DA08661L, 0x8FCB0562L, 0x9C9BF696L, 0x6EF07595L,	0x417B1DBCL, 0xB3109EBFL, 0xA0406D4BL, 0x522BEE48L,	0x86E18AA3L, 0x748A09A0L, 0x67DAFA54L, 0x95B17957L,
		0xCBA24573L, 0x39C9C670L, 0x2A993584L, 0xD8F2B687L,	0x0C38D26CL, 0xFE53516FL, 0xED03A29BL, 0x1F682198L,	0x5125DAD3L, 0xA34E59D0L, 0xB01EAA24L, 0x42752927L,
		0x96BF4DCCL, 0x64D4CECFL, 0x77843D3BL, 0x85EFBE38L,	0xDBFC821CL, 0x2997011FL, 0x3AC7F2EBL, 0xC8AC71E8L,	0x1C661503L, 0xEE0D9600L, 0xFD5D65F4L, 0x0F36E6F7L,
		0x61C69362L, 0x93AD1061L, 0x80FDE395L, 0x72966096L,	0xA65C047DL, 0x5437877EL, 0x4767748AL, 0xB50CF789L,	0xEB1FCBADL, 0x197448AEL, 0x0A24BB5AL, 0xF84F3859L,
		0x2C855CB2L, 0xDEEEDFB1L, 0xCDBE2C45L, 0x3FD5AF46L,	0x7198540DL, 0x83F3D70EL, 0x90A324FAL, 0x62C8A7F9L,	0xB602C312L, 0x44694011L, 0x5739B3E5L, 0xA55230E6L,
		0xFB410CC2L, 0x092A8FC1L, 0x1A7A7C35L, 0xE811FF36L,	0x3CDB9BDDL, 0xCEB018DEL, 0xDDE0EB2AL, 0x2F8B6829L,	0x82F63B78L, 0x709DB87BL, 0x63CD4B8FL, 0x91A6C88CL,
		0x456CAC67L, 0xB7072F64L, 0xA457DC90L, 0x563C5F93L,	0x082F63B7L, 0xFA44E0B4L, 0xE9141340L, 0x1B7F9043L,	0xCFB5F4A8L, 0x3DDE77ABL, 0x2E8E845FL, 0xDCE5075CL,
		0x92A8FC17L, 0x60C37F14L, 0x73938CE0L, 0x81F80FE3L,	0x55326B08L, 0xA759E80BL, 0xB4091BFFL, 0x466298FCL,	0x1871A4D8L, 0xEA1A27DBL, 0xF94AD42FL, 0x0B21572CL,
		0xDFEB33C7L, 0x2D80B0C4L, 0x3ED04330L, 0xCCBBC033L,	0xA24BB5A6L, 0x502036A5L, 0x4370C551L, 0xB11B4652L,	0x65D122B9L, 0x97BAA1BAL, 0x84EA524EL, 0x7681D14DL,
		0x2892ED69L, 0xDAF96E6AL, 0xC9A99D9EL, 0x3BC21E9DL,	0xEF087A76L, 0x1D63F975L, 0x0E330A81L, 0xFC588982L,	0xB21572C9L, 0x407EF1CAL, 0x532E023EL, 0xA145813DL,
		0x758FE5D6L, 0x87E466D5L, 0x94B49521L, 0x66DF1622L,	0x38CC2A06L, 0xCAA7A905L, 0xD9F75AF1L, 0x2B9CD9F2L,	0xFF56BD19L, 0x0D3D3E1AL, 0x1E6DCDEEL, 0xEC064EEDL,
		0xC38D26C4L, 0x31E6A5C7L, 0x22B65633L, 0xD0DDD530L,	0x0417B1DBL, 0xF67C32D8L, 0xE52CC12CL, 0x1747422FL,	0x49547E0BL, 0xBB3FFD08L, 0xA86F0EFCL, 0x5A048DFFL,
		0x8ECEE914L, 0x7CA56A17L, 0x6FF599E3L, 0x9D9E1AE0L,	0xD3D3E1ABL, 0x21B862A8L, 0x32E8915CL, 0xC083125FL,	0x144976B4L, 0xE622F5B7L, 0xF5720643L, 0x07198540L,
		0x590AB964L, 0xAB613A67L, 0xB831C993L, 0x4A5A4A90L,	0x9E902E7BL, 0x6CFBAD78L, 0x7FAB5E8CL, 0x8DC0DD8FL,	0xE330A81AL, 0x115B2B19L, 0x020BD8EDL, 0xF0605BEEL,
		0x24AA3F05L, 0xD6C1BC06L, 0xC5914FF2L, 0x37FACCF1L,	0x69E9F0D5L, 0x9B8273D6L, 0x88D28022L, 0x7AB90321L,	0xAE7367CAL, 0x5C18E4C9L, 0x4F48173DL, 0xBD23943EL,
		0xF36E6F75L, 0x0105EC76L, 0x12551F82L, 0xE03E9C81L,	0x34F4F86AL, 0xC69F7B69L, 0xD5CF889DL, 0x27A40B9EL,	0x79B737BAL, 0x8BDCB4B9L, 0x988C474DL, 0x6AE7C44EL,
		0xBE2DA0A5L, 0x4C4623A6L, 0x5F16D052L, 0xAD7D5351L
	};

	// fomulated as in http://www.rasterbar.com/products/libtorrent/dht_sec.html
	LPBYTE data = (LPBYTE)data_in;

	crc = ~crc;
    while (length--)
         crc = crc32c_table[(crc ^ *data++) & 0xFFL] ^ (crc >> 8);

    return ~crc;
}

namespace _details
{
struct __crc64_init
{	
	ULONGLONG __crc64_table[256];
	__crc64_init()
	{
		ULONGLONG poly = 0xC96C5795D7870F42ULL;

		for(INT i=0; i<256; ++i)
		{
    		ULONGLONG crc = i;

    		for(UINT j=0; j<8; ++j)
    		{
				// is current coefficient set?
    			if(crc & 1)
				{
					// yes, then assume it gets zero'd (by implied x^64 coefficient of dividend)
					crc >>= 1;
    
					// and add rest of the divisor
    				crc ^= poly;
				}
    			else
    			{
    				// no? then move to next coefficient
    				crc >>= 1;
				}
    		}
    
			__crc64_table[i] = crc;
		}
	}
};
} // namespace _details

ULONGLONG crc64(LPCVOID stream_in, SIZE_T n, ULONGLONG crc)
{
	static _details::__crc64_init _crc64_init_run;

	LPCBYTE stream = (LPCBYTE)stream_in;
	LPCBYTE end = stream + n;
    while(stream < end)
    {
        BYTE index = (BYTE)((*stream) ^ crc);
        ULONGLONG lookup = _crc64_init_run.__crc64_table[index];

        crc >>= 8;
        crc ^= lookup;
		stream ++;
    }

    return crc;
}

SIZE_T UTF8EncodeLength(LPCU16CHAR pIn, SIZE_T len)	// number of char
{
	SIZE_T outlen = 0;
	for(SIZE_T i=0;i<len;i++)
	{
		WORD c = pIn[i];
		if(c <= 0x7f)
		{	outlen++;	}
		else if(c > 0x7ff)
		{	outlen+=3;	}
		else
		{	outlen+=2;	}
	}
	return outlen;
}

SIZE_T UTF8Encode(LPCU16CHAR pIn, SIZE_T len, LPSTR pOut)
{
	LPSTR p = pOut;
	for(SIZE_T i=0;i<len;i++)
	{
		WORD c = pIn[i];
		if(c <= 0x7f)
		{	*p = (char)c;
			p++;
		}
		else if(c > 0x7ff)	// 1110xxxx 	10xxxxxx 	10xxxxxx
		{	*((DWORD*)p) = 0x8080e0 | ((c>>12)&0xf) | ((c<<2)&0x3f00) | ((0x3f&c)<<16);
			p+=3;
		}
		else	// 110xxxxx 	10xxxxxx
		{	*((WORD*)p) = 0x80c0 | ((c>>6)&0x1f) | ((0x3f&c)<<8);
			p+=2;
		}
	}
	return (UINT)(p - pOut);
}

// UTF8 to UTF16
SIZE_T UTF8DecodeLength(LPCSTR pIn, SIZE_T len)	// number of wchar
{
	SIZE_T outlen = 0;
	for(SIZE_T i=0;i<len;outlen++)
	{
		BYTE c = pIn[i];
		if(c <= 0x7f)
		{	i++;
		}
		else if((c&0xf0) == 0xe0)	// 1110xxxx 	10xxxxxx 	10xxxxxx
		{	i+=3;
		}
		else if((c&0xe0) == 0xc0)	// 110xxxxx 	10xxxxxx
		{	i+=2;
		}
		else i++;	// skip
	}
	return outlen;
}

SIZE_T UTF8ByteOffset(LPCSTR pIn, SIZE_T len, SIZE_T num_of_utf8_char) // counting number of utf8 chatactors
{
	SIZE_T outlen = 0;
	SIZE_T i=0;
	for(;outlen<num_of_utf8_char;outlen++)
	{
		if(i>=len)return 0;

		BYTE c = pIn[i];
		if(c <= 0x7f)
		{	i++;
		}
		else if((c&0xf0) == 0xe0)	// 1110xxxx 	10xxxxxx 	10xxxxxx
		{	i+=3;
		}
		else if((c&0xe0) == 0xc0)	// 110xxxxx 	10xxxxxx
		{	i+=2;
		}
		else i++;	// skip
	}
	return i;
}

U16CHAR	UTF8Decode(LPCSTR& p)
{
	BYTE c = p[0];
	if(c <= 0x7f)
	{	
		p++;
		return c;
	}
	
	U16CHAR ret;
	if((c&0xf0) == 0xe0)	// 1110xxxx 	10xxxxxx 	10xxxxxx
	{	
		ret = ((c&0xf)<<12) | ((p[1]&0x3f)<<6) | (p[2]&0x3f);
		p+=3;
	}
	else if((c&0xe0) == 0xc0)	// 110xxxxx 	10xxxxxx
	{	
		ret = ((c&0x1f)<<6) | (p[1]&0x3f);
		p+=2;
	}
	else
	{
		p+=4;
		return '?'; // Unicode BMP
	}

	return ret;
}

SIZE_T UTF8Decode(LPCSTR pIn, SIZE_T len, LPU16CHAR pOut)		// number of wchar
{
	LPU16CHAR p = pOut;
	for(SIZE_T i=0;i<len;p++)
	{
		BYTE c = pIn[i];
		if(c <= 0x7f)
		{	*p = c;
			i++;
		}
		else if((c&0xf0) == 0xe0)	// 1110xxxx 	10xxxxxx 	10xxxxxx
		{	*p = ((c&0xf)<<12) | ((pIn[i+1]&0x3f)<<6) | (pIn[i+2]&0x3f);
			i+=3;
		}
		else if((c&0xe0) == 0xc0)	// 110xxxxx 	10xxxxxx
		{	*p = ((c&0x1f)<<6) | (pIn[i+1]&0x3f);
			i+=2;
		}
		else i++;
	}
	return (UINT)(p - pOut);
}

SSIZE_T UTF8EncodeLengthMax(LPCVOID pIn, SIZE_T len, const rt::String_Ref& charset_name, DWORD* pCharsetIndex)
{
	ASSERT(pCharsetIndex);

	if(charset_name.StartsWith(rt::SS("iso-8859-")))
	{	UINT latin_index;
		charset_name.SubStr(9).ToNumber(latin_index);
		if(latin_index <= 10)
		{
			*pCharsetIndex = CHARSET_LATIN_BASE + latin_index;
			return len*2;
		}
		else
		{
			return -1;
		}
	}
	else if(charset_name == rt::SS("shift-jis"))
	{	*pCharsetIndex = CHARSET_SHIFT_JIS;
		return (len+1)/2*3;
	}
	else if(charset_name.StartsWith(rt::SS("gb")))
	{
		UINT gb_num;
		charset_name.SubStr(2).ToNumber(gb_num);
		switch(gb_num)
		{
		case 2312: *pCharsetIndex = CHARSET_GB_2312; break;
		case 18030:*pCharsetIndex = CHARSET_GB_18030; break;
		default: return -1;
		}
		return (len+1)/2*3; 
	}
	else if(charset_name == rt::SS("big5"))
	{	*pCharsetIndex = CHARSET_BIG5;
		return (len+1)/2*3;
	}
	else if(charset_name.StartsWith("ks") || charset_name == rt::SS("korean"))
	{
		*pCharsetIndex = CHARSET_KOREAN;
		return (len+1)/2*3;
	}
	else if(charset_name == rt::SS("utf-16"))
	{
		*pCharsetIndex = CHARSET_UTF_16;
		return (len+1)/2*3;
	}
	else if(charset_name == rt::SS("utf-8"))
	{
		*pCharsetIndex = CHARSET_UTF_8;
		return len;
	}
	else if(charset_name.StartsWith("koi8-r"))
	{
		*pCharsetIndex = CHARSET_KOI8_R;
		return len*2;
	}
	else if(charset_name.StartsWith("koi8-u"))
	{
		*pCharsetIndex = CHARSET_KOI8_U;
		return len*2;
	}
	else
	{	return -1;
	}
}

SIZE_T UTF8Encode(LPCVOID pIn, SIZE_T len, LPSTR pOut, DWORD charset_index)
{
	if(charset_index == CHARSET_UTF_8)
	{	memcpy(pOut, pIn, len);
		pOut[len] = 0;
		return len;
	}
	if(charset_index == CHARSET_UTF_16)
	{	SIZE_T outlen = UTF8Encode((LPCU16CHAR)pIn, len/2, pOut);
		pOut[outlen] = 0;
		return outlen;
	}

#ifdef PLATFORM_WIN
	LPU16CHAR buf;
	rt::Buffer<U16CHAR>	_temp;
	if(len*2 > 64*1024)
	{	// allocate on heap
		_temp.SetSize(len);
		buf = _temp;
	}
	else
	{	// allocate on stack
		buf = (LPU16CHAR)alloca(2*len);
	}
	int outlen = MultiByteToWideChar(charset_index, 0, (LPCCH)pIn, (int)len, buf, (int)len);
	return UTF8Encode(buf, outlen, pOut);
#else
	ASSERT(0);
	return 0;
#endif
}

namespace _details
{
INLFUNC bool _isb64(int c)
{
	return	(c>='a' && c<='z') ||
			(c>='A' && c<='Z') ||
			(c>='0' && c<='9') ||
			c == '+' ||
			c == '/';
}
} // namespace _details


bool Base64Encode(const rt::String_Ref&in, rt::String& out)
{
	if(!out.SetLength(Base64EncodeLength(in.GetLength())))
		return false;

	Base64Encode(out.Begin(), in.Begin(), in.GetLength());
	return true;
}

bool Base64Decode(const rt::String_Ref&in, rt::String& out)
{
	out.SetLength(Base64DecodeLength(in.Begin(), in.GetLength()));
	SIZE_T actual_len = out.GetLength();
	bool ret = Base64Decode(out.Begin(), &actual_len, in.Begin(), in.GetLength());
	out.SetLength(actual_len);
	return ret;
}

SIZE_T Base64EncodeLength(SIZE_T len)
{
	return (len+2)/3*4;
}

void Base64Encode(LPSTR pBase64Out,LPCVOID pData, SIZE_T data_len)
{
	static const char table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	
	LPCBYTE p = (LPCBYTE)pData;
	for(;data_len>=3;data_len-=3,p+=3,pBase64Out+=4)
	{
		pBase64Out[0] = table[p[0]>>2];
		pBase64Out[1] = table[((p[0]&0x3)<<4)|(p[1]>>4)];
		pBase64Out[2] = table[((p[1]&0xf)<<2)|(p[2]>>6)];
		pBase64Out[3] = table[p[2]&0x3f];
	}

	if(data_len == 1)
	{
		pBase64Out[0] = table[p[0]>>2];
		pBase64Out[1] = table[((p[0]&0x3)<<4)];
		pBase64Out[2] = '=';
		pBase64Out[3] = '=';
		//pBase64Out[4] = '\0';
	}
	else if(data_len == 2)
	{
		ASSERT(data_len == 2);
		pBase64Out[0] = table[p[0]>>2];
		pBase64Out[1] = table[((p[0]&0x3)<<4)|(p[1]>>4)];
		pBase64Out[2] = table[((p[1]&0xf)<<2)];
		pBase64Out[3] = '=';
		//pBase64Out[4] = '\0';
	}
	//else
	//	pBase64Out[0] = '\0';
}

SIZE_T Base64DecodeLength(LPCSTR pBase64, SIZE_T str_len)
{
	if(str_len<=3)return 0;

	SSIZE_T fulllen = (str_len+3)/4*3;
	SSIZE_T miss = (4 - (str_len&3))&3;

	if(_details::_isb64(pBase64[str_len-2]) && _details::_isb64(pBase64[str_len-1]) && miss <= 2)
	{	
		fulllen -= miss;
	}
	else if(_details::_isb64(pBase64[str_len-2]) && !_details::_isb64(pBase64[str_len-1]))
	{
		fulllen -= (miss + 1);
	}
	else if(!_details::_isb64(pBase64[str_len-2]) && !_details::_isb64(pBase64[str_len-1]) && miss == 0)
	{
		fulllen -= 2;
	}
	else return fulllen;

	return rt::max((SSIZE_T)0, fulllen);
}

bool Base64Decode(LPVOID pDataOut, SIZE_T* pDataOutLen,LPCSTR pBase64, SIZE_T str_len)
{
	static const int rev_table[80] = // base on 2B
	{
		62,-1,-1,-1,			//'+' 2B,2C,2D,2E,
		63,						//'/' 2F
		52,53,54,55,56,57,58,59,60,61,	// '0' - '9', 30 - 39
		-1,-1,-1,-1,-1,-1,-1,	// '=', 3A, 3B, 3C, |3D|, 3E, 3F, 40
		0,1,2,3,4,5,6,7,8,9,	// 'A'-'Z', 41 - 5A
		10,11,12,13,14,15,16,17,18,19,
		20,21,22,23,24,25,		
		-1,-1,-1,-1,-1,-1,		// 5B 5C 5D 5E 5F 60
		26,27,28,29,30,31,32,33,34,35, // 'a'-'z' 61 - 7A
		36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51
	};

	LPBYTE p = (LPBYTE)pDataOut;

	int a[4];

	for(;str_len>4;str_len-=4,pBase64+=4,p+=3)
	{
		if(rt::IsInRange_CC<int>(pBase64[0],0x2b,0x7a) && (a[0]=rev_table[pBase64[0]-0x2b])>=0){}
		else break;
		if(rt::IsInRange_CC<int>(pBase64[1],0x2b,0x7a) && (a[1]=rev_table[pBase64[1]-0x2b])>=0){}
		else break;
		if(rt::IsInRange_CC<int>(pBase64[2],0x2b,0x7a) && (a[2]=rev_table[pBase64[2]-0x2b])>=0){}
		else break;
		if(rt::IsInRange_CC<int>(pBase64[3],0x2b,0x7a) && (a[3]=rev_table[pBase64[3]-0x2b])>=0){}
		else break;

		p[0] = (a[0]<<2) | (a[1]>>4);
		p[1] = ((a[1]&0xf)<<4) | (a[2]>>2);
		p[2] = ((a[2]&0x3)<<6) | a[3];
	}

	*pDataOutLen = (p - (LPBYTE)pDataOut);
	if(str_len == 0)return true;

	//int remaining = (int)rt::min(str_len, (SIZE_T)4);
	int miss = (4 - (str_len&3))&3;

	char last4char[4];
	*((DWORD*)last4char) = 0x3d3d3d3d; // all '='
	memcpy(last4char, pBase64, 4 - miss);

	if(rt::IsInRange_CC<int>(last4char[0],0x2b,0x7a) && (a[0]=rev_table[last4char[0]-0x2b])>=0){}
	else return false;
	if(rt::IsInRange_CC<int>(last4char[1],0x2b,0x7a) && (a[1]=rev_table[last4char[1]-0x2b])>=0){}
	else return false;

	if(_details::_isb64(last4char[2]) && _details::_isb64(last4char[3]))
	{
		(*pDataOutLen) += 3;

		a[2]=rev_table[last4char[2]-0x2b];
		a[3]=rev_table[last4char[3]-0x2b];

		p[0] = (a[0]<<2) | (a[1]>>4);
		p[1] = ((a[1]&0xf)<<4) | (a[2]>>2);
		p[2] = ((a[2]&0x3)<<6) | a[3];

		return str_len <= 4;
	}
	else if(_details::_isb64(last4char[2]) && !_details::_isb64(last4char[3]))
	{
		(*pDataOutLen) += 2;
		a[2]=rev_table[last4char[2]-0x2b];

		p[0] = (a[0]<<2) | (a[1]>>4);
		p[1] = ((a[1]&0xf)<<4) | (a[2]>>2);

		return str_len <= 4;
	}
	else if(!_details::_isb64(last4char[2]) && !_details::_isb64(last4char[3]))
	{
		(*pDataOutLen) += 1;
		
		p[0] = (a[0]<<2) | (a[1]>>4);

		return str_len <= 4;
	}

	return false;
}

SIZE_T Base16EncodeLength(SIZE_T len){ return len*2; }
SIZE_T Base16DecodeLength(SIZE_T len){ return len/2; }

void Base16Encode(LPSTR pBase16Out,LPCVOID pData_in, SIZE_T data_len)
{
	LPCBYTE pData = (LPCBYTE)pData_in;
	for(SIZE_T i=0;i<data_len;i++)
	{
		int c1 = pData[i]>>4;
		int c2 = pData[i]&0xf;
		pBase16Out[2*i+0] = (c1>9)?('a'+c1-10):('0'+c1);
		pBase16Out[2*i+1] = (c2>9)?('a'+c2-10):('0'+c2);
	}
}

bool Base16Decode(LPVOID pDataOut_in,SIZE_T data_len,LPCSTR pBase16, SIZE_T str_len)
{
	LPBYTE pDataOut = (LPBYTE)pDataOut_in;

	if(str_len != data_len*2)return false;
	for(SIZE_T i=0;i<data_len;i++)
	{
		int c[2] = { pBase16[2*i+0], pBase16[2*i+1] };

		for(int j=0;j<2;j++)
		{
			if(c[j]<='9')
			{	if(c[j]<'0')return false;
				c[j] -= '0';
			}
			else if(c[j]>='A' && c[j]<='F')
			{	c[j] -= 'A' - 10;
			}
			else if(c[j]>='a' && c[j]<='f')
			{	c[j] -= 'a' - 10;
			}
			else return false;
		}

		pDataOut[i] = (c[0]<<4) | c[1];
	}

	return true;
}

bool Base16Encode(const rt::String_Ref&in, rt::String& out)
{
	out.SetLength(Base16EncodeLength(in.GetLength()));
	Base16Encode(out, in.Begin(), in.GetLength());
	return true;
}

bool Base16Decode(const rt::String_Ref&in, rt::String& out)
{
	out.SetLength(Base16DecodeLength(in.GetLength()));
	return Base16Decode(out.Begin(), out.GetLength(), in.Begin(), in.GetLength());
}

namespace _details
{
	// Modified based on Crockford's Base32 - http://en.wikipedia.org/wiki/Base32
	// also avoids 's' and 'z', which confuse with 2 and 5
	//value		encode	decode			value	encode	decode
	//0			0		0 o O			16		G		g G
	//1			1		1 l L			17		H		h H
	//2			2		2 z Z			18		I		i I		
	//3			3		3				19		J		j J
	//4			4		4				20		K		k K
	//5			5		5				21		M		m M
	//6			6		6				22		N		n N
	//7			7		7				23		P		p P
	//8			8		8				24		Q		q Q
	//9			9		9				25		R		r R
	//10		A		a A				26		s		s S
	//11		B		b B				27		T		t T
	//12		C		c C				28		V		v V u U
	//13		D		d D				29		W		w W
	//14		E		e E				30		X		x X
	//15		F		f F				31		Y		y Y

	static const int _base32_crockford_decoding['z' - '0' + 1] = 
	{
		0,1,2,3,4,5,6,7,8,9,	/*0-9*/
		-1,-1,-1,-1,-1,-1,-1,	/*:	; <	= >	? @	*/
		10,11,12,13,14,15,16,17,18,19,20,/*A-K*/	
		1,						/*L	= 1*/
		21,22,					/*M	N */	
		0,						/*O = 0	*/
		23,24,25,26,27,			/*PQRST	*/		
		28,28,					/*U V */	
		29,30,31,2,				/*WXYZ*/
		-1,-1,-1,-1,			/*[	\ ] ^*/
		-1,						/* _ */
		-1,						/* ` */	
		10,11,12,13,14,15,16,17,18,19,20,/*a-k*/
		1,						/*l	= 1*/
		21,22,					/*m	n */
		0,						/*o = 0	*/
		23,24,25,26,27,			/*pqrst	*/		
		28,28,					/*u v */
		29,30,31,2,				/*wxyz*/
	}; 															//			1		  2			3
																//01234567890123456789012345678901
	static const char _base32_crockford_encoding_uppercase[33] = "0123456789ABCDEFGHIJKMNPQRSTVWXY";
	static const char _base32_crockford_encoding_lowercase[33] = "0123456789abcdefghijkmnpqrstvwxy";
																//			1		  2			3
																//01234567890123456789012345678901
	static const char _base32_extendhex_encoding_uppercase[33] = "0123456789ABCDEFGHIJKLMNOPQRSTUV";
	static const char _base32_extendhex_encoding_lowercase[33] = "0123456789abcdefghijklmnopqrstuv";

	template<bool uppercase = true>
	struct table_extend_hex
	{
		FORCEINL static bool decode(int&b, int d)
		{	if(d>='0' && d<='9'){ b = d - '0'; return false;}
			else if(d>='a' && d<='v'){ b = d - 'a' + 10; return false; }
			else if(d>='A' && d<='V'){ b = d - 'A' + 10; return false; }
			else return true;
		}
		FORCEINL static int encode(int d){ return _base32_extendhex_encoding_uppercase[d]; }
	};
		template<> struct table_extend_hex<false>: public table_extend_hex<true>
		{	FORCEINL static int encode(int d){ return _base32_extendhex_encoding_lowercase[d]; }
		};

	template<bool uppercase = true>
	struct table_crockford
	{
		FORCEINL static bool decode(int&b, int d)
		{	
			if(d>='0' && d<='z'){ b = _base32_crockford_decoding[d - '0']; return false; }
			return true;
		}
		FORCEINL static int encode(int d){ return _base32_crockford_encoding_uppercase[d]; }
	};
		template<> struct table_crockford<false>: public table_crockford<true>
		{	FORCEINL static int encode(int d){ return _base32_crockford_encoding_lowercase[d]; }
		};

	template<typename BASE32_TABLE>
	FORCEINL void _Base32Encode(LPSTR p, LPCVOID pdata, SIZE_T data_len)
	{
		LPCBYTE d = (LPCBYTE)pdata;
		LPCBYTE e = d + data_len;
		for(;(d + 4)<e;d+=5, p+=8)
		{	
			p[0] = BASE32_TABLE::encode(d[0]>>3);
			p[1] = BASE32_TABLE::encode(((d[0]&0x7)<<2) | (d[1]>>6));
			p[2] = BASE32_TABLE::encode((d[1]>>1) & 0x1f);
			p[3] = BASE32_TABLE::encode(((d[1]&1)<<4) | (d[2]>>4));
			p[4] = BASE32_TABLE::encode(((d[2]&0xf)<<1) | (d[3]>>7));
			p[5] = BASE32_TABLE::encode((d[3]>>2) & 0x1f);
			p[6] = BASE32_TABLE::encode(((d[3]&0x3)<<3) | (d[4]>>5));
			p[7] = BASE32_TABLE::encode(d[4]&0x1f);
		}

		if(d == e)return;

		p[0] = BASE32_TABLE::encode(d[0]>>3);
		if(d+1 == e)	// nnnXX
		{	p[1] = BASE32_TABLE::encode(d[0]&0x7);
			return;
		}

		p[1] = BASE32_TABLE::encode(((d[0]&0x7)<<2) | (d[1]>>6));
		p[2] = BASE32_TABLE::encode((d[1]>>1) & 0x1f);
		if(d+2 == e)	// nXXXX
		{	p[3] = BASE32_TABLE::encode(d[1]&0x1);
			return;
		}

		p[3] = BASE32_TABLE::encode(((d[1]&1)<<4) | (d[2]>>4));
		if(d+3 == e)	// nnnnX
		{	p[4] = BASE32_TABLE::encode(d[2]&0xf);
			return;
		}

		p[4] = BASE32_TABLE::encode(((d[2]&0xf)<<1) | (d[3]>>7));
		p[5] = BASE32_TABLE::encode((d[3]>>2) & 0x1f);
		ASSERT(d+4 == e);	// nnXXX
		p[6] = BASE32_TABLE::encode(d[3]&0x3);

		return;
	}

	template<typename BASE32_TABLE>
	FORCEINL bool _Base32Decode(LPVOID pDataOut, SIZE_T data_len, LPCSTR pBase32, SIZE_T str_len)
	{
		if(os::Base32DecodeLength(str_len) != data_len)return false;

		LPBYTE p = (LPBYTE)pDataOut;
		LPCSTR d = pBase32;
		LPCSTR e = d + str_len;
		for(;(d + 8)<=e; p+=5)
		{
			int b[8];
			if(BASE32_TABLE::decode(b[0], *d++))return false;
			if(BASE32_TABLE::decode(b[1], *d++))return false;
			if(BASE32_TABLE::decode(b[2], *d++))return false;
			if(BASE32_TABLE::decode(b[3], *d++))return false;
			if(BASE32_TABLE::decode(b[4], *d++))return false;
			if(BASE32_TABLE::decode(b[5], *d++))return false;
			if(BASE32_TABLE::decode(b[6], *d++))return false;
			if(BASE32_TABLE::decode(b[7], *d++))return false;

			p[0] = (b[0]<<3) | (b[1]>>2);
			p[1] = (b[1]<<6) | (b[2]<<1) | (b[3]>>4);
			p[2] = (b[3]<<4) | (b[4]>>1);
			p[3] = (b[4]<<7) | (b[5]<<2) | (b[6]>>3);
			p[4] = (b[6]<<5) | b[7];
		}

		if(d == e)return true;

		int t;
		int b[7];
		if(BASE32_TABLE::decode(b[0], *d++))return false;

		if(d+1 == e)
		{		
			if(BASE32_TABLE::decode(t, *d) || t > 7)return false;
			p[0] = (b[0]<<3) | t;
			return true;
		}
	
		if(BASE32_TABLE::decode(b[1], *d++))return false;
		p[0] = (b[0]<<3) | (b[1]>>2);
		if(BASE32_TABLE::decode(b[2], *d++))return false;

		if(d+1 == e)
		{	
			if(BASE32_TABLE::decode(t, *d) || t > 1)return false;
			p[1] = (b[1]<<6) | (b[2]<<1) | t;
			return true;
		}
	
		if(BASE32_TABLE::decode(b[3], *d++))return false;
		p[1] = (b[1]<<6) | (b[2]<<1) | (b[3]>>4);

		if(d+1 == e)
		{	
			if(BASE32_TABLE::decode(t, *d) || t > 16)return false;
			p[2] = (b[3]<<4) | t;
			return true;
		}

		if(BASE32_TABLE::decode(b[4], *d++))return false;
		p[2] = (b[3]<<4) | (b[4]>>1);
		if(BASE32_TABLE::decode(b[5], *d++))return false;

		if(d+1 == e)
		{	
			if(BASE32_TABLE::decode(t, *d) || t > 4)return false;
			p[3] = (b[4]<<7) | (b[5]<<2) | t;
			return true;
		}

		return false;
	}
} // namespace _details

SIZE_T Base32DecodeLength(SIZE_T len)
{						    // 0, 1, 2, 3, 4, 5, 6, 7
	static const int tl[] = {  0,-1, 1,-1, 2, 3,-1, 4 } ;
	int tail_len = tl[len&0x7];
	if(tail_len >=0)
		return len/8*5 + tail_len;
	else
		return 0;
}

SIZE_T Base32EncodeLength(SIZE_T len)
{
	return (len*8+4)/5;// + 1;
}

bool Base32CrockfordDecode(LPVOID pDataOut, SIZE_T data_len, LPCSTR pBase32, SIZE_T str_len)
{
	return _details::_Base32Decode<_details::table_crockford<>>(pDataOut, data_len, pBase32, str_len);
}

void Base32CrockfordEncode(LPSTR pBase32Out, LPCVOID pData, SIZE_T data_len)
{
	_details::_Base32Encode<_details::table_crockford<true>>(pBase32Out, pData, data_len);
}

void Base32CrockfordEncodeLowercase(LPSTR pBase32Out,LPCVOID pData, SIZE_T data_len)
{
	_details::_Base32Encode<_details::table_crockford<false>>(pBase32Out, pData, data_len);
}

bool Base32Decode(LPVOID pDataOut,SIZE_T data_len,LPCSTR pBase32, SIZE_T str_len)
{
	return _details::_Base32Decode<_details::table_extend_hex<>>(pDataOut, data_len, pBase32, str_len);
}

void Base32Encode(LPSTR pBase32Out,LPCVOID pData, SIZE_T data_len)
{
	_details::_Base32Encode<_details::table_extend_hex<true>>(pBase32Out, pData, data_len);
}

void Base32EncodeLowercase(LPSTR pBase32Out,LPCVOID pData, SIZE_T data_len)
{
	_details::_Base32Encode<_details::table_extend_hex<false>>(pBase32Out, pData, data_len);
}

void UrlEncode(const rt::String_Ref& url, rt::String& encoded_url)
{
	encoded_url.SetLength(url.GetLength()*3);
	encoded_url.SetLength(UrlEncode((LPCSTR)url.Begin(), (UINT)url.GetLength(), (LPSTR)encoded_url.Begin()));
}

UINT UrlEncode(LPCSTR url, UINT url_len, LPSTR encoded_url)
{
	UINT out_len = 0;
	for(UINT i=0;i<url_len;i++)
	{
		if(	(url[i]>='a' && url[i]<='z') ||
			(url[i]>='A' && url[i]<='Z') ||
			(url[i]>='0' && url[i]<='9') ||
			url[i] == '-' ||
			url[i] == '_' ||
			url[i] == '.' ||
			url[i] == '~'
		)
		{	encoded_url[out_len++] = url[i];
		}
		else
		{	encoded_url[out_len++] = '%';

			rt::tos::Binary<> code(&url[i], 1);

			encoded_url[out_len++] = code[0];
			encoded_url[out_len++] = code[1];
		}
	}

	return out_len;
}

UINT UrlDecode(LPCSTR encoded_url, UINT encoded_url_len, LPSTR url)  // return encoded length
{
	UINT out_len = 0;
	for(UINT i=0;i<encoded_url_len;i++)
	{
		if(encoded_url[i] == '%')
		{
			int c1 = encoded_url[i+1];
			int c2 = encoded_url[i+2];

			if(c1>='0' && c1<='9')
				c1 -= '0';
			else if(c1>='A' && c1<='F')
				c1 -= c1-'A' + 10;
			else if(c1>='a' && c1<='f')
				c1 -= c1-'a' + 10;
			else break;

			if(c2>='0' && c2<='9')
				c2 -= '0';
			else if(c2>='A' && c2<='F')
				c2 = c2-'A' + 10;
			else if(c2>='a' && c2<='f')
				c2 = c2-'a' + 10;
			else break;

			url[out_len++] = (c1<<4) | c2;
			i+=2;
		}
		else
		{	url[out_len++] = encoded_url[i];
		}
	}

	return out_len;
}

void UrlDecode(const rt::String_Ref& encoded_url, rt::String& url)
{
	url.SetLength(encoded_url.GetLength());
	url.SetLength(UrlDecode(encoded_url.Begin(), (UINT)encoded_url.GetLength(), url.Begin()));
}

} // namespace os

