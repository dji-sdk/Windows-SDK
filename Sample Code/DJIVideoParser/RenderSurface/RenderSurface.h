#pragma once

#include <Windows.Foundation.Collections.h>
#include <windows.media.core.h>
#include "Windows.UI.Core.h"
#include <Windows.UI.Xaml.Controls.h>

#define GL_GLEXT_PROTOTYPES 
//#define GL2_PROTOTYPES 1
#include <GLES2\gl2.h>
#include <EGL\egl.h>
#include <angle_windowsstore.h>
#include <Windows.Foundation.Collections.h>
#include <GLES2\gl2ext.h>
#include <GLES2\gl2platform.h>
#include <gl\GL.h>
#include <gl\GLU.h>

using namespace Platform;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Core;

class RenderSurface final
{
public:
	RenderSurface();
	~RenderSurface();

	//To draw with OpenGL, we need a SwapChainPanel.
	bool Initialize(const std::string& source_path, Windows::UI::Xaml::Controls::SwapChainPanel^ swap_chain_panel);
	void Uninitialize();

	bool Ready() const;

	//Set the sensor type. We need to calibrate the images of some sensors.
	void UpdateDeviceCameraSensor(DeviceCameraSensor sensor);
	void RenderRGBImageData(const unsigned char *data, int width, int height);
	void SetVideoInfo(int fov_state, int lut_index);

private:
	class impl;
	impl* pImpl = nullptr;
};