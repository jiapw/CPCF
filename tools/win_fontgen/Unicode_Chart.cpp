#include "StdAfx.h"
#include "Unicode_Chart.h"

static const unicode::ChartArea unicode::g_UnicodeCharts[UNICODE_CHART_SIZE] = 
{
	{_T("Latin Basic")				,	0x0020,		0x007E,		false,	unicode::UR_EUROPEAN},
	{_T("Latin-1")					,	0x00a0,		0x00ff,		false,	unicode::UR_EUROPEAN},
	{_T("Latin Extended-A")			,	0x0100,		0x017f,		false,	unicode::UR_EUROPEAN},
	{_T("Latin Extended-B")			,	0x0180,		0x024f,		false,	unicode::UR_EUROPEAN},
	{_T("Greek & Coptic")			,	0x0374,		0x03ff,		false,	unicode::UR_EUROPEAN},				// Greek / Egypt
	{_T("Cyrillic")					,	0x0400,		0x04ff,		false,	unicode::UR_EUROPEAN},				// Russian
	{_T("Armenian")					,	0x0531,		0x058a,		false,	unicode::UR_EUROPEAN},				// Iraq, Syria, Lebanon, Turkey, Greece, Israel 
	{_T("Hebrew")					,	0x0591,		0x05f4,		false,	unicode::UR_MIDDLE_EASTERN},			// Israel  (Jew)
	{_T("Arabic")					,	0x0600,		0x06ff,		false,	unicode::UR_MIDDLE_EASTERN},			// Arab world
	{_T("Syriac")					,	0x0700,		0x074f,		false,	unicode::UR_MIDDLE_EASTERN}, 		// Armenia, Azerbaijan, Georgia, Iran, Iraq, Lebanon, Palestine, Israel, Syria, Turkey, Kerala
	{_T("Thaana")					,	0x0780,		0x07b1,		false,	unicode::UR_MIDDLE_EASTERN}, 		// Maldives (马尔代夫)
	{_T("Arabic Supplement")		,	0x0750,		0x076d,		false,	unicode::UR_MIDDLE_EASTERN}, 		
	{_T("N'Ko")						,	0x07c0,		0x07fa,		false,	unicode::UR_AFRICAN},				// West Africa
	{_T("Devanagari")				,	0x0900,		0x097f,		false,	unicode::UR_INDIA},					// Nepal
	{_T("Bengali")					,	0x0980,		0x09fa,		false,	unicode::UR_INDIA},					// Eastern Indian subcontinent
	{_T("Gurmukhi")					,	0x0a00,		0x0a74,		false,	unicode::UR_INDIA},
	{_T("Gujarati")					,	0x0a80,		0x0af1,		false,	unicode::UR_INDIA},
	{_T("Oriya")					,	0x0b00,		0x0b71,		false,	unicode::UR_INDIA},					// Indian
	{_T("Tamil")					,	0x0b80,		0x0bfa,		false,	unicode::UR_INDIA},					// Tamils in India and Sri Lanka
	{_T("Telugu")					,	0x0c00,		0x0c6f,		false,	unicode::UR_INDIA},					// Indian state of Andhra Pradesh
	{_T("Kannada")					,	0x0c80,		0x0cf2,		false,	unicode::UR_INDIA},					// Southern India
	{_T("Malayalam")				,	0x0d00,		0x0d6f,		false,	unicode::UR_INDIA},					// India, Malaysia and Singapore 
	{_T("Sinhala")					,	0x0d80,		0x0df4,		false,	unicode::UR_INDIA},					// Sri Lanka
	{_T("Thai")						,	0x0e00,		0x0e5b,		false,	unicode::UR_SOUTH_EAST_ASIAN},		// Thailand
	{_T("Lao")						,	0x0e80,		0x0edd,		false,	unicode::UR_SOUTH_EAST_ASIAN},		// Laos
	{_T("Tibetan")					,	0x0f00,		0x0fd1,		false,	unicode::UR_CENTRAL_ASIAN},			// eastern Central Asia bordering South Asia (藏语)
	{_T("Myanmar")					,	0x1000,		0x1059,		false,	unicode::UR_SOUTH_EAST_ASIAN},		// Myanmar(缅甸)
	{_T("Georgian")					,	0x10a0,		0x10fc,		false,	unicode::UR_EUROPEAN},				// Georgia 乔治亚苏维埃社会主义共和国(位于前苏联高加索南部)
	{_T("Hangul Jamo")				,	0x1100,		0x11f9,		true ,	unicode::UR_EAST_ASIAN},				// Most Korean
	{_T("Ethiopic")					,	0x1200,		0x137c,		false,	unicode::UR_AFRICAN},				// current region of Eritrea and northern Ethiopia in the Horn of Africa
	{_T("Ethiopic Supplement")		,	0x1380,		0x1399,		false,	unicode::UR_AFRICAN},
	{_T("Cherokee")					,	0x13a0,		0x13f4,		false,	unicode::UR_AMERICAN},				// Native American tribes in the United States
	{_T("Canadian Syllabics")		,	0x1401,		0x1676,		false,	unicode::UR_AMERICAN},				// used to write all of the Cree dialects from Naskapi (spoken in Quebec) to the Rocky Mountains
	{_T("Ogham")					,	0x1680,		0x169c,		false,	unicode::UR_ANCIENT},				// Ancient Irish
	{_T("Runic")					,	0x16a0,		0x16f0,		false,	unicode::UR_ANCIENT},				// 古代北欧文字
	{_T("Tagalog")					,	0x1700,		0x1714,		false,	unicode::UR_PHILIPPINE},				// Philippines
	{_T("Hanunoo")					,	0x1720,		0x1736,		false,	unicode::UR_PHILIPPINE},				// the province of Mindoro in the Philippines.
	{_T("Buhid")					,	0x1740,		0x1753,		false,	unicode::UR_PHILIPPINE},				// the province of Mindoro in the Philippines
	{_T("Tagbanwa")					,	0x1760,		0x1773,		false,	unicode::UR_PHILIPPINE},				// the province of Mindoro in the Philippines
	{_T("Khmer")					,	0x1780,		0x17f9,		false,	unicode::UR_SOUTH_EAST_ASIAN},		// Khmer (柬埔寨)
	{_T("Mongolian")				,	0x1800,		0x18a9,		false,	unicode::UR_CENTRAL_ASIAN},			// Mongolia, China (Inner Mongolia)
	{_T("Limbu")					,	0x1900,		0x194f,		false,	unicode::UR_INDIA},					// Nepal, Sikkim and Darjeeling district, West Bengal, India
	{_T("Tai Le")					,	0x1950,		0x1974,		false,	unicode::UR_SOUTH_EAST_ASIAN},		// 傣语 China, Myanmar, Thailand, Laos 
	{_T("New Tai Lue")				,	0x1980,		0x19df,		false,	unicode::UR_SOUTH_EAST_ASIAN},		// Simplified Tai Lue, 傣语
	{_T("Khmer Symbols")			,	0x19e0,		0x19ff,		false,	unicode::UR_SOUTH_EAST_ASIAN},		
	{_T("Buginese")					,	0x1a00,		0x1a1f,		false,	unicode::UR_SOUTH_EAST_ASIAN},		// southern part of Sulawesi, Indonesia.
	{_T("Balinese")					,	0x1b00,		0x1b7c,		false,	unicode::UR_SOUTH_EAST_ASIAN},		// the Indonesian island of Bali
	{_T("Latin Extanded")			,	0x1e00,		0x1ef9,		false,	unicode::UR_EUROPEAN},
	{_T("Greek Extanded")			,	0x1f00,		0x1ffe,		false,	unicode::UR_EUROPEAN},
	{_T("Currency")					,	0x20a0,		0x20b5,		false,	unicode::UR_NEUTRAL},
	{_T("Number Forms")				,	0x2153,		0x2184,		false,	unicode::UR_NEUTRAL},
	{_T("Arrows")					,	0x2190,		0x21ff,		false,	unicode::UR_NEUTRAL},
	{_T("Math Operators")			,	0x2200,		0x22ff,		false,	unicode::UR_NEUTRAL},
	{_T("Box Drawing")				,	0x2500,		0x257f,		true ,	unicode::UR_NEUTRAL},
	{_T("Block Elements")			,	0x2580,		0x259f,		true ,	unicode::UR_NEUTRAL},
	{_T("Shapes")					,	0x25a0,		0x25ff,		false,	unicode::UR_NEUTRAL},
	{_T("Misc Symbols")				,	0x2600,		0x26b2,		false,	unicode::UR_NEUTRAL},
	{_T("Dingbats")					,	0x2700,		0x27be,		false,	unicode::UR_NEUTRAL},
	{_T("Braille Patterns")			,	0x2800,		0x28ff,		true ,	unicode::UR_NEUTRAL},
	{_T("Glagolitic")				,	0x2c00,		0x2c5e,		false ,	unicode::UR_ANCIENT},				// Eastern Europe
	{_T("Latin Extended-C")			,	0x2c60,		0x2c77,		false,	unicode::UR_EUROPEAN},
	{_T("Coptic")					,	0x2c80,		0x2cFF,		false,	unicode::UR_EUROPEAN},				// Egypt
	{_T("Tifinagh")					,	0x2d30,		0x2d6f,		false,	unicode::UR_AFRICAN},				// North Africa
	{_T("Ethiopic Extended")		,	0x2d80,		0x2dde,		false,	unicode::UR_AFRICAN},
	{_T("CJK Radicals")				,	0x2e80,		0x2ef3,		true ,	unicode::UR_EAST_ASIAN},
	{_T("KangXi Radicals")			,	0x2f00,		0x2fd5,		true ,	unicode::UR_EAST_ASIAN},
	{_T("Ideographic Description")	,	0x2ff0,		0x2ffb,		true ,	unicode::UR_EAST_ASIAN},
	{_T("CJK Symbol & Punctuation")	,	0x3000,		0x303f,		true ,	unicode::UR_EAST_ASIAN},
	{_T("Hiragana")					,	0x3041,		0x309f,		true ,	unicode::UR_EAST_ASIAN},		// Most Japanese (平假名)
	{_T("Katakana")					,	0x30a0,		0x30ff,		true ,	unicode::UR_EAST_ASIAN},		// Most Japanese (片假名)
	{_T("Bopomofo")					,	0x3105,		0x312c,		true ,	unicode::UR_EAST_ASIAN},
	{_T("Hangul Compatibility Jamo"),	0x3130,		0x318e,		true ,	unicode::UR_EAST_ASIAN},		// Most Korean
	{_T("Kanbun")					,	0x3190,		0x319f,		true ,	unicode::UR_EAST_ASIAN},		// 17th-century Japanese era
	{_T("Bopomofo Extended")		,	0x31a0,		0x31b7,		true ,	unicode::UR_EAST_ASIAN},
	{_T("CJK Strokes")				,	0x31c0,		0x31cf,		true ,	unicode::UR_EAST_ASIAN},
	{_T("Katakana Phonetic")		,	0x31f0,		0x31ff,		true ,	unicode::UR_EAST_ASIAN},		// Most Japanese (片假名)
	{_T("CJK Unified Ideographs Ext"),	0x3400,		0x4db5,		true ,	unicode::UR_EAST_ASIAN},
	{_T("Yijing Hexagram")			,	0x4dc0,		0x4dff,		true ,	unicode::UR_ANCIENT},		// Hexagram numbers
	{_T("CJK Unified Ideographs")	,	0x4e00,		0x9fbb,		true ,	unicode::UR_EAST_ASIAN},		// All Chinese
	{_T("Yi Syllables")				,	0xa000,		0xa48c,		true ,	unicode::UR_EAST_ASIAN},		// 彝族  China, Thailand 
	{_T("Yi Radicals")				,	0xa490,		0xa4c6,		true ,	unicode::UR_EAST_ASIAN},		// 彝族
	{_T("Latin Extended-D")			,	0xa720,		0xa721,		false,	unicode::UR_EUROPEAN},
	{_T("Syloti Nagri")				,	0xa800,		0xa82b,		false,	unicode::UR_ANCIENT},
	{_T("Phags-Pa")					,	0xa840,		0xa877,		false,	unicode::UR_ANCIENT},		// China
	{_T("Hangul Syllables")			,	0xac00,		0xd7a3,		true ,	unicode::UR_EAST_ASIAN},     // Most Korean
	{_T("Compatibility Ideographs")	,	0xf900,		0xfad9,		true ,	unicode::UR_EAST_ASIAN},     // Most Ancient Chinese
	{_T("Vertical Forms (CJK)")		,	0xfe10,		0xfe19,		true ,	unicode::UR_EAST_ASIAN},
	{_T("Halfwidth & Fullwidth")	,	0xff01,		0xffee,		true ,	unicode::UR_EAST_ASIAN}
};

static const LPCTSTR unicode::g_RegionNames[UR_REGION_MAX] = 
{
	_T("Neutral"),
	_T("European"),
	_T("African"),
	_T("Middle Eastern"),
	_T("American"),
	_T("India"),
	_T("Philippine"),
	_T("South East Asian"),
	_T("East Asian"),
	_T("Central Asian"),
	_T("Ancient")
};

