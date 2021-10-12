#include "Server.hpp"
#include "Config.hpp"
#include <pthread.h>

std::vector<ServerConfig> g_servers;

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
		if (argc != 2)
			exit(1);
		conf.parseConfig(argv[1]);
		g_servers = conf.getServers();
		if (g_servers.size() == 1)
		{
			Server server(g_servers[0]);

			if (!server.init_Server())
				return (-1);
			
			server.run();
			server.stop_server();
		}
		else
		{
			std::vector<pthread_t> t_ids;
			for (size_t i = 0; i < g_servers.size(); i++)
			{
				pthread_t t_id;
				pthread_create(&t_id, NULL, startServer, reinterpret_cast<void*>(&g_servers[i]));
				t_ids.push_back(t_id);
			}

			for (int i = 1; i < argc; i++)
				pthread_join(t_ids[i], NULL);
		}
	}
	catch (std::exception &e)
	{
		std::cout << e.what() << std::endl;
	}
}