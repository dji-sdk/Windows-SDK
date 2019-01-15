#include "pch.h"
#include "CalibrationDataSource.h"
#include <string>
#include <sstream>
#include <fstream>
#include <vector>

#pragma pack(1)
typedef struct {
	uint16_t y_decimal : 5;//1/32
	uint16_t y_integer : 11;
	uint16_t x_decimal : 5;//1/32
	uint16_t x_integer : 11;
}DJICalibratuonLutCoordinateStruct;
#pragma pack()


static inline std::vector<BYTE> readFile(const char* filename)
{
	// open the file:
	std::streampos fileSize;
	std::ifstream file(filename, std::ios::binary);

	// get its size:
	file.seekg(0, std::ios::end);
	fileSize = file.tellg();
	file.seekg(0, std::ios::beg);

	// read the data:
	std::vector<BYTE> fileData(fileSize);
	file.read((char*)&fileData[0], fileSize);
	return fileData;
}

CalibrationDataSource::CalibrationDataSource()
{

}

CalibrationDataSource::~CalibrationDataSource()
{
}

bool CalibrationDataSource::Initialize(const std::string &files_path, DeviceCameraSensor sensor)
{
	std::lock_guard<std::mutex> lock_guard(m_mutex);
	m_sensor = sensor;
	m_files_path = files_path;
	if (m_files_path.back() != '\\')
	{
		m_files_path += "\\";
	}
	return true;
}

void CalibrationDataSource::Uninitialize()
{
	std::lock_guard<std::mutex> lock_guard(m_mutex);
	m_sensor = DeviceCameraSensor::Unknown;
	m_files_path.clear();
}

std::string CalibrationDataSource::GetSensorName()
{
	switch (m_sensor)
	{
	case DeviceCameraSensor::imx283:
		return "imx283";
	case DeviceCameraSensor::imx477:
		return "imx477";
	}
	return "";
}

void CalibrationDataSource::LoadDataFromSize(int width, int height)
{
	std::lock_guard<std::mutex> lock_guard(m_mutex);

	if ((width == m_last_width && height == m_last_height) || m_is_loading || width*height==0)
	{
		return;
	}
	m_last_width = width;
	m_last_height = height;

	m_is_loading = true;
	InternalLoadData();
	InternalLoadIndex();
	m_is_loading = false;
}

uint32_t CalibrationDataSource::ValidIndexCountForResulotion(int width, int height)
{
	if (m_sensor == DeviceCameraSensor::imx477)
	{
		return 32;
	}
	return 1;
}

int CalibrationDataSource::DataIndexForResolution(int width, int height, int lut_index, int fov_state)
{
	if (m_sensor == DeviceCameraSensor::imx477)
		return lut_index/2;
	return MAXINT32;
}

void CalibrationDataSource::InternalLoadIndex()
{
	uint32_t lut_width = m_last_width / 8;
	uint32_t lut_height = m_last_height / 8;

	int size_needed = 6 * lut_width * lut_height * sizeof(uint32_t);
	if (m_index_len < size_needed)
	{
		if (m_index_loaded)
		{
			free(m_index_loaded);
			m_index_loaded = nullptr;
		}
		m_index_len = 0;
	}
	if (!m_index_loaded)
	{
		m_index_len = size_needed;
		m_index_loaded = malloc(m_index_len);
	}

	uint32_t w = lut_width + 1;
	for (uint32_t y = 0; y < lut_height; y++) {
		uint32_t offsetY = y * lut_width * 6;
		uint32_t posY = y * w;
		for (uint32_t x = 0; x < lut_width; x++) {
			uint32_t offsetX = offsetY + x * 6;
			uint32_t* ptr = (uint32_t*)m_index_loaded + offsetX;
			ptr[0] = posY + x;
			ptr[1] = ptr[0] + w;
			ptr[2] = ptr[1] + 1;
			ptr[3] = ptr[0];
			ptr[4] = ptr[1] + 1;
			ptr[5] = ptr[0] + 1;
		}
	}
}

