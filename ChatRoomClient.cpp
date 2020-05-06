#include "ChatRoomClient.h"

ChatRoomClient *ChatRoomClient::instance=0;

ChatRoomClient::ChatRoomClient()
{
    this->client=new QTcpSocket();
    client->abort();
}

ChatRoomClient *ChatRoomClient::getClient()
{
    if(instance==0)
        instance=new ChatRoomClient;
    return instance;
}
