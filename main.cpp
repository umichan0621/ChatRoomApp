#include "Server.h"
#include "ChatRoom.h"

int main()
{
	Server* server = new Server;
	if (server->startServer(6000))
		std::cout << "Start server succeed..." << std::endl;
	else
		std::cout << "fail" << std::endl;
	server->epollStart();

	return 0;
}