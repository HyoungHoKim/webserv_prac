#include "Server.hpp"

Server::Server(int port, std::string diskpath)
{
	this->serv_sock = INVALID_SOCKET;
	this->serv_port = port;
	this->kq = -1;

	std::cout << "Port: " << port << std::endl;
	std::cout << "Disk path: " << diskpath << std::endl;

	this->resHost = new ResourceHost(diskpath);
}

Server::Server(const Server &_server)
{
	*this = _server;
}

Server::~Server()
{
	delete resHost;
}

Server &Server::operator=(const Server &_server)
{
	if (this == &_server)
		return (*this);
	this->serv_sock = _server.serv_sock;
	this->serv_adr.sin_family = _server.serv_adr.sin_family;
	this->kq = _server.kq;
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

struct kevent *Server::getEvent_list(void)
{
	return (this->event_list);
}

bool Server::init_Server(void)
{
	if ((this->serv_sock = socket(PF_INET, SOCK_STREAM, 0)) == -1)
		return (exit_with_perror("socket() error!!!"));
	
	memset(&this->serv_adr, 0, sizeof(this->serv_adr));
	this->serv_adr.sin_family = AF_INET;
	this->serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	this->serv_adr.sin_port = htons(this->serv_port);

	fcntl(this->serv_sock, F_SETFL, O_NONBLOCK);

	if (bind(this->serv_sock, (struct sockaddr *)&(this->serv_adr), sizeof(this->serv_adr)) == -1)
		return (exit_with_perror("bind() error!!!"));

	if (listen(this->serv_sock, SOMAXCONN) != 0)
		return (exit_with_perror("listen() error!!!"));

	if ((this->kq = kqueue()) == -1)
		return (exit_with_perror("kqueue() error!!!"));

	add_kevent(this->serv_sock, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);

	std::cout << "Server ready. Listening on port " << serv_port << "..." << std::endl;
	return (true);
}

void Server::stop_server(void)
{
	if (this->serv_sock != INVALID_SOCKET)
	{
		std::map<int, Client*>::iterator iter;
		for (iter = this->clientMap.begin(); iter != this->clientMap.end(); iter++)
			disconnected_client(iter->second, false);
		this->clientMap.clear();
		add_kevent(this->serv_sock, EVFILT_READ, EV_DELETE, 0, 0, NULL);
		shutdown(this->serv_sock, SHUT_RDWR);
		close(serv_sock);
		this->serv_sock = INVALID_SOCKET;
	}

	if (this->kq != -1)
	{
		close(kq);
		kq = -1;
	}

	std::cout << "Server shutdown" << std::endl;
}

void Server::add_kevent(uintptr_t ident, int16_t filter, uint16_t flags, uint32_t fflags, uintptr_t data, void *udata)
{
	struct kevent temp_event;
	EV_SET(&temp_event, ident, filter, flags, fflags, data, udata);
	kevent(this->kq, &temp_event, 1, NULL, 0, NULL);
}

void Server::disconnected_client(Client *cl, bool mapErase)
{
	if (cl == NULL)
		return;

	std::cerr << "client socket error" << std::endl;
	std::cout << "client disconnected : " << cl->getClientIP() << std::endl;

	add_kevent(cl->getSocket(), EVFILT_READ, EV_DELETE, 0, 0, NULL);
	add_kevent(cl->getSocket(), EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
	
	close(cl->getSocket());

	if (mapErase)
		clientMap.erase(cl->getSocket());

	delete cl;
}

void Server::accept_new_client(void)
{
	int clnt_sock;
	sockaddr_in clnt_addr;
	socklen_t clnt_len = sizeof(clnt_addr);

	if ((clnt_sock = accept(this->serv_sock, (struct sockaddr *)&clnt_addr, &clnt_len)) == -1)
		exit_with_perror("accept() error!!!");
	std::cout << "accept new client : " << clnt_sock << std::endl;
	fcntl(clnt_sock, F_SETFL, O_NONBLOCK);

	Client *cl = new Client(clnt_sock, clnt_addr);

	this->add_kevent(clnt_sock, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
	this->add_kevent(clnt_sock, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
	
	clientMap.insert(std::pair<int, Client*>(clnt_sock, cl));

	std::cout << "Client : " << cl->getClientIP() << " connected!!!" << std::endl; 
}

Client *Server::getClient(int clnt_sock)
{
	std::map<int, Client*>::iterator iter = clientMap.find(clnt_sock);

	if (iter == clientMap.end())
		return (NULL);
	
	return (iter->second);
}

void Server::run(void)
{
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

			if (static_cast<int>(curr_event->ident) == this->serv_sock)
				accept_new_client();
			else
			{
				if (this->clientMap.find(curr_event->ident) == this->clientMap.end())
				{
					std::cout << "Could not find client" << std::endl;
					continue;
				}

				if (curr_event->flags & EV_EOF)
				{
					std::cout << "flags EOF" << std::endl;
					disconnected_client(clientMap[curr_event->ident]);
					continue;
				}

				if (curr_event->filter == EVFILT_READ)
				{
					readClient(clientMap[curr_event->ident], curr_event->data);

					add_kevent(curr_event->ident, EVFILT_READ, EV_DISABLE, 0, 0, NULL);
					add_kevent(curr_event->ident, EVFILT_WRITE, EV_ENABLE, 0, 0, NULL);
				}
				else if (curr_event->filter == EVFILT_WRITE)
				{
					if (!writeClient(clientMap[curr_event->ident], curr_event->data))
					{
						add_kevent(curr_event->ident, EVFILT_READ, EV_ENABLE, 0, 0, NULL);
						add_kevent(curr_event->ident, EVFILT_WRITE, EV_DISABLE, 0, 0, NULL);
					}
				}
			}
		}
	}
}

void Server::readClient(Client *cl, int data_len)
{
	if (cl == NULL)
		return ;
	
	if (data_len <= 0)
		data_len = 1400;
	
	HTTPRequest *req;
	char *pData = new char[data_len];
	bzero(pData, data_len);

	int flags = 0;
	ssize_t lenRecv = recv(cl->getSocket(), pData, data_len, flags);

	if (lenRecv == 0)
	{
		std::cout << "[" << cl->getSocket() << "] has opted to close the connection" << std::endl;
		disconnected_client(cl);
	}
	else if (lenRecv < 0)
		disconnected_client(cl);
	else
	{
		req = new HTTPRequest((byte*)pData, lenRecv);
		handleRequest(cl, req);
		delete req;
	}

	delete[] pData;
}

bool Server::writeClient(Client *cl, int avail_bytes)
{
	if (cl == NULL)
		return (false);
	
	std::cout << "avail_bytes: " << avail_bytes << std::endl;
 	int actual_sent = 0;
	int attempt_sent = 0;
	int remaining = 0;
	bool disconnect = false;
	byte *pData = NULL;

	if (avail_bytes > 1400)
		avail_bytes = 1400;
	else if (avail_bytes == 0)
		avail_bytes = 64;

	SendQueueItem *item = cl->nextInSendQueue();
	if (item == NULL)
		return (false);
	
	pData = item->getData();
	remaining = item->getSize() - item->getOffset();
	disconnect = item->getDisconnect();

	std::cout << pData << std::endl;

	if (avail_bytes >= remaining)
		attempt_sent = remaining;
	else 
		attempt_sent = avail_bytes;

	actual_sent = send(cl->getSocket(), pData + (item->getOffset()), attempt_sent, 0);
	if (actual_sent >= 0)
		item->setOffset(item->getOffset() + actual_sent);
	else
		disconnect = true;

	std::cout << "[" << cl->getClientIP() << "] was sent " << actual_sent << " bytes " << std::endl;

	if (item->getOffset() >= item->getSize())
		cl->dequeueFromSendQueue();
	
	if (disconnect)
	{
		disconnected_client(cl);
		return (false);
	}
	return (true);
}

void Server::handleRequest(Client *cl, HTTPRequest *req)
{
	if (!req->parse())
	{
		std::cout << "[" << cl->getClientIP() << "] There was an error processing thre request type: " 
			<< req->methodIntToStr(req->getMethod()) << std::endl;
		sendStatusResponse(cl, Status(BAD_REQUEST), req->getParseError());
		return;
	}

	std::cout << "[" << cl->getClientIP() << "] " << req->methodIntToStr(req->getMethod()) << " "
		<< req->getRequestUri() << std::endl;

	switch (req->getMethod())
	{
	case Method(GET):
		handleGet(cl, req);
		break;
	default:
		std::cout << "[" << cl->getClientIP() << "] Could not handle or determine request of type " <<
			req->methodIntToStr(req->getMethod()) << std::endl;
		sendStatusResponse(cl, Status(NOT_IMPLEMENTED));
		break;
	}
}

void Server::handleGet(Client *cl, HTTPRequest *req)
{
	std::cout << "GET Method processing" << std::endl;
	std::string uri = req->getRequestUri();
	Resource *r = resHost->getResource(uri);

	if (r != NULL)
	{
		std::cout << "[" << cl->getClientIP() << "]" << "Sending file: " << uri << std::endl;

		HTTPResponse *resp = new HTTPResponse();
		resp->setStatus(Status(OK));
		resp->addHeader("Content-Type", r->getMimeType());
		resp->addHeader("Content-Length", r->getSize());

		if (req->getMethod() == Method(GET))
			resp->setData(r->getData(), r->getSize());
		
		bool dc = false;

		std::string connection_val = req->getHeaderValue("Connection");
		if (connection_val.compare("close") == 0)
			dc = true;

		sendResponse(cl, resp, dc);
		delete resp;
		delete r;
	}
	else
	{
		std::cout << "[" << cl->getClientIP() << "] " << "File not found: " << uri << std::endl;
		sendStatusResponse(cl, Status(NOT_FOUND));
	}
}

void Server::sendStatusResponse(Client *cl, int status, std::string msg)
{
	HTTPResponse *resp = new HTTPResponse();
	resp->setStatus(Status(status));

	std::string body = resp->getReason();
	if (msg.length() > 0)
		body += ": " + msg;

	unsigned int slen = body.length();
	char *sdata = new char[slen];
	bzero(sdata, slen);
	strncpy(sdata, body.c_str(), slen);

	resp->addHeader("Content-Type", "text/plain");
	resp->addHeader("Content-Length", slen);
	resp->setData((byte*)sdata, slen);

	sendResponse(cl, resp, true);

	delete resp;
}

void Server::sendResponse(Client *cl, HTTPResponse *resp, bool disconnect)
{
	resp->addHeader("Server", "httpserver/1.0");

	std::string tstr;
	char tbuf[36] = {0};
	time_t rawtime;
	struct tm *ptm;
	time(&rawtime);
	ptm = gmtime(&rawtime);
	strftime(tbuf, 36, "%a, %d %b %Y %H:%M:%S GMT", ptm);
	tstr = tbuf;
	resp->addHeader("Date", tstr);

	if (disconnect)
		resp->addHeader("Connection", "close");
	
	byte *pData = resp->create();

	cl->addToSendQueue(new SendQueueItem(pData, resp->size(), disconnect));
}

bool exit_with_perror(const std::string &msg)
{
	std::cerr << msg << std::endl;
	std::cerr << strerror(errno) << std::endl;
	return (false);
}