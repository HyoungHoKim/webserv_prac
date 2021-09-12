#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include "ByteBuffer.hpp"
#include "HTTPMessage.hpp"
#include "ServerConfig.hpp"
#include <sys/stat.h>

class HTTPRequest : public HTTPMessage
{
private:
	bool isPreBodyDone;

	int method;
	std::string config_dir;
	std::string requestUri;

protected:
	virtual void init();

public:
	HTTPRequest();
	HTTPRequest(std::string sData);
	HTTPRequest(byte *pData, unsigned int len);
	virtual ~HTTPRequest();

	virtual byte *create();
	int parseStartLine();
	bool checkMethod(std::string& startLine);
	bool checkUri(std::string& startLine);
	void deleteRequest();
	virtual int parse();

	int methodStrToInt(std::string name);
	std::string methodIntToStr(unsigned int mid);

	void setMethod(int m)
	{
		method = m;
	}

	int getMethod()
	{
		return (method);
	}

	void setRequestUri(std::string u)
	{
		requestUri = u;
	}

	std::string getRequestUri()
	{
		return (requestUri);
	}

	std::string getConfig_dir()
	{
		return (config_dir);
	}
};

#endif