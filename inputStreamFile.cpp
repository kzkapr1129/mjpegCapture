#include "inputStreamFile.h"
#include <stdio.h>

InputStreamFile::InputStreamFile(const char* filename) : mFd(NULL) {
	mFd = fopen(filename, "rb");
}

InputStreamFile::~InputStreamFile() {
	close();
}

void InputStreamFile::close() {
	if (mFd) {
		fclose((FILE*)mFd);
		mFd = NULL;
	}
}

size_t InputStreamFile::read(uint8_t* buf, size_t size) {
	size_t n = fread(buf, sizeof(uint8_t), size, (FILE*)mFd);
	if (n <= 0) {
		return ERR_EOF;
	}
	return n;
}