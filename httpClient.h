#pragma once

#include "inputStream.h"
#include <string>

class Response : public InputStream {
public:
	Response(int sock);
	~Response();

	void close();
	size_t read(uint8_t* buf, size_t size);

private:
	int mSocket;
};

class HttpClient {
public:
	static Response* request(const std::string& ip, int port, const std::string& uri);
};