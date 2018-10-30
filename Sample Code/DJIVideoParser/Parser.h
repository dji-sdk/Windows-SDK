#pragma once

namespace DJIVideoParser
{
	public delegate void VideoDataCallback(const Platform::Array<byte>^ data, int witdth, int height);

	public delegate void VideoFrameBufferCallback(const Platform::Array<byte>^ data, unsigned int uiWidth, unsigned int uiHeight, unsigned __int64 ulTimeStamp);

    public ref class Parser sealed
    {
    public:
        Parser();
		void Initialize(); 
		void Uninitialize(); 
		void SetVideoDataCallack(int product_id, int index, VideoDataCallback^ callback);
		//void SetFrameDataCallback(VideoFrameBufferCallback^ callback); 
		void PushVideoData(int product_id, int index, const Platform::Array<byte>^ data, int size);
    };
}
