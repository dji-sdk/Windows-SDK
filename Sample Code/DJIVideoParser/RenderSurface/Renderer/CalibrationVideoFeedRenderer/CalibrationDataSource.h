#pragma once
#include <iostream>
#include <vector>
#include <mutex>

class CalibrationDataSource final
{
public:
	CalibrationDataSource();
	virtual ~CalibrationDataSource();

	bool Initialize(const std::string &files_path, DeviceCameraSensor sensor);
	void Uninitialize();

	int DataIndexForResolution(int width, int height, int lut_index, int fov_state);

public:
	void LoadDataFromSize(int width, int height);
	void GetVertexIndex(int width, int height, uint32_t*& data, int& size);
	void GetVertexData(int width, int height, float32*& data, int& stride_for_index, int& index_count);
	bool DataReady(int width, int height);
	bool GetIsLoading() { return m_is_loading; };
private:
	uint32_t ValidIndexCountForResulotion(int width, int height);
	std::vector<uint8_t> TextureCoordinateData(int width, int height, int index);
	std::string GetSensorName();

private:
	void InternalLoadIndex();
	void InternalLoadData();

private:
	DeviceCameraSensor m_sensor = DeviceCameraSensor::Unknown;
	std::string m_files_path;

	bool m_is_loading = false;

	int m_last_width = 0;
	int m_last_height = 0;

	void *m_data_loaded = nullptr;
	int m_loaded_len = 0;

	void *m_index_loaded = nullptr;
	int m_index_len = 0;

	bool use_calbration_file = true;

	std::mutex m_mutex;
};

