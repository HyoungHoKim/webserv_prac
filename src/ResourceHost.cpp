#include "ResourceHost.hpp"

ResourceHost::ResourceHost(std::string base, bool _autoIndex, std::vector<std::string> _validIndexes)
: baseDiskPath(base), autoIndex(_autoIndex), validIndexes(_validIndexes)
{
	initMimeMap();
}

ResourceHost::~ResourceHost()
{ }

std::string ResourceHost::lookupMimeType(std::string ext)
{
	std::map<std::string, std::string>::iterator it = mimeMap.find(ext);
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

	int numIndexes = this->validIndexes.size();
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
	
	std::string listing = "";
	if (this->autoIndex)
		listing = generateDirList(path);
	else
	{
		std::ifstream fin("./www/dirPage.html");
		fin >> listing;
		fin.close();
	}

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

		ret << "<a href=\"" << uri << ent->d_name << "\">" << ent->d_name << "</a><br />";
	}
	closedir(dir);
	ret << "</body><html>";

	return (ret.str());
}

Resource *ResourceHost::getResource(std::string uri)
{
	if (uri.length() > 255)
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

std::string ResourceHost::getBaseDiskPath(void) const
{
	return (this->baseDiskPath);
}

void ResourceHost::initMimeMap(void)
{
	this->mimeMap["html"] = "text/html";
	this->mimeMap["htm"] = "text/html";
	this->mimeMap["shtml"] = "text/html";
	this->mimeMap["css"] = "text/css";
	this->mimeMap["xml"] = "text/xml";
	this->mimeMap["gif"] = "image/gif";
	this->mimeMap["jpeg"] = "image/jpeg";
	this->mimeMap["jpg"] = "image/jpeg";
	this->mimeMap["js"] = "application/javascript";
	this->mimeMap["atom"] = "application/atom+xml";
	this->mimeMap["rss"] = "application/rss+xml";
	this->mimeMap["mml"] = "text/mathml";
	this->mimeMap["txt"] = "text/plain";
	this->mimeMap["jad"] = "text/vnd.sun.j2me.app-descriptor";
	this->mimeMap["wml"] = "text/vnd.wap.wml";
	this->mimeMap["htc"] = "text/x-component";
	this->mimeMap["png"] = "image/png";
	this->mimeMap["svg"] = "image/svg+xml";
	this->mimeMap["svgz"] = "image/svg+xml";
	this->mimeMap["tif"] = "image/tiff";
	this->mimeMap["tiff"] = "image/tiff";
	this->mimeMap["wbmp"] = "image/vnd.wap.wbmp";
	this->mimeMap["webp"] = "image/webp";
	this->mimeMap["ico"] = "image/x-icon";
	this->mimeMap["jng"] = "image/x-jng";
	this->mimeMap["bmp"] = "image/x-ms-bmp";
	this->mimeMap["woff"] = "font/woff";
	this->mimeMap["woff2"] = "font/woff2";
	this->mimeMap["jar"] = "application/java-archive";
	this->mimeMap["war"] = "application/java-archive";
	this->mimeMap["ear"] = "application/java-archive";
	this->mimeMap["json"] = "application/json";
	this->mimeMap["hqx"] = "application/mac-binhex40";
	this->mimeMap["doc"] = "application/msword";
	this->mimeMap["pdf"] = "application/pdf";
	this->mimeMap["ps"] = "application/postscript";
	this->mimeMap["eps"] = "application/postscript";
	this->mimeMap["ai"] = "application/postscript";
	this->mimeMap["rtf"] = "application/rtf";
	this->mimeMap["m3u8"] = "application/vnd.apple.mpegurl";
	this->mimeMap["kml"] = "application/vnd.google-earth.kml+xml";
	this->mimeMap["kmz"] = "application/vnd.google-earth.kmz";
	this->mimeMap["xls"] = "application/vnd.ms-excel";
	this->mimeMap["eot"] = "application/vnd.ms-fontobject";
	this->mimeMap["ppt"] = "application/vnd.ms-powerpoint";
	this->mimeMap["odg"] = "application/vnd.oasis.opendocument.graphics";
	this->mimeMap["odp"] = "application/vnd.oasis.opendocument.presentation";
	this->mimeMap["ods"] = "application/vnd.oasis.opendocument.spreadsheet";
	this->mimeMap["odt"] = "application/vnd.oasis.opendocument.text";
	this->mimeMap["pptx"] = "application/vnd.openxmlformats-officedocument.presentationml.presentation";
	this->mimeMap["xlsx"] = "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
	this->mimeMap["docx"] = "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
	this->mimeMap["wmlc"] = "application/vnd.wap.wmlc";
	this->mimeMap["7z"] = "application/x-7z-compressed";
	this->mimeMap["cco"] = "application/x-cocoa";
	this->mimeMap["jardiff"] = "application/x-java-archive-diff";
	this->mimeMap["jnlp"] = "application/x-java-jnlp-file";
	this->mimeMap["run"] = "application/x-makeself";
	this->mimeMap["pl"] = "application/x-perl";
	this->mimeMap["pm"] = "application/x-perl";
	this->mimeMap["prc"] = "application/x-pilot";
	this->mimeMap["pdb"] = "application/x-pilot";
	this->mimeMap["rar"] = "application/x-rar-compressed";
	this->mimeMap["rpm"] = "application/x-redhat-package-manager";
	this->mimeMap["sea"] = "application/x-sea";
	this->mimeMap["swf"] = "application/x-shockwave-flash";
	this->mimeMap["sit"] = "application/x-stuffit";
	this->mimeMap["tcl"] = "application/x-tcl";
	this->mimeMap["tk"] = "application/x-tcl";
	this->mimeMap["der"] = "application/x-x509-ca-cert";
	this->mimeMap["pem"] = "application/x-x509-ca-cert";
	this->mimeMap["crt"] = "application/x-x509-ca-cert";
	this->mimeMap["xpi"] = "application/x-xpinstall";
	this->mimeMap["xhtml"] = "application/xhtml+xml";
	this->mimeMap["xspf"] = "application/xspf+xml";
	this->mimeMap["zip"] = "application/zip";
	this->mimeMap["bin"] = "application/octet-stream";
	this->mimeMap["exe"] = "application/octet-stream";
	this->mimeMap["dll"] = "application/octet-stream";
	this->mimeMap["deb"] = "application/octet-stream";
	this->mimeMap["dmg"] = "application/octet-stream";
	this->mimeMap["iso"] = "application/octet-stream";
	this->mimeMap["img"] = "application/octet-stream";
	this->mimeMap["msi"] = "application/octet-stream";
	this->mimeMap["msp"] = "application/octet-stream";
	this->mimeMap["msm"] = "application/octet-stream";
	this->mimeMap["mid"] = "audio/midi";
	this->mimeMap["midi"] = "audio/midi";
	this->mimeMap["kar"] = "audio/midi";
	this->mimeMap["mp3"] = "audio/mpeg";
	this->mimeMap["ogg"] = "audio/ogg";
	this->mimeMap["m4a"] = "audio/x-m4a";
	this->mimeMap["ra"] = "audio/x-realaudio";
	this->mimeMap["3gpp"] = "video/3gpp";
	this->mimeMap["3gp"] = "video/3gpp";
	this->mimeMap["ts"] = "video/mp2t";
	this->mimeMap["mp4"] = "video/mp4";
	this->mimeMap["mpeg"] = "video/mpeg";
	this->mimeMap["mpg"] = "video/mpeg";
	this->mimeMap["mov"] = "video/quicktime";
	this->mimeMap["webm"] = "video/webm";
	this->mimeMap["flv"] = "video/x-flv";
	this->mimeMap["m4v"] = "video/x-m4v";
	this->mimeMap["mng"] = "video/x-mng";
	this->mimeMap["asx"] = "video/x-ms-asf";
	this->mimeMap["asf"] = "video/x-ms-asf";
	this->mimeMap["wmv"] = "video/x-ms-wmv";
	this->mimeMap["avi"] = "video/x-msvideo";
}