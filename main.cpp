#include "ChatRoomClient.h"
#include "ChatRoomMainWindow.h"
#include "ConnectServerSubWindow.h"
#include "ChatRoomListWidget.h"
#include <QApplication>
#include <Encryption.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);


    //Encryption A;

    //A.convertQStringToBinary("大马哈鱼");


    ChatRoomMainWindow* mainWindow=new ChatRoomMainWindow;
    mainWindow->hide();

    return a.exec();
}
