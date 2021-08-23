#ifndef RESOURCEHOST_HPP
#define RESOURCEHOST_HPP

#include <iostream>
#include <sstream>
#include <string>
#include <cstring>
#include <unordered_map>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>

#include "Resource.hpp"

const static char* const validIndexes[] = 
{
	"index.html",
	"index.htm"
};

class ResourceHost
{
private:
	std::string baseDiskPath;
	std::unordered_map<std::string, std::string> mimeMap;

	std::string lookupMimeType(std::string ext);
	Resource *readFile(std::string path, struct stat sb);
	Resource *readDirectory(std::string path, struct stat sb);
	std::string generateDirList(std::string dirPath);

public:
	ResourceHost(std::string base);
	~ResourceHost();

	void putResource(Resource *res, bool writeToDisk);

	Resource *getResource(Resource *res, bool writeToDisk);

	Resource *getResource(std::string uri);
};


#endif