#ifndef __videoparsarmgr__
#define __videoparsarmgr__

#ifdef _ANDROID_
#include <jni.h>
#endif

#include <string>
#include <unordered_map>
#include <memory>
#include "videoparser.h"
#include <mutex>
#include <Windows.UI.Xaml.Controls.h>
#include "RenderSurface/RenderSurface.h"

namespace dji
{
	namespace videoparser
	{

		typedef std::unordered_map<int, std::shared_ptr<VideoParser>> IndexParserMap;
		typedef std::unordered_map<int, IndexParserMap> ParserMap;

		class VideoParserMgr final : public std::enable_shared_from_this<VideoParserMgr>
		{
		public:
			VideoParserMgr();
			~VideoParserMgr();

			bool Initialize(const std::string & source_path, std::function< DJIDecodingAssistInfo (uint8_t* data, int length)> decoding_assist_info_parser);
			void Uninitialize();

			bool AddDevice(int product_id);
			void RemoveDevice(int product_id);

			bool SetWindow(int product_id, int component_index, std::function<void(uint8_t *data, int width, int height)> func, Windows::UI::Xaml::Controls::SwapChainPanel^ swap_chain_panel);

			void SetSensor(DeviceCameraSensor sensor);

			bool StartRecord(int product_id, int component_index) { return  true; }
			bool StopRecord(int product_id, int component_index) { return true; }

			void ParserData(int product_id, int component_index, const unsigned char* data, unsigned int len);
		protected:
			void FreeMap();
		private:
			std::string m_source_path;
			ParserMap m_map_parser;
			std::recursive_mutex m_mutex_map_parser;
			Windows::UI::Xaml::Controls::SwapChainPanel^ m_swap_chain_panel = nullptr;

			std::unique_ptr<RenderSurface> m_render_surface = std::make_unique<RenderSurface>();
			std::function< DJIDecodingAssistInfo (uint8_t* data, int length)> m_decoding_assist_info_parser;
		};

	}
}
//#endif
#endif
