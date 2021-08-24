#include "HTTPMessage.hpp"

HTTPMessage::HTTPMessage()
: ByteBuffer(4096)
{
	this->init();
}

HTTPMessage::HTTPMessage(std::string sData)
: ByteBuffer(sData.size() + 1)
{
	putBytes((byte*)sData.c_str(), sData.size() + 1);
	this->init();
}

HTTPMessage::HTTPMessage(byte *pData, unsigned int len)
: ByteBuffer(pData, len)
{
	this->init();
}

HTTPMessage::~HTTPMessage()
{
	this->headers->clear();
	delete this->headers;
}

void HTTPMessage::init()
{
	this->parseErrorStr = "";

	this->data = NULL;
	this->dataLen = 0;

	this->version = DEFAULT_HPP_VERSION;

	this->headers = new std::map<std::string, std::string>();
}

void HTTPMessage::putLine(std::string str, bool crlf_end)
{
	if (crlf_end)
		str += "\r\n";
	
	putBytes((byte*)str.c_str(), str.size());
}

void HTTPMessage::putHeaders()
{
	std::map<std::string, std::string>::iterator iter;
	for (iter = this->headers->begin(); iter != this->headers->end(); iter++)
	{
		std::string final = iter->first + ": " + iter->second;
		putLine(final, true);
	}	
	putLine();
}

std::string HTTPMessage::getLine()
{
	std::string ret = "";
	int startPos = getReadPos();
	bool newLineReached = false;
	char c = 0;

	for (unsigned int i = startPos; i < size(); i++)
	{
		c = peek();
		if (c == 13 || c == 10)
		{
			newLineReached = true;
			break;
		}

		ret += getChar();
	}

	if (!newLineReached)
	{
		setReadPos(startPos);
		ret = "";
		return (ret);
	}

	unsigned int k = 0;
	for (unsigned int i = getReadPos(); i < size(); i++)
	{
		if (k++ >= 2)
			break;
		c = getChar();
		if (c != 13 && c != 10)
		{
			setReadPos(getReadPos() - 1);
			break;
		}
	}
	return (ret);
}

std::string HTTPMessage::getStrElement(char delim)
{
	std::string ret = "";
	int startPos = getReadPos();
	unsigned int size = 0;
	int endPos = find(delim, startPos);

	size = (endPos + 1) - startPos;

	if (endPos == -1 || size <= 0)
		return "";
	
	char *str = new char[size];
	bzero(str, size);
	getBytes(reinterpret_cast<byte*>(str), size);
	str[size - 1] = '\0';
	ret.assign(str);

	setReadPos(endPos + 1);
	
	return (ret);
}

void HTTPMessage::parseHeaders()
{
	std::string hline = "";
	std::string app = "";

	hline = getLine();

	while (hline.size() > 0)
	{
		app = hline;
		while (app[app.size() - 1] == ',')
		{
			app = getLine();
			hline += app;
		}
		addHeader(hline);
		hline = getLine();
	}
}

bool HTTPMessage::parseBody()
{
	std::string hlenstr = "";
	unsigned int contentLen = 0;
	hlenstr = getHeaderValue("Content-Length");

	if (hlenstr.empty())
		return (true);
	
	contentLen = atoi(hlenstr.c_str());

	if (contentLen > bytesRemaining() + 1)
	{
		std::stringstream pes;
		pes << "Content-Length (" << hlenstr << ") is greater than remaining bytes (" << bytesRemaining() << ")";
		this->parseErrorStr = pes.str();
		return (false);
	}
	else
		this->dataLen = contentLen;

	unsigned int dIdx = 0;
	unsigned int s = size();
	this->data = new byte[dataLen];

	for (unsigned int i = getReadPos(); i < s; i++)
	{
		this->data[dIdx] = get(i);
		dIdx++;
	}
	return (true);
}

void HTTPMessage::addHeader(std::string line)
{
	std::string key = "";
	std::string value = "";
	size_t kpos;
	unsigned long i = 0;
	kpos = line.find(':');
	if (kpos == std::string::npos)
	{
		std::cout << "Could not addHeader: " << line.c_str() << std::endl;
		return;
	}
	key = line.substr(0, kpos);
	value = line.substr(kpos + 1, line.size() - kpos - 1);

	while (i < value.size() && value.at(i) == ' ')
		i++;
	value = value.substr(i, value.size());

	addHeader(key, value);
}

void HTTPMessage::addHeader(std::string key, std::string value)
{
	this->headers->insert(std::pair<std::string, std::string>(key, value));
}

void HTTPMessage::addHeader(std::string key, int value)
{
	std::stringstream sz;
	sz << value;
	headers->insert(std::pair<std::string, std::string>(key, sz.str()));
}

std::string HTTPMessage::getHeaderValue(std::string key)
{
	char c;
	std::string key_lower = "";

	std::map<std::string, std::string>::iterator it = this->headers->find(key);

	if (it == this->headers->end())
	{
		for (unsigned long i = 0; i < key.length(); i++)
		{
			c = key.at(i);
			key_lower += tolower(c);
		}

		it = this->headers->find(key_lower);
		if (it == this->headers->end())
			return "";
	}

	return (it->second);
}

std::string HTTPMessage::getHeaderStr(int index)
{
	int i = 0;
	std::string ret = "";
	std::map<std::string, std::string>::iterator it;
	for (it = this->headers->begin(); it != this->headers->end(); it++)
	{
		if (i == index)
		{
			ret = it->first + ": " + it->second;
			break;
		}
		i++;
	}
	return (ret);
}

int HTTPMessage::getNumHeaders()
{
	return (this->headers->size());
}

void HTTPMessage::clearHeaders()
{
	this->headers->clear();
}