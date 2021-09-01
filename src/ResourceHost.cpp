#include "ResourceHost.hpp"

ResourceHost::ResourceHost(std::string base)
: baseDiskPath(base)
{
	mimeMap["html"] = "text/html";
	mimeMap["htm"] = "text/html";
	mimeMap["shtml"] = "text/html";
	mimeMap["css"] = "text/css";
	mimeMap["xml"] = "text/xml";
	mimeMap["gif"] = "image/gif";
	mimeMap["jpeg"] = "image/jpeg";
	mimeMap["jpg"] = "image/jpeg";
	mimeMap["js"] = "application/javascript";
	mimeMap["atom"] = "application/atom+xml";
	mimeMap["rss"] = "application/rss+xml";

	mimeMap["mml"] = "text/mathml";
	mimeMap["txt"] = "text/plain";
	mimeMap["jad"] = "text/vnd.sun.j2me.app-descriptor";
	mimeMap["wml"] = "text/vnd.wap.wml";
	mimeMap["htc"] = "text/x-component";
	mimeMap["png"] = "image/png";
	mimeMap["svg"] = "image/svg+xml";
	mimeMap["svgz"] = "image/svg+xml";
	mimeMap["tif"] = "image/tiff";
	mimeMap["tiff"] = "image/tiff";
	mimeMap["wbmp"] = "image/vnd.wap.wbmp";
	mimeMap["webp"] = "image/webp";
	mimeMap["ico"] = "image/x-icon";
	mimeMap["jng"] = "image/x-jng";
	mimeMap["bmp"] = "image/x-ms-bmp";
	mimeMap["woff"] = "font/woff";
	mimeMap["woff2"] = "font/woff2";
	mimeMap["jar"] = "application/java-archive";
	mimeMap["war"] = "application/java-archive";
	mimeMap["ear"] = "application/java-archive";
	mimeMap["json"] = "application/json";
	mimeMap["hqx"] = "application/mac-binhex40";
	mimeMap["doc"] = "application/msword";
	mimeMap["pdf"] = "application/pdf";
	mimeMap["ps"] = "application/postscript";
	mimeMap["eps"] = "application/postscript";
	mimeMap["ai"] = "application/postscript";
	mimeMap["rtf"] = "application/rtf";
	mimeMap["m3u8"] = "application/vnd.apple.mpegurl";
	mimeMap["kml"] = "application/vnd.google-earth.kml+xml";
	mimeMap["kmz"] = "application/vnd.google-earth.kmz";
	mimeMap["xls"] = "application/vnd.ms-excel";
	mimeMap["eot"] = "application/vnd.ms-fontobject";
	mimeMap["ppt"] = "application/vnd.ms-powerpoint";
	mimeMap["odg"] = "application/vnd.oasis.opendocument.graphics";
	mimeMap["odp"] = "application/vnd.oasis.opendocument.presentation";
	mimeMap["ods"] = "application/vnd.oasis.opendocument.spreadsheet";
	mimeMap["odt"] = "application/vnd.oasis.opendocument.text";
	mimeMap["pptx"] = "application/vnd.openxmlformats-officedocument.presentationml.presentation";
	mimeMap["xlsx"] = "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
	mimeMap["docx"] = "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
	mimeMap["wmlc"] = "application/vnd.wap.wmlc";
	mimeMap["7z"] = "application/x-7z-compressed";
	mimeMap["cco"] = "application/x-cocoa";
	mimeMap["jardiff"] = "application/x-java-archive-diff";
	mimeMap["jnlp"] = "application/x-java-jnlp-file";
	mimeMap["run"] = "application/x-makeself";
	mimeMap["pl"] = "application/x-perl";
	mimeMap["pm"] = "application/x-perl";
	mimeMap["prc"] = "application/x-pilot";
	mimeMap["pdb"] = "application/x-pilot";
	mimeMap["rar"] = "application/x-rar-compressed";
	mimeMap["rpm"] = "application/x-redhat-package-manager";
	mimeMap["sea"] = "application/x-sea";
	mimeMap["swf"] = "application/x-shockwave-flash";
	mimeMap["sit"] = "application/x-stuffit";
	mimeMap["tcl"] = "application/x-tcl";
	mimeMap["tk"] = "application/x-tcl";
	mimeMap["der"] = "application/x-x509-ca-cert";
	mimeMap["pem"] = "application/x-x509-ca-cert";
	mimeMap["crt"] = "application/x-x509-ca-cert";
	mimeMap["xpi"] = "application/x-xpinstall";
	mimeMap["xhtml"] = "application/xhtml+xml";
	mimeMap["xspf"] = "application/xspf+xml";
	mimeMap["zip"] = "application/zip";
	mimeMap["bin"] = "application/octet-stream";
	mimeMap["exe"] = "application/octet-stream";
	mimeMap["dll"] = "application/octet-stream";
	mimeMap["deb"] = "application/octet-stream";
	mimeMap["dmg"] = "application/octet-stream";
	mimeMap["iso"] = "application/octet-stream";
	mimeMap["img"] = "application/octet-stream";
	mimeMap["msi"] = "application/octet-stream";
	mimeMap["msp"] = "application/octet-stream";
	mimeMap["msm"] = "application/octet-stream";
	mimeMap["mid"] = "audio/midi";
	mimeMap["midi"] = "audio/midi";
	mimeMap["kar"] = "audio/midi";
	mimeMap["mp3"] = "audio/mpeg";
	mimeMap["ogg"] = "audio/ogg";
	mimeMap["m4a"] = "audio/x-m4a";
	mimeMap["ra"] = "audio/x-realaudio";
	mimeMap["3gpp"] = "video/3gpp";
	mimeMap["3gp"] = "video/3gpp";
	mimeMap["ts"] = "video/mp2t";
	mimeMap["mp4"] = "video/mp4";
	mimeMap["mpeg"] = "video/mpeg";
	mimeMap["mpg"] = "video/mpeg";
	mimeMap["mov"] = "video/quicktime";
	mimeMap["webm"] = "video/webm";
	mimeMap["flv"] = "video/x-flv";
	mimeMap["m4v"] = "video/x-m4v";
	mimeMap["mng"] = "video/x-mng";
	mimeMap["asx"] = "video/x-ms-asf";
	mimeMap["asf"] = "video/x-ms-asf";
	mimeMap["wmv"] = "video/x-ms-wmv";
	mimeMap["avi"] = "video/x-msvideo";
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

std::string ResourceHost::getBaseDiskPath(void) const
{
	return (this->baseDiskPath);
}