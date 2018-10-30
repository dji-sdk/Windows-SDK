#include "pch.h"
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <errno.h>
#include "VideoWrapper.h"
#include "Utils.h"
#include "videoparser.h"


#undef  LOG_TAG
#define LOG_TAG "VideoWrapper.cpp"
namespace dji
{
	namespace videoparser
	{
		static int s_rate = 30;
		static int s_sleep = 1 * 1000 * 1000 / 33;
		static long s_cur_time = 0;

		VideoWrapper::VideoWrapper(void)
		{
		}

		VideoWrapper::~VideoWrapper(void)
		{
		}

		bool VideoWrapper::Initialize(VideoParser* owner)
		{
			if (owner == nullptr)
				return false;
			m_owner = owner;

			m_current_pts = 0;

			m_video_parser.Initialize(this);
			return true;
		}

		void VideoWrapper::Uninitialize()
		{
			m_video_parser.Uninitialize();

			m_is_pause = false;
			m_owner = nullptr;
		}

		void VideoWrapper::SetVideoFrameCallBack(std::function<void(uint8_t *data, int width, int height)> func)
		{
			if (nullptr != func)
			{
				m_dataObserver = func;
			}
		}

		int VideoWrapper::PauseParserThread(bool isPause) {
			m_is_pause = isPause;
			return 0;
		}

		void VideoWrapper::ClearFrame() {
			m_current_pts = 0;
			m_video_queue.clear();
		}

		int VideoWrapper::PutToQueue(const uint8_t* buffer, int size, uint64_t pts)
		{
			m_video_parser.videoFrameParse(buffer, size, pts == 0 ? FrameType_VideoLive : FrameType_Video, pts);
			return 0;
		}

		int VideoWrapper::PutVideoToQueue(const uint8_t* buffer, int size, uint64_t pts)
		{
			m_video_parser.videoFrameParse(buffer, size, FrameType_Video, pts);
			return 0;
		}

		int VideoWrapper::FramePacket(uint8_t* buff, int size, FrameType type, int width, int height)
		{
			if (nullptr != m_dataObserver)
			{
				m_dataObserver(buff, width, height);
			}
			return 0;
		}
	}
}



