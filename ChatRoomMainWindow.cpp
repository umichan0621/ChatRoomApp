#include "ChatRoomMainWindow.h"
#include "ui_ChatRoomMainWindow.h"



ChatRoomMainWindow::ChatRoomMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ChatRoomMainWindow)
{


    //初始化MainWindow
    ui->setupUi(this);
    this->setWindowIcon(QPixmap(":Image/icon.png"));
    this->setWindowTitle("ChatRoom");
    ui->plainTextEdit->setFixedHeight(100);
    ui->UserList->setFixedWidth(180);
    QFont font ("Microsoft YaHei", 14, 50);
    ui->TextBrowser->setFont(font);
    ui->UserList->setFont(font);
    ui->ChatRoomName->setFont(QFont("Microsoft YaHei", 20, 87));
    ui->UserListLabel->setFont(QFont("Microsoft YaHei", 20, 87));
    //创建ChatRoomList控件
    listWidget=new ChatRoomListWidget();
    listWidget->setParent(this);

    //创建子窗口
    subWindow=new ConnectServerSubWindow();
    //显示子窗口
    subWindow->show();
    //隐藏其他所有控件
    this->hide();
    ui->MainWindow->hide();
    listWidget->hide();

    timer=new QTimer(this);

    //连接上服务器，关闭子窗口，打开RoomList控件
    connect(subWindow,SIGNAL(connectedToServer()),this,SLOT(showTheRoomList()));
    //加入ChatRoom，关闭RoomList控件，打开主窗口控件
    connect(listWidget,SIGNAL(iminChatRoom()),this,SLOT(enterTheChatRoom()));
    //
    connect(ui->SendMessage,SIGNAL(clicked()),this,SLOT(sendMessage()));

    connect(timer,SIGNAL(timeout()),this,SLOT(recvMessage()));

}

ChatRoomMainWindow::~ChatRoomMainWindow()
{
    delete ui;
}

void ChatRoomMainWindow::keyPressEvent(QKeyEvent *ev)
{
    qDebug()<<ev->key();
    if ((ev->modifiers() == Qt::ControlModifier) && (ev->key() == Qt::Key_Return))
    {
        ui->plainTextEdit->appendPlainText("");
    }
    else if(ev->key()==Qt::Key_Enter|| ev->key() == Qt::Key_Return)
    {
        sendMessage();
    }
    else if(ev->key()==Qt::Key_Backspace)
    {
        qDebug()<<"cr enter";
        //得到当前text的光标
        QTextCursor cursor=ui->plainTextEdit->textCursor();
        if(cursor.hasSelection())//如果有选中，则取消，以免受受影响
            cursor.clearSelection();
        cursor.deletePreviousChar();//删除前一个字符
        ui->plainTextEdit->setTextCursor(cursor);//让光标移到删除后的位置
    }
}

void ChatRoomMainWindow::showTheRoomList()
{
    this->show();
    this->setFixedSize(450,560);
    listWidget->setFixedSize(450,560);
    listWidget->setRoomList(subWindow->roomList);
    listWidget->show();

}

void ChatRoomMainWindow::enterTheChatRoom()
{
    listWidget->hide();
    //调整窗口大小
    this->setFixedSize(720, 600);
    ui->MainWindow->setFixedSize(720, 600);
    //设置聊天室名字
    ui->ChatRoomName->setText(ChatRoomClient::getClient()->roomName);

    //如果没有用户
    if(ChatRoomClient::getClient()->userList.size()!=0)
    {
        QString tp=ChatRoomClient::getClient()->userName+" 已创建 "
                +ChatRoomClient::getClient()->roomName;
        QColor tpclrR(0,100,0);
        stringToHtml(tp,tpclrR);
        ui->TextBrowser->moveCursor(QTextCursor::End);
        ui->TextBrowser->textCursor().insertHtml(tp);
        ui->TextBrowser->textCursor().insertText("\n");
        ChatRoomClient::getClient()->userList.clear();
    }


    timer->start(100);
    ui->MainWindow->show();
    grabKeyboard();
}

void ChatRoomMainWindow::sendMessage()
{
    QTime now=QTime::currentTime();

    QString hour=QString::number(now.hour());
    if(now.hour()<10)
        hour="0"+hour;
    QString minute=QString::number(now.minute());
    if(now.minute()<10)
        minute="0"+minute;
    QString second=QString::number(now.second());
    if(now.second()<10)
        second="0"+second;
    QString currentTime=" "+hour+":"+minute+":"+second;

    QString message=ui->plainTextEdit->toPlainText();
    //消息格式
    //消息类型(/01) 用户名+时间 分隔 消息
    message="/01"+ChatRoomClient::getClient()->userName+currentTime+"|"+message+"||";
    QByteArray tp="";
    tp+=message;
    //发送消息
    ChatRoomClient::getClient()->client->write(tp);
    ui->plainTextEdit->clear();


}

