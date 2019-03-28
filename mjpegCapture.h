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

	MJpegCapture& operator >> (cv::Mat& image);

private:
	static void* decodeThread(void* arg);
	void readFrame(HttpReader& reader, const std::string& boundary);
	void startDecoding(HttpReader& reader);

	InputStream* mStream;
	cv::Mat mCurImg;
	uint8_t* mTmpBuffer;
	size_t mTmpBufferLen;
	pthread_t mThread;
	volatile bool mStopRequest;
};