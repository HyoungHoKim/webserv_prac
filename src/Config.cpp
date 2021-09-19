#include "Config.hpp"

Config::Config()
{
}

Config::~Config()
{
}

std::string			Config::readFile(char *file)
{
	char			c;
	std::string		buffer;
	std::ifstream	fin(file);

	while (fin.get(c))
		buffer += c;
	fin.close();
	return (buffer);
}

void				Config::parseConfig(char *file)
{
	std::string		buffer;
	std::vector<std::string>::iterator it;
	int				i;

	buffer = readFile(file);
	tokenize(buffer);
	i = 1;
	for (it = tokens.begin(); it != tokens.end(); it++)
	{
		if (*it == "server")
		{
			ServerConfig	server_config;
			server_config.setID(i++);
			server_config.initServer(++it);
			this->servers.push_back(server_config);
		}
	}
}

void				Config::tokenize(std::string &buffer)
{
	std::string		line;
	std::string		temp;
	std::vector<std::string>	brace;
	std::string::size_type		first;
	std::string::size_type		last;

	while (!buffer.empty())
	{
		last = 0;
		line = ft::getline(buffer);
		while ((first = line.find_first_not_of(" \t", last)) != std::string::npos)
		{
			if (line[first] == '#')
				break ;
			last = line.find_first_of(" \t", first);
			if (last == std::string::npos)
				last = line.size();
			temp = line.substr(first, last - first);
			if (temp == "{")
				brace.push_back(temp);
			else if (temp == "}")
			{
				if (brace.empty())
					throw Config::errorInConfig();
				else
					brace.pop_back();
			}
			if (isValidDirective(temp) && line[line.find_last_not_of(" \t", line.length())] != ';')
				throw Config::errorInConfig();
			if (temp.find(';', temp.length() - 1) != std::string::npos)
			{
				temp.erase(temp.length() - 1);
				tokens.push_back(temp);
				tokens.push_back(";");
			}
			else
				tokens.push_back(temp);
		}
	}
}

bool	Config::isValidDirective(std::string temp)
{
	if (!temp.compare("listen") ||
		!temp.compare("location") ||
		!temp.compare("root") ||
		!temp.compare("methods") ||
		!temp.compare("index") ||
		!temp.compare("server_name") ||
		!temp.compare("client_max_body_size"))
		return (true);
	else
		return (false);
}

const char* Config::errorInConfig::what() const throw()
{
	return ("error in config");
}

std::vector<ServerConfig> Config::getServers() const
{
	return (this->servers);
}