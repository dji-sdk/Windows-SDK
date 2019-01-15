//
//  modulemediator.h
//  djisdk
//
//  Created by ccbben on 2017/9/25.
//
//

#ifndef modulemediator_h
#define modulemediator_h

#include <set>
#include "videoparsermgr.h"
#include "Windows.UI.Core.h"

namespace dji
{
    namespace videoparser
    {
        class ModuleMediator final
        {
        public:
            ModuleMediator();
            ~ModuleMediator();
            
			bool Initialize(const std::string & source_path, std::function < DJIDecodingAssistInfo (uint8_t* data, int length) > decoding_assist_info_parser);
            void Uninitialize();
            
            inline std::weak_ptr<VideoParserMgr> GetVideoParserMgr() { return m_video_parser_mgr; }
        private:
            std::shared_ptr<VideoParserMgr> m_video_parser_mgr;
        };
        
        extern ModuleMediator* g_pModuleMediator;
    }
}

#endif /* modulemediator_h */
