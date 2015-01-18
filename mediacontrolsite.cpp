#include "mediacontrolsite.h"

MediaControlSite::MediaControlSite(QObject *parent) : QObject(parent)
{
    server = new QTcpServer(this);

    connect(server,SIGNAL(newConnection()),this,SLOT(newConnection()));

    if (!server->listen(QHostAddress::Any,1234))
    {
        qDebug() << "Server could not start";
    }
    else
    {
        qDebug() << "Server started";
    }
}

void MediaControlSite::newConnection()
{
    socket = server->nextPendingConnection();

    socket->write("Hello\r\n");
    socket->flush();
    //socket->waitForBytesWritten(3000);

    connect(socket,SIGNAL(readyRead()),this->parent()->parent(),SLOT(userDataWritten()),Qt::DirectConnection);
}
