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
        ModuleMediator::ModuleMediator()
        {
            
        }
        
        ModuleMediator::~ModuleMediator()
        {
            
        }
        
        bool ModuleMediator::Initialize()
        {
            m_video_parser_mgr = std::make_shared<VideoParserMgr>();
            if (!m_video_parser_mgr->Initialize())
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
