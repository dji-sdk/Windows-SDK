#include "pch.h"
#include "videoparsermgr.h"
#include <plog/Log.h>

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

		bool VideoParserMgr::Initialize()
		{
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
					//                	LOGE << "cannot find product  map count " << m_map_parser.size();
					return;
				}

				auto index_map = m_map_parser[product_id];
				auto parser_iterator = index_map.find(component_index);
				if (parser_iterator == index_map.end())
				{
					//					LOGE << "cannot find parser_iterator " << component_index;
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
#ifdef _ANDROID_
		bool VideoParserMgr::SetWindow(int product_id, int product_type, int component_index, void *window)
		{
			std::lock_guard<std::recursive_mutex> lock(m_mutex_map_parser);
			auto map_iter = m_map_parser.find(product_id);
			if (map_iter == m_map_parser.end())
			{
				AddDevice(product_id);
			}

			auto& parser_map = m_map_parser[product_id];

			auto parser_map_iter = parser_map.find(component_index);
			if (parser_map_iter == parser_map.end())
			{
				if (window == nullptr)
				{
					LOGE << "window == nullptr";
					return true;
				}

				auto parser = std::make_shared<VideoParser>();

				if (!parser->Initialize(product_type))
				{
					LOGE << "parser->Initialize failed";
					parser = nullptr;
					return false;
				}

				if (!parser->GetCodec()->SetNativeWindow((ANativeWindow*)window))
				{
					LOGE << "parser->GetCodec()->SetNativeWindow failed";
					parser->Uninitialize();
					parser = nullptr;
					return false;
				}

				parser_map.insert(std::make_pair(component_index, parser));
			}
			else
			{
				std::shared_ptr<VideoParser>& video_parser = parser_map[component_index];
				if (window == nullptr)
				{
					video_parser->Uninitialize();
					video_parser = nullptr;
					parser_map.erase(component_index);
					return true;
				}

				auto codec = video_parser->GetCodec();
				if (codec == nullptr)
				{
					return false;
				}

				return codec->SetNativeWindow((ANativeWindow*)window);
			}

			LOGE << "m_map_parser[device_id] count : " << m_map_parser[component_index].size() << " component_index " << component_index;
			return true;
		}
#else
		bool VideoParserMgr::SetWindow(int product_id, int component_index, std::function<void(uint8_t *data, int width, int height)> func)
		{
			std::lock_guard<std::recursive_mutex> lock(m_mutex_map_parser);
			auto map_iter = m_map_parser.find(product_id);
			if (map_iter == m_map_parser.end())
			{
				AddDevice(product_id);
			}

			auto& parser_map = m_map_parser[product_id];


			auto parser_map_iter = parser_map.find(component_index);
			if (parser_map_iter == parser_map.end())
			{

				auto parser = std::make_shared<VideoParser>();

				if (!parser->Initialize(product_id))
				{
					LOGE << "parser->Initialize failed";
					parser = nullptr;
					return false;
				}

				auto video_wrapper = parser->GetVideoWrapper();
				video_wrapper->SetVideoFrameCallBack(func);

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

				auto video_wrapper = video_parser->GetVideoWrapper();
				video_wrapper->SetVideoFrameCallBack(func);

			}

			return true;
		}
#endif
	}
}
