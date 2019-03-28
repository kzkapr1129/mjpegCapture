#pragma once

#include "inputStream.h"
#include <opencv2/core.hpp>

class HttpReader;
class MJpegCapture {
public:
	MJpegCapture(InputStream& stream);

	void start();

	MJpegCapture& operator >> (cv::Mat& image);

private:
	static void* decodeThread(void* arg);
	void readFrame(HttpReader& reader, const std::string& boundary);
	void startDecoding(HttpReader& reader);

	InputStream& mStream;
	cv::Mat mCurImg;
};