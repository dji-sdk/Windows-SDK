#include "pch.h"
#include "videoparser.h"
#include <plog/Log.h>

namespace dji
{
	namespace videoparser
	{
		static const uint8_t s_aud_buffer[] = { 0x00,0x00,0x00,0x01,0x09,0x10 };
		static const uint8_t s_aud_size = 6;

		static const uint8_t s_filler_buffer[] = { 0x00,0x00,0x00,0x01,0x0C,0x00,0x00,0x00,0x01,0x09,0x10 };
		static const uint8_t s_filler_size = 11;

		static const uint8_t s_aud_buffer_2[] = { 0x00,0x00,0x00,0x01,0x09,0x10, 0x00,0x00,0x00,0x01,0x09,0x10 };
		static const uint8_t s_aud_size_2 = 12;

		VideoParser::VideoParser()
		{
		}

		VideoParser::~VideoParser()
		{

		}

		bool VideoParser::Initialize(int product_type)
		{
			m_is_initialized = true;

#ifdef _ANDROID_

			m_previewer = new Previewer();
			if (!m_previewer->Initialize(this))
			{
				delete m_previewer;
				return false;
			}

			m_dji_codec = new DjiCodec();
			if (!m_dji_codec->Initialize(product_type))
			{
				delete m_dji_codec;
				return false;
			}
#elif defined _PC_
			m_videoWrapper = new VideoWrapper();
			if (!m_videoWrapper->Initialize(this))
			{
				delete m_videoWrapper;
				return false;
			}
#endif
			return true;
		}

		void VideoParser::Uninitialize()
		{
			LOGE << "VideoParser::Uninitialize() m_videoWrapper";
#ifdef _ANDROID_

			if (m_previewer)
			{
				m_previewer->Uninitialize();
				delete m_previewer;
				m_previewer = nullptr;
			}

			LOGE << "VideoParser::Uninitialize() m_dji_codec";
			if (m_dji_codec)
			{
				m_dji_codec->Uninitialize();
				delete m_dji_codec;
				m_dji_codec = nullptr;
			}
#elif defined _PC_
			if (m_videoWrapper)
			{
				m_videoWrapper->Uninitialize();
				delete m_videoWrapper;
				m_videoWrapper = nullptr;
			}
#endif
			m_is_initialized = false;
		}

		void VideoParser::PauseParserThread(bool is_pause)
		{
#ifdef _ANDROID_
			m_previewer->PauseParserThread(is_pause);
#elif defined _PC_
			m_videoWrapper->PauseParserThread(is_pause);
#endif // _PC_

		}

		void VideoParser::ParserData(const unsigned char* buff, unsigned int size)
		{

#ifdef  _ANDROID_
			//			LOGD << "ParserData" << size;

			if (size >= s_filler_size
				&& memcmp(s_filler_buffer, buff + size - s_filler_size, s_filler_size) == 0)
			{
				m_previewer->PutToQueue(buff, size - s_filler_size, 0);
			}
			else if (size >= s_aud_size_2 &&
				memcmp(s_aud_buffer_2, buff + size - s_aud_size_2, s_aud_size_2) == 0)
			{
				m_previewer->PutToQueue(buff, size - s_aud_size_2, 0);
			}
			else if (size >= s_aud_size
				&& memcmp(s_aud_buffer, buff + size - s_aud_size, s_aud_size) == 0)
			{
				m_previewer->PutToQueue(buff, size - s_aud_size, 0);
			}
			else
			{
				m_previewer->PutToQueue(buff, size, 0);
			}
#elif defined _PC_
			LOGD << "Size: " << (int)size; 
			m_videoWrapper->PutToQueue(buff, size, 0);
#endif //  _ANDROID_
		}

	}
}

//#endif
