#ifndef _H264_DECODER_
#define	_H264_DECODER_

#include "Queue.h"
#include "threadsafequeue.h"
#include <thread>
#include <vector>
#include <array>
#include <queue>
#include <functional>

#include "Windows.UI.Core.h"

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

	int videoFrameParse(const uint8_t* buff, int video_size, FrameType type, uint64_t pts);

	void Stop();

	void Initialize(dji::videoparser::VideoWrapper* video_wrapper = nullptr, std::function<DJIDecodingAssistInfo(uint8_t* data, int length)> decoding_assist_info_parser = nullptr);
	void Uninitialize();

protected:
	bool InitFFMPEG();

	bool InitFrameBuffer();

	void DecoderThread();

	int m_preWidth = 0;
	int m_preHeight = 0;

	std::thread *m_thread_decoder = nullptr;

	bool m_bInitBuffer = false;
	bool m_bDecoderRun = false;

	dji::videoparser::VideoWrapper * m_videoWrapperPtr = nullptr;

	dji::videoparser::threadsafe_queue<std::vector<uint8_t>> m_vectorSafeQueue;

	std::function< DJIDecodingAssistInfo (uint8_t* data, int length)> m_assistInfoParser;

};

#endif //_H264DECODER_
