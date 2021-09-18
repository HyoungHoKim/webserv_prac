#ifndef CONFIG_HPP
# define CONFIG_HPP
# include <exception>
# include "libft.hpp"
# include "ServerConfig.hpp"

class Config
{
	public:
		typedef void	type;
	private:
		std::string readFile(char *file);
		std::vector<std::string> tokens;
		std::vector<ServerConfig> servers;

	public:
		Config();
		~Config();

		void	tokenize(std::string &buffer);
		void	parseConfig(char *file);
		std::vector<ServerConfig> getServers() const;
		class	errorInConfig : public std::exception
		{
			public:
				virtual const char* what() const throw();
		};
};
#endif