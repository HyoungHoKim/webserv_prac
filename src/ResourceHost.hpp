#ifndef RESOURCEHOST_HPP
#define RESOURCEHOST_HPP

#include <iostream>
#include <sstream>
#include <string>
#include <cstring>
#include <map>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>
#include <vector>

#include "Resource.hpp"

class ResourceHost
{
private:
	std::string baseDiskPath;
	std::vector<std::string> validIndexes;
	std::map<std::string, std::string> mimeMap;

	void initMimeMap(void);
	std::string lookupMimeType(std::string ext);
	Resource *readFile(std::string path, struct stat sb);
	Resource *readDirectory(std::string path, struct stat sb);
	std::string generateDirList(std::string dirPath);

public:
	ResourceHost(std::string base, std::vector<std::string> _validIndexes);
	~ResourceHost();

	std::string getBaseDiskPath(void) const;

	Resource *getResource(Resource *res, bool writeToDisk);
	Resource *getResource(std::string uri);
};

#endif