#pragma once

#include <collection.h>
#include <ppltasks.h>

enum class DeviceCameraSensor
{
	imx283,
	imx477,
	Unknown
};

#pragma pack(1)
typedef struct {
	uint8_t has_lut_idx; // lut index
	uint8_t has_time_stamp; // the global time stamp
	uint8_t should_ignore;//render and processor ignore this frame after decoding
	uint8_t force_30_fps;//when liveview fps = 60，downsample to keep 30fps rendering and encoding
	uint8_t lut_idx; //has_lut_idx == 0,ignored
	uint8_t fov_state;//has_lut_idx == 0,ignored
	uint32_t timestamp; //has_time_stamp == 0,ignored
} DJIDecodingAssistInfo;
#pragma pack()