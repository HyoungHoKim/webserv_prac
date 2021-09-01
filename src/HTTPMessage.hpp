#ifndef HTTPMESSAGE_HPP
#define HTTPMESSAGE_HPP

#include <map>
#include <string>
#include <sstream>

#include "ByteBuffer.hpp"

#define DEFAULT_HPP_VERSION "HTTP/1.1"
#define NUM_METHODS 9

enum Method
{
	HEAD = 0,
	GET = 1,
	POST = 2,
	PUT = 3,
	DEL = 4,
	TRACE = 5,
	OPTIONS = 6,
	CONNECT = 7,
	PATCH = 8
};

const static char* const requestMethodStr[NUM_METHODS] = {
	"HEAD",
	"GET",
	"POST",
	"PUT",
	"DELETE",
	"TRACE",
	"OPTIONS",
	"CONNECT",
	"PATCH"
};

enum Status
{
	// 1xx Informational
	CONTINUE = 100,

	// 2xx Success
	OK = 200,
	CREATE = 201,

	// 3xx Redirection

	// 4xx Client Error
	BAD_REQUEST = 400,
	NOT_FOUND = 404,

	// 5xx Server Error
	SERVER_ERROR = 500,
	NOT_IMPLEMENTED = 501
};

class HTTPMessage : public ByteBuffer
{
private:
	std::map<std::string, std::string> *headers;

protected:
	std::string parseErrorStr;
	std::string version;

	// Message Body data (Resource in the case of a response, extra parameters in the case of a request)
	byte* data;
	unsigned int dataLen;
	virtual void init();

public:
	HTTPMessage();
	HTTPMessage(std::string sData);
	HTTPMessage(byte *pData, unsigned int len);
	virtual ~HTTPMessage();

	virtual byte* create() = 0;
	virtual bool parse() = 0;

	// Create helpers
	void putLine(std::string str = "", bool crlf_end = true);
	void putHeaders();

	// Parse helpers
	std::string getLine();
	std::string getStrElement(char delim = ' '); // 0x20 = "space"
	void parseHeaders();
	bool parseBody();

	void addHeader(std::string line);
	void addHeader(std::string key, std::string value);
	void addHeader(std::string key, int value);
	std::string getHeaderValue(std::string key);
	std::string getHeaderStr(int index);
	int getNumHeaders();
	void clearHeaders();

	std::string getParseError()
	{
		return (parseErrorStr);
	}

	void setVersion(std::string v)
	{
		version = v;
	}

	std::string getVersion()
	{
		return (version);
	}

	void setData(byte *d, unsigned int len)
	{
		data = d;
		dataLen = len;
	}

	byte *getData()
	{
		return (data);
	}

	unsigned int getDataLength()
	{
		return (dataLen);
	}
};

#endif