#include "Server.hpp"

#include <pthread.h>

void *startServer(void *arg)
{
	Server server(atoi((char*)arg), "./htdocs");
	server.init_Server();
	server.run();
	return (NULL);
}

int main(int argc, char *argv[])
{
	// config 파일 파싱
	
	

	if (argc == 1)
	{
		Server server(8000, "./htdocs");

		if (!server.init_Server())
			return (-1);

		server.run();
	}
	else if (argc == 2)
	{
		Server server(atoi(argv[1]), "./htdocs");

		if (!server.init_Server())
			return (-1);

		server.run();
	}
	else
	{
		std::vector<pthread_t> t_ids;
		for (int i = 1; i < argc; i++)
		{
			pthread_t t_id;
			pthread_create(&t_id, NULL, startServer, (void*)argv[i]);
			t_ids.push_back(t_id);
		}

		for (int i = 1; i < argc; i++)
			pthread_join(t_ids[i], NULL);
	}
}