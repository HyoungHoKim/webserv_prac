#include "Server.hpp"
#include "Config.hpp"
#include <pthread.h>

void *startServer(void *arg)
{
	ServerConfig *temp = reinterpret_cast<ServerConfig*>(arg);
	Server server(*temp);
	if (!server.init_Server())
		return (NULL);
	server.run();
	server.stop_server();
	return (NULL);
}

int main(int argc, char *argv[])
{
	// config 파일 파싱
	try
	{	
		Config conf;
		std::vector<ServerConfig> servers;
		if (argc != 2)
			exit(1);
		conf.parseConfig(argv[1]);
		servers = conf.getServers();
		if (servers.size() == 1)
		{
			Server server(servers[0]);

			if (!server.init_Server())
				return (-1);
			
			server.run();
			server.stop_server();
		}
		else
		{
			std::vector<pthread_t> t_ids;
			for (size_t i = 0; i < servers.size(); i++)
			{
				pthread_t t_id;
				pthread_create(&t_id, NULL, startServer, reinterpret_cast<void*>(&servers[i]));
				t_ids.push_back(t_id);
			}

			for (size_t i = 0; i < servers.size(); i++)
				pthread_join(t_ids[i], NULL);
		}
	}
	catch (std::exception &e)
	{
		std::cout << e.what() << std::endl;
	}
}