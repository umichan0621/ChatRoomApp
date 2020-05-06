#include "ChatRoomListWidget.h"
#include "ui_ChatRoomListWidget.h"

ChatRoomListWidget::ChatRoomListWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChatRoomListWidget)
{
    ui->setupUi(this);

    ui->userName->setText("anonym");
    ui->ChatRoomNoOrName->setText("ChatRoom01");

    ui->label_3->setFont(QFont("Microsoft YaHei", 14, 50));
    ui->ChatRoomList->setFont(QFont("Microsoft YaHei", 12, 50));

    timer=new QTimer(this);

    //连接 创建ChatRoom
    connect(ui->CreateChatRoom,SIGNAL(clicked()),this,SLOT(createChatRoom()));
    connect(ui->AddChatRoom,SIGNAL(clicked()),this,SLOT(enterChatRoom()));

    connect(timer,SIGNAL(timeout()),this,SLOT(inChatRoom()));


}

ChatRoomListWidget::~ChatRoomListWidget()
{
    delete ui;
}

void ChatRoomListWidget::setRoomList(QString roomList)
{
    if (roomList=="NULL")
    {
        ui->ChatRoomList->setText("当前没有在线的ChatRoom");
        return;
    }
    while(roomList.size()!=0)
    {
        int pos=roomList.indexOf("|");
        QString tp=roomList.left(pos);
        roomList=roomList.mid(pos+1);
        QString roomName=tp.mid(8);
        tp=tp.left(8);

        int roomNo=tp.toInt()-10000000;
        ChatRoomClient::getClient()->ChatRoomList.insert(roomNo,roomName);
    }
    QMap<int,QString>::iterator it;
    for(it=ChatRoomClient::getClient()->ChatRoomList.begin();
        it!=ChatRoomClient::getClient()->ChatRoomList.end();it++)
    {   //显示ChatRoom信息
        QString tp="ChatRoom名:"+it.value()+"(编号"+QString::number(it.key())+")\n";
        ui->ChatRoomList->insertPlainText(tp);
    }
}

void ChatRoomListWidget::createChatRoom()
{
    QString userName=ui->userName->text();
    QString ChatRoomName=ui->ChatRoomNoOrName->text();
    ChatRoomClient::getClient()->roomName=ChatRoomName;
    ChatRoomClient::getClient()->userName=userName;

    QString tp="00000000"+ChatRoomName+"|"+userName;
    QByteArray message="";
    message+=tp;
    ChatRoomClient::getClient()->client->write(message);
    //将新用户名加入显示的用户列表
    ChatRoomClient::getClient()->userList.push_back
            (ChatRoomClient::getClient()->userName);

    timer->start(100);
}

void ChatRoomListWidget::enterChatRoom()
{
    QString userName=ui->userName->text();
    int roomNo=ui->ChatRoomNoOrName->text().toInt()+10000000;
    QString ChatRoomNo=QString::number(roomNo);
    QString tp=ChatRoomNo+"|"+userName;
    ChatRoomClient::getClient()->roomName=
            ChatRoomClient::getClient()->ChatRoomList[roomNo-10000000];
    ChatRoomClient::getClient()->userName=userName;

    QByteArray message="";
    message+=tp;
    ChatRoomClient::getClient()->client->write(message);
    timer->start(100);
}

void ChatRoomListWidget::inChatRoom()
{
    QString labelText=ChatRoomClient::getClient()->roomName;
    labelText="正在加入["+labelText+"]...";
    ui->staticLabel->setText(labelText);
    static int counter=0;   
    QString buffer=ChatRoomClient::getClient()->client->readAll();

    if(buffer.left(8)=="00000000")
    {
        timer->stop();       
        ChatRoomClient::getClient()->serverMessage=buffer.mid(8);
        this->hide();
        emit iminChatRoom();
    }
    counter++;
    if(counter==50)//5秒后没有收到信息
    {
        //此时连接服务器失败
        ui->staticLabel->setText("加入ChatRoom失败");
        timer->stop();
    }
}
