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
#include <fstream>

#include "Client.hpp"
#include "HTTPMessage.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "ResourceHost.hpp"
#include "Config.hpp"

#define INVALID_SOCKET -1
#define QSIZE 1024 // 

class Server
{
private:
	ServerConfig serv_config;
	int serv_sock;
	int serv_port;
	struct sockaddr_in serv_adr;
	struct timespec kqTimeout;
	int kq;
	struct kevent event_list[QSIZE];

	std::map<int, Client*> clientMap;

	ResourceHost *resHost;

public:
	Server(ServerConfig &_sc);
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
	
	bool readClient(Client *cl, int data_len);
	bool writeClient(Client *cl, int avail_bytes);

	bool check_allowed_methods(HTTPRequest *req, int& idx);
	void handleRequest(Client *cl, HTTPRequest *req);
	void handleGet(Client *cl, HTTPRequest *req, size_t maxBody = 0);
	void handlePost(Client *cl, HTTPRequest *req, size_t maxBody = 0);
	void handlePut(Client *cl, HTTPRequest *req, size_t maxBody = 0);
	void handleDelete(Client *cl, HTTPRequest *Req, size_t maxBody = 0);

	void sendStatusResponse(Client *cl, int status, std::string msg = "");
	void sendResponse(Client *cl, HTTPResponse *resp, bool disconnect, size_t maxBody = 0);
};

bool exit_with_perror(const std::string &msg);

#endif 