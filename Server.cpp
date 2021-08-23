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
	for (int i = 0; i < QSIZE; i++)
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

struct kevent *Server::getEvent_list(void)
{
	return (this->event_list);
}

bool Server::init_Server(int port)
{
	if ((this->serv_sock = socket(PF_INET, SOCK_STREAM, 0)) == -1)
		return (exit_with_perror("socket() error!!!"));
	
	memset(&this->serv_adr, 0, sizeof(this->serv_adr));
	this->serv_adr.sin_family = AF_INET;
	this->serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	this->serv_adr.sin_port = htons(port);

	fcntl(this->serv_sock, F_SETFL, O_NONBLOCK);

	if (::bind(this->serv_sock, (struct sockaddr *)&(this->serv_adr), sizeof(this->serv_adr)) == -1)
		return (exit_with_perror("bind() error!!!"));

	if (listen(this->serv_sock, 5) == -1)
		return (exit_with_perror("listen() error!!!"));

	if ((this->kq = kqueue()) == -1)
		return (exit_with_perror("kqueue() error!!!"));

	add_kevent(this->serv_sock, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
	
	return (true);
}

void Server::stop_server(void)
{
	if (this->serv_sock != INVALID_SOCKET)
	{
		map<int, string>::iterator iter;
		for (iter = this->clients.begin(); iter != this->clients.end(); iter++)
			disconnected_client(iter->first);
		this->clients.clear();
		add_kevent(this->serv_sock, EVFILT_READ, EV_DELETE, 0, 0, NULL);
		shutdown(this->serv_sock, SHUT_RDWR);
		this->serv_sock = INVALID_SOCKET;
	}

	if (this->kq != -1)
	{
		close(kq);
		kq = -1;
	}

	cout << "Server shutdown" << endl;
}

void Server::add_kevent(uintptr_t ident, int16_t filter, uint16_t flags, uint32_t fflags, uintptr_t data, void *udata)
{
	struct kevent temp_event;
	EV_SET(&temp_event, ident, filter, flags, fflags, data, udata);
	kevent(this->kq, &temp_event, 1, NULL, 0, NULL);
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
	cout << "Client : " << clnt_sock << " connected!!!" << endl; 
}

void Server::run(void)
{
	cout << "run!" << endl;
	int new_events;
	struct kevent *curr_event;
	while(1)
	{
		new_events = kevent(this->kq, NULL, 0, this->event_list, QSIZE, NULL);
		
		if (new_events <= 0)
			continue;
			
		for (int i = 0; i < new_events; ++i)
		{
			curr_event = &(this->event_list[i]);

			if (curr_event->ident == this->serv_sock)
				accept_new_client();
			else
			{
				if (this->clients.find(curr_event->ident) == this->clients.end())
				{
					cout << "Could not find client" << endl;
					continue;
				}

				if (curr_event->flags & EV_EOF)
				{
					disconnected_client(curr_event->ident);
					continue;
				}

				if (curr_event->filter == EVFILT_READ)
				{
					char req_line[1024];
					char method[10];
					char ct[15];
					char file_name[30];
			
					recv(curr_event->ident, req_line, 1023, 0);	
					cout << req_line << endl;

					if (strstr(req_line, "HTTP/") == NULL)
					{
						cerr << "Not HTTP protocol" << endl;
						disconnected_client(curr_event->ident);
						return ;
					}

					strcpy(method, strtok(req_line, " /"));
					cout << "method : " << method << endl;
					strcpy(file_name, strtok(NULL, " /"));
					cout << "filename : " << file_name << endl;
					strcpy(ct, content_type(file_name));
					cout << "content_type : " << ct << endl;
					if (strcmp(method, "GET") != 0)
					{
						cerr << "Method is not GET!!!" << endl;
						disconnected_client(curr_event->ident);
						return ;
					}
					send_data(curr_event->ident, ct, file_name);
					add_kevent(curr_event->ident, EVFILT_READ, EV_DISABLE, 0, 0, NULL);
					add_kevent(curr_event->ident, EVFILT_WRITE, EV_ENABLE, 0, 0, NULL);
				}
			}
		}
	}
}

void Server::readClient(int clnt_sock, int data_len)
{
	if (clnt_sock == -1)
		return ;
	if (data_len <= 0)
		data_len = 1400;
	
	HTTPRequest *req;
	char *pData = new char[data_len];
	bzero(pData, data_len);

	int flags = 0;
	ssize_t lenRecv = recv(clnt_sock, pData, data_len, flags);

	if (lenRecv == 0)
	{
		std::cout << "[" << clnt_sock << "] has opted to close the connection" << std::endl;
		disconnected_client(clnt_sock);
	}
	else if (lenRecv < 0)
		disconnected_client(clnt_sock);
	else
	{
		req = new HTTPRequest((byte*)pData, lenRecv);
		handleRequest(clnt_sock, req);
		delete req;
	}

	delete[] pData;
}

bool Server::writeClient(int clnt_sock, int avail_bytes)
{
	if (clnt_sock == -1)
		return (false);
	
	int actual_sent = 0;
	int attemp_sent = 0;
	int remaining = 0;
	bool disconnect = false;
	byte *pData = NULL;

	if (avail_bytes > 1400)
		avail_bytes = 1400;
	else if (avail_bytes == 0)
		avail_bytes = 64;
}

bool exit_with_perror(const string &msg)
{
	cerr << msg << endl;
	cerr << strerror(errno) << endl;
	return (false);
}