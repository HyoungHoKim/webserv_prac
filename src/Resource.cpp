#include "Resource.hpp"

Resource::Resource(std::string loc, bool dir)
{
	this->location = loc;
	this->directory = dir;
	this->mimeType = "";
	this->size = 0;
	this->data = NULL;
}

Resource::~Resource()
{
	if (this->data != NULL)
	{
		delete[] this->data;
		this->data = NULL;
	}
}