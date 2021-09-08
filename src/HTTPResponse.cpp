#include "HTTPMessage.hpp"
#include "HTTPResponse.hpp"

HTTPResponse::HTTPResponse() : HTTPMessage()
{
	this->init();
}

HTTPResponse::HTTPResponse(std::string sData) : HTTPMessage(sData)
{
	this->init();
}

HTTPResponse::HTTPResponse(byte *pData, unsigned int len) : HTTPMessage(pData, len)
{
	this->init();
}

HTTPResponse::~HTTPResponse()
{ }

void HTTPResponse::init()
{
	status = 0;
	reason = "";
}

void HTTPResponse::determineStatusCode()
{
	if (reason.find("Continue") != std::string::npos)
		status = Status(CONTINUE);
	else if (reason.find("OK") != std::string::npos)
		status = Status(OK);
	else if (reason.find("Create") != std::string::npos)
		status = Status(CREATE);
	else if (reason.find("No Content") != std::string::npos)
		status = Status(NO_CONTENT);
	else if (reason.find("Bad Request") != std::string::npos)
		status = Status(BAD_REQUEST);
	else if (reason.find("Not Found") != std::string::npos)
		status = Status(NOT_FOUND);
	else if (reason.find("Method Not Allowed") != std::string::npos)
		status = Status(METHOD_NOT_ALLOW);
	else if (reason.find("Server Error") != std::string::npos)
		status = Status(SERVER_ERROR);
	else if (reason.find("Not Implemented") != std::string::npos)
		status = Status(NOT_IMPLEMENTED);
}

void HTTPResponse::determineReasonStr()
{
	switch (status)
	{
	case Status(CONTINUE):
		reason = "Continue";
		break;
	case Status(OK):
		reason = "OK";
		break;
	case Status(CREATE):
		reason = "Create";
		break;
	case Status(NO_CONTENT):
		reason = "No Content";
		break;
	case Status(BAD_REQUEST):
		reason = "Bad Request";
		break;
	case Status(NOT_FOUND):
		reason = "Not Found";
		break;
	case Status(METHOD_NOT_ALLOW):
		reason = "Method Not Allowed";
		break;
	case Status(SERVER_ERROR):
		reason = "Internal Server Error";
		break;
	case Status(NOT_IMPLEMENTED):
		reason = "Not Implemented";
		break;
	default:
		break;
	}
}

byte *HTTPResponse::create()
{
	clear();

	std::stringstream sline;
	sline << version << " " << status << " " << reason;
	putLine(sline.str());

	putHeaders();

	if ((data != NULL) && dataLen > 0)
		putBytes(data, dataLen);
	
	byte *createRetData = new byte[size()];
	setReadPos(0);
	getBytes(createRetData, size());

	return (createRetData);
}

bool HTTPResponse::parse()
{
	std::string statusstr;

	version = getStrElement();
	statusstr = getStrElement();
	determineStatusCode();
	reason = getLine();

	parseHeaders();

	if (!parseBody())
		return (false);

	return (true);
}