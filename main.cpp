#include "mjpegCapture.h"
#include <iostream>
#include <opencv2/opencv.hpp>
#include <unistd.h>

int main(int argc, char* argv[]) {
	if (argc != 2) {
		printf("Usage: %s ip_addr\n", argv[0]);
		return 0;
	}

	MJpegCapture cap;
	if (cap.start(argv[1], 8080, "/?action=stream")) {
		printf("failed to connect: %s\n", argv[1]);
		return -1;
	}

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