void ChatRoomMainWindow::recvMessage()
{

    QString buffer=ChatRoomClient::getClient()->client->readAll();
    if(ChatRoomClient::getClient()->serverMessage.size()!=0)
    {
        buffer=ChatRoomClient::getClient()->serverMessage+buffer;
        ChatRoomClient::getClient()->serverMessage.clear();
    }
    while(buffer.size()!=0)
    {
        qDebug()<<buffer;
        int pos=buffer.indexOf("||");
        QString tp=buffer.left(pos);
        buffer=buffer.mid(pos+2);
        if(tp.left(3)=="/01")//用户发送的文字消息
        {
            pos=tp.indexOf("|");

            QString user=tp.mid(3,pos-3);
            QColor userclrR(0,205,205);

            stringToHtml(user,userclrR);
            ui->TextBrowser->moveCursor(QTextCursor::End);
            ui->TextBrowser->textCursor().insertHtml(user);
            ui->TextBrowser->textCursor().insertText("\n");

            QString message=tp.mid(pos+1);
            QColor messageclrR(255,0,0);

            stringToHtml(message,messageclrR);
            ui->TextBrowser->moveCursor(QTextCursor::End);          
            ui->TextBrowser->textCursor().insertHtml(message);
            ui->TextBrowser->textCursor().insertText("\n");
            ui->TextBrowser->moveCursor(QTextCursor::End);
        }
        else if(tp.left(3)=="/02")//新用户加入的消息
        {
          pos=tp.indexOf("|");
          QString newUserName=tp.mid(3);
          //将新用户名加入显示的用户列表
          ChatRoomClient::getClient()->userList.push_back(newUserName);
          //初始化ChatRoom的用户列表
          QString userList="";
          for(auto user:ChatRoomClient::getClient()->userList)
              userList=userList+user+"\n";

          ui->UserList->setText(userList);

          QColor newUserNameclrR(0,100,0);
          newUserName=newUserName+" 加入了 "+ChatRoomClient::getClient()->roomName;
          stringToHtml(newUserName,newUserNameclrR);
          ui->TextBrowser->moveCursor(QTextCursor::End);      
          ui->TextBrowser->textCursor().insertHtml(newUserName);
          ui->TextBrowser->textCursor().insertText("\n");
          ui->TextBrowser->moveCursor(QTextCursor::End);
        }
        else if(tp.left(3)=="/03")//用户退出的消息
        {
            QString quitUserName=tp.mid(3);
            pos=ChatRoomClient::getClient()->userList.indexOf(quitUserName);
            if(pos!=-1)
                ChatRoomClient::getClient()->userList.removeAt(pos);


            QString userList="";
            for(auto user:ChatRoomClient::getClient()->userList)
                userList=userList+user+"\n";

            ui->UserList->setText(userList);

            QColor quitUserNameclrR(0,100,0);
            quitUserName=quitUserName+" 退出了 "+ChatRoomClient::getClient()->roomName;
            stringToHtml(quitUserName,quitUserNameclrR);
            ui->TextBrowser->moveCursor(QTextCursor::End);
            ui->TextBrowser->textCursor().insertHtml(quitUserName);
            ui->TextBrowser->textCursor().insertText("\n");
            ui->TextBrowser->moveCursor(QTextCursor::End);
        }
        else if(tp.left(3)=="/04")//用户列表
        {

            tp=tp.mid(3);
            tp=tp+"|";
            while(tp.size()!=0)
            {
                pos=tp.indexOf("|");
                QString userName=tp.left(pos);
                tp=tp.mid(pos+1);
                ChatRoomClient::getClient()->userList.push_back(userName);

            }
            //初始化ChatRoom的用户列表
            QString userList="";
            for(auto user:ChatRoomClient::getClient()->userList)
                userList=userList+user+"\n";

            ui->UserList->setText(userList);
        }
    }
}

void stringToHtml(QString &str,QColor crl)
{
    //注意这几行代码的顺序不能乱，否则会造成多次替换
    str.replace("&","&amp;");
    str.replace(">","&gt;");
    str.replace("<","&lt;");
    str.replace("\"","&quot;");
    str.replace("\'","&#39;");
    str.replace(" ","&nbsp;");
    str.replace("\n","<br>");
    str.replace("\r","<br>");

    QByteArray array;
    array.append(crl.red());
    array.append(crl.green());
    array.append(crl.blue());
    QString strC(array.toHex());
    str = QString("<span style=\" color:#%1;\">%2</span>").arg(strC).arg(str);
}
