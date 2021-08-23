#include "ResourceHost.hpp"

ResourceHost::ResourceHost(std::string base)
: baseDiskPath(base)
{ }

ResourceHost::~ResourceHost()
{ }

std::string ResourceHost::lookupMimeType(std::string ext)
{
	std::unordered_map<std::string, std::string>::iterator it = mimeMap.find(ext);
	if (it == mimeMap.end())
		return ("");
	return (it->second);
}

Resource *ResourceHost::readFile(std::string path, struct stat sb)
{
	if (!(sb.st_mode & S_IRWXU))
		return (NULL);
	
	std::ifstream file;
	unsigned int len = 0;

	file.open(path.c_str(), std::ios::binary);

	if (!file.is_open())
		return (NULL);
	
	len = sb.st_size;

	byte *fdata = new byte[len];
	bzero(fdata, len);
	file.read((char*)fdata, len);

	file.close();

	Resource *res = new Resource(path);
	std::string name = res->getName();
	if (name.length() == 0)
		return (NULL);
	
	if (name.c_str()[0] == '.')
		return (NULL);
	
	std::string mimetype = lookupMimeType(res->getExtension());
	if (mimetype.length() != 0)
		res->setMimeType(mimetype);
	else
		res->setMimeType("application/octet-stream");
	
	res->setData(fdata, len);
	return (res);
}

Resource *ResourceHost::readDirectory(std::string path, struct stat sb)
{
	Resource *res = NULL;
	if (path.empty() || path[path.length() - 1] != '/')
		path += "/";

	int numIndexes = sizeof(validIndexes) / sizeof(*validIndexes);
	std::string loadIndex;
	struct stat sidx;
	for (int i = 0; i < numIndexes; i++)
	{
		loadIndex = path + validIndexes[i];
		if (stat(loadIndex.c_str(), &sidx) != -1)
			return (readFile(loadIndex.c_str(), sidx));
	}

	if (!(sb.st_mode & S_IRWXU))
		return (NULL);
	
	std::string listing = generateDirList(path);

	unsigned int slen = listing.length();
	char *sdata = new char[slen];
	bzero(sdata, slen);
	strncpy(sdata, listing.c_str(), slen);

	res = new Resource(path, true);
	res->setMimeType("text/html");
	res->setData((byte*)sdata, slen);

	return (res);
}

std::string ResourceHost::generateDirList(std::string path)
{
	size_t uri_pos = path.find(baseDiskPath);
	std::string uri = "?";
	if (uri_pos != std::string::npos)
		uri = path.substr(uri_pos + baseDiskPath.length());

	std::stringstream ret;
	ret << "<html><head><title>" << uri << "</title></head><body>";

	DIR *dir;
	struct dirent *ent;

	dir = opendir(path.c_str());
	if (dir == NULL)
		return ("");

	ret << "<h1>Index of " << uri << "</h1><hr /><br />";

	while ((ent = readdir(dir)) != NULL)
	{
		if (ent->d_name[0] == '.')
			continue;

		ret << "<a href=\"" << uri << ent->d_name << "\"" << ent->d_name << "</a><br />";
	}
	closedir(dir);
	ret << "</body><html>";

	return (ret.str());
}

Resource *ResourceHost::getResource(std::string uri)
{
	if (uri.length() > 255 || uri.empty())
		return (NULL);
	
	if (uri.find("../") != std::string::npos || uri.find("/..") != std::string::npos)
		return (NULL);

	std::string path = baseDiskPath + uri;
	Resource *res = NULL;

	struct stat sb;
	if (stat(path.c_str(), &sb) == -1)
		return (NULL);
	
	if (sb.st_mode & S_IFDIR)
		res = readDirectory(path, sb);
	else if (sb.st_mode & S_IFREG)
		res = readFile(path, sb);
	else
		return (NULL);

	return (res);
}