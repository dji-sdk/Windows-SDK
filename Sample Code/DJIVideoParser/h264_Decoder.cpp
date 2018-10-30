#include "pch.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <algorithm> 
#include <iterator>
//#include <vld.h>

#pragma warning(disable : 4996)

#include "h264_Decoder.h"
#include "Utils.h"

#define LOG_TAG "h264_Decoder"
#include <plog/Log.h>
#include "VideoWrapper.h"

using namespace dji::videoparser;

static bool s_stop_flag = false;
static bool s_is_init_ffmpeg = false;

#define  HEAD_SIZE 12

bool Pack_Compositer::CompositePack(const uint8_t *data, const uint32_t size)
{
	m_packBuffer.insert(m_packBuffer.end(), data, data + size);

	while (!m_hasPackHeader && m_packBuffer.size() >= HEAD_SIZE)
	{
		const uint8_t* pData = (const uint8_t*)m_packBuffer.data();

		if (pData[0] == 0x00 && pData[1] == 0x00 && pData[2] == 0x01 && pData[3] == 0xff)
		{
			if (HeaderXor(pData))
			{
				m_hasPackHeader = true;
				break;
			}
		}

		m_packBuffer.erase(m_packBuffer.begin());

	}

	if (m_hasPackHeader)
	{
		m_pCurDataItem = (HPI_CHL_DATA*)(m_packBuffer.data());
		uint32_t totalSize = m_pCurDataItem->total_size();

		if (m_packBuffer.size() < totalSize) return false;

		m_hasPackHeader = false;

		if (m_pCurDataItem->chnl_id == m_channelId)
		{

			m_outBuffer.resize(m_pCurDataItem->payload_size());

			std::copy(m_pCurDataItem->data_byte, m_pCurDataItem->data_byte + m_pCurDataItem->payload_size(), &m_outBuffer.front());

			m_packBuffer.erase(m_packBuffer.begin(), m_packBuffer.begin() + totalSize);

			return true;
		}
		else {

			m_packBuffer.erase(m_packBuffer.begin(), m_packBuffer.begin() + totalSize);

		}
	}

	return false;
}

bool Pack_Compositer::HeaderXor(const uint8_t* pData)
{
	int result = 0;
	for (int i = 0; i < HEAD_SIZE - 1; i++)
	{
		result ^= pData[i];
	}

	return (result == 0);
}

h264_Decoder::h264_Decoder()
{

}

void h264_Decoder::Initialize(dji::videoparser::VideoWrapper* video_wrapper)
{
	m_videoWrapperPtr = video_wrapper;

	InitFFMPEG();

	m_thread_decoder = new std::thread([this]()
	{
		m_bDecoderRun = true;
		DecoderThread();
	});
}

void h264_Decoder::Uninitialize()
{
	m_bDecoderRun = false;
	m_thread_decoder->join();
	delete m_thread_decoder;
	m_thread_decoder = nullptr;

	m_videoWrapperPtr = nullptr;
}

h264_Decoder::~h264_Decoder()
{
	if (m_codec_context)
	{
		avcodec_close(m_codec_context);
		m_codec_context = nullptr;
	}

	if (m_outBuffer)
	{
		m_outBuffer = nullptr;
		delete[] m_outBuffer;
	}
	av_free(m_av_frame);
	av_free(m_dst_frame);
	av_free(m_codec_context);
	av_parser_close(m_codec_paser);
	sws_freeContext(m_sws_ctx);
}

bool h264_Decoder::InitFFMPEG()
{
	if (s_is_init_ffmpeg == false)
	{
		avcodec_register_all();
		av_register_all();
		s_is_init_ffmpeg = true;
	}

	m_av_codec = avcodec_find_decoder(AV_CODEC_ID_H264);
	m_codec_context = avcodec_alloc_context3(m_av_codec);
	m_codec_paser = av_parser_init(AV_CODEC_ID_H264);

	if (m_av_codec == nullptr || m_codec_context == nullptr)
	{
		return false;
	}

	m_codec_context->flags2 |= CODEC_FLAG2_FAST;
	m_codec_context->thread_count = 2;
	m_codec_context->thread_type = FF_THREAD_FRAME;

	if (m_av_codec->capabilities&CODEC_FLAG_LOW_DELAY) {
		m_codec_context->flags |= CODEC_FLAG_LOW_DELAY;
	}

	//if (m_av_codec->capabilities & CODEC_CAP_TRUNCATED)
	//        m_codec_context->flags |= CODEC_FLAG_TRUNCATED;

	//m_codec_context->thread_count = 4;
	//m_codec_context->thread_type = FF_THREAD_FRAME;

	if (avcodec_open2(m_codec_context, m_av_codec, nullptr) < 0)
	{
		m_av_codec = nullptr;
		return false;
	}

	m_av_frame = avcodec_alloc_frame();

	if (m_av_frame == nullptr)
	{
		LOGE << "av_frame_alloc fail: av frame.";
		return false;
	}

	m_dst_frame = avcodec_alloc_frame();

	if (m_dst_frame == nullptr)
	{
		LOGE << "av_frame_alloc fail : dst frame.";
		return false;
	}

	return true;
}

