#pragma once
#include "ChatRoom.h"
class Server
{
public:
	bool startServer(unsigned int port);//����������
	bool epollStart();//����epoll
	~Server();

private:
	static int chatRoomSeq;
	sockaddr_in serverAddr;//����˵�ַ�塢�˿ڡ�ip��ַ����������ͨ�ŵ�ַ	
	sockaddr_in clientAddr;//�ͻ��˵�ַ�塢�˿ڡ�ip��ַ����������ͨ�ŵ�ַ
	socklen_t addrLen;
	int serverSocketFD;//��������socket�׽���������
	int epollFD;//epoll������
	std::list<ChatRoom*> roomList;//���ڴ��ChatRoom������
	std::mutex roomListLock;
	std::unordered_map<int, ChatRoom*> mapOfUserChatRoom;
	std::mutex mapOfUserChatRoomLock;
	void addEvent(int fd, int state);
	void deleteEvent(int fd, int state);
	void changeEvent(int fd, int state);
	void handleEvents(epoll_event* events, int num);//�����µ��¼�
	void handleNewConnection();//�����µ��¼���������
	void recvClientMessage(int fd);
	bool showChatRoomInfo(int userNo);//��ͻ��˷���ChatRoom����Ϣ	
	void createNewChatRoom(int userNo, std::string& clientReply);
	bool addUserToChatRoom(int userNo, int roomNo, std::string& clientReply);

};