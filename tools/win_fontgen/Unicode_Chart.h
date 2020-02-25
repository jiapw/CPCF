#pragma once


namespace unicode
{

enum UnicodeRegion
{	
	UR_NEUTRAL = 0,
	UR_EUROPEAN,
	UR_AFRICAN,
	UR_MIDDLE_EASTERN,
	UR_AMERICAN,
	UR_INDIA,
	UR_PHILIPPINE,
	UR_SOUTH_EAST_ASIAN,
	UR_EAST_ASIAN,
	UR_CENTRAL_ASIAN,
	UR_ANCIENT,
	UR_REGION_MAX
};

enum Mics_Counter
{
	UNICODE_CHART_SIZE = 88
};


struct ChartArea
{
	LPCTSTR		Name;
	UINT		CodeStart;
	UINT		CodeEnd;

	bool		FixedWidth;
	DWORD		Region;
};

extern const ChartArea	g_UnicodeCharts[UNICODE_CHART_SIZE];
extern const LPCTSTR	g_RegionNames[UR_REGION_MAX];


}


