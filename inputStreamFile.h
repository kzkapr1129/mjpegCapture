#pragma once

#include "inputStream.h"

class InputStreamFile : public InputStream {
public:
	InputStreamFile(const char* filename);
	~InputStreamFile();

	void close();
	size_t read(uint8_t* buf, size_t size);

private:
	void* mFd;
};