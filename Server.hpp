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

#include <iostream>
#include <map>
#include <vector>

#include "HTTPMessage.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"

#define INVALID_SOCKET -1
#define QSIZE 1024

using namespace std;

class Server
{
private:
	int serv_sock;
	struct sockaddr_in serv_adr;
	int kq;
	map<int, string> clients;
	struct kevent event_list[QSIZE];

public:
	Server() { };
	~Server() { };
	Server(const Server &_server);
	Server &operator=(const Server &_server);

	int getServ_sock(void) const;
	int get_kq(void) const;
	map<int, string> &get_clients(void);
	vector<struct kevent> &getChange_list(void);
	struct kevent *getEvent_list(void);

	bool init_Server(int port);
	void stop_server(void);
	void add_kevent(uintptr_t ident, int16_t filter, uint16_t flags, uint32_t fflags, uintptr_t data, void *udata);
	void disconnected_client(int clnt_fd);
	void accept_new_client(void);
	void run(void);
	
	void readClient(int clnt_sock, int data_len);
	void writeClient(int clnt_sock, int avail_bytes);

	void handleRequest(int clnt_sock, HTTPRequest *req);
	void handleGet(int clnt_sock, HTTPRequest *req, ResourceHost *resHost);
	void handleOptions(int clnt_sock, HTTPRequest *req);
	void handleTrace(int clnt_sock, HTTPRequest *req);

	void sendStatusResponce(int clnt_sock, int status, std::string msg = "");
	void sendResponse(int clnt_sock, HTTPResponse *resp, bool disconnect);
};

void send_data(int clnt_sock, char *ct, char *file_name);
bool exit_with_perror(const string &msg);
char *content_type(char *file);
void send_error(int clnt_sock);

#endif 