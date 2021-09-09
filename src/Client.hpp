#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <arpa/inet.h>
#include <queue>

#include "HTTPRequest.hpp"
#include "SendQueueItem.hpp"

typedef unsigned char byte;

class Client
{
private:
	int sock_fd;
	sockaddr_in clnt_addr;
	HTTPRequest* req; 

	std::queue<SendQueueItem*> sendQueue;

public:
	Client(int fd, sockaddr_in addr);
	~Client();

	sockaddr_in getClientAddr()
	{
		return (clnt_addr);
	}

	int getSocket()
	{
		return (sock_fd);
	}

	char *getClientIP()
	{
		return (inet_ntoa(clnt_addr.sin_addr));
	}

	HTTPRequest* getRequset()
	{
		return (this->req);
	}

	void recvRequestData(char *buf);

	void addToSendQueue(SendQueueItem *item);
	unsigned int sendQueueSize();
	SendQueueItem *nextInSendQueue();
	void dequeueFromSendQueue();
	void clearSendQueue();
};

#endif