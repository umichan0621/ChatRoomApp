#pragma once
#include <QMainWindow>
#include "ChatRoomClient.h"
#include "ChatRoomListWidget.h"
#include "ConnectServerSubWindow.h"




QT_BEGIN_NAMESPACE
namespace Ui { class ChatRoomMainWindow; }
QT_END_NAMESPACE

class ChatRoomMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    ChatRoomMainWindow(QWidget *parent = nullptr);
    ~ChatRoomMainWindow();
public slots:
    void showTheRoomList();
    void enterTheChatRoom();
    void sendMessage();
    void recvMessage();

private:

    ChatRoomListWidget *listWidget;
    ConnectServerSubWindow *subWindow;
    Ui::ChatRoomMainWindow *ui;
    QTimer *timer;
    virtual void keyPressEvent(QKeyEvent *ev);

};
void stringToHtmlFilter(QString &str);
void stringToHtml(QString &str,QColor crl);
