#include "httpReader.h"
#include <algorithm>

static bool isDelimitor(uint8_t c) {
	if (c == ' ') {
		return true;
	} else if (c == ',') {
		return true;
	} else if (c == ':') {
		return true;
	} else if (c == ';') {
		return true;
	} else if (c == '=') {
		return true;
	} else if (c == '\r') {
		return true;
	} else if (c == '\n') {
		return true;
	}

	return false;
} 

HttpReader::HttpReader(InputStream& stream) : mStream(stream), mOffset(-1), mBufLen(0) {
}

HttpReader::~HttpReader() {
}

void HttpReader::readNextToken() {
	std::string ret;
	bool foundDelimitor = false;
	while (!foundDelimitor) {
		if (mOffset < 0 || mBufLen <= mOffset) {
			size_t n = mStream.read(mTmpBuffer, BUF_SIZE);
			if (n <= 0) {
				mToken.value = ret;
				mToken.isDelimitor = false;
				throw EofException();
			}

			mBufLen = n;
			mOffset = 0;
		}

		int startOffset = mOffset;
		for (; mOffset < mBufLen; mOffset++) {
			uint8_t c = mTmpBuffer[mOffset];
			if (isDelimitor(c)) {
				if (mOffset - startOffset == 0 && ret.length() == 0) {
					ret = c;
					mOffset++;
					mToken.value = ret;
					mToken.isDelimitor = true;
					return;
				}

				ret += std::string((char*)&mTmpBuffer[startOffset], mOffset - startOffset);
				foundDelimitor = true;
				break;
			}
		}

		if (!foundDelimitor) {
			ret += std::string((char*)&mTmpBuffer[startOffset], mOffset - startOffset);
		}
	}

	mToken.value = ret;
	mToken.isDelimitor = false;
}

int HttpReader::readBuffer(uint8_t* buf, int size) {

	int len = 0;
	while (len < size) {
		if (mOffset < 0 || mBufLen <= mOffset) {
			size_t n = mStream.read(mTmpBuffer, BUF_SIZE);
			if (n <= 0) {
				throw EofException();
			}

			mBufLen = n;
			mOffset = 0;
		}

		int cpySize = std::min(mBufLen - mOffset, size - len);
		memcpy(&buf[len], &mTmpBuffer[mOffset], cpySize);

		mOffset += cpySize;
		len += cpySize;
	}

	return 0;
}