#pragma once
#include <QMainWindow>
#include <QThread>
#include <QTimer>
#include <QLabel>


namespace Ui {
class ConnectServerSubWindow;
}

class ConnectServerSubWindow : public QMainWindow
{
    Q_OBJECT



public:
    QString roomList;
    explicit ConnectServerSubWindow(QWidget *parent = nullptr);
    ~ConnectServerSubWindow();
signals:
    void connectedToServer();


private:
    Ui::ConnectServerSubWindow *ui;
    QTimer *timer;




private slots:
    void recvAndshowRoomList();
    void connectServer();


};


