#pragma once

// Enable function definitions in the GL headers below
#define GL_GLEXT_PROTOTYPES

// OpenGL ES includes
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

// EGL includes
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglplatform.h>

#include <angle_windowsstore.h>

class OpenGLES
{
public:
    OpenGLES();
    ~OpenGLES();

    EGLSurface CreateSurface(Windows::UI::Xaml::Controls::SwapChainPanel^ panel, const Windows::Foundation::Size* renderSurfaceSize, const float* renderResolutionScale);
    void GetSurfaceDimensions(const EGLSurface surface, EGLint *width, EGLint *height);
    void DestroySurface(const EGLSurface surface);
    void MakeCurrent(const EGLSurface surface);
    EGLBoolean SwapBuffers(const EGLSurface surface);
    void Reset();

private:
    void Initialize();
    void Cleanup();

private:
    EGLDisplay mEglDisplay;
    EGLContext mEglContext;
    EGLConfig  mEglConfig;
};
