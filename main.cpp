#include "mjpegCapture.h"
#include <iostream>
#include <opencv2/opencv.hpp>
#include <unistd.h>

int main() {
	MJpegCapture cap;
	cap.start("192.168.11.6", 8080, "/?action=stream");

	while (true) {
		cv::Mat frame;

		cap >> frame;

		if (frame.empty()) {
			sleep(1);
			continue;
		}

		cv::imshow("frame", frame);
		int key = cv::waitKey(1);
		if (key == 27) {
			break;
		}
	}

	cap.stop();

	printf("exit main\n");
	return 0;
}