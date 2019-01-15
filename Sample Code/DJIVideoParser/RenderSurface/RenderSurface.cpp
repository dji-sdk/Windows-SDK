#include "pch.h"
#include "RenderSurface.h"

#include <EGL\egl.h>
#include <angle_windowsstore.h>
#include "OpenGLES.h"

#include "Renderer\VideoFeedRenderer.h"
#include "Renderer\CalibrationVideoFeedRenderer\CalibrationVideoFeedRenderer.h"

class RenderSurface::impl final
{
public:
	impl() {}
	~impl() {}

	bool Initialize(const std::string& source_path, Windows::UI::Xaml::Controls::SwapChainPanel^ swap_chain_panel)
	{
		if (source_path.empty())
			return false;
		if (swap_chain_panel == nullptr)
			return false;
		m_source_path = source_path;
		mOpenGLES = new OpenGLES();
		m_swap_chain_panel = swap_chain_panel;

		CreateRenderSurface();
		mOpenGLES->MakeCurrent(mRenderSurface);
		UpdateRenderer(DeviceCameraSensor::Unknown, DeviceCameraSensor::Unknown);
		m_ready = true;
		return true;
	}

	void Uninitialize() 
	{
		m_ready = false;
		DestroyRenderSurface();
		m_swap_chain_panel = nullptr;
		if (mOpenGLES)
		{
			delete mOpenGLES;
			mOpenGLES = nullptr;
		}
	}

	void UpdateDeviceCameraSensor(DeviceCameraSensor sensor)
	{
		//TODO: logic
		if (m_sensor == sensor)
		{
			return;
		}
		UpdateRenderer(sensor, m_sensor);
		m_sensor = sensor;

	}

	void RenderRGBImageData(const unsigned char *data, int width, int height)
	{
		if (!m_renderer)
			return;
		EGLint panelWidth = 0;
		EGLint panelHeight = 0;
		mOpenGLES->GetSurfaceDimensions(mRenderSurface, &panelWidth, &panelHeight);
		m_renderer->RenderRGBImage(data, width, height, panelWidth, panelHeight);

		if (mOpenGLES->SwapBuffers(mRenderSurface) != GL_TRUE)
		{
			mOpenGLES->Reset();
			mOpenGLES->MakeCurrent(mRenderSurface);
		}
	}

	bool Ready() const
	{
		return m_ready;
	}

	void SetVideoInfo(int fov_state, int lut_index)
	{
		m_fov_state = fov_state;
		m_lut_index = lut_index;

		m_renderer->m_fov_state = fov_state;
		m_renderer->m_lut_index = lut_index;
	}

private:

	void UpdateRenderer(DeviceCameraSensor sensor, DeviceCameraSensor pre_sensor)
	{
		if (sensor == DeviceCameraSensor::Unknown)
		{
			if (m_sensor == DeviceCameraSensor::Unknown && m_renderer)
				return;

			if (m_renderer)
			{
				delete m_renderer;
				m_renderer = nullptr;
			}
			m_renderer = new VideoFeedRenderer();
		}
		else
		{
			CalibrationVideoFeedRenderer* target = nullptr;
			if (pre_sensor == DeviceCameraSensor::Unknown)
			{
				delete m_renderer;
				m_renderer = nullptr;
			}
			if (!m_renderer)
			{
				target = new CalibrationVideoFeedRenderer(m_source_path);
			}
			else 
			{
				target = (CalibrationVideoFeedRenderer *)this;
			}

			target->SetDeviceSensor(sensor);
			m_renderer = target;

		}
	}

	void CreateRenderSurface()
	{
		if (mOpenGLES && mRenderSurface == EGL_NO_SURFACE)
		{
			mRenderSurface = mOpenGLES->CreateSurface(m_swap_chain_panel, nullptr, nullptr);
		}
	}

	void DestroyRenderSurface()
	{
		if (mOpenGLES)
		{
			mOpenGLES->DestroySurface(mRenderSurface);
		}
		mRenderSurface = EGL_NO_SURFACE;
	}


private:
	int m_fov_state = 0;
	int m_lut_index = 0;
	std::string m_source_path;
	VideoFeedRenderer * m_renderer = nullptr;
	DeviceCameraSensor m_sensor = DeviceCameraSensor::Unknown;
	bool m_ready = false;
	Windows::UI::Xaml::Controls::SwapChainPanel^ m_swap_chain_panel = nullptr;
	OpenGLES* mOpenGLES;
	EGLSurface mRenderSurface = EGL_NO_SURFACE;     // This surface is associated with a swapChainPanel on the page
};

RenderSurface::RenderSurface()
{
	pImpl = new impl;
}

RenderSurface::~RenderSurface()
{
	delete pImpl;
	pImpl = nullptr;
}

bool RenderSurface::Initialize(const std::string& source_path, Windows::UI::Xaml::Controls::SwapChainPanel^ swap_chain_panel)
{		
	return pImpl->Initialize(source_path, swap_chain_panel);
}

void RenderSurface::Uninitialize()
{
	pImpl->Uninitialize();
}

void RenderSurface::RenderRGBImageData(const unsigned char *data, int width, int height)
{
	pImpl->RenderRGBImageData(data, width, height);
}

void RenderSurface::UpdateDeviceCameraSensor(DeviceCameraSensor sensor)
{
	pImpl->UpdateDeviceCameraSensor(sensor);
}

bool RenderSurface::Ready() const
{
	return pImpl->Ready();
}

void RenderSurface::RenderSurface::SetVideoInfo(int fov_state, int lut_index)
{
	pImpl->SetVideoInfo(fov_state, lut_index);
}