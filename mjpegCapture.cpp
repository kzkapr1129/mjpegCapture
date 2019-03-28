#include "mjpegCapture.h"
#include "httpReader.h"
#include "httpClient.h"
#include <opencv2/opencv.hpp>
#include <pthread.h>
#include <sys/time.h>

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

	// X-Timestampまで読み飛ばし
	do {
		reader.readNextToken();
	} while (token.value != "X-Timestamp");

	// デリミタの読み飛ばし
	do {
		reader.readNextToken();
	} while (token.isDelimitor);

	float timestamp = atof(token.value.c_str());

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

	if (mTmpBuffer == NULL || mTmpBufferLen < length) {
		delete[] mTmpBuffer;
		size_t allocSize = length * 1.2;
		mTmpBuffer = new uint8_t[allocSize];
		if (mTmpBuffer) {
			mTmpBufferLen = allocSize;
		}
	}

	reader.readBuffer(mTmpBuffer, length);

	cv::Mat rawData( 1, length, CV_8UC1, (void*)mTmpBuffer);
	cv::Mat decodedImage = cv::imdecode( rawData, 1);
	if (!decodedImage.empty()) {
		mCurImg = decodedImage;
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
		if (mStopRequest) {
			break;
		}

		readFrame(reader, boundary);

		while (token.value != boundary) {
			// バウンダリー文字列で終わっていない場合は次のバウンダリー文字列まで読み飛ばす
			reader.readNextToken();
		}

	} while(true);

	printf("exit decoding\n");
}

MJpegCapture::MJpegCapture() : mTmpBuffer(NULL), mTmpBufferLen(0), mStopRequest(false) {
	pthread_cond_init(&mCond, NULL);
	pthread_mutex_init(&mMutex, NULL);
}

MJpegCapture::~MJpegCapture() {
	stop();

	pthread_mutex_destroy(&mMutex);
	pthread_cond_destroy(&mCond);
}

int MJpegCapture::start(const char* ip, int port, const char* uri) {
	mStream = HttpClient::request(ip, port, uri);
	if (mStream == NULL) {
		return -1;
	}

	mStopRequest = false;
	int ret = pthread_create(&mThread, NULL, &decodeThread, this);
	if (ret) {
		return -1;
	}

	pthread_mutex_lock(&mMutex);

	timespec timeout;
	timeval now;
	gettimeofday(&now, NULL);
	timeout.tv_sec = now.tv_sec + 5;
	timeout.tv_nsec = now.tv_usec * 1000;

	ret = pthread_cond_timedwait(&mCond, &mMutex, &timeout);
	if (ret == ETIMEDOUT) {
		// IGNORE
	}
	pthread_mutex_unlock(&mMutex);

	return 0;
}

void MJpegCapture::stop() {
	if (mStopRequest) {
		return;
	}

	mStopRequest = true;
	pthread_join(mThread, NULL);

	delete mStream;
	mStream = NULL;
}

bool MJpegCapture::isRunning() const {
	return mIsRunning;
}

MJpegCapture& MJpegCapture::operator >> (cv::Mat& image) {
	image = mCurImg;
	return *this;
}

void* MJpegCapture::decodeThread(void* arg) {
	MJpegCapture* self = static_cast<MJpegCapture*>(arg);
	self->mIsRunning = true;

	HttpReader reader(*self->mStream);

	pthread_mutex_lock(&self->mMutex);
	pthread_cond_broadcast(&self->mCond);
	pthread_mutex_unlock(&self->mMutex);

	try {
		self->startDecoding(reader);
	} catch (...) {

	}

	self->mIsRunning = false;
	return NULL;
}