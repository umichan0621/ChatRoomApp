#pragma once
#include <QTcpSocket>
#include <QList>
#include <QMap>
#include <QTime>

class ChatRoomClient
{
private:
    ChatRoomClient();
    static ChatRoomClient *instance;
public:
    static ChatRoomClient *getClient();
    QTcpSocket *client;
    QString roomName;
    QString userName;
    QString serverMessage;
    QList<QString> userList;
    QMap<int,QString> ChatRoomList;
};
