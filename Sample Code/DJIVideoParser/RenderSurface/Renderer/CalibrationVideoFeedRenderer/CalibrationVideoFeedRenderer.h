#pragma once 

#include <iostream>
#include "../VideoFeedRenderer.h"
#include "CalibrationDataSource.h"

//Calibrate the image and draw 
class CalibrationVideoFeedRenderer final : public VideoFeedRenderer 
{
public:
	CalibrationVideoFeedRenderer(std::string source_path);
	virtual ~CalibrationVideoFeedRenderer();

	virtual void RenderRGBImage(const uint8_t *data, int width, int height, int window_width, int window_height) override;
	void SetDeviceSensor(DeviceCameraSensor sensor);

private:
	void UpdateVertextBuffer(int width, int height);
	void UpdateIndexBuffer(int width, int height);

	void ReleaseCachedData();

private:
	std::string m_source_path;
	DeviceCameraSensor m_sensor = DeviceCameraSensor::Unknown;
	CalibrationDataSource * m_data_source = nullptr;

	uint8_t * m_res_data = nullptr;
	int m_res_data_size = 0;

	GLuint m_vertex_buffer = 0;
	GLuint m_index_buffer = 0;
	std::vector<GLuint> m_vector_vertex_buffer;
};