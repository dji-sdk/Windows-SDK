#include "pch.h"
#include "Parser.h"
#include "modulemediator.h"

#include <Windows.Storage.h>

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

using namespace dji::videoparser;

namespace DJIVideoParser {
	//dji::videoparser::ModuleMediator* g_pModuleMediator = new dji::videoparser::ModuleMediator;
}
Parser::Parser()
{
}

// wstring => string
static std::string WString2String(const std::wstring& ws)
{
	auto wideData = ws.c_str();
	int bufferSize = WideCharToMultiByte(CP_ACP, 0, wideData, -1, nullptr, 0, NULL, NULL);
	auto targetData = std::make_unique<char[]>(bufferSize);
	if (0 == WideCharToMultiByte(CP_ACP, 0, wideData, -1, targetData.get(), bufferSize, NULL, NULL))
		throw std::exception("Can't convert string to ACP");

	return std::string(targetData.get());
}

void DJIVideoParser::Parser::Initialize(VideoAssistantInfoParserHandle^ handle)
{
	auto folder = Windows::Storage::ApplicationData::Current->LocalFolder->Path;

	auto wstring = std::wstring(folder->Begin());
	std::string path = WString2String(wstring);
	g_pModuleMediator->Initialize(path, [handle](uint8_t* data, int length)
	{
		if (handle)
		{
			Platform::Array<unsigned char>^ out = nullptr;
			//DecodingAssistInfo^ info = nullptr;
			auto info = handle(ref new Platform::Array<unsigned char>(data, length));
			
			DJIDecodingAssistInfo res = { 0 };
			if (info->Length >= 7)
			{
				res.has_lut_idx = info[0];
				res.has_time_stamp = info[1];
				res.should_ignore = info[2];
				res.force_30_fps = info[3];
				res.lut_idx = info[4];
				res.fov_state = info[5];
				res.timestamp = info[6];
			}
			return res;
		}
	});
}

void DJIVideoParser::Parser::Uninitialize()
{
	g_pModuleMediator->Uninitialize(); 
}

void DJIVideoParser::Parser::SetSurfaceAndVideoCallback(int product_id, int index, Windows::UI::Xaml::Controls::SwapChainPanel^ swap_chain_panel, VideoDataCallback^ callback)
{
	std::shared_ptr<dji::videoparser::VideoParserMgr> video_parser_mgr = g_pModuleMediator->GetVideoParserMgr().lock();

	if (video_parser_mgr)
	{
		if (callback == nullptr)
		{
			video_parser_mgr->SetWindow(product_id, index, nullptr, swap_chain_panel);
		}
		else
		{
			video_parser_mgr->SetWindow(product_id, index, [callback](uint8_t *data, int width, int height) {
				callback(Platform::ArrayReference<byte>(data, width * height * 4), width, height);
			}, swap_chain_panel);
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


void DJIVideoParser::Parser::SetCameraSensor(AircraftCameraType sensor)
{
	std::shared_ptr<dji::videoparser::VideoParserMgr> video_parser_mgr = g_pModuleMediator->GetVideoParserMgr().lock();

	static std::map<AircraftCameraType, DeviceCameraSensor> map = 
	{
		{ AircraftCameraType::Mavic2Pro, DeviceCameraSensor::imx283 },
		{ AircraftCameraType::Mavic2Zoom, DeviceCameraSensor::imx477 },
		{ AircraftCameraType::Others, DeviceCameraSensor::Unknown },
	};
	
	video_parser_mgr->SetSensor(map[sensor]);
}