bool h264_Decoder::InitFrameBuffer()
{
	if (m_dst_frame)
	{
#ifdef _PC_
		prev_width_ = m_codec_context->width;
		prev_height_ = m_codec_context->height;
		m_outBuffer = new uint8_t[avpicture_get_size(PIX_FMT_BGRA, m_codec_context->width, m_codec_context->height)];

		avpicture_fill((AVPicture *)m_dst_frame, m_outBuffer, PIX_FMT_BGRA, m_codec_context->width, m_codec_context->height);

		m_sws_ctx = sws_getContext
		(
			m_codec_context->width,
			m_codec_context->height,
			m_codec_context->pix_fmt,
			m_codec_context->width,
			m_codec_context->height,
			PIX_FMT_BGRA,
			SWS_BILINEAR,
			nullptr,
			nullptr,
			nullptr
		);
#else
		m_outBuffer = new uint8_t[avpicture_get_size(AV_PIX_FMT_BGR24, m_codec_context->width, m_codec_context->height)];

		avpicture_fill((AVPicture *)m_dst_frame, m_outBuffer, AV_PIX_FMT_BGR24, m_codec_context->width, m_codec_context->height);

		m_sws_ctx = sws_getContext
		(
			m_codec_context->width,
			m_codec_context->height,
			m_codec_context->pix_fmt,
			m_codec_context->width,
			m_codec_context->height,
			AV_PIX_FMT_BGR24,
			SWS_BILINEAR,
			nullptr,
			nullptr,
			nullptr
		);

#endif


		return true;
	}

	return false;

}

void h264_Decoder::Stop()
{
	s_stop_flag = true;
}

void h264_Decoder::DecoderThread()
{
	while (m_bDecoderRun)
	{
		std::vector<uint8_t> vecbuffer;

		static bool is_sps_pps_found = false;

		while (m_vectorSafeQueue.wait_for_item([this] {return !s_stop_flag; }))
		{
			if (s_stop_flag)
				break;

			std::vector<uint8_t> queue_vector;
			m_vectorSafeQueue.wait_and_pop(queue_vector);

			uint8_t *pFrameBuff = nullptr;
			int parser_length_in = queue_vector.size();
			if (parser_length_in > 0)
			{
				pFrameBuff = &queue_vector[0];
			}

			uint8_t *data = nullptr;
			int size = 0, got_picture = 0;

			while (parser_length_in > 0)
			{
				AVPacket packet;
				av_init_packet(&packet);

				auto parser_len = av_parser_parse2(m_codec_paser, m_codec_context, &packet.data, &packet.size, pFrameBuff,
					parser_length_in, AV_NOPTS_VALUE, AV_NOPTS_VALUE, AV_NOPTS_VALUE);

				parser_length_in -= parser_len;
				pFrameBuff += parser_len;

				if (packet.size > 0) {
					if (!is_sps_pps_found)
					{
						is_sps_pps_found = m_codec_paser->frame_has_sps ? true : false;
					}

					if (!is_sps_pps_found)
					{
						av_free_packet(&packet);
						continue;
					}

					got_picture = 0;
					auto len = avcodec_decode_video2(m_codec_context, m_av_frame, &got_picture, &packet);

					if (!got_picture) {
						av_free_packet(&packet);
						continue;
					}

					if (!m_bInitBuffer)
					{
						m_bInitBuffer = InitFrameBuffer();
					}

					if (prev_height_ != m_codec_context->height ||
						prev_width_ != m_codec_context->width)
					{
						delete m_outBuffer;
						m_outBuffer = new uint8_t[avpicture_get_size(PIX_FMT_BGRA, m_codec_context->width, m_codec_context->height)];
						avpicture_fill((AVPicture *)m_dst_frame, m_outBuffer, PIX_FMT_BGRA, m_codec_context->width, m_codec_context->height);
						prev_height_ = m_codec_context->height;
						prev_width_ = m_codec_context->width;
					}

					sws_scale
					(
						m_sws_ctx,
						(uint8_t const * const *)m_av_frame->data,
						m_av_frame->linesize,
						0,
						m_codec_context->height,
						m_dst_frame->data,
						m_dst_frame->linesize
					);

					auto buffSize = int(m_codec_context->width * m_codec_context->height * 4);

					if (m_videoWrapperPtr)
					{
						m_videoWrapperPtr->FramePacket(m_dst_frame->data[0], buffSize, FrameType_Video, m_codec_context->width, m_codec_context->height);
					}

				}
				av_free_packet(&packet);
			}
		}
	}
}

int h264_Decoder::videoFrameParse(const uint8_t* buff, int video_size, FrameType type, uint64_t pts)
{
	if (video_size <= 0)
	{
		return 0;
	}

	// Need padding for FFMpeg. Otherwise Address Sanitizer will complain heap overflow.
	auto buf_vec = std::vector<uint8_t>();
	buf_vec.resize(video_size + FF_INPUT_BUFFER_PADDING_SIZE);
	buf_vec.assign(buff, buff + video_size);
	m_vectorSafeQueue.push(std::move(buf_vec));

	return 1;

}
