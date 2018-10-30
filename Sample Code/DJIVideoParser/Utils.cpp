#include "pch.h"
#include "Utils.h"
#include <time.h>
#include <math.h>

enum AVColorPrimaries {
	AVCOL_PRI_BT709 = 1, ///< also ITU-R BT1361 / IEC 61966-2-4 / SMPTE RP177 Annex B
	AVCOL_PRI_UNSPECIFIED = 2,
	AVCOL_PRI_BT470M = 4,
	AVCOL_PRI_BT470BG = 5, ///< also ITU-R BT601-6 625 / ITU-R BT1358 625 / ITU-R BT1700 625 PAL & SECAM
	AVCOL_PRI_SMPTE170M = 6, ///< also ITU-R BT601-6 525 / ITU-R BT1358 525 / ITU-R BT1700 NTSC
	AVCOL_PRI_SMPTE240M = 7, ///< functionally identical to above
	AVCOL_PRI_FILM = 8,
	AVCOL_PRI_BT2020 = 9, ///< ITU-R BT2020
	AVCOL_PRI_NB, ///< Not part of ABI
};

enum AVColorTransferCharacteristic {
	AVCOL_TRC_BT709 = 1, ///< also ITU-R BT1361
	AVCOL_TRC_UNSPECIFIED = 2,
	AVCOL_TRC_GAMMA22 = 4, ///< also ITU-R BT470M / ITU-R BT1700 625 PAL & SECAM
	AVCOL_TRC_GAMMA28 = 5, ///< also ITU-R BT470BG
	AVCOL_TRC_SMPTE170M = 6, ///< also ITU-R BT601-6 525 or 625 / ITU-R BT1358 525 or 625 / ITU-R BT1700 NTSC
	AVCOL_TRC_SMPTE240M = 7,
	AVCOL_TRC_LINEAR = 8, ///< "Linear transfer characteristics"
	AVCOL_TRC_LOG = 9, ///< "Logarithmic transfer characteristic (100:1 range)"
	AVCOL_TRC_LOG_SQRT = 10, ///< "Logarithmic transfer characteristic (100 * Sqrt( 10 ) : 1 range)"
	AVCOL_TRC_IEC61966_2_4 = 11, ///< IEC 61966-2-4
	AVCOL_TRC_BT1361_ECG = 12, ///< ITU-R BT1361 Extended Colour Gamut
	AVCOL_TRC_IEC61966_2_1 = 13, ///< IEC 61966-2-1 (sRGB or sYCC)
	AVCOL_TRC_BT2020_10 = 14, ///< ITU-R BT2020 for 10 bit system
	AVCOL_TRC_BT2020_12 = 15, ///< ITU-R BT2020 for 12 bit system
	AVCOL_TRC_NB, ///< Not part of ABI
};

enum AVColorSpace {
	AVCOL_SPC_RGB = 0,
	AVCOL_SPC_BT709 = 1, ///< also ITU-R BT1361 / IEC 61966-2-4 xvYCC709 / SMPTE RP177 Annex B
	AVCOL_SPC_UNSPECIFIED = 2,
	AVCOL_SPC_FCC = 4,
	AVCOL_SPC_BT470BG = 5, ///< also ITU-R BT601-6 625 / ITU-R BT1358 625 / ITU-R BT1700 625 PAL & SECAM / IEC 61966-2-4 xvYCC601
	AVCOL_SPC_SMPTE170M = 6, ///< also ITU-R BT601-6 525 / ITU-R BT1358 525 / ITU-R BT1700 NTSC / functionally identical to above
	AVCOL_SPC_SMPTE240M = 7,
	AVCOL_SPC_YCOCG = 8, ///< Used by Dirac / VC-2 and H.264 FRext, see ITU-T SG16
	AVCOL_SPC_BT2020_NCL = 9, ///< ITU-R BT2020 non-constant luminance system
	AVCOL_SPC_BT2020_CL = 10, ///< ITU-R BT2020 constant luminance system
	AVCOL_SPC_NB, ///< Not part of ABI
};

typedef struct AVRational {
	int num; ///< numerator
	int den; ///< denominator
} AVRational;

#define MAX_SPS_COUNT          32
#define MIN_LOG2_MAX_FRAME_NUM    4
#define MAX_LOG2_MAX_FRAME_NUM    (12 + 4)

#define FF_ARRAY_ELEMS(a) (sizeof(a) / sizeof((a)[0]))

#define EXTENDED_SAR       255

