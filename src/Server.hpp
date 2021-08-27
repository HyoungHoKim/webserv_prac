#ifndef SERVER_HPP
#define SERVER_HPP

#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>

#include <iostream>
#include <map>
#include <vector>

#include "Client.hpp"
#include "HTTPMessage.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "ResourceHost.hpp"

#define INVALID_SOCKET -1
#define QSIZE 1024

class Server
{
private:
	int serv_sock;
	int serv_port;
	struct sockaddr_in serv_adr;
	int kq;
	struct kevent event_list[QSIZE];

	std::map<int, Client*> clientMap;

	ResourceHost *resHost;

public:
	Server(int port, std::string diskpath);
	~Server();
	Server(const Server &_server);
	Server &operator=(const Server &_server);

	int getServ_sock(void) const;
	int get_kq(void) const;
	std::map<int, std::string> &get_clients(void);
	std::vector<struct kevent> &getChange_list(void);
	struct kevent *getEvent_list(void);

	bool init_Server(void);
	void stop_server(void);
	void add_kevent(uintptr_t ident, int16_t filter, uint16_t flags, uint32_t fflags, uintptr_t data, void *udata);
	void disconnected_client(Client *cl, bool mapErase = true);
	void accept_new_client(void);
	Client *getClient(int clnt_sock);
	void run(void);
	
	void readClient(Client *cl, int data_len);
	bool writeClient(Client *cl, int avail_bytes);

	void handleRequest(Client *cl, HTTPRequest *req);
	void handleGet(Client *cl, HTTPRequest *req);

	void sendStatusResponse(Client *cl, int status, std::string msg = "");
	void sendResponse(Client *cl, HTTPResponse *resp, bool disconnect);
};

bool exit_with_perror(const std::string &msg);

#endif 