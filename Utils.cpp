#include "Server.hpp"

#define SMALL_BUF 1024

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