#include "Server.h"

void sendMessage(int userNo, std::string& message);
int Server::chatRoomSeq = 10000000;


Server::~Server()
{
	//关闭socket套接字描述符
	close(serverSocketFD);
}

bool Server::startServer(unsigned int port)
{
	addrLen = sizeof(clientAddr);
	//若成功则返回一个sockfd (套接字描述符)
	serverSocketFD = socket(AF_INET, SOCK_STREAM, 0);
	//下面设置sockaddr_in 结构体中相关参数
	serverAddr.sin_family = AF_INET;
	//将一个无符号短整型数值转换为网络字节序，即大端模式
	serverAddr.sin_port = htons(port);
	//INADDR_ANY就是指定地址为0.0.0.0的地址，表示不确定地址，或"所有地址"、“任意地址”。
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	//绑定IP地址端口信息
	if(bind(serverSocketFD, (struct sockaddr*) & serverAddr, sizeof(serverAddr))==-1)
		return false;
	//监听Socket端口
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
		//获取已经准备好的描述符事件
		/*
		如果要设置read超时
		1.设置socket非阻塞
		2.设置epoll_wait超时1秒
		3.每次进入epoll_wait之前，遍历在线用户列表，踢出长时间没有请求的用户
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
	//如果是ET模式，设置EPOLLET
	ev.events |= EPOLLET;
	//设置是否阻塞
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
		//表示有新的连接
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
	//表示有新的客户端连接，接受之后设定读取该客户端消息
	if (clientSocketFD == -1)
		return;

	std::cout << "New Connection" << std::endl;

	//发送ChatRoomList
	std::thread* roomInfoSender = new std::thread(&Server::showChatRoomInfo, this, clientSocketFD);
	roomInfoSender->detach();

	//添加客户端描述符为读消息
	addEvent(clientSocketFD, EPOLLIN);
}

void Server::recvClientMessage(int fd)
{
	char buffer[BUFFERSIZE];
	ssize_t returnVal = -1;
	memset(buffer, 0, sizeof(buffer));
	returnVal = recv(fd, buffer, sizeof(buffer), 0);
	if (returnVal > 0)//正常消息
	{
		if (mapOfUserChatRoom.find(fd) == mapOfUserChatRoom.end())//新的用户
		{
			std::string clientReply(buffer);
			std::string tp = clientReply.substr(0, 8);
			//表示创建新的ChatRoom
			if (tp == "00000000")
			{
				createNewChatRoom(fd, clientReply);
				return;
			}
			int roomNo = atoi(tp.c_str());//转成数字
			if (roomNo < 10000000 || roomNo>20000000)//非法连接
			{
				deleteEvent(fd, EPOLLHUP);
				return;
			}
			addUserToChatRoom( fd, roomNo, clientReply);
			return;
		}
		else//转发用户的消息
		{
			ChatRoom* CR = mapOfUserChatRoom[fd];
			std::string tp(buffer);

			std::thread* messageSender = new std::thread(&ChatRoom::sendMessageToUsers, CR, tp);
			messageSender->detach();
		}		
	}
	else if (returnVal == -1)//异常消息，无视
		return;
	else if (returnVal == 0)//客户端断开了连接
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
		//删除映射		
		mapOfUserChatRoom.erase(it);
		mapOfUserChatRoomLock.unlock();
		CR->deleteUser(fd);
		deleteEvent(fd, EPOLLHUP);
		//删除用户后，ChatRoom内没有用户，则解散
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
	//发送ChatRoom列表给客户端
	//return false表示非法连接，直接断开
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

	//发送roomList信息
	ssize_t returnVal = send(userNo, buffer, sizeof(buffer), 0);

	//发送失败
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
	//同意新user加入ChatRoom
	std::string ok = "00000000";
	sendMessage(userNo, ok);
	ssize_t pos = clientReply.find("|");
	std::string roomName = clientReply.substr(8, pos - 8);
	std::string userName = clientReply.substr(pos + 1, clientReply.size() - pos);
	//创建新的ChatRoom
	chatRoomSeq++;
	ChatRoom* CR = new ChatRoom(roomName);
	CR->roomNo = chatRoomSeq;

	std::cout << userName << " create " << roomName << std::endl;
	roomListLock.lock();
	roomList.push_back(CR);
	roomListLock.unlock();
	//为ChatRoom添加新User
	CR->addUser(userNo, userName);

	//为User-ChatRoom映射表添加数据
	mapOfUserChatRoomLock.lock();
	mapOfUserChatRoom.insert(std::make_pair(userNo, CR));
	mapOfUserChatRoomLock.unlock();
}

bool Server::addUserToChatRoom(int userNo,int roomNo, std::string& clientReply)
{
	//用户请求加入roomNo的ChatRoom
	for (auto it : roomList)
	{
		if (it->roomNo == roomNo)
		{
			ssize_t pos = clientReply.find("|");
			std::string userName = clientReply.substr(pos + 1, clientReply.size() - pos);
			//同意新user加入ChatRoom
			std::string ok = "00000000";
			sendMessage(userNo, ok);

			
			//向新User发送UserList		
			it->addUser(userNo, userName);

			it->showUserListInfo(userNo);

			std::cout << userName << " enter " << it->roomName << std::endl;

			//为User-ChatRoom映射表添加数据
			mapOfUserChatRoomLock.lock();
			mapOfUserChatRoom.insert(std::make_pair(userNo, it));
			mapOfUserChatRoomLock.unlock();
			return true;
		}
	}
	return false;
}


