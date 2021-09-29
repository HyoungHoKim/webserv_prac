#include "Server.hpp"

Server::Server(ServerConfig &_sc)
{
	this->serv_config = _sc;
	this->serv_sock = INVALID_SOCKET;
	std::string _port = this->serv_config.getListen();
	this->serv_port = ft::stoi(_port);
	this->kqTimeout.tv_sec = 2;
	this->kqTimeout.tv_nsec = 0;
	this->kq = -1;
	this->resHost = NULL;
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
	int optvalue = 1;
	setsockopt(this->serv_sock, SOL_SOCKET, SO_REUSEADDR, &optvalue, sizeof(optvalue));

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
	cl->deleteRequest();

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

	this->add_kevent(clnt_sock, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
	this->add_kevent(clnt_sock, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
	
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
		//new_events = kevent(this->kq, NULL, 0, this->event_list, QSIZE, &(this->kqTimeout));
		new_events = kevent(this->kq, NULL, 0, this->event_list, QSIZE, 0);
		
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
					continue;
				
				if (curr_event->flags & EV_EOF)
				{
					std::cout << "flags EOF" << std::endl;
					disconnected_client(clientMap[curr_event->ident]);
					continue;
				}

				if (curr_event->filter == EVFILT_READ)
				{
					if (!readClient(clientMap[curr_event->ident], curr_event->data))
						disconnected_client(clientMap[curr_event->ident]);
					
					add_kevent(curr_event->ident, EVFILT_READ, EV_DISABLE, 0, 0, NULL);
					add_kevent(curr_event->ident, EVFILT_WRITE, EV_ENABLE, 0, 0, NULL);
				}
				if (curr_event->filter == EVFILT_WRITE)
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

bool Server::readClient(Client *cl, int data_len)
{
	if (cl == NULL)
		return (false);
	
	if (data_len <= 0)
		data_len = 1400;
	
	char *pData = new char[data_len + 1];
	bzero(pData, data_len + 1);

	ssize_t lenRecv = recv(cl->getSocket(), pData, data_len, 0);
	cl->recvRequestData(pData);
	delete[] pData;
	int status = cl->getRequset()->parse();

	if (lenRecv <= 0)
		return (false);

	if (status == Status(BAD_REQUEST))
	{
		std::cout << "[" << cl->getClientIP() << "] There was an error processing there request type: " 
			<< cl->getRequset()->methodIntToStr(cl->getRequset()->getMethod()) << std::endl;
		sendStatusResponse(cl, Status(BAD_REQUEST), cl->getRequset()->getParseError());
		cl->deleteRequest();
		return (true);
	}
	else if (status == Parsing(COMPLETE))
	{
		handleRequest(cl, cl->getRequset());
		cl->getRequset()->printData();
		cl->deleteRequest();
		return (true);
	}
	return (true);
}

bool Server::writeClient(Client *cl, int avail_bytes)
{
	//std::cout << "writeClient" << std::endl;
	if (cl == NULL)
		return (false);
	
 	int actual_sent = 0;
	int attempt_sent = 0;
	int remaining = 0;
	bool disconnect = false;
	byte *pData = NULL;
 
	if (avail_bytes > 1300)
		avail_bytes = 1300;
	else if (avail_bytes == 0)
		avail_bytes = 64;

	SendQueueItem *item = cl->nextInSendQueue();
	if (item == NULL)
		return (false);
	pData = item->getData();
	remaining = item->getSize() - item->getOffset();
	disconnect = item->getDisconnect();

	if (avail_bytes >= remaining)
		attempt_sent = remaining;
	else 
		attempt_sent = avail_bytes;

	actual_sent = send(cl->getSocket(), pData + (item->getOffset()), attempt_sent, 0);
	if (actual_sent >= 0)
		item->setOffset(item->getOffset() + actual_sent);

	if (item->getOffset() >= item->getSize())
		cl->dequeueFromSendQueue();
	
	if (disconnect || actual_sent <= 0)
		disconnected_client(cl);
	
	return (true);
}

// 여기 수정
bool Server::check_allowed_methods(HTTPRequest *req, int& idx)
{
	std::vector<ServerConfig> dir_config = this->serv_config.getLocations();
	for (idx = 0; idx < static_cast<int>(dir_config.size()); idx++)
	{
		if (req->getConfig_dir() == dir_config[idx].getUri())
			break ;
	}
	if (static_cast<size_t>(idx) == dir_config.size())
	{
		idx = 0;
		req->setRequestUri(req->getConfig_dir() + req->getRequestUri());
	}
	if (this->resHost)
		delete this->resHost;
	std::vector<std::string> config_index = this->serv_config.getLocations()[idx].getIndex();
	this->resHost = new ResourceHost(this->serv_config.getLocations()[idx].getRoot(),
		this->serv_config.getLocations()[idx].getAutoindex(), config_index);

	std::vector<std::string> allowed_methods = this->serv_config.getLocations()[idx].getMethod();
	for (size_t i = 0; i < allowed_methods.size(); i++)
	{
		if (req->methodIntToStr(req->getMethod()) == allowed_methods[i])
			return (true);
	}
	return (false);
}

void Server::handleRequest(Client *cl, HTTPRequest *req)
{
	int idx = 0;
	
	if (!check_allowed_methods(req, idx))
	{
		std::cout << "[" << cl->getClientIP() << "] Could not handle or determine request of type " <<
			req->methodIntToStr(req->getMethod()) << std::endl;
		sendStatusResponse(cl, Status(METHOD_NOT_ALLOW));
		return;
	}

	size_t max_body = this->serv_config.getLocations()[idx].getClientMaxBodySize();

	if (idx != 0 
		&& this->serv_config.getLocations()[idx].getStatusCode() != 0
		&& this->serv_config.getLocations()[idx].getRedir() != "")
	{
		if (req->getMethod() == Method(GET))
		{
			int redir_code = this->serv_config.getLocations()[idx].getStatusCode();
			if (redir_code == Status(MOVED_PERMANENTLY) || redir_code == Status(FOUND))
			{
				HTTPResponse *resp = new HTTPResponse();
				resp->setStatus(Status(redir_code));
				resp->addHeader("Location", this->serv_config.getLocations()[idx].getRedir());
				sendResponse(cl, resp, false);
				delete resp;
			}
			else
				sendStatusResponse(cl, Status(BAD_REQUEST));
		}
		else
		{
			std::cout << "[" << cl->getClientIP() << "] Could not handle or determine request of type " <<
				req->methodIntToStr(req->getMethod()) << std::endl;
			sendStatusResponse(cl, Status(METHOD_NOT_ALLOW));
		}
		return ;
	}

	switch (req->getMethod())
	{
	case Method(HEAD):
	case Method(GET):
		handleGet(cl, req, max_body);
		break;
	case Method(POST):
		handlePost(cl, req, max_body);
		break;
	case Method(PUT):
		handlePut(cl, req, max_body);
		break;
	case Method(DELETE):
		handleDelete(cl, req, max_body);
		break;
	}
}

void Server::handleGet(Client *cl, HTTPRequest *req, size_t maxBody)
{
	std::cout << "GET or HEAD" << " Method processing" << std::endl;
	std::string uri = req->getRequestUri();
	Resource *r = resHost->getResource(uri);

	if (r != NULL)
	{
		HTTPResponse *resp = new HTTPResponse();

		if (r->isDirectory())
			resp->setStatus(Status(NOT_FOUND));
		else
			resp->setStatus(Status(OK));
		resp->addHeader("Content-Type", r->getMimeType());
		resp->addHeader("Content-Length", r->getSize());

		if (req->getMethod() == Method(GET))
			resp->setData(r->getData(), r->getSize());
		
		bool dc = false;
		std::string connection_val = req->getHeaderValue("Connection");
		if (connection_val.compare("close") == 0)
			dc = true;

		sendResponse(cl, resp, dc, maxBody);
		delete resp;
		delete r;
	}
	else
	{
		std::cout << "[" << cl->getClientIP() << "] " << "File not found: " << uri << std::endl;
		sendStatusResponse(cl, Status(NOT_FOUND));
	}
}

void Server::handlePost(Client *cl, HTTPRequest *req, size_t maxBody)
{
	std::cout << "POST Method processing" << std::endl;
	std::string uri = req->getRequestUri();
	Resource *r = resHost->getResource(uri);

	// 파일이 존재하지 않을 시
	if (!r)
	{
		std::string path = resHost->getBaseDiskPath() + uri;
		// 생성 후 데이터 입력
		std::ofstream fout(path);
		fout << req->getData();

		HTTPResponse *resp = new HTTPResponse();
		resp->setStatus(Status(CREATE));
		resp->addHeader("Location",uri);
		
		bool dc = false;
		std::string connection_val = req->getHeaderValue("Connection");
		if (connection_val.compare("close") == 0)
			dc = true;

		sendResponse(cl, resp, dc, maxBody);
		delete resp;
		delete r;
		fout.close();
	}
	else
	{
		std::ofstream fout(r->getLocation(), std::ios::out | std::ios::app);
		fout << req->getData();
		delete r;
		r = NULL;
		r = resHost->getResource(uri);
		HTTPResponse *resp = new HTTPResponse();
		resp->setStatus(Status(OK));
		resp->addHeader("Content-Type", r->getMimeType());
		resp->addHeader("Content-Length", r->getSize());
		resp->setData(r->getData(), r->getSize());
		
		bool dc = false;
		std::string connection_val = req->getHeaderValue("Connection");
		if (connection_val.compare("close") == 0)
			dc = true;

		sendResponse(cl, resp, dc, maxBody);
		delete resp;
		delete r;
		fout.close();
	}
}

void Server::handlePut(Client *cl, HTTPRequest *req, size_t maxBody)
{
	std::cout << "PUT Method processing" << std::endl;
	std::string uri = req->getRequestUri();
	Resource *r = resHost->getResource(uri);
	int status = Status(NO_CONTENT);
	std::string path;

	// 파일이 존재하지 않을 시
	if (!r)
	{
		path = resHost->getBaseDiskPath() + uri;
		status = Status(CREATE);
	}
	else
		path = r->getLocation();
	std::ofstream fout(path);
	fout << req->getData();

	HTTPResponse *resp = new HTTPResponse();
	resp->setStatus(status);
	resp->addHeader("Location", path);
	if (!r)
		resp->addHeader("Content-Length", "0");
	
	bool dc = false;
	std::string connection_val = req->getHeaderValue("Connection");
	if (connection_val.compare("close") == 0)
		dc = true;
	
	sendResponse(cl, resp, dc, maxBody);
	if (resp->getData())
		delete resp->getData();
	delete resp;
	delete r;
	fout.close();
}

void Server::handleDelete(Client *cl, HTTPRequest *req, size_t maxBody)
{
	std::cout << "DELETE Method processing" << std::endl;
	std::string uri = req->getRequestUri();
	Resource *r = resHost->getResource(uri);

	if (!r)
	{
		std::cout << "[" << cl->getClientIP() << "] " << "File not found: " << uri << std::endl;
		sendStatusResponse(cl, Status(NOT_FOUND));
	}
	else
	{
		if (std::remove(r->getLocation().c_str()) != 0)
			exit_with_perror("Cannot delete file!!!");
		
		char delBody[] = 
		"<!DOCTYPE html>\n\
         <html>\n\
         <body>\n\
            <h1>File deleted</h1>\n\
    	 </body>\n\
    	 </html>";

		HTTPResponse *resp = new HTTPResponse();
		resp->setStatus(Status(OK));
		resp->addHeader("Content-Type", "text/html");
		resp->addHeader("Content-Length", strlen(delBody));
		resp->setData(reinterpret_cast<byte*>(delBody), strlen(delBody));

		bool dc = false;

		std::string connection_val = req->getHeaderValue("Connection");
		if (connection_val.compare("close") == 0)
			dc = true;

		sendResponse(cl, resp, dc, maxBody);
		delete resp;
		delete r;
	}
}

void Server::sendStatusResponse(Client *cl, int status, std::string msg)
{
	HTTPResponse *resp = new HTTPResponse();
	resp->setStatus(Status(status));
	
	std::string body = "";
	if (status == Status(NOT_FOUND))
	{
		std::string error_path = this->serv_config.getError();
		std::ifstream fin(error_path);
		while (!fin.eof())
		{
			std::string str;
			getline(fin, str);
			body += str;
		}
		fin.close();
	}
	else
	{
		body = resp->getReason();
		if (msg.length() > 0)
			body += ": " + msg;
	}

	unsigned int slen = body.length();
	char *sdata = new char[slen];
	bzero(sdata, slen);
	strncpy(sdata, body.c_str(), slen);

	resp->addHeader("Content-Type", "text/plain");
	resp->addHeader("Content-Length", slen);

	if (cl->getRequset()->getMethod() != Method(HEAD))
		resp->setData((byte*)sdata, slen);
	
	sendResponse(cl, resp, false);
	if (sdata)
		delete[] sdata;
	delete resp;
}

void Server::sendResponse(Client *cl, HTTPResponse *resp, bool disconnect, size_t maxBody)
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
	if (maxBody < resp->getDataLength() && maxBody != 0)
		sendStatusResponse(cl, Status(REQUEST_ENTITY_TOO_LARGE));
	else
		cl->addToSendQueue(new SendQueueItem(pData, resp->size(), disconnect));
}


bool exit_with_perror(const std::string &msg)
{
	std::cerr << msg << std::endl;
	std::cerr << strerror(errno) << std::endl;
	return (false);
}