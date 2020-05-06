#pragma once
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h> 
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <list>
#include <thread>
#include <mutex>
#include <string.h>
#include <string>
#include <unordered_map>
#include <signal.h>

#define QUEUE 20  //连接请求队列
#define FDSIZE 1000
#define MAXEVENTS 500
#define BUFFERSIZE 10240//Socket通信收发信息缓存区大小

class ChatRoom
{
public:
	int roomNo;
	std::string roomName;
	int userCounter;
	std::mutex userCounterLock;
	std::unordered_map<int, char*> userList;

	std::mutex userListLock;
	ChatRoom(std::string roomName);
	void addUser(int userNo, std::string& userName);
	void deleteUser(int userNo);
	void sendMessageToUsers(std::string message);
	bool showUserListInfo(int userNo);//向客户端发送在线User列表
};

void sendMessage(int userNo, std::string& message);