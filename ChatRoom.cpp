#include "ChatRoom.h"

ChatRoom::ChatRoom(std::string roomName) { this->roomName = roomName; }

void ChatRoom::addUser(int userNo, std::string& userName)
{
	/*
	1.将新User加入userList（写操作，上锁）
	2.userCounter++（写操作，上锁）
	3.通知所有User新User加入ChatRoom的事情
	*/

	char* p = (char*)malloc(userName.size() * sizeof(char));
	memset(p, 0, userName.size() * sizeof(char));
	strcpy(p, userName.c_str());

	userListLock.lock();
	userList.insert(std::make_pair(userNo, p));
	userListLock.unlock();

	userCounterLock.lock();
	userCounter++;
	userCounterLock.unlock();

	std::string message = "/02" + userName + "||";

	std::thread* messageSender = new std::thread(&ChatRoom::sendMessageToUsers, this, message);
	messageSender->detach();
}

void ChatRoom::deleteUser(int userNo)
{
	/*
	1.找到userList中userNo符合的User
	2.删除User（写操作，上锁）
	3.
	a.如果聊天室内没有User，则解散
	b.否则向其他User告知该User退出ChatRoom
	*/
	std::string userName;
	std::unordered_map<int, char*>::iterator it;
	userListLock.lock();
	it = userList.find(userNo);
	if (it != userList.end())
	{
		char* p = (*it).second;
		userName = p;
		if (p != nullptr)
		{
			free(p);
			p = nullptr;
		}
	}
	//删除该user		
	userList.erase(it);
	userListLock.unlock();

	std::cout << userName << " has disconnected." << std::endl;
	//发送消息给所有的用户		
	std::string message = "/03" + userName + "||";


	sigset_t signal_mask;
	sigemptyset(&signal_mask);
	sigaddset(&signal_mask, SIGPIPE);
	int rc = pthread_sigmask(SIG_BLOCK, &signal_mask, NULL);
	if (rc != 0) {
		printf("block sigpipe error\n");
	}
	
	std::thread* messageSender = new std::thread(&ChatRoom::sendMessageToUsers, this, message);
	messageSender->detach();

	//user总数-1
	userCounterLock.lock();
	userCounter--;
	userCounterLock.unlock();
}

void ChatRoom::sendMessageToUsers(std::string message)
{
	/*
	1.向所有聊天室内的User发送消息
	2.只转发，不管客户端是否收到，也不做判断
	*/
	userListLock.lock();
	for (auto user : userList)
	{
		int userNo = user.first;
		
		sendMessage(userNo, message);
	}
	userListLock.unlock();
}

bool ChatRoom::showUserListInfo(int userNo)
{
	std::string message;
	userListLock.lock();
	for (auto user : userList)
	{
		if (user.first != userNo)
		{
			char* userName = user.second;

			message = "/04";
			message = message +userName + "||";
			sendMessage(userNo, message);
		}
	}
	userListLock.unlock();
	return true;
}

void sendMessage(int userNo, std::string& message)
{
	//只管发，不管收的到收不到

	//string转char[]
	char buffer[BUFFERSIZE];
	message.copy(buffer, message.size());
	buffer[message.size() + 1] = '\0';
	struct timeval timeout = { 3,0 };
	setsockopt(userNo, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(struct timeval));
	send(userNo, buffer, message.size(), 0);

}

