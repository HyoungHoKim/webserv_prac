#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include "HTTPMessage.hpp"

class HTTPResponse : public HTTPMessage
{
private:
	int status;
	std::string reason;

	void determineReasonStr();
	void determineStatusCode();

protected:
	virtual void init();

public:
	HTTPResponse();
	HTTPResponse(std::string sData);
	HTTPResponse(byte *pData, unsigned int len);
	virtual ~HTTPResponse();

	virtual byte *create();
	virtual bool parse();

	void setStatus (int scode)
	{
		status = scode;
		determineReasonStr();
	}

	std::string getReason()
	{
		return (reason);
	}
};

#endif