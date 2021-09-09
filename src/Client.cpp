#include "Client.hpp"

Client::Client(int fd, sockaddr_in addr)
{
	this->sock_fd = fd;
	this->clnt_addr = addr;
	this->req = NULL;

}

Client::~Client()
{
	clearSendQueue();
	if (!req)
		delete this->req;
}

void Client::recvRequestData(char *buf)
{
	if (!req)
		this->req = new HTTPRequest(reinterpret_cast<byte*>(buf), strlen(buf));
	else
	{
		this->req->putBytes(reinterpret_cast<byte*>(buf), strlen(buf));
		//this->req->printData();
	}
}

void Client::addToSendQueue(SendQueueItem *item)
{
	sendQueue.push(item);
}

unsigned int Client::sendQueueSize()
{
	return (sendQueue.size());
}

SendQueueItem *Client::nextInSendQueue()
{
	if (sendQueue.empty())
		return (NULL);
	return (sendQueue.front());
}

void Client::dequeueFromSendQueue()
{
	SendQueueItem *item = nextInSendQueue();
	if (item != NULL)
	{
		sendQueue.pop();
		delete item;
	}
}

void Client::clearSendQueue()
{
	while (!sendQueue.empty())
	{
		delete sendQueue.front();
		sendQueue.pop();
	}
}