#include "Server.h"

void sendMessage(int userNo, std::string& message);
int Server::chatRoomSeq = 10000000;


Server::~Server()
{
	//�ر�socket�׽���������
	close(serverSocketFD);
}

bool Server::startServer(unsigned int port)
{
	addrLen = sizeof(clientAddr);
	//���ɹ��򷵻�һ��sockfd (�׽���������)
	serverSocketFD = socket(AF_INET, SOCK_STREAM, 0);
	//��������sockaddr_in �ṹ������ز���
	serverAddr.sin_family = AF_INET;
	//��һ���޷��Ŷ�������ֵת��Ϊ�����ֽ��򣬼����ģʽ
	serverAddr.sin_port = htons(port);
	//INADDR_ANY����ָ����ַΪ0.0.0.0�ĵ�ַ����ʾ��ȷ����ַ����"���е�ַ"���������ַ����
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	//��IP��ַ�˿���Ϣ
	if(bind(serverSocketFD, (struct sockaddr*) & serverAddr, sizeof(serverAddr))==-1)
		return false;
	//����Socket�˿�
	if (listen(serverSocketFD, QUEUE) == -1)
		return false;
	return true;
}

bool Server::epollStart()
{
	struct epoll_event events[MAXEVENTS];
	int retVal;
	if ((epollFD = epoll_create(FDSIZE)) == -1)
	{
		std::cout << "Fali to start epoll" << std::endl;
		return false;
	}
	addEvent(serverSocketFD, EPOLLIN);
	while (1)
	{
		//��ȡ�Ѿ�׼���õ��������¼�
		/*
		���Ҫ����read��ʱ
		1.����socket������
		2.����epoll_wait��ʱ1��
		3.ÿ�ν���epoll_wait֮ǰ�����������û��б��߳���ʱ��û��������û�
		*/
		retVal= epoll_wait(epollFD, events, MAXEVENTS, -1);
		handleEvents(events, retVal);
	}
	close(epollFD);

}

void Server::addEvent(int fd, int state)
{
	struct epoll_event ev;
	ev.events = state;
	ev.data.fd = fd;
	/*
	//�����ETģʽ������EPOLLET
	ev.events |= EPOLLET;
	//�����Ƿ�����
	int flags = fcntl(fd, F_GETFL);
	fcntl(fd, F_SETFL, flags | O_NONBLOCK);
	*/
	epoll_ctl(epollFD, EPOLL_CTL_ADD, fd, &ev);
}

void Server::deleteEvent(int fd, int state)
{
	struct epoll_event ev;
	ev.events = state;
	ev.data.fd = fd;
	epoll_ctl(epollFD, EPOLL_CTL_DEL, fd, &ev);
}

void Server::changeEvent(int fd, int state)
{
	struct epoll_event ev;
	ev.events = state;
	ev.data.fd = fd;
	epoll_ctl(epollFD, EPOLL_CTL_MOD, fd, &ev);
}

void Server::handleEvents(epoll_event* events, int num)
{
	int fd;
	for (int i = 0; i < num; i++)
	{
		fd = events[i].data.fd;
		//��ʾ���µ�����
		if((fd==serverSocketFD)&&events[i].events&EPOLLIN)
			handleNewConnection();
		else if (events[i].events & EPOLLIN)
		{
			std::thread* messageRecver = new std::thread(&Server::recvClientMessage, this, fd);
			messageRecver->detach();
		}
			
		else
			close(fd);
	}
}

void Server::handleNewConnection()
{
	int clientSocketFD;
	clientSocketFD=accept(serverSocketFD, (struct sockaddr*) & clientAddr, &addrLen);
	//��ʾ���µĿͻ������ӣ�����֮���趨��ȡ�ÿͻ�����Ϣ
	if (clientSocketFD == -1)
		return;

	std::cout << "New Connection" << std::endl;

	//����ChatRoomList
	std::thread* roomInfoSender = new std::thread(&Server::showChatRoomInfo, this, clientSocketFD);
	roomInfoSender->detach();

	//��ӿͻ���������Ϊ����Ϣ
	addEvent(clientSocketFD, EPOLLIN);
}

