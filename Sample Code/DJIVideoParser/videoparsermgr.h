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

			bool Initialize();
			void Uninitialize();

			bool AddDevice(int product_id);
			void RemoveDevice(int product_id);

#ifdef _ANDROID_
			bool SetWindow(int product_id, int product_type, int component_index, void *window);
#else
			bool SetWindow(int product_id, int component_index, std::function<void(uint8_t *data, int width, int height)> func);
#endif

			bool StartRecord(int product_id, int component_index) { return  true; }
			bool StopRecord(int product_id, int component_index) { return true; }

			int GetRenderTexture(int product_id, int component_index) { return -1; }


			void ParserData(int product_id, int component_index, const unsigned char* data, unsigned int len);
		protected:
			void FreeMap();
		private:
			ParserMap m_map_parser;
			std::recursive_mutex m_mutex_map_parser;
		};

	}
}
//#endif
#endif
