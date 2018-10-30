#include "pch.h"
#include "Queue.h"

#undef LOG_TAG
#define LOG_TAG "Queue"

CFrameQueue::CFrameQueue() {
	pthread_mutex_init(&m_Lock, NULL);
	pthread_cond_init(&m_cond, NULL);
	m_pQueueRear = m_pQueueHead = NULL;
	m_iCurrentSize = 0;
}
CFrameQueue::~CFrameQueue() {

	stBuffNode* pNode = NULL;
	stBuffNode* pNodeNext = NULL;
	pNode = m_pQueueHead;
	while (pNode) {
		pNodeNext = pNode->next;
		delete (pNode);
		pNode = pNodeNext;
	}
	pthread_mutex_destroy(&m_Lock);
}

void CFrameQueue::clear() {
	pthread_mutex_lock(&m_Lock);

	stBuffNode* pNode = NULL;
	stBuffNode* pNodeNext = NULL;
	pNode = m_pQueueHead;
	while (pNode) {
		pNodeNext = pNode->next;
		delete (pNode);
		pNode = pNodeNext;
	}
	m_iCurrentSize = 0;
	m_pQueueRear = m_pQueueHead = NULL;
	pthread_mutex_unlock(&m_Lock);
}

int CFrameQueue::Size() {
	return m_iCurrentSize;
}

int CFrameQueue::Push(uint8_t* pBuff, int iSize) {
	return Push(pBuff, iSize, FrameType_Video, -1);
}

int CFrameQueue::Push(uint8_t* pBuff, int iSize, FrameType type, uint64_t pts) {
	return Push(pBuff, iSize, type, pts, 0, 0);
}

stBuffNode* CFrameQueue::Get() {

	pthread_mutex_lock(&m_Lock);

	if (m_iCurrentSize == 0) {
		pthread_mutex_unlock(&m_Lock);
		return NULL;
	}
	stBuffNode* pNode = m_pQueueHead;
	m_pQueueHead = pNode->next;

	if (m_pQueueHead == NULL) {
		m_pQueueRear = NULL;
	}
	m_iCurrentSize--;
	pthread_mutex_unlock(&m_Lock);
	return pNode;
}



int CFrameQueue::Push(uint8_t* pBuff, int iSize, FrameType type, int width, int height) {
	return Push(pBuff, iSize, type, -1, width, height);
}

int CFrameQueue::Push(uint8_t* pBuff, int iSize, FrameType type, uint64_t pts, int width, int height)
{
	return Push(pBuff, iSize, type, pts, width, height, 0, 0, 0, -1);
}

int CFrameQueue::Push(uint8_t* pBuff, int iSize, FrameType type, uint64_t pts, int isKeyFrame, int hasSps, int hasPps, int frameNum)
{
	return Push(pBuff, iSize, type, pts, 0, 0, isKeyFrame, hasSps, hasPps, frameNum);
}

int CFrameQueue::Push(uint8_t* pBuff, int iSize, FrameType type, uint64_t pts, int width, int height,
	int isKeyFrame, int hasSps, int hasPps, int frameNum) {
	stBuffNode* pNode = new stBuffNode;
	if (pNode == NULL) {
		return -1;
	}
	pNode->pts = pts;
	pNode->m_pFrameBuff = (uint8_t*)av_malloc(iSize);
	if (pNode->m_pFrameBuff == NULL) {
		delete pNode;
		return -1;
	}
	memcpy(pNode->m_pFrameBuff, pBuff, iSize);
	pNode->m_iSize = iSize;
	pNode->type = type;
	pNode->next = NULL;
	pNode->m_width = width;
	pNode->m_height = height;
	pNode->isIFrame = isKeyFrame;
	pNode->hasSps = hasSps;
	pNode->hasPps = hasPps;
	pNode->frameNum = frameNum;

	pthread_mutex_lock(&m_Lock);
	if (m_iCurrentSize > MAX_QUEUE_SIZE) {
		pthread_mutex_unlock(&m_Lock);
		return -1;
	}

	if (m_pQueueRear == NULL) {
		m_pQueueRear = m_pQueueHead = pNode;
	}
	else {
		m_pQueueRear->next = pNode;
		m_pQueueRear = pNode;
	}

	m_iCurrentSize++;
	pthread_cond_signal(&m_cond);
	pthread_mutex_unlock(&m_Lock);
	return m_iCurrentSize;
}

int CFrameQueue::Push(short* pBuff, int iSize, FrameType type, int timeStamp) {

	stBuffNode* pNode = new stBuffNode;
	if (pNode == NULL) {
		return -1;
	}
	pNode->m_pAudioBuff = (short*)malloc(iSize * sizeof(short));
	if (pNode->m_pAudioBuff == NULL) {
		delete pNode;
		return -1;
	}
	memcpy(pNode->m_pAudioBuff, pBuff, iSize * sizeof(short));
	pNode->timeStamp = timeStamp;
	//	LOGE("audio push pNode pts: %d, pts: %d", pNode->timeStamp, timeStamp);
	pNode->m_iSize = iSize;
	pNode->type = type;
	int64_t start_time = av_gettime();
	//LOGE("audio type %d", pNode->type);
	//LOGE("log time %d", (av_gettime() - start_time)/1000);
	pNode->next = NULL;

	pthread_mutex_lock(&m_Lock);
	if (m_iCurrentSize > MAX_QUEUE_SIZE) {
		pthread_mutex_unlock(&m_Lock);
		return -1;
	}
	if (m_pQueueRear == NULL) {
		m_pQueueRear = m_pQueueHead = pNode;
	}
	else {
		m_pQueueRear->next = pNode;
		m_pQueueRear = pNode;
	}

	m_iCurrentSize++;
	pthread_mutex_unlock(&m_Lock);
	pthread_cond_signal(&m_cond);
	return m_iCurrentSize;
}

int CFrameQueue::Push(uint8_t* pBuff, int iSize, FrameType type, int width, int height, int isIFrame, int timeStamp) {

	stBuffNode* pNode = new stBuffNode;
	if (pNode == NULL) {
		return -1;

	}
	if (type == 1) {
		//		LOGE("alloc buf size %d", iSize);
	}
	pNode->m_pFrameBuff = (uint8_t*)av_malloc(iSize);
	if (pNode->m_pFrameBuff == NULL) {
		delete pNode;
		return -1;
	}
	memcpy(pNode->m_pFrameBuff, pBuff, iSize);
	pNode->m_iSize = iSize;
	pNode->type = type;
	pNode->next = NULL;
	pNode->m_width = width;
	pNode->m_height = height;
	pNode->isIFrame = isIFrame;
	pNode->timeStamp = timeStamp;

	pthread_mutex_lock(&m_Lock);
	if (m_iCurrentSize > MAX_QUEUE_SIZE) {
		pthread_mutex_unlock(&m_Lock);
		return -1;
	}


	if (m_pQueueRear == NULL) {
		m_pQueueRear = m_pQueueHead = pNode;
	}
	else {
		m_pQueueRear->next = pNode;
		m_pQueueRear = pNode;
	}

	m_iCurrentSize++;
	//LOGI("push now size %d" ,m_iCurrentSize);
	pthread_cond_signal(&m_cond);
	pthread_mutex_unlock(&m_Lock);
	return m_iCurrentSize;
}
