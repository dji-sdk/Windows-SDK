//
//  modulemediator.cpp
//  djisdk
//
//  Created by ccbben on 2017/9/25.
//
//

#include "pch.h"
#include <stdio.h>
#include "modulemediator.h"

namespace dji
{
    namespace videoparser
    {
		ModuleMediator* g_pModuleMediator = new ModuleMediator();

        ModuleMediator::ModuleMediator()
        {
            
        }
        
        ModuleMediator::~ModuleMediator()
        {
            
        }
        
        bool ModuleMediator::Initialize(const std::string & source_path, std::function<DJIDecodingAssistInfo(uint8_t* data, int length)> decoding_assist_info_parser)
        {
            m_video_parser_mgr = std::make_shared<VideoParserMgr>();
            if (!m_video_parser_mgr->Initialize(source_path, decoding_assist_info_parser))
            {
                return false;
            }
            return true;
        }
        
        void ModuleMediator::Uninitialize()
        {
            if (m_video_parser_mgr)
            {
                m_video_parser_mgr->Uninitialize();
                m_video_parser_mgr = nullptr;
            }
        }
    }
}
