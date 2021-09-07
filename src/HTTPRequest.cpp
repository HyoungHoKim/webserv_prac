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
	this->method = 0;
	this->requestUri = "";
	this->config_dir = "/";
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

bool HTTPRequest::parse()
{
	std::string initial = "";
	std::string methodName = "";

	methodName = getStrElement();
	this->requestUri = getStrElement();
	this->version = getLine();

	for (size_t i = 1; i < this->requestUri.length(); i++)
	{
		if (this->requestUri[i] == '/')
			break;
		this->config_dir += requestUri[i];
	}

	method = methodStrToInt(methodName);
	if (method == -1)
	{
		parseErrorStr = "Invalid Method: " + methodName;
		return (false);
	}

	parseHeaders();

	if ((method != POST) && (method != PUT))
		return (true);
	
	if (!parseBody())
		return (false);

	return (true);
}