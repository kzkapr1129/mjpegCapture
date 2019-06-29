#pragma once

#include "inputStream.h"
#include <opencv2/core.hpp>
#include <pthread.h>

class HttpReader;
class MJpegCapture {
public:
	MJpegCapture();
	~MJpegCapture();

	int start(const char* ip, int port, const char* uri);
	void stop();
	bool isRunning() const;

	MJpegCapture& operator >> (cv::Mat& image);

private:
	static void* decodeThread(void* arg);
	void doDecoding(HttpReader& reader);
	void readFrame(HttpReader& reader, const std::string& boundary);

	InputStream* mStream;
	cv::Mat mCurImg;
	uint8_t* mTmpBuffer;
	size_t mTmpBufferLen;
	pthread_t mThread;
	pthread_mutex_t mMutex;
	pthread_cond_t mCond;
	volatile bool mIsRunning;
	volatile bool mStopRequest;
};