void Server::recvClientMessage(int fd)
{
	char buffer[BUFFERSIZE];
	ssize_t returnVal = -1;
	memset(buffer, 0, sizeof(buffer));
	returnVal = recv(fd, buffer, sizeof(buffer), 0);
	if (returnVal > 0)//������Ϣ
	{
		if (mapOfUserChatRoom.find(fd) == mapOfUserChatRoom.end())//�µ��û�
		{
			std::string clientReply(buffer);
			std::string tp = clientReply.substr(0, 8);
			//��ʾ�����µ�ChatRoom
			if (tp == "00000000")
			{
				createNewChatRoom(fd, clientReply);
				return;
			}
			int roomNo = atoi(tp.c_str());//ת������
			if (roomNo < 10000000 || roomNo>20000000)//�Ƿ�����
			{
				deleteEvent(fd, EPOLLHUP);
				return;
			}
			addUserToChatRoom( fd, roomNo, clientReply);
			return;
		}
		else//ת���û�����Ϣ
		{
			ChatRoom* CR = mapOfUserChatRoom[fd];
			std::string tp(buffer);

			std::thread* messageSender = new std::thread(&ChatRoom::sendMessageToUsers, CR, tp);
			messageSender->detach();
		}		
	}
	else if (returnVal == -1)//�쳣��Ϣ������
		return;
	else if (returnVal == 0)//�ͻ��˶Ͽ�������
	{
		bool flag = true;
		mapOfUserChatRoomLock.lock();
		std::unordered_map<int, ChatRoom*>::iterator it = mapOfUserChatRoom.find(fd);
		if (it == mapOfUserChatRoom.end())
			flag = false;
		mapOfUserChatRoomLock.unlock();
		if(flag == false)
		{
			deleteEvent(fd, EPOLLHUP);
			return;
		}
		mapOfUserChatRoomLock.lock();
		ChatRoom* CR = mapOfUserChatRoom[fd];
		//ɾ��ӳ��		
		mapOfUserChatRoom.erase(it);
		mapOfUserChatRoomLock.unlock();
		CR->deleteUser(fd);
		deleteEvent(fd, EPOLLHUP);
		//ɾ���û���ChatRoom��û���û������ɢ
		if (CR->userCounter == 0)
		{
			std::list<ChatRoom*>::iterator it;
			roomListLock.lock();
			for (it = roomList.begin(); it != roomList.end(); it++)
			{			
				if ((*it)->roomNo == CR->roomNo)
				{		
					std::cout<< "ChatRoom No."<< CR->roomNo <<" dismiss." << std::endl;	
					roomList.erase(it);		
					break;
				}
			}
			roomListLock.unlock();
			sleep(5);
			delete CR;
			
		}
		
	}
}

bool Server::showChatRoomInfo(int userNo)
{
	//����ChatRoom�б���ͻ���
	//return false��ʾ�Ƿ����ӣ�ֱ�ӶϿ�
	std::string message = "";
	roomListLock.lock();
	for(auto CR:this->roomList)
		message=message+ std::to_string(CR->roomNo)+CR->roomName+"|";
	roomListLock.unlock();

	if (message.size() == 0)
		message = "NULL";


	char buffer[BUFFERSIZE];
	message.copy(buffer, message.size());
	buffer[message.size() + 1] = '\0';

	//����roomList��Ϣ
	ssize_t returnVal = send(userNo, buffer, sizeof(buffer), 0);

	//����ʧ��
	if (returnVal == -1)
	{
		std::cout << "Illegal Connection" << std::endl;
		close(userNo);
		return false;
	}
		
	return true;
}

void Server::createNewChatRoom(int userNo,std::string& clientReply)
{
	//ͬ����user����ChatRoom
	std::string ok = "00000000";
	sendMessage(userNo, ok);
	ssize_t pos = clientReply.find("|");
	std::string roomName = clientReply.substr(8, pos - 8);
	std::string userName = clientReply.substr(pos + 1, clientReply.size() - pos);
	//�����µ�ChatRoom
	chatRoomSeq++;
	ChatRoom* CR = new ChatRoom(roomName);
	CR->roomNo = chatRoomSeq;

	std::cout << userName << " create " << roomName << std::endl;
	roomListLock.lock();
	roomList.push_back(CR);
	roomListLock.unlock();
	//ΪChatRoom�����User
	CR->addUser(userNo, userName);

	//ΪUser-ChatRoomӳ����������
	mapOfUserChatRoomLock.lock();
	mapOfUserChatRoom.insert(std::make_pair(userNo, CR));
	mapOfUserChatRoomLock.unlock();
}

bool Server::addUserToChatRoom(int userNo,int roomNo, std::string& clientReply)
{
	//�û��������roomNo��ChatRoom
	for (auto it : roomList)
	{
		if (it->roomNo == roomNo)
		{
			ssize_t pos = clientReply.find("|");
			std::string userName = clientReply.substr(pos + 1, clientReply.size() - pos);
			//ͬ����user����ChatRoom
			std::string ok = "00000000";
			sendMessage(userNo, ok);

			
			//����User����UserList		
			it->addUser(userNo, userName);

			it->showUserListInfo(userNo);

			std::cout << userName << " enter " << it->roomName << std::endl;

			//ΪUser-ChatRoomӳ����������
			mapOfUserChatRoomLock.lock();
			mapOfUserChatRoom.insert(std::make_pair(userNo, it));
			mapOfUserChatRoomLock.unlock();
			return true;
		}
	}
	return false;
}


