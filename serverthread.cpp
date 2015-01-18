#include "serverthread.h"
#include "mediacontrolsite.h"

ServerThread::ServerThread(QObject *parent) :
    QThread(parent)
{

}

void ServerThread::run()
{
    site = new MediaControlSite(this);
}