typedef struct SPS {
	unsigned int sps_id;
	int profile_idc;
	int level_idc;
	int chroma_format_idc;
	int transform_bypass;              ///< qpprime_y_zero_transform_bypass_flag
	int log2_max_frame_num;            ///< log2_max_frame_num_minus4 + 4
	int poc_type;                      ///< pic_order_cnt_type
	int log2_max_poc_lsb;              ///< log2_max_pic_order_cnt_lsb_minus4
	int delta_pic_order_always_zero_flag;
	int offset_for_non_ref_pic;
	int offset_for_top_to_bottom_field;
	int poc_cycle_length;              ///< num_ref_frames_in_pic_order_cnt_cycle
	int ref_frame_count;               ///< num_ref_frames
	int gaps_in_frame_num_allowed_flag;
	int mb_width;                      ///< pic_width_in_mbs_minus1 + 1
	int mb_height;                     ///< pic_height_in_map_units_minus1 + 1
	int frame_mbs_only_flag;
	int mb_aff;                        ///< mb_adaptive_frame_field_flag
	int direct_8x8_inference_flag;
	int crop;                          ///< frame_cropping_flag

									   /* those 4 are already in luma samples */
	unsigned int crop_left;            ///< frame_cropping_rect_left_offset
	unsigned int crop_right;           ///< frame_cropping_rect_right_offset
	unsigned int crop_top;             ///< frame_cropping_rect_top_offset
	unsigned int crop_bottom;          ///< frame_cropping_rect_bottom_offset
	int vui_parameters_present_flag;
	AVRational sar;
	int video_signal_type_present_flag;
	int full_range;
	int colour_description_present_flag;
	enum AVColorPrimaries color_primaries;
	enum AVColorTransferCharacteristic color_trc;
	enum AVColorSpace colorspace;
	int timing_info_present_flag;
	unsigned long num_units_in_tick;
	unsigned long time_scale;
	int fixed_frame_rate_flag;
	short offset_for_ref_frame[256]; // FIXME dyn aloc?
	int bitstream_restriction_flag;
	int num_reorder_frames;
	int scaling_matrix_present;
	unsigned char scaling_matrix4[6][16];
	unsigned char scaling_matrix8[6][64];
	int nal_hrd_parameters_present_flag;
	int vcl_hrd_parameters_present_flag;
	int pic_struct_present_flag;
	int time_offset_length;
	int cpb_cnt;                          ///< See H.264 E.1.2
	int initial_cpb_removal_delay_length; ///< initial_cpb_removal_delay_length_minus1 + 1
	int cpb_removal_delay_length;         ///< cpb_removal_delay_length_minus1 + 1
	int dpb_output_delay_length;          ///< dpb_output_delay_length_minus1 + 1
	int bit_depth_luma;                   ///< bit_depth_luma_minus8 + 8
	int bit_depth_chroma;                 ///< bit_depth_chroma_minus8 + 8
	int residual_color_transform_flag;    ///< residual_colour_transform_flag
	int constraint_set_flags;             ///< constraint_set[0-3]_flag
										  //int new;                              ///< flag to keep track if the decoder context needs re-init due to changed SPS
} SPS;

static const unsigned char default_scaling4[2][16] = {
	{ 6, 13, 20, 28, 13, 20, 28, 32,
	20, 28, 32, 37, 28, 32, 37, 42 },
{ 10, 14, 20, 24, 14, 20, 24, 27,
20, 24, 27, 30, 24, 27, 30, 34 }
};

static const unsigned char default_scaling8[2][64] = {
	{ 6, 10, 13, 16, 18, 23, 25, 27,
	10, 11, 16, 18, 23, 25, 27, 29,
	13, 16, 18, 23, 25, 27, 29, 31,
	16, 18, 23, 25, 27, 29, 31, 33,
	18, 23, 25, 27, 29, 31, 33, 36,
	23, 25, 27, 29, 31, 33, 36, 38,
	25, 27, 29, 31, 33, 36, 38, 40,
	27, 29, 31, 33, 36, 38, 40, 42 },
{ 9, 13, 15, 17, 19, 21, 22, 24,
13, 13, 17, 19, 21, 22, 24, 25,
15, 17, 19, 21, 22, 24, 25, 27,
17, 19, 21, 22, 24, 25, 27, 28,
19, 21, 22, 24, 25, 27, 28, 30,
21, 22, 24, 25, 27, 28, 30, 32,
22, 24, 25, 27, 28, 30, 32, 33,
24, 25, 27, 28, 30, 32, 33, 35 }
};

