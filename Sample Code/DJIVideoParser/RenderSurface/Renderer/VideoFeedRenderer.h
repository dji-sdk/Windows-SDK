#pragma once

#define GL_GLEXT_PROTOTYPES 
#include <GLES2\gl2.h>
#include <EGL\egl.h>
#include <angle_windowsstore.h>
#include <Windows.Foundation.Collections.h>
#include <GLES2\gl2ext.h>
#include <GLES2\gl2platform.h>
#include <gl\GL.h>
#include <gl\GLU.h>


//Simple Video Feed Renderer by which we could draw the RGB Video Feed on the screen
class VideoFeedRenderer
{
public:
	VideoFeedRenderer();
	virtual ~VideoFeedRenderer();

	virtual void RenderRGBImage(const uint8_t *data, int width, int height, int window_width, int window_height);

public:
	int m_lut_index = 0;
	int m_fov_state = 0;


protected:
	virtual void HandleImageTexture(const uint8_t *data, int width, int height, int window_width, int window_height);
	//virtual void DrawImageProcess();
	GLuint CompileShader(GLenum type, const std::string &source);
	GLuint CompileProgram(const std::string &vsSource, const std::string &fsSource);

protected:

	GLuint mProgram;
	GLsizei mWindowWidth;
	GLsizei mWindowHeight;
	
	GLsizei mPreWidth = 0;
	GLsizei mPreHeight = 0;

	// Attribute locations
	GLint mPositionLoc;
	GLint mTexCoordLoc;

	// Sampler location
	GLint mSamplerLoc;

	// Texture handle
	GLuint mTexture;

};