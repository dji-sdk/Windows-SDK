#ifndef _H264_DECODER_
#define	_H264_DECODER_

#include "Queue.h"
#include "threadsafequeue.h"
#include <thread>
#include <vector>
#include <array>
#include <queue>

extern "C" {

#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavcodec/avcodec.h"
}

namespace dji {
	namespace videoparser {
		class VideoWrapper;
	}
}

#pragma pack(1)
typedef struct tagHPI_CHL_DATA
{
	uint32_t    magic;
	uint32_t    len;
	uint8_t     ver_hsize;
	uint8_t     chnl_id;
	uint8_t     checksum;
	uint8_t     padding;
	uint8_t     data_byte[1];

	uint32_t total_size() const
	{
		return payload_size() + 12;
	}
	uint32_t payload_size() const
	{
		uint32_t temp = ((len & 0xFF000000) >> 8) | (len & 0x0000FFFF);
		return temp;
	}
}HPI_CHL_DATA;
#pragma pack()

class Pack_Compositer
{
public:
	Pack_Compositer() : m_hasPackHeader(false), m_channelId(0x11) {}
	~Pack_Compositer() {}

	bool CompositePack(const uint8_t *data, const uint32_t size);
	void SetChannelIdx(const uint8_t chID) { m_channelId = chID; }
	HPI_CHL_DATA* m_pCurDataItem;

	std::vector<uint8_t> m_outBuffer;
private:
	bool HeaderXor(const uint8_t* pData);

private:
	std::vector<uint8_t> m_packBuffer;
	bool       m_hasPackHeader;
	uint8_t    m_channelId;
	int        m_counter = 0;
};

class h264_Decoder {
public:
	h264_Decoder();
	~h264_Decoder();

	AVFrame* m_av_frame = nullptr;
	AVFrame* m_dst_frame = nullptr;

	uint8_t* m_outBuffer = nullptr;
	AVCodecContext* m_codec_context = nullptr;
	AVCodec* m_av_codec = nullptr;
	AVCodecParserContext* m_codec_paser = nullptr;

	SwsContext* m_sws_ctx = nullptr;
	Pack_Compositer m_pack_compositer;

	int videoFrameParse(const uint8_t* buff, int video_size, FrameType type, uint64_t pts);

	void Stop();

	void Initialize(dji::videoparser::VideoWrapper* video_wrapper = nullptr);
	void Uninitialize();

protected:
	bool InitFFMPEG();

	bool InitFrameBuffer();

	void DecoderThread();


	std::thread *m_thread_decoder = nullptr;

	bool m_bInitBuffer = false;
	bool m_bDecoderRun = false;

	dji::videoparser::VideoWrapper * m_videoWrapperPtr = nullptr;

	dji::videoparser::threadsafe_queue<std::vector<uint8_t>> m_vectorSafeQueue;
	uint32_t prev_width_ = 0;
	uint32_t prev_height_ = 0;
};

#endif //_H264DECODER_
