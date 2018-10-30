#ifndef __videoparsar__
#define __videoparsar__

#ifdef _ANDROID_
#include <memory>
#include "videoparser/previewer.h"
#include "videoparser/djicodec.h"
#else
#include "VideoWrapper.h"

#endif

#include <atomic>
#include <memory>


namespace dji
{
	namespace videoparser
	{
		class VideoParser : public std::enable_shared_from_this<VideoParser>
		{
		public:
			VideoParser();
			~VideoParser();

			bool Initialize(int product_type);
			void Uninitialize();

			void PauseParserThread(bool is_pause);
			void ParserData(const unsigned char* data, unsigned int len);

			inline void SetDecoderType(int type) { m_decoder_type = type; }
			inline int GetDecoderType() { return m_decoder_type; }

			inline void SetFrameRate(int frame_rate) { if (frame_rate > 0) m_frame_rate = frame_rate; }
			inline int GetFrameRate() { return m_frame_rate; }

			inline void SetIsFixRate(bool is_fix_rate) { m_is_fix_rate = is_fix_rate; }
			inline bool GetIsFixRate() { return m_is_fix_rate; }

			inline void SetIsAuthValue(bool is_auth_value) { m_is_auth_value = is_auth_value; }
			inline bool getIsAuthValue() { return m_is_auth_value; }
#ifdef _ANDROID_
			inline DjiCodec* GetCodec() { return m_dji_codec; }
#else
			inline dji::videoparser::VideoWrapper* GetVideoWrapper() { return m_videoWrapper; }
#endif



		private:
			bool m_is_initialized = false;

			//Hardware(0), Software(1), None(2)
			int m_decoder_type = 2;

			int m_frame_rate = 29;

			bool m_is_fix_rate = true;

			bool m_is_auth_value = true;

#ifdef _ANDROID_
			DjiCodec* m_dji_codec = nullptr;
			Previewer* m_previewer = nullptr;
#else
			dji::videoparser::VideoWrapper* m_videoWrapper = nullptr;
#endif
		};

	}
}

#endif

//#endif
