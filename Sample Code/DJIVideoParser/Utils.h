#ifndef __UTILS_H__
#define __UTILS_H__
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#ifdef WIN32
#include <windows.h>
#include <stdint.h>
#else
#include <sys/time.h>
#include <unistd.h>
#endif

int32_t getTickCount();

long getTickCount2();

long djiSleep(long long  ns);

int32_t findSPSPPSHeader(unsigned char* bufPtr, int bufLen, unsigned char* nalBuf, int* nalLen);

//int32_t kmpMatch(uint8_t* str, int ls, uint8_t* pat, int lp);

int	h264_decode_seq_parameter_set(unsigned char * buf, unsigned int nLen, int *Width, int *Height, int *framerate, int *log2_max_frame_num);

int find_SPS_PPS(uint8_t* pInBuff, int iSize, uint8_t* pSPS, int* iSpsLen, uint8_t* pPpsBuf, int* iPpsLen);

void parseSpsPps(uint8_t* _buffer, int checkEnd, int& sps_start, int& sps_size, int&pps_start, int&pps_size);

int32_t convertOSD(uint8_t* osdBuf, int osdLen, uint8_t* convBuf, int* convLen);

int findNALU(void* buffer, int i, int end, int mask, int compare);

//long djiSleep(long delay);

long getComprehensivePts(long ptsMs, long frameIndex, long frameNum);

#endif

