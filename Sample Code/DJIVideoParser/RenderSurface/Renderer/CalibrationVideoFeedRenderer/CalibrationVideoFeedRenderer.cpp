#include "pch.h"
#include "CalibrationVideoFeedRenderer.h"


static inline GLuint GenGLBuffer(GLenum target, GLenum usage, GLsizei size, void* data)
{
	GLuint buffer = 0;
	glGenBuffers(1, &buffer);
	glBindBuffer(target, buffer);
	glBufferData(target, size, data, usage);
	glBindBuffer(target, 0);

	return buffer;
}

CalibrationVideoFeedRenderer::CalibrationVideoFeedRenderer(std::string source_path) : VideoFeedRenderer()
{
	m_source_path = std::move(source_path);
	m_data_source = new CalibrationDataSource();
}

CalibrationVideoFeedRenderer::~CalibrationVideoFeedRenderer()
{
	ReleaseCachedData();
	if (m_sensor != DeviceCameraSensor::Unknown)
	{
		m_data_source->Uninitialize();
	}
	delete m_data_source;
	m_data_source = nullptr;
}

void CalibrationVideoFeedRenderer::SetDeviceSensor(DeviceCameraSensor sensor)
{
	if (m_sensor == sensor)
	{
		return;
	}
	if (m_sensor != DeviceCameraSensor::Unknown)
	{
		m_data_source->Uninitialize();
	}
	if (sensor != DeviceCameraSensor::Unknown)
	{
		m_data_source->Initialize(m_source_path, sensor);
	}
	m_sensor = sensor;
}

void CalibrationVideoFeedRenderer::ReleaseCachedData()
{
	if (m_index_buffer != 0)
	{
		glDeleteBuffers(1, &m_index_buffer);
		m_index_buffer = 0;
	}
	for (auto vertex : m_vector_vertex_buffer)
	{
		glDeleteBuffers(1, &vertex);
	}
	m_vertex_buffer = 0;
	m_vector_vertex_buffer.clear();
}

void CalibrationVideoFeedRenderer::RenderRGBImage(const uint8_t *data, int width, int height, int window_width, int window_height)
{
	bool draw_without_calibration = false;
	do
	{

		if (width != mPreWidth || height != mPreHeight)
		{
			ReleaseCachedData();
		}

		if (!m_data_source || !m_data_source->DataReady(width, height))
		{
			if (m_data_source && !m_data_source->GetIsLoading())
			{
				std::thread([this, width, height]
				{
					m_data_source->LoadDataFromSize(width, height);
				}).detach();
			}
			draw_without_calibration = true;
			break;
		}


		if (m_vertex_buffer != 0) {
			int index = m_lut_index;
			int fovState = m_fov_state;
			int dataIndex = MAXINT32;
			if (m_data_source != nullptr) {
				dataIndex = m_data_source->DataIndexForResolution(width, height, index, fovState);
			}
			if (dataIndex != MAXINT32
				&& dataIndex < m_vector_vertex_buffer.size()) {
				GLuint vertexBufferValue = m_vector_vertex_buffer[dataIndex];
				if (vertexBufferValue != m_vertex_buffer) {
					m_vertex_buffer = 0;
				}
			}
			else {
				m_vertex_buffer = 0;
			}
		}

		if (m_vertex_buffer == 0
			&& width != 0
			&& height != 0) {
			UpdateVertextBuffer(width, height);
		}
		if (m_index_buffer == 0
			&& width != 0
			&& height != 0) {
			UpdateIndexBuffer(width, height);
		}

		if (m_vertex_buffer == 0
			|| m_index_buffer == 0
			|| width == 0
			|| height == 0) {
			draw_without_calibration = true;
			break;
		}

		HandleImageTexture(data, width, height, window_width, window_height);

		glUseProgram(mProgram);
		glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index_buffer);

		int offset = 0;
		glVertexAttribPointer(mPositionLoc,
			2,
			GL_FLOAT,
			false,
			4 * sizeof(GLfloat),
			(void*)offset);
		offset += 2 * sizeof(GLfloat);
		glVertexAttribPointer(mTexCoordLoc,
			2,
			GL_FLOAT,
			false,
			4 * sizeof(GLfloat),
			(void*)offset);

		glEnableVertexAttribArray(mPositionLoc);
		glEnableVertexAttribArray(mTexCoordLoc);

		// Bind the texture
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mTexture);

		// Set the texture sampler to texture unit to 0
		glUniform1i(mSamplerLoc, 0);

		GLuint lutWidth = width / 8;
		GLuint lutHeight = height / 8;
		glDrawElements(GL_TRIANGLES,
			lutWidth * lutHeight * 6,
			GL_UNSIGNED_INT,
			0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		mPreWidth = width;
		mPreHeight = height;

	} while (0);

	if (draw_without_calibration)
	{
		VideoFeedRenderer::RenderRGBImage(data, width, height, window_width, window_height);
	}

}


void CalibrationVideoFeedRenderer::UpdateVertextBuffer(int width, int height)
{
	GLuint lutWidth = width / 8;
	GLuint lutHeight = height / 8;
	GLuint w = lutWidth + 1;
	GLuint h = lutHeight + 1;
	int sizeNeeded = 4 * w * h * sizeof(GLfloat);
	int idx = m_lut_index;
	int fovState = m_fov_state;
	int dataIndex = MAXINT32;
	if (m_data_source != nullptr) {
		dataIndex = m_data_source->DataIndexForResolution(width, height, idx, fovState);
	}
	if (m_vector_vertex_buffer.size() > 0) {
		int index = min(dataIndex, m_vector_vertex_buffer.size() - 1);
		m_vertex_buffer = m_vector_vertex_buffer[index];
		return;
	}
	GLfloat* vertex = nullptr;
	int stride = 0;
	int total_index = 0;
	m_data_source->GetVertexData(width, height, vertex, stride, total_index);
	if (vertex == nullptr
		|| stride < sizeNeeded
		|| total_index <= 0) {
		return;
	}

	std::vector<GLuint> new_vector_vertex_buffer;
	for (int index = 0; index < total_index; index++) {
		//TODO 

		GLuint vertexBuffer = GenGLBuffer(GL_ARRAY_BUFFER, GL_STATIC_DRAW, (GLuint)stride, ((uint8_t*)vertex + index * stride));
		new_vector_vertex_buffer.push_back(vertexBuffer);
	}
	m_vector_vertex_buffer = std::move(new_vector_vertex_buffer);
	if (new_vector_vertex_buffer.size() > 0) {
		int index = min(dataIndex, new_vector_vertex_buffer.size() - 1);
		m_vertex_buffer = new_vector_vertex_buffer[index];
	}
}


void CalibrationVideoFeedRenderer::UpdateIndexBuffer(int width, int height)
{
	GLuint lutWidth = width / 8;
	GLuint lutHeight = height / 8;
	int sizeNeeded = 6 * lutWidth * lutHeight * sizeof(GLuint);
	if (!m_data_source)
	{
		return;
	}
	uint32_t *indexes = nullptr;
	int size = 0;
	m_data_source->GetVertexIndex(width, height, indexes, size);
	if (indexes == nullptr
		|| size < sizeNeeded) {
		return;
	}
	m_index_buffer = GenGLBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, (GLuint)size, indexes);
}