void CalibrationDataSource::InternalLoadData()
{
	uint32_t lutWidth = m_last_width / 8;
	uint32_t lutHeight = m_last_height / 8;
	//CGSize resolution = CGSizeMake(m_last_width, m_last_height);
	uint32_t w = lutWidth + 1;
	uint32_t h = lutHeight + 1;
	uint32_t idxCount = this->ValidIndexCountForResulotion(m_last_width, m_last_height);
	if (idxCount == 0) {
		if (m_data_loaded != NULL) {
			free(m_data_loaded);
			m_data_loaded = NULL;
		}
		m_loaded_len = 0;
		return;
	}
	uint32_t totalIdx = idxCount;
	uint32_t sizeNeeded = 4 * w * h * sizeof(float);
	uint32_t sizeNeededAll = sizeNeeded * totalIdx;
	if (m_loaded_len < sizeNeededAll) {
		if (m_data_loaded != NULL) {
			free(m_data_loaded);
			m_data_loaded = NULL;
		}
		m_loaded_len = 0;
	}
	if (!m_data_loaded) {
		m_loaded_len = sizeNeededAll;
		m_data_loaded = malloc(sizeNeededAll);
	}
	//first zone calculation
	float factorY = 1.0 / lutHeight;
	float factorX = 1.0 / lutWidth;
	for (uint32_t y = 0; y < h; y++) {
		uint32_t offsetY = y * w * 4;
		float vertexY = factorY * y;
		for (uint32_t x = 0; x < w; x++) {
			uint32_t offsetX = offsetY + x * 4;
			float* ptr = (float*)m_data_loaded + offsetX;
			ptr[2] = x * factorX;
			ptr[3] = vertexY;
			ptr[0] = ptr[2] * 2 - 1;
			ptr[1] = ptr[3] * 2 - 1;
		}
	}
	//copy from the index 0
	for (uint32_t index = 1; index < totalIdx; index++) {
		memcpy((uint8_t*)m_data_loaded + index * sizeNeeded,
			m_data_loaded,
			sizeNeeded);
	}
	if (use_calbration_file) {
		for (uint32_t index = 0; index < totalIdx; index++) {
			auto lut_res = this->TextureCoordinateData(m_last_width, m_last_height, index);
			void* lutData = &lut_res[0];
			int lutDataLen = lut_res.size();
			DJICalibratuonLutCoordinateStruct* lutBytes = nullptr;
			if (lutData
				&& lutDataLen >= (sizeof(DJICalibratuonLutCoordinateStruct) * h * w)) {
				lutBytes = (DJICalibratuonLutCoordinateStruct*)lutData;
			}
			if (!lutBytes) {
				continue;
			}
			float* dataLoaded = (float*)((uint8_t*)m_data_loaded + index * sizeNeeded);
			for (uint32_t y = 0; y < h; y++) {
				uint32_t offsetY = y * w * 4;
				for (uint32_t x = 0; x < w; x++) {
					uint32_t offsetX = offsetY + x * 4;
					float* ptr = dataLoaded + offsetX;
					ptr[2] = (lutBytes->x_integer + lutBytes->x_decimal / 32.0) / ((double)m_last_width>1.0e-3 ? m_last_width : 1.0e-3);
					ptr[3] = (lutBytes->y_integer + lutBytes->y_decimal / 32.0) / ((double)m_last_height>1.0e-3 ? m_last_height : 1.0e-3);
					lutBytes++;
				}
			}
		}
	}

}

std::vector<uint8_t> CalibrationDataSource::TextureCoordinateData(int width, int height, int index)
{
	//read data
	char file_name_buffer[60] = { 0 };
	snprintf(file_name_buffer, sizeof(file_name_buffer), "calibration_data\\%s_%dx%d_to_%dx%d_%d_0_0.bin", this->GetSensorName().c_str(), width, height, width, height, index);
	return readFile((m_files_path + file_name_buffer).c_str());
}


void CalibrationDataSource::GetVertexIndex(int width, int height, uint32_t*& data, int& size)
{
	uint32_t* retData = NULL;
	uint32_t dataSize = 0;//bytes
	bool ready = DataReady(width, height) ;
	if (ready) {
		uint32_t lutWidth = m_last_width / 8;
		uint32_t lutHeight = m_last_height / 8;
		uint32_t sizeNeeded = 6 * lutWidth * lutHeight * sizeof(uint32_t);
		if (sizeNeeded <= m_index_len
			&& m_index_loaded != NULL) {
			retData = (uint32_t*)m_index_loaded;
			dataSize = sizeNeeded;
		}
	}
	data = retData;
	size = dataSize;
}

void CalibrationDataSource::GetVertexData(int width, int height, float32*& data, int& stride_for_index, int& index_count)
{
	float32* retData = NULL;
	uint32_t strideForIndex = 0;//bytes
	uint32_t indexCount = 0;
	bool ready = DataReady(width, height);
	if (ready) {
		uint32_t lutWidth = m_last_width / 8;
		uint32_t lutHeight = m_last_height / 8;
		uint32_t w = lutWidth + 1;
		uint32_t h = lutHeight + 1;
		uint32_t sizeNeeded = 4 * w * h * sizeof(float32);
		uint32_t idxCount = ValidIndexCountForResulotion(width, height);
		if (m_loaded_len >= (sizeNeeded * idxCount)
			&& m_data_loaded != NULL) {
			retData = (float32*)m_data_loaded;
			strideForIndex = sizeNeeded;
			indexCount = idxCount;
		}
	}
	data = retData;
	stride_for_index = strideForIndex;
	index_count = indexCount;
}

bool CalibrationDataSource::DataReady(int width, int height)
{
	std::lock_guard<std::mutex> lock_guard(m_mutex);
	if (m_is_loading || width != m_last_width || height != m_last_height || width * height == 0 || m_sensor == DeviceCameraSensor::Unknown || m_files_path.empty())
	{
		return false;
	}
	return true;
}