static const unsigned char zigzag_scan[16 + 1] = {
	0 + 0 * 4, 1 + 0 * 4, 0 + 1 * 4, 0 + 2 * 4,
	1 + 1 * 4, 2 + 0 * 4, 3 + 0 * 4, 2 + 1 * 4,
	1 + 2 * 4, 0 + 3 * 4, 1 + 3 * 4, 2 + 2 * 4,
	3 + 1 * 4, 3 + 2 * 4, 2 + 3 * 4, 3 + 3 * 4,
};

static const unsigned char ff_zigzag_direct[64] = {
	0,   1,  8, 16,  9,  2,  3, 10,
	17, 24, 32, 25, 18, 11,  4,  5,
	12, 19, 26, 33, 40, 48, 41, 34,
	27, 20, 13,  6,  7, 14, 21, 28,
	35, 42, 49, 56, 57, 50, 43, 36,
	29, 22, 15, 23, 30, 37, 44, 51,
	58, 59, 52, 45, 38, 31, 39, 46,
	53, 60, 61, 54, 47, 55, 62, 63
};

static unsigned int Ue(unsigned char *pBuff, unsigned int nLen, unsigned int *nStartBit)
{
	unsigned int nZeroNum = 0;
	while (*nStartBit < nLen * 8)
	{
		if (pBuff[*nStartBit / 8] & (0x80 >> (*nStartBit % 8)))
		{
			break;
		}
		nZeroNum++;
		(*nStartBit)++;
	}
	(*nStartBit)++;

	unsigned long dwRet = 0;
	unsigned int i;
	for (i = 0; i<nZeroNum; i++)
	{
		dwRet <<= 1;
		if (pBuff[*nStartBit / 8] & (0x80 >> (*nStartBit % 8)))
		{
			dwRet += 1;
		}
		(*nStartBit)++;
	}

	return (1 << nZeroNum) - 1 + dwRet;
}


static int Se(unsigned char *pBuff, unsigned int nLen, unsigned int *nStartBit)
{
	int UeVal = Ue(pBuff, nLen, nStartBit);
	double k = UeVal;
	int nValue = ceil(k / 2);
	if (UeVal % 2 == 0)
	{
		nValue = -nValue;
	}

	return nValue;
}

static unsigned long u(unsigned int BitCount, unsigned char * buf, unsigned int *nStartBit)
{
	unsigned long dwRet = 0;
	unsigned int i;
	for (i = 0; i<BitCount; i++)
	{
		dwRet <<= 1;
		if (buf[*nStartBit / 8] & (0x80 >> (*nStartBit % 8)))
		{
			dwRet += 1;
		}
		(*nStartBit)++;
	}
	return dwRet;
}




#ifdef WIN32

//struct timespec { long tv_sec; long tv_nsec; };    //header part

int clock_gettime(int, struct timespec *spec)      //C-file part
{
	__int64 wintime; GetSystemTimeAsFileTime((FILETIME*)&wintime);
	wintime -= 116444736000000000i64;  //1jan1601 to 1jan1970
	spec->tv_sec = wintime / 10000000i64;           //seconds
	spec->tv_nsec = wintime % 10000000i64 * 100;      //nano-seconds
	return 0;
}

/* Windows sleep in 100ns units */
BOOLEAN nanosleep(LONGLONG ns) {
	/* Declarations */
	HANDLE timer;	/* Timer handle */
	LARGE_INTEGER li;	/* Time defintion */
						/* Create timer */
	if (!(timer = CreateWaitableTimer(NULL, TRUE, NULL)))
		return FALSE;
	/* Set timer properties */
	li.QuadPart = -ns;
	if (!SetWaitableTimer(timer, &li, 0, NULL, NULL, FALSE)) {
		CloseHandle(timer);
		return FALSE;
	}
	/* Start & wait for timer */
	WaitForSingleObject(timer, INFINITE);
	/* Clean resources */
	CloseHandle(timer);
	/* Slept without problems */
	return TRUE;
}

long djiSleep(LONGLONG  ns) {

	return nanosleep(ns);
}

