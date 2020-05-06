#include "Encryption.h"

Encryption::Encryption()
{

}

void Encryption::convertQStringToBinary(QString message)
{
    qDebug()<<message;
    QByteArray tp="";
    tp+=message;
    tp=tp.toBase64();
    qDebug()<<tp;
    tp=QByteArray::fromBase64(tp);
    message=tp;
    qDebug()<<message;

    int x=1224;
    qDebug()<<QString::number(x,2);

}
