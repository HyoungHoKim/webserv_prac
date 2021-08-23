#include "Server.hpp"

#include <pthread.h>

void *startServer(void *arg)
{
	Server server;
	server.init_Server(atoi((char*)arg));
	server.run();
	return (NULL);
}

int main(int argc, char *argv[])
{
	if (argc == 1)
	{
		Server server;

		if (!server.init_Server(8000))
			return (-1);

		server.run();
	}
	else
	{
		vector<pthread_t> t_ids;
		for (int i = 1; i < argc; i++)
		{
			pthread_t t_id;
			t_ids.push_back(t_id);
			pthread_create(&t_id, NULL, startServer, (void*)argv[i]);
		}

		for (int i = 1; i < argc; i++)
			pthread_join(t_ids[i], NULL);
	}
}