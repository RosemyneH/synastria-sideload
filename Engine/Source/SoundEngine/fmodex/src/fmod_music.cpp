#include "fmod_settings.h"

#if defined(FMOD_SUPPORT_MOD) || defined(FMOD_SUPPORT_S3M) || defined(FMOD_SUPPORT_XM) || defined(FMOD_SUPPORT_IT)

#include "fmod_debug.h"
#include "fmod_music.h"
#include "fmod_channeli.h"
#include "fmod_channel_real.h"
#include "fmod_dspi.h"
#include "fmod_outputi.h"
#include "fmod_systemi.h"
#if defined(PLATFORM_PS2)
    #include "fmodps2.h"
#endif

#include <string.h>

namespace FMOD
{

MusicSample          gDummySample;
MusicVirtualChannel  gDummyVirtualChannel;
MusicChannel         gDummyChannel;
MusicInstrument      gDummyInstrument;

// SINE TABLE FOR TREMOLO AND VIBRATO (from protracker so 100% compatible)
unsigned char gSineTable[32] = 
{
	   0, 24, 49, 74, 97,120,141,161,
	 180,197,212,224,235,244,250,253,
	 255,253,250,244,235,224,212,197,
	 180,161,141,120, 97, 74, 49, 24
};


signed char gFineSineTable[256] = 
{
	  0,  2,  3,  5,  6,  8,  9, 11, 12, 14, 16, 17, 19, 20, 22, 23,
	 24, 26, 27, 29, 30, 32, 33, 34, 36, 37, 38, 39, 41, 42, 43, 44,
	 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 56, 57, 58, 59,
	 59, 60, 60, 61, 61, 62, 62, 62, 63, 63, 63, 64, 64, 64, 64, 64,
	 64, 64, 64, 64, 64, 64, 63, 63, 63, 62, 62, 62, 61, 61, 60, 60,
	 59, 59, 58, 57, 56, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46,
	 45, 44, 43, 42, 41, 39, 38, 37, 36, 34, 33, 32, 30, 29, 27, 26,
	 24, 23, 22, 20, 19, 17, 16, 14, 12, 11,  9,  8,  6,  5,  3,  2,
	  0, -2, -3, -5, -6, -8, -9,-11,-12,-14,-16,-17,-19,-20,-22,-23,
	-24,-26,-27,-29,-30,-32,-33,-34,-36,-37,-38,-39,-41,-42,-43,-44,
	-45,-46,-47,-48,-49,-50,-51,-52,-53,-54,-55,-56,-56,-57,-58,-59,
	-59,-60,-60,-61,-61,-62,-62,-62,-63,-63,-63,-64,-64,-64,-64,-64,
	-64,-64,-64,-64,-64,-64,-63,-63,-63,-62,-62,-62,-61,-61,-60,-60,
	-59,-59,-58,-57,-56,-56,-55,-54,-53,-52,-51,-50,-49,-48,-47,-46,
	-45,-44,-43,-42,-41,-39,-38,-37,-36,-34,-33,-32,-30,-29,-27,-26,
	-24,-23,-22,-20,-19,-17,-16,-14,-12,-11, -9, -8, -6, -5, -3, -2,
};

// AMIGA TYPE PERIOD TABLE, FOR 11 OCTAVES (octave 9 and 10 are only for
// .MOD's that use these sort of periods when loading, i.e dope.mod)
unsigned int gPeriodTable[134] = 
{
  27392,25856,24384,23040,21696,20480,19328,18240,17216,16256,15360,14496,//0
  13696,12928,12192,11520,10848,10240, 9664, 9120, 8608, 8128, 7680, 7248,//1
   6848, 6464, 6096, 5760, 5424, 5120, 4832, 4560, 4304, 4064, 3840, 3624,//2
   3424, 3232, 3048, 2880, 2712, 2560, 2416, 2280, 2152, 2032, 1920, 1812,//3
   1712, 1616, 1524, 1440, 1356, 1280, 1208, 1140, 1076, 1016,  960,  906,//4
	856,  808,  762,  720,  678,  640,  604,  570,  538,  508,  480,  453,//5
	428,  404,  381,  360,  339,  320,  302,  285,  269,  254,  240,  226,//6
	214,  202,  190,  180,  170,  160,  151,  143,  135,  127,  120,  113,//7
	107,  101,   95,   90,   85,   80,   75,   71,   67,   63,   60,   56,//8
	 53,   50,   47,   45,   42,   40,   37,   35,   33,   31,   30,   28,//9
	 26,   25,   23,   22,   21,   20,   18,   17,   16,   15,   15,   14,//10
	  0,    0       // <- these last 2 are for no note (132) and keyoff (133) ?
};

int gITLogPeriodTable[] = 
{
  54784,51712,48768,46080,43392,40960,38656,36480,34432,32512,30720,28992,//0
  27392,25856,24384,23040,21696,20480,19328,18240,17216,16256,15360,14496,//1
  13696,12928,12192,11520,10848,10240, 9664, 9120, 8608, 8128, 7680, 7248,//2
   6848, 6464, 6096, 5760, 5424, 5120, 4832, 4560, 4304, 4064, 3840, 3624,//3
   3424, 3232, 3048, 2880, 2712, 2560, 2416, 2280, 2152, 2032, 1920, 1812,//4
   1712, 1616, 1524, 1440, 1356, 1280, 1208, 1140, 1076, 1016,  960,  906,//5
	856,  808,  762,  720,  678,  640,  604,  570,  538,  508,  480,  453,//6
	428,  404,  381,  360,  339,  320,  302,  285,  269,  254,  240,  226,//7
	214,  202,  190,  180,  170,  160,  151,  143,  135,  127,  120,  113,//8
	107,  101,   95,   90,   85,   80,   75,   71,   67,   63,   60,   56,//9
	 53,   50,   47,   45,   42,   40,   37,   35,   33,   31,   30,   28,//10
	 26,   25,   23,   22,   21,   20,   18,   17,   16,   15,   15,   14,//11
};

#ifdef FMOD_NO_FPU

unsigned int gLinearTable[768] = 
{
    535232,534749,534266,533784,533303,532822,532341,531861,
    531381,530902,530423,529944,529466,528988,528511,528034,
    527558,527082,526607,526131,525657,525183,524709,524236,
    523763,523290,522818,522346,521875,521404,520934,520464,
    519994,519525,519057,518588,518121,517653,517186,516720,
    516253,515788,515322,514858,514393,513929,513465,513002,
    512539,512077,511615,511154,510692,510232,509771,509312,
    508852,508393,507934,507476,507018,506561,506104,505647,
    505191,504735,504280,503825,503371,502917,502463,502010,
    501557,501104,500652,500201,499749,499298,498848,498398,
    497948,497499,497050,496602,496154,495706,495259,494812,
    494366,493920,493474,493029,492585,492140,491696,491253,
    490809,490367,489924,489482,489041,488600,488159,487718,
    487278,486839,486400,485961,485522,485084,484647,484210,
    483773,483336,482900,482465,482029,481595,481160,480726,
    480292,479859,479426,478994,478562,478130,477699,477268,
    476837,476407,475977,475548,475119,474690,474262,473834,
    473407,472979,472553,472126,471701,471275,470850,470425,
    470001,469577,469153,468730,468307,467884,467462,467041,
    466619,466198,465778,465358,464938,464518,464099,463681,
    463262,462844,462427,462010,461593,461177,460760,460345,
    459930,459515,459100,458686,458272,457859,457446,457033,
    456621,456209,455797,455386,454975,454565,454155,453745,
    453336,452927,452518,452110,451702,451294,450887,450481,
    450074,449668,449262,448857,448452,448048,447644,447240,
    446836,446433,446030,445628,445226,444824,444423,444022,
    443622,443221,442821,442422,442023,441624,441226,440828,
    440430,440033,439636,439239,438843,438447,438051,437656,
    437261,436867,436473,436079,435686,435293,434900,434508,
    434116,433724,433333,432942,432551,432161,431771,431382,
    430992,430604,430215,429827,429439,429052,428665,428278,
    427892,427506,427120,426735,426350,425965,425581,425197,
    424813,424430,424047,423665,423283,422901,422519,422138,
    421757,421377,420997,420617,420237,419858,419479,419101,
    418723,418345,417968,417591,417214,416838,416462,416086,
    415711,415336,414961,414586,414212,413839,413465,413092,
    412720,412347,411975,411604,411232,410862,410491,410121,
    409751,409381,409012,408643,408274,407906,407538,407170,
    406803,406436,406069,405703,405337,404971,404606,404241,
    403876,403512,403148,402784,402421,402058,401695,401333,
    400970,400609,400247,399886,399525,399165,398805,398445,
    398086,397727,397368,397009,396651,396293,395936,395579,
    395222,394865,394509,394153,393798,393442,393087,392733,
    392378,392024,391671,391317,390964,390612,390259,389907,
    389556,389204,388853,388502,388152,387802,387452,387102,
    386753,386404,386056,385707,385359,385012,384664,384317,
    383971,383624,383278,382932,382587,382242,381897,381552,
    381208,380864,380521,380177,379834,379492,379149,378807,
    378466,378124,377783,377442,377102,376762,376422,376082,
    375743,375404,375065,374727,374389,374051,373714,373377,
    373040,372703,372367,372031,371695,371360,371025,370690,
    370356,370022,369688,369355,369021,368688,368356,368023,
    367691,367360,367028,366697,366366,366036,365706,365376,
    365046,364717,364388,364059,363731,363403,363075,362747,
    362420,362093,361766,361440,361114,360788,360463,360137,
    359813,359488,359164,358840,358516,358193,357869,357547,
    357224,356902,356580,356258,355937,355616,355295,354974,
    354654,354334,354014,353695,353376,353057,352739,352420,
    352103,351785,351468,351150,350834,350517,350201,349885,
    349569,349254,348939,348624,348310,347995,347682,347368,
    347055,346741,346429,346116,345804,345492,345180,344869,
    344558,344247,343936,343626,343316,343006,342697,342388,
    342079,341770,341462,341154,340846,340539,340231,339924,
    339618,339311,339005,338700,338394,338089,337784,337479,
    337175,336870,336566,336263,335959,335656,335354,335051,
    334749,334447,334145,333844,333542,333242,332941,332641,
    332341,332041,331741,331442,331143,330844,330546,330247,
    329950,329652,329355,329057,328761,328464,328168,327872,
    327576,327280,326985,326690,326395,326101,325807,325513,
    325219,324926,324633,324340,324047,323755,323463,323171,
    322879,322588,322297,322006,321716,321426,321136,320846,
    320557,320267,319978,319690,319401,319113,318825,318538,
    318250,317963,317676,317390,317103,316817,316532,316246,
    315961,315676,315391,315106,314822,314538,314254,313971,
    313688,313405,313122,312839,312557,312275,311994,311712,
    311431,311150,310869,310589,310309,310029,309749,309470,
    309190,308911,308633,308354,308076,307798,307521,307243,
    306966,306689,306412,306136,305860,305584,305308,305033,
    304758,304483,304208,303934,303659,303385,303112,302838,
    302565,302292,302019,301747,301475,301203,300931,300660,
    300388,300117,299847,299576,299306,299036,298766,298497,
    298227,297958,297689,297421,297153,296884,296617,296349,
    296082,295815,295548,295281,295015,294749,294483,294217,
    293952,293686,293421,293157,292892,292628,292364,292100,
    291837,291574,291311,291048,290785,290523,290261,289999,
    289737,289476,289215,288954,288693,288433,288173,287913,
    287653,287393,287134,286875,286616,286358,286099,285841,
    285583,285326,285068,284811,284554,284298,284041,283785,
    283529,283273,283017,282762,282507,282252,281998,281743,
    281489,281235,280981,280728,280475,280222,279969,279716,
    279464,279212,278960,278708,278457,278206,277955,277704,
    277453,277203,276953,276703,276453,276204,275955,275706,
    275457,275209,274960,274712,274465,274217,273970,273722,
    273476,273229,272982,272736,272490,272244,271999,271753,
    271508,271263,271018,270774,270530,270286,270042,269798,
    269555,269312,269069,268826,268583,268341,268099,267857 
};


#ifdef FMOD_SUPPORT_IT

/*
    // To generate following table
    void main()
    {
        int count;
        for (count=0; count < TABSIZE; count++)
        {
            float scalar = (float)FMUSIC_ITFINETUNETABLEMAX / (float)FMUSIC_ITFINETUNETABLESIZE * (float)count;
            float logs;

            logs = (float)log(scalar);	
            logs /= 0.693147f; 
            logs *= 768.0f;	

            printf("%4d, ", (int)logs);
            if (!((count+1) % 11)) printf("\n");
        }
    }
*/

signed short gITFineTuneTable[FMUSIC_ITFINETUNETABLESIZE] =
{
       0, -4926, -4158, -3709, -3390, -3143, -2941, -2770, -2622, -2492, -2375,
    -2269, -2173, -2084, -2002, -1926, -1854, -1787, -1724, -1664, -1607, -1553,
    -1501, -1452, -1405, -1360, -1316, -1274, -1234, -1195, -1158, -1121, -1086,
    -1052, -1019, -987, -956, -925, -896, -867, -839, -812, -785, -759,
    -733, -709, -684, -660, -637, -614, -592, -570, -548, -527, -506,
    -486, -466, -447, -427, -408, -390, -371, -353, -336, -318, -301,
    -284, -267, -251, -235, -219, -203, -188, -172, -157, -143, -128,
    -113,  -99,  -85,  -71,  -57,  -44,  -30,  -17,   -4,    8,   21,
      34,   46,   58,   71,   83,   95,  107,  118,  130,  141,  153,
     164,  175,  186,  197,  208,  219,  229,  240,  250,  261,  271,
     281,  291,  301,  311,  320,  330,  340,  349,  359,  368,  377,
     386,  396,  405,  414,  422,  431,  440,  449,  457,  466,  474,
     483,  491,  500,  508,  516,  524,  532,  540,  548,  556,  564,
     572,  579,  587,  595,  602,  610,  617,  624,  632,  639,  646,
     654,  661,  668,  675,  682,  689,  696,  703,  710,  717,  723,
     730,  737,  743,  750,  757,  763,  770,  776,  783,  789,  795,
     802,  808,  814,  820,  826,  833,  839,  845,  851,  857,  863,
     869,  875,  881,  886,  892,  898,  904,  909,  915,  921,  926,
     932,  938,  943,  949,  954,  960,  965,  971,  976,  981,  987,
     992,  997, 1003, 1008, 1013, 1018, 1023, 1029, 1034, 1039, 1044,
    1049, 1054, 1059, 1064, 1069, 1074, 1079, 1084, 1088, 1093, 1098,
    1103, 1108, 1112, 1117, 1122, 1127, 1131, 1136, 1141, 1145, 1150,
    1154, 1159, 1164, 1168, 1173, 1177, 1182, 1186, 1190, 1195, 1199,
    1204, 1208, 1212, 1217, 1221, 1225, 1230, 1234, 1238, 1242, 1247,
    1251, 1255, 1259, 1263, 1268, 1272, 1276, 1280, 1284, 1288, 1292,
    1296, 1300, 1304, 1308, 1312, 1316, 1320, 1324, 1328, 1332, 1336,
    1340, 1343, 1347, 1351, 1355, 1359, 1363, 1366, 1370, 1374, 1378,
    1381, 1385, 1389, 1392, 1396, 1400, 1404, 1407, 1411, 1414, 1418,
    1422, 1425, 1429, 1432, 1436, 1439, 1443, 1447, 1450, 1454, 1457,
    1461, 1464, 1467, 1471, 1474, 1478, 1481, 1485, 1488, 1491, 1495,
    1498, 1501, 1505, 1508, 1511, 1515, 1518, 1521, 1525, 1528, 1531,
    1534, 1538, 1541, 1544, 1547, 1551, 1554, 1557, 1560, 1563, 1566,
    1570, 1573, 1576, 1579, 1582, 1585, 1588, 1591, 1594, 1598, 1601,
    1604, 1607, 1610, 1613, 1616, 1619, 1622, 1625, 1628, 1631, 1634,
    1637, 1640, 1643, 1646, 1649, 1651, 1654, 1657, 1660, 1663, 1666,
    1669, 1672, 1675, 1677, 1680, 1683, 1686, 1689, 1692, 1694, 1697,
    1700, 1703, 1706, 1708, 1711, 1714, 1717, 1720, 1722, 1725, 1728,
    1730, 1733, 1736, 1739, 1741, 1744, 1747, 1749, 1752, 1755, 1757,
    1760, 1763, 1765, 1768, 1771, 1773, 1776, 1778, 1781, 1784, 1786,
    1789, 1791, 1794, 1797, 1799, 1802, 1804, 1807, 1809, 1812, 1814,
    1817, 1819, 1822, 1824, 1827, 1829, 1832, 1834, 1837, 1839, 1842,
    1844, 1847, 1849, 1852, 1854, 1856, 1859, 1861, 1864, 1866, 1868,
    1871, 1873, 1876, 1878, 1880, 1883, 1885, 1888, 1890, 1892, 1895,
    1897, 1899, 1902, 1904, 1906, 1909, 1911, 1913, 1916, 1918, 1920,
    1922, 1925, 1927, 1929, 1932, 1934, 1936, 1938, 1941, 1943, 1945,
    1947, 1950, 1952, 1954, 1956, 1958, 1961, 1963, 1965, 1967, 1969,
    1972, 1974, 1976, 1978, 1980, 1983, 1985, 1987, 1989, 1991, 1993,
    1996, 1998, 2000, 2002, 2004, 2006, 2008, 2010, 2013, 2015, 2017,
    2019, 2021, 2023, 2025, 2027, 2029, 2031, 2033, 2036, 2038, 2040,
    2042, 2044, 2046, 2048, 2050, 2052, 2054, 2056, 2058, 2060, 2062,
    2064, 2066, 2068, 2070, 2072, 2074, 2076, 2078, 2080, 2082, 2084,
    2086, 2088, 2090, 2092, 2094, 2096, 2098, 2100, 2102, 2104, 2106,
    2108, 2109, 2111, 2113, 2115, 2117, 2119, 2121, 2123, 2125, 2127,
    2129, 2131, 2132, 2134, 2136, 2138, 2140, 2142, 2144, 2146, 2147,
    2149, 2151, 2153, 2155, 2157, 2159, 2160, 2162, 2164, 2166, 2168,
    2170, 2172, 2173, 2175, 2177, 2179, 2181, 2182, 2184, 2186, 2188,
    2190, 2191, 2193, 2195, 2197, 2199, 2200, 2202, 2204, 2206, 2207,
    2209, 2211, 2213, 2215, 2216, 2218, 2220, 2222, 2223, 2225, 2227,
    2229, 2230, 2232, 2234, 2235, 2237, 2239, 2241, 2242, 2244, 2246,
    2247, 2249, 2251, 2253, 2254, 2256, 2258, 2259, 2261, 2263, 2264,
    2266, 2268, 2269, 2271, 2273, 2274, 2276, 2278, 2279, 2281, 2283,
    2284, 2286, 2288, 2289, 2291, 2293, 2294, 2296, 2298, 2299, 2301,
    2302, 2304, 2306, 2307, 2309, 2311, 2312, 2314, 2315, 2317, 2319,
    2320, 2322, 2323, 2325, 2327, 2328, 2330, 2331, 2333, 2334, 2336,
    2338, 2339, 2341, 2342, 2344, 2345, 2347, 2349, 2350, 2352, 2353,
    2355, 2356, 2358, 2359, 2361, 2362, 2364, 2366, 2367, 2369, 2370,
    2372, 2373, 2375, 2376, 2378, 2379, 2381, 2382, 2384, 2385, 2387,
    2388, 2390, 2391, 2393, 2394, 2396, 2397, 2399, 2400, 2402, 2403,
    2405, 2406, 2408, 2409, 2411, 2412, 2414, 2415, 2417, 2418, 2419,
    2421, 2422, 2424, 2425, 2427, 2428, 2430, 2431, 2433, 2434, 2435,
    2437, 2438, 2440, 2441, 2443, 2444, 2445, 2447, 2448, 2450, 2451,
    2453, 2454, 2455, 2457, 2458, 2460, 2461, 2462, 2464, 2465, 2467,
    2468, 2469, 2471, 2472, 2474, 2475, 2476, 2478, 2479, 2481, 2482,
    2483, 2485, 2486, 2488, 2489, 2490, 2492, 2493, 2494, 2496, 2497,
    2498, 2500, 2501, 2503, 2504, 2505, 2507, 2508, 2509, 2511, 2512,
    2513, 2515, 2516, 2517, 2519, 2520, 2521, 2523, 2524, 2525, 2527,
    2528, 2529, 2531, 2532, 2533, 2535, 2536, 2537, 2539, 2540, 2541,
    2542, 2544, 2545, 2546, 2548, 2549, 2550, 2552, 2553, 2554, 2555,
    2557, 2558, 2559, 2561, 2562, 2563, 2565, 2566, 2567, 2568, 2570,
    2571, 2572, 2573, 2575, 2576, 2577, 2579, 2580, 2581, 2582, 2584,
    2585, 2586, 2587, 2589, 2590, 2591, 2592, 2594, 2595, 2596, 2597,
    2599, 2600, 2601, 2602, 2604, 2605, 2606, 2607, 2609, 2610, 2611,
    2612, 2613, 2615, 2616, 2617, 2618, 2620, 2621, 2622, 2623, 2624,
    2626, 2627, 2628, 2629, 2630, 2632, 2633, 2634, 2635, 2636, 2638,
    2639, 2640, 2641, 2642, 2644, 2645, 2646, 2647, 2648, 2650, 2651,
    2652, 2653, 2654, 2656, 2657, 2658, 2659, 2660, 2661, 2663, 2664,
    2665, 2666, 2667, 2668, 2670, 2671, 2672, 2673, 2674, 2675, 2677,
    2678, 2679, 2680, 2681, 2682, 2684, 2685, 2686, 2687, 2688, 2689,
    2690, 2692, 2693, 2694, 2695, 2696, 2697, 2698, 2700, 2701, 2702,
    2703, 2704, 2705, 2706, 2707, 2709, 2710, 2711, 2712, 2713, 2714,
    2715, 2716, 2718, 2719, 2720, 2721, 2722, 2723, 2724, 2725, 2726,
    2728, 2729, 2730, 2731, 2732, 2733, 2734, 2735, 2736, 2737, 2739,
    2740, 2741, 2742, 2743, 2744, 2745, 2746, 2747, 2748, 2750, 2751
};

#endif  /* FMOD_SUPPORT_IT */

#endif /* FMOD_NO_FPU */



#if defined(FMOD_SUPPORT_MOD) || defined(FMOD_SUPPORT_S3M) || defined(FMOD_SUPPORT_XM) || defined(FMOD_SUPPORT_IT)


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
// VERIFY: 687ecbea manual
// OFFSET: 0x00935520
FMOD_RESULT MusicSong::play()
{
    FMOD_RESULT result;
	int count;

	result = stop();
    if (result != FMOD_OK)
    {
        return result;
    }

    mGlobalVolume       = mDefaultGlobalVolume;
 	mSpeed              = mDefaultSpeed;
	mRow                = 0;
	mOrder              = 0;
	mNextOrder          = 0;
	mNextRow            = 0;
	mMixerSamplesLeft   = 0;
	mTick               = 0;
	mPatternDelay       = 0;
	mPatternDelayTicks  = 0;
    mPCMOffset          = 0;
	mFinished           = false;

    new (&mChannelGroup) ChannelGroupI();
    mChannelGroup.mVolume  = 1.0f;
    mChannelGroup.mDSPHead = mDSPHead;

    if (mVisited)
    {
        FMOD_memset(mVisited, 0, sizeof(bool) * mNumOrders * FMUSIC_MAXROWS);
    }

	setBPM(mDefaultBPM);

    if (mNumChannels && mMusicChannel)
    {
    	MusicChannel *cptr;

	    for (count = 0; count < mNumChannels; count++)
	    {
		    cptr = mMusicChannel[count];

            FMOD_memset(cptr, 0, sizeof(MusicChannel));
            cptr->mVirtualChannelHead.initNode();
		    cptr->mGlobalVolume = mDefaultVolume[count];
		    cptr->mPan          = mDefaultPan[count];
	    }
    }

    if (mNumVirtualChannels)
    {
    	MusicVirtualChannel *vcptr;

	    for (count = 0; count < mNumVirtualChannels; count++)
	    {
		    vcptr = &mVirtualChannel[count];

            FMOD_memset(vcptr, 0, sizeof(MusicVirtualChannel));
            vcptr->mChannel.init();
            vcptr->mIndex = count;
            vcptr->mChannel.mIndex = count;
            vcptr->mSong = this;
	    }
    }

	mPlaying = true;

	return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
// VERIFY: 1e15375b manual
// OFFSET: 0x00934E40
FMOD_RESULT MusicSong::spawnNewVirtualChannel(MusicChannel *cptr, MusicSample *sptr, MusicVirtualChannel **newvcptr)
{
    MusicVirtualChannel *vcptr = 0;
    int count;
    
    for (count = 0; count < mNumVirtualChannels; count++)
    {
        if (!mVirtualChannel[count].mAllocated)
        {
            vcptr = &mVirtualChannel[count];
            vcptr->mAllocated = true;
            break;
        }
    }
    if (!vcptr)
    {
        return FMOD_ERR_INTERNAL;
    }
	
	/*
        Insert the virtual channel as the first node in the fmusic_channel's vchannel list
    */
    vcptr->addAfter(&cptr->mVirtualChannelHead);
	
	/*
        Initialize some stuff
    */
	vcptr->mBackground      = false;

	vcptr->mEnvVolume.mTick     = 0;
	vcptr->mEnvVolume.mPosition = 0;
	vcptr->mEnvVolume.mValue    = 64;
	vcptr->mEnvVolume.mFraction = 64 << 16;
	vcptr->mEnvVolume.mDelta    = 0;
	vcptr->mEnvVolume.mStopped  = false;
						
	vcptr->mEnvPan.mTick     = 0;
	vcptr->mEnvPan.mPosition = 0;			
	vcptr->mEnvPan.mValue    = 128;
	vcptr->mEnvPan.mFraction = 128 << 16;
	vcptr->mEnvPan.mDelta    = 0;
	vcptr->mEnvPan.mStopped  = false;
						
	vcptr->mEnvPitchTick    = 0;
	vcptr->mEnvPitchPos     = 0;
	vcptr->mEnvPitchFrac    = 0;
	vcptr->mEnvPitch        = 0;
	vcptr->mEnvPitchDelta   = 0;
	vcptr->mEnvPitchStopped = false;

	vcptr->mFadeOutVolume   = 1024;

    if (newvcptr)
    {
        *newvcptr = vcptr;
    }
    
	return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
// VERIFY: d7a3402d auto
// OFFSET: 0x00934F40
FMOD_RESULT MusicVirtualChannel::cleanUp()
{
    bool playing = false;

    mChannel.isPlaying(&playing);
    
	if (!playing)
	{	
        if (mSong->mLowPass)
        {
            mSong->mLowPass[mChannel.mIndex]->remove();
        }

		mNoteControl = 0;	
		removeNode();            /* unlink channel from mod's mixlist */
        mAllocated = false;
	}

	return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
// VERIFY: 46e32ffe auto
// OFFSET: 0x00934FB0
FMOD_RESULT MusicSong::setBPM(int bpm)
{
	float hz;

	if (bpm < 1)
    {
		bpm = 1;
    }

	mBPM = bpm;

	hz = (float)bpm * 2.0f / 5.0f;
	hz *= mMasterSpeed;

	/* number of samples */
    if (hz >= 0.01f)
    {
	    mMixerSamplesPerTick = (int)((float)waveformat[0].frequency / hz);
    }

    #ifdef FMOD_SUPPORT_HARDWAREXM
    #ifdef FMOD_SUPPORT_SOFTWARE
    if (!mChannelSoftware)
    #endif
    {
        mSystem->mStreamThread.setPeriod(1000 / hz);
    }
    #endif

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
// VERIFY: fa086cbf manual
// OFFSET: 0x00935010
FMOD_RESULT MusicSong::stop()
{
    int count;

	mPlaying = false;
    mFinished = true;
	
    if (mMusicChannel)
    {
	    /*
            Stop everything playing
        */
	    for (count = 0; count < mNumChannels; count++)
	    {
            MusicChannel *cptr;

		    cptr = mMusicChannel[count];

            if (cptr && cptr->mVirtualChannelHead.getNext())
            {       
                while (!cptr->mVirtualChannelHead.isEmpty())
                {
                    MusicVirtualChannel *vcptr = (MusicVirtualChannel *)cptr->mVirtualChannelHead.getNext();
		            vcptr->mChannel.stopEx(false, false, true, true, false, false, false);           /* stop this channel */
                    vcptr->mChannel.mRealChannel[0] = 0;
                    if (mLowPass)
                    {
                        mLowPass[vcptr->mChannel.mIndex]->remove();
                    }
                    vcptr->cleanUp();
                }
            }
	    }

        for (count = 0; count < mNumVirtualChannels; count++)
        {
            #ifdef FMOD_SUPPORT_HARDWAREXM
            #ifdef FMOD_SUPPORT_SOFTWARE
            if (!mChannelSoftware)
            #endif
            {
                FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "MusicSong::stop", "UNRESERVING HW VOICE %d / %d\n", count, mNumVirtualChannels));        

                mSystem->mOutput->reserveVoice(count, false);
            }
            #endif
        }
    }

    #ifdef FMOD_SUPPORT_HARDWAREXM
    #ifdef FMOD_SUPPORT_SOFTWARE
    if (!mChannelSoftware)
    #endif
    {
        mSystem->mStreamThread.setPeriod(10);
    }
    #endif

	return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
// VERIFY: 0d991fb3 manual
// OFFSET: 0x00935150
FMOD_RESULT MusicSong::playSound(MusicSample *sample, MusicVirtualChannel *vcptr, bool addfilter, SNDMIXPLUGIN *plugin)
{
    FMOD_RESULT result;
    ChannelI    *channel = &vcptr->mChannel;
    ChannelReal *realchannel = channel->mRealChannel[0];


    result = mChannelPool->allocateChannel(&realchannel, vcptr->mFlip ? vcptr->mIndex + mNumVirtualChannels : vcptr->mIndex, 1, 0);
    if (result != FMOD_OK)
    {
        return result;
    }

    //FLOG((FMOD_DEBUG_LEVEL_ERROR, __FILE__, __LINE__, "MusicSong::playSound", "use vcptr->mIndex %d : flip = %d.\n", vcptr->mIndex, vcptr->mFlip));        

    vcptr->mFlip = !vcptr->mFlip;

    if (channel->mRealChannel[0])
    {
        vcptr->mChannel.setVolume(0);
    }
    channel->mRealChannel[0] = realchannel;

    /*
        If this channel is using an FX send, send it to the plugin DSP, otherwise send it to the normal mixing group.
    */
    if (plugin)
    {
        channel->mChannelGroup = &plugin->mChannelGroup;
    }
    else
    {
        channel->mChannelGroup = &mChannelGroup;
    }

    result = channel->play(sample->mSound, true, true, false);
    if (result != FMOD_OK)
    {
        channel->stopEx(false, false, true, true, false, false, false);
        return result;
    }
    if (vcptr->mSampleOffset > 0)
    {
        channel->setPosition(vcptr->mSampleOffset, FMOD_TIMEUNIT_PCM);
        vcptr->mSampleOffset = 0;
    }

    if (mLowPass)
    {
        mLowPass[channel->mIndex]->remove();
        if (addfilter)
        {
            channel->addDSP(mLowPass[channel->mIndex]);
        }
    }

    channel->setPaused(false);

    /*
        It would be nice if this only did the ones that were connected in the mod player instead of flushing everything.
        If we dont do this the connects/disconnects dont happen until the main sfx mixer feels like it.
    */
    mSystem->flushDSPConnectionRequests();

    return FMOD_OK;    
}

/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
// VERIFY: 4424f7bb auto
// OFFSET: 0x00935280
FMOD_RESULT MusicSong::fineTune2Hz(unsigned char ft, unsigned int *hz) 
{
    if (!hz)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

	switch(ft) 
	{
		case  0 : *hz = 8363; break;
		case  1 : *hz = 8413; break;
		case  2 : *hz = 8463; break;
		case  3 : *hz = 8529; break;
		case  4 : *hz = 8581; break;
		case  5 : *hz = 8651; break;
		case  6 : *hz = 8723; break;
		case  7 : *hz = 8757; break;
		case  8 : *hz = 7895; break;
		case  9 : *hz = 7941; break;
		case 10 : *hz = 7985; break;
		case 11 : *hz = 8046; break;
		case 12 : *hz = 8107; break;
		case 13 : *hz = 8169; break;
		case 14 : *hz = 8232; break;
		case 15 : *hz = 8280; break;
		default : *hz = 8363; break;
	};

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
// VERIFY: d9aa76bc auto
// OFFSET: 0x009353B0
FMOD_RESULT MusicSong::getLengthInternal(unsigned int *length, FMOD_TIMEUNIT lengthtype)
{
    if (lengthtype == FMOD_TIMEUNIT_MODORDER)
    {
        *length = mNumOrders;

        return FMOD_OK;
    }
    else if (lengthtype == FMOD_TIMEUNIT_MODPATTERN)
    {
        *length = mNumPatterns;

        return FMOD_OK;
    }
    else if (lengthtype == FMOD_TIMEUNIT_MODROW)
    {
        *length = mPattern[mOrderList[mOrder]].mRows;

        return FMOD_OK;
    }

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
// VERIFY: 6f65adfd auto
// OFFSET: 0x00935410
FMOD_RESULT MusicSong::getPositionInternal(unsigned int *position, FMOD_TIMEUNIT postype)
{
    if (postype == FMOD_TIMEUNIT_MODORDER)
    {
        *position = mOrder;

        return FMOD_OK;
    }
    else if (postype == FMOD_TIMEUNIT_MODPATTERN)
    {
        *position = mOrderList[mOrder];

        return FMOD_OK;
    }
    else if (postype == FMOD_TIMEUNIT_MODROW)
    {
        *position = mRow;

        return FMOD_OK;
    }

    return FMOD_OK;
}




/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
// VERIFY: e6a96ab4 manual
// OFFSET: 0x00935470
FMOD_RESULT F_CALLBACK MusicSong::getLengthCallback(FMOD_CODEC_STATE *codec, unsigned int *length, FMOD_TIMEUNIT lengthtype)
{
    MusicSong *cmusic = (MusicSong *)codec;

    return cmusic->getLengthInternal(length, lengthtype);
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
// VERIFY: 9ffe9033 manual
// OFFSET: 0x009354B0
FMOD_RESULT F_CALLBACK MusicSong::getPositionCallback(FMOD_CODEC_STATE *codec, unsigned int *position, FMOD_TIMEUNIT postype)
{
    MusicSong *cmusic = (MusicSong *)codec;

    return cmusic->getPositionInternal(position, postype);
}

#endif

}

#endif
