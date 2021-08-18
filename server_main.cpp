#include "Server.hpp"

#define SMALL_BUF 1024

char *content_type(char *file);
void send_data(int clnt_sock, char *ct, char *file_name);
void send_error(int clnt_sock);

int main()
{
	Server server;

	server.init_Server();

	int new_events;
	struct kevent *curr_event;
	while(1)
	{
		new_events = kevent(server.get_kq(), &(server.getChange_list()[0]), server.getChange_list().size(),
			server.getEvent_list(), 8, NULL);
		if (new_events == -1)
			exit_with_perror("kevent() error!!!");
		
		server.getChange_list().clear();

		for (int i = 0; i < new_events; ++i)
		{
			curr_event = &(server.getEvent_list()[i]);

			if (curr_event->flags & EV_ERROR)
			{
				if (curr_event->ident == server.getServ_sock())
					exit_with_perror("server socket error!!!");
				else
					server.disconnected_client(curr_event->ident);
			}
			else if (curr_event->filter == EVFILT_READ)
			{
				if (curr_event->ident == server.getServ_sock())
					server.accept_new_client();
				else if (server.get_clients().find(curr_event->ident) != server.get_clients().end())
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
						server.disconnected_client(curr_event->ident);
						return (0);
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
						server.disconnected_client(curr_event->ident);
						return (0);
					}
					send_data(curr_event->ident, ct, file_name);
				}
			}
		}
	}
}

char *content_type(char *file)
{
	char extension[SMALL_BUF];
	char file_name[SMALL_BUF];
	strcpy(file_name, file);
	strtok(file_name, ".");
	strcpy(extension, strtok(NULL, "."));
	if (!strcmp(extension, "html") || !strcmp(extension, "htm"))
		return "text/html";
	else
		return "text/plain";
}

void send_data(int clnt_sock, char *ct, char *file_name)
{
	char protocol[] = "HTTP/1.0 200 OK\r\n";
	char server[] = "Server:Linux Web Server \r\n";
	char cnt_len[] = "Content-length:2048\r\n";
	char cnt_type[SMALL_BUF];
	char buf[2048];
	int send_file;

	sprintf(cnt_type, "Content-type:%s\r\n\r\n", ct);
	send_file = open(file_name, O_RDONLY);
	if (send_file < 0)
	{
		send_error(clnt_sock);
		return ;
	}

	// 헤더 정보 전송
	send(clnt_sock, protocol, strlen(protocol), 0);
	send(clnt_sock, server, strlen(server), 0);
	send(clnt_sock, cnt_len, strlen(cnt_len), 0);
	send(clnt_sock, cnt_type, strlen(cnt_type), 0);
	
	int len;
	while ((len = read(send_file, buf, 2048)) > 0)
	{
		cout << buf << endl;
		buf[len] = '\0';
		send(clnt_sock, buf, len + 1, 0);
	}
}

void send_error(int clnt_sock)
{
	char protocol[] = "HTTP/1.0 400 Bad Request\r\n";
	char server[] = "Server:Linux Web Server \r\n";
	char cnt_len[] = "Content-length:2048\r\n";
	char cnt_type[] = "Content-type:text/html\r\n\r\n";
	char content[] = "<html><head><title>NETWORK</title></head>"
		"<body><font size = +5><br>오류 발생! 요청 파일명 및 요청 방식 확인!"
		"</font></body></html>";
	
	send(clnt_sock, protocol, strlen(protocol), 0);
	send(clnt_sock, server, strlen(server), 0);
	send(clnt_sock, cnt_len, strlen(cnt_len), 0);
	send(clnt_sock, cnt_type, strlen(cnt_type), 0);
	send(clnt_sock, content, strlen(content), 0);
}