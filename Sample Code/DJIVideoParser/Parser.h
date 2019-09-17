#pragma once

namespace DJIVideoParser
{
	public enum class AircraftCameraType : int {
		Mavic2Pro,
		Mavic2Zoom,
		Others
	};

	public delegate void VideoDataCallback(const Platform::Array<byte>^ data, int witdth, int height);

	public delegate void VideoFrameBufferCallback(const Platform::Array<byte>^ data, unsigned int uiWidth, unsigned int uiHeight, unsigned __int64 ulTimeStamp);

	//User should set this handler to call "DJISDKManager.Instance.VideoFeeder.ParseAssitantDecodingInfo(0, data);" inside. This would help DJIWindowsSDK to handle the image data and ask Mavic 2 for i frame and do some calibration jobs.
	public delegate Platform::Array<int>^ VideoAssistantInfoParserHandle(const Platform::Array<byte>^ data);

    public ref class Parser sealed
    {
    public:
        Parser();
		void Initialize(VideoAssistantInfoParserHandle^ handle);
		void Uninitialize(); 
		//would return RGBA image
		void SetSurfaceAndVideoCallback(int product_id, int index,  Windows::UI::Xaml::Controls::SwapChainPanel^ swap_chain_panel, VideoDataCallback^ callback);
		void PushVideoData(int product_id, int index, const Platform::Array<byte>^ data, int size);
		void SetCameraSensor(AircraftCameraType sensor);
    };
}
