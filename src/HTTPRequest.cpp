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
	this->status = Parsing(START_LINE);
	this->method = -1;
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
	std::cout << "methodName : " << methodName << std::endl;
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
		{
			std::cout << "Method Error!!! : " << this->method << std::endl;
			
			return (Status(BAD_REQUEST));
		}
		if (!checkUri(startLine))
		{
			std::cout << "Uri Error!!! : " << this->requestUri << std::endl;
			return (Status(BAD_REQUEST));
		}
		this->version = startLine;
		if (this->version != "HTTP/1.1")
		{
			std::cout << "Version Error!!! : " << this->version << std::endl;
			return (Status(BAD_REQUEST));
		}
		erase(0, getReadPos());
		return (Parsing(HEADERS));
	}
	else
		return (Parsing(START_LINE));
}

int HTTPRequest::parse()
{
	if (status == Parsing(START_LINE))
		status = parseStartLine();
	if (status == Parsing(HEADERS))
		status = parseHeaders();
	std::cout << "after parseHeader parse : " << status << std::endl;
	if ((method != POST) && (method != PUT))
	{
		if (status == Parsing(PREBODY))
			status = Parsing(COMPLETE);
	}
	if (status == Parsing(PREBODY))
		status = checkChunked();
	std::cout << "after checkChunked parse : " << status << std::endl;
	if (status == Parsing(BODY))
		status = parseBody_contentLen();
	if (status == Parsing(CHUNK))
		status = parseBody_chunked();
	std::cout << "after Body parse : " << status << std::endl;
	return (status);
}