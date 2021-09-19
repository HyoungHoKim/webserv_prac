#include "ServerConfig.hpp"

ServerConfig::ServerConfig() : max_body_size(0)
{
}

ServerConfig::~ServerConfig()
{
}

bool	ServerConfig::isValidDirective(std::string temp)
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

void	ServerConfig::initServer(std::vector<std::string>::iterator &it)
{
	if (*it != "{")
	{
		std::cout << "3" << std::endl;
		throw errorInConfig();
	}
	while (*(++it) != "}")
	{
		if (isValidDirective(*it))
			getDirective(it);
	}
}

void	ServerConfig::getDirective(std::vector<std::string>::iterator &it)
{
	std::string		temp;

	temp = *it;
	if (!temp.compare("listen"))
		this->parseListen(++it);		
	else if (!temp.compare("location"))
		this->parseLocations(++it);
	else if (!temp.compare("root"))
		this->parseRoot(++it);
	else if (!temp.compare("methods"))
		this->parseMethod(++it);
	else if (!temp.compare("index"))
		this->parseIndex(++it);
	else if (!temp.compare("server_name"))
		this->parseServerName(++it);
	else if (!temp.compare("client_max_body_size"))
		this->parseClientMaxBodySize(++it);
}

void	ServerConfig::parseListen(std::vector<std::string>::iterator &it)
{
	std::string temp = *it;

	this->listen = temp;
	it++;
	if (*it == ";")
		it++;
}

void	ServerConfig::parseLocations(std::vector<std::string>::iterator &it)
{
	ServerConfig loc;

	loc = *this;
	loc.loopLocation(it, this->locations);
}

void	ServerConfig::loopLocation(std::vector<std::string>::iterator &it, std::vector<ServerConfig> &locations)
{
	this->uri = *it++;
	// if (*it != "{")
	// 	error
	while (*(++it) != "}")
	{
		if (isValidDirective(*it))
			getDirective(it);
	}
	locations.push_back(*this);
}

void	ServerConfig::parseRoot(std::vector<std::string>::iterator &it)
{
	std::string temp = *it;

	this->root = temp;
	it++;
}

void	ServerConfig::parseMethod(std::vector<std::string>::iterator &it)
{
	std::string temp;

	while (1)
	{
		temp = *it;
		if (temp.find(',', temp.length() - 1) != std::string::npos)
		{
			temp.erase(temp.length() - 1);
			this->method.push_back(temp);
		}
		else if (temp.find(',', temp.length() - 1) == std::string::npos)
		{
			this->method.push_back(temp);
			it++;
			break ;
		}
		it++;
	}
}

void	ServerConfig::parseIndex(std::vector<std::string>::iterator &it)
{
	while (*it != ";")
		this->index.push_back(*it++);
}

void	ServerConfig::parseServerName(std::vector<std::string>::iterator &it)
{
	while (*it != ";")
		ServerName.push_back(*it++);
}

void	ServerConfig::parseClientMaxBodySize(std::vector<std::string>::iterator &it)
{
	this->max_body_size = ft::stoi(*it++);
}

int	ServerConfig::getID() const
{
	return (this->id);
}

std::string ServerConfig::getListen() const
{
	return (this->listen);
}

std::vector<ServerConfig> ServerConfig::getLocations() const
{
	return (this->locations);
}

std::string ServerConfig::getRoot()
{
	return (this->root);
}

std::string ServerConfig::getError() const
{
	return (this->error);
}

std::vector<std::string> ServerConfig::getMethod() const
{
	return (this->method);
}

std::vector<std::string> ServerConfig::getIndex()
{
	return (this->index);
}

std::vector<std::string> ServerConfig::getServerName() const
{
	return (this->ServerName);
}

std::string		ServerConfig::getUri() const
{
	return (this->uri);
}

size_t	ServerConfig::getClientMaxBodySize() const
{
	return (this->max_body_size);
}

void	ServerConfig::setID(int _id)
{
	this->id = _id;
}

const char* ServerConfig::errorInConfig::what() const throw()
{
	return ("Error in Config");
}