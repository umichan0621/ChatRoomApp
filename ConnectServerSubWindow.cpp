#include "ConnectServerSubWindow.h"
#include "ui_ConnectServerSubWindow.h"
#include "ChatRoomClient.h"



ConnectServerSubWindow::ConnectServerSubWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ConnectServerSubWindow)
{
    ui->setupUi(this);
    QFont font ("Microsoft YaHei", 15, 75);
    ui->label->setFont(font);
    ui->ServerIPAddress->setText("175.24.16.186");
    ui->ServerPort->setText("6000");
    this->setFixedSize(400,280);
    this->setWindowTitle("ChatRoom");
    this->setWindowIcon(QPixmap(":Image/icon.png"));

    //connect连接服务器Button
    connect (ui->ConnectServer,SIGNAL(clicked()),this,SLOT(connectServer()));

    timer=new QTimer(this);
    //连接服务器后，启动定时器，接受RoomList
    connect(timer,SIGNAL(timeout()),this,SLOT(recvAndshowRoomList()));
}

void ConnectServerSubWindow::connectServer()
{
    //连接服务器

    //获取输入的端口和服务器
    quint16 ServerPort=ui->ServerPort->text().toInt();
    QString ServerIPAddress=ui->ServerIPAddress->text();
    //连接服务器
    ChatRoomClient::getClient()->client->
            connectToHost(ServerIPAddress,ServerPort);

    ui->ConnectStatic->setText("正在连接服务器...");
    //启动定时器
    timer->start(100);
}

void ConnectServerSubWindow::recvAndshowRoomList()
{
    //获取服务器发送的RoomList

    static int counter=0;
    QString buffer=ChatRoomClient::getClient()->client->readAll();
    if(buffer.size()!=0)
    {
        roomList=buffer;
        timer->stop();
        this->hide();
        emit this->connectedToServer();
    }
    counter++;
    if(counter==50)//5秒后没有收到信息
    {
        //此时连接服务器失败
        ui->ConnectStatic->setText("连接服务器失败，请查看IP和端口是否正确");
        qDebug()<<"Connect Fail";
        timer->stop();
    }
}

ConnectServerSubWindow::~ConnectServerSubWindow()
{

    delete ui;
}
