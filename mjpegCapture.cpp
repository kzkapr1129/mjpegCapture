#include "mjpegCapture.h"
#include "httpReader.h"
#include <opencv2/opencv.hpp>
#include <pthread.h>

void MJpegCapture::readFrame(HttpReader& reader, const std::string& boundary) {
	const Token& token = reader.token();

	// Content-Lengthまで読み飛ばし
	do {
		reader.readNextToken();
	} while (token.value != "Content-Length");

	// デリミタの読み飛ばし
	do {
		reader.readNextToken();
	} while (token.isDelimitor);

	int length = atoi(token.value.c_str());
	printf("frame length: %d\n", length);

	// X-Timestampまで読み飛ばし
	do {
		reader.readNextToken();
	} while (token.value != "X-Timestamp");

	// デリミタの読み飛ばし
	do {
		reader.readNextToken();
	} while (token.isDelimitor);

	float timestamp = atof(token.value.c_str());
	printf("frame timestamp: %f\n", timestamp);

	reader.readNextToken(); // ¥r
	if (token.value[0] != '\r') {
		fprintf(stderr, "missed ¥r");
		return;
	}

	reader.readNextToken(); // ¥n
	if (token.value[0] != '\n') {
		fprintf(stderr, "missed ¥n");
		return;
	}

	reader.readNextToken(); // ¥r
	if (token.value[0] != '\r') {
		fprintf(stderr, "missed ¥r");
		return;
	}

	reader.readNextToken(); // ¥n
	if (token.value[0] != '\n') {
		fprintf(stderr, "missed ¥n");
		return;
	}

	uint8_t* buf = new uint8_t[length];
	reader.readBuffer(buf, length);

	cv::Mat rawData( 1, length, CV_8UC1, (void*)buf);
	cv::Mat decodedImage = cv::imdecode( rawData, 0);
	if (!decodedImage.empty()) {
		mCurImg = decodedImage;
	}

	delete[] buf;

	reader.readNextToken(); // ¥r
	if (token.value[0] != '\r') {
		fprintf(stderr, "missed ¥r");
		return;
	}

	reader.readNextToken(); // ¥n
	if (token.value[0] != '\n') {
		fprintf(stderr, "missed ¥n");
		return;
	}

	reader.readNextToken(); // boundary
	if (token.value != boundary) {
		fprintf(stderr, "missed boundary");
		return;
	}
}

void MJpegCapture::startDecoding(HttpReader& reader) {
	const Token& token = reader.token();

	// 1行目のHTTPヘッダだけ読み込む
	reader.readNextToken();
	if (token.isDelimitor) {
		fprintf(stderr, "invalid http header");
		return;
	}
	std::string httpVer = token.value;

	// デリミタの読み飛ばし
	do {
		reader.readNextToken();
	} while (token.isDelimitor);

	// ステータスコードの取得
	std::string statusCode = token.value;

	// デリミタの読み飛ばし
	do {
		reader.readNextToken();
	} while (token.isDelimitor);

	// ステータスメッセージの取得
	std::string statusMsg = token.value;

	if (statusCode != "200") {
		fprintf(stderr, "Invalid http status: %s %s\n", statusCode.c_str(), statusMsg.c_str());
		return;
	}

	// その他HTTPヘッダの読み飛ばし
	do {
		reader.readNextToken();
	} while (token.value != "boundary");

	reader.readNextToken(); // =
	reader.readNextToken(); // boundarydonotcross

	const std::string boundary = "--" + token.value;

	// バウンダリーまで読み飛ばし
	do {
		reader.readNextToken();
	} while (token.value != boundary);

	do {
		readFrame(reader, boundary);

		while (token.value != boundary) {
			// バウンダリー文字列で終わっていない場合は次のバウンダリー文字列まで読み飛ばす
			reader.readNextToken();
		}

	} while(true);
}

MJpegCapture::MJpegCapture(InputStream& stream) : mStream(stream) {
}

void MJpegCapture::start() {
	pthread_t pt;
	pthread_create(&pt, NULL, &decodeThread, this);
}

MJpegCapture& MJpegCapture::operator >> (cv::Mat& image) {
	image = mCurImg;
	return *this;
}

void* MJpegCapture::decodeThread(void* arg) {
	MJpegCapture* self = static_cast<MJpegCapture*>(arg);

	HttpReader reader(self->mStream);
	self->startDecoding(reader);

	return NULL;
}