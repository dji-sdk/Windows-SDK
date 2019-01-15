#include "pch.h"
#include "videoparsermgr.h"
#include <plog/Log.h>
#include "modulemediator.h"

namespace dji
{
	namespace videoparser
	{

		VideoParserMgr::VideoParserMgr()
		{
		}

		VideoParserMgr::~VideoParserMgr()
		{

		}

		bool VideoParserMgr::Initialize(const std::string & source_path, std::function<DJIDecodingAssistInfo(uint8_t* data, int length)> decoding_assist_info_parser)
		{
			m_source_path = source_path;
			m_decoding_assist_info_parser = decoding_assist_info_parser;
			return true;
		}

		void VideoParserMgr::Uninitialize()
		{
			FreeMap();
		}

		bool VideoParserMgr::AddDevice(int product_id)
		{
			std::lock_guard<std::recursive_mutex> lock(m_mutex_map_parser);
			if (m_map_parser.find(product_id) != m_map_parser.end())
				return false;

			IndexParserMap new_map;
			m_map_parser.insert(std::make_pair(product_id, new_map));

			return true;
		}

		void VideoParserMgr::ParserData(int product_id, int component_index, const unsigned char* data, unsigned int len)
		{
			std::shared_ptr<VideoParser> parser = nullptr;
			{
				std::lock_guard<std::recursive_mutex> lock(m_mutex_map_parser);
				if (m_map_parser.find(product_id) == m_map_parser.end())
				{
					return;
				}

				auto index_map = m_map_parser[product_id];
				auto parser_iterator = index_map.find(component_index);
				if (parser_iterator == index_map.end())
				{
					return;
				}

				parser = parser_iterator->second;
			}

			if (parser)
			{
				parser->ParserData(data, len);
			}
		}

		void VideoParserMgr::RemoveDevice(int product_id)
		{
			std::lock_guard<std::recursive_mutex> lock(m_mutex_map_parser);
			auto map_iter = m_map_parser.find(product_id);
			if (map_iter != m_map_parser.end())
			{
				for (auto& index : m_map_parser[product_id])
				{
					m_map_parser[product_id][index.first]->Uninitialize();
					m_map_parser[product_id][index.first] = nullptr;
				}
				m_map_parser[product_id].clear();

				m_map_parser.erase(map_iter);
			}
		}

		void VideoParserMgr::FreeMap()
		{
			std::lock_guard<std::recursive_mutex> lock(m_mutex_map_parser);
			for (auto &kv : m_map_parser)
			{
				for (auto &pkv : kv.second)
				{
					pkv.second->Uninitialize();
					pkv.second = nullptr;
				}
				m_map_parser[kv.first].clear();
			}
			m_map_parser.clear();
		}

		void VideoParserMgr::SetSensor(DeviceCameraSensor sensor)
		{
			m_render_surface->UpdateDeviceCameraSensor(sensor);
		}

		bool VideoParserMgr::SetWindow(int product_id, int component_index, std::function<void(uint8_t *data, int width, int height)> func, Windows::UI::Xaml::Controls::SwapChainPanel^ swap_chain_panel)
		{

			if (m_swap_chain_panel != swap_chain_panel)
			{
				if (m_swap_chain_panel)
					m_render_surface->Uninitialize();
				 m_swap_chain_panel = swap_chain_panel;
				 if (swap_chain_panel)
					m_render_surface->Initialize(m_source_path, swap_chain_panel);
			}


			std::lock_guard<std::recursive_mutex> lock(m_mutex_map_parser);
			auto map_iter = m_map_parser.find(product_id);
			if (map_iter == m_map_parser.end())
			{
				AddDevice(product_id);
			}

			auto& parser_map = m_map_parser[product_id];

			dji::videoparser::VideoWrapper* wrapper = nullptr;
			auto parser_map_iter = parser_map.find(component_index);
			if (parser_map_iter == parser_map.end())
			{

				auto parser = std::make_shared<VideoParser>();

				if (!parser->Initialize(product_id, m_decoding_assist_info_parser))
				{
					LOGE << "parser->Initialize failed";
					parser = nullptr;
					return false;
				}

				wrapper = parser->GetVideoWrapper();

				parser_map.insert(std::make_pair(component_index, parser));
			}
			else
			{
				std::shared_ptr<VideoParser>& video_parser = parser_map[component_index];

				if (video_parser == nullptr)
				{
					video_parser->Uninitialize();
					video_parser = nullptr;
					parser_map.erase(component_index);
					return true;
				}

				wrapper = video_parser->GetVideoWrapper();

			}
			wrapper->SetVideoFrameCallBack([this, func](uint8_t *data, int width, int height, const DJIDecodingAssistInfo& assistant_info)
			{
					if(func)
						func(data, width, height);
					int actual_size = width * height * 4;
					std::shared_ptr<uint8_t> shared_data = std::shared_ptr<uint8_t>(new uint8_t[actual_size], [](uint8_t *data) {delete[] data;});
					memcpy(shared_data.get(), data, actual_size);
					if (m_render_surface->Ready())
					{
						this->m_swap_chain_panel->Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal,
							ref new Windows::UI::Core::DispatchedHandler([assistant_info, this, shared_data, width, height]
						{
							//TODO: maybe lock
							if (!m_render_surface->Ready())
								return;

							m_render_surface->SetVideoInfo(assistant_info.fov_state, assistant_info.lut_idx);
							m_render_surface->RenderRGBImageData(shared_data.get(), width, height);
						}));
					}


			});


			return true;
		}
	}
}
