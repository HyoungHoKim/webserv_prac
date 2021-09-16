#include <cctype>

#include "HTTPMessage.hpp"
#include "HTTPRequest.hpp"

HTTPRequest::HTTPRequest() : HTTPMessage()
{
	this->init();
}

HTTPRequest::HTTPRequest(std::string sData) : HTTPMessage(sData)
{
	this->init();
}

HTTPRequest::HTTPRequest(byte *pData, unsigned int len) : HTTPMessage(pData, len)
{
	this->init();
}

HTTPRequest::~HTTPRequest()
{ }

void HTTPRequest::init()
{
	this->isPreBodyDone = false;
	this->isError = false;
	this->method = 0;
	this->requestUri = "";
	this->config_dir = "";
}

int HTTPRequest::methodStrToInt(std::string name)
{
	if (name.empty() || name.size() >= 10)
		return (-1);
	
	int ret = -1;
	for (unsigned int i = 0; i < NUM_METHODS; i++)
	{
		if (strcmp(requestMethodStr[i], name.c_str()) == 0)
		{
			ret = i;
			break;
		}
	}
	return (ret);
}

std::string HTTPRequest::methodIntToStr(unsigned int mid)
{
	if (mid >= NUM_METHODS)
		return ("");
	return (requestMethodStr[mid]);
}

byte* HTTPRequest::create()
{
	clear();

	std::string mstr = "";
	mstr = methodIntToStr(method);
	if (mstr.empty())
	{
		std::cout << "Could not create HTTPRequest, unknown method id: " << method << std::endl;
		return (NULL);
	}
	putLine(mstr + " " + requestUri + " " + version);

	putHeaders();

	if ((data != NULL) && dataLen > 0)
		putBytes(data, dataLen);
	
	byte *createRetData = new byte[size()];
	setReadPos(0);
	getBytes(createRetData, size());

	return (createRetData);
}

bool HTTPRequest::checkMethod(std::string& startLine)
{
	std::string methodName = "";
	int spacePos = startLine.find(' ');

	methodName = startLine.substr(0, spacePos);
	startLine = startLine.substr(spacePos + 1);

	if (methodName == "")
		return (false);
	for (size_t i = 0; i < methodName.length(); i++)
	{
		if (isupper(methodName[i]) == 0)
			return (false);
	}
	this->method = methodStrToInt(methodName);
	return (true);
}

bool HTTPRequest::checkUri(std::string& startLine)
{
	std::string urlName = "";
	std::string dirName = "";
	int spacePos = startLine.find(' ');

	urlName = startLine.substr(0, spacePos);
	startLine = startLine.substr(spacePos + 1);
	
	if (urlName == "")
		return (false);
	if (urlName[0] != '/')
		return (false);
	dirName += "/";
	size_t i = 1;
	for (; i < urlName.length(); i++)
	{
		if (urlName[i] == '/')
			break;
		dirName += urlName[i];
	}
	if (dirName == urlName)
	{
		if (dirName.find(".") == std::string::npos)
			this->config_dir = dirName;
		else
		{
			this->config_dir = "/";
			this->requestUri = dirName;
		}
	}
	else
	{
		if (dirName.find(".") == std::string::npos)
		{
			this->config_dir = dirName;
			this->requestUri = urlName.substr(i);
		}
		else
			return (false);
	}
	return (true);	
}

int HTTPRequest::parseStartLine()
{
	std::string startLine = getLine();
	if (startLine != "")
	{
		if (!checkMethod(startLine))
			return (Status(BAD_REQUEST));
		if (!checkUri(startLine))
			return (Status(BAD_REQUEST));
		this->version = startLine;
		if (this->version != "HTTP/1.1")
			return (Status(BAD_REQUEST));
		return (Parsing(SUCESSES));
	}
	else
		return (Parsing(REREAD));
}

int HTTPRequest::parse()
{
	int status = 0;

	if (this->isError)
		return (Parsing(REREAD));

	if (!(this->isPreBodyDone))
	{
		setReadPos(0);
		status = parseStartLine();
		if (status == Status(BAD_REQUEST))
		{
			this->isError = true;
			return (status);
		}
		status = parseHeaders();
		if (status == Status(BAD_REQUEST))
		{
			this->isError = true;
			return (status);
		}
		if (status == Parsing(SUCESSES))
		{
			this->isPreBodyDone = true;
			erase(0, getReadPos());
		}
	}
	if (!(this->isPreBodyDone))
		return (Parsing(REREAD));
	if ((method != POST) && (method != PUT))
		return (Parsing(SUCESSES));
	if (checkChunked() == Status(BAD_REQUEST))
	{
		this->isError = true;
		return (Status(BAD_REQUEST));
	}
	status = parseBody();
	if (status == Status(BAD_REQUEST)) 
	{
		this->isError = true;
		return (BAD_REQUEST);
	}
	return (status);
}