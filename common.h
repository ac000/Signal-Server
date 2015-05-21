#ifndef _COMMON_H_
#define _COMMON_H_

#ifdef HD
   // HD mode, 30m res
  #define MAXPAGES 9
  #define ARRAYSIZE 14844
  #define IPPD 3600
#else
  // 90m mode (default)
  #define MAXPAGES 64
  #define ARRAYSIZE 76810
  #define IPPD 1200
#endif

struct dem {
	int min_north;
	int max_north;
	int min_west;
	int max_west;
	int max_el;
	int min_el;
	short data[IPPD][IPPD];
	unsigned char mask[IPPD][IPPD];
	unsigned char signal[IPPD][IPPD];
};

#endif /* _COMMON_H_ */
