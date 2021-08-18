#include "Server.hpp"

Server::Server(const Server &_server)
{
	*this = _server;
}

Server &Server::operator=(const Server &_server)
{
	if (this == &_server)
		return (*this);
	this->serv_sock = _server.serv_sock;
	this->serv_adr.sin_family = _server.serv_adr.sin_family;
	this->kq = _server.kq;
	this->clients = _server.clients;
	this->change_list = _server.change_list;
	for (int i = 0; i < 8; i++)
		this->event_list[i] = _server.event_list[i];
	return (*this);
}

int Server::getServ_sock(void) const
{
	return (this->serv_sock);
}

int Server::get_kq(void) const
{
	return (this->kq);
}

map<int, string> &Server::get_clients(void)
{
	return (this->clients);
}

vector<struct kevent> &Server::getChange_list(void)
{
	return (this->change_list);
}

struct kevent *Server::getEvent_list(void)
{
	return (this->event_list);
}

void Server::init_Server()
{
	if ((this->serv_sock = socket(PF_INET, SOCK_STREAM, 0)) == -1)
		exit_with_perror("socket() error!!!");
	
	memset(&this->serv_adr, 0, sizeof(this->serv_adr));
	this->serv_adr.sin_family = AF_INET;
	this->serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	this->serv_adr.sin_port = htons(8080);

	if (::bind(this->serv_sock, (struct sockaddr *)&(this->serv_adr), sizeof(this->serv_adr)) == -1)
		exit_with_perror("bind() error!!!");

	if (listen(this->serv_sock, 5) == -1)
		exit_with_perror("listen() error!!!");

	fcntl(this->serv_sock, F_SETFL, O_NONBLOCK);

	if ((this->kq = kqueue()) == -1)
		exit_with_perror("kqueue() error!!!");

	this->add_kevent(this->serv_sock, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
}

void Server::add_kevent(uintptr_t ident, int16_t filter, uint16_t flags, uint32_t fflags, uintptr_t data, void *udata)
{
	struct kevent temp_event;
	EV_SET(&temp_event, ident, filter, flags, fflags, data, udata);
	this->change_list.push_back(temp_event);
}

void Server::disconnected_client(int clnt_fd)
{
	cerr << "client socket error" << endl;
	cout << "client disconnected : " << clnt_fd << endl;
	close(clnt_fd);
	this->clients.erase(clnt_fd);
}

void Server::accept_new_client(void)
{
	int clnt_sock;
	if ((clnt_sock = accept(this->serv_sock, NULL, NULL)) == -1)
		exit_with_perror("accept() error!!!");
	cout << "accept new client : " << clnt_sock << endl;
	fcntl(clnt_sock, F_SETFL, O_NONBLOCK);

	this->add_kevent(clnt_sock, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
	this->add_kevent(clnt_sock, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
	clients[clnt_sock] = "";
}

void exit_with_perror(const string &msg)
{
	cerr << msg << endl;
	cerr << strerror(errno) << endl;
	exit(EXIT_FAILURE);
}