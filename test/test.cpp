#include <iostream>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <fstream>

void 		parseCGI()
{
	std::string			temp;
	std::string			header;
	std::string			key;
	std::string			value;
	size_t				pos;

	std::ifstream in("temp.txt");
	std::string body;

	if (in.is_open())
	{
		in.seekg(0, std::ios::end);
		int size = in.tellg();
		body.resize(size);
		in.seekg(0, std::ios::beg);
		in.read(&body[0], size);
	}
	else
		std::cout << "Cannot find file" << std::endl;	
	pos = body.find("\r\n\r\n");
	header = body.substr(0, pos);
	body = body.substr(pos + 1, body.size());
	if ((pos = header.find("Status")) != std::string::npos)
	{
		while (header[pos] != '\r')
			pos++;
	}
	while (header[pos] == '\r' || header[pos] == '\n')
		pos++;
	header = header.substr(pos, header.size());
	pos = 0;
	while (header[pos])
	{
		while (header[pos] && header[pos] != ':')
		{
			key += header[pos];
			pos++;
		}
		if (header[pos] == ':')
			pos += 2;
		while (header[pos] && header[pos] != '\r')
		{
			value += header[pos];
			pos++;
		}
		if (header[pos] == '\r')
			pos++;
		if (header[pos] == '\n')
			pos++;
	}
	pos = 0;
	while (body[pos] == '\r' || body[pos] == '\n')
		pos++;
	body = body.substr(pos, body.length());
}

int				main()
{
	parseCGI();
	return (0);
}
