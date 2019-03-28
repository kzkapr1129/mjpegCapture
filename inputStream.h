#pragma once

#include <stddef.h>
#include <stdint.h>

#define ERR_EOF 0

class InputStream {
public:
	virtual ~InputStream() {}
	virtual void close() = 0;
	virtual size_t read(uint8_t* buf, size_t size) = 0;
};