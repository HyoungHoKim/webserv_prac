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
	if (this->headers)
		delete this->headers;
}

void HTTPMessage::init()
{
	this->parseErrorStr = "";

	this->data = NULL;
	this->dataLen = 0;
	this->isChunked = false;
	this->chunked_status = false;
	this->chunk_size = -1;

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
		if (c == 13)
		{
			c = get();
			if (peek() == 10)
			{
				newLineReached = true;
				get();
				break;
			}
			setReadPos(getReadPos() - 1);
		}
		ret += getChar();
	}
	if (!newLineReached)
	{
		setReadPos(startPos);
		ret = "";
		return (ret);
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

bool HTTPMessage::checkHeaderEnd()
{
	std::string flag = "";

	setReadPos(getReadPos() - 2);
	for (int i = 0; i < 2; i++)
		flag += getChar();

	if (flag[0] == 13 && flag[1] == 10)
		return (true);
	return (false);
}

int HTTPMessage::parseHeaders()
{
	std::string hline = "";
	std::string app = "";

	while ((hline = getLine()) != "")
	{
		app = hline;
		size_t kpos = app.find(':');
		if (kpos == std::string::npos || kpos == 0)
		{
			std::cout << "Could not addHeader: " << app.c_str() << std::endl;
			return (Status(BAD_REQUEST));
		}
		addHeader(hline);
		erase(0, getReadPos());
	}
	if (checkHeaderEnd())
	{
		erase(0, getReadPos());
		return (Parsing(PREBODY));
	}
	return (Parsing(HEADERS));
}

int HTTPMessage::checkChunked()
{
	if (getHeaderValue("Transfer-Encoding") == "chunked")
		return (Parsing(CHUNK));
	else
	{
		std::string contentLen = getHeaderValue(("Content-Length"));
		if (contentLen != "")
		{
			for (size_t i = 0; i < contentLen.length(); i++)
			{
				if (std::isdigit(contentLen[i]) != 0)
				{
					std::cout << "ContentLength is not digit!!!" << std::endl;
					return (Status(BAD_REQUEST));
				}
			}
			
			return (Parsing(BODY));
		}
		else
		{
			std::cout << "CHUNK and Content-Length is not exist!!!" << std::endl;
			return (Status(BAD_REQUEST));
		}
	}
}

bool HTTPMessage::parseBody()
{
	if (!(this->isChunked))
		return (parseBody_contentLen());
	else
		return (parseBody_chunked());
}

int HTTPMessage::parseBody_contentLen()
{
	std::string hlenstr = "";
	unsigned int contentLen = 0;
	hlenstr = getHeaderValue("Content-Length");
	
	contentLen = atoi(hlenstr.c_str());

	if (contentLen > bytesRemaining() + 1)
		return (Parsing(BODY));
	
	this->dataLen = contentLen;
	unsigned int dIdx = 0;
	unsigned int s = size();
	this->data = new byte[dataLen + 1];
	bzero(this->data, dataLen + 1);

	for (unsigned int i = getReadPos(); i < s; i++)
	{
		this->data[dIdx] = get(i);
		dIdx++;
	}
	erase(0, getReadPos());
	return (Parsing(COMPLETE));
}

int HTTPMessage::parseBody_chunked()
{
	std::string line = "";
	std::string body = "";
	
	while ((line = getLine()) != "" && this->chunk_size != 0)
	{
		// false : data size
		if (!this->chunked_status)
		{
			if (line[0] == '0' && line[1] == 'x')
				this->chunk_size = static_cast<int>(std::strtol(line.c_str(), NULL, 0));
			else
				this->chunk_size = static_cast<int>(std::strtol(line.c_str(), NULL, 16));
			erase(0, getReadPos());
			this->chunked_status = true;
		}
		else
		{
			body += line;
			erase(0, getReadPos());
			this->chunked_status = false;
		}
	}
	
	if (body != "")
	{
		if (!this->data)
		{
			this->data = new byte[body.length() + 1];
			bzero(this->data,  body.length() + 1);
			memcpy(this->data, body.c_str(), body.length());
			this->dataLen = body.length();
		}
		else
		{
			int totalSize = this->dataLen + body.length() + 1;
			byte *temp = this->data;
			this->data = new byte[totalSize];
			bzero(this->data, totalSize);
			memcpy(this->data, temp, this->dataLen);
			memcpy(this->data + this->dataLen, body.c_str(), body.length());
			this->dataLen = totalSize - 1;
			if (temp)
				delete temp;
		}
	}

	if (this->chunk_size == 0)
	{
		if (checkHeaderEnd())
		{
			erase(0, getReadPos());
			return (Parsing(COMPLETE));
		}
	}
	return (Parsing(CHUNK));
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