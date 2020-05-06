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

#define QUEUE 20  //�����������
#define FDSIZE 1000
#define MAXEVENTS 500
#define BUFFERSIZE 10240//Socketͨ���շ���Ϣ��������С

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
	bool showUserListInfo(int userNo);//��ͻ��˷�������User�б�
};

void sendMessage(int userNo, std::string& message);