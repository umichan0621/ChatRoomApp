#include "ChatRoom.h"

ChatRoom::ChatRoom(std::string roomName) { this->roomName = roomName; }

void ChatRoom::addUser(int userNo, std::string& userName)
{
	/*
	1.����User����userList��д������������
	2.userCounter++��д������������
	3.֪ͨ����User��User����ChatRoom������
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
	1.�ҵ�userList��userNo���ϵ�User
	2.ɾ��User��д������������
	3.
	a.�����������û��User�����ɢ
	b.����������User��֪��User�˳�ChatRoom
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
	//ɾ����user		
	userList.erase(it);
	userListLock.unlock();

	std::cout << userName << " has disconnected." << std::endl;
	//������Ϣ�����е��û�		
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

	//user����-1
	userCounterLock.lock();
	userCounter--;
	userCounterLock.unlock();
}

void ChatRoom::sendMessageToUsers(std::string message)
{
	/*
	1.�������������ڵ�User������Ϣ
	2.ֻת�������ܿͻ����Ƿ��յ���Ҳ�����ж�
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
	//ֻ�ܷ��������յĵ��ղ���

	//stringתchar[]
	char buffer[BUFFERSIZE];
	message.copy(buffer, message.size());
	buffer[message.size() + 1] = '\0';
	struct timeval timeout = { 3,0 };
	setsockopt(userNo, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(struct timeval));
	send(userNo, buffer, message.size(), 0);

}

