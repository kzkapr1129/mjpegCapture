#pragma once

#include "inputStream.h"
#include <string>

struct EofException {
};

struct Token {
	std::string value;
	bool isDelimitor;
};

class HttpReader {
public:
	HttpReader(InputStream& stream);
	~HttpReader();

	void readNextToken();
	int readBuffer(uint8_t* buf, int size);

	const Token& token() const { return mToken; }

private:
	static const int BUF_SIZE = 1024;

	Token mToken;

	InputStream& mStream;
	uint8_t mTmpBuffer[BUF_SIZE];
	int mOffset;
	int mBufLen;
};