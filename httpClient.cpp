#include "httpClient.h"
#include <sys/fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>

Response::Response(int sock) : mSocket(sock) {
	/*
	struct timeval tv;
	tv.tv_sec = 60;
	tv.tv_usec = 0;
	setsockopt(mSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
	*/
}

Response::~Response() {
	close();
}

void Response::close() {
	if (0 <= mSocket) {
		::close(mSocket);
		mSocket = -1;
	}
}

size_t Response::read(uint8_t* buf, size_t size) {
	ssize_t ret;
	for (int retry = 0; retry < 3; retry++) {
		ret = ::read(mSocket, buf, size);
		if (ret < 0) {
			return 0;
		}

		if (0 < ret) {
			break;
		}

		// 読み込みバイト0のときはリトライ
	}
	return ret;
}

Response* HttpClient::request(const std::string& ip, int port, const std::string& uri) {

	int sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		return NULL;
	}

	sockaddr_in client_addr;
	memset(&client_addr, 0, sizeof(client_addr));
	client_addr.sin_family = PF_INET;
	client_addr.sin_addr.s_addr = inet_addr(ip.c_str());
	client_addr.sin_port = htons(port);

	if (connect(sock, (struct sockaddr *)&client_addr,  sizeof(client_addr)) < 0) {
		close(sock);
		return NULL;
	}

	std::string requestMessage = "GET " + uri + " HTTP/1.1\r\n\r\n";

	size_t len = write(sock, requestMessage.c_str(), requestMessage.length());
	if (len < 0) {
		close(sock);
		return NULL;
	}

	Response* res = new Response(sock);
	if (res == NULL) {
		close(sock);
		return NULL;
	}

	return res;
}