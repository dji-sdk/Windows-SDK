#include "pch.h"
#include "VideoFeedRenderer.h"

using namespace Platform;

VideoFeedRenderer::VideoFeedRenderer() :
	mWindowWidth(0),
	mWindowHeight(0)
{
	static const std::string vs =
		R"(attribute vec4 a_position;
            attribute vec2 a_texCoord;
            varying vec2 v_texCoord;
            void main()
            {
                gl_Position = a_position;
                v_texCoord = vec2(a_texCoord.x, 1.0-a_texCoord.y);
            })";

	static const std::string fs =
		R"(precision mediump float;
            varying vec2 v_texCoord;
            uniform sampler2D s_texture;
            void main()
            {
                gl_FragColor = texture2D(s_texture, v_texCoord);
            })";

	try
	{
		mProgram = CompileProgram(vs, fs);
	}
	catch (Exception ^ exception)
	{
		return;
	}
	if (!mProgram)
	{
		return;
	}

	// Get the attribute locations
	mPositionLoc = glGetAttribLocation(mProgram, "a_position");
	mTexCoordLoc = glGetAttribLocation(mProgram, "a_texCoord");

	// Get the sampler location
	mSamplerLoc = glGetUniformLocation(mProgram, "s_texture");

	// Load the texture
	glGenTextures(1, &mTexture);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

}


VideoFeedRenderer::~VideoFeedRenderer()
{
	glDeleteProgram(mProgram);
	if (mTexture)
		glDeleteTextures(1, &mTexture);
}


GLuint VideoFeedRenderer::CompileShader(GLenum type, const std::string &source)
{
	GLuint shader = glCreateShader(type);

	const char *sourceArray[1] = { source.c_str() };
	glShaderSource(shader, 1, sourceArray, NULL);
	glCompileShader(shader);

	GLint compileResult;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compileResult);

	if (compileResult == 0)
	{
		GLint infoLogLength;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);

		std::vector<GLchar> infoLog(infoLogLength);
		glGetShaderInfoLog(shader, (GLsizei)infoLog.size(), NULL, infoLog.data());

		std::wstring errorMessage = std::wstring(L"Shader compilation failed: ");
		errorMessage += std::wstring(infoLog.begin(), infoLog.end());

		throw Exception::CreateException(E_FAIL, ref new Platform::String(errorMessage.c_str()));
	}

	return shader;
}

GLuint VideoFeedRenderer::CompileProgram(const std::string &vsSource, const std::string &fsSource)
{
	GLuint program = glCreateProgram();

	if (program == 0)
	{
		throw Exception::CreateException(E_FAIL, L"Program creation failed");
	}

	GLuint vs = CompileShader(GL_VERTEX_SHADER, vsSource);
	GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fsSource);

	if (vs == 0 || fs == 0)
	{
		glDeleteShader(fs);
		glDeleteShader(vs);
		glDeleteProgram(program);
		return 0;
	}

	glAttachShader(program, vs);
	glDeleteShader(vs);

	glAttachShader(program, fs);
	glDeleteShader(fs);

	glLinkProgram(program);

	GLint linkStatus;
	glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);

	if (linkStatus == 0)
	{
		GLint infoLogLength;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);

		std::vector<GLchar> infoLog(infoLogLength);
		glGetProgramInfoLog(program, (GLsizei)infoLog.size(), NULL, infoLog.data());

		std::wstring errorMessage = std::wstring(L"Program link failed: ");
		errorMessage += std::wstring(infoLog.begin(), infoLog.end());

		throw Exception::CreateException(E_FAIL, ref new Platform::String(errorMessage.c_str()));
	}

	return program;
}

void VideoFeedRenderer::RenderRGBImage(const uint8_t *data, int width, int height, int window_width, int window_height)
{
	HandleImageTexture(data, width, height, window_width, window_height);

	// Use the program object
	glUseProgram(mProgram);

	GLfloat vertices[] =
	{
		-1.0f,  1.0f, 0.0f,  // Position 0
		0.0f,  1.0f,        // TexCoord 0
		-1.0f, -1.0f, 0.0f,  // Position 1
		0.0f,  0.0f,        // TexCoord 1
		1.0f, -1.0f, 0.0f,  // Position 2
		1.0f,  0.0f,        // TexCoord 2
		1.0f,  1.0f, 0.0f,  // Position 3
		1.0f,  1.0f         // TexCoord 3
	};
	GLushort indices[] = { 0, 1, 2, 0, 2, 3 };

	// Clear the color buffer
	glClear(GL_COLOR_BUFFER_BIT);

	// Load the vertex position
	glVertexAttribPointer(mPositionLoc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), vertices);
	// Load the texture coordinate
	glVertexAttribPointer(mTexCoordLoc, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), vertices + 3);

	glEnableVertexAttribArray(mPositionLoc);
	glEnableVertexAttribArray(mTexCoordLoc);

	// Bind the texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mTexture);

	// Set the texture sampler to texture unit to 0
	glUniform1i(mSamplerLoc, 0);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

	mPreWidth = width;
	mPreHeight = height;
}

void VideoFeedRenderer::HandleImageTexture(const uint8_t *data, int width, int height, int window_width, int window_height)
{
	mWindowWidth = width;
	mWindowHeight = height;

	float w_scale = (float)window_width / width;
	float h_scale = (float)window_height / height;

	float target_width = w_scale < h_scale ? window_width : (float)window_height * (float)width / (float)height;
	float target_height = w_scale > h_scale ? window_height : (float)window_width * (float)height / (float)width;

	glViewport((window_width - target_width)*0.5, (window_height - target_height)*0.5, target_width, target_height);
	
	if (mPreWidth != width || mPreHeight != height)
	{
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		// Bind the texture object
		glBindTexture(GL_TEXTURE_2D, mTexture);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

		// Set the filtering mode
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else
	{
		glBindTexture(GL_TEXTURE_2D, mTexture);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
	}

	// Set the viewport
	//glViewport(0, 0, mWindowWidth, mWindowHeight);

	//glBindTexture(GL_TEXTURE_2D, mTexture);
	//glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);


	// Clear the color buffer
	glClear(GL_COLOR_BUFFER_BIT);
	
}