int getTickCount() {
	struct timespec ts;
	clock_gettime(0, &ts);
	return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

long getTickCount2() {
	struct timespec ts;
	clock_gettime(0, &ts);
	return (ts.tv_sec * 1000000 + ts.tv_nsec / 1000);
}

#else

long djiSleep(long long delay) {
	struct timespec req, rem;
	rem.tv_sec = 0;
	rem.tv_nsec = 0;
	req.tv_sec = 0;
	req.tv_nsec = delay * 1000;
	return nanosleep(&req, &rem);
}

int getTickCount() {
	struct timespec ts;
	clock_gettime((clockid_t)0, &ts);
	return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

long getTickCount2() {
	struct timespec ts;
	clock_gettime((clockid_t)0, &ts);
	return (ts.tv_sec * 1000000 + ts.tv_nsec / 1000);
}

#endif





int32_t findSPSPPSHeader(unsigned char* bufPtr, int bufLen, unsigned char* nalBuf, int* nalLen) {
	int i;
	bool found = false;

	if ((bufLen < 5) && (bufPtr != NULL)) {
		return -1;
	}

	for (i = 0; i < bufLen - 5; i++) {
		if ((bufPtr[i] == 0x00) && (bufPtr[i + 1] == 0x00) && (bufPtr[i + 2] == 0x00) && (bufPtr[i + 3] == 0x01) && (bufPtr[i + 4] == 0x67)) {
			// Found a NAL unit with 4-byte startcode, do something with it
			*nalLen = bufLen - i;
			//nalBuf = recvBuf + i;
			//memcpy(nalBuf, bufPtr + i, *nalLen);
			found = true;
			break;
		}
		//bufPtr++;
	}
	return (found ? (0) : (-1));
}

//���� 00 00 00 01��ʶ�������ʶ��buffer�е�λ�ã��Լ����ر�ʶ�ĸ���
int32_t findHeadMarker(unsigned char* bufPtr, int bufLen, int *Offset) {
	int findCnt = 0;
	int i = 0;

	for (i = 0; i < bufLen - 4; i++) {
		if ((bufPtr[i] == 0x00) && (bufPtr[i + 1] == 0x00) && (bufPtr[i + 2] == 0x00) && (bufPtr[i + 3] == 0x01)) {
			*(Offset + findCnt) = i;
			findCnt++;

			if (findCnt >= 100) {
				break;
			}
		}
	}

	return findCnt;
}

static void decode_scaling_list(
	unsigned char * buf,
	unsigned int nLen,
	unsigned int *StartBit,
	unsigned char *factors,
	int size,
	const unsigned char *jvt_list,
	const unsigned char *fallback_list)
{
	int i, last = 8, next = 8;
	const unsigned char *scan = size == 16 ? zigzag_scan : ff_zigzag_direct;
	if (!u(1, buf, StartBit))
	{
		/* matrix not written, we use the predicted one */
		memcpy(factors, fallback_list, size * sizeof(unsigned char));
	}
	else
	{
		for (i = 0; i < size; i++)
		{
			if (next)
			{
				next = (last + Se(buf, nLen, StartBit)) & 0xff;
			}
			if (!i && !next)
			{
				/* matrix not written, we use the preset one */
				memcpy(factors, jvt_list, size * sizeof(unsigned char));
				break;
			}
			last = factors[scan[i]] = next ? next : last;
		}
	}
}

int	h264_decode_seq_parameter_set(unsigned char * buf, unsigned int nLen, int *Width, int *Height, int *framerate, int *log2_max_frame_num)
{
	unsigned int StartBit = 0;
	int profile_idc, level_idc, constraint_set_flags = 0;
	unsigned int sps_id;
	int i, log2_max_frame_num_minus4;
	SPS	tSPS;
	SPS	*sps = &tSPS;

	//skip 0x67
	u(8, buf, &StartBit);

	profile_idc = u(8, buf, &StartBit);
	constraint_set_flags |= u(1, buf, &StartBit) << 0;   // constraint_set0_flag
	constraint_set_flags |= u(1, buf, &StartBit) << 1;   // constraint_set1_flag
	constraint_set_flags |= u(1, buf, &StartBit) << 2;   // constraint_set2_flag
	constraint_set_flags |= u(1, buf, &StartBit) << 3;   // constraint_set3_flag
	constraint_set_flags |= u(1, buf, &StartBit) << 4;   // constraint_set4_flag
	constraint_set_flags |= u(1, buf, &StartBit) << 5;   // constraint_set5_flag
	u(2, buf, &StartBit);
	level_idc = u(8, buf, &StartBit);
	sps_id = Ue(buf, nLen, &StartBit);
	if (sps_id >= MAX_SPS_COUNT)
	{
		printf("sps_id error\n");
		return -1;
	}

	sps->sps_id = sps_id;
	sps->time_offset_length = 24;
	sps->profile_idc = profile_idc;
	sps->constraint_set_flags = constraint_set_flags;
	sps->level_idc = level_idc;
	sps->full_range = -1;
	memset(sps->scaling_matrix4, 16, sizeof(sps->scaling_matrix4));
	memset((void *)sps->scaling_matrix8, 16, sizeof(sps->scaling_matrix8));
	sps->scaling_matrix_present = 0;
	sps->colorspace = (AVColorSpace)2; //AVCOL_SPC_UNSPECIFIED

	if ((sps->profile_idc == 100)
		|| (sps->profile_idc == 110)
		|| (sps->profile_idc == 122)
		|| (sps->profile_idc == 244)
		|| (sps->profile_idc == 44)
		|| (sps->profile_idc == 83)
		|| (sps->profile_idc == 86)
		|| (sps->profile_idc == 118)
		|| (sps->profile_idc == 128)
		|| (sps->profile_idc == 144))
	{
		sps->chroma_format_idc = Ue(buf, nLen, &StartBit);
		if (sps->chroma_format_idc > 3U)
		{
			printf("chroma_format_idc error\n");
			return -1;
		}
		else if (sps->chroma_format_idc == 3)
		{
			sps->residual_color_transform_flag = u(1, buf, &StartBit);
			if (sps->residual_color_transform_flag)
			{
				printf("residual_color_transform_flag error\n");
				return -1;
			}
		}
		sps->bit_depth_luma = Ue(buf, nLen, &StartBit) + 8;
		sps->bit_depth_chroma = Ue(buf, nLen, &StartBit) + 8;
		if (sps->bit_depth_chroma != sps->bit_depth_luma)
		{
			printf("bit_depth_chroma1 error\n");
			return -1;
		}
		if (sps->bit_depth_luma > 14U || sps->bit_depth_chroma > 14U)
		{
			printf("bit_depth_chroma2 error\n");
			return -1;
		}
		sps->transform_bypass = u(1, buf, &StartBit);

		int is_sps = 1;
		int fallback_sps = !is_sps && sps->scaling_matrix_present;
		const unsigned char *fallback[4] =
		{
			fallback_sps ? sps->scaling_matrix4[0] : default_scaling4[0],
			fallback_sps ? sps->scaling_matrix4[3] : default_scaling4[1],
			fallback_sps ? sps->scaling_matrix8[0] : default_scaling8[0],
			fallback_sps ? sps->scaling_matrix8[3] : default_scaling8[1]
		};

		if (u(1, buf, &StartBit))
		{
			sps->scaling_matrix_present |= is_sps;
			decode_scaling_list(buf, nLen, &StartBit, sps->scaling_matrix4[0], 16, default_scaling4[0], fallback[0]);        // Intra, Y
			decode_scaling_list(buf, nLen, &StartBit, sps->scaling_matrix4[1], 16, default_scaling4[0], sps->scaling_matrix4[0]); // Intra, Cr
			decode_scaling_list(buf, nLen, &StartBit, sps->scaling_matrix4[2], 16, default_scaling4[0], sps->scaling_matrix4[1]); // Intra, Cb
			decode_scaling_list(buf, nLen, &StartBit, sps->scaling_matrix4[3], 16, default_scaling4[1], fallback[1]);        // Inter, Y
			decode_scaling_list(buf, nLen, &StartBit, sps->scaling_matrix4[4], 16, default_scaling4[1], sps->scaling_matrix4[3]); // Inter, Cr
			decode_scaling_list(buf, nLen, &StartBit, sps->scaling_matrix4[5], 16, default_scaling4[1], sps->scaling_matrix4[4]); // Inter, Cb
			if (is_sps)
			{
				decode_scaling_list(buf, nLen, &StartBit, sps->scaling_matrix8[0], 64, default_scaling8[0], fallback[2]); // Intra, Y
				decode_scaling_list(buf, nLen, &StartBit, sps->scaling_matrix8[3], 64, default_scaling8[1], fallback[3]); // Inter, Y
				if (sps->chroma_format_idc == 3)
				{
					decode_scaling_list(buf, nLen, &StartBit, sps->scaling_matrix8[1], 64, default_scaling8[0], sps->scaling_matrix8[0]); // Intra, Cr
					decode_scaling_list(buf, nLen, &StartBit, sps->scaling_matrix8[4], 64, default_scaling8[1], sps->scaling_matrix8[3]); // Inter, Cr
					decode_scaling_list(buf, nLen, &StartBit, sps->scaling_matrix8[2], 64, default_scaling8[0], sps->scaling_matrix8[1]); // Intra, Cb
					decode_scaling_list(buf, nLen, &StartBit, sps->scaling_matrix8[5], 64, default_scaling8[1], sps->scaling_matrix8[4]); // Inter, Cb
				}
			}
		}
	}
	else
	{
		sps->chroma_format_idc = 1;
		sps->bit_depth_luma = 8;
		sps->bit_depth_chroma = 8;
	}
	log2_max_frame_num_minus4 = Ue(buf, nLen, &StartBit);
	if ((log2_max_frame_num_minus4 < MIN_LOG2_MAX_FRAME_NUM - 4)
		|| (log2_max_frame_num_minus4 > MAX_LOG2_MAX_FRAME_NUM - 4))
	{
		printf("log2_max_frame_num_minus4 error\n");
		return -1;
	}
	sps->log2_max_frame_num = log2_max_frame_num_minus4 + 4;
	*log2_max_frame_num = sps->log2_max_frame_num;
	//LOGE("extractSize()  poc1 %d, %d, %d",StartBit, buf[5], nLen);
	sps->poc_type = Ue(buf, nLen, &StartBit);
	//LOGE("extractSize()  poc2 %d",StartBit);
	//LOGE("extractSize()  poc %d",sps->poc_type);


	if (sps->poc_type == 0)
	{
		// FIXME #define
		unsigned t = Ue(buf, nLen, &StartBit);
		if (t>12)
		{
			printf("t error\n");
			return -1;
		}
		sps->log2_max_poc_lsb = t + 4;
	}
	else if (sps->poc_type == 1)
	{
		// FIXME #define
		sps->delta_pic_order_always_zero_flag = u(1, buf, &StartBit);
		sps->offset_for_non_ref_pic = Se(buf, nLen, &StartBit);
		sps->offset_for_top_to_bottom_field = Se(buf, nLen, &StartBit);
		sps->poc_cycle_length = Ue(buf, nLen, &StartBit);

		if ((unsigned)sps->poc_cycle_length >= FF_ARRAY_ELEMS(sps->offset_for_ref_frame))
		{
			printf("poc_cycle_length error\n");
			return -1;
		}

		for (i = 0; i < sps->poc_cycle_length; i++)
		{
			sps->offset_for_ref_frame[i] = Se(buf, nLen, &StartBit);
		}

	}
	else if (sps->poc_type != 2)
	{
		printf("poc_type error\n");
		return -1;
	}

	sps->ref_frame_count = Ue(buf, nLen, &StartBit);
	sps->gaps_in_frame_num_allowed_flag = u(1, buf, &StartBit);
	sps->mb_width = Ue(buf, nLen, &StartBit);
	sps->mb_height = Ue(buf, nLen, &StartBit);

	*Width = (sps->mb_width + 1) * 16;
	*Height = (sps->mb_height + 1) * 16;

	sps->frame_mbs_only_flag = u(1, buf, &StartBit);
	if (!sps->frame_mbs_only_flag)
	{
		sps->mb_aff = u(1, buf, &StartBit);
	}

	sps->direct_8x8_inference_flag = u(1, buf, &StartBit);

	sps->crop = u(1, buf, &StartBit);
	if (sps->crop)
	{
		//crop_left
		Ue(buf, nLen, &StartBit);
		//crop_right
		Ue(buf, nLen, &StartBit);
		//crop_top
		Ue(buf, nLen, &StartBit);
		//crop_bottom
		Ue(buf, nLen, &StartBit);
	}

	sps->vui_parameters_present_flag = u(1, buf, &StartBit);
	if (sps->vui_parameters_present_flag)
	{
		int aspect_ratio_info_present_flag;
		unsigned int aspect_ratio_idc;

		aspect_ratio_info_present_flag = u(1, buf, &StartBit);

		if (aspect_ratio_info_present_flag)
		{
			aspect_ratio_idc = u(8, buf, &StartBit);
			if (aspect_ratio_idc == EXTENDED_SAR)
			{
				sps->sar.num = u(16, buf, &StartBit);
				sps->sar.den = u(16, buf, &StartBit);
			}
		}

		if (u(1, buf, &StartBit))
		{
			u(1, buf, &StartBit);
		}

		sps->video_signal_type_present_flag = u(1, buf, &StartBit);
		if (sps->video_signal_type_present_flag)
		{
			u(3, buf, &StartBit);                 /* video_format */
			sps->full_range = u(1, buf, &StartBit); /* video_full_range_flag */

			sps->colour_description_present_flag = u(1, buf, &StartBit);
			if (sps->colour_description_present_flag)
			{
				sps->color_primaries = (AVColorPrimaries)u(8, buf, &StartBit); /* colour_primaries */
				sps->color_trc = (AVColorTransferCharacteristic)u(8, buf, &StartBit); /* transfer_characteristics */
				sps->colorspace = (AVColorSpace)u(8, buf, &StartBit); /* matrix_coefficients */
			}
		}

		/* chroma_location_info_present_flag */
		if (u(1, buf, &StartBit))
		{
			/* chroma_sample_location_type_top_field */
			Ue(buf, nLen, &StartBit);
			Ue(buf, nLen, &StartBit);
		}

		sps->timing_info_present_flag = u(1, buf, &StartBit);
		if (sps->timing_info_present_flag)
		{
			sps->num_units_in_tick = u(32, buf, &StartBit);
			sps->time_scale = u(32, buf, &StartBit);
			sps->fixed_frame_rate_flag = u(1, buf, &StartBit);

			*framerate = ceil(sps->time_scale * 1.0f / sps->num_units_in_tick / 2);
		}
		else
		{
			*framerate = 30;
		}


	}



	return 0;
}

int32_t convertOSD(uint8_t* osdBuf, int osdLen, uint8_t* convBuf, int* convLen) {
	if (osdLen > 250)
		return -1;

	uint8_t buff[1024];
	memcpy(buff, osdBuf, osdLen);

	int i;
	int count = 0;
	int it = 0;
	int pos = -1;
	for (i = 0; i < osdLen; i++)
	{
		if (i < osdLen - 2)
		{
			if ((buff[i] == 0x00) && (buff[i + 1] == 0x00))
			{
				if (buff[i + 2] == 0x03)
				{
					count++;
					pos = i + 2;
				}
				else
				{
					//return -1;
				}
			}
		}
		if (i != pos)
			convBuf[it++] = buff[i];
	}

	*convLen = osdLen - count;
	return 0;
}

uint8_t spsFlag[] = { 0x00,0x00,0x00,0x01,0x07 };
uint8_t ppsFlag[] = { 0x00,0x00,0x00,0x01,0x08 };
uint8_t endFlag[] = { 0x00,0x00,0x00,0x01 };

//int find_SPS_PPS1(uint8_t* pInBuff, int iSize, uint8_t* pSPS, int* iSpsLen, uint8_t* pPpsBuf,int* iPpsLen)
//{
//	int flag = 0x66;
//	int spsPos = 0;
//	int ppsPos = 0;
//	int i;
//	for( i = 0;i<iSize;i++)
//	{
//		if(spsPos>250||ppsPos>250) return -1;
//		if(flag == 0x66)
//		{
//			if( memcmp(pInBuff+i, spsFlag, sizeof(spsFlag))==0)
//			{
//				memcpy(pSPS+spsPos,spsFlag,sizeof(spsFlag));
//				spsPos+=sizeof(spsFlag);
//				i+=sizeof(spsFlag)-1;
//				flag = 0x67;
//			}
//		}else if(flag == 0x67)
//		{
//			if( memcmp(pInBuff+i, ppsFlag, sizeof(ppsFlag))==0)
//			{
//				memcpy(pPpsBuf+ppsPos,ppsFlag,sizeof(ppsFlag));
//				ppsPos+=sizeof(ppsFlag);
//				i+=sizeof(ppsFlag)-1;
//				flag = 0x68;
//			}
//			else
//			{
//				pSPS[spsPos] = pInBuff[i];
//				spsPos++;
//			}
//
//		}else if(flag == 0x68)
//		{
//			if( memcmp(pInBuff+i, endFlag, sizeof(endFlag))==0)
//			{
//				flag = 0x01;
//			}
//			else
//			{
//				pPpsBuf[ppsPos] = pInBuff[i];
//				ppsPos++;
//			}
//		}
//		else if(flag == 0x01)
//		{
//			*iSpsLen = spsPos;
//			*iPpsLen = ppsPos;
//
//			return 0;
//		}
//	}
//
//	return -1;
//
//}

/*
* if to find an end, simply set mask = 0x00ffffff, compare = 0x00010000.
* if to find a SPS, set mask = 0x1fffffff, compare = 0x07010000.
* if to find a PPS, set mask = 0x1fffffff, compare = 0x08010000
* if to find a SEI, set mask = 0x1fffffff, compare = 0x06010000
* if to find an I-frame, set mask = 0x1fffffff, compare = 0x05010000
* if to find a non-I-frame, set mask = 0x1fffffff, compare = 0x01010000
* if to find an I-frame or non-I-frame, set mask = 0x1bffffff, compare = 0x01010000
*/
int findNALU(void* buffer, int i, int end, int mask, int compare)
{
	while (i <= end - 4 && ((*(uint32_t*)((unsigned char*)buffer + i)) & mask) != compare)
	{
		//	 LOGD("after mask: %x", ((*(uint32_t*)((unsigned char*)buffer+i)) & mask) );
		i++;
	}

	if (i>end - 4)
	{
		return end;
	}

	if (i>0 && ((unsigned char*)buffer)[i - 1] == 0)
	{
		i--;
	}

	return i;
}

void parseSpsPps(uint8_t* _buffer, int checkEnd, int& sps_start, int& sps_size, int&pps_start, int&pps_size) {
	int i = 0;

	sps_start = pps_start = -1;

	/*
	* if to find an end, simply set mask = 0x00ffffff, compare = 0x00010000.
	* if to find a SPS, set mask = 0x1fffffff, compare = 0x07010000.
	* if to find a PPS, set mask = 0x1fffffff, compare = 0x08010000
	* if to find an I-frame, set mask = 0x1fffffff, compare = 0x05010000
	* if to find a non-I-frame, set mask = 0x1fffffff, compare = 0x01010000
	* if to find an I-frame or non-I-frame, set mask = 0x1bffffff, compare = 0x01010000
	*/

	//look for sps
	i = findNALU(_buffer, i, checkEnd, 0x1fffffff, 0x07010000);
	//	LOGI("sps i=%d", i);
	if (i < checkEnd) {
		//we get the sps
		sps_start = i;

		//seek to the next byte after sps
		i += 4;
		i = findNALU(_buffer, i, checkEnd, 0x00ffffff, 0x00010000);

		sps_size = i - sps_start;
		//mark 1 represents the following is sps
		//		LOGI("sps_size=%d", sps_size);
	}

	//look for pps
	i = findNALU(_buffer, i, checkEnd, 0x1fffffff, 0x08010000);

	if (i < checkEnd) {
		//we get the sps
		pps_start = i;

		//seek to the next byte after sps
		i += 4;
		i = findNALU(_buffer, i, checkEnd, 0x00ffffff, 0x00010000);

		pps_size = i - pps_start;
		//mark 2 represents the following is pps
		//		LOGI("pps_size=%d", pps_size);
	}
}

int find_SPS_PPS(uint8_t* pInBuff, int iSize, uint8_t* pSPS, int* iSpsLen, uint8_t* pPpsBuf, int* iPpsLen)
{
	int flag = 0x66;
	int spsPos = 0;
	int ppsPos = 0;
	int i;
	int endsize = sizeof(endFlag);

	for (i = 0; i<iSize; i++) {
		if (spsPos>250 || ppsPos>250) return -1;

		if (memcmp(pInBuff + i, endFlag, endsize) == 0) {

			int nextbuff = pInBuff[i + endsize];

			if (flag == 0x66 && ((nextbuff & 0x1F) == 0x07)) {
				//LOGE("find_SPS_PPS nextbuff %x %d", nextbuff, nextbuff&0x1F);
				flag = 0x67;
				memcpy(pSPS + spsPos, endFlag, endsize);
				pSPS[spsPos + endsize] = nextbuff;
				spsPos += endsize + 1;
				i += endsize;

			}
			else if (flag == 0x67 && ((nextbuff & 0x1F) == 0x08)) {
				//LOGE("find_SPS_PPS nextbuff %x %d", nextbuff, nextbuff&0x1F);
				flag = 0x68;
				memcpy(pPpsBuf + ppsPos, endFlag, endsize);
				pPpsBuf[ppsPos + endsize] = nextbuff;
				ppsPos += endsize + 1;
				i += endsize;

			}
			else if (flag == 0x68) {

				*iSpsLen = spsPos;
				*iPpsLen = ppsPos;
				//LOGE("find_SPS_PPS ok");
				return 0;
			}

		}
		else {
			if (flag == 0x67) {

				pSPS[spsPos] = pInBuff[i];
				spsPos++;

			}
			else if (flag == 0x68) {

				pPpsBuf[ppsPos] = pInBuff[i];
				ppsPos++;

			}
		}
	}

	return -1;

}

/*
long djiSleep(long delay) {
struct timespec req, rem;
rem.tv_sec = 0;
rem.tv_nsec = 0;
req.tv_sec = 0;
req.tv_nsec = delay * 1000;
return nanosleep(&req, &rem);
}
*/
static const int KEY_I_FRAME_INTERVAL = 1;
static const long FRAME_INDEX_MAX = 0xfffffL;
static const long FRAME_PTS_MAX = 0x0ffffffL;
static const long FRAME_NUM_MAX = 0x3CL;
static const long FRAME_NUM_MAX_MUX = 0x3FL;
static const int position = 6;

static long start_time = getTickCount();

long getComprehensivePts(long ptsMs, long frameIndex, long frameNum)
{
	long valid_frameIndex = (frameIndex>FRAME_INDEX_MAX ? FRAME_INDEX_MAX : frameIndex);
	long valid_pts = ((ptsMs - start_time)&FRAME_PTS_MAX);
	long valid_frameNum = (frameNum>FRAME_NUM_MAX ? FRAME_NUM_MAX : frameNum);
	return (((valid_frameIndex << 24) + valid_pts) << position) + valid_frameNum;
}
