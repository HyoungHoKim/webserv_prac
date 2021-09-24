#ifndef SERVERCONFIG_HPP
# define SERVERCONFIG_HPP
# include <iostream>
# include <fstream>
# include <string>
# include <map>
# include <exception>
# include <vector>
# include "libft.hpp"

class	ServerConfig
{
	private:
		bool	isValidDirective(std::string temp);
		bool	autoindex;
		int		id;
		std::string listen;
		std::vector<ServerConfig> locations;
		std::string root;
		std::string error;
		std::string cgi_ext;
		std::string exec;
		size_t		max_body_size;
		std::vector<std::string> method;
		std::vector<std::string> index;
		std::vector<std::string> ServerName;
		std::string uri;

	public:
		ServerConfig();
		~ServerConfig();

		void		setID(int _id);
		int			getID() const;
		bool		getAutoindex() const;
		std::string	getListen() const;
		std::vector<ServerConfig> getLocations() const;
		std::string getExec() const;
		std::string getCgiExt() const;
		std::string getRoot();
		std::string getError() const;
		std::string getUri() const;
		size_t		getClientMaxBodySize() const;
		std::vector<std::string> getMethod() const;
		std::vector<std::string> getIndex();
		std::vector<std::string> getServerName() const;
		void		initServer(std::vector<std::string>::iterator &it);
		void		getDirective(std::vector<std::string>::iterator &it);
		void		parseListen(std::vector<std::string>::iterator &it);
		void		parseLocations(std::vector<std::string>::iterator &it);
		void		parseRoot(std::vector<std::string>::iterator &it);
		void		parseMethod(std::vector<std::string>::iterator &it);
		void		parseIndex(std::vector<std::string>::iterator &it);
		void		parseAutoindex(std::vector<std::string>::iterator &it);
		void		parseServerName(std::vector<std::string>::iterator &it);
		void		parseClientMaxBodySize(std::vector<std::string>::iterator &it);
		void		parseCGI(std::vector<std::string>::iterator &it);
		void		parseExec(std::vector<std::string>::iterator &it);
		void		loopLocation(std::vector<std::string>::iterator &it, std::vector<ServerConfig> &locations);
		class	errorInConfig : public std::exception
		{
			public:
				virtual const char* what() const throw();
		};
};
#endif

