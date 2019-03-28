#include "inputStreamFile.h"
#include "httpReader.h"
#include "httpClient.h"
#include "mjpegCapture.h"
#include <iostream>
#include <opencv2/opencv.hpp>
#include <unistd.h>

int main() {
	Response* response = HttpClient::request("192.168.11.2", 8080, "/?action=stream");
	MJpegCapture cap(*response);
	cap.start();

	while (true) {
		cv::Mat frame;

		cap >> frame;

		if (frame.empty()) {
			sleep(1);
			continue;
		}

		cv::imshow("frame", frame);
		cv::waitKey(1);
	}

	return 0;
}