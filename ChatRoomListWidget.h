#pragma once
#include <QWidget>
#include "ChatRoomClient.h"
#include <QTimer>

namespace Ui {
class ChatRoomListWidget;
}

class ChatRoomListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ChatRoomListWidget(QWidget *parent = nullptr);
    ~ChatRoomListWidget();
    void setRoomList(QString roomList);


signals:
    void iminChatRoom();

private:
    Ui::ChatRoomListWidget *ui;
    QTimer *timer;
public slots:
    void createChatRoom();
    void enterChatRoom();
    void inChatRoom();
};

