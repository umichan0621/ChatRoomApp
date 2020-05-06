#pragma once
#include "ChatRoom.h"
class Server
{
public:
	bool startServer(unsigned int port);//启动服务器
	bool epollStart();//启动epoll
	~Server();

private:
	static int chatRoomSeq;
	sockaddr_in serverAddr;//服务端地址族、端口、ip地址，用来处理通信地址	
	sockaddr_in clientAddr;//客户端地址族、端口、ip地址，用来处理通信地址
	socklen_t addrLen;
	int serverSocketFD;//服务器端socket套接字描述符
	int epollFD;//epoll描述符
	std::list<ChatRoom*> roomList;//用于存放ChatRoom的链表
	std::mutex roomListLock;
	std::unordered_map<int, ChatRoom*> mapOfUserChatRoom;
	std::mutex mapOfUserChatRoomLock;
	void addEvent(int fd, int state);
	void deleteEvent(int fd, int state);
	void changeEvent(int fd, int state);
	void handleEvents(epoll_event* events, int num);//处理新的事件
	void handleNewConnection();//处理新的事件：新连接
	void recvClientMessage(int fd);
	bool showChatRoomInfo(int userNo);//向客户端发送ChatRoom的信息	
	void createNewChatRoom(int userNo, std::string& clientReply);
	bool addUserToChatRoom(int userNo, int roomNo, std::string& clientReply);

};