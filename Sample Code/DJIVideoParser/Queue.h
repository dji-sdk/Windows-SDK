#ifndef __FRAME_QUEUE__
#define __FRAME_QUEUE__

//#include <jni.h>
#include <pthread.h>

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/mem.h"
#include "libavutil/time.h"
}

#define MAX_QUEUE_SIZE  1000

enum FrameType {
	FrameType_VideoLive,
	FrameType_Video,
	FrameType_Audio
};

typedef struct stBuffNode {

	uint64_t pts;
	uint8_t* m_pFrameBuff;
	short* m_pAudioBuff;
	stBuffNode* next;
	FrameType type;
	int m_iSize;
	int m_width;
	int m_height;
	int isIFrame;
	int hasSps;
	int hasPps;
	int frameNum;
	int timeStamp;

	stBuffNode() {
		pts = -1;
		m_pFrameBuff = NULL;
		m_pAudioBuff = NULL;
		next = NULL;
		m_iSize = 0;
		m_width = 0;
		m_height = 0;
		type = FrameType_Video;
	}

	~stBuffNode() {
		if (m_pFrameBuff) {
			av_free(m_pFrameBuff);
		}
		if (m_pAudioBuff) {
			free(m_pAudioBuff);
		}
	}
}stBuffNode;

class CFrameQueue {
public:
	CFrameQueue();
	~CFrameQueue();
	void clear();
	int Size();
	void Delete();
	stBuffNode* Get();
	int Push(uint8_t* pBuff, int iSize);
	int Push(uint8_t* pBuff, int iSize, FrameType type, uint64_t pts);
	int Push(uint8_t* pBuff, int iSize, FrameType type, uint64_t pts, int isKeyFrame, int hasSps, int hasPps, int frameNum);
	int Push(uint8_t* pBuff, int iSize, FrameType type, int width, int height);
	int Push(uint8_t* pBuff, int iSize, FrameType type, uint64_t pts, int width, int height);
	int Push(uint8_t* pBuff, int iSize, FrameType type, uint64_t pts, int width, int height, int isKeyFrame, int hasSps, int hasPps, int frameNum);
	int Push(short* pBuff, int iSize, FrameType type, int timeStamp);

	//	int Push(uint8_t* pBuff,int iSize, int type);
	//	int Push(short* pBuff, int iSize, int type);
	//	int PushStreaming(uint8_t* pBuff,int iSize, int type);
	int Push(uint8_t* pBuff, int iSize, FrameType type, int width, int height, int isIFrame, int timeStamp);

private:
	int m_iCurrentSize;
	stBuffNode* m_pQueueHead;
	stBuffNode* m_pQueueRear;
	pthread_mutex_t m_Lock;
	pthread_cond_t m_cond;
private:
};

#endif // FFMPEG_MEDIAPLAYER_H


