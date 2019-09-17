#ifndef __DJIVIDEO_VIDEOWRAPPER_H_
#define __DJIVIDEO_VIDEOWRAPPER_H_

#include <memory>
#include <thread>
#include "h264_Decoder.h"
#include "Queue.h"

namespace dji {
	namespace videoparser {
		class VideoParser;

		class VideoWrapper final
		{
		public:
			VideoWrapper(void);

			~VideoWrapper(void);

			bool Initialize(VideoParser *owner, std::function< DJIDecodingAssistInfo (uint8_t* data, int length)> decoding_assist_info_parser);

			void Uninitialize();

			int PauseParserThread(bool is_pause);


			int FramePacket(uint8_t *buff, int size, FrameType type, int width, int height, const DJIDecodingAssistInfo & assistant_info);

			void SetVideoFrameCallBack(std::function<void(uint8_t *data, int width, int height, const DJIDecodingAssistInfo & assistant_info)> func);

			void ClearFrame();

			int PutToQueue(const uint8_t *pBuff, int size, uint64_t pts);

			int PutVideoToQueue(const uint8_t *pBuff, int size, uint64_t pts);

		private:
			VideoParser * m_owner = nullptr;
			uint64_t m_current_pts = 0;

			h264_Decoder m_video_parser;
			CFrameQueue m_video_queue;

			bool m_is_pause = false;
			std::function<void(uint8_t *data, int width, int height, const DJIDecodingAssistInfo & assistant_info)> m_dataObserver;

		};
	}
}
#endif //__DJIVIDEO_VIDEOWRAPPER_H_
