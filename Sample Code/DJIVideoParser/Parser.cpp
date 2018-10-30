#include "pch.h"
#include "Parser.h"
#include "modulemediator.h"

using namespace DJIVideoParser;
using namespace Platform;


using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Automation::Peers;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::UI::Xaml::Interop;
using namespace concurrency;
using namespace Platform;
using namespace Windows::Storage;


namespace DJIVideoParser {
	dji::videoparser::ModuleMediator* g_pModuleMediator = new dji::videoparser::ModuleMediator;
}
Parser::Parser()
{
}

void DJIVideoParser::Parser::Initialize()
{
	g_pModuleMediator->Initialize(); 
}

void DJIVideoParser::Parser::Uninitialize()
{
	g_pModuleMediator->Uninitialize(); 
}

void DJIVideoParser::Parser::SetVideoDataCallack(int product_id, int index, VideoDataCallback ^ callback)
{
	std::shared_ptr<dji::videoparser::VideoParserMgr> video_parser_mgr = g_pModuleMediator->GetVideoParserMgr().lock();

	if (video_parser_mgr)
	{
		if (callback == nullptr)
		{
			video_parser_mgr->SetWindow(product_id, index, nullptr); 
		}
		else
		{
			video_parser_mgr->SetWindow(product_id, index, [callback](uint8_t *data, int width, int height) {
				callback(Platform::ArrayReference<byte>(data, width * height * 4), width, height);
			});
		}
	}
}

void DJIVideoParser::Parser::PushVideoData(int product_id, int index, const Platform::Array<byte>^ data, int size)
{
	std::shared_ptr<dji::videoparser::VideoParserMgr> video_parser_mgr = g_pModuleMediator->GetVideoParserMgr().lock();

	if (video_parser_mgr)
	{
		//                LOGD << "video_parser_mgr->ParserData ";
		video_parser_mgr->ParserData(product_id, index, data->Data, size);
	}
}
